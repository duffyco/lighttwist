#ifndef IMGU_RECYCLE_H
#define IMGU_RECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup plugins
 *
 * This section groups the image recycling fonctions.
 * It provides a standard interface to manage queues of images.
 *
 * @{
 * @name Recycling images
 * @{
 */


  void imguSetRecycleQueue(imgu *I,rqueue *Q);
  int imguRecycle(imgu *I);


  /* @} */
  /* @} */


#ifdef __cplusplus
}
#endif



#endif

