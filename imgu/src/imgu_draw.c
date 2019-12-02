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

#include <string.h>

#include "imgu.h"


/**
 * Draw a line between two points
 *
 * This function draws a line between th points @c (x1,y1) and @c (x2,y2)
 * using the @c color specified. It can draw on all channels at the same time (is @c c < 0 )
 * or draw to a single channel (when @c >= 0).
 *
 * @param[in] I Image
 * @param[in] x1,y1 First point of line
 * @param[in] x2,y2 Second point of line
 * @param[in] c Number of colors. If c<0 then assumes @c color is for all channels, otherwise use c as the channel selection (the color must be in color[0]).
 * @param[in] color Array of 16bit colors to draw. When c<0, color must be at least of size I->cs. When c>=0, color can be of size 1.
 */
void imguDrawLine(imgu *I,double x1,double y1,double x2,double y2,int c,pix_t *color)
{
  double dx,dy;
  int x,y;
  int nb,i;

  if (I==NULL || I->data==NULL) return;

  if( isnan(x1) || isnan(y1) || isnan(x2) || isnan(y2) ) return;

  //printf("LINE (%12.6f,%12.6f) (%12.6f,%12.6f)\n",x1,y1,x2,y2);
  dx=x2-x1;if( dx<0 ) dx=-dx;
  dy=y2-y1;if( dy<0 ) dy=-dy;
  if( dx<dy ) dx=dy;
  nb=dx*2;
  // loop back to overwrite at 0
  for(i=nb-1;i>=0;i--) {
    x=(int)((x2-x1)*i/(nb-1.0)+x1+0.5);
    y=(int)((y2-y1)*i/(nb-1.0)+y1+0.5);
    if( x<0 || x>=I->xs || y<0 || y>=I->ys ) continue;
    if( c<0 ) {
	int cc;
	for(cc=0;cc<I->cs;cc++)
	    I->data[(y*I->xs+x)*I->cs+cc]=color[cc];
    }else{
	    I->data[(y*I->xs+x)*I->cs+c]=color[0];
    }
  }
}


static double distance(double p1,double p2,double c1,double c2)
{
    double len=sqrt((p1-c1)*(p1-c1)+(p2-c2)*(p2-c2));

    return len;
}


/**
 * @todo doc
 *
 * linelen is a fraction of the image, i.e. should be between 0 and 1
 */
int imguDrawLineLength(imgu *img,double cx,double cy,double x,double y,vector3 rgb,double linelen)
{
    int j,l;
    double dx,dy;
    double len;
    double px,py;
    int ix,iy;
    double centerx,centery;
    int index;
    double currlen,maxlen;

    if (img==NULL || img->data==NULL) return -1;

    len=sqrt(x*x+y*y);
    if (len<0.0001) return -1;

    dx=x;
    dy=y;
    dx/=len;
    dy/=len;
    dx/=2;
    dy/=2;

    centerx=img->xs/2.0;
    centery=img->ys/2.0;
    maxlen=sqrt(centerx*linelen*centerx*linelen+centery*linelen*centery*linelen);

    centerx=cx;
    centery=cy;

    //2 pass: after and before center
    for(j=0;j<2;j++)
    {
      px=centerx;
      py=centery;
      currlen=distance(px,py,centerx,centery);
      while(currlen<maxlen)
      {
        ix=(int)(px+0.5);
        iy=(int)(py+0.5);
//fprintf(stderr,"FIRST %d %f %f %f\n",j,px,py,currlen);
        if (imguCheck(img,(float)(ix),(float)(iy))==0) 
        {
          for(l=0;l<img->cs;l++)
          {
            index=(iy*img->xs+ix)*img->cs+l;
            img->data[index]=(pix_t)(rgb[l]);
          }
        }

        if (j==0) 
        {
          px+=dx;
          py+=dy;
        }
        else
        {
          px-=dx;
          py-=dy;
        }
        currlen=distance(px,py,centerx,centery);
      }
    }

    return 0;
}

/**
 * @todo doc
 */
int imguDrawSegment(imgu *img, float x1, float y1, float x2, float y2,vector3 rgb)
{
    int i, l;
    float dx, dy;
    float temp;
    int index;
    int px,py;
    int csize;

    if (img==NULL || img->data==NULL) return -1;

    if (fabs(x1 - x2) < 0.0001) return -1;
    if (fabs(y1 - y2) < 0.0001) return -1;

    csize=img->cs;

    /* Is line more horizontal than vertical? */
    if (fabs(x2 - x1) < fabs(y2 - y1)) {
        /* Put points in increasing order by column. */
        if (y1 > y2) {
            temp = x1; x1 = x2; x2 = temp;
            temp = y1; y1 = y2; y2 = temp;
        }
        dx = x2 - x1;
        dy = y2 - y1;
        for (i = (int)(y1); i <= (int)(y2); i++)
        {
            px=(int)(x1 + (i - y1) * dx / dy);
            py=i;
            if (imguCheck(img,(float)(px),(float)(py))==0)
            {
                for (l=0;l<csize;l++)
                {
                    index=(py*img->xs+px)*csize+l;
                    img->data[index] = (pix_t)(rgb[l]);
                }
            }
        }
    } else {
        if (x1 > x2) {
            temp = x1; x1 = x2; x2 = temp;
            temp = y1; y1 = y2; y2 = temp;
        }
        dx = x2 - x1;
        dy = y2 - y1;
        for (i = (int)(x1); i <= (int)(x2); i++)
        {
            px=i;
            py=(int)(y1 + (i - x1) * dy / dx);
            if (imguCheck(img,(float)(px),(float)(py))==0)
            {
                for (l=0;l<csize;l++)
                {
                    index=(py*img->xs+px)*csize+l;
                    img->data[index] = (pix_t)(rgb[l]);
                }
            }
        }
    }

    return 0;
}

/**
 * @todo doc
 *
 */
int imguDrawCross(imgu *img, float x, float y,int len,vector3 rgb)
{
  int i,j,k,l;
  int px,py;

  if (img==NULL || img->data==NULL) return -1;
  if (img->cs>3) return -1;

  px=(int)(x+0.5);
  py=(int)(y+0.5);
  for (l=-len/2;l<=len/2;l++)  
  {
    j=px+l;
    i=py;
    if (imguCheck(img,(float)(j),(float)(i))==0)
    {
      for (k=0;k<img->cs;k++)
      {
        img->data[(i*img->xs+j)*img->cs+k]=rgb[k];
      }
    }
    j=px;
    i=py+l;
    if (imguCheck(img,(float)(j),(float)(i))==0)
    {
      for (k=0;k<img->cs;k++)
      {
        img->data[(i*img->xs+j)*img->cs+k]=rgb[k];
      }
    }
  }
        
  return 0;
}


