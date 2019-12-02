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
#include "fftn.h"

//#define TOLERANCE 0.01
#define TOLERANCE 0.1
#define COMPLEX_SMALL 0.000000000001

#define HLSMAX IMGU_MAXVAL
#define RGBMAX IMGU_MAXVAL

#define LUM_MIN 8192
#define LUM_SCALE 0.75

static float HueToRGB(float n1, float n2, float hue)
{
    /* range check: note values passed add/subtract thirds of range */
    if (hue < 0) hue += HLSMAX;

    if (hue > HLSMAX) hue -= HLSMAX;

    /* return r,g, or b value from this tridrant */
    if (hue < (HLSMAX/6)) return ( n1 + (((n2-n1)*hue+(HLSMAX/12))/(HLSMAX/6)) );
    if (hue < (HLSMAX/2)) return ( n2 );
    if (hue < ((HLSMAX*2)/3)) return ( n1 + (((n2-n1)*(((HLSMAX*2)/3)-hue)+(HLSMAX/12))/(HLSMAX/6)));
    else return ( n1 );
}


void imguHLStoRGB(vector3 hls,vector3 rgb)
{
    float r,g,b;
    float Magic1,Magic2;

    if (hls[2]<COMPLEX_SMALL)
    {        /* achromatic case */
        r=g=b=(hls[1]*RGBMAX)/HLSMAX;
    }
    else
    {       /* chromatic case */
         /* set up magic numbers */
         if (hls[1] <= (HLSMAX/2)) Magic2 = (hls[1]*(HLSMAX + hls[2]) + (HLSMAX/2))/HLSMAX;
         else Magic2 = hls[1] + hls[2] - ((hls[1]*hls[2]) + (HLSMAX/2))/HLSMAX;
         Magic1 = 2*hls[1]-Magic2;

         /* get RGB, change units from HLSMAX to RGBMAX */
         r = (HueToRGB(Magic1,Magic2,hls[0]+(HLSMAX/3))*RGBMAX + (HLSMAX/2))/HLSMAX;
         g = (HueToRGB(Magic1,Magic2,hls[0])*RGBMAX + (HLSMAX/2)) / HLSMAX;
         b = (HueToRGB(Magic1,Magic2,hls[0]-(HLSMAX/3))*RGBMAX + (HLSMAX/2))/HLSMAX;
    }

    if (r>IMGU_MAXVAL) r=IMGU_MAXVAL;
    if (g>IMGU_MAXVAL) g=IMGU_MAXVAL;
    if (b>IMGU_MAXVAL) b=IMGU_MAXVAL;
    if (r<0) r=0;
    if (g<0) g=0;
    if (b<0) b=0;

    rgb[0]=r;
    rgb[1]=g;
    rgb[2]=b;
}


static float Fmin(float a,float b)
{
    if (a<b) return a;
    else return b;
}

static float Fmax(float a,float b)
{
    if (a>b) return a;
    else return b;
}

void imguRGBtoHLS(vector3 rgb,vector3 hls)
{
    float min;
    float max;
    float r,g,b,h,l,s;
    float rdelta,gdelta,bdelta;
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];

    // find minimum and maximum
    max = Fmax(Fmax(r,g),b);
    min = Fmin(Fmin(r,g),b);

    l = (((max+min)*HLSMAX)+RGBMAX)/(2*RGBMAX); //lightness

    if (fabs(max-min)<COMPLEX_SMALL)
    {
        s = 0;
        h = 0;
    }
    else
    {
        if (l <= (HLSMAX/2)) s = ( ((max-min)*HLSMAX) + ((max+min)/2) ) / (max+min);
        else s = (((max-min)*HLSMAX) + ((2*RGBMAX-max-min)/2) ) / (2*RGBMAX-max-min);

         /* hue */
        rdelta = (((max-r)*(HLSMAX/6)) + ((max-min)/2)) / (max-min);
        gdelta = (((max-g)*(HLSMAX/6)) + ((max-min)/2)) / (max-min);
        bdelta = (((max-b)*(HLSMAX/6)) + ((max-min)/2)) / (max-min);

        if (fabs(r-max)<COMPLEX_SMALL) h = bdelta - gdelta;
        else if (fabs(g-max)<COMPLEX_SMALL) h = (HLSMAX/3) + rdelta - bdelta;
        else h = ((2*HLSMAX)/3) + gdelta - rdelta;

        if (h < 0) h += HLSMAX;
        if (h > HLSMAX) h -= HLSMAX;
    }

    if (h>IMGU_MAXVAL) h=IMGU_MAXVAL;
    if (l>IMGU_MAXVAL) l=IMGU_MAXVAL;
    if (s>IMGU_MAXVAL) s=IMGU_MAXVAL;
    if (h<0) h=0;
    if (l<0) l=0;
    if (s<0) s=0;

    hls[0]=h;
    hls[1]=l;
    hls[2]=s;
}

int imguConvertToComplexComponent(imgu **dest,imgu *src,unsigned char component)
{
    int index;
    int xsize,ysize,csize,size;
    char *buf;
    double min,max,scale;

    if (component!=0 && component!=1) return -1;
    if (dest==NULL || src==NULL || src->data==NULL) return -1;

    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;

    if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;

    min=0.0;
    scale=(double)(IMGU_MAXVAL);

    buf=imguGetText(src,"FMIN");
    if (buf!=NULL) sscanf(buf,"%lg",&min);
    buf=imguGetText(src,"FMAX");
    if (buf!=NULL)
    {
      sscanf(buf,"%lg",&max);
      scale=max-min;
    }

//fprintf(stderr,"FACTOR: %1.14f (%f,%f)\n",scale,min,max);

    size=ysize*xsize*csize;
    for(index=0;index<size;index++)
    {
        if (scale<COMPLEX_SMALL) (*dest)->complex[index][component]=(complex_t)(src->data[index]+min);
        else (*dest)->complex[index][component]=(complex_t)(src->data[index]*scale/IMGU_MAXVAL+min);
    }

    return 0;
}

