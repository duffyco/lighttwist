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
 * This is example code for that tests rounding errors when converting from complex to unsigned short.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <imgu.h>
#include <math.h>

double MeanAbsoluteDifference(imgu *I1,imgu *I2)
{
  int i;
  double diff;
  int size;

  if (I1==NULL || I2==NULL) return -1.0;
  if (I1->complex==NULL || I2->complex==NULL) return -1.0;
  if (I1->xs!=I2->xs) return -1.0;
  if (I1->ys!=I2->ys) return -1.0;
  if (I1->cs!=I2->cs) return -1.0;
  size=I1->xs*I1->ys*I1->cs;
  if (size==0) return -1.0;

  diff=0.0;
  for (i=0;i<size;i++)
  {
    diff+=fabs(I1->complex[i][0]-I2->complex[i][0]);
    diff+=fabs(I1->complex[i][1]-I2->complex[i][1]);
  }

  diff/=(2*size);

  return diff;
}

int testConversion(double range)
{
  imgu *I,*J,*K;
  int i;
  int xs,ys,cs;
  int size;
  double diff;

  xs=512;
  ys=512;
  cs=1;

  //test data will range between -range and range
  fprintf(stderr,"The range of data in both real and imaginary parts is from %f to %f\n",-range,range);

  I=NULL;
  J=NULL;
  K=NULL;

  imguAllocateComplex(&I,xs,ys,cs);
  size=xs*ys*cs;

  //put random complex numbers
  for (i=0;i<size;i++)
  {
    I->complex[i][0]=matRandNumber()*2*range-range;
    I->complex[i][1]=matRandNumber()*2*range-range;
  }

  //convert in color format
  imguConvertFromComplexFlow(&J,I);
  imguConvertToComplexFlow(&K,J);    
  diff=MeanAbsoluteDifference(I,K);
  fprintf(stderr,"Flow encoding error:\t\t\t%f\n",diff);

  //convert in UV->RGB format
  imguConvertFromComplexUV(&J,I,COMPLEX_RESCALE);
  imguConvertToComplexUV(&K,J);    
  diff=MeanAbsoluteDifference(I,K);
  fprintf(stderr,"UV encoding error:\t\t\t%f\n",diff);

  //convert each complex part in separate images
  imguConvertFromComplexComponent(&J,I,0,COMPLEX_RESCALE);
  imguConvertFromComplexComponent(&K,I,1,COMPLEX_RESCALE);
  imguConvertToComplexComponent(&J,J,0);
  imguConvertToComplexComponent(&J,K,1);
  diff=MeanAbsoluteDifference(I,J);
  fprintf(stderr,"Separate channel encoding error:\t%f\n",diff);

  imguFree(&I);
  imguFree(&J);
  imguFree(&K);

  return 0;	
}

int main(int argc,char *argv[])
{

  testConversion(1.0);
  testConversion(32.0);
  testConversion(1024.0);
  testConversion(16000.0);
  testConversion(32000.0);

  return 0;	
}


