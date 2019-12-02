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

#define INNAME	"/usr/local/share/imgu/Azureus.png"


void scale(imgu *I,char *outname,double sx,double sy)
{
imgu *J=NULL;
	if( imguScale(&J,I,sx,sy) ) { printf("ERR scale '%s'\n",outname);exit(-1); }

	imguSave(J,outname,1,SAVE_16_BITS);
	imguFree(&J);
}


void autoscale(imgu *I,char *outname,double sx,double sy)
{
imgu *J=NULL;

	imguCopy(&J,I);

	if( imguScale(&J,J,sx,sy) ) { printf("ERR scale '%s'\n",outname);exit(-1); }

	imguSave(J,outname,1,SAVE_16_BITS);
	imguFree(&J);
}

int compare(char *N1,char *N2,char *Nout)
{
imgu *I=NULL,*J=NULL,*K=NULL;

	printf("compare %s .. %s\n",N1,N2);

	if( imguLoad(&I,N1,LOAD_AS_IS) ) { printf("Unable to load '%s'\n",N1);return(-1); }
	if( imguLoad(&J,N2,LOAD_AS_IS) ) { printf("Unable to load '%s'\n",N2);return(-1); }

	// compare
	if( I->xs!=J->xs || I->ys!=J->ys || I->cs!=J->cs ) {
		printf("size mismatch!\n");return(-1);
	}

	K=imguCreate(I->xs,I->ys,I->cs);

	int i;
	int k=0;
	int v;
	for(i=I->xs*I->ys*I->cs-1;i>=0;i--) {
		v=I->data[i]-J->data[i];if( v<0 ) v= -v;
		K->data[i]=v;
		if( I->data[i]!=J->data[i] ) k++;
	}

	imguSave(K,Nout,1,SAVE_16_BITS);

	if( k ) { printf("mismatch (%d pixels)\n",k);return(-1); }

	imguFree(&I);
	imguFree(&J);
	imguFree(&K);
	return(0);
}


int main(int argc,char *argv[])
{
int k;
imgu *I;
  
    I=NULL;

	imguSetInternalFormat(Y_ORIGIN_IS_AT_TOP);
	imguSetExternalFormat(Y_ORIGIN_IS_AT_TOP);

	if( imguLoad(&I,INNAME,MAKE_16_BITS) ) { printf("Unable to load '%s'\n",INNAME);exit(-1); }

	//imguConvert8bitTo16bit(&I,I);

	imguSave(I,"/tmp/ref0.png",1,SAVE_16_BITS);

	autoscale(I,"/tmp/oot1.png",1.0,1.0);
	autoscale(I,"/tmp/oot2.png",0.5,0.5);
	autoscale(I,"/tmp/oot3.png",8.0,8.0);
	autoscale(I,"/tmp/oot4.png",0.25,4.0);
	autoscale(I,"/tmp/oot5.png",4.0,0.25);

	scale(I,"/tmp/out1.png",1.0,1.0);
	scale(I,"/tmp/out2.png",0.5,0.5);
	scale(I,"/tmp/out3.png",8.0,8.0);
	scale(I,"/tmp/out4.png",0.25,4.0);
	scale(I,"/tmp/out5.png",4.0,0.25);

//	compare("/tmp/ref0.png","/tmp/oot1.png","/tmp/refoot1.png");
//	compare("/tmp/ref0.png","/tmp/out1.png","/tmp/refout1.png");

	k=0;
	k+=compare("/tmp/oot1.png","/tmp/out1.png","/tmp/dif1.png");
	k+=compare("/tmp/oot2.png","/tmp/out2.png","/tmp/dif2.png");
	k+=compare("/tmp/oot3.png","/tmp/out3.png","/tmp/dif3.png");
	k+=compare("/tmp/oot4.png","/tmp/out4.png","/tmp/dif4.png");
	k+=compare("/tmp/oot5.png","/tmp/out5.png","/tmp/dif5.png");

	imguFree(&I);
	if( k ) exit(1);
	exit(0);
}


