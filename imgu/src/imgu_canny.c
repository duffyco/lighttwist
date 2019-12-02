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


#define CANNY_MAGN 0
#define CANNY_PHASE 1
#define CANNY_VISITED 0
#define CANNY_COUNT 1

static int NONMAX_SUPPRESSION(imgu *I)
{
  int i,j,index;
  int xsize,ysize;
  double temp;
  int direction;

  if (I==NULL || I->complex==NULL) return -1;

  xsize=I->xs;
  ysize=I->ys;

  index=0;
  for(j=1;j<ysize-1;j++)
  {
    for(i=1;i<xsize-1;i++)
    { 
      index=j*xsize+i;
      //phase is between 0 and 2*pi
      if (I->complex[index][CANNY_PHASE]<M_PI) temp=I->complex[index][CANNY_PHASE];
      else temp=I->complex[index][CANNY_PHASE]-M_PI;
      direction=(int)(temp/(M_PI/8));

      I->complex[index][CANNY_PHASE]=I->complex[index][CANNY_MAGN];

      //find smallest direction difference and set to 0 if not the maximum
      if (direction==0 || direction==7) //0 or pi
      {
        if (I->complex[index][CANNY_MAGN]<=I->complex[j*xsize+i-1][CANNY_MAGN] || I->complex[index][CANNY_MAGN]<I->complex[j*xsize+i+1][CANNY_MAGN]) I->complex[index][CANNY_PHASE]=0;
      }
      else if (direction<3)
      {
        if (I->complex[index][CANNY_MAGN]<=I->complex[(j-1)*xsize+i-1][CANNY_MAGN] || I->complex[index][CANNY_MAGN]<I->complex[(j+1)*xsize+i+1][CANNY_MAGN]) I->complex[index][CANNY_PHASE]=0;
      }
      else if (direction<5)
      {
        if (I->complex[index][CANNY_MAGN]<=I->complex[(j-1)*xsize+i][CANNY_MAGN] || I->complex[index][CANNY_MAGN]<I->complex[(j+1)*xsize+i][CANNY_MAGN]) I->complex[index][CANNY_PHASE]=0;
      }
      else
      {
        if (I->complex[index][CANNY_MAGN]<=I->complex[(j-1)*xsize+i+1][CANNY_MAGN] || I->complex[index][CANNY_MAGN]<I->complex[(j+1)*xsize+i-1][CANNY_MAGN]) I->complex[index][CANNY_PHASE]=0;
      }
      index++;
    }
  }

  index=0;
  for(j=0;j<ysize;j++)
  {
    for(i=0;i<xsize;i++)
    { 
      if (i==0 || j==0 || i==xsize || j==ysize) I->complex[index][CANNY_MAGN]=0;
      else I->complex[index][CANNY_MAGN]=I->complex[index][CANNY_PHASE];
      index++;
    }
  }


  return 0;
}

static int HYSTERESIS_THRESH_HELPER(imgu *E,imgu *visited,int x,int y,int eindex,double th,double tl,int level)
{
  int index;
  int xsize,ysize;
  double temp;
  int direction;

  if (E==NULL || visited==NULL) return -1;
  if (E->complex==NULL || visited->complex==NULL) return -1;

  xsize=E->xs;
  ysize=E->ys;

  if (x<0) return 0;
  if (x>=xsize) return 0;
  if (y<0) return 0;
  if (y>=ysize) return 0;
  
  index=y*xsize+x;

  if (visited->complex[index][CANNY_VISITED]!=-1.0) return 0;
  if (E->complex[index][CANNY_MAGN]>th || (level && E->complex[index][CANNY_MAGN]>tl))
  {
    visited->complex[index][CANNY_VISITED]=(double)(eindex);
    visited->complex[eindex][CANNY_COUNT]++;
  }
  else return 0;
  
  //phase is between 0 and 2*pi
  if (E->complex[index][CANNY_PHASE]<M_PI) temp=E->complex[index][CANNY_PHASE];
  else temp=E->complex[index][CANNY_PHASE]-M_PI;
  direction=(int)(temp/(M_PI/8));

  //find smallest direction difference and recursively continue to track the edge in both directions
  if (direction==0 || direction==7) //0 or pi
  {
    HYSTERESIS_THRESH_HELPER(E,visited,x,y-1,eindex,th,tl,level++);
    HYSTERESIS_THRESH_HELPER(E,visited,x,y+1,eindex,th,tl,level++);
  }
  else if (direction<3) //pi/4
  {
    HYSTERESIS_THRESH_HELPER(E,visited,x-1,y+1,eindex,th,tl,level++);
    HYSTERESIS_THRESH_HELPER(E,visited,x+1,y-1,eindex,th,tl,level++);
  }
  else if (direction<5) //2*pi/4
  {
    HYSTERESIS_THRESH_HELPER(E,visited,x-1,y,eindex,th,tl,level++);
    HYSTERESIS_THRESH_HELPER(E,visited,x+1,y,eindex,th,tl,level++);
  }
  else //3*pi/4
  {
    HYSTERESIS_THRESH_HELPER(E,visited,x+1,y+1,eindex,th,tl,level++);
    HYSTERESIS_THRESH_HELPER(E,visited,x-1,y-1,eindex,th,tl,level++);
  }
  
  return  0;
}

