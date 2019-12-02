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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrixmath.h"

//// matrix manipulation package

// r=a-b
void vect3Subtract(vector3 a,vector3 b,vector3 r)
{
	r[0]=a[0]-b[0];
	r[1]=a[1]-b[1];
	r[2]=a[2]-b[2];
}

// r=aXb
void vect3Cross(vector3 a,vector3 b,vector3 r)
{
	r[0]=-(a[2]*b[1]) + a[1]*b[2];
	r[1]=a[2]*b[0] - a[0]*b[2];
	r[2]=-(a[1]*b[0]) + a[0]*b[1];
}

void vect3Print(vector3 v)
{
    fprintf(stdout,"%f\t%f\t%f\n",v[0],v[1],v[2]);
}

void vect3Copy(vector3 a,vector3 b) { vectxCopy(a,b,3); }
void vect4Copy(vector4 a,vector4 b) { vectxCopy(a,b,4); }

// homogenize a 3D projective point, so you have [x,y,z,1]
int vect4Homogenize3D(vector4 v)
{
int k=0;
	if( fabs(v[3])<1e-8 ) { v[3]=1e-8;k=-1; }
	v[0]/=v[3];
	v[1]/=v[3];
	v[2]/=v[3];
	v[3]=1.0;
	return(k);
}

// homogenize a 2D projective point, so you have [x,y,1,w]
int vect4Homogenize2D(vector4 v)
{
int k=0;
	if( fabs(v[2])<1e-8 ) { v[2]=1e-8;k=-1; }
	v[0]/=v[2];
	v[1]/=v[2];
	v[3]/=v[2];
	v[2]=1.0;
	return(k);
}

/**
 * Compute the square distance between two vectors
 * @param[in] pt1 a vector 
 * @param[in] pt2 a vector
 */
double vect4SquareDist(vector4 pt1, vector4 pt2) {
    vector4 diff;
    diff[0] = pt2[0]-pt1[0];
    diff[1] = pt2[1]-pt1[1];
    diff[2] = pt2[2]-pt1[2];
    diff[3] = pt2[3]-pt1[3];
    return vectxDot(diff,diff,4);
}

int vect3Homogenize2D(vector3 v)
{
int k=0;
	if( fabs(v[2])<1e-8 ) { v[2]=1e-8;k=-1; }
	v[0]/=v[2];
	v[1]/=v[2];
	v[2]=1.0;
	return(k);
}

void vect4Print(vector4 v)
{
    fprintf(stdout,"%f\t%f\t%f\t%f\n",v[0],v[1],v[2],v[3]);
	//printf("(%14.6e,%14.6e,%14.6e,%14.6e)",v[0],v[1],v[2],v[3]);
}


void vectxPrint(vector4 v, int len)
{
    int i;
    for (i = 0; i < len; i++ ) fprintf( stdout, "%f ", v[i] );
    printf("\n");
}



void mat3Identity(matrix3 m)
{
int i;
	for(i=1;i<9;i++) m[i]=0.0;
	m[0]=m[4]=m[8]=1.0;
}

void mat4Identity(matrix4 m)
{
int i;
	for(i=1;i<15;i++) m[i]=0.0;
	m[0]=m[5]=m[10]=m[15]=1.0;
}



int mat4Rotation(double a,vector3 n,matrix4 r)
{
double u,v,w,u2,v2,w2,ca,sa,uvw2,suvw2;
	u=n[0]; v=n[1]; w=n[2];
	u2=n[0]*n[0];
	v2=n[1]*n[1];
	w2=n[2]*n[2];
	ca=cos(a);
	sa=sin(a);
	uvw2=u2+v2+w2;
	suvw2=sqrt(uvw2);

	if( a==0.0 ) {
		mat4Identity(r);
		return(0);
	}

	if( uvw2==0.0 ) {
		mat4Identity(r);
		return(-1);
	}

	r[0]=(u2 + ca*(v2 + w2))/uvw2;
	r[1]=(u*v - ca*u*v - sa*suvw2*w)/uvw2;
	r[2]=(sa*suvw2*v + u*w - ca*u*w)/uvw2;
	r[3]=0;
	r[4]=(u*v - ca*u*v + sa*suvw2*w)/uvw2;
	r[5]=(v2 + ca*(u2 + w2))/uvw2;
	r[6]=-((sa*u*suvw2 + (-1 + ca)*v*w)/uvw2);
	r[7]=0;
	r[8]=-((sa*suvw2*v + (-1 + ca)*u*w)/uvw2);
	r[9]=(sa*u*suvw2 + v*w - ca*v*w)/ uvw2;
	r[10]=(ca*(u2 + v2) + w2)/uvw2;
	r[11]=0;
	r[12]=0;
	r[13]=0;
	r[14]=0;
	r[15]=1;

	return(0);
}

/**
 * Extracts the rotation values around the x, y, and z axis from a rotation matrix.
 *
 * @param[in] pose Pose or rotation matrix
 * @param[out] r rotation angle around (x,y,z) in radians
 *
 * @return 0 : success; <0 : error
 * 
 */
int mat4ExtractRotations(matrix4 pose, vector3 r) 
{
    int i;
    vector4 ax,ay,az;
    double tuples[32][3];
    double val1;
    double val2;
    // toutes les possibilites d'angle
    double ay1,ay2,ax1,ax2,ax3,ax4,az1,az2,az3,az4;

    val1 = 0;
    val2 = 0;
    
    ay1 = fmod( asin(pose[2]), (2*M_PI));
    ay2 = fmod( (M_PI-ay1), (2*M_PI));
    
    ax1 = fmod( asin(pose[6]/-cos(ay1)), (2*M_PI));
    ax2 = fmod( asin(pose[6]/-cos(ay2)), (2*M_PI));
    ax3 = fmod( (M_PI-ax1), (2*M_PI));
    ax4 = fmod( (M_PI-ax2), (2*M_PI));

    az1 = fmod( acos(pose[0]/cos(ay1)), (2*M_PI));
    az2 = fmod( acos(pose[0]/cos(ay2)), (2*M_PI));
    az3 = fmod( (2*M_PI-az1), (2*M_PI));
    az4 = fmod( (1*M_PI-az2), (2*M_PI));

    ay[0]=ay1;ay[1]=ay2;
    ax[0]=ax1;ax[1]=ax2;ax[2]=ax3;ax[3]=ax4;
    az[0]=az1;az[1]=az2;az[2]=az3;az[3]=az4;

    // Construire tous les tuples d'angles possibles
    for (i=0 ; i<32 ; i++) 
    {
        tuples[i][1] = ay[(i/16)%2];
        tuples[i][0] = ax[(i/4)%4];
        tuples[i][2] = az[i%4];
        //printf("%i,%i,%i\n",(i/16)%2,(i/4)%4,i%4);
    }
    
    // Verifier tous les tuples possibles
    for (i=0 ; i<32 ; i++) 
    {
        // test 1
        val1 = cos(tuples[i][0])*cos(tuples[i][1]);
        val2 = pose[10];
        //printf("[%i] %f {%f, %f, %f}\n",i,fabs(val2-val1),tuples[i][0],tuples[i][1],tuples[i][2]);
        if (fabs(val2-val1) > 0.001)
            continue;
        // test 2
        val1 = -cos(tuples[i][1])*sin(tuples[i][2]);
        val2 = pose[1];
        //printf("[%i] %f {%f, %f, %f}\n",i,fabs(val2-val1),tuples[i][0],tuples[i][1],tuples[i][2]);
        if (fabs(val2-val1) > 0.001)
            continue;
        // test 3
        val1 = cos(tuples[i][2])*sin(tuples[i][0])*sin(tuples[i][1]) + cos(tuples[i][0])*sin(tuples[i][2]);
        val2 = pose[4];
        //printf("[%i] %f {%f, %f, %f}\n",i,fabs(val2-val1),tuples[i][0],tuples[i][1],tuples[i][2]);
        if (fabs(val2-val1) > 0.001)
            continue;
        // test 4
        val1 = cos(tuples[i][0])*cos(tuples[i][2]) - sin(tuples[i][0])*sin(tuples[i][1])*sin(tuples[i][2]);
        val2 = pose[5];
        //printf("[%i] %f {%f, %f, %f}\n",i,fabs(val2-val1),tuples[i][0],tuples[i][1],tuples[i][2]);
        if (fabs(val2-val1) > 0.001)
            continue;
        // test 5
        val1 = cos(tuples[i][2])*sin(tuples[i][0]) + cos(tuples[i][0])*sin(tuples[i][1])*sin(tuples[i][2]);
        val2 = pose[9];
        //printf("[%i] %f {%f, %f, %f}\n",i,fabs(val2-val1),tuples[i][0],tuples[i][1],tuples[i][2]);
        if (fabs(val2-val1) > 0.001)
            continue;
        // test 6
        val1 = -cos(tuples[i][0])*cos(tuples[i][2])*sin(tuples[i][1]) + sin(tuples[i][0])*sin(tuples[i][2]);
        val2 = pose[8];
        //printf("[%i] %f {%f, %f, %f}\n",i,fabs(val2-val1),tuples[i][0],tuples[i][1],tuples[i][2]);
        if (fabs(val2-val1) > 0.001)
            continue;

        r[0] = tuples[i][0];
        r[1] = tuples[i][1];
        r[2] = tuples[i][2];
        //printf("%f %f %f\n", *rx, *ry, *rz);

        return 0;
    }
    
    return -1;
}

void mat4Translation(vector3 t,matrix4 m)
{
	m[0]=1.0; m[1]=0.0; m[2]=0.0; m[3]=t[0];
	m[4]=0.0; m[5]=1.0; m[6]=0.0; m[7]=t[1];
	m[8]=0.0; m[9]=0.0; m[10]=1.0; m[11]=t[2];
	m[12]=0.0; m[13]=0.0; m[14]=0.0; m[15]=1.0;
}

void mat4Scale(vector3 s,matrix4 m)
{
	m[0]=s[0]; m[1]=0.0; m[2]=0.0; m[3]=0.0;
	m[4]=0.0; m[5]=s[1]; m[6]=0.0; m[7]=0.0;
	m[8]=0.0; m[9]=0.0; m[10]=s[2]; m[11]=0.0;
	m[12]=0.0; m[13]=0.0; m[14]=0.0; m[15]=1.0;
}

void mat3Multiply(matrix3 a,matrix3 b,matrix3 m)
{
	m[0]=a[0]*b[0] + a[1]*b[3] + a[2]*b[6];
	m[1]=a[0]*b[1] + a[1]*b[4] + a[2]*b[7]; 
	m[2]=a[0]*b[2] + a[1]*b[5] + a[2]*b[8];
	m[3]=a[3]*b[0] + a[4]*b[3] + a[5]*b[6];
	m[4]=a[3]*b[1] + a[4]*b[4] + a[5]*b[7]; 
	m[5]=a[3]*b[2] + a[4]*b[5] + a[5]*b[8];
	m[6]=a[6]*b[0] + a[7]*b[3] + a[8]*b[6];
	m[7]=a[6]*b[1] + a[7]*b[4] + a[8]*b[7];
	m[8]=a[6]*b[2] + a[7]*b[5] + a[8]*b[8];
}

// a(4x4).b(4x4) -> m(4x4)
void mat4Multiply(matrix4 a,matrix4 b,matrix4 m)
{

	m[0]=a[0]*b[0] + a[1]*b[4] + a[2]*b[8] + a[3]*b[12];
	m[1]=a[0]*b[1] + a[1]*b[5] + a[2]*b[9] + a[3]*b[13]; 
	m[2]=a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
	m[3]=a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];
	m[4]=a[4]*b[0] + a[5]*b[4] + a[6]*b[8] + a[7]*b[12];
	m[5]=a[4]*b[1] + a[5]*b[5] + a[6]*b[9] + a[7]*b[13]; 
	m[6]=a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
	m[7]=a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15]; 
	m[8]=a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
	m[9]=a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
	m[10]=a[8]*b[2] + a[9]*b[6] + a[10]*b[10] + a[11]*b[14];
	m[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11] + a[11]*b[15]; 
	m[12]=a[12]*b[0] + a[13]*b[4] + a[14]*b[8] + a[15]*b[12];
	m[13]=a[12]*b[1] + a[13]*b[5] + a[14]*b[9] + a[15]*b[13];
	m[14]=a[12]*b[2] + a[13]*b[6] + a[14]*b[10] + a[15]*b[14];
	m[15]=a[12]*b[3] + a[13]*b[7] + a[14]*b[11] + a[15]*b[15];
 
}

// a(4x4).b(4x1) -> v(4x1)
void mat4MultiplyVector(matrix4 a,vector4 b,vector4 v)
{
	v[0]=a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];
	v[1]=a[4]*b[0]+a[5]*b[1]+a[6]*b[2]+a[7]*b[3];
	v[2]=a[8]*b[0]+a[9]*b[1]+a[10]*b[2]+a[11]*b[3];
	v[3]=a[12]*b[0]+a[13]*b[1]+a[14]*b[2]+a[15]*b[3];
}

void mat3MultiplyVector(matrix3 a,vector3 b,vector3 v)
{
	v[0]=a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
	v[1]=a[3]*b[0]+a[4]*b[1]+a[5]*b[2];
	v[2]=a[6]*b[0]+a[7]*b[1]+a[8]*b[2];
}

