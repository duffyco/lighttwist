#ifndef IMGU_PYRAMID_H
#define IMGU_PYRAMID_H




/**
 * @defgroup multiscale Multiscale support
 *
 * @ingroup imgugeneral
 *
 * Sample program:
 *
 * @code
 *
 * imgu *imgs;
 * pyramid *scales;
 *
 * imgs=NULL;
 * scales=NULL;
 *
 * imguLoadMulti(&imgs,"images.png",LOAD_16_BITS); //load a sequence of images
 * //create a list of pyramids, one for each image in the sequence
 * prmdCreateMulti(&scales,imgs,3,1.5,1.0); //images will remain the same size because scale_factor=1.0, but more and more blurred at each GAUSSIAN level
 * imguSave(scales->levels[1][LAPLACIAN],"output.png",FAST_COMPRESSION,SAVE_16_BITS); //save 2nd level of LAPLACIAN pyramid (for the first image in the sequence)
 *
 * imguFreeMulti(&imgs);
 * prmdFreeMulti(&scales);
 *
 * @endcode
 *
 *@{
 */

#define GAUSSIAN 0
#define LAPLACIAN 1

typedef struct prmd_next{
  int nblevels;
  imgu ***levels; //gaussian and laplacian pyramid

  int lsize;
  struct prmd_next *next;
}pyramid;



#ifdef __cplusplus
extern "C" {
#endif


int prmdCreate(pyramid **p,imgu *ims,int nblevels,double dev,double scale_factor,int (*fcn)(pyramid *,imgu *,double,double));
int prmdCreateMulti(pyramid **p,imgu *ims,int nblevels,double dev,double scale_factor,int (*fcn)(pyramid *,imgu *,double,double));
int prmdLaplacian(pyramid *p,imgu *img,double dev,double scale_factor);
int prmdMipMap(pyramid *p,imgu *img,double dev,double scale_factor);

void prmdFree(pyramid **p);
void prmdFreeData(pyramid **p);
void prmdFreeMulti(pyramid **p);


#ifdef __cplusplus
}
#endif

/*@}*/




#endif
