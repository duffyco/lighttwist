/*
 * This file is part of IMGU.
 *
 * @Copyright 2008 Université de Montréal, Laboratoire Vision3D
 *   Sébastien Roy (roys@iro.umontreal.ca)
 *   Vincent Chapdelaine-Couture (chapdelv@iro.umontreal.ca)
 *   Lucie Bélanger
 *   Jamil Draréni
 *
 * IMGU is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * IMGU is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with IMGU.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// Diapo analyzer
//
// usage:  imguDiapoSubtitle8 diapos.pdf presentation.ogg
//

#include <imgu.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <glob.h>


typedef struct {
	int slide; // best slide (or -1 si "no match")
	double start;
	double end;
	char txt[32];
	double cpp; // cost per pixel
} sequence;

#define MAXSEQ	10000
int nbSeq;
sequence Seq[MAXSEQ];


int nbPdfImg;
imgu **pdfImg; // [nbPdfImg]
int pdfXs,pdfYs;

typedef struct {
	int level;
	char *title;
} bookmark;
bookmark *bm; // [nbPdfImg] [slide] avec slide=0..nbPdfImg-1

rqueue *Qsee;
rqueue *Qpoubelle;

imgu *Iindien;

int ScaleSmaller(imgu *IA,imgu *IB);

// assume IA is source
// assume IB is allocated with the correct size
int ScaleSmaller(imgu *IA,imgu *IB)
{
int sz;
int *cumul; // [sz]
int *nb; // [sz]
int i,x,y,c,xx,yy;

	if( IA->cs!=IB->cs ) { printf("unable to scale cs IA is %d, IB is %d\n",IA->cs,IB->cs);exit(0); }

	sz=IB->xs*IB->ys*IB->cs;
	cumul=(int *)malloc(sz*sizeof(int));
	nb=(int *)malloc(sz*sizeof(int));

	for(i=0;i<sz;i++) cumul[i]=0;
	for(i=0;i<sz;i++) nb[i]=0;

	for(y=0;y<IA->ys;y++) for(x=0;x<IA->xs;x++) {
		xx=(x*(IB->xs-1)*2+1)/(2*(IA->xs-1));
		yy=(y*(IB->ys-1)*2+1)/(2*(IA->ys-1));
		if( xx<0 || xx>=IB->xs || yy<0 || yy>=IB->ys ) { printf("YO\n");exit(0); }

		for(c=0;c<IA->cs;c++) {
			cumul[INDEX(IB,xx,yy,c)]+=PIXEL(IA,x,y,c);
			nb[INDEX(IB,xx,yy,c)]+=1;
		}
	}

	for(i=0;i<sz;i++) if( nb[i]==0 ) nb[i]=1;
	for(i=0;i<sz;i++) IB->data[i]=(cumul[i]*2+1)/(2*nb[i]);

	free(cumul);
	free(nb);
	return(0);
}


int compareImages(imgu *IA,imgu *IB)
{
int sum;
int sz;
int i,k;
	if( IA->xs!=IB->xs || IA->ys!=IB->ys || IA->cs!=IB->cs ) return(-1);

	sz=IA->xs*IA->ys*IA->cs;

	pix_t *p=IA->data;
	pix_t *q=IB->data;

	sum=0;
	for(;sz>0;sz--,p++,q++) sum+= (*p>=*q)?(*p - *q):(*q - *p);
	return(sum);
}



// best ans bestsum are [2]
int findBestPdf(imgu *IA,int *best,int *bestSum,int n)
{
static imgu *IB=NULL;
int sum;
int sz;
int i,k;
	if( IA->xs>pdfXs || IA->ys>pdfYs ) {
		imguAllocate(&IB,pdfXs,pdfYs,3);
		ScaleSmaller(IA,IB);
		//imguScale(&IB,IA,(double)pdfXs/IA->xs,(double)pdfYs/IA->ys);
	}else imguCopy(&IB,IA);
	//printf("New image size is (%d,%d)\n",IB->xs,IB->ys);

	// affiche!
	imgu *IC=NULL;
	RQueueRemoveFirst(Qpoubelle,(void*)&IC);
	imguCopy(&IC,IB);
	imguSetRecycleQueue(IC,Qpoubelle);
	imguReplaceAddText(IC,"VIEWNUM","0");
	RQueueAddLast(Qsee,(void *)&IC);

/**
	char buf[100];
	sprintf(buf,"out%04d.png",n);
	imguSave(IB,buf,1,SAVE_AS_IS);
**/

	best[0]=best[1]=-1;
	bestSum[0]=bestSum[1]=0;

	sz=IB->xs*IB->ys*IB->cs;

	//printf("pdf is %d x %d x %d\n",pdfImg[0]->xs,pdfImg[0]->ys,pdfImg[0]->cs);

	pix_t *p,*q;

	for(i=0;i<nbPdfImg;i++) {
		sum=0;
		p=IB->data;
		q=pdfImg[i]->data;
		for(k=0;k<sz;k++,p++,q++) sum+= (*p>=*q)?(*p - *q):(*q - *p);
		//printf(" pdf match %3d is cost %7d\n",i,sum);
		if( best[0]<0 || sum<bestSum[0] ) {  bestSum[0]=sum;best[0]=i; }
		else if( best[1]<0 || sum<bestSum[1] ) {  bestSum[1]=sum;best[1]=i; }
	}

	if( bestSum[1]<bestSum[0] ) {
		int z=bestSum[0];
		bestSum[0]=bestSum[1];
		bestSum[1]=z;
		z=best[0];best[0]=best[1];best[1]=z;
	}

	return(best[0]);
}

