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


int main(int argc,char *argv[])
{
char InName[100];
char OutName[100];
int from,to,step; // depart, arrivee (inclusif). from<0 -> debut, to<0 -> fin
int outputtype;
FILE *F,*G;
char *T;
int i;
int zcompress;

	InName[0]=0;
	OutName[0]=0;
	from=0;
	to=-1;
	zcompress=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-z",argv[i])==0 && i+1<argc ) {
			zcompress=atoi(argv[i+1]);
			i++;continue;
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

	imgu *I;
	I=NULL;
	for(i=0;;i++) {
		if( imguLoadFromFile(&I,F,LOAD_16_BITS) ) break;

		if( i>=from && (to<0 || i<=to) ) {

			//fprintf(stderr,"image %d...\n",i);

			if( (T=imguGetText(I,"rectangle")) ) drawRectangle(I,T);

			imguSaveToFile(I,G,zcompress,SAVE_16_BITS);
		}

		if( to>=0 && i>=to ) break;
	}
	if( InName[0] ) fclose(F);
	if( OutName[0] ) fclose(G);

    imguFree(&I);
}





