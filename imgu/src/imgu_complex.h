#ifndef IMGU_COMPLEX_H
#define IMGU_COMPLEX_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup complex Complex image processing
 * @ingroup imgugeneral
 *
 * Gestion des buffers "complexes" associés à chaque image.
 *
 * Le lien entre une image entiere et une image image complexe passe par des tags reservés:
 * CONVERSION linear 0 65535 -10.0 10.0
 * CONVERSION anglmag 0 65535 0.0 100.0
 *
 * CONVERSION_FUNCTION (LINEAR, ...)
 * INTEGER_RANGE (contient un min et max entre 0 .. 65535)  imin,imax
 * FLOAT_RANGE   (contient un in et max) fmin,fmax
 * Convertion: f= (i-imin)/(imax-imin)*(fmax-fmin)+fmin;
 *             i= (int)(f-fmin)/(fmax-fmin)*(imax-imin)+imin +0.5??;
 *
 * CONVERSION_FUNCTION (ANGLE_MAG)
 * (le range est automatique entre 0..65535)
 * MAGNITUDE_MAX  mmax (float range)
 * angle = atan(real,imag)
 * mag = norm(real,imag)
 * -> converti en couleur
 *
 * - Le buffer complexe est toujours à null dans une image
 * - Conversion Entier -> Complexe
 *   - imguConvertToComplex()  (alloc/reuse un buffer complexe)
 *   - imguConvertToInteger()  (alloc/reuse un buffer entier)
 *   - imguSplitImageComplex() (Data+Complexe -> 1 image Data + 1 image Complexe)
 *   - imguJoinImageComplex() (Data+Complexe -> 1 image Data + 1 image Complexe)
 *
 * - Fonctions specifiques Complexes
 *   - FFT et invFFT
 *   - Convolve, fitrage
 *   - gamma et autres corrections
 *   - Optical flow, etc...
 *   - calcule de gradient, ...
 *
 * @{
 */



/**
 * @name Memory management
 *
 * These functions allocate complex memory for images.
 *
 * @attention The imguAllocateComplex() function should only be called to create a new empty complex image; it does not have to be called before imguConvertToComplex(),imguConvertToComplexFromColor()
 *
 *@{
 */


#define imguAllocateComplexDebug(I,xs,ys,cs)  (printf("COMPLEX ALLOCATE (%dx%dx%d) at %s:%d\n",(xs),(ys),(cs),__FILE__, __LINE__),imguAllocateComplex((I),(xs),(ys),(cs)))

int imguAllocateComplex(imgu **cd,int xs,int ys,int cs);

int imguClearComplex(imgu *I,unsigned char component);

void imguFreeComplex(imgu *I);

/*@}*/


/**
 * @name Conversion options
 * @{
 */

#define COMPLEX_AS_IS 0
#define COMPLEX_RESCALE 1

#define DATA_MODE 0
#define COMPLEX_MODE 1

/*@}*/


/**
 * @name Conversion from/to complex data
 * @{
 */

int imguConvertToComplexComponent(imgu **dest,imgu *src,unsigned char component);
int imguConvertFromComplexComponent(imgu **dest,imgu *src,unsigned char component,unsigned char scale);

int imguConvertFromComplexUV(imgu **dest,imgu *src,unsigned char scale);
int imguConvertToComplexUV(imgu **dest,imgu *src);
int imguConvertFromComplexFlow(imgu **im,imgu *cd);
int imguConvertToComplexFlow(imgu **cd,imgu *im);


void imguHLStoRGB(vector3 hls,vector3 rgb);
void imguRGBtoHLS(vector3 rgb,vector3 hls);

/*@}*/

/**
 * @name Data transformation
 * @{
 */

int imguLog(imgu **dest,imgu *src,unsigned char component);
int imguAbsolute(imgu **dest,imgu *src,unsigned char component);
int imguMagnitude(imgu **dest,imgu *src);
int imguPower(imgu **dest,imgu *src);
int imguMagnPhase(imgu **dest,imgu *src);
int imguRealImag(imgu **dest,imgu *src);
int imguMapRangeComplex(imgu **dest,imgu *src,double min, double max,unsigned char component);
int imguSubtractMean(imgu **dest,imgu *src, unsigned char key,double *mean);

/*@}*/


/**
 * @name Convolution filters
 * @anchor convolution
 * @{
 */


int imguGaussianFilter(imgu **cd,double dev);
int imguFirstDerivFilter(imgu **cd);
int imguFirstDerivFilter2Pixels(imgu **cd);
int imguSecondDerivFilter(imgu **cd);
int imguAvgFilter(imgu **cd);
int imguAvgFilter2Pixels(imgu **cd);
int imguRotateFilter(imgu **dest,imgu *src,double angle);
int imguConvertFilterToFrequencyDomain(imgu **dest,imgu *src,int xs,int ys);

/*@}*/

/**
 * @name Convolution fonction
 * @{
 */

#define CONVOLVE_REMOVE_MARGIN 0
#define CONVOLVE_BLANK_MARGIN 1
#define CONVOLVE_KEEP_MARGIN 2

int imguConvolveComplex(imgu **dest,imgu *src,imgu *mask,int rem_margin);

/*@}*/

/**
 * @name FFT
 * @{
 */

#define FFT_ORIGINAL_QUADRANTS 0
#define FFT_ADJUST_QUADRANTS 1


// FFTW3 specific functions.  They do nothing if FFTW3 not included.
// They do not need to be used if performance is not an issue.
int imguFFTInitThreads(int nbThreads);
void imguFFTUninit(void);
int imguFFTPlan(imgu **dest,imgu *src, int isign);


int imguFFTForward(imgu **dest,imgu *src);
int imguFFTInverse(imgu **dest,imgu *src);
int imguRemoveDC(imgu **dest,imgu *src);
int imguAdjustQuadrants(imgu **dest,imgu *src);
int imguSpatialHardBandPass(imgu **dest,imgu *src,double radiusmin,double radiusmax);

/*@}*/



/**
 * @name Windows
 * @anchor cwindow
 * @{
 */

int imguGaussianWindow(imgu **cd,int size);
int imguRaisedCosineWindow(imgu **cd,int size);

/*@}*/

/**
 * @name Window function
 * @{
 */

int imguWindowComplex(imgu **dest,imgu *src,imgu *window);

/*@}*/

/*@}*/




#ifdef __cplusplus
}
#endif




#endif

