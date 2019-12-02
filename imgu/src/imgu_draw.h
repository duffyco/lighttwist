#ifndef IMGU_DRAW_H
#define IMGU_DRAW_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup images
 * @{
 * @name Drawing Features
 * @{
 */

  void imguDrawLine(imgu *I,double x1,double y1,double x2,double y2,int c,pix_t *color);
  int imguDrawLineLength(imgu *img,double cx,double cy,double x,double y,vector3 rgb,double linelen);
  int imguDrawSegment(imgu *img, float x1, float y1, float x2, float y2,vector3 rgb);
  int imguDrawCross(imgu *img, float x, float y,int len,vector3 rgb);

  /*@}*/
  /*@}*/


#ifdef __cplusplus
}
#endif


#endif