int imguConvertFromComplexComponent(imgu **dest,imgu *src,unsigned char component,unsigned char scale)
{
    int i,index;
    int xsize,ysize,csize,size;
    complex_t min,max;
    double temp;
    double factor;
    char buf[32];

    if (component!=0 && component!=1) return -1;
    if (dest==NULL || src==NULL || src->complex==NULL) return -1;

    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;

    if (imguAllocate(dest,xsize,ysize,csize)) return -1;

    size=xsize*ysize*csize;

    //find min and max
    min=max=src->complex[0][component];
    for(i=1;i<size;i++)
    {
        if (min>src->complex[i][component])
        {
            min=src->complex[i][component];
        }
        if (max<src->complex[i][component])
        {
            max=src->complex[i][component];
        }
    }

    factor=max-min;

    if (!scale)
    {
      min=0.0;
      factor=(double)(IMGU_MAXVAL);
    }

//fprintf(stderr,"FACTOR: %1.14f (%f,%f)\n",factor,min,max);

    sprintf(buf,"%f",min);
    imguReplaceAddText(*dest,"FMIN",buf);
    sprintf(buf,"%f",max);
    imguReplaceAddText(*dest,"FMAX",buf);

    index=0;
    for(index=0;index<size;index++)
    {
        if (factor<COMPLEX_SMALL) temp=src->complex[index][component]-min;
        else temp=(src->complex[index][component]-min)/factor*IMGU_MAXVAL;
        if (temp<0.0) src->data[index]=(pix_t)(0);
        else if (temp>IMGU_MAXVAL) (*dest)->data[index]=(pix_t)(IMGU_MAXVAL);
        else (*dest)->data[index]=(pix_t)(temp+0.5);
    }

    return 0;
}

int imguConvertFromComplexUV(imgu **dest,imgu *src,unsigned char scale)
{
    int i,j,index;
    int xsize,ysize,csize,size;
    complex_t min[2],max[2];
    double temp;
    double factor[2];
    char buf[32];

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if (src->cs!=1) return -1;
    if (src==(*dest)) return -1;

    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;

    if (imguAllocate(dest,xsize,ysize,3)) return -1;

    imguClear((*dest));

    size=xsize*ysize*csize;

    //find min and max
    for(i=0;i<size;i++)
    {
        for (j=0;j<2;j++)
        {
          if (i==0 || min[j]>src->complex[i][j])
          {
            min[j]=src->complex[i][j];
          }
          if (i==0 || max[j]<src->complex[i][j])
          {
            max[j]=src->complex[i][j];
          }
        }
    }
    for (j=0;j<2;j++)
    {
      factor[j]=max[j]-min[j];
      //if (factor[j]<1e-8) return -1;
    }

    for (j=0;j<2;j++)
    {
      fprintf(stderr,"MINMAX: %f %f\n",min[j],max[j]);
      fprintf(stderr,"FACTOR: %1.14f\n",factor[j]);
    }
    if (!scale)
    {
      for (j=0;j<2;j++)
      {
        min[j]=0.0;
        factor[j]=(double)(IMGU_MAXVAL);
      }
    }

    sprintf(buf,"%f",min[0]);
    imguReplaceAddText(*dest,"FMIN_U",buf);
    sprintf(buf,"%f",max[0]);
    imguReplaceAddText(*dest,"FMAX_U",buf);
    sprintf(buf,"%f",min[1]);
    imguReplaceAddText(*dest,"FMIN_V",buf);
    sprintf(buf,"%f",max[1]);
    imguReplaceAddText(*dest,"FMAX_V",buf);

    index=0;
    for(index=0;index<size;index++)
    {
      for (j=0;j<2;j++)
      {
        if (factor[j]<COMPLEX_SMALL) temp=src->complex[index][j]-min[j];
        else temp=(src->complex[index][j]-min[j])/factor[j]*IMGU_MAXVAL;
        if (temp<0.0) src->data[index*3+j]=(pix_t)(0);
        else if (temp>IMGU_MAXVAL) (*dest)->data[index*3+j]=(pix_t)(IMGU_MAXVAL);
        else (*dest)->data[index*3+j]=(pix_t)(temp+0.5);
      }
      //(*dest)->data[index*3+j]=IMGU_MAXVAL; //fill blue channel with IMGU_MAXVAL
    }

    return 0;
}

int imguConvertToComplexUV(imgu **dest,imgu *src)
{
    int j,index;
    int xsize,ysize,csize,size;
    char *buf;
    complex_t min[2],max[2],scale[2];

    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    if (src->cs!=3) return -1;
    if (src==(*dest)) return -1;

    xsize=src->xs;
    ysize=src->ys;
    csize=1;

    if (imguAllocateComplex(dest,src->xs,src->ys,1)) return -1;

    for (j=0;j<2;j++)
    {
      min[j]=0.0;
      scale[j]=(complex_t)(IMGU_MAXVAL);
    }

    buf=imguGetText(src,"FMIN_U");
    if (buf!=NULL) sscanf(buf,"%lg",&min[0]);
    buf=imguGetText(src,"FMAX_U");
    if (buf!=NULL)
    {
      sscanf(buf,"%lg",max+0);
      scale[0]=max[0]-min[0];
    }
    buf=imguGetText(src,"FMIN_V");
    if (buf!=NULL) sscanf(buf,"%lg",&min[1]);
    buf=imguGetText(src,"FMAX_V");
    if (buf!=NULL)
    {
      sscanf(buf,"%lg",max+1);
      scale[1]=max[1]-min[1];
    }
printf("%f %f %f %f\n",min[0],max[0],min[1],max[1]);

    size=ysize*xsize*csize;
    for(index=0;index<size;index++)
    {
      for (j=0;j<2;j++)
      {
        if (scale[j]<COMPLEX_SMALL) (*dest)->complex[index][j]=(complex_t)(src->data[index*3+j]+min[j]);
        else (*dest)->complex[index][j]=(complex_t)(src->data[index*3+j]*scale[j]/IMGU_MAXVAL+min[j]);
      }
    }

    return 0;
}

