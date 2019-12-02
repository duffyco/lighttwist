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

// test big-endian vs little-endian
//  cree une image rampe
//  sauvegarde l'image rampe
//  relit l'image rampe
//  compare a l'original

#include <stdlib.h>
#include <stdio.h>
#include <imgu.h>

#define OUTNAME	"/tmp/img.png"

int endian()
{
imgu *I,*J;
int i,k;

    I=NULL;
    J=NULL;

	printf("-- endian : init --\n");
	imguAllocate(&I,256,256,1);
	imguDump(I);

	// rampe
	for(i=0;i<I->xs*I->ys*I->cs;i++) I->data[i]=i%65536;

	// save
	k=imguSave(I,OUTNAME,1,SAVE_16_BITS);
	if( k ) return(-1);

	// reload
	imguLoad(&J,OUTNAME,LOAD_16_BITS);
	if( J==NULL ) { unlink(OUTNAME);return(-2); }

	// check!
	k=0;
	for(i=0;i<I->xs*I->ys*I->cs;i++) {
		if( J->data[i] != I->data[i] && k<10) {
			printf("[%8d] : %5d -> %5d\n",i,I->data[i],J->data[i]);
			k++;
		}
	}
	if( k ) { unlink(OUTNAME);return(-3); }

	imguFree(&I);
	imguFree(&J);
	unlink(OUTNAME);
	return(0);
}

int main(int argc,char *argv[])
{
int k;
	k=endian();
	if( k ) { printf("endian returning %d\n",k);return(k); }
	return(0);
}


