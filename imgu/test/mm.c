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

#include <stdio.h>
#include <math.h>

#include <matrixmath.h>

int main(int argc,char *argv[])
{
int i,k;
matrix4 a,b,c;
	printf("---- test matrixmath ----\n");

	mat4Identity(a);


	for(i=0;i<16;i++) a[i]=i+rand()%20;
	mat4Print(a);

	k=mat4Inverse(a,b);
	if( k==0 ) mat4Print(b);

	k=mat4Inverse(b,c);
	if( k==0 ) mat4Print(c);

	for(i=0;i<16;i++) a[i]=random();
	mat4Print(a);

	char buf[500];
	if( mat4Printf(a,buf,500) ) printf("ERR prinf\n");
	printf("string is >%s<\n",buf);

	k=mat4Scanf(buf,b);
	printf("got k=%d\n",k);

	mat4Print(b);

	return(0);
}

