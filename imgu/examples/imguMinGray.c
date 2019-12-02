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

#include <imgu/imgu.h>

#include <imgu/imgu.h>

#define STR_LEN 255

int main(int argc, char **argv)
{
  int i,index,bit;
  char inFilename[STR_LEN];
  char filename[STR_LEN];
  imgu *I,*J;
  matrix *codes;

  I=NULL;
  J=NULL;
  codes=NULL;

  matAllocate(&codes,1024,1);
  for (i=0;i<10;i++)
  {
    //loading 10 patterns of gupta graycodes, with stripes between 8 and 32 pixels wide
    sprintf(inFilename,"%02d.png",i+1);
    imguLoad(&J,inFilename,LOAD_16_BITS);
    imguScale(&J,J,1024.0/J->xs,1024.0/J->ys);
    imguAddLastMulti(&I,J);
    J=NULL;
    /*//testing...
    imguAllocate(&J,1024,768,1);
    stlPatternXOR(J,i,1);
    //stlPattern(J,i);
    sprintf(filename,"gc_%02d.png",i);
    imguSave(J,filename,1,SAVE_16_BITS);*/
  }
  if (imguCount(I)==0){ fprintf(stderr,"Error loading patterns.\n");exit(-1);}

  for (i=0;i<I->xs;i++)
  {
    index=0;
    J=I;
    bit=0;
    while(J!=NULL)
    {
      if (J->data[i]>IMGU_MAXVAL/2) index|=(1<<bit);
      J=J->next;
      bit++;
    }
    if (index>=codes->cs) fprintf(stderr,"Warning: index '%d' out of range.\n",index);
    else codes->values[index]=(double)(i);
  }
  //matPrint(codes);

  imguSaveMatrix(codes,I,"MinSWGray");
  imguSaveMulti(I,"MinSWGray.png",1,SAVE_8_BITS_HIGH);
  imguFreeMulti(&I);
  matFree(&codes);

  return 0;
}

