#ifndef IMGU_STEREO_H
#define IMGU_STEREO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <imgu/imgu.h>

/**
 * @addtogroup images
 * @{
 *
 * @name Stereo algorithms
 * @{
 */

#define DEPTHMAP_COST_INF 1000000.0

int imguIntegralFcn(imgu **Idest,imgu *IA,imgu *IB,double (*fcn)(double a,double b),unsigned char component);


int imguDepthMapDirectSearch(imgu **depthmap,imgu *Ileft,imgu *Iright,double *disps,int nbdisps);
int imguDepthMapDynamicProgramming(imgu **depthmap,imgu *Ileft,imgu *Iright,double *disps,int nbdisps,double lambda,unsigned char xmodulo);
int imguDepthMapGraphCuts(imgu **depthmap,imgu *Ileft,imgu *Iright,double *disps,int nbdisps,double lambda,unsigned char xmodulo,int nb_iter);
int imguDynamicProgramming(imgu *dsi,int drange,double *scosts,imgu *Idx,int y,double ocost);
int imguGraphCut(imgu *depthmap,imgu *dcosts,double *scosts,imgu *Idx,imgu *Idy,unsigned char xmodulo,int nbiter);

int imguSSDLocal(imgu **Idest,imgu *IA,imgu *IB,int wsize);

int imguZeroMeanNormalizedCrossCorrelation(imgu **Idest,imgu *IA,imgu *IB,int wsize);



/*@}*/
/*@}*/

#ifdef __cplusplus
}
#endif



#endif

