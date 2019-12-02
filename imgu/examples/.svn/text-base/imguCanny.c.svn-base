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
 * This example code detects edges using the Canny algorithm.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <imgu.h>
#include <math.h>

#define STR_LEN 255

int main(int argc,char *argv[])
{
  int i;
  char inFilename[STR_LEN];
  char outFilename[STR_LEN];
  imgu *I;

  I=NULL;

  inFilename[0]='\0';
  outFilename[0]='\0';

  for(i=1;i<argc;i++) {
      if( strcmp("-i",argv[i])==0 ) {
          strcpy(inFilename,argv[i+1]);
          i++;
          continue;
      }
      if( strcmp("-o",argv[i])==0 ) {
          strcpy(outFilename,argv[i+1]);
          i++;
          continue;
      }
  }

  if (inFilename[0]=='\0' || outFilename[0]=='\0') {fprintf(stderr,"Usage: imguFit -i <input image> -o <output image>\n");exit(0);}

  if (imguLoad(&I,inFilename,LOAD_16_BITS)) {fprintf(stderr,"Error loading input image '%s'\n",inFilename);exit(0);}

  //imguCannyWithEdgeTracking(&I,I,1.5,20.0*257,5.0*257);
  imguLineThinner(&I,I,2.5,60*257);

  imguSave(I,outFilename,FAST_COMPRESSION,SAVE_16_BITS);

  imguFree(&I);

  return 0;	
}


