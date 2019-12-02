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

/// pngtrack
//
// liste des cles supportees:
//
// rectangle  :  x1 y1 x2 y2   (double)
//
// Les images paires 0,2,4,... sont une sequence
// Les images impaires 1,3,5,... sont une autre sequence
//
// On track deux rectangles en meme temps.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <imgu.h>

//#define DONT_USE_MASK

typedef struct {
	int x1,y1,x2,y2;
} rect;

#define PATCH_WIDTH	20
#define PATCH_HEIGHT	20





/// soustraction de deux images (en valeur absolue)
// l'image IA sera modifiee
int imguAbsDiff(imgu *IA,imgu *IB)
{
int sz,i,v;
	if( IA->xs!=IA->xs || IA->ys!=IB->ys || IA->cs!=IB->cs ) return(-1);

	sz=IA->xs*IA->ys*IA->cs;
	for(i=0;i<sz;i++) {
		v=(int)IA->data[i]-(int)IB->data[i];
		if( v<0 ) v=-v;
		IA->data[i]=v;
	}
	return(0);
}


// treshold une image
// un masque de bits est fait et ensuite shifte vers le max
// cs=1 -> 0..1 << 15
// cs=3 -> 0..7 << 13
// cs=4 -> 0..15 << 12
imgu *imguThresh(imgu *I,unsigned short seuil,int cut)
{
int i,c,sz;
unsigned short m,mask;
imgu *J;
int nb;

	nb=0;
	imguAllocate(&J,I->xs,I->ys,1);
	sz=I->xs*I->ys;
	for(i=0;i<sz;i++) {
		mask=0;
		for(c=0,m=1;c<I->cs;c++,m<<=1) {
			if( I->data[i*I->cs+c]>seuil ) mask|=m;
		}
		m=mask<<(16-I->cs);
		// seuil encore une fois, sur le nb de canaux
		if( cut>=0 ) { if( m>=cut ) { m=65535;nb++; } else m=0; }
		J->data[i]=m;
	}
	//fprintf(stderr,"### %12.1f\n",(double)nb/sz*100.0);
	return(J);
}

// on donne le poucentage de pixels selectionnes, 0..100
imgu *imguThreshAdaptatif(imgu *I,int percent,int cut)
{
int i,c,sz;
unsigned short m,mask;
imgu *J;
int nb;
unsigned short a,b,seuil; // intervale de seuil
int pourcent;

	a=65535;
	b=0;

	imguAllocate(&J,I->xs,I->ys,1);
	sz=I->xs*I->ys;
	for(;b<a;) {
		seuil=(a+b)/2;
		nb=0;
		for(i=0;i<sz;i++) {
			mask=0;
			for(c=0,m=1;c<I->cs;c++,m<<=1) {
				if( I->data[i*I->cs+c]>=seuil ) mask|=m;
			}
			m=mask<<(16-I->cs);
			// seuil encore une fois, sur le nb de canaux
			if( cut>=0 ) { if( m>=cut ) { m=65535;nb++; } else m=0; }
			J->data[i]=m;
		}
		pourcent=(nb*100)/sz;
		//fprintf(stderr,"### pourcent = %d\n",pourcent);
		if( pourcent >= percent-1 && pourcent <= percent+1 ) break;

		if( pourcent<percent ) {
			// a trop bas, b trop haut, pourcent trop bas
			a=seuil-1;
		}else{
			// a trop bas, b trop haut, pourcent trop haut
			b=seuil+1;
		}
	}
	fprintf(stderr,"### DONE! seuil=%d\n\n",seuil);
	return(J);
}


rect getRect(char *T)
{
double x1,y1,x2,y2;
rect r;
	sscanf(T," %lf %lf %lf %lf",&x1,&y1,&x2,&y2);

	r.x1=(int)(x1+0.5);
	r.y1=(int)(y1+0.5);
	r.x2=(int)(x2+0.5);
	r.y2=(int)(y2+0.5);

	return(r);
}


