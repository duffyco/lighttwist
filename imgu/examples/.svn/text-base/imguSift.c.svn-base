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
  int i;
  char *sift_header;
  int nb_kpoints,descr_size;
  char buf[100];
  imgu* I;

  I=NULL;

  if (imguLoad(&I,"/home/roys/src/siftpp/scene.png",LOAD_16_BITS)) {fprintf(stderr,"Error: no input image.\n");exit(-1);} 

  imguSift(I);

  printf("%s\n",imguGetText(I,"sift"));
  sift_header=imguGetText(I,"sift");
  sscanf(sift_header," %d %d",&nb_kpoints,&descr_size);
  for(i=0;i<nb_kpoints;i++) {
    sprintf(buf,"sift_%08d",i);
    printf("%s\n",imguGetText(I,buf));
  }

  imguFree(&I);

  return 0;
}

