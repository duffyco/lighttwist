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

// test text management

#include <stdlib.h>
#include <stdio.h>
#include <imgu.h>

#define OUTNAME	"/tmp/img.png"

int texte()
{
imgu *I;
int i,k;
char key[100],txt[100];
char *g;

    I=NULL;

	printf("---- texte ----\n");
    imguAllocate(&I,256,256,1);

	// rampe
	for(i=0;i<I->xs*I->ys*I->cs;i++) I->data[i]=i%65536;

	// Ajoute des textes
	int n=100;
	for(i=0;i<n;i++) {
		sprintf(key,"key%04d",i);
		sprintf(txt,"key%04d",i);
		k=imguAddText(I,key,txt);
		if( k ) printf("->> returned %d\n",k);
	}
	imguDump(I);

	// save
	k=imguSave(I,OUTNAME,1,SAVE_16_BITS);
	if( k ) return(-1);

	// reload
	imguLoad(&I,OUTNAME,LOAD_16_BITS);
	if( I==NULL ) { unlink(OUTNAME);return(-2); }
	imguDump(I);

	// check!
	k=0;
	for(i=0;i<n;i++) {
		sprintf(key,"key%04d",i);
		sprintf(txt,"key%04d",i);
		g=imguGetText(I,key);
		if( g==NULL ||
			strcmp(g,txt)!=0 ) {
			printf("problem avec '%s' %d\n",txt,(g==NULL));
			k++;
		}
	}
	if( k ) { unlink(OUTNAME);return(-3); }

	imguFree(&I);
	unlink(OUTNAME);
	return(0);
}

int main(int argc,char *argv[])
{
int k;
	k=texte();
	if( k ) { printf("texte returning %d\n",k);return(k); }
	return(0);
}


