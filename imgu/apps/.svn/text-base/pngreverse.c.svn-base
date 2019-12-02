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

/// pngreverse a sequence
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <imgu.h>


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

	imgu *H; // liste des images
	imgu *I;
    I=NULL;
	H=NULL;
	for(i=0;;i++) {
		imguLoadFromFile(&I,F,LOAD_16_BITS);
		if( I==NULL ) break;
		//fprintf(stderr,"reading image %d I=0x%08lx\n",i,I);

		if( i>=from && (to<0 || i<=to) ) {
			// ajoute l'image au debut de la liste
			I->next=H;
			H=I;
            I=NULL; //reinitialiser I pour ne pas reutiliser sa structure (qui fait maintenant parti de H)
		}
        //else{
		//	imguFree(I);  //on a plus besoin de faire imguFree sur I, sa structure va etre reutiliser
		//}
		if( to>=0 && i>=to ) break;
	}
	if( InName[0] ) fclose(F);

	for(i=0,I=H;I!=NULL;i++,I=I->next) {
		//fprintf(stderr,"writing image %d  I=0x%08lx\n",i,I);
		imguSaveToFile(I,G,zcompress,SAVE_16_BITS);
	}
	if( OutName[0] ) fclose(G);

	imguFree(&H);
}