void mat3Copy(matrix3 a,matrix3 b) { vectxCopy(a,b,9); }
void mat4Copy(matrix4 a,matrix4 b) { vectxCopy(a,b,16); }

int mat3ConvertToMat4(matrix3 m,matrix4 r)
{
  mat4Identity(r);
  r[0]=m[0];
  r[1]=m[1];
  r[2]=m[2];
  r[4]=m[3];
  r[5]=m[4];
  r[6]=m[5];
  r[8]=m[6];
  r[9]=m[7];
  r[10]=m[8];

  return 0;
}

int mat4ConvertToMat3(matrix4 m,matrix3 r)
{
  r[0]=m[0];
  r[1]=m[1];
  r[2]=m[2];
  r[3]=m[4];
  r[4]=m[5];
  r[5]=m[6];
  r[6]=m[8];
  r[7]=m[9];
  r[8]=m[10];

  return 0;
}

/***   res = Inverse(m)	   ****/
/* returns -1 if inverse is not defined */
int mat3Inverse (matrix3 m,matrix3 im)
{
    double det;

    det=m[0]*m[4]*m[8]-m[2]*m[4]*m[6]+m[1]*m[5]*m[6]+m[2]*m[3]*m[7]-m[0]*m[5]*m[7]-m[1]*m[3]*m[8];
    if (fabs(det)<MAT_VERY_VERY_SMALL) return -1;

    im[0]=(-m[5]*m[7]+m[4]*m[8])/det;
    im[1]=(m[2]*m[7]-m[1]*m[8])/det;
    im[2]=(-m[2]*m[4]+m[1]*m[5])/det;
    im[3]=(m[5]*m[6]-m[3]*m[8])/det;
    im[4]=(-m[2]*m[6]+m[0]*m[8])/det;
    im[5]=(m[2]*m[3]-m[0]*m[5])/det;
    im[6]=(-m[4]*m[6]+m[3]*m[7])/det;
    im[7]=(m[1]*m[6]-m[0]*m[7])/det;
    im[8]=(-m[1]*m[3]+m[0]*m[4])/det;
    return 0;
}

// suppose que m[12..15] = {0,0,0,1}
int mat4InverseAffine(matrix4 m,matrix4 im)
{
double det;
int i;
	det=-(m[1]*m[10]*m[4]) + m[0]*m[10]*m[5] - m[2]*m[5]*m[8]
	       	+ m[1]*m[6]*m[8] + m[2]*m[4]*m[9] - m[0]*m[6]*m[9];
	if( fabs(det)<MAT_VERY_SMALL ) return(-1);
	im[0]=m[10]*m[5] - m[6]*m[9];
	im[1]=-(m[1]*m[10]) + m[2]*m[9];
	im[2]=-(m[2]*m[5]) + m[1]*m[6];
	im[3]=m[11]*m[2]*m[5] - m[10]*m[3]*m[5] - m[1]*m[11]*m[6]
	       	+ m[1]*m[10]*m[7] + m[3]*m[6]*m[9] - m[2]*m[7]*m[9];
	im[4]=-(m[10]*m[4]) + m[6]*m[8];
	im[5]=m[0]*m[10] - m[2]*m[8];
	im[6]=m[2]*m[4] - m[0]*m[6];
	im[7]=-(m[11]*m[2]*m[4]) + m[10]*m[3]*m[4] + m[0]*m[11]*m[6]
	       	- m[0]*m[10]*m[7] - m[3]*m[6]*m[8] + m[2]*m[7]*m[8];
	im[8]= -(m[5]*m[8]) + m[4]*m[9];
	im[9]=m[1]*m[8] - m[0]*m[9];
	im[10]=-(m[1]*m[4]) + m[0]*m[5];
	im[11]=m[1]*m[11]*m[4] - m[0]*m[11]*m[5] + m[3]*m[5]*m[8]
	       	- m[1]*m[7]*m[8] - m[3]*m[4]*m[9] + m[0]*m[7]*m[9];
	im[12]=0.0;
	im[13]=0.0;
	im[14]=0.0;
	im[15]=1.0;
	for(i=0;i<=11;i++) im[i]/=det;
	return(0);
}

// inverse general
int mat4Inverse(matrix4 m,matrix4 im)
{
double det;
int i;
	det=m[1]*m[11]*m[14]*m[4] - m[1]*m[10]*m[15]*m[4]
    - m[11]*m[13]*m[2]*m[4] + m[10]*m[13]*m[3]*m[4] - m[0]*m[11]*m[14]*m[5]
    + m[0]*m[10]*m[15]*m[5] + m[11]*m[12]*m[2]*m[5] - m[10]*m[12]*m[3]*m[5]
    - m[1]*m[11]*m[12]*m[6] + m[0]*m[11]*m[13]*m[6] + m[1]*m[10]*m[12]*m[7]
    - m[0]*m[10]*m[13]*m[7] - m[15]*m[2]*m[5]*m[8] + m[14]*m[3]*m[5]*m[8]
    + m[1]*m[15]*m[6]*m[8] - m[13]*m[3]*m[6]*m[8] - m[1]*m[14]*m[7]*m[8]
    + m[13]*m[2]*m[7]*m[8] + m[15]*m[2]*m[4]*m[9] - m[14]*m[3]*m[4]*m[9]
    - m[0]*m[15]*m[6]*m[9] + m[12]*m[3]*m[6]*m[9] + m[0]*m[14]*m[7]*m[9]
    - m[12]*m[2]*m[7]*m[9];

	if( fabs(det)<MAT_VERY_SMALL ) return(-1);

	im[0]=-m[11]*m[14]*m[5] + m[10]*m[15]*m[5] + m[11]*m[13]*m[6]
		- m[10]*m[13]*m[7] - m[15]*m[6]*m[9] + m[14]*m[7]*m[9];
	im[1]=m[1]*m[11]*m[14] - m[1]*m[10]*m[15] - m[11]*m[13]*m[2]
	       	+ m[10]*m[13]*m[3] + m[15]*m[2]*m[9] - m[14]*m[3]*m[9];
	im[2]=-m[15]*m[2]*m[5] + m[14]*m[3]*m[5] + m[1]*m[15]*m[6]
	       	- m[13]*m[3]*m[6] - m[1]*m[14]*m[7] + m[13]*m[2]*m[7];
	im[3]=m[11]*m[2]*m[5] - m[10]*m[3]*m[5] - m[1]*m[11]*m[6]
	       	+ m[1]*m[10]*m[7] + m[3]*m[6]*m[9] - m[2]*m[7]*m[9];
	im[4]=m[11]*m[14]*m[4] - m[10]*m[15]*m[4] - m[11]*m[12]*m[6]
	       	+ m[10]*m[12]*m[7] + m[15]*m[6]*m[8] - m[14]*m[7]*m[8];
	im[5]=-m[0]*m[11]*m[14] + m[0]*m[10]*m[15] + m[11]*m[12]*m[2]
	       	- m[10]*m[12]*m[3] - m[15]*m[2]*m[8] + m[14]*m[3]*m[8];
	im[6]=m[15]*m[2]*m[4] - m[14]*m[3]*m[4] - m[0]*m[15]*m[6]
	       	+ m[12]*m[3]*m[6] + m[0]*m[14]*m[7] - m[12]*m[2]*m[7];
	im[7]=-m[11]*m[2]*m[4] + m[10]*m[3]*m[4] + m[0]*m[11]*m[6]
	       	- m[0]*m[10]*m[7] - m[3]*m[6]*m[8] + m[2]*m[7]*m[8];
	im[8]=-m[11]*m[13]*m[4] + m[11]*m[12]*m[5] - m[15]*m[5]*m[8]
	       	+ m[13]*m[7]*m[8] + m[15]*m[4]*m[9] - m[12]*m[7]*m[9];
	im[9]=-m[1]*m[11]*m[12] + m[0]*m[11]*m[13] + m[1]*m[15]*m[8]
	       	- m[13]*m[3]*m[8] - m[0]*m[15]*m[9] + m[12]*m[3]*m[9];
	im[10]=-m[1]*m[15]*m[4] + m[13]*m[3]*m[4] + m[0]*m[15]*m[5]
	       	- m[12]*m[3]*m[5] + m[1]*m[12]*m[7] - m[0]*m[13]*m[7];
	im[11]=m[1]*m[11]*m[4] - m[0]*m[11]*m[5] + m[3]*m[5]*m[8]
	       	- m[1]*m[7]*m[8] - m[3]*m[4]*m[9] + m[0]*m[7]*m[9];
	im[12]=m[10]*m[13]*m[4] - m[10]*m[12]*m[5] + m[14]*m[5]*m[8]
	       	- m[13]*m[6]*m[8] - m[14]*m[4]*m[9] + m[12]*m[6]*m[9];
	im[13]=m[1]*m[10]*m[12] - m[0]*m[10]*m[13] - m[1]*m[14]*m[8]
	       	+ m[13]*m[2]*m[8] + m[0]*m[14]*m[9] - m[12]*m[2]*m[9];
	im[14]=m[1]*m[14]*m[4] - m[13]*m[2]*m[4] - m[0]*m[14]*m[5]
	       	+ m[12]*m[2]*m[5] - m[1]*m[12]*m[6] + m[0]*m[13]*m[6];
	im[15]=-m[1]*m[10]*m[4] + m[0]*m[10]*m[5] - m[2]*m[5]*m[8]
	       	+ m[1]*m[6]*m[8] + m[2]*m[4]*m[9] - m[0]*m[6]*m[9];

	for(i=0;i<16;i++) im[i]/=det;
	return(0);

}

// transpose m into mt, m==mt OK
void mat4Transpose(matrix4 m,matrix4 mt)
{
    mt[0]=m[0];
    mt[5]=m[5];
    mt[10]=m[10];
    mt[15]=m[15];

    mt[1]=m[4];    mt[4]=m[1];
    mt[2]=m[8];    mt[8]=m[2];
    mt[3]=m[12];   mt[12]=m[3];
    mt[6]=m[9];    mt[9]=m[6];
    mt[7]=m[13];   mt[13]=m[7];
    mt[11]=m[14];   mt[14]=m[11];
}

// transpose m into mt, m==mt OK
void mat3Transpose(matrix3 m,matrix3 mt)
{
    mt[0]=m[0];
    mt[4]=m[4];
    mt[8]=m[8];

    mt[1]=m[3];    mt[3]=m[1];
    mt[2]=m[6];    mt[6]=m[2];
    mt[5]=m[7];    mt[7]=m[5];
}

void mat3SkewSymmetric(vector3 n,matrix3 nx)
{
  nx[0]=0;
  nx[1]=-n[2];
  nx[2]=n[1];
  nx[3]=n[2];
  nx[4]=0;
  nx[5]=-n[0];
  nx[6]=-n[1];
  nx[7]=n[0];
  nx[8]=0;
}


void mat3Print(matrix3 m)
{
	printf("-----\n");
    printf("%f %f %f\n",m[0],m[1],m[2]);
	printf("%f %f %f\n",m[3],m[4],m[5]);
	printf("%f %f %f\n",m[6],m[7],m[8]);
	printf("-----\n");
}

void mat4Print(matrix4 m)
{
	printf("-----\n");
	//printf("%14.6e %14.6e %14.6e %14.6e\n",m[0],m[1],m[2],m[3]);
	//printf("%14.6e %14.6e %14.6e %14.6e\n",m[4],m[5],m[6],m[7]);
	//printf("%14.6e %14.6e %14.6e %14.6e\n",m[8],m[9],m[10],m[11]);
	//printf("%14.6e %14.6e %14.6e %14.6e\n",m[12],m[13],m[14],m[15]);
    printf("%f %f %f %f\n",m[0],m[1],m[2],m[3]);
	printf("%f %f %f %f\n",m[4],m[5],m[6],m[7]);
	printf("%f %f %f %f\n",m[8],m[9],m[10],m[11]);
	printf("%f %f %f %f\n",m[12],m[13],m[14],m[15]);
	printf("-----\n");
}

// format Mathematica
void mat4PrintMath(matrix4 m)
{
	printf("-----\n");
	//printf("%14.6e %14.6e %14.6e %14.6e\n",m[0],m[1],m[2],m[3]);
	//printf("%14.6e %14.6e %14.6e %14.6e\n",m[4],m[5],m[6],m[7]);
	//printf("%14.6e %14.6e %14.6e %14.6e\n",m[8],m[9],m[10],m[11]);
	//printf("%14.6e %14.6e %14.6e %14.6e\n",m[12],m[13],m[14],m[15]);
    printf("{{%f, %f, %f, %f},\n",m[0],m[1],m[2],m[3]);
	printf(" {%f, %f, %f, %f},\n",m[4],m[5],m[6],m[7]);
	printf(" {%f, %f, %f, %f},\n",m[8],m[9],m[10],m[11]);
	printf(" {%f, %f, %f, %f}}\n",m[12],m[13],m[14],m[15]);
	printf("-----\n");
}


