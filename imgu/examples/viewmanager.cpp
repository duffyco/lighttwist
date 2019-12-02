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

#include <viewmanager.h>

#include <iostream>
#include <sstream>


static void computeAspect(view *v,double *aspectX,double *aspectY)
{
	if( !v->img->valid() ) { *aspectX=*aspectY=1.0;return; }
	if( v->viewer!=NULL ) {
		const osg::GraphicsContext::Traits* tr=v->viewer->getCamera()->getGraphicsContext()->getTraits();

		*aspectX=(double)tr->width*(v->xmax - v->xmin)/v->img->s();
		*aspectY=(double)tr->height*(v->ymax - v->ymin)/v->img->t();
	}else{
		*aspectX=v->img->s();
		*aspectY=v->img->t();
	}
/*
	if( *aspectX>*aspectY ) { *aspectY/=*aspectX;*aspectX=1.0; }
	else{ *aspectX/=*aspectY;*aspectY=1.0; }
*/
	if( *aspectX<*aspectY ) { *aspectY/=*aspectX;*aspectX=1.0; }
	else{ *aspectX/=*aspectY;*aspectY=1.0; }
}



// coord relatives... (0..1 en X et en Y)
// reset the texmat, based on xoff,yoff,zoom,xc,yc, and image size
int viewportView(view *v)
{
	//printf("image size is (%d,%d) %d\n",v->img->s(),v->img->t(),v->img->valid());
	if( !v->img->valid() ) return(-1);

	// aspect ratio!
	double aspectX,aspectY;
	computeAspect(v,&aspectX,&aspectY);


	double sx,sy;
	if( v->zoom <= 0.0 ) v->zoom=1.0;

	double tx=v->xoff*v->img->s();
	double ty=v->yoff*v->img->t();
	double xcp=v->xc*v->img->s();
	double ycp=v->yc*v->img->t();

	v->texmat->setMatrix(
		osg::Matrix::translate(-xcp,-ycp,0.0)
		*osg::Matrix::scale(v->zoom,v->zoom,1)
		*osg::Matrix::translate(xcp,ycp,0.0)
		*osg::Matrix::translate(tx,ty,0.0)
		*osg::Matrix::scale(aspectX,aspectY,1)
	);

	return(0);
}


void resetView(view *v) {
	printf("RESET VIEW!\n");
	v->xoff=v->yoff=0;
	v->zoom=1.0;
	v->xc=v->yc=0.5;

	if( v->img==NULL ) { printf("no image\n");return; }
	if( v->viewer==NULL
	 || v->viewer->getCamera()==NULL
	 || v->viewer->getCamera()->getGraphicsContext()==NULL
	 || v->viewer->getCamera()->getGraphicsContext()->getTraits()==NULL ) { printf("no traits\n");return; }

	// aspect ratio!
	double aspectX,aspectY;
	computeAspect(v,&aspectX,&aspectY);

	const osg::GraphicsContext::Traits* tr;

	// choisit un zoom approprie...
	// The aspect is always <=1, so the image is larger than the window
	// the aspect=1.0 is the reference for the zoom.
	if( v->img->valid() ) {
		/*printf("Window is (%d,%d) image is (%d,%d)\n",
			tr->width,tr->height, v->img->s(),v->img->t());*/
		if( v->viewer!=NULL ) {
			if( tr->width>=tr->height ) {
				v->zoom=(double)tr->width/v->img->s();
			}else{
				v->zoom=(double)tr->height/v->img->t();
			}
		}else{
			v->zoom=1.0;
		}
		if( v->zoom>1.0 ) v->zoom=1.0; // window is bigger than image. Scale to image
		printf("zoom is %f\n",v->zoom);
	}
	viewportView(v);
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


class CameraEventCallback : public osg::NodeCallback
{
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
      if (nv->getVisitorType()==osg::NodeVisitor::EVENT_VISITOR) {
	osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
	if (ev) {
		osgGA::GUIActionAdapter* aa = ev->getActionAdapter();
		osgGA::EventQueue::Events& events = ev->getEvents();
		for(osgGA::EventQueue::Events::iterator itr=events.begin(); itr!=events.end(); ++itr) {
			handle(*(*itr), *aa, node, ev);
		}
	}
      }
        traverse(node,nv); // necessaire pour aller voir les enfants
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&, osg::Object* object, osg::NodeVisitor*)
    {
	osg::Camera* camera = dynamic_cast<osg::Camera*>(object);
	if (!camera) return false;

	switch(ea.getEventType()) {
                case(osgGA::GUIEventAdapter::MOVE):
                case(osgGA::GUIEventAdapter::DRAG):
                {
                    printf("camera move %f %f\n",ea.getY(),ea.getX());
                    break;
                }
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
			printf("camera keydown '%c'\n",ea.getKey());
                    break;
                }
                case(osgGA::GUIEventAdapter::RESIZE):
		{
			printf("camera RESIZE to (%d,%d)\n",ea.getWindowWidth(),ea.getWindowHeight());
		}
                default:
                    break;
            }
            return false; // not handled!
    }
};


