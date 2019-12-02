/*
 * This file is part of IMGU.
 * 
 * @Copyright 2008 Université de Montréal, Laboratoire Vision3D
 *   Sébastien Roy (roys@iro.umontreal.ca)
 *   Vincent Chapdelaine-Couture (chapdelv@iro.umontreal.ca)
 *   Lucie Bélanger
 *   Jamil Draréni 
 *
 * IMGU is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * IMGU is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with IMGU.  If not, see <http://www.gnu.org/licenses/>.
 */

/// gestion de l'output en OSG
//
// Concept:
// Pour creer un objet 3D, on utilise les tags suivants:
// osg (tag final contenant toutes les composantes)
//     Le osg commence par group { geode { et fini par } }
//     il contient normalement des geometry
// geometry contient les parties suivantes:
//     stateset -> contient un stateset
//     primitiveset -> contient les primitives (tout en index)
//     colorbinding -> normalement OVERVAL ou PER_VERTEX
//     normalbinding -> normalement OVERALL ou PER_VERTEX
//
//     vertexarray -> contient les vertex (3d)
//     colorarray -> contient les couleurs (4d)
//     normalarray -> contient les normales (3d)
//     texcoordarray -> contient les coord textures (2d)
//
// Quand on flush la geometry, tous ces buffers sont envoyes a la fin du buffer 'osg'
//
// La sequence normale pour construire un objet 3D est la suivante
//
// osg_begin()  -> efface tout "group { geode {"
//
//   geometry_begin() -> creer une nouvelle geometry, "geometry {"
//
//     stateset_begin()  -> "stateset {"
//     stateset_append("GL_LIGHTING OFF"); ...
//     stateset_end()    -> "}" et ajoute e stateset a la geometry, puis vide.
//
//     primitiveset_begin() -> "primitiveset {"
//     primitiveset_append("DrawElementsUShort LINES 0 { 0 1 }")
//     primitiveset_append("DrawElementsUShort TRIANGLES 0 { 0 1 2 }")
//     primitiveset_append("DrawElementsUShort LINES 0 { ");
//         primitiveset_append("0 1"); ... pour chaque ligne, donner l'index
//     primitiveset_append(" }");
//     primitiveset_end() -> "}" ajoute a la geometry, puis vide
//
//     vertexarray_begin() -> "VertexArray Vec3Array 0 {"
//     vertexarray_append("0.3 5.6 8.2");
//     vertexarray_end() -> "}", ajoute a geometry, puis vide
//
//     colorarray_begin() -> "ColorArray Vec4Array 0 {"
//     colorarray_append("1.0 0.0 0.0 1.0");
//     colorarray_end() -> "}", ajoute a geometry, puis vide
//
//     normalarray_begin() -> "NormalArray Vec3Array 0 {")
//     normalarray_append("0.3 5.6 8.2");
//     normalarray_end() -> "}" ajoute a geometry, puis vide
//
//     texcoordarray_begin() -> "TexCoordArray 0 Vec2Array 0 {")
//     texcoordarray_append("0.3 0.4");
//     texcoordarray_end() -> "}" ajoute a geometry, puis vide
//
//     
//     geometry_append("ColorBinding OVERALL");
//     geometry_append("NormalBinding PER_VERTEX");
//
//   geometry_end() -> "}", puis ajoute la geometry au osg, puis vide
//
// osg_end() -> "} }"
// -------------------
// begin("osg","group { geode {");
//   begin("geometry","geometry {");
//     begin{"stateset",""stateset {");
//     append("stateset","lighting OFF");
//     end{"stateset","}","geometry");
//   end("geometry","}","osg");
// end("osg","} }","");
//

#include <stdio.h>
#include <stdlib.h>


#include "imgu.h"


static int imguOsgBegin(imgu *I,char *key,char *prefix)
{
	imguRemoveKey(I,key,1);
	if( prefix ) imguAddText(I,key,prefix);
	return(0);
}

static int imguOsgEnd(imgu *I,char *key,char *suffix,char *addkey)
{
	if( suffix ) imguAppendText(I,key,suffix);

	if( addkey ) {
		char *txt = imguGetText(I,key);
		if( txt!=NULL ) {
			imguAppendText(I,addkey,txt);
			imguRemoveKey(I,key,0); // la premiere seulement
		}
	}
	return(0);
}

int imguOsgBeginGeode(imgu *I)
{
  return imguOsgBegin(I,"geode","Geode {\n");
}

int imguOsgEndGeode(imgu *I)
{
  return imguOsgEnd(I,"geode","}\n","osg");
}

