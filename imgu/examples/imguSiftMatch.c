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

#include "imgu.h"

int main(int argc, char **argv)
{
  imgu *I1, *I2, *Iout;
  I1=I2=NULL;

  if (argc <3 || argc>4) {
    printf("usage : imsuSiftMatch i1 i2 [out]\n");
    exit(-1);
  }

  if (imguLoad(&I1,argv[1],LOAD_16_BITS)) {fprintf(stderr,"Error: no input image.\n");exit(-1);} 
  if (imguLoad(&I2,argv[2],LOAD_16_BITS)) {fprintf(stderr,"Error: no input image.\n");exit(-1);}

  if (argc==4) {
    Iout = imguCreate(I1->xs,I1->ys+I2->ys,I1->cs);
    pix_t *p,*q;
    int j = I1->xs*I1->ys*I1->cs;
    p = I1->data;
    q = Iout->data;
    while (j--) *q++ = *p++;
    j=I2->xs*I2->ys*I2->cs;
    p = I2->data;
    q = Iout->data+I1->xs*I1->ys*I1->cs;
    while (j--) *q++ = *p++;
  } 

  imguSift(I1);
  imguSift(I2);

  match *m;
  imguMatchFromSift(I1,I2,&m,-1);

  match *mi = m;
  pix_t col[] = {65535};
 
  while (mi->x1>0) {
    printf("(%f,%f) --> (%f,%f)\n",mi->x1,mi->y1,mi->x2,mi->y2);
    if (argc==4) {
      imguDrawLine(Iout,mi->x1,mi->y1,mi->x2,mi->y2+I1->ys,0,col);
    }
    mi++;
  }

  imguFree(&I1);
  imguFree(&I2);

  if (argc==4) {
    imguSave(Iout, argv[3], NO_COMPRESSION, LOAD_16_BITS);
    imguFree(&Iout);
  }
  return 0;
}

