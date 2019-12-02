#ifndef IMGU_OSG_H
#define IMGU_OSG_H

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @addtogroup images
 * @{
 *
 * @name Exporting data in 3D (OpenSceneGraph format)
 * @{
 *
 * Visualization of an image as a 3D camera.
 *
 * If an image contains the following special keys:
 *   - cam_intern
 *   - cam_extern
 * then it can be visualized as 3D camera in osg (OpenSceneGraph).
 *
 * The resulting model is also a key:
 *   - cam_osg
 * utilise les parametres cam_intern et cam_extern pour generer
 * un modele osg ascii dans l'etiquette cam_osg
 *
 * @{
 */

/// Flag for imguOsgCylinder
#define USE_TEXTURE 1
/// Flag for imguOsgCylinder
#define USE_TOP     2
/// Flag for imguOsgCylinder
#define USE_SIDE    4
/// Flag for imguOsgCylinder
#define USE_BOTTOM  8

int imguOsgBeginGeode(imgu *I);
int imguOsgEndGeode(imgu *I);
int imguOsgSave(imgu *I,char *outNameOsg);

// r,g,b : couleur des lignes
// transparence de l'image affichee. Normalement: 1.0 (opaque)
int imguOsgCamera(imgu *I,camera *cam,vector4 color,char *texfile);
int imguOsgCameraScaled(imgu *I,camera *cam,vector4 color,char *texfile, double scale);
//int imguOsgCameraTruncated(imgu *I,double focal1,double focal2,double transparency,double color[3]);
// ajoute des axes dans un osg
int imguOsgAxis(imgu *I,matrix4 m,double scale);

// ajoute du texte dans une geode
int imguOsgText(imgu *I,vector3 position,vector3 color,int size,char *txt);

int imguOsgQuad(imgu *I,matrix4 mm,vector3 position,vector3 orientation,double width,double height,vector4 color,int flags,char *texfile);
int imguOsgQuadPoints(imgu *I,vector3 pbl,vector3 pbr,vector3 ptl,vector3 ptr,vector4 color,int flags,char *texfile);
int imguOsgSphere(imgu *I,matrix4 mm,vector3 position,vector3 orientation,double radius,vector4 color,double detail,int flags,char *texfile);
int imguOsgCylinder(imgu *I,matrix4 mm,vector3 position,vector3 orientation,double radius,double height,vector4 color,double detail,int flags,char *texfile);
int imguOsgCone(imgu *I,matrix4 mm,vector3 position,vector3 orientation,double radius,double height,vector4 color,double detail,int flags,char *texfile);
int imguOsgCube(imgu *I,matrix4 mm,vector3 position,vector3 orientation,vector3 size,vector4 color,double detail,int flags,char *texfile);

int imguOsgLine(imgu *I,matrix4 m1,vector3 e1,matrix4 m2,vector3 e2,vector4 color);


/*@}*/
/*@}*/

#ifdef __cplusplus
}
#endif



#endif
