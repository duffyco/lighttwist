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

#include <base.h>

#include <pthread.h>

//#include <iostream>
#include <sstream>


//#include "ImageViews.h"

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

// global a cause du callback...
//ImageViews* IV=NULL;

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


static void background(osgViewer::Viewer *v)
{

	printf("viewer in thread go.\n");

	while (!v->done()) {
		pthread_testcancel();
		usleep(20000);
		v->frame();
	}
	printf("viewer in thread done.\n");
	pthread_exit(0);
}


int main( int argc, char **argv )
{
int camnum=0;
int i,k;
char imageName[200];
int nbCam;
int initW,initH;

imgu *I=NULL;

	nbCam=0;

	//IV=new ImageViews(5);

	imageName[0]=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-i",argv[i])==0 && i+1<argc ) {
			strcpy(imageName,argv[i+1]);
			i++;continue;
		}
	}
	if( imageName[0]==0 ) { printf("img?\n");exit(0); }

	k=imguLoad(&I,imageName,MAKE_16_BITS); // LOAD_AS_IS | MAKE_16_BITS
	if( k ) { printf("unable to load '%s'\n",imageName);exit(0); }

	imgu *IA=NULL;
	imgu *IB=NULL;
	imgu *IC=NULL;
	imgu *ID=NULL;

	int w=I->xs/2;
	int h=I->ys/2;

	imguMirror(&I,I);

	imguExtractRectangle(&IA,I,0,0,w,h);
	imguExtractRectangle(&IB,I,w,0,I->xs-w,h);
	imguExtractRectangle(&IC,I,0,h,w,I->ys-h);
	imguExtractRectangle(&ID,I,w,h,I->xs-w,I->ys-h);


	// construct the viewer.
	osgViewer::Viewer viewer;

	// adjust initial window size
	initW=I->xs;
	initH=I->ys;
	while( initW>1220 || initH>708 ) { initW/=2;initH/=2; }
	while( initW<=1220/2 && initH<=708/2 ) { initW*=2;initH*=2; }

	printf("trying size %d,%d\n",initW,initH);

	//IV->adjustViewer(initW,initH);

	viewer.addEventHandler(new osgViewer::StatsHandler);

	osg::Group* root = new osg::Group();

	//root->addChild(IV->getScene());

	osg::Node* rec=createRectangleCam(0.0,0.0,1.0,1.0);
	root->addChild(rec);

	viewer.setSceneData(root);

	if (!viewer.isRealized()) { viewer.realize(); }

	//IV->resetAllViews();

        pthread_t threads;
        pthread_attr_t pthread_custom_attr;

	pthread_attr_init(&pthread_custom_attr);
	pthread_create(&threads, &pthread_custom_attr, (void* (*)(void*))background, &viewer);

	for(i=0;!viewer.done();i++) {
		//printf("Zzzzz... %d done=%d\n",i,viewer.done());
		sleep(1);
	}

	viewer.setDone(true);

	//pthread_cancel(threads);

	printf("waiting to be done.\n");

	pthread_join(threads,NULL);
   
	printf("we are done.\n");

	//delete IV;

}