// track image S into image I, with (0,0) at (x0,y0), range +-dx,+-dy
// si Imask!=null, alors il doit etre a 65535 pour qu'on considere un pixel
rect trackRect(imgu *I,imgu *S,int x0,int y0,int dx,int dy,imgu *Imask,int debug)
{
int delx,dely;
double cost,cost1,basecost;
rect r;
int nb,nbi,i;
int bestX,bestY;
double cmin;
	cmin=-1.0;
	bestX=bestY=0;
	if( Imask==NULL ) {
		nbi=S->xs*S->ys; // nb de pixels a considerer
	}else{
#ifdef DONT_USE_MASK
		for(i=0;i<Imask->xs*Imask->ys;i++) Imask->data[i]=65535;
#endif
		for(i=0,nbi=0;i<Imask->xs*Imask->ys;i++) if( Imask->data[i]==65535 ) nbi++;
	}
	if( debug==1 ) fprintf(stderr,"m=Partition[Delete[{\n");
	for(dely=-dy;dely<=dy;dely++)
	for(delx=-dx;delx<=dx;delx++) {
		//basecost=(delx*delx+dely*dely)*BASE_COST_MULT;
		cost=imguSSD(I,S,x0+delx,y0+dely,&nb,Imask);
		//cost1=imguSlowSSD(I,S,x0+delx,y0+dely,&nb,Imask);
		if( debug==1 ) fprintf(stderr,"%f,\n",cost/nbi);
		if( nb!=nbi ) continue; // partie de patch a l'exterieur!
		if( cost<0.0 ) continue;
		if( cost<cmin || cmin<0.0 ) { bestX=delx;bestY=dely;cmin=cost; }
	}
	if( debug==1 ) fprintf(stderr,"-1},-1],%d];\n",dx*2+1);
	r.x1=x0+bestX;
	r.y1=y0+bestY;
	r.x2=r.x1+S->xs-1;
	r.y2=r.y1+S->ys-1;
	//fprintf(stderr,"best at (%d,%d) displacement (c=%f)\n",bestX,bestY,cmin/nbi);
	if( debug>=0 ) fprintf(stderr,"{%d,%d,%f},\n",bestX,bestY,cmin/nbi);
	return(r);
}



void drawRectangle(imgu *I,char *T)
{
double x1,y1,x2,y2;
unsigned short color[4];
	if( T==NULL ) return;

	//fprintf(stderr,"drawing rectangle '%s'\n",T);

	sscanf(T," %lf %lf %lf %lf",&x1,&y1,&x2,&y2);

	color[0]=0;
	color[1]=0;
	color[2]=0;
	color[4]=65535;


	imguDrawLine(I,x1,y1,x2,y1,-1,color);
	imguDrawLine(I,x2,y1,x2,y2,-1,color);
	imguDrawLine(I,x2,y2,x1,y2,-1,color);
	imguDrawLine(I,x1,y2,x1,y1,-1,color);
}


imgu *avance(imgu *I,int n)
{
imgu *J;
int i;
	for(J=I,i=0;i<n && J!=NULL;i++,J=J->next) ;
	return(J);
}