void time2txt(double time,char *buf)
{
int h,m;
double s;
	h=(int)(time/3600);
	m=(int)((time-h*3600)/60);
	s=time-h*3600-m*60;
	sprintf(buf,"%02d:%02d:%06.3f",h,m,s);
}

void time2txtSimple(double time,char *buf)
{
int h,m;
double s;
	m=(int)((time)/60);
	s=time-m*60;
	sprintf(buf,"%2d:%02d",m,(int)s);
}

static int doVideoMatch(char *movieName,double minChange,double maxChange)
{
imgu *IA;
rqueue* Qrecycle;
rqueue* Qmovie;
//rqueue* Qsee;
int tid_movie;    // ffmpeg player
int tid_view;   // viewer
int i,k;
char buf[200];
int n;

	// create a recycling queue
	Qpoubelle=imguRegisterQueue("poubelle");
	Qrecycle=imguRegisterQueue("recycle");
	Qmovie=imguRegisterQueue("movie");
	Qsee=imguRegisterQueue("see");

	sprintf(buf,"-in recycle -out movie -file %s -fps fast -outformat rgb",movieName);
        tid_movie=imguStartPlugin("ffmpeg","camera",buf);
        if( tid_movie<0 ) { printf("Could not start the ffmpeg plugin\n");exit(-1); }

	// start the viewer
	sprintf(buf,"-in see -view0 rgb 0 0 0.5 1 -view1 rgb 0.5 0 1 1 -geom 0 0 1024 384 true");
	tid_view=imguStartPlugin("viewer","sink",buf);
	if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

	paramlist* pl_movie=imguGetPluginParameters(tid_movie);
	paramlist* pl_view=imguGetPluginParameters(tid_view);

	// ne sert a rien pour un movie...
	paramInvokeCommand(pl_movie,"START");

	// quelques images de recyclage
	IA=NULL;
	for(i=0;i<20;i++) RQueueAddLast(Qrecycle,(void *)&IA);

	IA=NULL;
	for(i=0;i<20;i++) RQueueAddLast(Qpoubelle,(void *)&IA);

	imguReplaceAddText(IA,"VIEWNUM","0");

	int best[2];
	int cost[2];
	imgu *IB;

	int currentBest=-1;
	int startFrame=-1;
	int currentBestCost=-1;

	double startTime=-1.0;
	double time;

	imgu *IL=NULL; // last frame from movie...
	double lastTime=-1.0;

	// main loop: read image, find closest pdf, output result
	for(n=0;;n++) {
		paramGetInt(pl_view,"done",&k);
		if( k ) { printf("We are done (by user)!\n");break; }

		// lire une image du movie
		while (RQueueRemoveFirstWait(Qmovie,(void*)&IA)) ;


		time=atof(imguGetText(IA,"VIDEOTIMESTAMP"));
		int loop=atoi(imguGetText(IA,"LOOPNUM"));

		//printf("time %12.6f  loop=%d\n",time,loop);

		if( loop>0 ) break;

		// si pas premier frame, compare au frame precedent...
		if( IL!=NULL ) {
			int change=compareImages(IL,IA);
			//printf("@ %8d\n",change);
			if( change<(int)(minChange*IA->xs*IA->ys) ) {
				// same!!!
				imguRecycle(IL);
				IL=IA;
				lastTime=time;
				continue;
			}
		}

		// match l'image au pdf
		findBestPdf(IA,best,cost,n);

		// check NO MATCH
		if( cost[0]>maxChange*IA->xs*IA->ys*IA->cs ) best[0]=-1; // no match

		//printf("%5d : best is pdf %3d , cost = %7d  : %3d,%7d\n",n,best[0],cost[0],best[1],cost[1]);
		//printf("{ %d, %d, %d,%d,%d}, \n",n,best[0],cost[0],best[1],cost[1]);

		if( currentBest!=best[0] ) {
			if( startFrame>=0 ) {
				//printf("sequence %d : %d - %d\n",currentBest,startFrame,n-1);
				char from[30];
				char to[30];
				time2txt(startTime,from);
				time2txt(lastTime,to);
				printf("sequence %d : %d - %d : %12.6f - %12.6f : %s -> %s\n",currentBest,startFrame,n-1,startTime,lastTime,from,to);
				if( nbSeq==MAXSEQ ) { printf("SKIP sequence");exit(0); }
				Seq[nbSeq].slide=currentBest;
				Seq[nbSeq].start=startTime;
				Seq[nbSeq].end=lastTime; // or time
				Seq[nbSeq].cpp=(double)currentBestCost/IA->xs/IA->ys/IA->cs;
				sprintf(Seq[nbSeq].txt,"diapo %d",currentBest);
				nbSeq++;
			}
			currentBest=best[0];
			currentBestCost=cost[0];
			startFrame=n;
			startTime=time;
		}

		// afficher cette image
		//RQueueAddLast(Qsee,(void *)&IA);
		if( IL!=NULL ) imguRecycle(IL);
		IL=IA;
		lastTime=time;

		// afficher son match pdf
		IB=NULL;
		RQueueRemoveFirst(Qpoubelle,(void*)&IB);
		if( best[0]>=0 ) 	imguCopy(&IB,pdfImg[best[0]]);
 		else			imguCopy(&IB,Iindien);
		imguSetRecycleQueue(IB,Qpoubelle);
		imguReplaceAddText(IB,"VIEWNUM","1");
		RQueueAddLast(Qsee,(void *)&IB);

		//sleep(1);
		
	}

	// last sequence
	if( startFrame>=0 && startFrame<n ) {
		printf("sequence %d : %d - %d\n",currentBest,startFrame,n-1);
		if( nbSeq==MAXSEQ ) { printf("SKIP sequence");exit(0); }
		Seq[nbSeq].slide=currentBest;
		Seq[nbSeq].start=startTime;
		Seq[nbSeq].end=lastTime; // or time
		sprintf(Seq[nbSeq].txt,"diapo %d",currentBest);
		Seq[nbSeq].cpp=currentBestCost;
		nbSeq++;
	}

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    //imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_movie);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("movie",NULL);
    imguUnregisterQueue("see",NULL);

    return(0);
}



