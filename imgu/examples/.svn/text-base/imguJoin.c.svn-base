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


#include <imgu.h>

#include <string.h>

#define MAXI	100
int nbI;
imgu *It[MAXI];

int main( int argc, char **argv )
{
int i,j,k,ii;
char imageName[200];
imgu *I=NULL;
unsigned short min,max;
FILE *F; // use a file to catch MULTI png files
char outName[200];
int direction; // 0==x, 1==y

	direction=0;
	nbI=0;
	for(i=0;i<MAXI;i++) It[i]=NULL;

	imageName[0]=0;
	outName[0]=0;

	for(i=1;i<argc;i++) {
		if( strcmp(argv[i],"-o")==0 && i+1<argc ) {
			strcpy(outName,argv[i+1]);
			i++;continue;
		}else if( strcmp(argv[i],"-x")==0 ) {
			direction=0;
			continue;
		}else if( strcmp(argv[i],"-y")==0 ) {
			direction=1;
			continue;
		}
		printf("------------------------\n");
		printf("FILE %s\n",argv[i]);
		F=fopen(argv[i],"rb");
		if( F==NULL ) { printf("ERR: unable to open '%s'\n",argv[i]);
			 exit(0); }


		for(ii=0;;ii++) {
			if( nbI==MAXI ) { printf("too many images\n");exit(-1); }
			k=imguLoadFromFile(It+nbI,F,LOAD_16_BITS); // LOAD_AS_IS | MAKE_16_BITS
			if( k ) { printf("ERR: error while loading '%s' k=%d\n",argv[i],k);exit(-1); }

			if( ii>0 ) printf("------ SUBIMAGE %d ---\n",ii);

			I=It[nbI];
			nbI++;

			printf("SIZE %d x %d x %d\n",I->xs,I->ys,I->cs);

			min=max=I->data[0];
			for(j=0;j<I->xs*I->ys*I->cs;j++) {
				if( I->data[j]<min ) min=I->data[j];
				else if( I->data[j]>max ) max=I->data[j];
			}
			printf("MIN %d MAX %d\n",min,max);

			// print all keys... there should be a function for this...
			for(j=0;j<I->ts;j++) {
				if( I->key[j]==NULL ) continue;
				printf("KEY %s : %s\n",I->key[j],I->txt[j]);
			}
			int c=fgetc(F);
			if( c==EOF ) break;
			ungetc(c,F);
		}
		fclose(F);

	}

	printf("done loading images\n");

	if( outName[0]==0 ) { printf("no output (-o) specified\n");exit(0); }

	int xs,ys,cs;
	xs=ys=cs=0;

	for(i=0;i<nbI;i++) {
		if( direction==0 ) {
			xs+=It[i]->xs;
			if( It[i]->ys > ys ) ys=It[i]->ys;
		}else{
			if( It[i]->xs > xs ) xs=It[i]->xs;
			ys+=It[i]->ys;
		}
		if( It[i]->cs > cs ) cs=It[i]->cs;
	}
	printf("Final size is %d x %d x %d\n",xs,ys,cs);

	imgu *Iz=NULL;
	imguAllocate(&Iz,xs,ys,cs);

	int x,y,c,ox,oy;
	ox=0;oy=0;
	for(i=0;i<nbI;i++) {
		// copy at offset
		for(y=0;y<It[i]->ys;y++)
		for(x=0;x<It[i]->xs;x++)
		for(c=0;c<Iz->cs;c++) {
			PIXEL(Iz,x+ox,y+oy,c)=PIXEL(It[i],x,y,(c%It[i]->cs));
		}
		if( direction==0 )	ox+=It[i]->xs;
		else			oy+=It[i]->ys;
	}

	imguSave(Iz,outName,1,SAVE_AS_IS);

	exit(0);
}

