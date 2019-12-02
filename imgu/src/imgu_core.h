#ifndef IMGU_CORE_H
#define IMGU_CORE_H

#include <stdio.h>

#ifdef HAVE_FFTW3
#include <fftw3.h>
#endif
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


///////////////////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////
///
/// Various pixel data type are set by defining (one is always defined):
/// IMGU8 -> pixels are unsigned char, 8 bit version of the library.
/// IMGU8 not defined -> pixels are unsigned short (default behavior)
///
/// the following macros must be defined:
/// pix_t -> unsigned char, or unsigned short
///


////// all definitions specific to pixel size go here!!!!

#ifdef IMGU8
    #define pix_t       unsigned char
    #define IMGU_MAXVAL 255
#else
    #define pix_t       unsigned short
    #define IMGU_MAXVAL 65535
#endif

#ifdef HAVE_RQUEUE
    #include <rqueue.h>
#endif


///////////////////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////

/* @cond PI */

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795029L
#endif

/* @endcond PI */


/**
 * @name Flags for ImageRepresention
 * @{
 */

#define Y_ORIGIN_IS_AT_TOP  0
#define Y_ORIGIN_IS_AT_BOTTOM   1

/*@}*/


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @name Two global variables for the Y formats of images
 *
 * @{
 */

extern int InternalFormat; ///< Where is the Y=0 row in memory?
extern int ExternalFormat; ///< Where is the Y=0 row in external file (for load and save)?

/*@}*/

/**
 * @addtogroup images
 * @{
 * @name Image Y representation
 * @{
 */

/**
 * @note for now, only Y_ORIGIN_IS_AT... is usable here
 * default: Y_ORIGIN_IS_AT_TOP for both Internal and External
 * Suggestion: only set Internalrepresentation once.
 * This only affects the loading and saving of images.
 */
void imguSetInternalFormat(int flag);
void imguSetExternalFormat(int flag);


#ifdef __cplusplus
}
#endif

/*@}*/


#define IMGU_MAX_NB_READ_ATTEMPTS 100

#define NO_COMPRESSION 0
#define FAST_COMPRESSION 1
#define BEST_COMPRESSION 9

/**
 * Determine if an 8 bpp (bits per pixel) image will be converted to 16 bits.
 * LOAD_16_BITS should be replaced by MAKE_16_BITS.
 * LOAD_AS_IS indicate to perform no conversion.
 *
 * @see imguLoad
 */

//#define MAKE_16_BITS 0
// 16bit in -> 16 out, 8bit in -> 16bit HIGH out
#define LOAD_16_BITS 0

// 8->8, 16->16
#define LOAD_AS_IS 1
//#define KEEP_8_BITS 1

// when loading a 16 bit image... keep the HI or the LOW? (AS_IS=HI)
// 8bit -> 16 low or hi. 16bit -> as is
#define LOAD_8_BITS_HIGH    2
#define LOAD_8_BITS_LOW     3

// si on 16 bit, correspond a LOAD_16_BITS
// si en 8 bits, retourne une image High et next -> image Low (donc 2 images)
#define LOAD_AS_HIGH_LOW    4
// a 16bit y is returned as a cs==2 8 bit image
// a 16bit ya is returned as a cs==4 8 bit image
// a 16bit rgb is returned as a cs==6 8 bit image
// a 16bit rgba is returned as a cs==8 8 bit image
#define LOAD_AS_KEEP16	    5

/**
 * Determine which data to save in an output image
 */

/// Save the full 16 bits as a 16 bpp image
//  a 8bpp file use this as SAVE_16_BITS_HIGH
#define SAVE_16_BITS 0
/// Save the lower 8 bits as an 8 bpp image
#define SAVE_8_BITS     1
#define SAVE_8_BITS_LOW 1

/// Save the higher 8 bits as an 8 bpp image
#define SAVE_8_BITS_HIGH 2
/// Save a single byte (8 bit image) as the HI of a 16bbp image
#define SAVE_16_BITS_HIGH 3
#define SAVE_16_BITS_LOW  0

