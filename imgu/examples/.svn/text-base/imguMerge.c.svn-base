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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <imgu.h>

int main(int argc,char *argv[])
{
    int i;
    char hFilename[255];
    char vFilename[255];
    char outFilename[255];

    imgu *Ih=NULL;
    imgu *Iv=NULL;
    imgu *Iout=NULL;
    imgu *Iout_cmap=NULL;

    hFilename[0]='\0';
    vFilename[0]='\0';
    outFilename[0]='\0';
	for(i=1;i<argc;i++) {
		if( strcmp("-h",argv[i])==0 && i+1<argc ) {
			strcpy(hFilename,argv[i+1]);
			i++;continue;
		}
		if( strcmp("-v",argv[i])==0 && i+1<argc ) {
			strcpy(vFilename,argv[i+1]);
			i++;continue;
		}
		if( strcmp("-o",argv[i])==0 && i+1<argc ) {
			strcpy(outFilename,argv[i+1]);
			i++;continue;
		}
	}

    imguLoad(&Ih,hFilename,LOAD_16_BITS);
    imguLoad(&Iv,vFilename,LOAD_16_BITS);

    imguConvertToComplexComponent(&Iout,Ih,0);
    imguConvertToComplexComponent(&Iout,Iv,1);
    imguConvertFromComplexUV(&Iout_cmap,Iout,COMPLEX_RESCALE);
    imguSave(Iout_cmap,outFilename,1,SAVE_16_BITS);

    imguFree(&Ih);
    imguFree(&Iv);
    imguFree(&Iout);
    imguFree(&Iout_cmap);

    return 0;
}

