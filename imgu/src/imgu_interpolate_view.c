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

int imguForwardMapping(imgu **Idest,imgu *dmA,imgu *dmB,imgu *IA,imgu *IB,double alpha)
{
  int i,j,k,index;
  matrix *disps;
  double da,db;
  double xa,xb;

  if (dmA==NULL || dmA->data==NULL) return -1;
  if (dmB==NULL || dmB->data==NULL) return -1;
  if (IA==NULL || IA->data==NULL) return -1;
  if (IB==NULL || IB->data==NULL) return -1;
  if ((*Idest)==dmA || (*Idest)==dmB || (*Idest)==IA || (*Idest)==IB) return -1;

  if (dmA->xs!=dmB->xs) return -1;
  if (dmA->ys!=dmB->ys) return -1;
  if (dmA->cs!=dmB->cs) return -1;
  if (dmA->cs!=1) return -1;

  if (dmA->xs!=IA->xs) return -1;
  if (dmA->ys!=IA->ys) return -1;

  if (IA->xs!=IB->xs) return -1;
  if (IA->ys!=IB->ys) return -1;
  if (IA->cs!=IB->cs) return -1;

  if (IA->cs>4) return -1;

  if (alpha<0.0 || alpha>1.0) return -1;

  if (imguAllocate(Idest,IA->xs,IA->ys,IA->cs)!=0) return -1;
  if (imguAllocateComplex(Idest,IA->xs,IA->ys,IA->cs)!=0) return -1;
  imguClear((*Idest));
  imguClearComplex((*Idest),0);

  disps=NULL;
  if (imguLoadMatrix(&disps,dmA,"DISPARITIES")!=0) return -1;

  for (i=0;i<IA->ys;i++)
  {
    for (j=0;j<IA->xs;j++)
    {
      da=fabs(matGet(disps,dmA->data[i*dmA->xs+j]-1,0)); //indexing in depthmap starts at 1, but 'disps' indexing starts at 0
      xa=j-alpha*da;
      if (imguCheck(IA,xa,(double)(i))==0)
      {
        index=(i*IA->xs+((int)(xa)))*IA->cs;
        //keep mapping of closest depth
        if ((*Idest)->complex[index][0]<1e-8 || (*Idest)->complex[index][0]<da)
        {
          (*Idest)->complex[index][0]=da;
          for (k=0;k<IA->cs;k++)
          {
            index=(i*IA->xs+((int)(xa)))*IA->cs+k;      
            (*Idest)->data[index]=PIXEL(IA,j,i,k);
          }
        }
      }

      //same as above, but for image IB
      db=fabs(matGet(disps,dmB->data[i*dmB->xs+j]-1,0));
      xb=j+(1.0-alpha)*db;
      if (imguCheck(IB,xb,(double)(i))==0)
      {
        index=(i*IB->xs+((int)(xb)))*IB->cs;
        if ((*Idest)->complex[index][0]<1e-8 || (*Idest)->complex[index][0]<db)
        {
          (*Idest)->complex[index][0]=db;
          for (k=0;k<IB->cs;k++)
          {
            index=(i*IB->xs+((int)(xb)))*IB->cs+k;      
            (*Idest)->data[index]=PIXEL(IB,j,i,k);
          }
        }
      }
    }
  }

  matFree(&disps);

  return 0;
}

int imguBackwardMapping(imgu **Idest,imgu *dmA,imgu *dmB,imgu *IA,imgu *IB,double alpha)
{
  int i,j,k,d;
  double xa,xb;
  double xamin,xbmin;
  //int dmin;
  double dval,dvalmin;
  double disp;
  vector4 avals,bvals;

  matrix *disps;

  if (dmA==NULL || dmA->data==NULL) return -1;
  if (dmB==NULL || dmB->data==NULL) return -1;
  if (IA==NULL || IA->data==NULL) return -1;
  if (IB==NULL || IB->data==NULL) return -1;
  if ((*Idest)==dmA || (*Idest)==dmB || (*Idest)==IA || (*Idest)==IB) return -1;

  if (dmA->xs!=dmB->xs) return -1;
  if (dmA->ys!=dmB->ys) return -1;
  if (dmA->cs!=dmB->cs) return -1;
  if (dmA->cs!=1) return -1;

  if (dmA->xs!=IA->xs) return -1;
  if (dmA->ys!=IA->ys) return -1;

  if (IA->xs!=IB->xs) return -1;
  if (IA->ys!=IB->ys) return -1;
  if (IA->cs!=IB->cs) return -1;

  if (IA->cs>4) return -1;

  if (alpha<0.0 || alpha>1.0) return -1;

  if (imguAllocate(Idest,IA->xs,IA->ys,IA->cs)!=0) return -1;

  disps=NULL;
  if (imguLoadMatrix(&disps,dmA,"DISPARITIES")!=0) return -1;

  xamin=0;
  xbmin=0;
  for (i=0;i<IA->ys;i++)
  {
    for (j=0;j<IA->xs;j++)
    {
      for (d=0;d<disps->cs;d++)
      {
        disp=fabs(disps->values[d]);
        xa=j+alpha*disp;
        xb=j-(1.0-alpha)*disp;
        imguInterpolateBilinear(IA,xa,(double)(i),avals);
        imguInterpolateBilinear(IB,xb,(double)(i),bvals);
        dval=0;
        for (k=0;k<IA->cs;k++)
        {
          dval+=fabs(avals[k]-bvals[k]);
        }
        if (d==0 || dval<dvalmin)
        {
          dvalmin=dval;
          //dmin=d;
          xamin=xa;
          xbmin=xb;
        }
      }

      imguInterpolateBilinear(IA,xamin,(double)(i),avals);
      imguInterpolateBilinear(IB,xbmin,(double)(i),bvals);
      for (k=0;k<IA->cs;k++)
      {
        (*Idest)->data[(i*IA->xs+j)*IA->cs+k]=alpha*avals[k]+(1.0-alpha)*bvals[k];
        //(*Idest)->data[(i*IA->xs+j)*IA->cs+k]=(1.0-alpha)*avals[k]+alpha*bvals[k];
        //(*Idest)->data[(i*IA->xs+j)*IA->cs+k]=dmin;
      }
    }
  }

  matFree(&disps);

  return 0;
}



