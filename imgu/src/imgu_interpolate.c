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

/**
 * Interpolation to the closest pixel
 *
 * Interpolates a pixel value from the closest pixel to position @c (x,y).
 * @c val is undefined if @c (x,y) is outside the image.
 *
 * @b Usage:
 * @code
 * imgu *IA;
 * double xf,yf,color[4]; // 4 is the maximum, but can use IA->cs
 * ...
 * if( imguInterpolateClosest(IA,xf,yf,color) ) { printf("out of image\n"); }
 * ...
 * printf("pixel (%f,%f) has color (",xf,yf);
 * for(i=0;i<IA->cs;i++) printf("%f,",color[i]);
 * printf(")\n");
 * @endcode
 *
 * @example imguInterpolate.c
 * @see imguInterpolateBilinear() imguInterpolateBicubic()
 *
 * @param[in] I Image
 * @param[in] x position X
 * @param[in] y position Y
 * @param[out] val Array (of size I->cs) of interpolated values (as doubles)
 * @return 0 if ok, -1 if outside the image
 *
 *
 */
int imguInterpolateClosest(imgu *I,double x,double y,double *val)
{
    int ix,iy,c;

    if (I==NULL || val==NULL) return -1;
    if (I->data==NULL) return -1;

    if (imguCheck(I,x,y)!=0) return -1;

    //center of first pixel is 0.5, 0.5
    //values in [0,1) correspond to first pixel
    ix=(int)(x);
    iy=(int)(y);

    for(c=0;c<I->cs;c++) val[c]=I->data[(iy*I->xs+ix)*I->cs+c];

    return(0);
}


int imguInterpolateClosestComplex(imgu *I,double x,double y,double *val,unsigned char component)
{
    int ix,iy,c;

    if (component>1) return -1;
    if (I==NULL || val==NULL) return -1;
    if (I->complex==NULL) return -1;

    if (imguCheck(I,x,y)!=0) return -1;

    //same as imguInterpolateClosest
    ix=(int)(x);
    iy=(int)(y);

    for(c=0;c<I->cs;c++) val[c]=I->complex[(iy*I->xs+ix)*I->cs+c][component];

    return(0);
}

/**
 * Bilinear interpolation 
 *
 * Interpolates a pixel value at an arbitrary location @c (x,y), using bilinear interpolation.
 * @c val is undefined if @c (x,y) is outside the image.
 *
 * @example imguInterpolate.c
 * @see imguInterpoleClosest() imguInterpolateBicubic()
 *
 * @param[in] I Image
 * @param[in] x position X
 * @param[in] y position Y
 * @param[out] val Array (of size I->cs) of interpolated values (as doubles)
 * @return 0 if ok, -1 if outside the image
 */
int imguInterpolateBilinear(imgu *I,double x,double y,double *val)
{
    int ix,iy,ch;
    int P;
    double dx,dy;
    double a,b,c,d;

    if (I==NULL || val==NULL) return -1;
    if (I->data==NULL) return -1;

    if (imguCheck(I,x,y)!=0) return -1;
    if (x>I->xs-1) return -1;
    if (y>I->ys-1) return -1;

    //value of 0.5,0.5 should return value intensity of pixel 0
    x-=0.5;
    y-=0.5;
    if (x<0) x=0;
    if (y<0) y=0;

    // default value
    for(ch=0;ch<I->cs;ch++) val[ch]=0.0;

    ix=(int)floor(x);
    iy=(int)floor(y);
    dx=x-(double)ix;
    dy=y-(double)iy;
    /* Check for noise... */
    if( dx<1e-8 ) dx=0.0;
    if( dy<1e-8 ) dy=0.0;

    P=(iy*I->xs+ix)*I->cs;

    for(ch=0;ch<I->cs;ch++) {
        a=I->data[P+ch]; // (x,y)
        if(dx==0.0) b=a;
        else
        {
          b=I->data[P+I->cs+ch];  // (x+1,y)
        }
        if(dy==0.0) c=a;
        else
        {
          c=I->data[P+I->xs*I->cs+ch]; // (x,y+1)
        }
        if(dx==0.0) d=c;
        else if(dy==0.0) d=b;
        else
        {
          d=I->data[P+(I->xs+1)*I->cs+ch]; // (x+1,y+1)
        }

        /* bi-linear interpolation */
        a+=(c-a)*dy;
        b+=(d-b)*dy;
        a+=(b-a)*dx;
        if (a<0) a=0;
        if (a>IMGU_MAXVAL) a=(double)(IMGU_MAXVAL);
        val[ch]=a;        
    }
    return(0);
}

