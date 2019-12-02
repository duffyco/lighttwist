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

int prmdLaplacian(pyramid *p,imgu *img,double dev,double scale_factor)
{
    int l;
    imgu *temp;

    if (p==NULL) return -1;
    if (img==NULL) return -1;

    if (scale_factor<0.0001) return -1;

    temp=NULL;

    imguCopy(&temp,img);
    for(l=0;l<p->nblevels;l++)
    {
        imguBlur(&(p->levels[l][GAUSSIAN]),temp,dev,dev,CONVOLVE_KEEP_MARGIN);
        imguSubtract(&(p->levels[l][LAPLACIAN]),temp,p->levels[l][GAUSSIAN],IMGU_MAXVAL/2);
        imguCopy(&temp,p->levels[l][GAUSSIAN]);
        imguScale(&temp,temp,1/scale_factor,1/scale_factor);
        //dev*=scale_factor;
    }

    imguFree(&temp);

    return 0;
}

int prmdMipMap(pyramid *p,imgu *img,double dev,double scale_factor)
{
    int l;
    imgu *temp,*temp2;

    if (p==NULL) return -1;
    if (img==NULL) return -1;

    if (scale_factor<0.0001) return -1;

    temp=NULL;
    temp2=NULL;

    imguCopy(&temp,img);
    for(l=0;l<p->nblevels;l++)
    {
        imguCopy(&(p->levels[l][GAUSSIAN]),temp);
        imguFree(&(p->levels[l][LAPLACIAN]));
        imguBlur(&temp2,temp,dev,dev,CONVOLVE_KEEP_MARGIN);
        imguScale(&temp,temp2,1/scale_factor,1/scale_factor);
        //dev*=scale_factor;
    }

    imguFree(&temp);
    imguFree(&temp2);

    return 0;
}

int prmdCreate(pyramid **p,imgu *ims,int nblevels,double dev,double scale_factor,int (*fcn)(pyramid *,imgu *,double,double))
{
    int i;
    imgu ***levels;

    if ((*p)==NULL)
    {
      (*p)=(pyramid *)(malloc(sizeof(pyramid)));
      (*p)->levels=NULL;
      (*p)->lsize=0;
      (*p)->next=NULL;
    }

    if ((*p)->lsize<nblevels)
    {
      prmdFreeData(p);
      levels=(imgu ***)(malloc(sizeof(imgu **)*nblevels));
      if(levels==NULL)
      {
        (*p)->nblevels=0;
        (*p)->levels=NULL;
        return(-1);
      }

      for (i=0;i<nblevels;i++)
      {
        levels[i]=(imgu **)(malloc(sizeof(imgu *)*2));
        if (levels[i]==NULL)
        {
          prmdFree(p);
          return(-1);
        }
        levels[i][GAUSSIAN]=NULL;
        levels[i][LAPLACIAN]=NULL;
      }

      (*p)->levels=levels;
      (*p)->lsize=nblevels;
    }
    (*p)->nblevels=nblevels;

    (*fcn)((*p),ims,dev,scale_factor);

    return 0;
}

int prmdCreateMulti(pyramid **p,imgu *ims,int nblevels,double dev,double scale_factor,int (*fcn)(pyramid *,imgu *,double,double))
{
  if (ims==NULL)
  {
    prmdFreeMulti(p); //in case prmdAllocateMulti was called before with a longer ims list, we need to free the remaining structures
    return 0;
  }

  prmdCreate(p,ims,nblevels,dev,scale_factor,fcn);
  return  prmdCreateMulti(&((*p)->next),ims->next,nblevels,dev,scale_factor,fcn);
}

void prmdFreeData(pyramid **p)
{
    int i;
    if( p!=NULL && (*p)!=NULL && (*p)->levels!=NULL)
    {
        for(i=0;i<(*p)->nblevels;i++)
        {
          if ((*p)->levels[i]!=NULL)
          {
            imguFree(&((*p)->levels[i][GAUSSIAN]));
            imguFree(&((*p)->levels[i][LAPLACIAN]));
          }
          free((*p)->levels[i]);
          (*p)->levels[i]=NULL;
        }
        free((*p)->levels);
        (*p)->levels=NULL;
    }
    (*p)->nblevels=0;
    (*p)->lsize=0;
}

void prmdFree(pyramid **p)
{
    if (p==NULL || (*p)==NULL) return; 
    prmdFreeData(p);
    free((*p));
    (*p)=NULL;
}

void prmdFreeMulti(pyramid **p)
{
    if (p==NULL) return;

    pyramid *ptemp;

    while( *p!=NULL ) {
	// free *p, but save the next link before its too late...
	ptemp=(*p)->next;
	prmdFree(p);
	*p=ptemp;
    }
}



