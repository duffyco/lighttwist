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

#include <base.h>

// xmin..xmax,ymin..ymax est la position du quad dans l'image de la camera (en fraction 0..1)
osg::Node* createRectangleCam(double xmin,double ymin,double xmax,double ymax)
{
    // create geometry
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = osg::Vec3(xmin,ymax,0.0);	// top left
    (*vertices)[1] = osg::Vec3(xmin,ymin,0.0);	// bottom left;
    (*vertices)[2] = osg::Vec3(xmax,ymin,0.0);	// bottom right;
    (*vertices)[3] = osg::Vec3(xmax,ymax,0.0);	// top right;
    geom->setVertexArray(vertices);

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

    osg::TextureRectangle* texture = new osg::TextureRectangle(new osg::Image);

    // pour voir les pixels
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);

    texture->setBorderColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture::CLAMP_TO_BORDER);

    osg::TexMat *texmat = new osg::TexMat;
    texmat->setScaleByTextureRectangleSize(true);

    // setup state
    osg::StateSet* state;
    state = geom->getOrCreateStateSet();
    state->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
    state->setTextureAttributeAndModes(0, texmat, osg::StateAttribute::ON);

    // to see under the current image
    state->setMode(GL_BLEND,osg::StateAttribute::ON);
    state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    // turn off lighting
    state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    geom->setName("imguView"); // pour retrouver le stateset

    osg::Group* root = new osg::Group;

    osg::ClearNode* clearNode = new osg::ClearNode;
    clearNode->setRequiresClear(false); // we've got base and sky to do it.

    root->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    root->addChild(clearNode);

    osg::Camera* camera = new osg::Camera;
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1,0,1));
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(0);
    camera->setAllowEventFocus(false);

    camera->addChild(geode);
    root->addChild(camera);
    return root;
}




