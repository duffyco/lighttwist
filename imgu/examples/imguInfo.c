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

int main( int argc, char **argv )
{
int i,j,k,ii;
char imageName[200];
imgu *I=NULL;
unsigned short min,max;
FILE *F; // use a file to catch MULTI png files
int math=0;


	imageName[0]=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-math",argv[i])==0 ) { math=1;continue; }
		if( !math ) printf("------------------------\n");
		if( !math ) printf("FILE %s\n",argv[i]);
		F=fopen(argv[i],"rb");
		if( F==NULL ) { printf("ERR: unable to open '%s'\n",argv[i]); continue; }

		for(ii=0;;ii++) {

			//k=imguLoad(&I,argv[i],LOAD_16_BITS); // LOAD_AS_IS | MAKE_16_BITS
			k=imguLoadFromFile(&I,F,LOAD_16_BITS); // LOAD_AS_IS | MAKE_16_BITS
			if( k ) { printf("ERR: error while loading '%s'\n",argv[i]);break; }

			if( ii>0 ) printf("------ SUBIMAGE %d ---\n",ii);

			if( !math ) printf("SIZE %d x %d x %d\n",I->xs,I->ys,I->cs);

			min=max=I->data[0];
			for(j=0;j<I->xs*I->ys*I->cs;j++) {
				if( I->data[j]<min ) min=I->data[j];
				else if( I->data[j]>max ) max=I->data[j];
			}
			if( !math ) printf("MIN %d MAX %d\n",min,max);

			// print all keys... there should be a function for this...
			for(j=0;j<I->ts;j++) {
				if( I->key[j]==NULL ) continue;
				if( !math ) printf("KEY %s : %s\n",I->key[j],I->txt[j]);
				else printf("{ \"KEY\", \"%s\", \"%s\"}\n",I->key[j],I->txt[j]);
			}
			int c=fgetc(F);
			if( c==EOF ) break;
			ungetc(c,F);
		}
		fclose(F);
	}
	imguFree(&I);

    return 0;
}