int imguInterpolateBilinearComplex(imgu *I,double x,double y,double *val,unsigned char component)
{
    int ix,iy,ch;
    int P;
    double dx,dy;
    double a,b,c,d;

    if (I==NULL || val==NULL) return -1;
    if (component>1) return -1;
    if (I->complex==NULL) return -1;

    if (imguCheck(I,x,y)!=0) return -1;
    if (x>I->xs-1) return -1;
    if (y>I->ys-1) return -1;

    //value of 0.5,0.5 should return value intensity of pixel 0
    x-=0.5;
    y-=0.5;
    if (x<0) x=0;
    if (y<0) y=0;

    // default value
    for(ch=0;ch<I->cs;ch++) val[ch]=0.0;

    ix=(int)floor(x);
    iy=(int)floor(y);
    dx=x-(double)ix;
    dy=y-(double)iy;
    /* Check for noise... */
    if( dx<1e-8 ) dx=0.0;
    if( dy<1e-8 ) dy=0.0;

    P=(iy*I->xs+ix)*I->cs;

    for(ch=0;ch<I->cs;ch++) {
        a=I->complex[P+ch][component];
        if(dx==0.0) b=a;
        else
        {
          b=I->complex[P+I->cs+ch][component];
        }
        if(dy==0.0) c=a;
        else
        {
          c=I->complex[P+I->xs*I->cs+ch][component];
        }
        if(dx==0.0) d=c;
        else if(dy==0.0) d=b;
        else
        {
          d=I->complex[P+(I->xs+1)*I->cs+ch][component];
        }

        /* bi-linear interpolation */
        a+=(c-a)*dy;
        b+=(d-b)*dy;
        a+=(b-a)*dx;
        if (a<0) a=0;
        if (a>IMGU_MAXVAL) a=(double)(IMGU_MAXVAL);
        val[ch]=a;
    }
    return(0);
}

/**
 * return the cubic parameters a,b,c,d (param[0..3]) from 4 points
 * 
 * ASSUME x[0..3]=[-1,0,1,2] and f(x) = f[0..3]
 */
static void SimpleCubicParam(double *f,double *param)
{
    param[0]=(-f[0] + 3*f[1] - 3*f[2] + f[3])/6;
    param[1]=(f[0] - 2*f[1] + f[2])/2;
    param[2]=(-2*f[0] - 3*f[1] + 6*f[2] - f[3])/6;
    param[3]=f[1];
}

/**
 * Evaluate a x^3+b x^2+c x+d with {a,b,c,d}=q[0..3] 
 */
static double EvalCube(double *q,double x)
{
    return((((q[0]*x)+q[1])*x+q[2])*x+q[3]);
}



#ifdef UNUSED
/**
 * return the cubic parameters a,b,c,d (param[0..3]) from 4 points
 *
 * x[0..3] and f(x) = f[0..3]
 */
static void CubicParam(double *x,double *f,double *param)
{
    double x11,x12,x13,x21,x22,x23,x31,x32,x33,x41,x42,x43;
    double m21,m22,m31,m32,m41,m42;
    double d2,d3,d4,p2,p3,p4,q3,q4,r3,r4,s3,s4,qs34,qr34;
    double a,b,c,d;
    x11=x[0];
    x12=x11*x11;
    x13=x11*x12;
    x21=x[1];
    x22=x21*x21;
    x23=x21*x22;
    x31=x[2];
    x32=x31*x31;
    x33=x31*x32;
    x41=x[3];
    x42=x41*x41;
    x43=x41*x42;

    m21=(x13*x21 - x11*x23);
    m22=(x13*x22 - x12*x23);
    m31=(x13*x31 - x11*x33);
    m32=(x13*x32 - x12*x33);
    m41=(x13*x41 - x11*x43);
    m42=(x13*x42 - x12*x43);

    d2=(x13 - x23);
    d3=(x13 - x33);
    d4=(x13 - x43);

    p2=(x23*f[0] - x13*f[1]);
    p3=(x33*f[0] - x13*f[2]);
    p4=(x43*f[0] - x13*f[3]);

    q3=(m22*m31 - m21*m32);
    q4=(m22*m41 - m21*m42);

    r3=(d3*m22 - d2*m32);
    r4=(d4*m22 - d2*m42);

    s3=(m22*p3 - m32*p2);
    s4=(m22*p4 - m42*p2);

    qs34=(q3*s4 - q4*s3);
    qr34=(q3*r4 - q4*r3);

    a=(m22 * qs34 * q3 - m22 * qs34 * r3 * x11 + m22 * qr34 * s3 * x11
            + p2 * qr34 * q3 * x12 - d2 * qs34 * q3 * x12
            + m21 * qs34 * r3 * x12 - m21 * qr34 * s3 * x12
            + m22 * qr34 * q3 * f[0]) / (m22 * qr34 * q3 * x13);

    b=(d2 * qs34 * q3 - m21 * qs34 * r3 + m21 * qr34 * s3
            - p2 * qr34 * q3) / (m22 * qr34 * q3);

    c=(qs34 * r3 - qr34 * s3) / (qr34 * q3);

    d= - qs34 / qr34;

    param[0]=a;
    param[1]=b;
    param[2]=c;
    param[3]=d;
}
#endif