int imguConvertFromComplexFlow(imgu **im,imgu *cd)
{
    char buf[32];
    int i,j,k,index;
    int xsize,ysize,csize,size;
    complex_t magn,angle;
    complex_t min,max;
    imgu *cdmagn;
    complex_t scale;
    vector3 hls,rgb;

    if (im==NULL) return -1;
    if (cd==NULL || cd->complex==NULL) return -1;
    if (cd==(*im)) return -1;

    csize=cd->cs;
    ysize=cd->ys;
    xsize=cd->xs;

    cdmagn=NULL;

    imguMagnPhase(&cdmagn,cd);

    //find min and max
    size=xsize*ysize*csize;
    min=max=cdmagn->complex[0][0];
    for(i=1;i<size;i++)
    {
        if (min>cdmagn->complex[i][0])
        {
            min=cdmagn->complex[i][0];
        }
        if (max<cdmagn->complex[i][0])
        {
            max=cdmagn->complex[i][0];
        }
    }

    scale=max-min;

    imguAllocate(im,xsize,ysize,csize*3);

    sprintf(buf,"%f",min);
    imguReplaceAddText((*im),"FMIN",buf);
    sprintf(buf,"%f",max);
    imguReplaceAddText((*im),"FMAX",buf);

    index=0;
    for (j=0;j<ysize;j++)
    {
        for (k=0;k<xsize;k++)
        {
          for (i=0;i<csize;i++)
          {
            if (scale<COMPLEX_SMALL) magn=cdmagn->complex[index][0]-min;
            else magn=(cdmagn->complex[index][0]-min)/scale*IMGU_MAXVAL;
            angle=cdmagn->complex[index][1]/(2*M_PI)*IMGU_MAXVAL;
            hls[0]=angle;
            hls[1]=magn;
            hls[2]=IMGU_MAXVAL;
            //luminance is scaled and shifted to prevent it from begin close to 0 or IMGU_MAXVAL.
            //in these cases, rgb values are all close to black or white, which makes the hue value disappear
            //since this scaling makes us lose precision, it is unclear what the optimal LUM_MIN and LUM_SCALE are
            hls[1]=LUM_MIN+hls[1]*LUM_SCALE;
            imguHLStoRGB(hls,rgb);
            (*im)->data[index*3]=(pix_t)(rgb[0]+0.5);
            (*im)->data[index*3+1]=(pix_t)(rgb[1]+0.5);
            (*im)->data[index*3+2]=(pix_t)(rgb[2]+0.5);
            index++;
          }
        }
    }

    imguFree(&cdmagn);

    return 0;
}

int imguConvertToComplexFlow(imgu **cd,imgu *im)
{
    char *buf;
    int j,k,index;
    int xsize,ysize,csize;
    complex_t min,max,scale;
    imgu *cdcomp;
    vector3 hls,rgb;

    if (im==NULL || im->data==NULL) return -1;
    if (cd==NULL) return -1;
    if ((*cd)==im) return -1;

    min=0.0;
    scale=(complex_t)(IMGU_MAXVAL);

    buf=imguGetText(im,"FMIN");
    if (buf!=NULL) sscanf(buf,"%lg",&min);
    buf=imguGetText(im,"FMAX");
    if (buf!=NULL)
    {
      sscanf(buf,"%lg",&max);
      scale=max-min;
    }

    xsize=im->xs;
    ysize=im->ys;
    csize=im->cs;

    cdcomp=NULL;
    imguAllocateComplex(&cdcomp,xsize,ysize,csize/3);

    index=0;
    for (j=0;j<ysize;j++)
    {
        for (k=0;k<xsize;k++)
        {
            rgb[0]=(double)(im->data[index*csize]);
            rgb[1]=(double)(im->data[index*csize+1]);
            rgb[2]=(double)(im->data[index*csize+2]);

            imguRGBtoHLS(rgb,hls);
            //see imguConvertFromComplexToColor to see why the following line is necessary
            hls[1]=(hls[1]-LUM_MIN)/LUM_SCALE;
            if (scale<COMPLEX_SMALL) cdcomp->complex[index][0]=(complex_t)(hls[1]+min);
            else cdcomp->complex[index][0]=(complex_t)(hls[1]*scale/IMGU_MAXVAL+min);
            cdcomp->complex[index][1]=(complex_t)(hls[0]*(2*M_PI)/IMGU_MAXVAL);

            index++;
        }
    }

    imguRealImag(cd,cdcomp);

    imguFree(&cdcomp);

    return 0;
}

/**
 * Allocates image with complex data.
 *
 * An integer image will also be allocated.
 * Complex (and integer) information is only allocated and NOT initialized to 0 (see imguClearComplex() and imguClear()).
 *
 * @attention This function keeps any text information that @p I already has. To reset text information, use imguResetText().
 * @warning Any data information should be assumed lost, except if @p xs,ys,cs correspond exactly to the data dimensions.
 *
 * @param[out] cd Address of an image pointer.
 * @param xs Width of image
 * @param ys Height of image
 * @param cs Number of color channels
 * @return 0: success; <0: error
 *
 */
int imguAllocateComplex(imgu **cd,int xs,int ys,int cs)
{
    fftw_complex *d;
    int Size;

    if (cd==NULL) return -1;

    if( imguAllocate(cd,xs,ys,cs) ) return -1;

    Size=xs*ys*cs;

    if (Size <= (*cd)->ds && (*cd)->complex!=NULL)
    {
        (*cd)->xs=xs;
        (*cd)->ys=ys;
        (*cd)->cs=cs;
        //for (i=0;i<Size;i++)
        //{
        //    (*cd)->complex[i][0]=0;
        //    (*cd)->complex[i][1]=0;
        //}
        return 0;
    }
    if (Size > (*cd)->ds) imguFreeData(*cd);
    else if ((*cd)->data!=NULL) Size=(*cd)->ds; //allocate possibly more memory to be consistent with data memory (although data info is probably useless because of dimension change)

    imguFreeComplex(*cd);

#ifdef HAVE_FFTW3
    d = (fftw_complex *) fftw_malloc( Size * sizeof(fftw_complex ));
#else
    d = (fftw_complex *) (malloc(sizeof(fftw_complex)*Size));
#endif

    if( d==NULL ) {
        (*cd)->xs=(*cd)->ys=(*cd)->cs=(*cd)->ds=0;
        (*cd)->complex=NULL;
        return -1;
    }

    (*cd)->xs=xs;
    (*cd)->ys=ys;
    (*cd)->cs=cs;
    (*cd)->ds=Size;
    (*cd)->complex=d;
#ifdef HAVE_FFTW3
    (*cd)->fftplan = NULL;
    (*cd)->fftplan_target = NULL;
#endif
    //for (i=0;i<Size;i++)
    //{
    //    (*cd)->complex[i][0]=0;
    //    (*cd)->complex[i][1]=0;
    //}

    return 0;
}

/**
 * Sets complex data to 0.
 *
 * @param I Image to clear
 * @param component Indicates the component (0 for Real, 1 for Imaginary, -1 for both).
 *
 */
int imguClearComplex(imgu *I,unsigned char component)
{
    int sz,i;

    if( component>1) return -1;
    if (I==NULL || I->complex==NULL) return -1;

    sz=I->xs*I->ys*I->cs;

    for (i=0;i<sz;i++) I->complex[i][component]=0;

    return 0;
}


/**
 * Free complex data.
 * Only the data is freed. The image structure and any interger data is preserved.

 * @param[in,out] cd Address of an image pointer.
 */
void imguFreeComplex(imgu *I)
{
    if(I==NULL) return;
    if (I->complex!=NULL) free(I->complex);
    I->complex=NULL;
    if (I->data==NULL) I->xs=I->ys=I->cs=I->ds=0;
}