// extract a 4x4 from a string (0=ok, -1=err)
int mat4Scanf(char *buf,matrix4 m)
{
int k;
	if( buf==NULL ) return(-1);
	k=sscanf(buf," %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		m+0,m+1,m+2,m+3,m+4,m+5,m+6,m+7,m+8,m+9,m+10,m+11,m+12,m+13,m+14,m+15);
	if( k!=16 ) return(-1);
	return(0);
}

//  a string for a matrix. len is the size of buf. return 0 if ok, -1 if err
int mat4Printf(matrix4 m,char *buf,int len)
{
int i;
	buf[0]=0;
	// on test 17 parce qu'on imprime %16 plus un espace, plus un zero a la fin
	if( len < 17*16+1 ) return(-1); // pas assez de place!!
	for(i=0;i<16;i++) sprintf(buf+strlen(buf)," %16f",m[i]);
	return(0);
}


// a.b
double vectxDot(double *a,double *b,int len)
{
int i;
double S;
	S=0.0;
	for(i=0;i<len;i++) S+=a[i]*b[i];
	return(S);
}

// angle between two vectors of some len
double vectxAngle(double *a,double *b,int len)
{
double La,Lb;
	La=vectxNorm(a,len);
	Lb=vectxNorm(b,len);
	return(acos(vectxDot(a,b,len)/(La*Lb)));
}

double vectxNorm(double *w,int len) { return(sqrt(vectxNorm2(w,len))); }

double vectxNorm2(double *w,int len)
{
double s;
int i;
	s=0.0;
	for(i=0;i<len;i++) s+=w[i]*w[i];
	return(s);
}

int vectxNormalize(double *w,int len)
{
double L;
int i;
	L=vectxNorm(w,len);
	if( L<1e-8 ) return(-1);
	for(i=0;i<len;i++) w[i]/=L;
	return(0);
}

void vectxCopy(const double *a,double *b,int len)
{
int i;
	for(i=0;i<len;i++) b[i]=a[i];
}

void vectxAdd(double *a,double *b,double *m,int len)
{
int i;
	for(i=0;i<len;i++) m[i]=a[i]+b[i];
}

void vectxScale(double *a,double scale,double *m,int len)
{
int i;
	for(i=0;i<len;i++) m[i]=a[i]*scale;
}

void vectxSubtract(double *a,double *b,double *m,int len)
{
int i;
	for(i=0;i<len;i++) m[i]=a[i]-b[i];
}

double vectxAvg(double *a,int len)
{
  int i;
  double avg;

  if (len<1) return 0.0;

  avg=0;
  for (i=0;i<len;i++)
  {
    avg+=a[i];
  }
  avg/=len;

  return avg;
}

double vectxDev(double *a,int len)
{
  int i;
  double avg,stdev;

  if (len<2) return 0.0;

  avg=0;
  stdev=0;
  for (i=0;i<len;i++)
  {
    avg+=a[i];
    stdev+=a[i]*a[i];
  }

  avg/=len;
  stdev=(stdev-len*avg*avg)/(len-1);
  if (stdev<0) stdev=0;
  else stdev=sqrt(stdev);

  return stdev;
}

double vectxDevP(double *a,int len)
{
  int i;
  double avg,stdev;

  if (len<1) return 0.0;

  avg=0;
  stdev=0;
  for (i=0;i<len;i++)
  {
    avg+=a[i];
    stdev+=a[i]*a[i];
  }

  avg/=len;
  stdev=(stdev-len*avg*avg)/len;
  if (stdev<0) stdev=0;
  else stdev=sqrt(stdev);

  return stdev;
}

double vectxMax(double *a,int len)
{
  int i;
  double max=0;

  for (i=0;i<len;i++)
  {
    if (i==0 || max<a[i]) max=a[i];
  }

  return max;
}

double vectxMin(double *a,int len)
{
  int i;
  double min=0;

  for (i=0;i<len;i++)
  {
    if (i==0 || min>a[i]) min=a[i];
  }

  return min;
}

///// DEBUG //////

#ifdef SKIP

int main(int argc,char *argv[])
{
double r1[16],r2[16],n[3];
double s1[16],is1[16];
double cam[16],icam[16];
double camIn[16],camEx[16];
double p[10],qa[4],pa[4],pb[4];
double L;
	printf("allo\n");

	n[0]=1.0;
	n[1]=0.0;
	n[2]=0.0;
	rotationMatrix(M_PI/4.0,n,r1);

	n[0]=1.0;
	n[1]=1.0;
	n[2]=1.0;
	rotationMatrix(-M_PI/4.0,n,r2);

	n[0]=2.0;
	n[1]=3.0;
	n[2]=4.0;
	scaleMatrix(n,r2);

	multMM(r1,r2,s1);

	dump4x4(r1);
	dump4x4(r2);
	dump4x4(s1);

	inverseMaffine(s1,is1);

	dump4x4(is1);

	inverseMaffine(is1,s1);
	dump4x4(s1);
	internalMatrix(256.0,128.0,64.0,64.0,s1);
	dump4x4(s1);


	// interne
	p[0]=512.0;
	p[1]=512.0;
	p[2]=512.0;
	p[3]=512.0;

	// translation
	p[4]=1.0;
	p[5]=2.0;
	p[6]=3.0;

	// rotation
	p[7]=1.0;
	p[8]=1.0;
	p[9]=1.0; normalize(p+7,3); //  p[7,8,9] into an axis
	L=10*1.5*M_PI/180.0; // rotation magnitude
	p[7]*=L;p[8]*=L;p[9]*=L;

	cameraMatrix(p,cam,camIn,camEx);
	dump4x4(cam);
	inverseMaffine(cam,icam);
	dump4x4(icam);

	qa[0]=10.0;
	qa[1]=15.0;
	qa[2]=20.0;
	qa[3]=1.0;
	multMV(cam,qa,pa);
	dump4x1(qa);printf(" -> ");dump4x1(pa);

	homogene2d(pa);
	printf(" -H-> ");dump4x1(pa);
	printf("\n");

	multMV(icam,pa,pb);
	printf("in 3d -> ");dump4x1(pb);
	homogene3d(pb);
	printf(" -H-> ");dump4x1(pb);
	printf("\n");

	pa[3]/=2;
	multMV(icam,pa,pb);
	printf("in 3d -> ");dump4x1(pb);
	homogene3d(pb);
	printf(" -H-> ");dump4x1(pb);
	printf("\n");

	pa[0]=1.0;
	pa[1]=0.0;
	pa[2]=0.0;
	pa[3]=0.0;
	multMV(camEx,pa,pb);
	printf("NORMALE: ");dump4x1(pa);printf(" --> ");dump4x1(pb);printf("\n");

}

#endif

int matAllocate(matrix **mat,int cs,int rs)
{
    double *v;
    int size;

    if (mat==NULL) return -1;

    size=cs*rs;
    if (size<0) return -1;

    if ((*mat)==NULL)
    {
      (*mat)=(matrix *)(malloc(sizeof(matrix)));
      if ((*mat)==NULL) return -1;
      (*mat)->cs=0;
      (*mat)->rs=0;
      (*mat)->ds=0;
      (*mat)->values=NULL;
    }
    else if (size <= (*mat)->ds)
    {
        (*mat)->cs=cs;
        (*mat)->rs=rs;
        memset((*mat)->values,0,size*sizeof(double));
        return 0;
    }

    if ((*mat)->values!=NULL) free((*mat)->values);

    v=(double *)(malloc(sizeof(double)*size));
    if( v==NULL ) {
        (*mat)->rs=(*mat)->cs=(*mat)->ds=0;
        (*mat)->values=NULL;
        return -1;
    }
    (*mat)->rs=rs;
    (*mat)->cs=cs;
    (*mat)->ds=size;
    (*mat)->values=v;

    memset((*mat)->values,0,size*sizeof(double));

    return 0;
}

int matCopy(matrix **copy,matrix *mat)
{
    int size,cs,rs;
    int i;

    if (copy==NULL) return -1;
    if (mat==NULL) return -1;

    if( (*copy)==mat ) return 0;

    cs=mat->cs;
    rs=mat->rs;
    size=cs*rs;
    matAllocate(copy,cs,rs);

    for(i=0;i<size;i++)
    {    
        (*copy)->values[i]=mat->values[i];
    }

    return 0;
}


void matFree(matrix **m)
{
    if( m==NULL || (*m)==NULL) return;

    if( (*m)->values!=NULL)
    {
        free((*m)->values);
        (*m)->values=NULL;
    }
    free((*m));
    (*m)=NULL;
}

double matGet(matrix *mat,int r,int c)
{
  if (mat==NULL) return 0.0;
  if (r>=mat->cs) return 0.0;
  if (c>=mat->rs) return 0.0;

  return mat->values[r*mat->rs+c];
}

int matSet(matrix *mat,unsigned int r,unsigned int c,double val)
{
  if (mat==NULL) return -1;
  if (r>=mat->cs) return -1;
  if (c>=mat->rs) return -1;

  mat->values[r*mat->rs+c]=val;

  return 0;
}

int matSelectRows(matrix **res,matrix *mat,int rstart,int nbrows)
{
    int i,j;
    if (res==NULL) return -1;
    if (mat==NULL) return -1;
    if (rstart<0) return -1;
    if (rstart+nbrows>mat->cs) return -1;
    if (nbrows<1) return -1;

    if( *res!=mat ) { if(matAllocate(res,nbrows,mat->rs)) return(-1); }

    for (i=0;i<nbrows;i++)
    {
      for (j=0;j<mat->rs;j++)
      {
        (*res)->values[i*mat->rs+j]=mat->values[(i+rstart)*mat->rs+j];
      }
    }

    (*res)->cs=nbrows;
  
    return 0;
}

int matSelectColumns(matrix **res,matrix *mat,int cstart,int nbcols)
{
    int i,j;
    if (res==NULL) return -1;
    if (mat==NULL) return -1;
    if (cstart<0) return -1;
    if (cstart+nbcols>mat->rs) return -1;
    if (nbcols<1) return -1;

    if( *res!=mat ) { if(matAllocate(res,mat->cs,nbcols)) return(-1); }

    for (i=0;i<mat->cs;i++)
    {
      for (j=0;j<nbcols;j++)
      {
        (*res)->values[i*nbcols+j]=mat->values[i*mat->rs+j+cstart];
      }
    }

    (*res)->rs=nbcols;
  
    return 0;
}

int matRemoveRow(matrix **res,matrix *mat,int row)
{
    int i,j;
    if (res==NULL) return -1;
    if (mat==NULL) return -1;
    if (row<0 || row>=mat->cs) return -1;

    if( *res!=mat ) { if(matCopy(res,mat)) return(-1); }

    i=row;
    while(i<mat->cs-1)
    {
      for (j=0;j<mat->rs;j++)
      {
        (*res)->values[i*mat->rs+j]=mat->values[(i+1)*mat->rs+j];
      }
      i++;
    }

    (*res)->cs=mat->cs-1;
  
    return 0;
}

int matRemoveColumn(matrix **res,matrix *mat,int col)
{
    int i,j,k;
    if (res==NULL) return -1;
    if (mat==NULL) return -1;
    if (col<0 || col>=mat->rs) return -1;

    if( *res!=mat ) { if(matCopy(res,mat)) return(-1); }

    for (i=0;i<mat->cs;i++)
    {
      for (j=0;j<mat->rs-1;j++)
      {
        k=j;
        if (k>=col) k++;
        (*res)->values[i*(mat->rs-1)+j]=mat->values[i*mat->rs+k];
      }
    }

    (*res)->rs=mat->rs-1;
  
    return 0;
}

int matInsertColumn(matrix **res,matrix *mat,int col,double val)
{
    int i,j,k;
    unsigned char realloc;
    matrix *matcpy;

    if (res==NULL) return -1;
    if (mat==NULL) return -1;
    if (col<0 || col>mat->rs) return -1;

    if((*res)==mat)
    {
        matcpy=NULL;
        matCopy(&matcpy,mat);
        realloc=1;
    }
    else
    {
        matcpy=mat;
        realloc=0;
    }
    if(matAllocate(res,mat->cs,mat->rs+1)) {if (realloc) {matFree(&matcpy);}return(-1);}

    for (i=0;i<matcpy->cs;i++)
    {
      for (j=0;j<matcpy->rs;j++)
      {
        k=j;
        if (k>=col) k++;
        (*res)->values[i*(*res)->rs+k]=matcpy->values[i*matcpy->rs+j];
      }
    }
    for (i=0;i<matcpy->cs;i++)
    {
      (*res)->values[i*(*res)->rs+col]=val;
    }

    if (realloc) matFree(&matcpy);
  
    return 0;
}

int matConcatColumns(matrix **res,matrix *mat1,matrix *mat2)
{
    int i,j;
    unsigned char realloc1,realloc2;
    matrix *mat1cpy;
    matrix *mat2cpy;

    if (res==NULL) return -1;
    if (mat1==NULL && mat2==NULL) return -1;
    if (mat1==NULL) 
    {
      matCopy(res,mat2);
      return 0;
    }
    if (mat2==NULL) 
    {
      matCopy(res,mat1);
      return 0;
    }
    if (mat1->rs!=mat2->rs) return -1;

    if ((*res)==mat1)
    {
        mat1cpy=NULL;
        matCopy(&mat1cpy,mat1);
        realloc1=1;
    }
    else
    {
        mat1cpy=mat1;
        realloc1=0;
    }
    if ((*res)==mat2)
    {
        mat2cpy=NULL;
        matCopy(&mat2cpy,mat2);
        realloc2=1;
    }
    else
    {
        mat2cpy=mat2;
        realloc2=0;
    }

    if(matAllocate(res,mat1cpy->cs+mat2cpy->cs,mat1->rs)) {if (realloc1) matFree(&mat1cpy);if (realloc2) matFree(&mat2cpy);return(-1);}

    for (i=0;i<mat1cpy->cs;i++)
    {
      for (j=0;j<mat1cpy->rs;j++)
      {
        (*res)->values[i*(*res)->rs+j]=mat1cpy->values[i*mat1cpy->rs+j];
      }
    }
    for (i=0;i<mat2cpy->cs;i++)
    {
      for (j=0;j<mat2cpy->rs;j++)
      {
        (*res)->values[(i+mat1cpy->cs)*(*res)->rs+j]=mat2cpy->values[i*mat2cpy->rs+j];
      }
    }

    if (realloc1) matFree(&mat1cpy);
    if (realloc2) matFree(&mat2cpy);
  
    return 0;
}

int matConcatRows(matrix **res,matrix *mat1,matrix *mat2)
{
    int i,j;
    unsigned char realloc1,realloc2;
    matrix *mat1cpy;
    matrix *mat2cpy;

    if (res==NULL) return -1;
    if (mat1==NULL && mat2==NULL) return -1;
    if (mat1==NULL)
    {
      matCopy(res,mat2);
      return 0;
    }
    if (mat2==NULL)
    {
      matCopy(res,mat1);
      return 0;
    }
    if (mat1->cs!=mat2->cs) return -1;

    if ((*res)==mat1)
    {
        mat1cpy=NULL;
        matCopy(&mat1cpy,mat1);
        realloc1=1;
    }
    else
    {
        mat1cpy=mat1;
        realloc1=0;
    }
    if ((*res)==mat2)
    {
        mat2cpy=NULL;
        matCopy(&mat2cpy,mat2);
        realloc2=1;
    }
    else
    {
        mat2cpy=mat2;
        realloc2=0;
    }

    if(matAllocate(res,mat1cpy->cs,mat1->rs+mat2cpy->rs)) {if (realloc1) matFree(&mat1cpy);if (realloc2) matFree(&mat2cpy);return(-1);}

    for (i=0;i<mat1cpy->cs;i++)
    {
      for (j=0;j<mat1cpy->rs;j++)
      {
        (*res)->values[i*(*res)->rs+j]=mat1cpy->values[i*mat1cpy->rs+j];
      }
    }
    for (i=0;i<mat2cpy->cs;i++)
    {
      for (j=0;j<mat2cpy->rs;j++)
      {
        (*res)->values[i*(*res)->rs+j+mat1cpy->rs]=mat2cpy->values[i*mat2cpy->rs+j];
      }
    }

    if (realloc1) matFree(&mat1cpy);
    if (realloc2) matFree(&mat2cpy);
  
    return 0;
}

int matInverse(matrix **res,matrix *mat)
{
    int ret;
    unsigned char realloc;
    matrix *matcpy;

    if (res==NULL || mat==NULL) return -1;
    if (mat->rs!=mat->cs) return -1;
    if((*res)==mat)
    {
        matcpy=NULL;
        matCopy(&matcpy,mat);
        realloc=1;
    }
    else
    {
        matcpy=mat;
        realloc=0;
    }
    if(matAllocate(res,mat->cs,mat->rs)) {if (realloc) {matFree(&matcpy);}return(-1);}

    if (mat->rs==1)
    {
      if (matcpy->values[0]<MAT_VERY_VERY_SMALL) ret=-1;
      else
      {
        (*res)->values[0]=1.0/matcpy->values[0];
        ret=0;
      }
    }
    else if (mat->rs==2)
    {
      double det=matcpy->values[0]*matcpy->values[3]-matcpy->values[1]*matcpy->values[2];
      if (fabs(det)<MAT_VERY_VERY_SMALL) ret=-1;
      else
      {
        (*res)->values[0]=matcpy->values[3]/det;
        (*res)->values[1]=-matcpy->values[1]/det;
        (*res)->values[2]=-matcpy->values[2]/det;
        (*res)->values[3]=matcpy->values[0]/det;
        ret=0;
      }
    }
    else if (mat->rs==3) //3x3 matrix
    {
        ret=mat3Inverse(matcpy->values,(*res)->values);
    }
    else if (mat->rs==4 && matGet(mat,3,0)==0.0 && matGet(mat,3,1)==0.0 && matGet(mat,3,2)==0.0 && matGet(mat,3,3)==1.0) //4x4 affine matrix
    {
        ret=mat4InverseAffine(matcpy->values,(*res)->values);
    }
    else if (mat->rs==4) //4x4 matrix
    {
        ret=mat4Inverse(matcpy->values,(*res)->values);
    }
    else
    {
        fprintf(stderr,"Warning: matInverse is not defined for matrices bigger than 4x4\n");
        ret=-1;
    }

    if (realloc) matFree(&matcpy);

    return ret;
}

int matTranspose(matrix **res,matrix *mat)
{
    int i,j;
    unsigned char realloc;
    matrix *matcpy;

    if (res==NULL || mat==NULL) return -1;
    if((*res)==mat)
    {
        matcpy=NULL;
        matCopy(&matcpy,mat);
        realloc=1;
    }
    else
    {
        matcpy=mat;
        realloc=0;
    }
    if(matAllocate(res,mat->rs,mat->cs)) {if (realloc) {matFree(&matcpy);}return(-1);}

    for (i=0;i<(*res)->cs;i++)
    {
        for (j=0;j<(*res)->rs;j++)
        {
            (*res)->values[i*(*res)->rs+j]=matcpy->values[j*matcpy->rs+i];
        }
    }

    if (realloc) matFree(&matcpy);

    return 0;
}

int matMultiply(matrix **res,matrix *mat1,matrix *mat2)
{
    int i,j,k;
    unsigned char realloc1;
    unsigned char realloc2;
    matrix *mat1cpy;
    matrix *mat2cpy;

    if (res==NULL || mat1==NULL || mat2==NULL) return -1;
    if (mat1->rs!=mat2->cs) return -1;

    if((*res)==mat1 || mat1==mat2)
    {
        mat1cpy=NULL;
        matCopy(&mat1cpy,mat1);
        realloc1=1;
    }
    else
    {
        mat1cpy=mat1;
        realloc1=0;
    }
    if((*res)==mat2)
    {
        mat2cpy=NULL;
        matCopy(&mat2cpy,mat2);
        realloc2=1;
    }
    else
    {
        mat2cpy=mat2;
        realloc2=0;
    }
    if(matAllocate(res,mat1cpy->cs,mat2cpy->rs)) {if (realloc1) {matFree(&mat1cpy);}if (realloc2) {matFree(&mat2cpy);}return(-1);}

    for (i=0;i<mat1cpy->cs;i++)
    {
        for (j=0;j<mat2cpy->rs;j++)
        {
            (*res)->values[i*(*res)->rs+j]=0;
            for (k=0;k<mat2cpy->cs;k++)
            {
                (*res)->values[i*(*res)->rs+j]+=(mat1cpy->values[i*mat1cpy->rs+k]*mat2cpy->values[k*mat2cpy->rs+j]);
            }
        }  
    }

    if (realloc1) matFree(&mat1cpy);
    if (realloc2) matFree(&mat2cpy);

    return 0;
}

int matAdd(matrix **res,matrix *mat1,matrix *mat2)
{
    int i,size;

    if (res==NULL || mat1==NULL || mat2==NULL) return -1;
    if (mat1->rs!=mat2->rs || mat1->cs!=mat2->cs) return -1;
    if ((*res)!=mat1 && (*res)!=mat2)
    {
      if (matAllocate(res,mat1->cs,mat1->rs)) return -1;
    }

    size=mat1->cs*mat1->rs;
    for (i=0;i<size;i++)
    {
        (*res)->values[i]=mat1->values[i]+mat2->values[i];
    }

    return 0;
}

int matSubtract(matrix **res,matrix *mat1,matrix *mat2)
{
    int i,size;

    if (res==NULL || mat1==NULL || mat2==NULL) return -1;
    if (mat1->rs!=mat2->rs || mat1->cs!=mat2->cs) return -1;

    if ((*res)!=mat1 && (*res)!=mat2)
    {
      if (matAllocate(res,mat1->cs,mat1->rs)) return -1;
    }

    size=mat1->cs*mat1->rs;
    for (i=0;i<size;i++)
    {
        (*res)->values[i]=mat1->values[i]-mat2->values[i];
    }

    return 0;
}

int matScale(matrix **res,matrix *mat,double scale)
{
    int i,size;

    if (res==NULL || mat==NULL) return -1;
    if ((*res)!=mat) 
    {
      if (matAllocate(res,mat->cs,mat->rs)) return -1;
    }

    size=mat->cs*mat->cs;
    for (i=0;i<size;i++)
    {
        (*res)->values[i]*=scale;
    }

    return 0;
}

static double determinantrec(double *a, int n)
{
    int i,j,j1,j2;
    double det = 0;
    double *m = NULL;

    if (n < 1) { /* Error */
    } else if (n == 1) { /* Shouldn't get used */
        det = a[0];
    } else if (n == 2) {
        det = a[0] * a[n+1] - a[n] * a[1];
    } else {
        det = 0;
        for (j1=0;j1<n;j1++) {
            m = malloc((n-1)*(n-1)*sizeof(double));
            for (i=1;i<n;i++) {
                j2 = 0;
                for (j=0;j<n;j++) {
                    if (j == j1) continue;
                    m[(i-1)*(n-1)+j2] = a[i*n+j];
                    j2++;
                }
            }
            det += pow(-1.0,1.0+j1+1.0) * a[j1] * determinantrec(m,n-1);
            free(m);
        }
    }
    return(det);
}

double matDeterminant(matrix *mat)
{
    if (mat->rs!=mat->cs)
    {
        return 0.0;
    }

    return determinantrec(mat->values,mat->rs);
}

double matTrace(matrix *mat)
{
    int i;
    double trace;

    if (mat->rs!=mat->cs) return 0.0;

    trace=0.0;

    for (i=0;i<mat->cs;i++)
    {
        trace+=mat->values[i*mat->rs+i];
    }

    return trace;
}

void matPrint(matrix *mat)
{
    int i,j,index;

    if (mat==NULL) return;
    printf("%d %d\n",mat->cs,mat->rs);
    index=0;
    for (i=0;i<mat->cs;i++)
    {
        for (j=0;j<mat->rs;j++)
        {
            printf("%12.6f\t",mat->values[index]);
            index++;
        }
        printf("\n");    
    }
    printf("\n");
}

int matSolveA(matrix **res,matrix *A)
{
    int i;
    matrix *U,*D,*V;

    if (res==NULL || A==NULL) return -1; 
    if ((*res)==A) return -1;

    U=NULL;
    D=NULL;
    V=NULL;

    if (matSVD(A,&U,&D,&V)) return -1;

    matAllocate(res,A->rs,1);

    for (i=0;i<A->rs;i++) (*res)->values[i]=V->values[(V->cs-1)*V->rs+i];

    matFree(&U);
    matFree(&D);
    matFree(&V);

    return 0;
}

int matSolveAb(matrix **res,matrix *A,matrix *b)
{
    matrix *U,*d,*V;

    if (res==NULL || A==NULL || b==NULL) return -1; 
    if ((*res)==A || (*res)==b) return -1;

    if (A->rs>A->cs)
    {
        fprintf(stderr,"Error in SVD: number of equations smaller than number of unknowns.");
        return -1;
    }

    U=NULL;
    d=NULL;
    V=NULL;

    matAllocate(&V,A->rs,A->rs);
    matAllocate(&d,1,A->rs);
    matCopy(&U,A);

    matAllocate(res,A->rs,1);

    SVDcmp(U->values, A->cs, A->rs, d->values, V->values);
    SVDb(U->values, d->values, V->values, A->cs, A->rs, b->values, (*res)->values);

    matFree(&U);
    matFree(&d);
    matFree(&V);

    return 0;
}

//
//  A=U.D.V
//  (contrairement a Mathematica qui retourne V tel que A=U.D.Transpose[V])
//  Les vecteurs propres sont donc les rangees de V
//
int matSVD(matrix *A,matrix **U,matrix **D, matrix **V)
{
    int i,j;
    matrix *d;
    matrix *utemp;
    matrix *dtemp;
    matrix *vtemp;
    int start;
    int maxIndex;
    double maxValue=-1;
    int sort;

    if (A==NULL || U==NULL || D==NULL || V==NULL) return -1;
    if (A==(*U) || A==(*D) || A==(*V)) return -1;
    if (U==D || U==V) return -1;
    if (D==V) return -1;

    sort=1;
    d=NULL;
    utemp=NULL;
    dtemp=NULL;
    vtemp=NULL;

    //if (A->rs>A->cs)
    //{
    //    fprintf(stderr,"Error in SVD: number of equations smaller than number of unknowns!\n");
    //    return -1;
    //}

    if (A->rs<=A->cs)
    {
      matCopy(U,A);
    }
    else //pad A (copied in U) with 0s so that it becomes square
    {
      matAllocate(U,A->rs,A->rs);    
      matCopy(U,A);
      (*U)->cs=A->rs;
    }
    matAllocate(V,A->rs,A->rs);
    matAllocate(&d,1,A->rs);

    SVDcmp((*U)->values, (*U)->cs, (*U)->rs, d->values, (*V)->values);

    maxIndex=0;
    //sort eigenvalues and eigenvectors  
    if (sort)
    {
        matCopy(&utemp,(*U));
        matCopy(&dtemp,d);
        matCopy(&vtemp,(*V));

        for(i=0;i<A->rs;i++)
        {
            //find max eigenvalue not equal to -1.0
            start=1;
            for(j=0;j<A->rs;j++)
            {
                if (dtemp->values[j]==-1.0) continue;
                if (start || dtemp->values[j]>maxValue)
                {
                    start=0;
                    maxIndex=j;
                    maxValue=dtemp->values[j];
                }
            }

            dtemp->values[maxIndex]=-1.0;
            d->values[i]=maxValue;
            for(j=0;j<A->cs;j++)
            {
                matSet((*U),j,i,matGet(utemp,j,maxIndex));
            }
            for(j=0;j<A->rs;j++)
            {
                matSet((*V),j,i,matGet(vtemp,j,maxIndex));
            }
        }
        matFree(&utemp);
        matFree(&dtemp);
        matFree(&vtemp);
    }

    matTranspose(V,(*V));
    matAllocate(D,A->rs,A->rs);
    for (i=0;i<A->rs;i++) matSet((*D),i,i,d->values[i]);

    matFree(&d);

    return 0;
}

int matIdentity(matrix **res,int size)
{
    int i;

    if (res==NULL) return -1;
    if (size<1) return -1;

    matAllocate(res,size,size);

    for (i=0;i<size;i++) (*res)->values[i*size+i]=1.0;

    return 0;
}

int mat4CrossProduct(vector3 t,matrix4 m)
{
    m[0]=0.0;
    m[1]=-t[2];
    m[2]=t[1];
    m[3]=0.0;
    m[4]=t[2];
    m[5]=0.0;
    m[6]=-t[0];
    m[7]=0.0;
    m[8]=-t[1];
    m[9]=t[0];
    m[10]=0.0;
    m[11]=0.0;
    m[12]=0.0;
    m[13]=0.0;
    m[14]=0.0;
    m[15]=1.0;

    return 0;
}

int mat4XRotation(double degrees,matrix4 m)
{
    double rad=degrees/180*M_PI;

    m[0]=1.0;
    m[1]=0.0;
    m[2]=0.0;
    m[3]=0.0;
    m[4]=0.0;
    m[5]=cos(rad);
    m[6]=-sin(rad);
    m[7]=0.0;
    m[8]=0.0;
    m[9]=sin(rad);
    m[10]=cos(rad);
    m[11]=0.0;
    m[12]=0.0;
    m[13]=0.0;
    m[14]=0.0;
    m[15]=1.0;

    return 0;
}

int mat4YRotation(double degrees,matrix4 m)
{
    double rad=degrees/180*M_PI;

    m[0]=cos(rad);
    m[1]=0.0;
    m[2]=sin(rad);
    m[3]=0.0;
    m[4]=0.0;
    m[5]=1.0;
    m[6]=0.0;
    m[7]=0.0;
    m[8]=-sin(rad);
    m[9]=0.0;
    m[10]=cos(rad);
    m[11]=0.0;
    m[12]=0.0;
    m[13]=0.0;
    m[14]=0.0;
    m[15]=1.0;

    return 0;
}

int mat4ZRotation(double degrees,matrix4 m)
{
    double rad=degrees/180*M_PI;

    m[0]=cos(rad);
    m[1]=-sin(rad);
    m[2]=0.0;
    m[3]=0.0;
    m[4]=sin(rad);
    m[5]=cos(rad);
    m[6]=0.0;
    m[7]=0.0;
    m[8]=0.0;
    m[9]=0.0;
    m[10]=1.0;
    m[11]=0.0;
    m[12]=0.0;
    m[13]=0.0;
    m[14]=0.0;
    m[15]=1.0;

    return 0;
}

int mat4RotationFromAxis(vector3 t,matrix4 m)
{
    double theta;
    matrix *I,*w;
    matrix *Rtemp1,*Rtemp2,*Rtemp3;
    matrix *R1,*R2;
    double axis[3];

    theta=sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]);

    I=NULL;
    w=NULL;
    Rtemp1=NULL;
    Rtemp2=NULL;
    Rtemp3=NULL;
    R1=NULL;
    R2=NULL;

    if (theta<MAT_SMALL)
    {
        axis[0]=t[0];
        axis[1]=t[1];
        axis[2]=t[2];
        matIdentity(&I,3);
        matAllocate(&w,3,3);
        matSet(w,0,1,-axis[2]);
        matSet(w,0,2,axis[1]);
        matSet(w,1,0,axis[2]);
        matSet(w,1,2,-axis[0]);
        matSet(w,2,0,-axis[1]);
        matSet(w,2,1,axis[0]);

        matAdd(&R2,I,w);
    }
    else
    {
        axis[0]=t[0]/theta;
        axis[1]=t[1]/theta;
        axis[2]=t[2]/theta;

        matIdentity(&Rtemp1,3);
        matScale(&Rtemp1,Rtemp1,cos(theta));

        matAllocate(&Rtemp2,3,3);
        matSet(Rtemp2,0,0,axis[0]*axis[0]);
        matSet(Rtemp2,0,1,axis[0]*axis[1]);
        matSet(Rtemp2,0,2,axis[0]*axis[2]);
        matSet(Rtemp2,1,0,axis[1]*axis[0]);
        matSet(Rtemp2,1,1,axis[1]*axis[1]);
        matSet(Rtemp2,1,2,axis[1]*axis[2]);
        matSet(Rtemp2,2,0,axis[2]*axis[0]);
        matSet(Rtemp2,2,1,axis[2]*axis[1]);
        matSet(Rtemp2,2,2,axis[2]*axis[2]);
        matScale(&Rtemp2,Rtemp2,1.0-cos(theta));

        matAllocate(&Rtemp3,3,3);
        matSet(Rtemp3,0,1,-axis[2]);
        matSet(Rtemp3,0,2,axis[1]);
        matSet(Rtemp3,1,0,axis[2]);
        matSet(Rtemp3,1,2,-axis[0]);
        matSet(Rtemp3,2,0,-axis[1]);
        matSet(Rtemp3,2,1,axis[0]);
        matScale(&Rtemp3,Rtemp3,sin(theta));

        matAdd(&R1,Rtemp1,Rtemp2);
        matAdd(&R2,R1,Rtemp3);
    }

    m[0]=matGet(R2,0,0);
    m[1]=matGet(R2,0,1);
    m[2]=matGet(R2,0,2);
    m[3]=0.0;
    m[4]=matGet(R2,1,0);
    m[5]=matGet(R2,1,1);
    m[6]=matGet(R2,1,2);
    m[7]=0.0;
    m[8]=matGet(R2,2,0);
    m[9]=matGet(R2,2,1);
    m[10]=matGet(R2,2,2);
    m[11]=0.0;
    m[12]=0.0;
    m[13]=0.0;
    m[14]=0.0;
    m[15]=1.0;

    matFree(&I);
    matFree(&w);

    matFree(&Rtemp1);
    matFree(&Rtemp2);
    matFree(&Rtemp3);

    matFree(&R1);
    matFree(&R2);

    return 0;
}

