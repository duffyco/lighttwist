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
// FASexpo
// on genere des images a partir de textes
// on affiche le tout a travers une homographie
//

#include <imgu.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

//#define MAISON
#ifdef MAISON
	#define LUT_NAME "./lut.png"
	#define CMD_NAME "/home/roys/projet/atrium/inkscape/subst.tcl"
	#define TEMPLATE_NAME "/home/roys/projet/atrium/inkscape/template2.svg"
	#define THESE_NAME "/home/roys/projet/atrium/theses.txt"
	#define URL "wget --quiet -O - http://192.168.15.201/htdocs/projets/fas/download.php"
#else
	#define LUT_NAME "./lut.png"
	#define CMD_NAME "/home/vision/projet/atrium/inkscape/subst.tcl"
	#define TEMPLATE_NAME "/home/vision/projet/atrium/inkscape/template2.svg"
	#define THESE_NAME "/home/vision/projet/atrium/theses.txt"
	#define URL "wget --quiet -O - http://vision3d.iro.umontreal.ca/expofas/download.php"
	//#define CAPSULES "/home/vision/projet/atrium/capsules/capsule%d.png"
	#define CAPSULES "/home/vision/projet/atrium/capsules/capsule_%d_vecto.png"
	#define CAPSULES_NB	8
#endif


static int theseSize;
static char *theseBuf; // [theseSize]

typedef struct {
	int match; // temporaire
	double prob; // probabilite (en fait un poids)
	double cumul;
	char *departement;
	char *date;
	char *auteur;
	char *titre;
} these;

static int nbT=0;
static these *T; // [nbT] point dans theseBuf

static char* rien = "ALLO";

static int readThese(void)
{
FILE *F;
int size;
int k,i;
	F=fopen(THESE_NAME,"r");
	if( F==NULL ) { printf("unable to open %s\n",THESE_NAME);exit(0); }

	// read the whole file
	fseek(F,0L,SEEK_END);
	size=ftell(F);
	printf("Size is %d\n",size);

	theseSize=size+1;
	theseBuf=(char *)malloc(theseSize); // xxxxx

	fseek(F,0L,SEEK_SET);
	k=fread(theseBuf,size,1,F);
	if( k!=1 ) { printf("IO error read these.\n");exit(0); }

	theseBuf[size]=0;

	fclose(F);

	// compte le nb de lignes
	int nbL=0;
	for(i=0;i<theseSize;i++) { if(theseBuf[i]=='\n') nbL++; }
	printf("nbLignes = %d\n",nbL);

	nbT=nbL/4;
	T=(these *)malloc(nbT*sizeof(these));

	int t;
	for(t=0;t<nbT;t++) {
		T[t].auteur=rien;
		T[t].date=rien;
		T[t].titre=rien;
		T[t].departement=rien;
	}

	t=0;
	for(i=0;t<nbT && i<theseSize;) {
		T[t].departement=theseBuf+i;
		while( theseBuf[i]!='\n' && theseBuf[i]!=0 ) i++;
		if( theseBuf[i]==0 ) break; else theseBuf[i++]=0;

		T[t].date=theseBuf+i;
		while( theseBuf[i]!='\n' && theseBuf[i]!=0 ) i++;
		if( theseBuf[i]==0 ) break; else theseBuf[i++]=0;

		T[t].auteur=theseBuf+i;
		while( theseBuf[i]!='\n' && theseBuf[i]!=0 ) i++;
		if( theseBuf[i]==0 ) break; else theseBuf[i++]=0;

		T[t].titre=theseBuf+i;
		while( theseBuf[i]!='\n' && theseBuf[i]!=0 ) i++;
		if( theseBuf[i]==0 ) break; else theseBuf[i++]=0;

		t++;
	}

	printf("t=%d should be %d\n",t,nbT);

	int maxTitre=0;
	int maxAuteur=0;
	for(t=0;t<nbT;t++) {
		if( strlen(T[t].titre) > maxTitre ) maxTitre=strlen(T[t].titre);
		if( strlen(T[t].auteur) > maxAuteur ) maxAuteur=strlen(T[t].auteur);
	}
	printf("MaxTitre=%d maxAuteur=%d\n",maxTitre,maxAuteur);

/*
	for(t=0;t<nbT;t++) {
		printf("%4d : %s : %s : %s : %s\n",t,T[t].auteur,T[t].date,T[t].departement,T[t].titre);
	}
*/


	return(0);
}