void imguFreeData(imgu *I)
{
    if(I==NULL) return;
    if (I->data!=NULL) free(I->data);
    I->data=NULL;
    if (I->complex==NULL) I->xs=I->ys=I->cs=I->ds=0;
}

int imguMapRangeComplex(imgu **dest, imgu *src,double min, double max,unsigned char component)
{
    int i,j,k,index;
    int start;
    int zs,ys,xs;
    double temp;
    double valmin,valmax;

    if (component!=0 && component!=1) return -1;

    if (dest==NULL || src==NULL) return -1;
    zs=src->cs;
    ys=src->ys;
    xs=src->xs;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,xs,ys,zs)) return -1;
    }

	// pour faire plaisir au compilateur
	valmin=valmax=0;

    start=1;
    for(i=0;i<zs;i++)
    {
        for(j=0;j<ys;j++)
        {
            for(k=0;k<xs;k++)
            {
                index=(j*xs+k)*zs+i;
                if (start)
                {
                    start=0;
                    valmin=src->complex[index][component];
                    valmax=src->complex[index][component];
                }
                else
                {
                    if (valmin>src->complex[index][component])
                    {
                        valmin=src->complex[index][component];
                    }
                    if (valmax<src->complex[index][component])
                    {
                        valmax=src->complex[index][component];
                        //printf("MAX %d %d %d %f\n",i,j,k,valmax);
                    }
                }
            }
        }
    }

    if (fabs(valmax-valmin)<COMPLEX_SMALL)
    {
        for(i=0;i<zs;i++)
        {
            for(j=0;j<ys;j++)
            {
                for(k=0;k<xs;k++)
                {
                    index=(j*xs+k)*zs+i;
                    (*dest)->complex[index][component]=(complex_t)(IMGU_MAXVAL);
                }
            }
        }
        return -1;
    }

    //fprintf(stdout,"%f %f %f\n",(valmax-valmin),valmax,valmin);

    for(i=0;i<zs;i++)
    {
        for(j=0;j<ys;j++)
        {
            for(k=0;k<xs;k++)
            {
                index=(j*xs+k)*zs+i;
                temp=((double)(src->complex[index][component]-valmin))/(valmax-valmin);
                (*dest)->complex[index][component]=(complex_t)(temp*(max-min)+min);
            }
        }
    }

    return 0;
}

/**
 * Creates a 1D gaussian filter of arbitrary size.
 *
 * @param [cd] Image that will contain the filter, stored in the complex table.
 * @param [dev] standard deviation of Gaussian function
 *
 **/
int imguGaussianFilter(imgu **cd,double dev)
{
    int i;
    double sum=0;
    double mean;
    int GSize;

    mean=0;
    sum=0;

    if (fabs(dev)<COMPLEX_SMALL)
    {
      imguAllocateComplex(cd,1,1,1);
      (*cd)->complex[0][0]=1.0;
      (*cd)->complex[0][1]=1.0;
      return 0;
    }

    GSize=(int)(sqrt(log(TOLERANCE)*(dev*dev)/(-0.5))+mean)+1;
    GSize++; //to avoid not satisfying the TOLERANCE condition because of integer rounding (in previous line)


    if (imguAllocateComplex(cd,2*GSize-1,1,1)) return -1;

    i=0;
    while(i<GSize)
    {
        (*cd)->complex[i+GSize-1][0]=exp(-0.5*(i-mean)*(i-mean)/(dev*dev));
        if (i>0) sum+=2*(*cd)->complex[i+GSize-1][0];
        else sum+=(*cd)->complex[i+GSize-1][0];
        i++;
    }

    if (sum)
    {
        for(i=0;i<GSize;i++)
        {
          (*cd)->complex[i+GSize-1][0]/=sum;
          //(*cd)->complex[i+GSize-1][1]=(*cd)->complex[i+GSize-1][0];
          (*cd)->complex[i+GSize-1][1]=0;
        }
    }

    for(i=1;i<GSize;i++)
    {
        (*cd)->complex[GSize-1-i][0]=(*cd)->complex[i+GSize-1][0];
        //(*cd)->complex[GSize-1-i][1]=(*cd)->complex[i+GSize-1][1];
        (*cd)->complex[GSize-1-i][1]=0;
    }

    return 0;
}

int imguGaussianWindow(imgu **cd,int size)
{
    double filter;
    int i;
    double mean;
    double dev;

    mean=size/2.0-0.5;
    dev=size/6.0;

    imguAllocateComplex(cd,size,1,1);

    for(i=0;i<size;i++)
    {
        filter=exp(-0.5*(i-mean)*(i-mean)/(dev*dev));
        if (filter<0) filter=0;
        (*cd)->complex[i][0]=filter;
        (*cd)->complex[i][1]=filter;
    }

    return 0;
}

int imguRaisedCosineWindow(imgu **cd,int size)
{
    double filter;
    int i;
    double mean;

    mean=size/2.0-0.5;

    imguAllocateComplex(cd,size,1,1);

    for(i=0;i<size;i++)
    {
        filter=(fabs(i-mean))/(size/2.0);
        if (filter>1.0) filter=1.0;
        filter=cos(filter*M_PI); //ranges between -1 and 1
        filter=(filter+1)/2;
        (*cd)->complex[i][0]=(complex_t)(filter);
        (*cd)->complex[i][1]=(complex_t)(filter);
    }

    return 0;
}

