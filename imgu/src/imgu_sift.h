#ifndef IMGU_SIFT_H
#define IMGU_SIFT_H

/**
 * @addtogroup images
 * @{
 * @name Locating Features
 * @anchor locating-features
 * @{
 */

/**
 * Sift match structure used in @ref locating-features "Locating features".
 * This contains the information about sift points that are matched.
 */
typedef struct {
  double x1,y1,s1,d1; ///< (x,y), size, direction (-pi..pi)
  double x2,y2,s2,d2;
  double dist; ///< distance between vectors
} match;


#ifdef __cplusplus
extern "C" {
#endif

int imguSift(imgu *I);
int imguSiftDelete(imgu *I);
int imguMatchFromSift(imgu *IA,imgu *IB,match **pm,int max_nb_matches);


void imguDrawMatch(imgu *I,match *m,int nbmatch);
void imguDrawMatchPoints(imgu *IA,imgu *IB,match *m,int nbmatch);
void imguDrawSiftPoints(imgu *I);

/*@}*/

#ifdef __cplusplus
}
#endif

/*@}*/

#endif