static int timeIsPassing(double vitesse)
{
int i;
	for(i=0;i<nbT;i++) T[i].prob*=vitesse;
	return(0);
}

static int checkKeyWordThese(char *mot,int date)
{
int i;
int nb=0;
	for(i=0;i<nbT;i++) {
		T[i].match=0;
		if( strstr(T[i].auteur,mot) ||
		 strstr(T[i].departement,mot) ||
		 strstr(T[i].date,mot) ||
		 strstr(T[i].titre,mot) )  {
			T[i].match=1;
			nb++;
		}
	}
	printf("KEYWORD %s match %d entry\n",mot,nb);
	for(i=0;i<nbT;i++) {
		if( T[i].match==0 ) continue;
		T[i].prob+=500.0;
		printf("Entry '%s' match %s\n",T[i].auteur,mot);
	}
	return(0);
}

static int updateKeywords(void)
{
FILE *F;
char buf[500];
int date;
char mot[100];
	printf("*** UPDATING ***\n");
	F=popen(URL,"r");
	if( F==NULL ) { printf("Unable to wget\n");return(-1); }

	while( fgets(buf,500,F)==buf ) if( strcmp(buf,"<pre>\n")==0 ) break;
	while( fgets(buf,500,F)==buf ) {
		if( buf[0]=='#' ) continue;
		if( strcmp(buf,"</pre>\n")==0 ) break;
		if( sscanf(buf,"%d %s",&date,mot)!=2 ) break;
		printf("Check mot '%s' date=%d\n",mot,date);
		checkKeyWordThese(mot,date);
	}
	fclose(F);
	printf("*** DONE UPDATING ***\n");
	return(0);
}

// choisi une these en fonction des probabilites
static int choisirThese(void)
{
int i;
double max;
double pos;
	for(i=1;i<nbT;i++) T[i].cumul=T[i-1].cumul+T[i].prob;
	max=T[nbT-1].cumul;
	printf("Maximum is %f\n",max);

	if( max<0.01 ) {
		// tout est a zero!! On reset!!
		for(i=0;i<nbT;i++) T[i].prob=1.0;
		for(i=1;i<nbT;i++) T[i].cumul=T[i-1].cumul+T[i].prob;
		max=T[nbT-1].cumul;
	}

	pos=drand48()*max;
	for(i=0;i<nbT;i++) {
		if( pos<=T[i].cumul ) break;
	}
	printf("J'ai choisi %d (pos = %f%%)\n",i,pos/max*100.0);

	// reset la prob de celui-ci
	T[i].prob=0.0;
	return(i);
}


int lireCapsule(imgu **pIA,int capsuleNum)
{
//
char buf[300];
int i;
	printf("CAPSULE %d!!\n",capsuleNum);
	sprintf(buf,CAPSULES,capsuleNum+1);

	//*pIA=NULL;
	int k=imguLoad(pIA,buf,LOAD_AS_IS);
	if( k ) { printf("Unable to read capsule '%s'\n",buf);return(-1); }

	imgu *IA=*pIA;
	printf("loaded image (%d,%d,%d)\n",IA->xs,IA->ys,IA->cs);
	for(i=0;i<IA->xs*IA->ys;i++) {
		IA->data[i*IA->cs+0]<<=8;
		IA->data[i*IA->cs+1]<<=8;
		IA->data[i*IA->cs+2]<<=8;
	}
	return(0);
}

// on a choisi la these t

