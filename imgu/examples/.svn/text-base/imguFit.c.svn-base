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
 * This example code finds a curve fit for some feature points and saves the curve in the input image.
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
  int i,nb_points;
  int degree;
  char inFilename[STR_LEN];
  char outFilename[STR_LEN];
  double *x,*y,*coeffs;
  matrix *est_coeffs;
  imgu *I;
  vector3 red;

  I=NULL;
  est_coeffs=NULL;
  red[0]=IMGU_MAXVAL;
  red[1]=0;
  red[2]=0;

  nb_points=32;
  degree=2;

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

  x=(double *)(malloc(sizeof(double)*nb_points));
  y=(double *)(malloc(sizeof(double)*nb_points));
  coeffs=(double *)(malloc(sizeof(double)*(degree+1)));
  matAllocate(&est_coeffs,degree+1,1);

  //make some test curve coefficients
  for (i=0;i<=degree;i++)
  {
    coeffs[i]=(double)(i);
  }

  for (i=0;i<nb_points;i++)
  {
    x[i]=(double)(i)*I->xs/nb_points;
  }
  matEval(x,y,nb_points,coeffs,degree);
  matFit(x,y,nb_points,est_coeffs->values,degree);

  fprintf(stdout,"Estimated coefficients are: \t");
  for (i=0;i<=degree;i++)
  {
    fprintf(stdout,"%f\t",est_coeffs->values[i]);
  }
  fprintf(stdout,"\n");

  matEval(x,y,nb_points,est_coeffs->values,degree);
  for (i=0;i<nb_points;i++)
  {
    imguDrawLineLength(I,x[i],y[i],1.0,0.0,red,0.02);
    imguDrawLineLength(I,x[i],y[i],0.0,1.0,red,0.02);
  }

  imguSaveMatrix(est_coeffs,I,"CURVE_FIT");
  imguRemoveAlphaLayer(&I,I);

  imguSave(I,outFilename,FAST_COMPRESSION,SAVE_16_BITS);

  free(x);
  free(y);
  free(coeffs);
  matFree(&est_coeffs);

  return 0;	
}


