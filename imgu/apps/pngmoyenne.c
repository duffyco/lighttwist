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

/// pngmoyenne
//
// calcule l'image moyenne
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <imgu/imgu.h>


int main(int argc,char *argv[])
{
char InName[100];
char OutName[100];
int from,to; // depart, arrivee (inclusif). from<0 -> debut, to<0 -> fin
FILE *F,*G;
int i,j;
int zcompress;
int is8bits;

	InName[0]=0;
	OutName[0]=0;
	from=0;
	to=-1;
	zcompress=1;
	is8bits=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-8",argv[i])==0 && i+1<argc ) {
			is8bits=1;
			continue;
		}
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

	unsigned int *Acc; /* [xs*ys] */
	int sz;

	Acc=NULL;
	imgu *I;
	I=NULL;
	for(i=0;;i++) {
		if (imguLoadFromFile(&I,F,LOAD_16_BITS)) break; // try to reuse I (when non NULL)

		if( i<from ) continue;

		if( Acc==NULL ) {
			sz=I->xs*I->ys*I->cs;
			fprintf(stderr,"sz %d\n",sz);
			Acc=(unsigned int *)malloc(sz*sizeof(unsigned int));
			for(j=0;j<sz;j++) Acc[j]=0;
		}
		if( I->xs*I->ys*I->cs!=sz ) { fprintf(stderr,"size mismatch %d,%d,%d %d %d\n",I->xs,I->ys,I->cs,I->xs*I->ys*I->cs,sz);exit(0); }

		for(j=0;j<sz;j++) Acc[j]+=I->data[j];
		
		if( to>=0 && i>=to ) break;
	}

	fprintf(stderr,"summed %d images\n",i);

	for(j=0;j<sz;j++) I->data[j]=(Acc[j]*2+i)/(i*2);

	imguSaveToFile(I,G,zcompress,is8bits?SAVE_8_BITS_HIGH:SAVE_16_BITS);

	if( InName[0] ) fclose(F);
	if( OutName[0] ) fclose(G);

    imguFree(&I);

    return 0;
}