/// 16 bit is saved as it is. 8 bits is saved as it is also
/// a 8bit cs==6 is saved as a rgb 16bit png
/// a 8bit cs==8 is saved as a rgba 16bit png
#define SAVE_AS_IS  5

/*@}*/

/**
 * @defgroup images Integer images processing
 *
 * @ingroup imgugeneral
 *
 * @brief This module contains everything related to basic integer image processing.
 *
 * @{
 */



/**
 * Complex data type inside images
 *
 */
//#define COMPLEX_FLOAT
#ifdef COMPLEX_FLOAT
    #define complex_t float
#else
    #define complex_t double
#endif

#ifndef HAVE_FFTW3
typedef complex_t fftw_complex[2];
#endif

typedef struct imgu {
  int xs; ///< width of image
  int ys; ///< height of image
  int cs; ///< number of channels (1=mono, 2=gray+alpha, 3=rgb, 4=rgba, >4=ok but undefined)
  int ds; ///< number of elements in data array (0 when data==NULL) Can be > xs*ys*cs
  pix_t *data;  /**< Image Data: row-order (y,x,c)
                 *  In RGB mode : RGB RGB RGB RGB ...
                 *  NULL -> No Image loaded
                 *  Pixel (x,y,c) is at data[(y*xs+x)*cs+c]
                 */

  fftw_complex *complex; // Complex image. Array is also of size ds
#ifdef HAVE_FFTW3
  fftw_plan fftplan;
  fftw_complex *fftplan_target;
#endif



  int ts;   ///< nb de textes disponibles
  char **key;   ///< [ts] contient des strings avec des cles pour les textes.
            ///< @attention certains [i] peuvent etre NULL
  char **txt;   ///< [ts] contient des textes qui vont avec les cles
  ///< @attention certains [i] peuvent etre NULL
  int *txtsz;   ///< [ts] contient la longueur du texte allouee par le malloc.
  ///< quand on fait append, on double la taille si on fait malloc. sinon, ordinaire.
  ///< txtlen n'a pas a etre mis a 0 lorsque txt[] est null.

  /// user data and number are not used by the library
  int userNumber;
  void *userData;

  /// used only by imguCompress / imguUncompress
  unsigned long compressed_data_size;

  /// champ totalement interne. NE PAS UTILISER
  struct imgu *next;

    /// donne la queue ou l'on doit retourner l'image sur l'appel imguRecycle()
    /// si NULL, alors on ne fait rien. Definir avec imguSetRecycleQueue()
#ifdef HAVE_RQUEUE
    rqueue *Qrecycle;
#endif
} imgu;


// to simplify usage of type-specific imgu structure
// #define imgu RENAME(imgu)


/// acces a un pixel
#define INDEX(I,x,y,c)  (((y)*(I)->xs+(x))*(I)->cs+(c))
#define PIXEL(I,x,y,c)  ((I)->data[INDEX(I,x,y,c)])
#define COMPLEX(I,x,y,c,u)  ((I)->complex[INDEX(I,x,y,c)][u])


  //declaration d'une image

// est-ce que ca sert vraiment?????
#define IMGU_DECLARE(I) imgu *I = NULL