int imguOsgSave(imgu *I,char *outNameOsg)
{
imgu *J;
FILE *F;
char *osg;
	F=fopen(outNameOsg,"w");
	if( F==NULL ) return(-1);

	// commence le fichier osg par un clear
	fprintf(F,"Group {\n");
	//fprintf(F," ClearNode { clearColor 0 0 0 1}\n");

    J=I;
	for(J=I;J!=NULL;J=J->next) {
		osg=imguGetText(J,"osg");
		if( osg==NULL ) { continue; }
		fputs(osg,F);
	}
	fprintf(F,"}\n");
	fclose(F);
	return(0);
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//
// Support a la creation d'objets 3D
//
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

/////////////////////////////////////////////
////////////////////////////////////////////
//
// visualisation .osg
//
// INTPUT:
//	cam_interne
//	cam_externe
//	saved_filename (nom du fichier utilise pour la texture, auto au dernier Save)
// OUTPUT:
// 	osg (contient le modele 3D de la camera)
//
///////////////////////////////////
//////////////////////////////////

static void imguOsgStateSet(imgu *I,int flags,char *filename)
{
    char bf[200];
    if (I==NULL) return;

    if( filename && (flags&USE_TEXTURE) ) {
	  imguOsgBegin(I,"stateset","    StateSet {\n");
	  imguAppendText(I,"stateset" ,"     GL_BLEND ON\n"); // pour la transparence
	  // il faut ajouter tout ceci pour bien controler la transparence...
	  imguAppendText(I,"stateset" ,"     rendering_hint TRANSPARENT_BIN\n");
	  imguAppendText(I,"stateset" ,"     renderBinMode USE\n");
	  imguAppendText(I,"stateset" ,"     binNumber 10\n");
   	  imguAppendText(I,"stateset" ,"     binName DepthSortedBin\n");

	  imguAppendText(I,"stateset" ,"     textureUnit 0 {\n");
      imguAppendText(I,"stateset" ,"     GL_TEXTURE_2D ON\n");
	  sprintf(bf,"        Texture2D { file \"%s\" }\n",filename);
      imguAppendText(I,"stateset" ,bf);
      imguAppendText(I,"stateset" ,"     }\n");
      imguOsgEnd(I,"stateset","    }\n","geometry");
    }
    else
    {
    }
}

// retourne 0 si ok, -1..-6 : err
// place le plan focal a la distance 'focal' de l'origine
// transparence de l'image affichee. Normalement: 1.0 (opaque)
// r,g,b : couleur des lignes
int imguOsgCamera(imgu *I,camera *cam,vector4 color,char *texfile)
{
char bf[100];
vector4 pbl,pbr,ptl,ptr,pc;

        if (I==NULL) return -1;
        if (cam==NULL) return -1;
        if (cam->img==NULL) return -1;

	//
	// les points images
	//
	if( InternalFormat==Y_ORIGIN_IS_AT_TOP ) {
		camImageToWorld(cam,0.0,0.0,1.0,ptl);
		camImageToWorld(cam,(double)(cam->img->xs),0.0,1.0,ptr);
		camImageToWorld(cam,0.0,(double)(cam->img->ys),1.0,pbl);
		camImageToWorld(cam,(double)(cam->img->xs),(double)(cam->img->ys),1.0,pbr);
	}else{
		camImageToWorld(cam,0.0,(double)(cam->img->ys),1.0,ptl);
		camImageToWorld(cam,(double)(cam->img->xs),(double)(cam->img->ys),1.0,ptr);
		camImageToWorld(cam,0.0,0.0,1.0,pbl);
		camImageToWorld(cam,(double)(cam->img->xs),0.0,1.0,pbr);
	}
	camImageToWorld(cam,(double)(cam->img->xs/2.0),(double)(cam->img->ys/2.0),1.0,pc);
	// le centre de la camera

        //printf("center is (%12.6f,%12.6f,%12.6f,%12.6f)\n",pc[0],pc[1],pc[2],pc[3]);

	// on suppose toujours que le bottom/left est selon InternalFormat
        //printf("bot/left  is (%12.6f,%12.6f,%12.6f,%12.6f)\n",pbl[0],pbl[1],pbl[2],pbl[3]);
        //printf("bot/right is (%12.6f,%12.6f,%12.6f,%12.6f)\n",pbr[0],pbr[1],pbr[2],pbr[3]);
        //printf("top/left  is (%12.6f,%12.6f,%12.6f,%12.6f)\n",ptl[0],ptl[1],ptl[2],ptl[3]);
        //printf("top/right is (%12.6f,%12.6f,%12.6f,%12.6f)\n",ptr[0],ptr[1],ptr[2],ptr[3]);


        // un simple osg
        //imguOsgBegin(I,"geode"," Geode {\n");

        ////////////// les lignes

        imguOsgBegin(I,"geometry","  Geometry {\n");

        imguOsgStateSet(I,0,NULL);

        // les points
        imguOsgBegin(I,"vertexarray","    VertexArray UniqueID v1 Vec3Array 0 {\n");
        sprintf(bf,"%f %f %f\n",cam->position[0],cam->position[1],cam->position[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",ptl[0],ptl[1],ptl[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",pbl[0],pbl[1],pbl[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",pbr[0],pbr[1],pbr[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",ptr[0],ptr[1],ptr[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",pc[0],pc[1],pc[2]);imguAppendText(I,"vertexarray",bf);
        imguOsgEnd(I,"vertexarray","    }\n","geometry");

        // les lignes
        imguOsgBegin(I,"primitiveset","    PrimitiveSets 0 {\n");
        //imguAppendText(I,"primitiveset","      DrawElementsUShort LINES 0 { 0 1 0 2 0 3 0 4 }\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort LINES 0 { 0 5 }\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort LINE_LOOP 0 { 1 2 3 4 }\n");
        imguOsgEnd(I,"primitiveset","    }\n","geometry");

        // les couleurs (une seule pour l'instant) (PER_VOXEL, PER_PRIMITIVE, PER_PRIMITIVE_SET)
        imguAppendText(I,"geometry","    ColorBinding OVERALL\n");
        imguOsgBegin(I,"colorarray","    ColorArray Vec4Array 0 {\n");
        sprintf(bf,"%f %f %f %f\n",color[0],color[1],color[2],color[3]);
        imguAppendText(I,"colorarray",bf);
        imguOsgEnd(I,"colorarray","    }\n","geometry");

        imguOsgEnd(I,"geometry","}\n","geode");

        ////////////// le polygone

        imguOsgBegin(I,"geometry","  Geometry {\n");

        imguOsgStateSet(I,USE_TEXTURE,texfile);

        imguAppendText(I,"geometry","    VertexArray Use v1\n");

        // la texture (les points sont: pc,pbl,pbr,ptr,ptl)
        // TexCoordArray <texture_unit>
        // OPENGL load la texture telle que le y=0 est en haut...
        // ExternalFormat should be 0 at top, otherwise flip texture...
        imguOsgBegin(I,"texcoordarray","    TexCoordArray 0 Vec2Array 0 {\n");
        if( ExternalFormat==Y_ORIGIN_IS_AT_TOP ) {
          imguAppendText(I,"texcoordarray","0 0   0 1   0 0   1 0   1 1   0 0\n");
        }else{ 
          imguAppendText(I,"texcoordarray","0 0   0 0   0 1   1 1   1 0   0 0\n");
        }
        imguOsgEnd(I,"texcoordarray","    }\n","geometry");

        // la normale
        /***
        imguAppendText(I,"geometry","    NormalBinding PER_PRIMITIVE\n");
        imguOsgBegin(I,"normalarray","    NormalArray Vec3Array 0 {\n");
        imguAppendText(I,"normalarray","0.0 0.0 1.0\n");
        imguOsgEnd(I,"normalarray","    }\n","geometry");
        ***/

        imguOsgBegin(I,"primitiveset","    PrimitiveSets 0 {\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort QUADS 0 { 1 2 3 4 }\n");
        imguOsgEnd(I,"primitiveset","    }\n","geometry");

        imguAppendText(I,"geometry","    ColorBinding PER_PRIMITIVE\n");
        imguOsgBegin(I,"colorarray","    ColorArray Vec4Array 0 {\n");
        sprintf(bf,"1.0 1.0 1.0 %f\n",color[3]);
        imguAppendText(I,"colorarray",bf);
        imguOsgEnd(I,"colorarray","    }\n","geometry");


        imguOsgEnd(I,"geometry","}\n","geode");

        //imguOsgEnd(I,"geode"," }\n","osg");


        return(0);
}

/*int imguOsgCameraTruncated(imgu *I,double focal1,double focal2,double transparency,double color[3])
{
int k;
char *camInt,*camExt,*savedName;
double mInt[16],mExt[16];
char bf[100];
double pbl1[4],pbr1[4],ptl1[4],ptr1[4];
double pbl2[4],pbr2[4],ptl2[4],ptr2[4];
double pc[4];
camera cam;

        if (I==NULL) return -1;

        cam.img=I;
        camLoadParams(&cam);

        printf("-- interne --\n"); mat4Print(cam.Mint);
        printf("-- externe --\n"); mat4Print(cam.Mext);

	//
	// les points images
	//
    camImageToWorld(&cam,0.0,0.0,0.0,pc);
	if( InternalFormat==Y_ORIGIN_IS_AT_TOP ) {
		camImageToWorld(&cam,0.0,0.0,focal1,ptl1);
		camImageToWorld(&cam,I->xs-1.0,0.0,focal1,ptr1);
		camImageToWorld(&cam,0.0,I->ys-1.0,focal1,pbl1);
		camImageToWorld(&cam,I->xs-1.0,I->ys-1.0,focal1,pbr1);
	}else{
		camImageToWorld(&cam,0.0,I->ys-1.0,focal1,ptl1);
		camImageToWorld(&cam,I->xs-1.0,I->ys-1.0,focal1,ptr1);
		camImageToWorld(&cam,0.0,0.0,focal1,pbl1);
		camImageToWorld(&cam,I->xs-1.0,0.0,focal1,pbr1);
	}
	// le centre de la camera
	if( InternalFormat==Y_ORIGIN_IS_AT_TOP ) {
		camImageToWorld(&cam,0.0,0.0,focal2,ptl2);
		camImageToWorld(&cam,I->xs-1.0,0.0,focal2,ptr2);
		camImageToWorld(&cam,0.0,I->ys-1.0,focal2,pbl2);
		camImageToWorld(&cam,I->xs-1.0,I->ys-1.0,focal2,pbr2);
	}else{
		camImageToWorld(&cam,0.0,I->ys-1.0,focal2,ptl2);
		camImageToWorld(&cam,I->xs-1.0,I->ys-1.0,focal2,ptr2);
		camImageToWorld(&cam,0.0,0.0,focal2,pbl2);
		camImageToWorld(&cam,I->xs-1.0,0.0,focal2,pbr2);
	}


	// on suppose toujours que le bottom/left est le (0,0) en pixel


        // un simple osg
        //imguOsgBegin(I,"geode"," Geode {\n");

        ////////////// les lignes

        imguOsgBegin(I,"geometry","  Geometry {\n");

        imguOsgBegin(I,"stateset","    StateSet {\n");
        imguAppendText(I,"stateset" ,"     GL_LIGHTING OFF\n");
        //imguAppendText(I,"stateset" ,"     LineWidth { width 2 }\n");
        imguOsgEnd(I,"stateset","    }\n","geometry");

        // les points
        imguOsgBegin(I,"vertexarray","    VertexArray UniqueID v1 Vec3Array 0 {\n");
        sprintf(bf,"%f %f %f\n",pc[0],pc[1],pc[2]);imguAppendText(I,"vertexarray",bf);//0
        sprintf(bf,"%f %f %f\n",pbl1[0],pbl1[1],pbl1[2]);imguAppendText(I,"vertexarray",bf);//1
        sprintf(bf,"%f %f %f\n",pbr1[0],pbr1[1],pbr1[2]);imguAppendText(I,"vertexarray",bf);//2
        sprintf(bf,"%f %f %f\n",ptr1[0],ptr1[1],ptr1[2]);imguAppendText(I,"vertexarray",bf);//3
        sprintf(bf,"%f %f %f\n",ptl1[0],ptl1[1],ptl1[2]);imguAppendText(I,"vertexarray",bf);//4

        sprintf(bf,"%f %f %f\n",pbl2[0],pbl2[1],pbl2[2]);imguAppendText(I,"vertexarray",bf);//5
        sprintf(bf,"%f %f %f\n",pbr2[0],pbr2[1],pbr2[2]);imguAppendText(I,"vertexarray",bf);//6
        sprintf(bf,"%f %f %f\n",ptr2[0],ptr2[1],ptr2[2]);imguAppendText(I,"vertexarray",bf);//7
        sprintf(bf,"%f %f %f\n",ptl2[0],ptl2[1],ptl2[2]);imguAppendText(I,"vertexarray",bf);//8
        imguOsgEnd(I,"vertexarray","    }\n","geometry");

        // les lignes
        imguOsgBegin(I,"primitiveset","    PrimitiveSets 0 {\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort LINES 0 { 1 5 2 6 3 7 4 8 }\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort LINE_LOOP 0 { 1 2 3 4}\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort LINE_LOOP 0 { 5 6 7 8}\n");
        imguOsgEnd(I,"primitiveset","    }\n","geometry");

        // les couleurs (une seule pour l'instant) (PER_VOXEL, PER_PRIMITIVE, PER_PRIMITIVE_SET)
        imguAppendText(I,"geometry","    ColorBinding OVERALL\n");
        imguOsgBegin(I,"colorarray","    ColorArray Vec4Array 0 {\n");
        sprintf(bf,"%f %f %f 1.0\n",color[0],color[1],color[2]);
        imguAppendText(I,"colorarray",bf);
        imguOsgEnd(I,"colorarray","    }\n","geometry");

        imguOsgEnd(I,"geometry","}\n","geode");

        ////////////// le polygone

        imguOsgBegin(I,"geometry","  Geometry {\n");

        imguOsgBegin(I,"stateset","    StateSet {\n");
        imguAppendText(I,"stateset" ,"     GL_LIGHTING OFF\n");
        imguAppendText(I,"stateset" ,"     GL_BLEND ON\n"); // pour la transparence
        // il faut ajouter tout ceci pour bien controler la transparence...
        imguAppendText(I,"stateset" ,"     rendering_hint TRANSPARENT_BIN\n");
        imguAppendText(I,"stateset" ,"     renderBinMode USE\n");
        imguAppendText(I,"stateset" ,"     binNumber 10\n");
        imguAppendText(I,"stateset" ,"     binName DepthSortedBin\n");
        imguAppendText(I,"stateset" ,"     textureUnit 0 {\n");
        imguAppendText(I,"stateset" ,"        GL_TEXTURE_2D ON\n");
	if( savedName==NULL ) savedName="../data/texture_rgba.png";
	sprintf(bf,"        Texture2D { file \"%s\" }\n",savedName);
        imguAppendText(I,"stateset" ,bf);
        imguAppendText(I,"stateset" ,"     }\n");
        //imguAppendText(I,"stateset" ,"     PolygonOffset { factor -1 units  -1 }\n");

        imguOsgEnd(I,"stateset","    }\n","geometry");

        imguAppendText(I,"geometry","    VertexArray Use v1\n");

        // la texture (les points sont: pc,pbl,pbr,ptr,ptl)
        // TexCoordArray <texture_unit>
        imguOsgBegin(I,"texcoordarray","    TexCoordArray 0 Vec2Array 0 {\n");
	if( ExternalFormat==Y_ORIGIN_IS_AT_TOP ) {
		imguAppendText(I,"texcoordarray","0 0   0 0   1 0   1 1   0 1   0 0   1 0   1 1   0 1 \n");
	}else{
		imguAppendText(I,"texcoordarray","0 0   0 1   1 1   1 0   0 0   0 1   1 1   1 0   0 0 \n");
	}
        imguOsgEnd(I,"texcoordarray","    }\n","geometry");

        // la normale
        //imguAppendText(I,"geometry","    NormalBinding PER_PRIMITIVE\n");
        //imguOsgBegin(I,"normalarray","    NormalArray Vec3Array 0 {\n");
        //imguAppendText(I,"normalarray","0.0 0.0 1.0\n");
        //imguOsgEnd(I,"normalarray","    }\n","geometry");

        imguOsgBegin(I,"primitiveset","    PrimitiveSets 0 {\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort QUADS 0 { 1 2 3 4 }\n");
        imguOsgEnd(I,"primitiveset","    }\n","geometry");

        imguAppendText(I,"geometry","    ColorBinding PER_PRIMITIVE\n");
        imguOsgBegin(I,"colorarray","    ColorArray Vec4Array 0 {\n");
        sprintf(bf,"1.0 1.0 1.0 %f\n",transparency);
        imguAppendText(I,"colorarray",bf);
        imguOsgEnd(I,"colorarray","    }\n","geometry");


        imguOsgEnd(I,"geometry","}\n","geode");

        //imguOsgEnd(I,"geode"," }\n","osg");

        return(0);
}*/


//////////
//
// Systeme d'axe (x,y,z)
//
// Prend une matrice et illustre le systeme d'axe
//
// Ajoute le tout dans 'osg'
//
//////////

int imguOsgAxis(imgu *I,double m[16],double scale)
{
vector4 x,y,z,o;
vector4 px,py,pz,po;
matrix4 im;
vector4 color;
char bf[200];

	x[0]=1.0; x[1]=0.0; x[2]=0.0; x[3]=1.0;
	y[0]=0.0; y[1]=1.0; y[2]=0.0; y[3]=1.0;
	z[0]=0.0; z[1]=0.0; z[2]=1.0; z[3]=1.0;
	o[0]=0.0; o[1]=0.0; o[2]=0.0; o[3]=1.0;

    color[0]=0.0;
    color[1]=0.0;
    color[2]=0.0;
    color[3]=1.0;

	mat4Inverse(m,im);
	mat4MultiplyVector(im,x,px);
	mat4MultiplyVector(im,y,py);
	mat4MultiplyVector(im,z,pz);
	mat4MultiplyVector(im,o,po);

	px[3]/=scale;
	py[3]/=scale;
	pz[3]/=scale;
	po[3]/=scale;

        vect4Homogenize3D(px);
        vect4Homogenize3D(py);
        vect4Homogenize3D(pz);
        vect4Homogenize3D(po);

        //imguOsgBegin(I,"geode"," Geode {\n");
        imguOsgBegin(I,"geometry","  Geometry {\n");

        imguOsgStateSet(I,0,NULL);

        // les points
        imguOsgBegin(I,"vertexarray","    VertexArray UniqueID v1 Vec3Array 0 {\n");
        sprintf(bf,"%f %f %f\n",po[0],po[1],po[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",px[0],px[1],px[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",py[0],py[1],py[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",pz[0],pz[1],pz[2]);imguAppendText(I,"vertexarray",bf);
        imguOsgEnd(I,"vertexarray","    }\n","geometry");

        // les lignes
        imguOsgBegin(I,"primitiveset","    PrimitiveSets 0 {\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort LINES 0 { 0 1 0 2 0 3 }\n");
        imguOsgEnd(I,"primitiveset","    }\n","geometry");

        // les couleurs (une seule pour l'instant) (PER_VOXEL, PER_PRIMITIVE, PER_PRIMITIVE_SET)
        imguAppendText(I,"geometry","    ColorBinding PER_PRIMITIVE\n");
        imguOsgBegin(I,"colorarray","    ColorArray Vec4Array 0 {\n");
        imguAppendText(I,"colorarray","1 0 0 1\n");
        imguAppendText(I,"colorarray","0 1 0 1\n");
        imguAppendText(I,"colorarray","0 0 1 1\n");
        imguOsgEnd(I,"colorarray","    }\n","geometry");

        imguOsgEnd(I,"geometry","}\n","geode");

	imguOsgText(I,px,color,36,"X");
	imguOsgText(I,py,color,36,"Y");
	imguOsgText(I,pz,color,36,"Z");

        //imguOsgEnd(I,"geode"," }\n","osg");
	return(0);
}

// ajoute un simple texte dans une geode
int imguOsgText(imgu *I,vector3 position,double color[4],int size,char *txt)
{
char bf[200];
    imguOsgBegin(I,"geometry","  osgText::Text {\n");

    imguOsgStateSet(I,0,NULL);

	//imguAppendText(I,"geometry","     font /usr/local/share/OpenSceneGraph-Data/fonts/fudd.ttf\n");
	sprintf(bf, "     font %s/fonts/arial.ttf\n", getenv("OSG_FILE_PATH"));
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"     fontResolution %d %d\n",size,size);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"     characterSize %d 1\n",size);
	imguAppendText(I,"geometry",bf);
	imguAppendText(I,"geometry","     characterSizeMode SCREEN_COORDS\n");
	imguAppendText(I,"geometry","     autoRotateToScreen TRUE\n");
	sprintf(bf,"    position %f %f %f\n",position[0],position[1],position[2]);
	imguAppendText(I,"geometry",bf);
       	sprintf(bf,"    color %f %f %f %f\n",color[0],color[1],color[2],color[3]);
       	imguAppendText(I,"geometry",bf);

	imguAppendText(I,"geometry","     text \"");
	imguAppendText(I,"geometry",txt);
	imguAppendText(I,"geometry","\"\n");
	imguOsgEnd(I,"geometry","  }\n","geode");

	return(0);
}

int imguOsgQuad(imgu *I,matrix4 mm,vector3 position,vector3 orientation,double width,double height,vector4 color,int flags,char *texfile)
{
int i;
char bf[100];
vector4 pbl,pbr,ptl,ptr,ptemp,ptemp2;
matrix4 minv;
matrix4 R;

    if (I==NULL) return -1;

	mat4Inverse(mm,minv);
    mat4RotationFromAxis(orientation,R);

    ptemp[0]=-width/2;
    ptemp[1]=height/2;
    ptemp[2]=0.0;
    ptemp[3]=1.0;
    mat4MultiplyVector(R,ptemp,ptemp2);
    for (i=0;i<3;i++) ptemp2[i]+=position[i];
	mat4MultiplyVector(minv,ptemp2,ptl);
    vect4Homogenize3D(ptl);
    ptemp[0]=width/2;
    ptemp[1]=height/2;
    ptemp[2]=0.0;
    ptemp[3]=1.0;
    mat4MultiplyVector(R,ptemp,ptemp2);
    for (i=0;i<3;i++) ptemp2[i]+=position[i];
	mat4MultiplyVector(minv,ptemp2,ptr);
    vect4Homogenize3D(ptr);
    ptemp[0]=-width/2;
    ptemp[1]=-height/2;
    ptemp[2]=0.0;
    ptemp[3]=1.0;
    mat4MultiplyVector(R,ptemp,ptemp2);
    for (i=0;i<3;i++) ptemp2[i]+=position[i];
	mat4MultiplyVector(minv,ptemp2,pbl);
    vect4Homogenize3D(pbl);
    ptemp[0]=width/2;
    ptemp[1]=-height/2;
    ptemp[2]=0.0;
    ptemp[3]=1.0;
    mat4MultiplyVector(R,ptemp,ptemp2);
    for (i=0;i<3;i++) ptemp2[i]+=position[i];
	mat4MultiplyVector(minv,ptemp2,pbr);
    vect4Homogenize3D(pbr);

    return imguOsgQuadPoints(I,pbl,pbr,ptl,ptr,color,flags,texfile);
}

int imguOsgQuadPoints(imgu *I,vector3 pbl,vector3 pbr,vector3 ptl,vector3 ptr,vector4 color,int flags,char *texfile)
{
int i;
char bf[100];

    if (I==NULL) return -1;

    imguOsgBegin(I,"geometry","  Geometry {\n");

    imguOsgStateSet(I,0,NULL);

    // les points
    imguOsgBegin(I,"vertexarray","    VertexArray UniqueID v1 Vec3Array 0 {\n");
    sprintf(bf,"%f %f %f\n",ptl[0],ptl[1],ptl[2]);imguAppendText(I,"vertexarray",bf);
    sprintf(bf,"%f %f %f\n",pbl[0],pbl[1],pbl[2]);imguAppendText(I,"vertexarray",bf);
    sprintf(bf,"%f %f %f\n",pbr[0],pbr[1],pbr[2]);imguAppendText(I,"vertexarray",bf);
    sprintf(bf,"%f %f %f\n",ptr[0],ptr[1],ptr[2]);imguAppendText(I,"vertexarray",bf);
    imguOsgEnd(I,"vertexarray","    }\n","geometry");

    // les couleurs (une seule pour l'instant) (PER_VOXEL, PER_PRIMITIVE, PER_PRIMITIVE_SET)
    imguAppendText(I,"geometry","    ColorBinding OVERALL\n");
    imguOsgBegin(I,"colorarray","    ColorArray Vec4Array 0 {\n");
    sprintf(bf,"%f %f %f %f\n",color[0],color[1],color[2],color[3]);
    imguAppendText(I,"colorarray",bf);
    imguOsgEnd(I,"colorarray","    }\n","geometry");

    imguOsgEnd(I,"geometry","}\n","geode");

    ////////////// le polygone
    imguOsgBegin(I,"geometry","  Geometry {\n");

    imguOsgStateSet(I,flags,texfile);

    imguAppendText(I,"geometry","    VertexArray Use v1\n");

    // la texture (les points sont: pbl,pbr,ptr,ptl)
    // TexCoordArray <texture_unit>
    // OPENGL load la texture telle que le y=0 est en haut...
    // ExternalFormat should be 0 at top, otherwise flip texture...
    imguOsgBegin(I,"texcoordarray","    TexCoordArray 0 Vec2Array 0 {\n");
	if( ExternalFormat==Y_ORIGIN_IS_AT_TOP ) {
		imguAppendText(I,"texcoordarray","0 1   0 0   1 0   1 1\n");
	}else{
		imguAppendText(I,"texcoordarray","0 0   0 1   1 1   1 0\n");
	}
    imguOsgEnd(I,"texcoordarray","    }\n","geometry");

    imguOsgBegin(I,"primitiveset","    PrimitiveSets 0 {\n");
    imguAppendText(I,"primitiveset","      DrawElementsUShort QUADS 0 { 0 1 2 3 }\n");
    imguOsgEnd(I,"primitiveset","    }\n","geometry");

    imguAppendText(I,"geometry","    ColorBinding PER_PRIMITIVE\n");
    imguOsgBegin(I,"colorarray","    ColorArray Vec4Array 0 {\n");
    sprintf(bf,"%f %f %f %f\n",color[0],color[1],color[2],color[3]);
    imguAppendText(I,"colorarray",bf);
    imguOsgEnd(I,"colorarray","    }\n","geometry");

    imguOsgEnd(I,"geometry","}\n","geode");

    return(0);
}

// ajoute une sphere a un endroit choisi en 3D exprime dans
// un systeme de coord mm
int imguOsgSphere(imgu *I,matrix4 mm,vector3 position,vector3 orientation,double radius,vector4 color,double detail,int flags,char *texfile)
{
char bf[200];
matrix4 minv;
matrix4 R;
vector4 ptemp,pos,orient;
vector4 quaternion;

	// the point center is expressed in the mm coord system
	// we have p_monde = Inverse(mm) p_mm
	// The radius is always expressed in the world coord system
	mat4Inverse(mm,minv);
    ptemp[0]=position[0];
    ptemp[1]=position[1];
    ptemp[2]=position[2];
    ptemp[3]=1.0;
    mat4MultiplyVector(minv,ptemp,pos);
    vect4Homogenize3D(pos);
    ptemp[0]=orientation[0];
    ptemp[1]=orientation[1];
    ptemp[2]=orientation[2];
    ptemp[3]=1.0;
    mat4MultiplyVector(minv,ptemp,orient);
    vect4Homogenize3D(orient);

    mat4RotationFromAxis(orient,R);
    mat4RotationToQuaternion(R,quaternion);
	
	imguOsgBegin(I,"geometry","  ShapeDrawable {\n");
    imguOsgStateSet(I,flags,texfile);
	sprintf(bf,"    Sphere { Center %f %f %f Radius %f }\n",
		pos[0],pos[1],pos[2],radius);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"    color %f %f %f %f\n",
		color[0],color[1],color[2],color[3]);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"    TessellationHints { detailRatio %f createFaces TRUE FALSE createNormals TRUE createTextureCoords %s createParts TRUE TRUE TRUE }\n",
      detail,
	  (flags&USE_TEXTURE)?"TRUE":"FALSE"
	);
	imguAppendText(I,"geometry",bf);
	imguOsgEnd(I,"geometry","  }\n","geode");

	return(0);
}


// ajoute un cylindre a un endroit choisi en 3D exprime dans
// un systeme de coord mm. radius et height sont dans le monde, pas dans mm.
int imguOsgCylinder(imgu *I,matrix4 mm,vector3 position,vector3 orientation,double radius,double height,vector4 color,double detail,int flags,char *texfile)
{
char bf[200];
matrix4 minv;
matrix4 R;
vector4 ptemp,pos,orient;
matrix4 rotation,rot_offset_x,rot_offset_y,rot_offset,rotcam;
vector4 quaternion;

	// the point center is expressed in the mm coord system
	// we have p_monde = Inverse(mm) p_mm
	// The radius is always expressed in the world coord system
	mat4Inverse(mm,minv);
    ptemp[0]=position[0];
    ptemp[1]=position[1];
    ptemp[2]=position[2];
    ptemp[3]=1.0;
    mat4MultiplyVector(minv,ptemp,pos);
    vect4Homogenize3D(pos);
    ptemp[0]=orientation[0];
    ptemp[1]=orientation[1];
    ptemp[2]=orientation[2];
    ptemp[3]=1.0;
    mat4MultiplyVector(minv,ptemp,orient);
    vect4Homogenize3D(orient);

    //align cylinder correctly with axis: y point up, (0,0,z) points at beginning of cylinder
    mat4XRotation(-90.0,rot_offset_x);
    mat4YRotation(90.0,rot_offset_y);
    mat4Multiply(rot_offset_y,rot_offset_x,rot_offset);
    mat4RotationFromAxis(orient,rotcam);
    mat4Multiply(rotcam,rot_offset,rotation);

    mat4RotationToQuaternion(rotation,quaternion);

	imguOsgBegin(I,"geometry","  ShapeDrawable {\n");
    imguOsgStateSet(I,flags,texfile);
	sprintf(bf,"    Cylinder { Center %f %f %f Radius %f Height %f Rotation %f %f %f %f }\n",
		pos[0],pos[1],pos[2],radius,height,quaternion[0],quaternion[1],quaternion[2],quaternion[3]);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"    color %f %f %f %f\n",
		color[0],color[1],color[2],color[3]);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"    TessellationHints { detailRatio %f createFaces TRUE FALSE createNormals TRUE createTextureCoords %s createParts %s %s %s }\n",
      detail,
	  (flags&USE_TEXTURE)?"TRUE":"FALSE",
	  (flags&USE_TOP)?"TRUE":"FALSE",
	  (flags&USE_SIDE)?"TRUE":"FALSE",
	  (flags&USE_BOTTOM)?"TRUE":"FALSE"
	);
	imguAppendText(I,"geometry",bf);
	imguOsgEnd(I,"geometry","  }\n","geode");
	return(0);
}

int imguOsgCube(imgu *I,matrix4 mm,vector3 position,vector3 orientation,vector3 size,vector4 color,double detail,int flags,char *texfile)
{
char bf[200];
matrix4 minv;
matrix4 R;
vector4 ptemp,pos,orient;
vector4 quaternion;

	// the point center is expressed in the mm coord system
	// we have p_monde = Inverse(mm) p_mm
	// The radius is always expressed in the world coord system
	mat4Inverse(mm,minv);
    ptemp[0]=position[0];
    ptemp[1]=position[1];
    ptemp[2]=position[2];
    ptemp[3]=1.0;
    mat4MultiplyVector(minv,ptemp,pos);
    vect4Homogenize3D(pos);
    ptemp[0]=orientation[0];
    ptemp[1]=orientation[1];
    ptemp[2]=orientation[2];
    ptemp[3]=1.0;
    mat4MultiplyVector(minv,ptemp,orient);
    vect4Homogenize3D(orient);

    mat4RotationFromAxis(orient,R);
    mat4RotationToQuaternion(R,quaternion);

	imguOsgBegin(I,"geometry","  ShapeDrawable {\n");
    imguOsgStateSet(I,flags,texfile);
	sprintf(bf,"    Box { Center %f %f %f HalfLengths %f %f %f Rotation %f %f %f %f }\n",
  	  pos[0],pos[1],pos[2],size[0],size[1],size[2],quaternion[0],quaternion[1],quaternion[2],quaternion[3]);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"    color %f %f %f %f\n",color[0],color[1],color[2],color[3]);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"    TessellationHints { detailRatio %f createFaces TRUE FALSE createNormals TRUE createTextureCoords %s createParts %s %s %s }\n",
      detail,
	  (flags&USE_TEXTURE)?"TRUE":"FALSE",
	  (flags&USE_TOP)?"TRUE":"FALSE",
	  (flags&USE_SIDE)?"TRUE":"FALSE",
	  (flags&USE_BOTTOM)?"TRUE":"FALSE"
	);
	imguAppendText(I,"geometry",bf);
	imguOsgEnd(I,"geometry","  }\n","geode");
	return(0);
}

