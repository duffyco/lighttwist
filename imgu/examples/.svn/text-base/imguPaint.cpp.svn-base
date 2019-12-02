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
#include <stdlib.h>

#define MULTICAST	"226.0.0.1"

#define HEADERSIZE	27

#if defined(TRY_IMGU8)
	#include "imgu8.h"
#else
	#include "imgu.h"
#endif


//
// pour le viewer
//
#include "imguextra.h"


#include <bmc/udpcast.h>
#include <bmc/bimulticast.h>


//
// dessine une mise a jour dans un carre au hasard
//
void drawIntoImage(imgu *I)
{
int x,y,c;
int dx,dy;
int w,h;
	w=15;
	h=15;
	x=rand()%(I->xs-w);
	y=rand()%(I->ys-h);

	for(dy=0;dy<h;dy++)
	for(dx=0;dx<w;dx++)
	for(c=0;c<I->cs;c++) {
		PIXEL(I,x+dx,y+dy,c)=IMGU_MAXVAL^PIXEL(I,x+dx,y+dy,c);
	}
}



int main( int argc, char **argv )
{
ImageViews* IV;
int vnum;
udpcast udp;//udpcast udp;
char buf[2000];
int i,x,y,width,height,t,j,bpp,yy,xx,imwidth,imheight;
int refresh;
char buffer[HEADERSIZE];
int port;
char mcast[100];

	imwidth=640;
	imheight=480;
	bpp=3;
	port=15000;
	strcpy(mcast,MULTICAST);

	for(i=1;i<argc;i++) {
		if( strcmp(argv[i],"-w")==0 && i+1<argc ) {
			imwidth=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-h")==0 && i+1<argc ) {
			imheight=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-bpp")==0 && i+1<argc ) {
			bpp=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-p")==0 && i+1<argc ) {
			port=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-mcast")==0 && i+1<argc ) {
			strcpy(mcast,argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-nomcast")==0 ) {
			mcast[0]=0;
		}
	}

	if( udp_init_receiver(&udp,port,mcast[0]?mcast:NULL) ) { printf("NO UDP!\n");exit(0); }

	IV=new ImageViews(10,1); // 1 vue, avec HUD

	imgu *IA=NULL;  // ne pas oublier le NULL!

	/*if( imguLoad(&IA,"/home/radhwan/Bureau/Capture.png",LOAD_16_BITS) ) {
		printf("Unable to load image\n");
		exit(0);
	}
	
	bi_receive_data(&udp,buf,sizeof(buf)-1);
	buffer[5]='\0';
	for(t=0;t<5;t++)
		buffer[t]=buf[t];
	imwidth=atoi(buffer);
	for(t=0;t<5;t++)
		buffer[t]=buf[5+t];
	imheight=atoi(buffer);
	for(t=0;t<5;t++)
		buffer[t]=buf[10+t];
	bpp=atoi(buffer);*/

	printf("%d %d %d\n",imwidth,imheight,bpp);
	
	IA=NULL;
	imguAllocate(&IA,imwidth,imheight,bpp);

	vnum=IV->addView("blub",0.0,0.0,1.0,1.0);
	
	printf("ImguHeight =%d ImguWidth=%d",IA->ys,IA->xs);



	//IV->updatePosition(num,osg::Matrix::identity());
	//IV->updatePosition(num,osg::Matrix::scale(1,1,1)*osg::Matrix::translate(0.5*nbCam,0.,0.)*osg::Matrix::rotate(M_PI/4*nbCam,0.,0.,1.) );
	/*osg::Matrix m;
	m.set(	1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		1,1,1,1);
	//IV->updatePosition(vnum,osg::Matrix::rotate(M_PI/4,0.,0.,1.));
	IV->updatePosition(vnum,m);*/

	// construct the viewer.
	osgViewer::Viewer* viewer=new osgViewer::Viewer();

	viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

	IV->setView(viewer);

	IV->adjustViewer(1440,0,800,600,false);
	//IV->adjustViewerFullScreen();


	viewer->addEventHandler(new osgViewer::StatsHandler);

	osg::Group* root = new osg::Group();
	root->addChild(IV->getScene());
	viewer->setSceneData(root);

	if (!viewer->isRealized()) { viewer->realize(); }
	
	for(yy=0;yy<imheight;yy++)
		for(xx=0;xx<imwidth;xx++)
			for(j=0;j<bpp;j++)
				PIXEL(IA,xx,yy,j)=IMGU_MAXVAL;

	IV->updateViewImage(vnum,IA);

	IV->resetAllViews();
	
	/*
	
	IV->updateViewImage(vnum,IA);
	
	viewer->frame();
	usleep(20000);
	*/
	for(i=0;!viewer->done();i++) {
		// receive stuff from network
		refresh=0;
		
		while( udp_receive_data_poll(&udp,(unsigned char*)buf,sizeof(buf)-1)>0 ) {
			//buf[strlen(buf)]=0;
			//printf("got >>%s<<\n",buf);
	
			if(strncmp(buf,"pain",4)==0||strncmp(buf,"unpa",4)==0){
				// copy header for analysis
				for(t=0;buf[t];t++) buffer[t]=buf[t]; buffer[t]=0;

				height=atoi(buffer+21);buffer[21]=0;
				width=atoi(buffer+16);buffer[16]=0;
				y=atoi(buffer+11);buffer[11]=0;
				x=atoi(buffer+6);buffer[6]=0;
				bpp=atoi(buffer+5);
				printf("bpp=%d (%d,%d)  %d %d\n",bpp,x,y,width,height);
				
				for(yy=0;yy<height;yy++){
					for(xx=0;xx<width;xx++){
					/*for(xx=0;xx<width*3;xx++){
						printf("%3d",*(buf+20+(x*3+y*imwidth*3)+(yy*width*3)+xx);
					*/
						if( x+xx<0 || x+xx>=IA->xs || y+yy<0 || y+yy>=IA->ys ) continue;
						for(j=0;j<bpp && j<IA->cs;j++)
							PIXEL(IA,x+xx,y+yy,j)=(*(buf+HEADERSIZE+(yy*width+xx)*bpp+j))*257;
					}	
					//printf("\n");
				}
			}
			//drawIntoImage(IA);
			refresh=1;
		}

		if( refresh ) IV->updateViewImage(vnum,IA);

		viewer->frame();
		usleep(20000);
	}

	//udp_uninit_receiver(&udp);
	delete IV;
	delete viewer;

}

