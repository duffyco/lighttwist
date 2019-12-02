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
 * @file
 *
 * This is example code for imguInterpolateClosest(), imguInterpolateBilinear(),
 * and imguInterpolateBicubic().
 *
 * Rien de special.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <imgu.h>
#include <math.h>



void test_interpolate_bilinear(const char *path)
{
imgu *I,*J;
char name[100];
int x,y,c;

    if (path==NULL) return;

    I=NULL;
    J=NULL;

	strcpy(name,path);strcat(name,"/info_rgba.png");
	printf("Loading >%s<\n",name);
	//if (imguLoad(&I,name,LOAD_AS_IS)) { printf("Unable to load image\n");exit(0); }
	if (imguLoad(&I,name,LOAD_16_BITS)) { printf("Unable to load image\n");exit(0); }

	imguAllocate(&J,I->xs,I->ys,I->cs);

	double ang=20*M_PI/180.0;
	double ca=cos(ang);
	double sa=sin(ang);
	double cx=I->xs/2;
	double cy=I->ys/2;
	double val[4]; // doit contenir au moins I->cs valeurs
	double xx,yy;


	/// Closest point
	for(y=0;y<I->ys;y++)
	for(x=0;x<I->xs;x++) {
		xx=ca*(x-cx)+sa*(y-cy)  +cx;
		yy=-sa*(x-cx)+ca*(y-cy) +cy;
		imguInterpolateClosest(I,xx,yy,val);
		for(c=0;c<I->cs;c++) J->data[(y*J->xs+x)*J->cs+c]=val[c];
	}
	strcpy(name,"rotated_closest.png");
	printf("Saving >%s<\n",name);
	imguSave(J,name,1,SAVE_16_BITS);

	/// bilinear
	for(y=0;y<I->ys;y++)
	for(x=0;x<I->xs;x++) {
		xx=ca*(x-cx)+sa*(y-cy)  +cx;
		yy=-sa*(x-cx)+ca*(y-cy) +cy;
		imguInterpolateBilinear(I,xx,yy,val);
		for(c=0;c<I->cs;c++) J->data[(y*J->xs+x)*J->cs+c]=val[c];
	}
	strcpy(name,"rotated_bilinear.png");
	printf("Saving >%s<\n",name);
	imguSave(J,name,1,SAVE_16_BITS);

	/// bicubic
	for(y=0;y<I->ys;y++)
	for(x=0;x<I->xs;x++) {
		xx=ca*(x-cx)+sa*(y-cy)  +cx;
		yy=-sa*(x-cx)+ca*(y-cy) +cy;
		imguInterpolateBicubic(I,xx,yy,val);
		for(c=0;c<I->cs;c++) J->data[(y*J->xs+x)*J->cs+c]=val[c];
	}
	strcpy(name,"rotated_bicubic.png");
	printf("Saving >%s<\n",name);
	imguSave(J,name,1,SAVE_16_BITS);


	imguFree(&I);
	imguFree(&J);
}


int main(int argc,char *argv[])
{
	test_interpolate_bilinear("..//data/");

    return 0;	
}


