#ifndef IMGU_CANNY_H
#define IMGU_CANNY_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup images
 * @{
 *
 * @name Edge detection
 * @{
 */

int imguCannyEdgeTracking(imgu **dest,imgu *src, double stddev,double th,double tl);
int imguCannyEdgeDetection(imgu **dest,imgu *src, double stddev);
int imguCannyHorizontalEdgeTracking(imgu **dest,imgu *src, double stddev,double th,double tl);
int imguCannyHorizontalEdgeDetection(imgu **dest,imgu *src, double stddev);
int imguCannyVerticalEdgeTracking(imgu **dest,imgu *src, double stddev,double th,double tl);
int imguCannyVerticalEdgeDetection(imgu **dest,imgu *src, double stddev);
int imguCannyBasic(imgu **dest,imgu *src, double stddev);
int imguLineThinner(imgu **Idest,imgu *Isrc, double stddev,int threshold);

/*@}*/
/*@}*/


#ifdef __cplusplus
}
#endif



#endif