int mat4RotationToQuaternion(matrix4 m,vector4 t)
{
    double trace,s;
    matrix *rot;
    rot=NULL;

    matAllocate(&rot,4,4);
    mat4Copy(m,rot->values);

    trace=matGet(rot,0,0)+matGet(rot,1,1)+matGet(rot,2,2)+1;

    if (trace>1e-8)
    {
        s=0.5/sqrt(trace);
        t[0]=(matGet(rot,2,1)-matGet(rot,1,2))*s;
        t[1]=(matGet(rot,0,2)-matGet(rot,2,0))*s;
        t[2]=(matGet(rot,1,0)-matGet(rot,0,1))*s;
        t[3]=0.25/s;
    }
    else if (matGet(rot,0,0)>matGet(rot,1,1) && matGet(rot,0,0)>matGet(rot,2,2))
    {
        s=2.0*sqrt(1.0+matGet(rot,0,0)-matGet(rot,1,1)-matGet(rot,2,2));
        t[0]=1.0/(2*s);
        t[1]=(matGet(rot,0,1)-matGet(rot,1,0))/s;
        t[2]=(matGet(rot,0,2)-matGet(rot,2,0))/s;
        t[3]=(matGet(rot,1,2)-matGet(rot,2,1))/s;
    }
    else if (matGet(rot,1,1)>matGet(rot,0,0) && matGet(rot,1,1)>matGet(rot,2,2))
    {
        s=2.0*sqrt(1.0-matGet(rot,0,0)+matGet(rot,1,1)-matGet(rot,2,2));
        t[0]=(matGet(rot,0,1)-matGet(rot,1,0))/s;
        t[1]=1.0/(2*s);
        t[2]=(matGet(rot,1,2)-matGet(rot,2,1))/s;
        t[3]=(matGet(rot,0,2)-matGet(rot,2,0))/s;
    }
    else if (matGet(rot,2,2)>matGet(rot,0,0) && matGet(rot,2,2)>matGet(rot,1,1))
    {
        s=2.0*sqrt(1.0-matGet(rot,0,0)-matGet(rot,1,1)+matGet(rot,2,2));
        t[0]=(matGet(rot,0,2)-matGet(rot,2,0))/s;
        t[1]=(matGet(rot,1,2)-matGet(rot,2,1))/s;
        t[2]=1.0/(2*s);
        t[3]=(matGet(rot,0,1)-matGet(rot,1,0))/s;
    }
    else
    {
      matFree(&rot);
      return -1;
    }

    matFree(&rot);

    return 0;
}

