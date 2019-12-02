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

/**
 * Select a sequence of png images
 *
 * @file
 * @ingroup apps
 *
 * in: une image, et un intervalle: from-to, step
 * cas special: to=-1 : jusqu'a la fin
 *
 * (rien) -> 0,1,2,...
 * -from 5 -> 5,6,7,...
 * -from 5 -to 8 -> 5,6,7,8
 * -from 5 -to 11 -step 2 -> 5,7,9,11
 *
 * -head 10 -> 0,1,2,...9 (les 10 permieres images)
 * -single 5 -> 5 (une seule image)
 *
 * -o -> output all in a single png
 * -oo -> output all in separate file, with index set as the image number
 * -ooo -> output all in separate files, index from 0 step 1
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <imgu.h>


#define SINGLE_MPNG		0
#define MULTIPLE_PNG_ORIGINAL	1
#define MULTIPLE_PNG_RENUMBER   2


char key[200];
char text[1000];

int main(int argc,char *argv[])
{
char InName[100];
char OutName[100];
int from,to,step; // depart, arrivee (inclusif). from<0 -> debut, to<0 -> fin
int outputtype;
FILE *F,*G;
int i;
int raw;
int rgb;

	InName[0]=0;
	OutName[0]=0;
	from=0;to=-1;step=1;
	outputtype=SINGLE_MPNG;
	raw=0;
	rgb=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-rgb",argv[i])==0 ) {
			rgb=1;
			continue;
		}
		if( strcmp("-raw",argv[i])==0 ) {
			raw=1;
			continue;
		}
		if( strcmp("-head",argv[i])==0 && i+1<argc ) {
			from=0;
			to=atoi(argv[i+1])-1;
			step=1;
			i++;continue;
		}
		if( strcmp("-single",argv[i])==0 && i+1<argc ) {
			from=to=atoi(argv[i+1]);
			step=1;
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
		if( strcmp("-step",argv[i])==0 && i+1<argc ) {
			step=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp("-i",argv[i])==0 && i+1<argc ) {
			strcpy(InName,argv[i+1]);
			i++;continue;
		}
		if( strcmp("-o",argv[i])==0 && i+1<argc ) {
			strcpy(OutName,argv[i+1]);
			outputtype=SINGLE_MPNG;
			i++;continue;
		}
		if( strcmp("-oo",argv[i])==0 && i+1<argc ) {
			strcpy(OutName,argv[i+1]);
			outputtype=MULTIPLE_PNG_ORIGINAL;
			i++;continue;
		}
		if( strcmp("-ooo",argv[i])==0 && i+1<argc ) {
			strcpy(OutName,argv[i+1]);
			outputtype=MULTIPLE_PNG_RENUMBER;
			i++;continue;
		}
	}

	if( from<0 || step<=0 ) { fprintf(stderr,"illegal from/step\n");exit(0); }

	
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


	if( outputtype==SINGLE_MPNG ) {
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
	}else{
		G=NULL;
	}

	imgu *I,*Icopy,*K;
	int k,j;
	I=NULL;
	Icopy=NULL;
	K=NULL;
	char buf[100];
	j=0; // inc si MULTIPLE_PNG_RENUMBER
	for(i=0;;i++) {
		imguLoadFromFile(&I,F,LOAD_16_BITS); // try to reuse I (when non NULL)

		if( rgb && I->cs!=3 )
        {
            imguCopy(&Icopy,I);
            imguConvertToRGB(&I,Icopy);
        }

		if( raw && i==0 ) {
			fprintf(stderr,"-rawvideo w=%d:h=%d:fps=29.97:format=rgb24\n",I->xs,I->ys);
		}

		if( i>=from && (to<0 || i<=to) && (i-from)%step==0 ) {
			fprintf(stderr,"image %d...\n",i);

			if( outputtype==SINGLE_MPNG ) {
				if( raw ) imguSaveRAW8ToFile(I,G,SAVE_8_BITS_HIGH);
				else	  imguSaveToFile(I,G,1,SAVE_16_BITS);
			}else{
				if( outputtype==MULTIPLE_PNG_ORIGINAL ) j=i;
				sprintf(buf,OutName,j);
				k=imguSave(I,buf,1,SAVE_16_BITS);
				if( k ) { fprintf(stderr,"unable to write %s\n",buf); }
			}
			j++;
		}

		// arrete des que possible
		if( to>=0 && i>to ) break;
	}
	if( InName[0] ) fclose(F);
	if( OutName[0] && outputtype==SINGLE_MPNG ) fclose(G);

}





