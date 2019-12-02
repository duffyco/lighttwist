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
 *
 * \example
 *
 * @file
 * @brief Change the text inside a png
 *
 * This is the long description of pngtext
 *
 *
*/

/// pngtext
//
// in: une image, et un texte
// out: la meme image avec le texte
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <imgu.h>

char key[200];
char text[1000];

unsigned char fbuf[1024];

// copy le fichier F dans G, au complet
void finish_copying(FILE *F,FILE *G)
{
int k;
	fprintf(stderr,"finishing...\n");
	for(;;) {
		k=fread(fbuf,1,sizeof(fbuf),F);
		if( k==0 ) break;
		fwrite(fbuf,1,k,G);
	}
}

int main(int argc,char *argv[])
{
char InName[100];
char OutName[100];
int num;
FILE *F,*G;
int i,j;

	InName[0]=0;
	OutName[0]=0;
	num=0;
	key[0]=text[0]=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-h",argv[i])==0 ) {
			fprintf(stderr,"pngtext [-i {input image}] [-o {output image}] [-num {image number}] -key {key} -txt {text}\n");	
			fprintf(stderr,"   (num<0 -> all images)\n");	
			exit(0);
		}
		if( strcmp("-num",argv[i])==0 && i+1<argc ) {
			num=atoi(argv[i+1]); // image to hit, -1 == ALL
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
		if( strcmp("-key",argv[i])==0 && i+1<argc ) {
			strcpy(key,argv[i+1]);
			i++;continue;
		}
		if( strcmp("-txt",argv[i])==0 && i+1<argc ) {
			strcpy(text,argv[i+1]);
			i++;continue;
		}
	}

	
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
	int k;
	I=NULL;
	for(i=0;;i++) {
		if (imguLoadFromFile(&I,F,LOAD_16_BITS)) break; // try to reuse I (when non NULL)
		fprintf(stderr,"image %d...\n",i);

		// dump tous les textes
		for(j=0;j<I->ts;j++) {
			if( I->key[j]==NULL || I->txt[j]==NULL ) continue;
			fprintf(stderr,"img %5d : '%s' = '%s'\n",i,I->key[j],I->txt[j]);
		}

		if( i==num || num<0 ) {
			if( text[0] ) {
				fprintf(stderr,"setting text to key='%s' txt='%s'\n",key,text);
				k=imguReplaceAddText(I,key,text);
				if( k ) { fprintf(stderr,"TXT err\n");exit(-1); }
			}else{
				imguRemoveKey(I,key,1);
			}
		}

		imguSaveToFile(I,G,1,SAVE_16_BITS);

		// est-ce qu'on peut copier direct le reste de la sequence?
		if( num>=0 && i==num ) { finish_copying(F,G);break; }
	
	}
	if( InName[0] ) fclose(F);
	if( OutName[0] ) fclose(G);

    imguFree(&I);
}