class GeodeEventCallback : public osg::NodeCallback
{
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
      if (nv->getVisitorType()==osg::NodeVisitor::EVENT_VISITOR) {
	osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
	if (ev) {
		osgGA::GUIActionAdapter* aa = ev->getActionAdapter();
		osgGA::EventQueue::Events& events = ev->getEvents();
		for(osgGA::EventQueue::Events::iterator itr=events.begin(); itr!=events.end(); ++itr) {
			handle(*(*itr), *aa, node, ev);
		}
	}
      }
        traverse(node,nv);
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&, osg::Object* object, osg::NodeVisitor*)
    {
	osg::Geode* geode = dynamic_cast<osg::Geode*>(object);
	if (!geode) return false;

	switch(ea.getEventType()) {
                case(osgGA::GUIEventAdapter::MOVE):
                case(osgGA::GUIEventAdapter::DRAG):
                {
                    printf("geode move %f %f\n",ea.getY(),ea.getX());
                    break;
                }
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
			printf("geode keydown '%c'\n",ea.getKey());
                    break;
                }
                case(osgGA::GUIEventAdapter::RESIZE):
			printf("geode RESIZE\n");
			break;
                default:
                    break;
            }
            return false; // not handled
    }

};









// si I !=nulll, on utilise l'image
int createRectanglePos(view *vv,osgViewer::Viewer* viewer,const char *name,double xmin,double ymin,double xmax,double ymax)
{
    vv->viewer=viewer;
    osg::Vec3 top_left(    xmin,ymax,0.0);
    osg::Vec3 bottom_left( xmin,ymin,0.0);
    osg::Vec3 bottom_right(xmax,ymin,0.0);
    osg::Vec3 top_right(   xmax,ymax,0.0);

	vv->xmin=xmin;
	vv->ymin=ymin;
	vv->xmax=xmax;
	vv->ymax=ymax;


    // create geometry
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = top_left;
    (*vertices)[1] = bottom_left;
    (*vertices)[2] = bottom_right;
    (*vertices)[3] = top_right;
    geom->setVertexArray(vertices);

    vv->vertices=vertices;

    osg::Vec2Array* texcoords = new osg::Vec2Array(4);
    (*texcoords)[0].set(0.0f, 0.0f);
    (*texcoords)[1].set(0.0f, 1.0f);
    (*texcoords)[2].set(1.0f, 1.0f);
    (*texcoords)[3].set(1.0f, 0.0f);
    geom->setTexCoordArray(0,texcoords);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(0.0f,0.0f,1.0f);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

    // disable display list so our modified tex coordinates show up
    geom->setUseDisplayList(false);

#ifdef SKIP
    if( I==NULL ) {
	// load image
	printf("loading\n");
	img = osgDB::readImageFile(filename);
    }else{
	printf("creating\n");
	img = new osg::Image;
	img->setImage(I->xs,I->ys, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);

    }

    // stats sur l'image
    printf("pixel format = %d\n",img->getPixelFormat());
printf("   %d %d %d %d %d %d %d\n", GL_LUMINANCE,GL_ALPHA,GL_LUMINANCE_ALPHA,GL_RGB,GL_RGBA,GL_BGR,GL_BGRA);
    printf("data type    = %d\n",img->getDataType());

    printf("   %d %d %d %d %d %d %d\n",GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT);


    printf("packing      = %d\n",img->getPacking());
    printf("pixel size bits = %d\n",img->getPixelSizeInBits());
    printf("row size bytes  = %d\n",img->getRowSizeInBytes());
    printf("image size bytes= %d\n",img->getImageSizeInBytes());
    printf("total size bytes= %d\n",img->getTotalSizeInBytes());


    // setup texture
    //osg::TextureRectangle* texture = new osg::TextureRectangle(img);
    texture = new osg::TextureRectangle(img);
#endif

    vv->img = new osg::Image;
	printf("created image 0x%08lx valid is %d\n",vv->img,vv->img->valid());
    osg::TextureRectangle* texture;
    texture = new osg::TextureRectangle(vv->img);

    // pour voir les pixels
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);

    texture->setBorderColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture::CLAMP_TO_BORDER);


    vv->texmat = new osg::TexMat;
    vv->texmat->setScaleByTextureRectangleSize(true);

	// scale l'image et translate (en pixels de l'image)
    //vv->texmat->setMatrix(osg::Matrix::scale(1,1,1)*osg::Matrix::translate(0,0,0.0));

    // setup state
    osg::StateSet* state;
    state = geom->getOrCreateStateSet();
    state->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
    state->setTextureAttributeAndModes(0, vv->texmat, osg::StateAttribute::ON);

    // to see under the current image
    state->setMode(GL_BLEND,osg::StateAttribute::ON);
    state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);


    // turn off lighting
    state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    // install 'update' callback
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    //geode->setUpdateCallback(new TexturePanCallback(texmat));
    geode->setName(name);

   geom->setName("imguView"); // pour retrouver le stateset

   vv->root = new osg::Group;

   osg::ClearNode* clearNode = new osg::ClearNode;
   clearNode->setRequiresClear(false); // we've got base and sky to do it.

   vv->root->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
   vv->root->addChild(clearNode);

    osg::Camera* camera = new osg::Camera;
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1,0,1));
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(0);
    camera->setAllowEventFocus(false);

    camera->addChild(geode);
    vv->root->addChild(camera);

    camera->setEventCallback(new CameraEventCallback);
    geode->setEventCallback(new GeodeEventCallback);

    return 0;
}


int createRectangle(view *vv,osgViewer::Viewer* viewer,char *name)
{
	return createRectanglePos(vv,viewer,name,0.0,0.0,1.0,1.0);
}


// ajuste une vue sur une image donnee
int updateView(view *v,imgu *I)
{
	//printf("UV img is 0x%08lx set to (%d,%d)\n",v->img,I->xs,I->ys);

	// change le buffer (dirty sera appelle sur la texture)
	if( I->cs==1 ) {
		v->img->setImage(I->xs,I->ys, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
	}else if( I->cs==3 ) {
		v->img->setImage(I->xs,I->ys, 1, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
	}else if( I->cs==4 ) {
		v->img->setImage(I->xs,I->ys, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
	}
	return(0);
}

// ajuste une vue sur une image donnee
// change la position des quads
int updateViewPos(view *v,double x[4],double y[4])
{
    (*(v->vertices))[0] = osg::Vec3(x[0],y[0],0);
    (*(v->vertices))[1] = osg::Vec3(x[1],y[1],0);
    (*(v->vertices))[2] = osg::Vec3(x[2],y[2],0);
    (*(v->vertices))[3] = osg::Vec3(x[3],y[3],0);
    return(0);
}