int imguRotateFilter(imgu **dest,imgu *src,double angle)
{
    int i,j,k,index;
    int xsize,ysize,zsize;
    double centerx,centery,centerz;
    double newcenterx,newcentery,newcenterz;
    matrix4 rotZ;
    vector4 sample,res;
    double alpha,beta;
    int ix,iy,iz;
    unsigned char realloc;
    imgu *srccpy;

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if (src->cs!=1 || src->ys!=1) return -1;
    xsize=src->xs;
    ysize=src->ys;
    zsize=src->cs;
    if((*dest)==src)
    {
        srccpy=NULL;
        imguCopy(&srccpy,src);
        realloc=1;
    }
    else
    {
        srccpy=src;
        realloc=0;
    }
    if(imguAllocateComplex(dest,xsize,xsize,zsize)) {if (realloc) {imguFree(&srccpy);}return(-1);}

    centerx=xsize/2.0-0.5;
    centery=ysize/2.0-0.5;
    centerz=zsize/2.0-0.5;

    mat4ZRotation(angle,rotZ);

    newcenterx=xsize/2.0-0.5;
    newcentery=xsize/2.0-0.5;
    newcenterz=zsize/2.0-0.5;

    index=0;
    for (i=0;i<zsize;i++)
    {
        for (j=0;j<ysize;j++)
        {
            for (k=0;k<xsize;k++)
            {
                sample[0]=k-centerx;
                sample[1]=j-centery;
                sample[2]=i-centerz;
                sample[3]=1.0;
                mat4MultiplyVector(rotZ, sample,res);
                res[0]+=newcenterx;
                res[1]+=newcentery;
                res[2]+=newcenterz;

                ix=(int)(res[0]);
                iy=(int)(res[1]);
                iz=(int)(res[2]);
                alpha=(res[0]-ix);
                beta=(res[1]-iy);

                (*dest)->complex[(iy*xsize+ix)*zsize+iz][0]+=(complex_t)(srccpy->complex[index][0]*((1-alpha)*(1-beta)));
                if (ix+1<xsize) (*dest)->complex[(iy*xsize+(ix+1))*zsize+iz][0]+=(complex_t)(srccpy->complex[index][0]*(alpha*(1-beta)));
                if (iy+1<xsize) (*dest)->complex[((iy+1)*xsize+ix)*zsize+iz][0]+=(complex_t)(srccpy->complex[index][0]*((1-alpha)*beta));
                if (ix+1<xsize && iy+1<xsize) (*dest)->complex[((iy+1)*xsize+(ix+1))*zsize+iz][0]+=(complex_t)(srccpy->complex[index][0]*(alpha*beta));

                index++;
            }
        }
    }

    if (realloc) imguFree(&srccpy);

    return 0;
}

int imguConvertFilterToFrequencyDomain(imgu **dest,imgu *src,int xs,int ys)
{
    int i,j;
    int fx,fy;
    double imean;

    if (dest==NULL || src==NULL) return -1;
    if (src->complex==NULL) return -1;
    if ((*dest)==src) return -1;
    if (xs<src->xs) return -1;
    if (ys<src->ys) return -1;
    if (src->cs!=1) return -1;

    imguAllocateComplex(dest,xs,ys,1);
    imguClearComplex((*dest),0);
    imguClearComplex((*dest),1);

    for(i=0;i<src->ys;i++)
    {
      fy=i-src->ys/2;
      if (fy<0) fy+=ys;
      for(j=0;j<src->xs;j++)
      {
        fx=j-src->xs/2;
        if (fx<0) fx+=xs;

        (*dest)->complex[fy*xs+fx][0]=src->complex[i*src->xs+j][0];
        (*dest)->complex[fy*xs+fx][1]=src->complex[i*src->xs+j][1];
      }
    }

    imguSubtractMean(dest,(*dest),-1,&imean);
    imguFFTForward(dest,(*dest));

    return 0;
}

int imguFirstDerivFilter(imgu **cd)
{
    imguAllocateComplex(cd,3,1,1);

    (*cd)->complex[0][0]=(complex_t)(-0.5);
    (*cd)->complex[1][0]=(complex_t)(0.0);
    (*cd)->complex[2][0]=(complex_t)(0.5);

    return 1;
}

int imguFirstDerivFilter2Pixels(imgu **cd)
{
    imguAllocateComplex(cd,2,1,1);

    (*cd)->complex[0][0]=(complex_t)(-1.0);
    (*cd)->complex[1][0]=(complex_t)(1.0);

    return 1;
}

int imguSecondDerivFilter(imgu **cd)
{
    imguAllocateComplex(cd,3,1,1);

    (*cd)->complex[0][0]=(complex_t)(1.0);
    (*cd)->complex[1][0]=(complex_t)(-2.0);
    (*cd)->complex[2][0]=(complex_t)(1.0);

    return 1;
}

int imguAvgFilter(imgu **cd)
{
    imguAllocateComplex(cd,3,3,1);

    (*cd)->complex[0][0]=(complex_t)(1.0/12);
    (*cd)->complex[1][0]=(complex_t)(1.0/6);
    (*cd)->complex[2][0]=(complex_t)(1.0/12);
    (*cd)->complex[3][0]=(complex_t)(1.0/6);
    (*cd)->complex[4][0]=(complex_t)(0.0);
    (*cd)->complex[5][0]=(complex_t)(1.0/6);
    (*cd)->complex[6][0]=(complex_t)(1.0/12);
    (*cd)->complex[7][0]=(complex_t)(1.0/6);
    (*cd)->complex[8][0]=(complex_t)(1.0/12);

    return 1;
}

int imguAvgFilter2Pixels(imgu **cd)
{
    imguAllocateComplex(cd,2,1,1);

    (*cd)->complex[0][0]=(complex_t)(0.5);
    (*cd)->complex[1][0]=(complex_t)(0.5);

    return 1;
}

