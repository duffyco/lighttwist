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

/// pngselect
//
// liste des cles supportees:
//
// rectangle  :  x1 y1 x2 y2   (double)
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <imgu.h>


void drawRectangle(imgu *I,char *T)
{
double x1,y1,x2,y2;
unsigned short color[4];
	if( T==NULL ) return;

	fprintf(stderr,"drawing rectangle '%s'\n",T);

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


int imguMergeV(imgu **K,imgu *I1,imgu *I2)
{
int xs,ys,cs;
unsigned short v;
int x,y,c,x1,y1,c1,x2,y2,c2;

    if (K==NULL) return -1;
    if (I1==NULL) return -1;
    if (I2==NULL) return -1;

	xs=I1->xs;
	if( I2->xs > xs ) xs=I2->xs;

	ys=I1->ys+I2->ys;

	cs=I1->cs;
	if( I2->cs > cs ) cs=I2->cs;

	imguAllocate(K,xs,ys,cs);

	for(y=0;y<ys;y++)
	for(x=0;x<xs;x++)
	for(c=0;c<cs;c++) {
		if( y<I1->ys ) {
			// image 1
			x1=x; y1=y; c1=c;
			if( c1>=I1->cs ) c1=0;
			if( x1>=I1->xs ) v=0;
			else v=I1->data[(y1*I1->xs+x1)*I1->cs+c1];
		}else{
			// image 2
			x2=x; y2=y-I1->ys; c2=c;
			if( c2>=I2->cs ) c2=0;
			if( x2>=I2->xs ) v=0;
			else v=I2->data[(y2*I2->xs+x2)*I2->cs+c2];
		}
		(*K)->data[(y*(*K)->xs+x)*(*K)->cs+c]=v;
	}
	return 0;
}


int main(int argc,char *argv[])
{
char InName1[100];
char InName2[100];
char OutName[100];
int from,to,step; // depart, arrivee (inclusif). from<0 -> debut, to<0 -> fin
int outputtype;
FILE *F1,*F2,*G;
char *T;
int i;
int zcompress;
int sequence;

	InName1[0]=0;
	InName2[0]=0;
	OutName[0]=0;
	from=0;
	to=-1;
	zcompress=0;
	sequence=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-z",argv[i])==0 && i+1<argc ) {
			zcompress=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp("-i",argv[i])==0 && i+2<argc ) {
			strcpy(InName1,argv[i+1]);
			strcpy(InName2,argv[i+2]);
			i+=2;continue;
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
		if( strcmp("-sequence",argv[i])==0 ) {
			sequence=1;
			continue;
		}
	}

	if( InName1[0]==0 || InName2[0]==0 ) { fprintf(stderr,"need -i n1 n2\n");exit(0); }

	if( from<0 || (to>=0 && to<from) ) { fprintf(stderr,"illegal from/to\n");exit(0); }

	F1=fopen(InName1,"r");
	if( F1==NULL ) { fprintf(stderr,"Unable to open in '%s'\n",InName1); exit(-1); }
	fprintf(stderr,"opened %s\n",InName1);

	F2=fopen(InName2,"r");
	if( F2==NULL ) { fprintf(stderr,"Unable to open in '%s'\n",InName2); exit(-1); }
	fprintf(stderr,"opened %s\n",InName2);

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

	imgu *I1;
	imgu *I2;
	imgu *K;
	I1=I2=NULL;
    K=NULL;
	for(i=0;;i++) {
		if (imguLoadFromFile(&I1,F1,LOAD_16_BITS)) break; // try to reuse I (when non NULL)

		if (imguLoadFromFile(&I2,F2,LOAD_16_BITS)) break; // try to reuse I (when non NULL)

		fprintf(stderr,"image %d...\n",i);

		if( sequence ) {
			imguSaveToFile(I1,G,zcompress,SAVE_16_BITS);
			imguSaveToFile(I2,G,zcompress,SAVE_16_BITS);
		}else{
			imguMergeV(&K,I1,I2);
			imguSaveToFile(K,G,zcompress,SAVE_16_BITS);
		}


	}
	fclose(F1);
	fclose(F2);
	if( OutName[0] ) fclose(G);

    imguFree(&K);
    imguFree(&I1);
    imguFree(&I2);
}