//step in Canny
static int HYSTERESIS_THRESH(imgu *I,double th,double tl)
{
  int i,j,index;
  int xsize,ysize;
  imgu *Itemp;

  if (tl>th) return -1;

  Itemp=NULL;

  imguCopy(&Itemp,I);

  xsize=I->xs;
  ysize=I->ys;

  index=0;
  for(i=0;i<ysize;i++)
  {
    for(j=0;j<xsize;j++)
    {
      I->complex[index][CANNY_VISITED]=-1.0;
      I->complex[index][CANNY_COUNT]=0.0;
      index++;
    }
  }

  index=0;
  for(i=0;i<ysize;i++)
  {
    for(j=0;j<xsize;j++)
    {
      HYSTERESIS_THRESH_HELPER(Itemp,I,j,i,index,th,tl,0);
      index++;
    }
  }

  imguFree(&Itemp);

  return 0;
}

#define CANNY_DX 0
#define CANNY_DY 1
#define CANNY_DXDY 2

static int CannyEdgeDetection(imgu **Idest,imgu *Isrc, double stddev,double th,double tl,int dxdy,int maxonly,int hysteresis)
{
  int i,j,index;
  imgu *Idx,*Idy,*derivFilter;
  imgu *Icpy;
  imgu *J;
  unsigned char realloc;

  if (Idest==NULL || Isrc==NULL) return -1;
  if (Isrc->data==NULL) return -1;

  Idx=NULL;
  Idy=NULL;
  derivFilter=NULL;

  realloc=0;
  Icpy=Isrc;
  if ((*Idest)==Isrc)
  {
    realloc=1;
    Icpy=NULL;
    imguCopy(&Icpy,Isrc);
  }

  //Apply Gaussian filter
  imguConvertToLuminance(Idest,Icpy);
  imguRemoveAlphaLayer(Idest,*Idest);
  if (dxdy==CANNY_DX) imguBlur(Idest,*Idest,stddev,0,CONVOLVE_KEEP_MARGIN);
  else if (dxdy==CANNY_DY) imguBlur(Idest,*Idest,0,stddev,CONVOLVE_KEEP_MARGIN);
  else imguBlur(Idest,*Idest,stddev,stddev,CONVOLVE_KEEP_MARGIN);

  J=*Idest;

  //Compute x,y derivatives
  imguFirstDerivFilter(&derivFilter);
  imguConvertToComplexComponent(&J,J,0);
  if (dxdy!=CANNY_DY) imguConvolveComplex(&Idx,J,derivFilter,CONVOLVE_BLANK_MARGIN);
  derivFilter->ys=derivFilter->xs;
  derivFilter->xs=1;
  if (dxdy!=CANNY_DX) imguConvolveComplex(&Idy,J,derivFilter,CONVOLVE_BLANK_MARGIN);

  //Copy derivatives
  index=0;
  for (i=0;i<J->ys;i++)
  {
    for (j=0;j<J->xs;j++)
    {
      if (dxdy!=CANNY_DY) J->complex[index][0]=Idx->complex[index][0];
      else J->complex[index][0]=0;
      if (dxdy!=CANNY_DX) J->complex[index][1]=Idy->complex[index][0];
      else J->complex[index][1]=0;
      index++;
    }
  }

  imguMagnPhase(&J,J);

  if (maxonly) NONMAX_SUPPRESSION(J);

  if (hysteresis)
  {
    HYSTERESIS_THRESH(J,th,tl);
    index=0;
    for (i=0;i<J->ys;i++)
    {
      for (j=0;j<J->xs;j++)
      {
        if (J->complex[index][CANNY_VISITED]!=-1.0)
        {
          //replace pixel values by the length of the edge it is part of   
          //J->complex[index][CANNY_VISITED]=J->complex[(int)(J->complex[index][CANNY_VISITED])][CANNY_COUNT];

          //replace pixel values of edges by IMGU_MAXVAL
          J->complex[index][CANNY_VISITED]=IMGU_MAXVAL;

          //only keep edges longer than 4
          //if (J->complex[(int)(J->complex[index][CANNY_VISITED])][CANNY_COUNT]<4) J->complex[index][CANNY_VISITED]=IMGU_MAXVAL;
          //else J->complex[index][CANNY_VISITED]=0;
        }
        index++;
      }
    }
  }

  imguConvertFromComplexComponent(&J,J,0,COMPLEX_RESCALE);

  if (realloc) imguFree(&Icpy);
  imguFree(&Idx);
  imguFree(&Idy);
  imguFree(&derivFilter);

  return 0;
}