#ifdef __cplusplus
extern "C" {
#endif



/**
 * @name Debug and image information
 * @{
 */

void imguDump(imgu *I);
void imguDumpMulti(imgu *I);

/*@}*/





/**
 * @name Text tags manipulation
 *
 * @par
 * These functions manipulate Text tags associated with image.
 * Each png image has a list of (key,value) pairs that can be used to add text
 * information to an image. The format is not strict, except that the key must
 * be a simple keyword and the text must be a nul-terminated string.
 * Key and text should be ascii only.
 *
 * @par
 * There is a set of reserved keys. They are in uppercase, so used lower case keys for your own
 * non reserved keys. The list of reserved tags is documented in @ref reservedkeys.
 *
 * @{
 */


int imguCopyText(imgu *dest,imgu *src);
void imguResetText(imgu *I);
void imguReset(imgu *I);

int imguAddText(imgu *I,const char *key,const char *txt);
int imguAppendText(imgu *I,const char *key,const char *txt);
int imguPushText(imgu *I,const char *key,const char *txt); ///< ajoute le texte au debut du courant, ou cree
int imguReplaceAddText(imgu *I,const char *key,const char *txt); ///< replace or add une copie du texte
int imguReplaceText(imgu *I,const char *key,const char *txt); ///< replace (or fail) une copie du texte
int imguRemoveKey(imgu *I,const char *key,int all); ///< enleve une cle (all ou single)


char *imguGetText(imgu *I,const char *key); ///< trouve le texte pour cette cle

// retourne le nb de textes associes a la cle key
// rempli le tableau fourni jusqu'a max valeurs
int imguGetAllText(imgu *I,const char *key,char **TxtTab,int max);


/*@}*/



/**
 * @name Memory Memory management
 *
 * @par
 * These functions allocate data memory for images.
 *
 * @attention The imguAllocate() function should only be called to create a new empty image; it does not have to be called before imguLoad().
 *
 * @{
 */


imgu *imguCreate(int xs,int ys,int cs);
#define imguAllocateDebug(I,xs,ys,cs)  (printf("ALLOCATE (%dx%dx%d) at %s:%d\n",(xs),(ys),(cs),__FILE__, __LINE__),imguAllocate((I),(xs),(ys),(cs)))
int imguAllocate(imgu **I,int xs,int ys,int cs);
int imguAllocateExtra(imgu **I,int xs,int ys,int cs,int extra);
int imguExtendExtra(imgu *I,int extra); // ajoute extra shorts au bout de l'image, en conservant l'image
int imguClear(imgu *I);


int imguCopy(imgu **dest,imgu *src);
int imguCopyMulti(imgu **dest,imgu *src);

void imguFree(imgu **I);
void imguFreeData(imgu *I);
void imguFreeMulti(imgu **I);


/*@}*/


/**
 * @name Loading
 * @{
 */


int imguLoad(imgu **I,const char *name,unsigned char option);
int imguLoadMulti(imgu **I,const char *name,unsigned char option);

int imguLoadFromFile(imgu **I,FILE *F,unsigned char option);
int imguLoadOrWaitFromFile(imgu **I,FILE *F,int nb_read_attempts,unsigned char option);

int imguLoadPNG(imgu **I,const char *name,unsigned char option);

/*@}*/

/**
 * @name Saving
 * @{
 */


int imguSave(imgu *I,const char *name,int compress,unsigned char option);
int imguSaveMulti(imgu *I,const char *name,int compress,unsigned char option);
int imguSaveToFile(imgu *I,FILE *F,int compress,unsigned char option);
int imguSaveRAW8ToFile(imgu *I,FILE *F,unsigned char option);
/* @cond SAVE */
int imguSavePNG(imgu *I,const char *name,int compress,unsigned char option);
/* @endcond */


/*@}*/

//////////////////////////
//////////////////////////
//////////////////////////


/**
 * @name Geometric transformations
 *
 *@{
 */

int imguScale(imgu **Iscaled,imgu *I,double sx,double sy);
int imguExtractRectangle(imgu **S,imgu *I,int xmin,int ymin,int width,int height);
int imguExtractRectangleSubPixel(imgu **S,imgu *I,double xmin,double ymin,int width,int height);
int imguAddRectangle(imgu *J,imgu *I,int x,int y);
int imguTranspose(imgu **Idest,imgu *Isrc);
int imguMirror(imgu **dest,imgu *src);
int imguFlip(imgu **dest,imgu *src);


/*@}*/

/**
 * @name Packing and Unpacking to bytes instead of shorts
 *@{
 */

int imguPack8bit(imgu *I,unsigned char *buffer,int mode);
int imguUnpack8bit(imgu *I,unsigned char *buffer,int mode);

/*@}*/


/**
 * @name Intensity conversion functions
 * @{
 */

int imguConvert8bitTo16bit(imgu **Idest,imgu *Isrc);
int imguConvert16bitTo8bit(imgu **Idest,imgu *Isrc);

int imguConvertToGray(imgu **Idest,imgu *Isrc);
int imguConvertToLuminance(imgu **Idest,imgu *Isrc);
int imguConvertToRelativeLuminance(imgu **Idest,imgu *Isrc);
int imguConvertToHue(imgu **Idest,imgu *Isrc);
int imguConvertToSaturation(imgu **Idest,imgu *Isrc);
int imguConvertToRGB(imgu **Idest,imgu *Isrc);

int imguMapRange(imgu **dest,imgu *src, pix_t min, pix_t max);

/*@}*/


/**
 * @name Alpha layer management functions
 * @{
 */

int imguAddAlphaLayer(imgu **Idest,imgu *Isrc,imgu *alpha);
int imguRemoveAlphaLayer(imgu **Idest,imgu *Isrc);

/*@}*/


/**
 * @name Comparing images
 * @{
 */

double imguSSD(imgu *I,imgu *P,int x0,int y0,int *nb,imgu *Pmask);
double imguSSDSubPixel(imgu *I,imgu *P,double x0,double y0,int *nb,imgu *Pmask);

/*@}*/


/**
 * @name Convolution and windowing
 *
 * @note See \ref convolution and \ref cwindow for functions that create necessary windows and masks.
 *
 *@{
 */

int imguWindow(imgu **dest,imgu *src,imgu *window);
int imguConvolve(imgu **dest,imgu *src,imgu *mask,int offset,int rem_margin);
int imguBlur(imgu **Iblur,imgu *I,double devx,double devy,int rem_margin);

int imguFastBlur5x5(imgu **Iblur,imgu *I,int nb_run);

/*@}*/





/**
 * @name Concatenated images
 *@{
 */

int imguConcat(imgu **I,imgu *seq,int offset,int nbimgs);
int imguSplit(imgu **I,imgu *seq,int nb_channels_per_img);
int imguSelectChannels(imgu **Iselect,imgu *img,int start_channel,int nb_channels);
int imguSelect(imgu **Iselect,imgu *I,int start_index,int nb_imgs);

/*@}*/

/**
 * @name Channel operations (temporal?)
 *@{
 */

int imguWindowGaussianChannels(imgu **dest,imgu *src,double ct,double dt);
int imguPaddChannels(imgu **dest,imgu *src, int ts);

/*@}*/



/**
 * @name Compression
 * @par
 * These functions are used to perform compression using zlib.  Compressed data is stored in the 'extra' buffer space above the regular image data space.
 *
 *@{
 */
int imguCompress(imgu *I);
int imguUncompress(imgu *I);


/*@}*/


/**
 * @name Unclassified functions
 * @todo SVP trouver un endroit pour ces fonctions...
 *
 * @{
 */

int imguEqualize(imgu **dest,imgu *src,imgu *src_goal,int hsize);
int imguAddNoise(imgu **dest,imgu *src,double dev);
int imguInvert(imgu **dest,imgu *src);
int imguWindowGaussian(imgu **dest,imgu *src,double cx,double cy,double dx,double dy);
int imguMedianFilter(imgu **dest,imgu *src, int wsize);
int imguPadd(imgu **Ipad,imgu *I,int paddleft,int paddright,int paddtop,int paddbottom);
int imguSubtract(imgu **Isub,imgu *img1,imgu *img2,pix_t offset);
int imguMosaic(imgu **mosaic,imgu *seq,int nb_x_patches,int nb_y_patches,int nb_t_patches);
int imguCount(imgu *I);
int imguCheck(imgu *I,float x,float y);
int imguAddFirstMulti(imgu **head,imgu *I);
int imguAddLastMulti(imgu **head,imgu *I);
imgu *imguRemoveFirstMulti(imgu **head);
imgu *imguRemoveLastMulti(imgu **head);


/*@}*/




/**
 * @name Optimization functions
 *
 * These functions are used to perform non-linear optimizations using the MatrixMath library.
 * @see matNLOptimization
 *
 *@{
 */

/**
 * @bug matPoseCosts does not exists and should be removed (?)
 */
int matPoseCosts(void *p, int nbeqs, int nbvars, const double *x, double *y, int flag);

/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif

#endif