int imguOsgCone(imgu *I,matrix4 mm,vector3 position,vector3 orientation,double radius,double height,vector4 color,double detail,int flags,char *texfile)
{
char bf[200];
matrix4 minv;
matrix4 R;
vector4 ptemp,pos,orient;
vector4 quaternion;

	// the point center is expressed in the mm coord system
	// we have p_monde = Inverse(mm) p_mm
	// The radius is always expressed in the world coord system
	mat4Inverse(mm,minv);
    ptemp[0]=position[0];
    ptemp[1]=position[1];
    ptemp[2]=position[2];
    ptemp[3]=1.0;
    mat4MultiplyVector(minv,ptemp,pos);
    vect4Homogenize3D(pos);
    ptemp[0]=orientation[0];
    ptemp[1]=orientation[1];
    ptemp[2]=orientation[2];
    ptemp[3]=1.0;
    mat4MultiplyVector(minv,ptemp,orient);
    vect4Homogenize3D(orient);

    mat4RotationFromAxis(orient,R);
    mat4RotationToQuaternion(R,quaternion);

	imguOsgBegin(I,"geometry","  ShapeDrawable {\n");
    imguOsgStateSet(I,flags,texfile);
	sprintf(bf,"    Cone { Center %f %f %f Radius %f Height %f Rotation %f %f %f %f }\n",
		pos[0],pos[1],pos[2],radius,height,quaternion[0],quaternion[1],quaternion[2],quaternion[3]);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"    color %f %f %f %f\n",color[0],color[1],color[2],color[3]);
	imguAppendText(I,"geometry",bf);
	sprintf(bf,"    TessellationHints { detailRatio %f createFaces TRUE FALSE createNormals TRUE createTextureCoords %s createParts TRUE %s %s }\n",
      detail,
	  (flags&USE_TEXTURE)?"TRUE":"FALSE",
	  (flags&USE_SIDE)?"TRUE":"FALSE",
	  (flags&USE_BOTTOM)?"TRUE":"FALSE"
	);
	imguAppendText(I,"geometry",bf);
	imguOsgEnd(I,"geometry","  }\n","geode");
	return(0);
}