int readPDF(char *pdfName)
{
char buf[200];
char cmd[200];
int k;
int i;
char pwd[200];
char dir[200];
char bookmarkName[200];

	getcwd(pwd,sizeof(pwd));

	sprintf(dir,"/tmp/pdf%d",getpid());
	printf("dir is %s\n",dir);

	sprintf(bookmarkName,"/tmp/bookmark%d",getpid());

	k=mkdir(dir,0777);
	printf("k=%d\n",k);

	k=chdir(dir);
	printf("k=%d\n",k);

	pdfXs=512;
	pdfYs=384;

	// Lecture du pdf
	char prefix[200];
	if( pdfName[0]=='/' ) prefix[0]=0;
	else { strcpy(prefix,pwd);strcat(prefix,"/"); }

	printf("extraction des bookmars...\n");
	sprintf(cmd,"pdfbookmarks %s%s >%s",prefix,pdfName,bookmarkName);
	printf("--- cmd ---\n",cmd);

	k=system(cmd);

	printf("conversion du pdf...\n");
	//sprintf(cmd,"convert %s -resize %dx%d -quality 95 blub%%04d.jpg",pdfName,pdfXs,pdfYs);
	sprintf(cmd,"convert -density 200 %s%s blub%%04d.gif",prefix,pdfName);
	printf("%s\n",cmd);
	//sprintf(cmd,"gs -q -sDEVICE=png48 -dBATCH -dNOPAUSE -r200  -sOutputFile=blub%%04d.png %s",pdfName);
	k=system(cmd);
	printf("convert k=%d\n",k);

	glob_t g;
	k=glob("*",0,NULL,&g);
	printf("glob k=%d\n",k);

	printf("nbfichier = %d\n",(int)g.gl_pathc);

	nbPdfImg=g.gl_pathc;
	pdfImg=(imgu **)malloc(nbPdfImg*sizeof(imgu *));

	// lecture des bookmarks
	bm=(bookmark *)malloc(sizeof(bookmark)*nbPdfImg);
	// valeurs par default des bookmark...
	for(i=0;i<nbPdfImg;i++) { bm[i].level=-1; bm[i].title=NULL; }
	{
		int level,slide;
		char title[200];
		FILE *F=fopen(bookmarkName,"r");
		if( F!=NULL ) {
			while( fgets(buf,sizeof(buf),F)==buf ) {
				sscanf(buf," %d %d BookmarkTitle: %[^\n]",&level,&slide,title);
				i=slide-1;
				bm[i].level=level;
				// trim
				int j;
				for(j=strlen(title)-1;j>=0 && isspace(title[j]);j--) title[j]=0;
				bm[i].title=strdup(title);
			}
			fclose(F);
		}
		unlink(bookmarkName);
		for(i=0;i<nbPdfImg;i++) {
		  if( bm[i].level>=0 ) printf("*** L=%2d D=%2d '%s'\n",bm[i].level,i,bm[i].title);
		  else printf("*** D=%2d ????\n",i);
		}
	}

	imgu *IZ;
	imgu *IY;
	for(i=0;i<g.gl_pathc;i++) {
		printf("file %3d is %s\n",i,g.gl_pathv[i]);
		sprintf(cmd,"convert %s out%04d.png",g.gl_pathv[i],i);
		k=system(cmd);

/**
		sprintf(buf,"out%04d.png",i);
		pdfImg[i]=NULL;
		k=imguLoad(&pdfImg[i],buf,LOAD_AS_IS);
		if( k ) { printf("Unable to load %s\n",buf);exit(0); }
**/

		sprintf(buf,"out%04d.png",i);
		IZ=NULL;
		k=imguLoad(&IZ,buf,LOAD_AS_IS);
		if( k ) { printf("Unable to load %s\n",buf);exit(0); }
		pdfImg[i]=NULL;
		imguAllocate(&pdfImg[i],pdfXs,pdfYs,3);

		printf("file %s has cs=%d\n",buf,IZ->cs);

		// convert to 3 colors
		if( IZ->cs>3 ) {
			IY=NULL;
			imguAllocate(&IY,IZ->xs,IZ->ys,3);
			int x,y,c;
			for(y=0;y<IZ->ys;y++)
			for(x=0;x<IZ->xs;x++)
			for(c=0;c<3;c++) {
				PIXEL(IY,x,y,c)=PIXEL(IZ,x,y,c);
			}
			imguFree(&IZ);
			IZ=IY;
			imguSetRecycleQueue(IZ,Qpoubelle);
		}else if( IZ->cs==1 ) {
			IY=NULL;
			imguAllocate(&IY,IZ->xs,IZ->ys,3);
			int x,y,c;
			for(y=0;y<IZ->ys;y++)
			for(x=0;x<IZ->xs;x++)
			for(c=0;c<3;c++) {
				PIXEL(IY,x,y,c)=PIXEL(IZ,x,y,0);
			}
			imguFree(&IZ);
			IZ=IY;
			imguSetRecycleQueue(IZ,Qpoubelle);
		}
		ScaleSmaller(IZ,pdfImg[i]);

		unlink(g.gl_pathv[i]);
		unlink(buf);
	}

	//pdfXs=pdfImg[0]->xs;
	//pdfYs=pdfImg[0]->ys;

	globfree(&g);


	chdir(pwd);

	k=rmdir(dir);
	printf("k=%d\n",k);

	return(0);
}