/**
 * Finds and tracks edges in image @p src. 
 *
 * Pixels of longest edge will be have the value IMGU_MAXVAL.
 * Pixels of shorter edges will have a value > 0.
 * Pixels not part of any edge will have a value of 0.
 *
 * @param[in] dest address of pointer to output image containing the edges
 * @param[in] src input image pointer
 * @param[in] stddev standard deviation of gaussian blurring kernel. Removes image noise, but also related to the scale at which the edges are detected (i.e. the bigger @p stddev is, the larger the scale). A typical value is 1.5.
 * @param[in] th high threshold for hysteresis step. If the gradient magnitude of a pixel is bigger than @p th, then this pixel can be accepted as a starting pixel in the edge tracking process.
 *            A typical value is 20*257 (i.e. 20 in standard 8-bit values, mapped to 16-bit values).
 * @param[in] tl low threshold for hysteresis step. If the gradient magnitude of a pixel is bigger than @p tl, then this pixel can be accepted as a NON-starting pixel in the edge tracking process, i.e.
 *            this pixel can continue the edge chain.
 *            A typical value is 5*257 (i.e. 5 in standard 8-bit values, mapped to 16-bit values).
 *
 * @attention Both parameters @p th,tl are highly dependent on image contrast (see imguEqualize() to normalize contrast by histogram normalization).
 *             
 * @return 0: success; <0: error
 *
 * @example imguCanny.c
 */
int imguCannyEdgeTracking(imgu **dest,imgu *src, double stddev,double th,double tl)
{
  return CannyEdgeDetection(dest,src,stddev,th,tl,CANNY_DXDY,1,1);
}

int imguCannyEdgeDetection(imgu **dest,imgu *src, double stddev)
{
  return CannyEdgeDetection(dest,src,stddev,0.0,0.0,CANNY_DXDY,1,0);
}

int imguCannyBasic(imgu **dest,imgu *src, double stddev)
{
  return CannyEdgeDetection(dest,src,stddev,0.0,0.0,CANNY_DXDY,0,0);
}

int imguCannyHorizontalEdgeTracking(imgu **dest,imgu *src, double stddev,double th,double tl)
{
  return CannyEdgeDetection(dest,src,stddev,th,tl,CANNY_DX,1,1);
}

int imguCannyHorizontalEdgeDetection(imgu **dest,imgu *src, double stddev)
{
  return CannyEdgeDetection(dest,src,stddev,0.0,0.0,CANNY_DX,1,0);
}

int imguCannyVerticalEdgeTracking(imgu **dest,imgu *src, double stddev,double th,double tl)
{
  return CannyEdgeDetection(dest,src,stddev,th,tl,CANNY_DY,1,1);
}

int imguCannyVerticalEdgeDetection(imgu **dest,imgu *src, double stddev)
{
  return CannyEdgeDetection(dest,src,stddev,0.0,0.0,CANNY_DY,1,0);
}

/**
 * @todo imguLineThinner
 */
int imguLineThinner(imgu **Idest,imgu *Isrc, double stddev,int threshold)
{
  int i,j,index;
  imgu *Idx,*Idy,*derivFilter;
  imgu *Icpy;
  imgu *J;
  unsigned char realloc;

  if (Idest==NULL || Isrc==NULL) return -1;
  if (Isrc->data==NULL) return -1;
  Idx=NULL;
  Idy=NULL;
  derivFilter=NULL;

  realloc=0;
  Icpy=Isrc;
  if ((*Idest)==Isrc)
  {
    realloc=1;
    Icpy=NULL;
    imguCopy(&Icpy,Isrc);
  }

  //Apply Gaussian filter
  imguConvertToGray(Idest,Icpy);
  //imguInvert(Idest,*Idest);
  imguRemoveAlphaLayer(Idest,*Idest);
  imguBlur(Idest,*Idest,stddev,stddev,CONVOLVE_KEEP_MARGIN);
  J=*Idest;

  //Compute x,y derivatives
  imguFirstDerivFilter(&derivFilter);
  imguConvertToComplexComponent(&J,J,0);
  imguConvolveComplex(&Idx,J,derivFilter,CONVOLVE_BLANK_MARGIN);
  derivFilter->ys=derivFilter->xs;
  derivFilter->xs=1;
  imguConvolveComplex(&Idy,J,derivFilter,CONVOLVE_BLANK_MARGIN);

  //Copy derivatives
  index=0;
  for (i=0;i<J->ys;i++)
  {
    for (j=0;j<J->xs;j++)
    {
      J->complex[index][0]=Idx->complex[index][0];
      J->complex[index][1]=Idy->complex[index][0];
      index++;
    }
  }

  imguMagnPhase(&J,J);

  index=0;
  for (i=0;i<J->ys;i++)
  {
    for (j=0;j<J->xs;j++)
    {
      J->complex[index][0]=J->data[index];
      index++;
    }
  }

  NONMAX_SUPPRESSION(J);

  imguConvertFromComplexComponent(&J,J,0,COMPLEX_RESCALE);

  for (i=0;i<J->ds;i++)
  {
    if (J->data[i]>threshold) J->data[i]=IMGU_MAXVAL;
    else J->data[i]=0;
  }

  if (realloc) imguFree(&Icpy);
  imguFree(&Idx);
  imguFree(&Idy);
  imguFree(&derivFilter);

  return 0;
}