static int imguConvolveHelper(imgu **dest,imgu *src,imgu *mask,int offset,int rem_margin,int mode)
{
    int i,j,k,l,m,n,index;
    int xsize,ysize,zsize;
    int maskx,masky,maskz;
    int centerx,centery,centerz;
    double tempx,tempy;
    double filtersumx;
    double filtersumy;
    int fcount;
    unsigned char realloc;
    imgu *srccpy;

    if (mode!=DATA_MODE && mode!=COMPLEX_MODE) return -1;

    if (dest==NULL || src==NULL || mask==NULL || mask->complex==NULL) return -1;
    if (mode==DATA_MODE && src->data==NULL) return -1;
    if (mode==COMPLEX_MODE && src->complex==NULL) return -1;

    xsize=src->xs;
    ysize=src->ys;
    zsize=src->cs;
    if((*dest)==src)
    {
        srccpy=NULL;
        imguCopy(&srccpy,src);
        realloc=1;
    }
    else
    {
        srccpy=src;
        realloc=0;
    }
    if (mode==COMPLEX_MODE) if(imguAllocateComplex(dest,xsize,ysize,zsize)) {if (realloc) {imguFree(&srccpy);}return(-1);}
    if (mode==DATA_MODE) if(imguAllocate(dest,xsize,ysize,zsize)) {if (realloc) {imguFree(&srccpy);}return(-1);}

    maskx=mask->xs;
    masky=mask->ys;
    maskz=mask->cs;

    centerx=maskx/2;
    if (maskx%2==0) centerx--;
    centery=masky/2;
    if (masky%2==0) centery--;
    centerz=maskz/2;
    if (maskz%2==0) centerz--;

    if (centerx<0) return -1;
    if (centery<0) return -1;
    if (centerz<0) return -1;

    if (xsize<maskx || ysize<masky || zsize<maskz)
    {
        fprintf(stderr,"Filter is too big! (%d %d %d)\n",maskx,masky,maskz);
        fflush(stderr);
        return -1;
    }

    for(i=0;i<zsize;i++)
    {
        for(j=0;j<ysize;j++)
        {
            for(k=0;k<xsize;k++)
            {
                tempx=0;
                tempy=0;
                index=0;
                filtersumx=0;
                filtersumy=0;
                fcount=0;
                for(l=0;l<maskz;l++)
                {
                    for(m=0;m<masky;m++)
                    {
                        for(n=0;n<maskx;n++)
                        {
                            if (i+l-centerz>=0 && j+m-centery>=0 && k+n-centerx>=0 &&
                                    i+l-centerz<zsize && j+m-centery<ysize && k+n-centerx<xsize)
                            {
                                if (mode==COMPLEX_MODE)
                                {
                                  tempx+=mask->complex[index][0]*srccpy->complex[((j+m-centery)*xsize+(k+n-centerx))*zsize+(i+l-centerz)][0];
                                  tempy+=mask->complex[index][1]*srccpy->complex[((j+m-centery)*xsize+(k+n-centerx))*zsize+(i+l-centerz)][1];
                                }
                                else tempx+=mask->complex[index][0]*srccpy->data[((j+m-centery)*xsize+(k+n-centerx))*zsize+(i+l-centerz)];
                                filtersumx+=mask->complex[index][0];
                                filtersumy+=mask->complex[index][1];
                                fcount++;
                            }
                            index++;
                        }
                    }
                }
                if (fabs(filtersumx)>COMPLEX_SMALL) //this should be worked on, assumes filter sums to 1: normalize if filter exceeds boundary of image
                {
                    tempx/=filtersumx;
                }
                if (fabs(filtersumy)>COMPLEX_SMALL)
                {
                    tempy/=filtersumy;
                }
                index=(j*xsize+k)*zsize+i;
                if (rem_margin!=CONVOLVE_BLANK_MARGIN || fcount==maskz*masky*maskx)
                {
                    if (mode==COMPLEX_MODE)
                    {
                      (*dest)->complex[index][0]=(complex_t)(tempx);
                      (*dest)->complex[index][1]=(complex_t)(tempy);
                    }
                    else
                    {
                      tempx+=offset;
                      if (tempx<0) tempx=0;
                      if (tempx>=(double)(IMGU_MAXVAL)) tempx=(double)(IMGU_MAXVAL);
                      (*dest)->data[index]=(pix_t)(tempx);
                    }
                }
                else //blank margin
                {
                    if (mode==COMPLEX_MODE)
                    {
                      (*dest)->complex[index][0]=0;
                      (*dest)->complex[index][1]=0;
                    }
                    else
                    {
                      (*dest)->data[index]=0;
                    }
                }
            }
        }
    }

    if (rem_margin==CONVOLVE_REMOVE_MARGIN)
    {
        imguExtractRectangle(dest,(*dest),centerx,centery,xsize-maskx+1,ysize-masky+1);
        imguSelectChannels(dest,(*dest),centerz,zsize-maskz+1);
    }

    if (realloc) imguFree(&srccpy);

    return 0;
}

int imguConvolve(imgu **dest,imgu *src,imgu *mask,int offset,int rem_margin)
{
  return imguConvolveHelper(dest,src,mask,offset,rem_margin,DATA_MODE);
}

int imguConvolveComplex(imgu **dest,imgu *src,imgu *mask,int rem_margin)
{
  return imguConvolveHelper(dest,src,mask,0,rem_margin,COMPLEX_MODE);
}

int imguBlur(imgu **Iblur,imgu *I,double devx,double devy,int rem_margin)
{
    int i,j,k,l,index;
    int xsize,ysize,csize;
    imgu **imchannels;
    imgu *gaussx,*gaussy;
    imgu *gaussx_freq,*gaussy_freq;

    if (I==NULL || I->data==NULL) return -1;
    if (Iblur==NULL) return -1;

    csize=I->cs;

    gaussx=NULL;
    gaussy=NULL;
    gaussx_freq=NULL;
    gaussy_freq=NULL;
    imguGaussianFilter(&gaussx,devx);
    imguGaussianFilter(&gaussy,devy);
    //imguConvertFilterToFrequencyDomain(&gaussx_freq,gaussx,I->xs,I->ys);
    //imguConvertFilterToFrequencyDomain(&gaussy_freq,gaussy,I->xs,I->ys);
    gaussy->ys=gaussy->xs;
    gaussy->xs=1;

    imchannels=(imgu **)(malloc(sizeof(imgu *)*I->cs));
    for (i=0;i<csize;i++)
    {
        imchannels[i]=NULL;
        imguSelectChannels(&(imchannels[i]),I,i,1);
        imguConvolve(&(imchannels[i]),imchannels[i],gaussx,0,rem_margin);
        imguConvolve(&(imchannels[i]),imchannels[i],gaussy,0,rem_margin);
        //attempting frequency domain blur
        /*imean=0;
        imguSubtractMean(&imchannels[i],imchannels[i],-1,&imean);
        imguFFTForward(&imchannels[i],imchannels[i]);
        imguWindowComplex(&imchannels[i],imchannels[i],gaussx_freq);
        imguWindowComplex(&imchannels[i],imchannels[i],gaussy_freq);
        imguFFTInverse(&imchannels[i],imchannels[i]);
        for(j=0;imchannels[i]->xs*imchannels[i]->ys;j++) imchannels[i]->complex[j][0]+=imean;
        imguConvertFromComplexComponent(&imchannels[i],imchannels[i],0,COMPLEX_AS_IS);*/
    }

    xsize=imchannels[0]->xs;
    ysize=imchannels[0]->ys;

    imguAllocate(Iblur,xsize,ysize,csize);

    index=0;
    for (j=0;j<ysize;j++)
    {
        for (k=0;k<xsize;k++)
        {
            for (l=0;l<csize;l++)
            {
                (*Iblur)->data[index]=imchannels[l]->data[j*xsize+k];
                index++;
            }
        }
    }

    imguFree(&gaussx);
    imguFree(&gaussy);
    imguFree(&gaussx_freq);
    imguFree(&gaussy_freq);
    for (i=0;i<csize;i++)
    {
        imguFree(&(imchannels[i]));
    }
    free(imchannels);

    return 0;
}