int dumpSeq(char *sbvName,char *xmlName)
{
char from[30];
char to[30];
char len[30];
char lens[30];
int i;
	if( sbvName[0]!=0 ) {
		FILE *F=fopen(sbvName,"w");
		if( F==NULL ) { printf("Unable to open %s\n",sbvName);return(-1); }
		for(i=0;i<nbSeq;i++) {
			time2txt(Seq[i].start,from);
			time2txt(Seq[i].end,to);
			if( Seq[i].slide>=0 ) 
				if( bm[Seq[i].slide].title!=NULL ) {
					fprintf(F,"%s,%s\n%s\n\n",from,to,bm[Seq[i].slide].title);
				}else{
					fprintf(F,"%s,%s\nDiapo %d (err=%6.2f)\n\n",from,to,Seq[i].slide+1,Seq[i].cpp);
				}
			else
				fprintf(F,"%s,%s\n? ? ? (err=%6.2f)\n\n",from,to,Seq[i].cpp);
		}
		fclose(F);
	}
	if( xmlName[0]!=0 ) {
		FILE *F=fopen(xmlName,"w");
		if( F==NULL ) { printf("Unable to open %s\n",xmlName);return(-1); }

		fprintf(F,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(F,"<tt xmlns:tts=\"http://www.w3.org/2006/04/ttaf1#styling\" xmlns=\"http://www.w3.org/2006/04/ttaf1\">\n");
		fprintf(F,"  <head/>\n");
		fprintf(F,"  <body>\n");
		fprintf(F,"    <div>\n");
		for(i=0;i<nbSeq;i++) {
			time2txt(Seq[i].start,from);
			time2txt(Seq[i].end-Seq[i].start,len);
			time2txtSimple(Seq[i].end-Seq[i].start,lens);
			time2txt(Seq[i].end,to);
			if( Seq[i].slide>=0 )  {
				//fprintf(F,"<p begin=\"%s\" dur=\"%s\">Diapo %d (%s , %s)</p>\n",from,len,Seq[i].slide+1,from,to);
			
				if( bm[Seq[i].slide].title!=NULL ) {
					fprintf(F,"<p begin=\"%s\" dur=\"%s\">%s (%s)</p>\n",from,len,bm[Seq[i].slide].title,lens);
				}else{
					fprintf(F,"<p begin=\"%s\" dur=\"%s\">Diapo %d (%s)</p>\n",from,len,Seq[i].slide+1,lens);
				}
			}else{
				//fprintf(F,"<p begin=\"%s\" dur=\"%s\">? ? ? (%s , %s)</p>\n",from,len,from,to);
				fprintf(F,"<p begin=\"%s\" dur=\"%s\">??? (%s)</p>\n",from,len,lens);
			}
		}
		fprintf(F,"    </div>\n");
		fprintf(F,"  </body>\n");
		fprintf(F,"</tt>\n");
		fclose(F);
	}

/*
   {
        id: 8,
        name: "Attempt 'Lauren'",
        seekTime: 63,
        time: 1:07,
        endTime: 69,
        actionEnding: false,
        state: 4,
    },
*/
		FILE *F=fopen("out.js","w");
		if( F==NULL ) { printf("Unable to open %s\n",sbvName);return(-1); }
		for(i=0;i<nbSeq;i++) {
			time2txt(Seq[i].start,from);
			time2txt(Seq[i].end-Seq[i].start,to);
			fprintf(F,"{\nid:%d,\nname: \"diapo %d\",\nseekTime:%d,\ntime:%d,\nendTime:%d,\nactionEnding: false,\nstate:4,\n},\n",i,Seq[i].slide,(int)Seq[i].start,(int)Seq[i].end-2,(int)Seq[i].end);
		}
		fclose(F);
	
}

int mergeSeq(double minLen)
{
int i,j;
double len;
	for(i=0,j=0;i<nbSeq;i++) {
		len=Seq[i].end-Seq[i].start;
		if( len>=minLen ) {
			Seq[j]=Seq[i];
			j++;
		}
	}
	printf("Merge: Before=%d After=%d\n",nbSeq,j);
	nbSeq=j;

	// joindre les sequences semblable
	for(i=1,j=1;i<nbSeq;i++) {
		if( Seq[i].slide==Seq[j-1].slide ) {
			Seq[j-1].end=Seq[i].end;
			continue;
		}
		Seq[j]=Seq[i];
		j++;
	}
	printf("Merge: Before=%d After=%d\n",nbSeq,j);
	nbSeq=j;
}

int main( int argc, char **argv )
{
char movieName[200];
char pdfName[200];
char sbvName[200];
char xmlName[200];
double minChange; // changement minimum moyen par pixel entre images voisines
double maxChange; // maximum de changement pour etre un match avec une diapo
double minLen; // duree minimale d'un intervale...
int i;

	Iindien=NULL;
	if( imguLoad(&Iindien,"/home/roys/Images/rca_indian_head_test_pattern.jpg",LOAD_AS_IS) ) {
		printf("indien?\n");exit(0);
	}

	nbSeq=0;

	/**
	char blub[100];
	time2txt(1*3600+43*60+23.4567,blub);
	printf("time is %s\n",blub);
	exit(0);
	**/



	movieName[0]=0;
	pdfName[0]=0;
	sbvName[0]=0;
	xmlName[0]=0;
	minChange=1.2;
	maxChange=3.0;
	minLen=5.0; // secondes

	for(i=1;i<argc;i++) {
		if( strcmp(argv[i],"-i")==0 && i+1<argc ) {
			strcpy(movieName,argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-pdf")==0 && i+1<argc ) {
			strcpy(pdfName,argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-sbv")==0 && i+1<argc ) {
			strcpy(sbvName,argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-xml")==0 && i+1<argc ) {
			strcpy(xmlName,argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-minchange")==0 && i+1<argc ) {
			minChange=atof(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-maxchange")==0 && i+1<argc ) {
			maxChange=atof(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-minlen")==0 && i+1<argc ) {
			minLen=atof(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-h")==0 ) {
			printf("%s -i movie -pdf diapo.pdf -sbv out-subtitle -xml out-xml -minchange 1.2 -maxchange 3.0 -minlen 5.0\n",argv[0]);
			exit(0);
		}
	}

	if( movieName[0]==0 ) { printf("need movie\n");exit(0); }
	if( pdfName[0]==0 ) { printf("need pdf\n");exit(0); }

	if( readPDF(pdfName) ) exit(0);

/**
	int cost,k;
	for(i=0;i<nbPdfImg;i++) {
		k=findBestPdf(pdfImg[i],&cost,0);
		printf("pdf %d match pdf %d cost %d (%12.6f/pixel)\n",i,k,cost,(double)cost/(pdfXs*pdfYs*3));
	}
	exit(0);
**/


	for(i=0;i<nbPdfImg;i++) {
		char buf[100];
		sprintf(buf,"pdf%04d.png",i);
		imguSave(pdfImg[i],buf,1,SAVE_AS_IS);
	}

	doVideoMatch(movieName,minChange,maxChange);

	mergeSeq(minLen);

	dumpSeq(sbvName,xmlName);

	return(0);

}


