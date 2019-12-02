#ifndef IMGU_JPEG_H
#define IMGU_JPEG_H


#ifdef __cplusplus
extern "C" {
#endif




/**
 * @addtogroup images
 * @{
 *
 * @name JPEG
 * @{
 */

int imguLoadJPEG(imgu **I,const char *name,unsigned char option);
int imguSaveJPEG(imgu *I,const char *name,int compress,unsigned char option);

/*@}*/
/*@}*/



#ifdef __cplusplus
}
#endif



#endif