int mat4RotationToAxis(matrix4 m,vector3 t)
{
    int ret;
    double q[4];
    double qlen,tlen;
    double angle;

    ret=mat4RotationToQuaternion(m,q);
    if (ret==-1) return -1;

    qlen=sqrt(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);
    if (qlen<MAT_SMALL) return -1;

    q[0]/=qlen;
    q[1]/=qlen;
    q[2]/=qlen;
    q[3]/=qlen;

    angle = acos(q[3]) * 2;

    t[0] = q[0];
    t[1] = q[1];
    t[2] = q[2];

    tlen=sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]);
    if (tlen<MAT_SMALL) return -1;

    t[0]/=tlen;
    t[1]/=tlen;
    t[2]/=tlen;

    t[0]*=angle;
    t[1]*=angle;
    t[2]*=angle;

    return 0;
}


int mat4NormalizeRotation(matrix4 m)
{
    matrix *temp,*mnorm;
    matrix *U,*D,*V;

    U=NULL;
    D=NULL;
    V=NULL;
    temp=NULL;
    mnorm=NULL;
    matAllocate(&temp,4,4);
    mat4Copy(m,temp->values);

    matSVD(temp,&U,&D,&V);
    mat4Identity(D->values);
    matMultiply(&temp,U,D);
    matMultiply(&mnorm,temp,V);

    mat4Copy(mnorm->values,m);

    matFree(&U);
    matFree(&D);
    matFree(&V);
    matFree(&temp);
    matFree(&mnorm);

    return 0;
}

double mat4Determinant(matrix4 m) {
    return determinantrec(m,4);
}

/**
 * Fits a curve of degree @p d with points @p n points x,y
 *
 * @param[in] x Array containing @p n x coordinates
 * @param[in] y Array containing @p n y coordinates
 * @param[in] n Number of points
 * @param[out] coeffs Array that will contain the @p d+1 curve coefficients 
 * @param[out] d Curve degree
 *
 * @return 0 : success; <0 : error
 * 
 */
int matFit(double *x,double *y,int n,double *coeffs,int d)
{
  int i,j;
  matrix *A,*res,*b;
  double temp;

  if (d<0) return -1;
  if (n<d+1) return -1;
  if (x==NULL || y==NULL || coeffs==NULL) return -1;

  A=NULL;
  res=NULL;
  b=NULL;

  matAllocate(&A,n,d+1);
  matAllocate(&res,d+1,1);
  matAllocate(&b,n,1);

  //a row of A is as follows: x^d x^d-1 ... x 1
  for (i=0;i<n;i++)
  {
    temp=1.0;
    matSet(A,i,d,temp);
    for (j=0;j<d;j++)
    {
      temp*=x[i];
      matSet(A,i,d-1-j,temp);
    }
  }  
  vectxCopy(y,b->values,n);

  matSolveAb(&res,A,b);

  vectxCopy(res->values,coeffs,res->cs);

  matFree(&A);
  matFree(&res);
  matFree(&b);

  return 0;
}