/**
 * Bicubic interpolation 
 *
 * Interpolates a pixel value at an arbitrary location @c (x,y), using bicubic interpolation.
 * @c val is undefined if @c (x,y) is outside the image.
 *
 * @example imguInterpolate.c
 * @see imguInterpoleClosest() imguInterpolateBilinear()
 *
 * @param[in] I Image
 * @param[in] x position X
 * @param[in] y position Y
 * @param[out] val Array (of size I->cs) of interpolated values (as doubles)
 * @return 0 if ok, -1 if outside the image
 */
int imguInterpolateBicubic(imgu *I,double x,double y,double *val)
{
    int ix,iy,c;
    int P;
    double dx,dy;
    double f[4],q[4],v[4];

    if (I==NULL || val==NULL) return -1;
    if (I->data==NULL) return -1;

    if (imguCheck(I,x,y)!=0) return -1;
    if (x>I->xs-1) return -1;
    if (y>I->ys-1) return -1;

    //value of 0.5,0.5 should return value intensity of pixel 0
    x-=0.5;
    y-=0.5;
    if (x<0) x=0;
    if (y<0) y=0;

    // default value
    for(c=0;c<I->cs;c++) val[c]=0.0;

    ix=(int)floor(x);
    iy=(int)floor(y);
    dx=x-(double)ix;
    dy=y-(double)iy;

    if( ix-1<0 || iy-1<0 ) return(-1);
    if( ix+2>=I->xs ) return(-1);
    if( iy+2>=I->ys ) return(-1);

    for(c=0;c<I->cs;c++) {

        /* First horizontal row of four points */
        P=((iy-1)*I->xs+(ix-1))*I->cs+c;
        f[0]=I->data[P];P+=I->cs;
        f[1]=I->data[P];P+=I->cs;
        f[2]=I->data[P];P+=I->cs;
        f[3]=I->data[P];
        if( P>=I->xs*I->ys*I->cs ) { printf("INTERPOLE: ixy=(%d,%d) dxy=(%f,%f) c=%d sz=(%d,%d,%d)\n",ix,iy,dx,dy,c,I->xs,I->ys,I->cs); }
        SimpleCubicParam(f,q);
        v[0]=EvalCube(q,dx);

        /* Second horizontal row of four points */
        P=((iy)*I->xs+(ix-1))*I->cs+c;
        f[0]=I->data[P];P+=I->cs;
        f[1]=I->data[P];P+=I->cs;
        f[2]=I->data[P];P+=I->cs;
        f[3]=I->data[P];
        if( P>=I->xs*I->ys*I->cs ) { printf("INTERPOLE: ixy=(%d,%d) dxy=(%f,%f) c=%d sz=(%d,%d,%d)\n",ix,iy,dx,dy,c,I->xs,I->ys,I->cs); }
        SimpleCubicParam(f,q);
        v[1]=EvalCube(q,dx);

        /* Third horizontal row of four points */
        P=((iy+1)*I->xs+(ix-1))*I->cs+c;
        f[0]=I->data[P];P+=I->cs;
        f[1]=I->data[P];P+=I->cs;
        f[2]=I->data[P];P+=I->cs;
        f[3]=I->data[P];
        if( P>=I->xs*I->ys*I->cs ) { printf("INTERPOLE: ixy=(%d,%d) dxy=(%f,%f) c=%d sz=(%d,%d,%d)\n",ix,iy,dx,dy,c,I->xs,I->ys,I->cs); }
        SimpleCubicParam(f,q);
        v[2]=EvalCube(q,dx);

        /* Fourth horizontal row of four points */
        P=((iy+2)*I->xs+(ix-1))*I->cs+c;
        f[0]=I->data[P];P+=I->cs;
        f[1]=I->data[P];P+=I->cs;
        f[2]=I->data[P];P+=I->cs;
        f[3]=I->data[P];
        if( P>=I->xs*I->ys*I->cs ) { printf("INTERPOLE: ixy=(%d,%d) dxy=(%f,%f) c=%d sz=(%d,%d,%d)\n",ix,iy,dx,dy,c,I->xs,I->ys,I->cs); }
        SimpleCubicParam(f,q);
        v[3]=EvalCube(q,dx);


        /* Extract final value from row v */
        SimpleCubicParam(v,q);
        val[c]=EvalCube(q,dy);

        if (val[c]<0) val[c]=0;
        if (val[c]>IMGU_MAXVAL) val[c]=(double)(IMGU_MAXVAL);
    }

    return(0);
}