static int imguWindowHelper(imgu **dest,imgu *src,imgu *window,int mode)
{
    int i,j,k,wi;
    int index;
    double temp;
    int xsize,ysize,zsize;

    if (mode!=DATA_MODE && mode!=COMPLEX_MODE) return -1;

    if (dest==NULL || src==NULL || window==NULL || window->complex==NULL) return -1;
    if (mode==DATA_MODE && src->data==NULL) return -1;
    if (mode==COMPLEX_MODE && src->complex==NULL) return -1;
    if ((*dest)==window || src==window) return -1;
    if ((*dest)!=src)
    {
      if (mode==DATA_MODE) if (imguAllocate(dest,src->xs,src->ys,src->cs)) return -1;
      if (mode==COMPLEX_MODE) if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }

    xsize=src->xs;
    ysize=src->ys;
    zsize=src->cs;

    index=0;
    for(j=0;j<ysize;j++)
    {
        for(k=0;k<xsize;k++)
        {
            for(i=0;i<zsize;i++)
            {
                wi=((j%window->ys)*window->xs+(k%window->xs))*window->cs+(i%window->cs);
                if (mode==COMPLEX_MODE)
                {
                  temp=src->complex[index][1]*window->complex[wi][0]+src->complex[index][0]*window->complex[wi][1];
                  (*dest)->complex[index][0]=src->complex[index][0]*window->complex[wi][0]-src->complex[index][1]*window->complex[wi][1];
                  (*dest)->complex[index][1]=temp;
                }
                else
                {
                  (*dest)->data[index]=src->data[index]*window->complex[wi][0];
                }
                index++;
            }
        }
    }

    return 0;
}

int imguWindow(imgu **dest,imgu *src,imgu *window)
{
  return imguWindowHelper(dest,src,window,DATA_MODE);
}

int imguWindowComplex(imgu **dest,imgu *src,imgu *window)
{
  return imguWindowHelper(dest,src,window,COMPLEX_MODE);
}


int imguFFTInitThreads( int nbThreads )
{
#ifdef HAVE_FFTW3

    int k = fftw_init_threads();
    fftw_plan_with_nthreads( nbThreads );

    if (!k) return -1;

#endif
    return 0;
}


void imguFFTUninit() {
#ifdef HAVE_FFTW3
    fftw_cleanup_threads();
#endif
}


int imguFFTPlan(imgu **dest,imgu *src, int isign) {
#ifdef HAVE_FFTW3
    //printf("imguFFTPlan planning...\n");
    int fftwsign;

    if (isign > 0) fftwsign = FFTW_FORWARD;
    else fftwsign = FFTW_BACKWARD;

    //printf("imguFFTPlan xs=%i; ys=%i; src=%p; dst=%p\n", src->ys, src->xs,
    //       src->complex, (*dest)->complex);
    if (src->cs == 1) {
        src->fftplan = fftw_plan_dft_2d( src->ys, src->xs,
                                         src->complex, (*dest)->complex,
                                         fftwsign, FFTW_MEASURE );
    } else {
        src->fftplan = fftw_plan_dft_3d( src->cs, src->ys, src->xs,
                                         src->complex, (*dest)->complex,
                                         fftwsign, FFTW_MEASURE );
    }
    if (!src->fftplan) return -1;
    //printf("imguFFTPlan plan done\n");
    src->fftplan_target = (*dest)->complex;
#endif
    return 0;
}

static int imguFFT(imgu **dest,imgu *src,int isign, double scaling)
{

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }

#ifdef HAVE_FFTW3

    if ( !src->fftplan || src->fftplan_target != (*dest)->complex ) {

        int fftwsign;
        if (isign > 0) fftwsign = FFTW_FORWARD;
        else fftwsign = FFTW_BACKWARD;

        if (src->cs == 1) {
            src->fftplan = fftw_plan_dft_2d( src->ys, src->xs,
                                             src->complex, (*dest)->complex,
                                             fftwsign, FFTW_ESTIMATE );
        } else {
            src->fftplan = fftw_plan_dft_3d( src->cs, src->ys, src->xs,
                                             src->complex, (*dest)->complex,
                                             fftwsign, FFTW_ESTIMATE );
        }
        src->fftplan_target = (*dest)->complex;
    }
    if (!src->fftplan) return -1;
    fftw_execute(src->fftplan);
#else
    int i;
    int dims[3],size;
    double *Re,*Im;

    dims[2]=src->ys;
    dims[1]=src->xs;
    dims[0]=src->cs;

    Re=(double *)(malloc(sizeof(double)*dims[0]*dims[1]*dims[2]));
    Im=(double *)(malloc(sizeof(double)*dims[0]*dims[1]*dims[2]));

    size=dims[0]*dims[1]*dims[2];
    for (i=0;i<size;i++)
    {
        Re[i]=src->complex[i][0];
        Im[i]=src->complex[i][1];
    }

    fftn(3, dims, Re, Im, isign, scaling);

    for (i=0;i<size;i++)
    {
      (*dest)->complex[i][0]=(complex_t)(Re[i]);
      (*dest)->complex[i][1]=(complex_t)(Im[i]);
    }

    fft_free();
    free(Re);
    free(Im);

#endif

    return 0;
}

int imguFFTForward(imgu **dest,imgu *src)
{
    int i,size;
    if (imguFFT(dest,src,1,-1.0)) return -1;

    size=(*dest)->xs*(*dest)->ys*(*dest)->cs;
    for (i=0;i<size;i++)
    {
      (*dest)->complex[i][0]*=size;
      (*dest)->complex[i][1]*=size;
    }

    return 0;
}

int imguFFTInverse(imgu **dest,imgu *src)
{
    //int size;
    if (imguFFT(dest,src,-1,-1.0)) return -1;

    /*size=(*dest)->xs*(*dest)->ys*(*dest)->cs;
    for (i=0;i<size;i++)
    {
      (*dest)->complex[i][0]*=size;
      (*dest)->complex[i][1]*=size;
    }*/

    return 0;
}

int imguRemoveDC(imgu **dest,imgu *src)
{
    if (imguCopy(dest,src)) return -1;

    (*dest)->complex[0][0]=0;
    (*dest)->complex[0][1]=0;

    return 0;
}