// ajoute une ligne entre les points e1 et e2, dans les systemes m1 et m2
int imguOsgLine(imgu *I,matrix4 m1,vector3 e1,matrix4 m2,vector3 e2,vector4 color)
{
char bf[200];
vector4 a,b;
vector4 p1,p2;
matrix4 im1,im2;

    p1[0]=e1[0];
    p1[1]=e1[1];
    p1[2]=e1[2];
    p1[3]=1.0;
    p2[0]=e2[0];
    p2[1]=e2[1];
    p2[2]=e2[2];
    p2[3]=1.0;

	mat4Inverse(m1,im1); mat4MultiplyVector(im1,p1,a); vect4Homogenize3D(a);
	mat4Inverse(m2,im2); mat4MultiplyVector(im2,p2,b); vect4Homogenize3D(b);

        imguOsgBegin(I,"geometry","  Geometry {\n");

        imguOsgStateSet(I,0,NULL);

        // les points
        imguOsgBegin(I,"vertexarray","    VertexArray Vec3Array 0 {\n");
        sprintf(bf,"%f %f %f\n",a[0],a[1],a[2]);imguAppendText(I,"vertexarray",bf);
        sprintf(bf,"%f %f %f\n",b[0],b[1],b[2]);imguAppendText(I,"vertexarray",bf);
        imguOsgEnd(I,"vertexarray","    }\n","geometry");

        // les lignes
        imguOsgBegin(I,"primitiveset","    PrimitiveSets 0 {\n");
        imguAppendText(I,"primitiveset","      DrawElementsUShort LINES 0 { 0 1 }\n");
        imguOsgEnd(I,"primitiveset","    }\n","geometry");

        // les couleurs (une seule pour l'instant) (PER_VOXEL, PER_PRIMITIVE, PER_PRIMITIVE_SET)
        imguAppendText(I,"geometry","    ColorBinding PER_PRIMITIVE\n");
        imguOsgBegin(I,"colorarray","    ColorArray Vec4Array 0 {\n");
        sprintf(bf,"%f %f %f %f\n",color[0],color[1],color[2],color[3]);
        imguAppendText(I,"colorarray",bf);
        imguOsgEnd(I,"colorarray","    }\n","geometry");

        imguOsgEnd(I,"geometry","}\n","geode");


	return(0);
}






