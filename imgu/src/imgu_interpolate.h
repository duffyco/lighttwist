#ifndef IMGU_INTERPOLATE_H
#define IMGU_INTERPOLATE_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup images
 * @{
 * @name Interpolation
 * @{
 */

int imguInterpolateClosest(imgu *I,double x,double y,double *val);
int imguInterpolateBilinear(imgu *I,double x,double y,double *val);
int imguInterpolateBicubic(imgu *I,double x,double y,double *val);

/*@}*/
/*@}*/


/**
 * @addtogroup complex
 * @{
 * @name Complex Interpolation
 * @{
 */

int imguInterpolateClosestComplex(imgu *I,double x,double y,double *val,unsigned char component);
int imguInterpolateBilinearComplex(imgu *I,double x,double y,double *val,unsigned char component);
int imguInterpolateBicubicComplex(imgu *I,double x,double y,double *val,unsigned char component);

/*@}*/
/*@}*/





#ifdef __cplusplus
}
#endif




#endif

