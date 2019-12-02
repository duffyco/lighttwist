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

#ifndef MATRIXMATH_H
#define MATRIXMATH_H

#include <math.h>
#include <time.h>
#include "svd.h"
#ifdef CMINPACK
  #include <cminpack.h>
#endif


#ifdef __cplusplus
        extern "C" {
#endif

/**
 * @defgroup matrix MatrixMath Library
 *
 *
 *@{
 */

typedef double vector3[3];
typedef double vector4[4];
typedef double matrix3[9];
typedef double matrix4[16];

typedef struct{
    int cs,rs;
    double *values;

    int ds; // internal buffer size
} matrix;

/*@}*/

/* @cond MAT */

#define MAT_SMALL 0.0001
#define MAT_VERY_SMALL 0.00000001

// ce facteur determine le determinant minimal
#define MAT_VERY_VERY_SMALL 1e-15
#define MAT_BUFFER_SIZE 512

/* @endcond */

/**
 * @defgroup vector Vectors of size 3 and 4
 *
 * @ingroup matrix
 *
 *@{
 */

//// matrix manipulation package

void vect3Copy(vector3 a,vector3 b);
// r=a-b
void vect3Subtract(vector3 a,vector3 b,vector3 r); // r=a-b (a=a-b or b=a-b OK)
// r=aXb
void vect3Cross(vector3 a,vector3 b,vector3 r);
void vect3Print(vector3 v);
int vect3Homogenize2D(vector3 v);

void vect4Copy(vector4 a,vector4 b);
// homogenize a 3D projective point, so you have [x,y,z,1]
int vect4Homogenize3D(vector4 v);
// homogenize a 2D projective point, so you have [x,y,1,w]
int vect4Homogenize2D(vector4 v);
double vect4SquareDist(vector4 a, vector4 b) ;
void vect4Print(vector4 v);
void vectxPrint(double* v, int len);
/*@}*/


/**
 * @defgroup matrix4 3x3 and 4x4 matrices
 *
 * @ingroup matrix
 *
 *@{
 */

void mat3Identity(matrix3 m);
int mat3Inverse(matrix3 m,matrix3 im);
void mat3Transpose(matrix3 m,matrix3 mt);
void mat3Copy(matrix3 a,matrix3 b);
int mat3ConvertToMat4(matrix3 m,matrix4 r);
int mat4ConvertToMat3(matrix4 m,matrix3 r);
void mat3Multiply(matrix3 a,matrix3 b,matrix3 m);
void mat3MultiplyVector(matrix3 a,vector3 b,vector3 v);
void mat3SkewSymmetric(vector3 n,matrix3 nx);
void mat3Print(matrix3 m);

void mat4Identity(matrix4 m);
int mat4Rotation(double a,vector3 n,matrix4 r);
int mat4ExtractRotations(matrix4 pose, vector3 r);
void mat4Translation(vector3 t,matrix4 m);
void mat4Scale(vector3 s,matrix4 m);

// a(4x4).b(4x4) -> m(4x4)
void mat4Multiply(matrix4 a,matrix4 b,matrix4 m);
// a(4x4).b(4x1) -> v(4x1)
void mat4MultiplyVector(matrix4 a,vector4 b,vector4 v);
void mat4Copy(matrix4 a,matrix4 b);
// suppose que m[12..15] = {0,0,0,1}
int mat4InverseAffine(matrix4 m,matrix4 im);
// inverse general
int mat4Inverse(matrix4 m,matrix4 im);
// transpose m into mt, m==mt OK

void mat4Transpose(matrix4 m,matrix4 mt);
void mat4Print(matrix4 m);
void mat4PrintMath(matrix4 m);
// extract a 4x4 from a string (0=ok, -1=err)
int mat4Scanf(char *buf,matrix4 m);
// make a string for a matrix. len is the size of buf. return 0 if ok, -1 if er

double mat4Determinant(matrix4 m);

int mat4CrossProduct(vector3 t,matrix4 m);
int mat4XRotation(double degrees,matrix4 m);
int mat4YRotation(double degrees,matrix4 m);
int mat4ZRotation(double degrees,matrix4 m);
int mat4RotationFromAxis(vector3 t,matrix4 m);
int mat4RotationToQuaternion(matrix4 m,vector4 t);
int mat4RotationToAxis(matrix4 m,vector3 t);
int mat4NormalizeRotation(matrix4 m);
int mat4Printf(matrix4 m,char *buf,int len);

/*@}*/

/**
 * @defgroup vectorx Vectors of specified length
 *
 * @ingroup matrix
 *
 *@{
 */

// a.b
double vectxDot(double *a,double *b,int len);
// angle between vector a and b (do not need to be norm 1)
double vectxAngle(double *a,double *b,int len);
double vectxNorm(double *w,int len);
double vectxNorm2(double *w,int len);
int vectxNormalize(double *w,int len);
void vectxCopy(const double *a,double *b,int len);

void vectxAdd(double *a,double *b,double *m,int len);
void vectxScale(double *a,double scale,double *m,int len);
void vectxSubtract(double *a,double *b,double *m,int len);

double vectxAvg(double *a,int len);
double vectxDev(double *a,int len);
double vectxDevP(double *a,int len);

double vectxMax(double *a,int len);
double vectxMin(double *a,int len);


/*@}*/

/**
 * @defgroup matrixgen General matrices
 *
 * @ingroup matrix
 *
 *@{
 */

int matAllocate(matrix **mat,int cs,int rs);
int matCopy(matrix **copy,matrix *mat);
void matFree(matrix **m);
double matGet(matrix *mat,int r,int c);
int matSet(matrix *mat,unsigned int r,unsigned int c,double val);

int matSelectRows(matrix **res,matrix *mat,int rstart,int nbrows);
int matSelectColumns(matrix **res,matrix *mat,int cstart,int nbcols);
int matRemoveRow(matrix **res,matrix *mat,int row);
int matRemoveColumn(matrix **res,matrix *mat,int col);
int matInsertColumn(matrix **res,matrix *mat,int col, double val);
int matConcatColumns(matrix **res,matrix *mat1,matrix *mat2);
int matConcatRows(matrix **res,matrix *mat1,matrix *mat2);

int matAdd(matrix **res,matrix *mat1,matrix *mat2);
int matSubtract(matrix **res,matrix *mat1,matrix *mat2);
int matInverse(matrix **res,matrix *mat);
int matMultiply(matrix **res,matrix *mat1,matrix *mat2);
int matTranspose(matrix **res,matrix *mat);
int matScale(matrix **res,matrix *mat,double scale);
double matDeterminant(matrix *mat);
double matTrace(matrix *mat);
int matSVD(matrix *A,matrix **U,matrix **D, matrix **V);
int matSolveA(matrix **res,matrix *A);
int matSolveAb(matrix **res,matrix *A,matrix *b);

int matIdentity(matrix **res,int size);
void matPrint(matrix *mat);

int matFit(double *x,double *y,int n,double *coeffs,int d);
int matEval(double *x,double *y,int n,double *coeffs,int d);
int matFitCubicSpline(matrix **Z,matrix **h,matrix *points);
int matEvalCubicSpline(matrix *Z,matrix *h,matrix *points,double x,double *y);

/**
 * @name Vision
 * @{
 */


double matRandNumber(void);
void matRandInit(void);
void matRandSeed(unsigned int s);

int matRansac(matrix *pts1,matrix *pts2,int nbsamples,int nbiterations,double validamount,double threshold, int (*modelfunc)(matrix *,matrix *,matrix **),int (*errorfunc)(matrix *,matrix *,matrix *,double *),matrix **hres,double *errors,unsigned char *inliers);
int matExtractInliers(matrix *pts,unsigned char *iflags,matrix **inliers);

int matHomographySolve(matrix *pts1,matrix *pts2,matrix **hres);
int matHomographyError(matrix *pts1,matrix *pts2,matrix *hres,double *errors);

int matNormalizePoints(matrix *pts,matrix **ptsnorm,matrix **T);
int matFundamentalSolve(matrix *pts1,matrix *pts2,matrix **hres);
int matFundamentalError(matrix *pts1,matrix *pts2,matrix *hres,double *errors);

int matEssentialSolve(matrix *pts1,matrix *pts2,matrix **hres);
int matEssentialError(matrix *pts1,matrix *pts2,matrix *hres,double *errors);

int matFundamentalToEssential(matrix3 k1, matrix3 k2, matrix3 f, matrix3 ess);
int matPoseFromEssential(matrix3 ess, matrix4 P[4]);

int matNLOptimization(matrix *params,int nbeqs,int nbiterations,void *user_data1,void *user_data2,int (*fcn)(void *,int, int, const double *, double *, int),double *errors,unsigned char options);
int matAbCosts(void *p, int nbeqs, int nbvars, const double *x, double *y, int flag);

/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
