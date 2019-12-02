#ifndef IMGU_MATRIX_H
#define IMGU_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @addtogroup images
 * @{
 *
 * @name Matrix load/save from text tag
 * @{
 *
 * @warning These functions are not part of the standalone MatrixMath library
 *
 */

int imguLoadMatrix(matrix **mat,imgu *I,const char *key);
int imguSaveMatrix(matrix *mat,imgu *I,const char *key);

/*@}*/
/*@}*/



#ifdef __cplusplus
}
#endif



#endif