int main(int argc,char *argv[])
{
char InName[100];
char OutName[100];
int from,to,step; // depart, arrivee (inclusif). from<0 -> debut, to<0 -> fin
int outputtype;
FILE *F,*G,*H,*M;
char *T;
int i;
int zcompress;
int draw; // draw the rectangle in the sequence???


	InName[0]=0;
	OutName[0]=0;
	from=0;
	to=-1;
	zcompress=0;
	draw=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-draw",argv[i])==0 ) {
			draw=1;
			continue;
		}
		if( strcmp("-i",argv[i])==0 && i+1<argc ) {
			strcpy(InName,argv[i+1]);
			i++;continue;
		}
		if( strcmp("-o",argv[i])==0 && i+1<argc ) {
			strcpy(OutName,argv[i+1]);
			i++;continue;
		}
		if( strcmp("-from",argv[i])==0 && i+1<argc ) {
			from=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp("-to",argv[i])==0 && i+1<argc ) {
			to=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp("-z",argv[i])==0 && i+1<argc ) {
			zcompress=atoi(argv[i+1]);
			i++;continue;
		}
	}

	if( from<0 || (to>=0 && to<from) ) { fprintf(stderr,"illegal from/to\n");exit(0); }

	if( InName[0] ) {
		F=fopen(InName,"r");
		if( F==NULL ) {
			fprintf(stderr,"Unable to open in '%s'\n",InName);
			exit(-1);
		}
		fprintf(stderr,"opened %s\n",InName);
	}else{
		F=stdin;
	}


	if( OutName[0] ) {
		G=fopen(OutName,"w");
		if( G==NULL ) {
			fprintf(stderr,"Unable to open out '%s'\n",OutName);
			exit(-1);
		}
		fprintf(stderr,"opened %s\n",OutName);
	}else{
		G=stdout;
	}

	H=fopen("patch.png","w");
	if( H==NULL ) {
		fprintf(stderr,"Unable to open out '%s'\n","patch.png");
		exit(-1);
	}
	M=fopen("mask.png","w");
	if( M==NULL ) {
		fprintf(stderr,"Unable to open out '%s'\n","mask.png");
		exit(-1);
	}


	char buf[100];
	int k,nbmatch;
	imgu *I1,*J1,*I2,*J2,*S1,*S2,*newS1,*newS2,*Imask,*Ithresh1,*Ithresh2;
	imgu *Sgray,*newSgray;
	I1=I2=NULL;
	rect r1,r2,nr1,nr2;
	int newrect1,newrect2;
	J1=NULL;
	S1=NULL; // subpatch containing the last seen rectangle (null si pas de rect)
	S2=NULL; // subpatch containing the last seen rectangle (null si pas de rect)
	for(i=0;;i+=2) {
		if( imguLoadFromFile(&I1,F,LOAD_16_BITS) ) break;
		if( imguLoadFromFile(&I2,F,LOAD_16_BITS) ) break;

		if( i<from ) continue;
		if( to>=0 && i>to ) break;

		fprintf(stderr,"images %d et %d...\n",i,i+1);

		newrect1=newrect2=0;

		// un rectangle defini? (image 1)
		if( (T=imguGetText(I1,"rectangle")) ) {
			if( S1!=NULL ) imguFree(&S1);
			r1=getRect(T);

			imguExtractRectangle(&S1,I1,r1.x1,r1.y1,r1.x2-r1.x1+1,r1.y2-r1.y1+1);
			fprintf(stderr,"Found rectangle\n");
			/**
			sprintf(buf,"patch1_%03di.png",i);
			imguSave(S1,buf,1);
			**/
			//imguSaveToFile(S1,H,1);
			newrect1=1; // rectangle defini!
		}
		if( (T=imguGetText(I2,"rectangle")) ) {
			if( S2!=NULL ) imguFree(&S2);
			r2=getRect(T);
			imguExtractRectangle(&S2,I2,r2.x1,r2.y1,r2.x2-r2.x1+1,r2.y2-r2.y1+1);
			fprintf(stderr,"Found rectangle\n");
			/**
			sprintf(buf,"patch2_%03di.png",i);
			imguSave(S2,buf,1);
			**/
			//imguSaveToFile(S2,H,1);
			newrect2=1; // rectangle defini!
		}


		if( newrect1==0 && newrect2==0 && S1!=NULL && S2!=NULL ) {
			// extract la nouvelle patch et le nouveau rect
			imguExtractRectangle(&Imask,I1,r1.x1,r1.y1,r1.x2-r1.x1+1,r1.y2-r1.y1+1);
			imguAbsDiff(Imask,S1); // trouve un masque des pixels sans mouvement
			Ithresh1=imguThresh(Imask,4000,57344);
			imguFree(&Imask);

			imguExtractRectangle(&Imask,I2,r2.x1,r2.y1,r2.x2-r2.x1+1,r2.y2-r2.y1+1);
			imguAbsDiff(Imask,S2); // trouve un masque des pixels sans mouvement
			Ithresh2=imguThresh(Imask,4000,57344);
			imguFree(&Imask);


			// current patch
			nr1=trackRect(I1,S1,r1.x1,r1.y1,10,10,Ithresh1,-1);
			sprintf(buf,"%d %d %d %d",nr1.x1,nr1.y1,nr1.x2,nr1.y2);
			k=imguReplaceAddText(I1,"rectangle",buf);
			if( k ) { fprintf(stderr,"TXT err\n"); }
			imguExtractRectangle(&newS1,I1,nr1.x1,nr1.y1,nr1.x2-nr1.x1+1,nr1.y2-nr1.y1+1);
			if( draw ) drawRectangle(I1,buf);

			nr2=trackRect(I2,S2,r2.x1,r2.y1,10,10,Ithresh2,-1);
			sprintf(buf,"%d %d %d %d",nr2.x1,nr2.y1,nr2.x2,nr2.y2);
			k=imguReplaceAddText(I2,"rectangle",buf);
			if( k ) { fprintf(stderr,"TXT err\n"); }
			imguExtractRectangle(&newS2,I2,nr2.x1,nr2.y1,nr2.x2-nr2.x1+1,nr2.y2-nr2.y1+1);
			if( draw ) drawRectangle(I2,buf);


			imguSaveToFile(Ithresh1,M,1,SAVE_16_BITS);
			imguSaveToFile(Ithresh2,M,1,SAVE_16_BITS);
			imguFree(&Ithresh1);
			imguFree(&Ithresh2);
			/* ajoute au debut de la liste des Mask */
			//Ithresh->next=Mask;
			//Mask=Ithresh;
			/**
			sprintf(buf,"patch%03d.png",i);
			imguSave(newS,buf,1);
			**/
			//imguSaveToFile(newS1,H,1);

			// current patch is updated
			r1=nr1; imguFree(&S1); S1=newS1;
			r2=nr2; imguFree(&S2); S2=newS2;
			// ajoute cette patch au debut de la liste des patches
			//S->next=Patch;
			//if( Patch==NULL ) Patch=S;
			/*free(pm);*/
		}
		if( S1 && S2 ) {
			fprintf(stderr,"saving patch\n");
			imguSaveToFile(S1,H,1,SAVE_16_BITS);
			imguSaveToFile(S2,H,1,SAVE_16_BITS);
		}

		if( newrect1 != newrect2 ) { fprintf(stderr,"yo?\n"); }
		if( S1==NULL || S2==NULL ) { fprintf(stderr,"no patch\n"); }

		imguSaveToFile(I1,G,zcompress,SAVE_16_BITS);
		imguSaveToFile(I2,G,zcompress,SAVE_16_BITS);

		if( to>=0 && i>=to ) break;
	}
	if( InName[0] ) fclose(F);
	if( OutName[0] ) fclose(G);
	fclose(H);
	fclose(M);

	//imguSavePNGMulti(Patch,"patch_all.png",1);
	//imguSavePNGMulti(Mask,"mask_all.png",1);

	//imguFree(Patch);
	//imguFree(Mask);

}