int imguAdjustQuadrants(imgu **dest,imgu *src)
{
    int i,j,k,index,ii;
    int fx,fy,ft;
    double tmp[2];
    int hxs,hys,hcs;

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;

    if ((*dest)!=src)
    {
      if (imguAllocate(dest,src->xs,src->ys,src->cs)) return -1;
    }

    hxs=src->xs/2;
    if (hxs==0) hxs++;
    hys=src->ys/2;
    if (hys==0) hys++;
    hcs=src->cs/2;
    if (hcs==0) hcs++;

    for (k=0;k<hxs;k++)
    {
        fx=k+src->xs/2;
        for (i=0;i<hcs;i++)
        {
            ft=i+src->cs/2;
            for (j=0;j<hys;j++)
            {
                fy=j+src->ys/2;

                index=(j*src->xs+k)*src->cs+i;
                ii=(fy*src->xs+fx)*src->cs+ft;
                tmp[0]=(*dest)->complex[ii][0];
                tmp[1]=(*dest)->complex[ii][1];
                (*dest)->complex[ii][0]=src->complex[index][0];
                (*dest)->complex[ii][1]=src->complex[index][1];
                src->complex[index][0]=tmp[0];
                src->complex[index][1]=tmp[1];
            }
            for (j=hys;j<src->ys;j++)
            {
                fy=j-src->ys/2;

                index=(j*src->xs+k)*src->cs+i;
                ii=(fy*src->xs+fx)*src->cs+ft;
                tmp[0]=(*dest)->complex[ii][0];
                tmp[1]=(*dest)->complex[ii][1];
                (*dest)->complex[ii][0]=src->complex[index][0];
                (*dest)->complex[ii][1]=src->complex[index][1];
                src->complex[index][0]=tmp[0];
                src->complex[index][1]=tmp[1];
            }
        }
    }

    return 0;
}

int imguSpatialHardBandPass(imgu **dest,imgu *src,double radiusmin,double radiusmax)
{
    int i,j,k;
    int xsize,ysize,csize;
    int index;
    double fx,fy;
    double ft;

    if (dest==NULL || src==NULL) return -1;

    csize=src->cs;
    ysize=src->ys;
    xsize=src->xs;

    if ((*dest)!=src)
    {
      if (imguAllocate(dest,xsize,ysize,csize)) return -1;
    }

    index=0;
    for (j=0;j<ysize;j++)
    {
        if (j>ysize/2) fy=j-ysize;
        else fy=j;

        for (k=0;k<xsize;k++)
        {
            if (k>xsize/2) fx=k-xsize;
            else fx=k;

            for (i=0;i<csize;i++)
            {
                //if (i>csize/2) ft=i-csize;
                //else ft=i;

                if (sqrt(fy*fy+fx*fx)>radiusmax)
                {
                    (*dest)->complex[index][0]=0;
                    (*dest)->complex[index][1]=0;
                }
                if (sqrt(fy*fy+fx*fx)<radiusmin)
                {
                    (*dest)->complex[index][0]=0;
                    (*dest)->complex[index][1]=0;
                }
                index++;
            }
        }
    }

    return 0;
}



int imguSubtractMean(imgu **dest,imgu *src, unsigned char key,double *mean)
{
    int i;
    int zeros;
    int zsize,xsize,ysize,Sz;

    if (dest==NULL || src==NULL || src->complex==NULL || mean==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }

    zsize=src->cs;
    ysize=src->ys;
    xsize=src->xs;

    zeros=0;

    (*mean)=0;

    Sz=xsize*ysize*zsize;
    for (i=0;i<Sz;i++)
    {
        (*mean)+=src->complex[i][0];
        if (fabs(src->complex[i][0]-key)<COMPLEX_SMALL) zeros++;
    }

    if (Sz==zeros) return -1;

    (*mean)/=(Sz-zeros);

    for (i=0;i<Sz;i++)
    {
        if (fabs(src->complex[i][0]-key)<COMPLEX_SMALL) (*dest)->complex[i][0]=src->complex[i][0];
        else (*dest)->complex[i][0]=(complex_t)(src->complex[i][0]-(*mean));
    }

    return 0;
}

int imguLog(imgu **dest,imgu *src,unsigned char component)
{
    int i;
    int size;

    if (component>1) return -1;

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }

    size=src->xs*src->ys*src->cs;

    for (i=0;i<size;i++)
    {
        (*dest)->complex[i][component]=(complex_t)(log(1.0+src->complex[i][component]));
    }

    return 0;
}

int imguAbsolute(imgu **dest,imgu *src,unsigned char component)
{
    int i;
    int size;

    if (component>1) return -1;

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }

    size=src->xs*src->ys*src->cs;

    for (i=0;i<size;i++)
    {
        (*dest)->complex[i][component]=(complex_t)(fabs(src->complex[i][component]));
    }

    return 0;
}

int imguMagnitude(imgu **dest,imgu *src)
{
    int i;
    int size;

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }

    size=src->xs*src->ys*src->cs;

    for (i=0;i<size;i++)
    {
        (*dest)->complex[i][0]=(complex_t)(sqrt(src->complex[i][0]*src->complex[i][0]+src->complex[i][1]*src->complex[i][1]));
        (*dest)->complex[i][1]=0.0;
    }

    return 0;
}

int imguPower(imgu **dest,imgu *src)
{
    int i;
    int size;

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }

    size=src->xs*src->ys*src->cs;

    for (i=0;i<size;i++)
    {
        (*dest)->complex[i][0]=src->complex[i][0]*src->complex[i][0]+src->complex[i][1]*src->complex[i][1];
        (*dest)->complex[i][1]=0.0;
    }

    return 0;
}

static double magn(double re,double im)
{
    return sqrt(re*re+im*im);
}

static double phase(double re,double im)
{
    double phase;

    phase=atan2(im,re);
    if (phase<0) phase+=2*M_PI;

    return phase;
}

#ifdef UNUSED
static double real(double magn,double phase)
{
    return cos(phase)*magn;
}

static double imag(double magn,double phase)
{
    return sin(phase)*magn;
}
#endif


int imguMagnPhase(imgu **dest,imgu *src)
{
    int i,size;
    double re,im;

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }

    size=src->xs*src->ys*src->cs;
    for (i=0;i<size;i++)
    {
        re=src->complex[i][0];
        im=src->complex[i][1];
        (*dest)->complex[i][0]=(complex_t)(magn(re,im));
        (*dest)->complex[i][1]=(complex_t)(phase(re,im));
    }

    return 0;
}

int imguRealImag(imgu **dest,imgu *src)
{
    int i,size;
    double magn,phase;

    if (dest==NULL || src==NULL || src->complex==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocateComplex(dest,src->xs,src->ys,src->cs)) return -1;
    }
    size=src->xs*src->ys*src->cs;
    for (i=0;i<size;i++)
    {
        magn=src->complex[i][0];
        phase=src->complex[i][1];
        //(*dest)->complex[i][0]=(complex_t)(real(magn,phase));
        //(*dest)->complex[i][1]=(complex_t)(imag(magn,phase));
        (*dest)->complex[i][0]=(complex_t)(cos(phase)*magn);
        (*dest)->complex[i][1]=(complex_t)(sin(phase)*magn);
    }

    return 0;
}