int matEval(double *x,double *y,int n,double *coeffs,int d)
{
  int i,j;
  double temp;

  if (d<0) return -1;
  if (n<d+1) return -1;
  if (x==NULL || y==NULL || coeffs==NULL) return -1;

  for (i=0;i<n;i++)
  {
    temp=1.0;
    y[i]=coeffs[d];
    for (j=0;j<d;j++)
    {
      temp*=x[i];
      y[i]+=temp*coeffs[d-1-j];
    }
  }

  return 0;
}

//equation is Ax^3+Bx^2+Cx+D
int matFitCubicSpline(matrix **Z,matrix **h,matrix *points)
{
    int i;
    int n;
    double temp,temp2;
    matrix *b,*u, *v, *S;

    if (points==NULL || Z==NULL || h==NULL) return -1;
    if (points->rs<2 || points->cs<2) return -1;

    b=NULL;
    u=NULL;
    v=NULL;
    S=NULL;

    n=points->cs;

    matAllocate(Z,n,1);
    matAllocate(h,n-1,1);

    //Temporary values often used in calculating Si (keeping the same notation as in Cheney & Kincaid)
    matAllocate(&b,n-1,1);
    matAllocate(&u,n-1,1);
    matAllocate(&v,n-1,1);
    matAllocate(&S,n-1,1);

    //h(i)=x(i+1)-x(i)
    //b(i)=(y(i+1)-y(i))/h(i)
    i = 0;
    while(i < n - 1)
    {
        (*h)->values[i]=matGet(points,i+1,0)-matGet(points,i,0);
        temp=matGet(points,i+1,1)-matGet(points,i,1);
        b->values[i]=temp/(*h)->values[i];
        i++;
    }

    //u(1)=2*(h(0)+h(1))
    //v(1)=6*(b(1)-b(0))
    //Note that index of u,v start at 1 as opposed to 0 for all other indexes
    u->values[1]=2*((*h)->values[0]+(*h)->values[1]);
    v->values[1]=6*(b->values[1]-b->values[0]);

    //u(i)=2*(h(i-1)+h(i))-(h(i-1))^2/u(i-1)
    //v(i)=6*(b(i)-b(i-1))-h(i)*v(i-1)/u(i-1)
    i = 2;
    while(i < n-1)
    {
        temp=2*((*h)->values[i-1]+(*h)->values[i]);
        temp2=(*h)->values[i-1]*(*h)->values[i-1];
        temp2=temp2/u->values[i-1];
        u->values[i]=temp-temp2;

        temp=6*(b->values[i]-b->values[i-1]);

        temp2=(*h)->values[i-1]*v->values[i-1];
        temp2=temp2/u->values[i-1];
        v->values[i]=temp-temp2;
        i++;
    }

    (*Z)->values[n-1]=0;
    //Z(i)=(v(i)-h(i)*Z(i+1))/u(i)
    //Note that, in loop, Z(0) corresponds to current Z(i+1)
    i = n - 2;
    while(i >= 1)
    {
        temp=(*h)->values[i]*(*Z)->values[i+1];
        temp=v->values[i]-temp;
        (*Z)->values[i]=temp/u->values[i];
        i--;
    }
    (*Z)->values[0]=0;

    matFree(&b);
    matFree(&u);
    matFree(&v);
    matFree(&S);

    return 0;
}

int matEvalCubicSpline(matrix *Z,matrix *h,matrix *points,double x,double *y)
{
    int i,n;
    double A,B,C,D;
    double temp;
    double bmin,bmax;

    if (points==NULL || Z==NULL || h==NULL) return -1;
    if (y==NULL) return -1;
    if (points->rs!=2) return -1;
    if (points->cs!=Z->cs) return -1;
    if (points->cs!=h->cs+1) return -1;

    n=points->cs;

    i = 0;
    while(i < n-1)
    {
      bmin=matGet(points,i,0);
      bmax=matGet(points,i+1,0);
      if (bmin>bmax)
      {
        temp=bmin;
        bmin=bmax;
        bmax=temp;
      }
      if (x>=bmin && x<=bmax)
      {  
        //Finding A,B,C,D in Ax^3+Bx^2+Cx+D
        A=matGet(points,i,1);
        B=-h->values[i]/6*Z->values[i+1];
        B-=Z->values[i]*h->values[i]/3;
        B+=(matGet(points,i+1,1)-matGet(points,i,1))/h->values[i];
        C=Z->values[i]/2;

        temp=Z->values[i+1]-Z->values[i];
        D=temp/(6*h->values[i]);

        temp=x-matGet(points,i,0);
        temp=D*temp*temp*temp+C*temp*temp+B*temp+A;
        (*y)=temp;
        return 0;
      }
      i++;
    }

    return -1;
}

double matRandNumber(void)
{
    double rd;
#ifdef WIN32
    rd=(double)(rand())/RAND_MAX;
#else
    rd=drand48();
#endif
    return rd;
}

void matRandInit(void)
{
#ifdef WIN32
    srand((unsigned int)time(NULL));
#else
    srand48((unsigned int)time(NULL));
#endif
}

void matRandSeed(unsigned int s)
{
#ifdef WIN32
    srand(s);
#else
    srand48(s);
#endif
}

static int compare_errors(const void *a, const void *b){
    const double *a_error = (const double *)a;
    const double *b_error = (const double *)b;
    return a_error[0] < b_error[0] ? -1 : (a_error[0] == b_error[0] ? 0 : 1);
}

/**
 * Apply a robust RANSAC estimation of model function @p modelfunc on input data @p pts1 and @p pts2, using error function @p errorfunc to measure error
 * 
 * @param[in] pts1 matrix pointer of size NxD containing N input data points of dimension D
 * @param[in] pts2 matrix pointer of size NxD containing corrresponfind data points to @p pts1. This pointer can be NULL is no correspondances are needed.
 * @param[in] nbsamples Minimum number of data samples required to evaluate the model function.
 * @param[in] nbiterations maximum number of iterations.
 * @param[in] validamount value between 0.0 and 1.0 indicating the expected amount of inliers. (1.0 means: 100% of data points are inliers.)
 * @param[in] threshold acceptable maximum error to stop iterations before @p nbiterations. If < 0, this parameter has no effect.
 * @param[in] modelfunc pointer to a function that estimates a function matrix @p mf from input data points, where pts2 can be NULL. 
 * @param[in] errorfunc pointer to a function that computes errors of function matrix @p mf on input data points, where pts2 can be NULL. . 
 * @param[out] hres address of matrix pointer that will contain estimated matrix function
 * @param[out] errors ???
 * @param[out] inliers array of size N that will be filled with either 1 (if inlier) or 0 (if outlier). Can be NULL if this information is not required.
 *
 * @return 0 : success; <0 : error
 * 
 */
int matRansac(matrix *pts1,matrix *pts2,int nbsamples,int nbiterations,double validamount,double threshold, int (*modelfunc)(matrix *,matrix *,matrix **),int (*errorfunc)(matrix *,matrix *,matrix *,double *),matrix **hres,double *errors,unsigned char *inliers)
{
    int i,j,k,l;
    matrix *htemp;
    matrix *msamples1,*msamples2;
    unsigned char *mselected;
    double rd;
    int rdpos;
    int mid;
    int count,mcount;
    double merror;
    double *derrors,*sorted_errors;
    int N,D;

    if (pts1==NULL) return -1;
    if (pts2!=NULL && (pts1->cs!=pts2->cs || pts1->rs!=pts2->rs)) return -1;

    N=pts1->cs; //N is the number of matches
    D=pts1->rs; //D is the dimension size

    htemp=NULL;
    msamples1=NULL;
    msamples2=NULL;
    mcount=0;

    if (hres==NULL) return -1;
    if (N<nbsamples) return -1;

    matAllocate(&msamples1,nbsamples,D);
    if (pts2!=NULL) matAllocate(&msamples2,nbsamples,D);
    mselected=(unsigned char *)(malloc(sizeof(unsigned char)*N));
    derrors=(double *)(malloc(sizeof(double)*N));
    sorted_errors=(double *)(malloc(sizeof(double)*N));

    mid=(int)((N-1)*validamount+0.5);
    if (mid>=N) mid=N-1;
    if (mid<0) mid=0;

    i=0;
    while(i<nbiterations)
    {
        for(j=0;j<N;j++)
        {
            mselected[j]=0;
        }
        k=0;
        while(k<nbsamples)
        {
            rd=matRandNumber();
            rdpos=(int)(rd*N);
            if (rdpos>=N) continue;
            if (mselected[rdpos]==1) continue;

            mselected[rdpos]=1;
            for (j=0;j<D;j++)
            {
              msamples1->values[k*D+j]=pts1->values[rdpos*D+j];
              msamples2->values[k*D+j]=pts2->values[rdpos*D+j];
            }
            k++;
        }
        (*modelfunc)(msamples1,msamples2,&htemp);
        (*errorfunc)(pts1,pts2,htemp,derrors);
        //matPrint(htemp);

        for (l=0;l<N;l++) sorted_errors[l]=derrors[l];
        //for (l=0;l<N;l++) fprintf(stderr,"E: %f\n",derrors[l]);
        qsort(sorted_errors, N, sizeof(double), compare_errors);
        //for (l=0;l<N;l++) fprintf(stderr,"Esorted: %f\n",derrors[l]);

        if (threshold<0) //don't use threshold: choose the solution that gives lowest at validamount sample
        {
          if (i==0 || (sorted_errors[mid]>=0 && sorted_errors[mid]<merror))
          {
            merror=sorted_errors[mid];
            matCopy(hres,htemp);
            if (inliers!=NULL)
            {
              for(j=0;j<N;j++)
              {
                if (derrors[j]>=0 && derrors[j]<=merror+1e-8) inliers[j]=1;
                else inliers[j]=0; 
              }
            }
          }
          //if (merror<threshold) break;
        }
        else //compute the number of samples with error<threshold; if number > validamount, we are done.
        {
          count=0;
          for(j=0;j<N;j++)
          {
              if (derrors[j]>=0 && derrors[j]<=threshold+1e-8) count++;
          }          
          if (i==0 || count>mcount)
          {
            mcount=count;
            matCopy(hres,htemp);
            if (inliers!=NULL)
            {
              for(j=0;j<N;j++)
              {
                if (derrors[j]>=0 && derrors[j]<=threshold+1e-8) inliers[j]=1;
                else inliers[j]=0; 
              }
            }
          }
          if (mcount>=mid) break;
        }
        i++;
    }

    if (errors!=NULL)
    {
      (*errorfunc)(pts1,pts2,(*hres),derrors);
      for (i=0;i<N;i++) errors[i]=derrors[i];
    }

    free(derrors);
    free(sorted_errors);
    free(mselected);
    matFree(&msamples1);
    matFree(&msamples2);

    return 0;
}

int matExtractInliers(matrix *pts,unsigned char *iflags,matrix **inliers)
{
    int i,j,k;
    int count,N,D;
    unsigned char realloc;
    matrix *ptscpy;

    if (pts==NULL) return -1;
    if (iflags==NULL) return -1;
    if (inliers==NULL) return -1;

    N=pts->cs;
    D=pts->rs;

    count=0;
    for (i=0;i<N;i++) 
    {
      if (iflags[i]!=0) count++;
    }

    if (count<=0) return -1;

    if((*inliers)==pts)
    {
        ptscpy=NULL;
        matCopy(&ptscpy,pts);
        realloc=1;
    }
    else
    {
        ptscpy=pts;
        realloc=0;
    }
    if(matAllocate(inliers,count,D)) {if (realloc) {matFree(&ptscpy);}return(-1);}

    k=0;
    for (i=0;i<N;i++)
    {
        if (iflags[i])
        {
            for (j=0;j<D;j++)
            {
                matSet((*inliers),k,j,matGet(ptscpy,i,j));
            }
            k++; 
        }
    }

    if (realloc) matFree(&ptscpy);

    return 0;
}

//
// pts1 : N x 2 , coord (x,y) observe
// pts2 : N x 2 , coord (u,v) correspondantes (reference)


