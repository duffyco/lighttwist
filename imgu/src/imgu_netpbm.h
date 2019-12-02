#ifndef IMGU_NETPBM_H
#define IMGU_NETPBM_H


#ifdef __cplusplus
extern "C" {
#endif



/**
 * @addtogroup images
 * @{
 *
 * @name NETPBM (pgm, ppm, ...)
 * @{
 */

int imguLoadPNM(imgu **I,const char *name,unsigned char option);
// only save 1 or 3 
int imguSavePNM(imgu *I,const char *name,unsigned char option);

/*@}*/
/*@}*/



#ifdef __cplusplus
}
#endif



#endif