int imguInterpolateBicubicComplex(imgu *I,double x,double y,double *val,unsigned char component)
{
    int ix,iy,c;
    int P;
    double dx,dy;
    double f[4],q[4],v[4];

    if (I==NULL || val==NULL) return -1;
    if (component>1) return -1;
    if (I->complex==NULL) return -1;

    if (imguCheck(I,x,y)!=0) return -1;
    if (x>I->xs-1) return -1;
    if (y>I->ys-1) return -1;

    //value of 0.5,0.5 should return value intensity of pixel 0
    x-=0.5;
    y-=0.5;
    if (x<0) x=0;
    if (y<0) y=0;

    // default value
    for(c=0;c<I->cs;c++) val[c]=0.0;

    ix=(int)floor(x);
    iy=(int)floor(y);
    dx=x-(double)ix;
    dy=y-(double)iy;

    if( ix-1<0 || iy-1<0 ) return(-1);
    if( ix+2>=I->xs ) return(-1);
    if( iy+2>=I->ys ) return(-1);

    for(c=0;c<I->cs;c++) {

        /* First horizontal row of four points */
        P=((iy-1)*I->xs+(ix-1))*I->cs+c;
        f[0]=I->complex[P][component];P+=I->cs;
        f[1]=I->complex[P][component];P+=I->cs;
        f[2]=I->complex[P][component];P+=I->cs;
        f[3]=I->complex[P][component];
        if( P>=I->xs*I->ys*I->cs ) { printf("INTERPOLE: ixy=(%d,%d) dxy=(%f,%f) c=%d sz=(%d,%d,%d)\n",ix,iy,dx,dy,c,I->xs,I->ys,I->cs); }
        SimpleCubicParam(f,q);
        v[0]=EvalCube(q,dx);

        /* Second horizontal row of four points */
        P=((iy)*I->xs+(ix-1))*I->cs+c;
        f[0]=I->complex[P][component];P+=I->cs;
        f[1]=I->complex[P][component];P+=I->cs;
        f[2]=I->complex[P][component];P+=I->cs;
        f[3]=I->complex[P][component];
        if( P>=I->xs*I->ys*I->cs ) { printf("INTERPOLE: ixy=(%d,%d) dxy=(%f,%f) c=%d sz=(%d,%d,%d)\n",ix,iy,dx,dy,c,I->xs,I->ys,I->cs); }
        SimpleCubicParam(f,q);
        v[1]=EvalCube(q,dx);

        /* Third horizontal row of four points */
        P=((iy+1)*I->xs+(ix-1))*I->cs+c;
        f[0]=I->complex[P][component];P+=I->cs;
        f[1]=I->complex[P][component];P+=I->cs;
        f[2]=I->complex[P][component];P+=I->cs;
        f[3]=I->complex[P][component];
        if( P>=I->xs*I->ys*I->cs ) { printf("INTERPOLE: ixy=(%d,%d) dxy=(%f,%f) c=%d sz=(%d,%d,%d)\n",ix,iy,dx,dy,c,I->xs,I->ys,I->cs); }
        SimpleCubicParam(f,q);
        v[2]=EvalCube(q,dx);

        /* Fourth horizontal row of four points */
        P=((iy+2)*I->xs+(ix-1))*I->cs+c;
        f[0]=I->complex[P][component];P+=I->cs;
        f[1]=I->complex[P][component];P+=I->cs;
        f[2]=I->complex[P][component];P+=I->cs;
        f[3]=I->complex[P][component];
        if( P>=I->xs*I->ys*I->cs ) { printf("INTERPOLE: ixy=(%d,%d) dxy=(%f,%f) c=%d sz=(%d,%d,%d)\n",ix,iy,dx,dy,c,I->xs,I->ys,I->cs); }
        SimpleCubicParam(f,q);
        v[3]=EvalCube(q,dx);


        /* Extract final value from row v */
        SimpleCubicParam(v,q);
        val[c]=EvalCube(q,dy);

        if( val[c]<0 ) val[c]=0;
        if( val[c]>IMGU_MAXVAL ) val[c]=(double)(IMGU_MAXVAL);

    }

    return(0);
}