int matHomographySolve(matrix *pts1,matrix *pts2,matrix **hres)
{
    int i,j;
    matrix *A,*b;
    matrix *T1,*T2,*T2inv;
    matrix *mtemp;
    double det;
    int N;

    if (pts1==NULL || pts2==NULL ||hres==NULL) return -1;
    if (pts1->rs<2) return -1;
    if (pts1->cs!=pts2->cs || pts1->rs!=pts2->rs) return -1;

    T1=NULL;
    T2=NULL;
    T2inv=NULL;
    mtemp=NULL;

    // Normalisation des points
    matrix *npts1=NULL;
    matrix *npts2=NULL;
    matNormalizePoints(pts1,&npts1,&T1);
    matNormalizePoints(pts2,&npts2,&T2);

    matAllocate(hres,3,3);

    A=NULL;
    b=NULL;

    //b = [-points(:, 3); -points(:, 4)];
    //A = [-points(:, 1), -points(:, 2), -ones(size(points, 1), 1), zeros(size(points, 1), 3), points(:, 1) .* points(:, 3), points(:, 2) .* points(:, 3);
    //     zeros(size(points, 1), 3), -points(:, 1), -points(:, 2), -ones(size(points, 1), 1), points(:, 1) .* points(:, 4), points(:, 2) .* points(:, 4) ];
    //x = A\b;
    //homography = [x(1), x(2), x(3); x(4), x(5), x(6); x(7), x(8), 1];

    N=pts1->cs;

    matAllocate(&A,2*N,8);
    matAllocate(&b,2*N,1);

    for (i=0;i<N;i++)
    {
        matSet(A,i,0,-matGet(npts1,i,0));
        matSet(A,i,1,-matGet(npts1,i,1));
        matSet(A,i,2,-1.0);
        matSet(A,i,3,0.0);
        matSet(A,i,4,0.0);
        matSet(A,i,5,0.0);
        matSet(A,i,6,matGet(npts1,i,0)*matGet(npts2,i,0));
        matSet(A,i,7,matGet(npts1,i,1)*matGet(npts2,i,0));
        b->values[i]=-matGet(npts2,i,0);
    }

    for (i=0;i<N;i++)
    {
        j=N+i;
        matSet(A,j,0,0.0);
        matSet(A,j,1,0.0);
        matSet(A,j,2,0.0);
        matSet(A,j,3,-matGet(npts1,i,0));
        matSet(A,j,4,-matGet(npts1,i,1));
        matSet(A,j,5,-1.0);
        matSet(A,j,6,matGet(npts1,i,0)*matGet(npts2,i,1));
        matSet(A,j,7,matGet(npts1,i,1)*matGet(npts2,i,1));
        b->values[j]=-matGet(npts2,i,1);
    }

    matSolveAb(hres,A,b);
    (*hres)->cs=3;
    (*hres)->rs=3;
    (*hres)->values[8]=1.0;

    matInverse(&T2inv,T2);
    matMultiply(&mtemp,T2inv,(*hres));
    matMultiply(hres,mtemp,T1);

    //force |H|=1
    det=matDeterminant((*hres));
    //printf("DET %f\n",det);
    if (det>1e-8)
    {
        det=pow(1.0/det,1.0/3.0);
        for(i=0;i<9;i++)
        {
            (*hres)->values[i]*=det;
        }
    }

    matFree(&A);
    matFree(&b);
    matFree(&T1);
    matFree(&T2);
    matFree(&T2inv);
    matFree(&mtemp);
    matFree(&npts1);
    matFree(&npts2);

    return 0;
}

int matHomographyError(matrix *pts1,matrix *pts2,matrix *hres,double *errors)
{
    int k;
    matrix *p1,*p2;
    double error;
    int N;

    p1=NULL;
    p2=NULL;

    if (pts1==NULL || pts2==NULL ||hres==NULL || errors==NULL) return -1;
    if (hres->cs!=3 || hres->rs!=3) return -1;
    if (pts1->rs<2) return -1;
    if (pts1->cs!=pts2->cs || pts1->rs!=pts2->rs) return -1;

    N=pts1->cs;

    matAllocate(&p1,3,1);
    matAllocate(&p2,3,1);

    for(k=0;k<N;k++)
    {
        p1->values[0]=matGet(pts1,k,0);
        p1->values[1]=matGet(pts1,k,1);
        p1->values[2]=1.0;                        
        matMultiply(&p2,hres,p1);
        if (fabs(p2->values[2])<MAT_VERY_SMALL) 
        {
          //fprintf(stderr,"Warning: unable to normalize point when computing Homography error\n");
          errors[k]=-1.0;
          continue;
        }
        p2->values[0]/=p2->values[2];
        p2->values[1]/=p2->values[2];
        p2->values[2]=1.0;
        error=0;
        error+=(matGet(pts2,k,0)-p2->values[0])*(matGet(pts2,k,0)-p2->values[0]);
        error+=(matGet(pts2,k,1)-p2->values[1])*(matGet(pts2,k,1)-p2->values[1]);
        error=sqrt(error);
        errors[k]=error;
    }

    matFree(&p1);
    matFree(&p2);

    return 0;
}

static int normalization2d(matrix *pts, matrix **normMat) {
    int i,m;
    m=pts->cs;
    matAllocate(normMat,3,3);

    double xm,ym,sum,sumx,sumy; // position moyenne
    sumx=0;
    sumy=0;
    for (i=0 ; i<m ; i++) {
        sumx += matGet(pts,i,0);
        sumy += matGet(pts,i,1);
    }
    xm = sumx/m; 
    ym = sumy/m;

    sum = sumx = sumy = 0.0;
    double meanDist,scale; // distance moyenne a la position moyenne
    for (i=0 ; i<m ; i++) {
        sum += sqrt((xm-matGet(pts,i,0))*(xm-matGet(pts,i,0)) + 
                    (ym-matGet(pts,i,1))*(ym-matGet(pts,i,1)));
    }
    meanDist = sum/m;
    scale = sqrt(2.0)/meanDist;

    // Matrice de normalisation qui ramene tous les points centres
    // en (0,0) avec une distance moyenne de sqrt(2) du centre
    matSet(*normMat,0,0,scale);
    matSet(*normMat,0,1,0.0);
    matSet(*normMat,0,2,scale*(-xm));
    matSet(*normMat,1,0,0.0);
    matSet(*normMat,1,1,scale);
    matSet(*normMat,1,2,scale*(-ym));
    matSet(*normMat,2,0,0.0);
    matSet(*normMat,2,1,0.0);
    matSet(*normMat,2,2,1.0);

    return 1;
}

/**
 * Normalize a group of points so that its mean position is at (0,0) and
 * that their mean distance to the center is sqrt(2)
 * 
 * @param[in] pts Matrix containing @p n (x,y) coordinates
 * @param[out] ptsnorm Matrix containing the normalized coordinates 
 * @param[out] T Matrix ???
 *
 * @return 0 : success; <0 : error
 * 
 */
int matNormalizePoints(matrix *pts,matrix **ptsnorm,matrix **T)
{
    int i,m,n;
    unsigned char realloc;
    matrix *ptscpy;

    if (pts==NULL || ptsnorm==NULL || T==NULL) return -1;
    if (pts->rs!=2) return -1;

    m=pts->cs;
    n=pts->rs;

    if (pts==(*ptsnorm))
    {
        ptscpy=NULL;
        matCopy(&ptscpy,pts);
        realloc=1;
    }
    else
    {
        ptscpy=pts;
        realloc=0;
    }
    if(matAllocate(ptsnorm,m,n)) {if (realloc) {matFree(&ptscpy);}return(-1);}
    
    normalization2d(ptscpy, T);

    vector3 pt, npt;
    for (i=0 ; i<m ; i++) {
        pt[0]=matGet(ptscpy,i,0);
        pt[1]=matGet(ptscpy,i,1);
        pt[2]=1.0;
        mat3MultiplyVector((*T)->values,pt,npt);
        vect3Homogenize2D(npt);
        matSet(*ptsnorm,i,0,npt[0]);
        matSet(*ptsnorm,i,1,npt[1]);
    }

    if (realloc) matFree(&ptscpy);

    return 0;
}

/**
 * Computes the fundamental matrix that relates two sets of matching pixel
 * coordinates. The coordinates are normalized prior to computing the
 * fundamental matrix F in pts2.F.pts1=0
 *
 * @param[in] pts1 matrix pointer containing @p n (x,y) coordinates
 * @param[in] pts2 matrix pointer containing @p n (x,y) coordinates matching pts1
 * @param[out] fres address of matrix pointer in which will be stored the estimatesd fundamental matrix 
 *
 * @return 0 : success; <0 : error
 * 
 */
int matFundamentalSolve(matrix *pts1,matrix *pts2,matrix **fres)
{
    int i;
    matrix *A,*At;
    matrix *U,*D,*V;
    matrix *mtemp;
    matrix *T1,*T2,*T2t;
    int N;

    if (fres==NULL) return -1;
    if (pts1==NULL || pts2==NULL) return -1;
    if (pts1->rs<2 || pts2->rs<2) return -1;
    if (pts1->cs!=pts2->cs) return -1;
    if (pts1->cs<8) return -1;

    T1=NULL;
    T2=NULL;
    T2t=NULL;

    // Normalisation des points
    matrix *npts1=NULL;
    matrix *npts2=NULL;
    matSelectColumns(&npts1,pts1,0,2);
    matSelectColumns(&npts2,pts2,0,2);
    matNormalizePoints(npts1,&npts1,&T1);
    matNormalizePoints(npts2,&npts2,&T2);

    N=pts1->cs;

    A=NULL;
    At=NULL;
    U=NULL;
    D=NULL;
    V=NULL;
    mtemp=NULL;

    matAllocate(&A,N,9);

    // Construction de la matrice du systeme d'equations
    for(i=0;i<N;i++)
    {
        matSet(A,i,0,matGet(npts1,i,0)*matGet(npts2,i,0));
        matSet(A,i,1,matGet(npts1,i,1)*matGet(npts2,i,0));
        matSet(A,i,2,matGet(npts2,i,0));
        matSet(A,i,3,matGet(npts1,i,0)*matGet(npts2,i,1));
        matSet(A,i,4,matGet(npts1,i,1)*matGet(npts2,i,1));
        matSet(A,i,5,matGet(npts2,i,1));
        matSet(A,i,6,matGet(npts1,i,0));
        matSet(A,i,7,matGet(npts1,i,1));
        matSet(A,i,8,1.0);
    }
    //matPrint(A);
    matTranspose(&At,A);
    matMultiply(&A,At,A);

    // Resolution du systeme d'equation

    if (matSolveA(fres,A)) return -1;
    
    //matPrint((*fres));
    (*fres)->cs=3;
    (*fres)->rs=3;

    // Singulariser la matrice fondamentale
    matSVD((*fres), &U,&D,&V);

    matSet(D,2,2,0.0);
    matMultiply(&mtemp,U,D);
    matMultiply(fres,mtemp,V);

    //Denormaliser la fondamentale pour l'exprimer en pixels
    matTranspose(&T2t,T2);
    matMultiply(&mtemp,T2t,(*fres));
    matMultiply(fres,mtemp,T1);

    matFree(&A);
    matFree(&At);
    matFree(&U);
    matFree(&D);
    matFree(&V);
    matFree(&T1);
    matFree(&T2);
    matFree(&T2t);
    matFree(&mtemp);
    matFree(&npts1);  
    matFree(&npts2);

    return 0;    
} 


/**
 * Computes the error of each matching coordinate. The error is computed with
 * pt2.F.pt1.
 *
 * @param[in] pts1 Matrix containing @p n (x,y) coordinates
 * @param[in] pts2 Matrix containing @p n (x,y) coordinates matching pts1
 * @param[in] F Fundamental matrix
 * @param[out] errors Array of errors 
 *
 * @return 0 : success; <0 : error
 * 
 */
int matFundamentalError(matrix *pts1,matrix *pts2,matrix *F,double *errors)
{
    int k;
    matrix *p1,*p2,*temp;
    int N;

    p1=NULL;
    p2=NULL;
    temp=NULL;

    if (pts1==NULL || pts2==NULL || F==NULL || errors==NULL) return -1;
    if (pts1->rs<2 || pts2->rs<2) return -1;
    if (F->cs!=3 || F->rs!=3) return -1;
    if (pts1->cs!=pts2->cs) return -1;

    N=pts1->cs;

    matAllocate(&p1,3,1);
    matAllocate(&p2,1,3);

    // Compute the error for each set of matching points
    // pt2.F.pt1 = error
    for(k=0;k<N;k++)
    {
        // Computation of pt2.F
        p1->values[0]=matGet(pts1,k,0);
        p1->values[1]=matGet(pts1,k,1);
        p1->values[2]=1.0;
        p2->values[0]=matGet(pts2,k,0);
        p2->values[1]=matGet(pts2,k,1);
        p2->values[2]=1.0;                                                    
        matMultiply(&temp,p2,F);

        /*temp->values[0]/=temp->values[2];
        temp->values[1]/=temp->values[2];
        temp->values[2]=1.0;*/

        // Computation of (pt2.F).pt1
        matMultiply(&temp,temp,p1);
        errors[k]=temp->values[0]*temp->values[0];
    }

    matFree(&p1);
    matFree(&p2);
    matFree(&temp);

    return 0;
}

static int essential_normalize(matrix* E)
{
    double N;
    double trace;
    matrix *Et,*EtE;

    if (E==NULL) return -1;

    Et=NULL;
    EtE=NULL;

    matTranspose(&Et,E);
    matMultiply(&EtE,Et,E);

    trace=matTrace(EtE);

    if (trace<MAT_VERY_SMALL) return -1;

    N=sqrt(trace/2.0);

    matScale(&E,E,1.0/N);

    matFree(&Et);
    matFree(&EtE);

    return 0;
}

int matFundamentalToEssential(matrix3 k1, matrix3 k2, matrix3 f, matrix3 ess)
{
	// E = Transpose[K2].F.K1
    matrix3 Tk2;
    matrix3 Tk2f;
	
	mat3Transpose(k2,Tk2);    // Transpose[k2]
    mat3Multiply(Tk2,f,Tk2f); // Transpose[k2].F
    mat3Multiply(Tk2f,k1,ess); // Transpose[k2].F.k1

    // {u, d, v} = SingularValueDecomposition[Es];
    // d[[1, 1]] = 1;
    // d[[2, 2]] = 1;
    // Ess = u.d.v;

    matrix *A;
    matrix *U,*D,*V;
    matrix *mtmp;
    matrix *E;

    A=NULL;
    U=NULL;
    D=NULL;
    V=NULL;
    mtmp=NULL;
    E=NULL;

    matAllocate(&E,3,3);
    mat3Copy(ess,E->values);

    matSVD(E, &U,&D,&V);

    matSet(D,0,0,1.0);
    matSet(D,1,1,1.0);

    matMultiply(&mtmp,U,D);
    matMultiply(&E,mtmp,V);

    essential_normalize(E);

    mat3Copy(E->values,ess);

    matFree(&U);
    matFree(&V);
    matFree(&A);
    matFree(&D);
    matFree(&mtmp);
    matFree(&E);

    return 0;
}

