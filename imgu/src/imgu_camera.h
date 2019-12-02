#ifndef IMGU_CAMERA_H
#define IMGU_CAMERA_H


/**
 * @defgroup camera Camera model
 *
 * @ingroup imgugeneral
 *
 * @todo doc
 *
 *@{
 */

#define IMAGE_SPACE 0
#define CAMERA_SPACE 1

typedef struct cam_next{
  imgu *img;

  double k1,k2;

  vector3 position;
  vector3 orientation; //in quaternion format
  matrix4 T,R; //pcam=T.R.pworld
  matrix4 M,Minv;

  matrix3 K,Kinv;

  matrix3 F;
  matrix3 E;

  matrix3 H;

  matrix *features; //features data points
  matrix *mfeatures; //matching features of features in camera 'fcam'
  struct cam_next *mcam; //camera in which are found the matching features

  struct cam_next *next;
}camera;


#ifdef __cplusplus
extern "C" {
#endif



int camAllocate(camera **cam,imgu *img);

int camAllocateMulti(camera **cam,imgu *ims);

int camLoadParams(camera *cam);

int camSaveParams(camera *cam);

int camCopyParams(camera *dest,camera *src);

void camDump(camera *cam);

void camFree(camera **cam);

void camFreeMulti(camera **cam);

int camCount(camera *cam);

int camSetInternalParams(camera *cam,double fx,double fy,double skew,double ox,double oy);

int camSetInternalParamsFromViewAngle(camera *cam,double hangle);

// w : axis + magnitude of rot
int camSetExternalParams(camera *cam,vector3 t,vector3 w);

int camSetExternalParamsFromMatrix(camera *cam,matrix4 Mext);

/**
 * prend un point 2D (x,y) dans l'image, donne lui la profondeur z et retourne sa coord p dans le monde.
 *
 * le point p est euclidien 3d (donc p[3]==1)
 * Le modele est p(image) = min . mext . (x,y,z,1)^T
 */
int camImageToCamera (camera *cam,vector3 pim,vector3 pcam);

int camCameraToImage (camera *cam,vector3 pcam,vector3 pim);

int camCameraToWorld (camera *cam,double x,double y,double z,double depth,vector4 pworld);

int camImageToWorld (camera *cam,double x,double y,double depth,vector4 pworld);

int camWorldToImage (camera *cam,vector4 pworld,vector3 pim);

int camWorldToCamera (camera *cam,vector4 pworld,vector4 pcam);

int camImageUndistort(camera *cam,vector3 pixel);

int camOriginAtCenter(camera *cam,vector3 pixel);

int camOriginAtUpperLeft(camera *cam,vector3 pixel);

int camCalibrateFrom7Points(camera *cam,matrix *pts2d,matrix *pts3d);

int camCalibrateFrom6Points(camera *cam,matrix *pts2d,matrix *pts3d);

int camInFront(camera *cam,vector4 pworld);

int camTriangulatePixels(camera *cam1,camera *cam2,matrix **pts_3d);

int camTriangulatePoints(camera *cam1,camera *cam2,matrix **pts_3d);

int camTriangulatePixelTracks(camera *cam1,camera *cam2,matrix **wpts,matrix **werrors,matrix **perrors);

int camSelectPose(camera *cam1,camera *cam2,matrix4 P[4]);


/*@}*/




#ifdef __cplusplus
}
#endif





#endif