int generateImage(imgu **pIA,int t)
{
imgu *IA;
int k,i;
static char cmd[3000];
static char titre[1000];
static char auteur[300];
static char date[30];
static char departement[100];
	if( t<0 || t>=nbT ) { printf("Illegal t=%d\n",t);*pIA=NULL;return(-1); }

	strcpy(titre,T[t].titre);
	strcpy(auteur,T[t].auteur);
	strcpy(date,T[t].date);
	strcpy(departement,T[t].departement);
	sprintf(cmd,"%s %s outout.png \"%s\" \"%s\" \"%s\" \"%s\"",CMD_NAME,TEMPLATE_NAME,titre,auteur,date,departement);
	k=system(cmd);
	//k=system("/home/roys/projet/atrium/inkscape/subst.tcl /home/roys/projet/atrium/inkscape/template1.svg outout.png 'un titre' 'seb roy' 1995");
	printf("k=%d\n",k);

	//*pIA=NULL;
	k=imguLoad(pIA,"outout.png",LOAD_AS_IS);
	if( k ) {
		printf("Unable to load!\n");
		imguAllocate(pIA,640,384,3);
		IA=*pIA;
		for(i=0;i<IA->xs*IA->ys*IA->cs;i++) {
			IA->data[i]=rand()%IMGU_MAXVAL;
		}
	}
	IA=*pIA;
	printf("loaded image (%d,%d,%d)\n",IA->xs,IA->ys,IA->cs);
	for(i=0;i<IA->xs*IA->ys;i++) {
		IA->data[i*IA->cs+0]<<=8;
		IA->data[i*IA->cs+1]<<=8;
		IA->data[i*IA->cs+2]<<=8;
	//=rand()%IMGU_MAXVAL;
	}
	return(0);
}


static int doFASExpo(void)
{
rqueue* Qrecycle;
rqueue* Qdisplay;
int tid_view;   // viewer
imgu *IA;
int k,i;

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    Qdisplay=imguRegisterQueue("display");

#ifdef MAISON
    tid_view=imguStartPlugin("viewer","sink","-in display -view0 rgblutblend 0 0 1 1 -geom 0 0 960 540 false");
#else
    tid_view=imguStartPlugin("viewer","sink","-in display -view0 rgblutblend 0 0 1 1 -geom 1920 0 1920 1080 false");
#endif
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    paramlist *pl_view=imguGetPluginParameters(tid_view);

	// on doit envoyer la lut!
	IA=NULL;
	k=imguLoad(&IA,LUT_NAME,LOAD_AS_IS);
	if( k ) { printf("Unable to load lut '%s'\n",LUT_NAME); exit(0); }
	imguSetRecycleQueue(IA,Qrecycle);

	imguReplaceAddText(IA,"UNIFORM","lut");
	imguReplaceAddText(IA,"VIEWNUM","0");
	RQueueAddLast(Qdisplay,(void *)&IA);

    int n,t;
    int capsule=0;
    int capsuleNum=0;
    for(n=0;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }

	if( capsule==0 && n%15==0 ) { capsule=1;capsuleNum=0; }

	if( capsule ) {
		IA=NULL;
		RQueueRemoveFirst(Qrecycle,(void *)&IA);

		if( lireCapsule(&IA,capsuleNum)==0 ) {
			imguSetRecycleQueue(IA,Qrecycle);

			imguReplaceAddText(IA,"VIEWNUM","0");
			printf("Affiche!\n");
			RQueueAddLast(Qdisplay,(void *)&IA);
		}
		sleep(15);
		capsuleNum++;
		if( capsuleNum>=CAPSULES_NB ) { capsule=0;n++; }
	}else{

		IA=NULL;
		RQueueRemoveFirst(Qrecycle,(void *)&IA);

		//t=rand()%nbT;

		t=choisirThese();
		generateImage(&IA,t);

		imguSetRecycleQueue(IA,Qrecycle);

		//char buf[100];
		//sprintf(buf,"tmp%04d.png",n);
		//imguSave(IA,buf,1,SAVE_AS_IS);

		// important!!!!
		imguReplaceAddText(IA,"VIEWNUM","0");

		// affiche 
		printf("Affiche!\n");
		RQueueAddLast(Qdisplay,(void *)&IA);
		sleep(10);
		n++;
		updateKeywords();
		timeIsPassing(0.7);
	}

    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    //imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("display",NULL);

    return(0);
}

int main( int argc, char **argv )
{
    srand(time(NULL));
    srand48(time(NULL));

    readThese();
    doFASExpo();
}