int matEssentialSolve(matrix *pts1,matrix *pts2,matrix **hres)
{
    int i;
    matrix *A,*At;
    matrix *U,*D,*V;
    matrix *mtemp;
    int minIndex;
    double minValue;
    int N;

    if (hres==NULL) return -1;
    if (pts1==NULL || pts2==NULL) return -1;
    if (pts1->cs<8) return -1;
    if (pts1->rs!=3) return -1;
    if (pts1->cs!=pts2->cs || pts1->rs!=pts2->rs) return -1;

    N=pts1->cs;

    A=NULL;
    At=NULL;
    U=NULL;
    D=NULL;
    V=NULL;
    mtemp=NULL;

    matAllocate(&A,N,9);

    for(i=0;i<N;i++)
    {
        matSet(A,i,0,matGet(pts2,i,0)*matGet(pts1,i,0));
        matSet(A,i,1,matGet(pts2,i,0)*matGet(pts1,i,1));
        matSet(A,i,2,matGet(pts2,i,0)*matGet(pts1,i,2));
        matSet(A,i,3,matGet(pts2,i,1)*matGet(pts1,i,0));
        matSet(A,i,4,matGet(pts2,i,1)*matGet(pts1,i,1));
        matSet(A,i,5,matGet(pts2,i,1)*matGet(pts1,i,2));
        matSet(A,i,6,matGet(pts2,i,2)*matGet(pts1,i,0));
        matSet(A,i,7,matGet(pts2,i,2)*matGet(pts1,i,1));
        matSet(A,i,8,matGet(pts2,i,2)*matGet(pts1,i,2));
    }

    matTranspose(&At,A);
    matMultiply(&A,At,A);
    matSolveA(hres,A);
    (*hres)->cs=3;
    (*hres)->rs=3;

    matSVD((*hres), &U,&D,&V);

    for (i=0;i<3;i++)
    {
        if (i==0 || minValue>matGet(D,i,i))
        {
            minIndex=i;
            minValue=matGet(D,i,i);
        }
    }

    matSet(D,minIndex,minIndex,0.0);

    matMultiply(&mtemp,U,D);
    matMultiply(hres,mtemp,V);

    essential_normalize((*hres));

    matFree(&A);
    matFree(&At);
    matFree(&U);
    matFree(&D);
    matFree(&V);
    matFree(&mtemp);

    return 0;
}

int matEssentialError(matrix *pts1,matrix *pts2,matrix *hres,double *errors)
{
    int k;
    matrix *p1,*p2;
    matrix *temp,*perror;
    int N;

    p1=NULL;
    p2=NULL;
    temp=NULL;
    perror=NULL;

    if (hres==NULL || errors==NULL) return -1;
    if (pts1==NULL || pts2==NULL) return -1;
    if (pts1->rs!=3) return -1;
    if (pts1->cs!=pts2->cs || pts1->rs!=pts2->rs) return -1;

    N=pts1->cs;

    matAllocate(&p1,1,3);
    matAllocate(&p2,3,1);

    for(k=0;k<N;k++)
    {
        p1->values[0]=matGet(pts2,k,0);
        p1->values[1]=matGet(pts2,k,1);
        p1->values[2]=matGet(pts2,k,2);
        p2->values[0]=matGet(pts1,k,0);
        p2->values[1]=matGet(pts1,k,1);
        p2->values[2]=matGet(pts1,k,2);
        matMultiply(&temp,p1,hres);
        matMultiply(&perror,temp,p2);
        errors[k]=perror->values[0]*perror->values[0];
    }

    matFree(&p1);
    matFree(&p2);
    matFree(&temp);
    matFree(&perror);

    return 0;
}

//matrix *mat,matrix **res
static int matSwitchDeterminant(matrix **mat) 
{

    if (mat==NULL) return -1;

    int i;
    matrix *A;
    matrix *U,*D,*V;
    matrix *mtmp;

    A=NULL;
    U=NULL;
    D=NULL;
    V=NULL;
    mtmp=NULL;

    //printf("mat avant inversion du determinant\n");
    //matPrint(*mat);
    
    //u.(-d).v
    matSVD(*mat, &U,&D,&V);
    for (i=0 ; i<D->rs*D->cs ; i++)
        D->values[i]*=-1.0;
    matMultiply(&mtmp,U,D);
    matMultiply(mat,mtmp,V);

    //printf("mat apres inversion du determinant\n");
    //matPrint(*mat);

    matFree(&A);
    matFree(&U);
    matFree(&D);
    matFree(&V);
    matFree(&mtmp);

    return 0;
}

int matPoseFromEssential(matrix3 ess, matrix4 P[4])
{
    matrix *A;
    matrix *U,*D,*TD,*V,*E;
    matrix *mtmp;

    A=NULL;
    U=NULL;
    D=NULL;
    TD=NULL;
    V=NULL;
    mtmp=NULL;
    E=NULL;

    matAllocate(&E,3,3);
    mat3Copy(ess,E->values);

    matSVD(E, &U,&D,&V);
    
    // Make sure the determinants of U and V are positive
    if (matDeterminant(U) < 0.0) {
        matSwitchDeterminant(&U);
        //printf("Switch determinant de U\n");
    }
    
    if (matDeterminant(V) < 0.0) {
        matSwitchDeterminant(&V);
        //printf("Switch determinant de V\n");
    }

    // Build the diagonal matrix
    int i;
    for (i=0 ; i<9 ; i++)
        D->values[i]=0.0;
    matSet(D,0,1,-1.0);
    matSet(D,1,0,1.0);
    matSet(D,2,2,1.0);
    
    // Set the transpose of the diagonal matrix
    matTranspose(&TD,D);

    // Get the two possible rotation matrices
    matrix *R=NULL;
    matrix3 R1,R2;
    // R1
    matMultiply(&mtmp,U,D);
    matMultiply(&R,mtmp,V);
    vectxCopy(R->values,R1,9);
    // R2
    matMultiply(&mtmp,U,TD);
    matMultiply(&R,mtmp,V);
    vectxCopy(R->values,R2,9);

    // Get U3 (last column of U)
    vector3 u3;
    u3[0] = matGet(U,0,2);
    u3[1] = matGet(U,1,2);
    u3[2] = matGet(U,2,2);

    // Compute the 4 possible poses
    matrix4 pose;
    
    // POSE 1
    mat3ConvertToMat4(R1,pose);
    pose[3] = u3[0];
    pose[7] = u3[1];
    pose[11] = u3[2];
    vectxCopy(pose,P[0],16);

    // POSE 2
    mat3ConvertToMat4(R1,pose);
    pose[3] = -u3[0];
    pose[7] = -u3[1];
    pose[11] = -u3[2];
    vectxCopy(pose,P[1],16);

    // POSE 3
    mat3ConvertToMat4(R2,pose);
    pose[3] = u3[0];
    pose[7] = u3[1];
    pose[11] = u3[2];
    vectxCopy(pose,P[2],16);

    // POSE 4
    mat3ConvertToMat4(R2,pose);
    pose[3] = -u3[0];
    pose[7] = -u3[1];
    pose[11] = -u3[2];
    vectxCopy(pose,P[3],16);

    matFree(&A);
    matFree(&U);
    matFree(&D);
    matFree(&TD);
    matFree(&V);
    matFree(&mtmp);
    matFree(&R);
    matFree(&E);

    return 0;
}

void *nl_fcn_global_user_data[2]; //2 pointers for user parameters
int nl_fcn_global_max_nb_iter; //maximum number of iterations
int nl_fcn_global_iter_nb; //current number of iterations
unsigned char nl_fcn_global_options; //user defined options
double *nl_fcn_global_errors;

/**
 * Minimizes the sum of the squares of @p nbeqs nonlinear functions in N variables by a modification of the Levenberg-Marquardt algorithm (N cannot exceed @p nbeqs). The user must provide a subroutine @p fnc which calculates the costsfunctions. 
 * The Jacobian is then calculated by a forward-difference approximation. 
 * 
 * @param[in] params matrix pointer of size Nx1 containing the N variables to optimize
 * @param[in] nbeqs the number of functions, i.e. cost functions
 * @param[in] nbiterations number of iterations
 * @param[in] user_data1 some void * pointer so that the user can pass data to the function @p fnc
 * @param[in] user_data2 some void * pointer so that the user can pass data to the function @p fnc
 * @param[in] fcn user supplied function that computes costs
 * @param[out] errors ???
 * @param[in] options ???
 *
 * @return 0 : success; <0 : error
 *
 * @see matAbCosts()
 *
 */
int matNLOptimization(matrix *params,int nbeqs,int nbiterations,void *user_data1,void *user_data2,int (*fcn)(void *,int, int, const double *, double *, int),double *errors,unsigned char options)
{
#ifndef CMINPACK
    printf("matNLOptimization : cminpack is not installed!!!\n");
    return(-1);
#else
    int i, m, n, info, lwa, *iwa;
    double tol, *fvec, *wa;
    //double fnorm;

    if (params==NULL) return -1;
    if (params->rs!=1) return -1;
    if (params->cs>nbeqs)
    {
      fprintf(stderr,"[imgu optimization]: the number of variables exceeds the number of functions (%d>%d).\n",params->cs,nbeqs);
      return -1;
    }

    m = nbeqs;
    n = params->cs;
    lwa = m*n + 5*n + m;

    iwa=(int *)(malloc(sizeof(int)*n));
    fvec=(double *)(malloc(sizeof(double)*m));
    wa=(double *)(malloc(sizeof(double)*lwa));
    if (errors!=NULL) nl_fcn_global_errors=(double *)(malloc(sizeof(double)*nbeqs));
    else nl_fcn_global_errors=NULL;

    nl_fcn_global_user_data[0]=user_data1;
    nl_fcn_global_user_data[1]=user_data2;
    nl_fcn_global_max_nb_iter=nbiterations;
    nl_fcn_global_iter_nb=0;

    nl_fcn_global_options=options;

    // set tol to the square root of the machine precision.  unless high
    // precision solutions are required, this is the recommended
    // setting. 

    tol = sqrt(dpmpar(1));

    info = lmdif1(fcn, 0, m, n, params->values, fvec, tol, iwa, wa, lwa);

    //fnorm = enorm(m, fvec);

    //printf("      FINAL L2 NORM OF THE RESIDUALS%15.7f\n\n",fnorm);
    //printf("      EXIT PARAMETER                %10i\n\n",info);

    //printf("[matrixmath]: optimization ended after %d iteration.\n",nl_fcn_global_iter_nb);

    if (errors!=NULL) 
    {
      for (i=0;i<nbeqs;i++) errors[i]=nl_fcn_global_errors[i];
      free(nl_fcn_global_errors);
      nl_fcn_global_errors=NULL;
    }

    free(iwa);
    free(fvec);
    free(wa);

    return 0;
#endif
}


/**
 * Optimize Ax=b 
 *
 * @param[in] p internal variable used by CMINPACK
 * @param[in] nbeqs number of equations used to compute the sum of errors
 * @param[in] nbvars number of variables to optimize
 * @param[in] x array of size @p nbvars containing the starting values of the variables to be optimized
 * @param[in] y array of size @p nbeqs used to store the computed errors
 * @param[in] flag internal variable used by CMINPACK
 *
 * user_data1 and user_data2 can be retrieved using the global variables 
 * nl_fcn_global_user_data[0] and nl_fcn_global_user_data[1]
 *
 * The global variable nl_fcn_global_iter_nb has to be updated and checked 
 * against nl_fcn_global_max_nb_iter. To exit the optimization, set all costs @p y to 0.
 */
int matAbCosts(void *p, int nbeqs, int nbvars, const double *x, double *y, int flag)
{
    int i;
    matrix *A,*b,*matx,*mattemp,*matres;

    if (nl_fcn_global_iter_nb >= nl_fcn_global_max_nb_iter) {
        for (i=0;i<nbeqs;i++)
        {
          y[i]=0.0;
        }
        return 0;
    }
    nl_fcn_global_iter_nb++;


    A=(matrix *)(nl_fcn_global_user_data[0]);
    b=(matrix *)(nl_fcn_global_user_data[1]);
    matx=NULL;
    matres=NULL;
    mattemp=NULL;

    matAllocate(&matx,nbvars,1);
    vectxCopy(x,matx->values,nbvars);

    matMultiply(&mattemp,A,matx);
    matSubtract(&matres,mattemp,b);

    for (i=0;i<nbeqs;i++)
    {
        y[i]=fabs(matres->values[i]);
    }

    matFree(&matx);
    matFree(&matres);
    matFree(&mattemp);

    return 0;
}



