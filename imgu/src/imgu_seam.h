#ifndef IMGU_SEAM_H
#define IMGU_SEAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <imgu/imgu.h>

/**
 * @addtogroup images
 * @{
 *
 * @name Seam algorithms
 * @{
 */

int imguSeam(imgu *Ivideo1,imgu *Ivideo2,unsigned char usegrad,imgu *Iinit);

/*@}*/
/*@}*/

#ifdef __cplusplus
}
#endif



#endif

