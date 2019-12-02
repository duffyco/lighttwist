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

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osg/Notify>
#include <osg/TextureRectangle>
#include <osg/Texture>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/StateAttribute>
#include <osg/StateSet>
#include <osg/Group>
#include <osg/TexMat>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osgText/Text>
#include <osgDB/Registry>
#include <osg/Camera>
#include <osg/io_utils>
#include <osg/GraphicsContext>


#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <imgu.h>

#include <sstream>

/*** this is the main structure that represent the main scene containing the views ***/
/* it is simply the viewer, base node for viewing, keyboard handle, picking handler,  */



/*** this is the structure that represent a single view on an image ***/

typedef struct {
	osg::Group* root;  // contains the complete subgraph to view this image
	osg::Image* img; // image in the texture. must be check to be .valid()
    	osg::TexMat* texmat; // texture matrix for scale/offset/aspect
	// quad coordinates (to be able to move the quad eventually)
	// not it is always derived from xmin,xmax,ymin,ymax
	osg::Vec3Array* vertices; // 4 coordinates, x/y = 0..1, z=0
	// viewer used to display
	osgViewer::Viewer* viewer; // to handle resize of the window
	// position information (used by viewportView)
	double xoff,yoff; // translation offset (normal: 0,0)
	double zoom; // zoom (normal=1, >1 = bigger pixel)
	double xc,yc; // center of zoom (normal 0.5,0.5)
	/// relative size of image in the window (not used by picking)
	double xmin,ymin,xmax,ymax;
} view;


// coord relatives... (0..1 en X et en Y)
// reset the texmat, based on xoff,yoff,zoom,xc,yc, and image size
int viewportView(view *v);

void resetView(view *v);

// ajuste une vue sur une image donnee
int updateView(view *v,imgu *I);

// update a position
int updateViewPos(view *v,double x[4],double y[4]);


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////



// si I !=nulll, on utilise l'image
int createRectanglePos(view *vv,osgViewer::Viewer* viewer,const char *name,double xmin,double ymin,double xmax,double ymax);

int createRectangle(view *vv,osgViewer::Viewer* viewer,char *name);



#endif
