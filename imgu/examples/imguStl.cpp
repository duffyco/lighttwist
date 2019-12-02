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

#include <pthread.h>

#include "ImageViews.h"

#define DEVICE "V4L"

int viewCam,viewProj;

imgu *IP; // pattern STL

////////////////////////////////////////////////////////////////

// global a cause du callback...
ImageViews* IVproj=NULL;
ImageViews* IVcam=NULL;

////////////////////////////////////////////////////////////////

///////////////// gray code ////////////////

int encode(int n) { return( n ^ (n>>1) ); }

int decode(int g, int nb)
{
int n,m,i;
	n=0;
	for(i=nb,m=1<<nb;i>=0;i--,m>>=1) n|=((n>>1)&m)^(g&m);
	return(n);
}

////////////////////////////////////////////////////////////////

int getNbBit(int w)
{
int nb=1;
	while( (1<<nb) < w ) nb++;
	return(nb);
}



// bit: 0..nbx-1  0..nby-1 + nbx  0..nbx-1 + nbx+nby  0..nby-1 + nbx+nby+nbx
// retourne 0 si ok, -1 si fini
int stlPattern(imgu *I,int bit)
{
int i,x,y,c,mask,gx,gy;
int inverse,seqy;
int nbx,nby;

	nbx=getNbBit(I->xs);
	nby=getNbBit(I->ys);

        //unsigned char *p=(unsigned char *)stlFrame->imageData;

        if( bit<0 || bit>=(nbx+nby)*2 ) {
                stlPattern(I,0);
                return(-1);
        }

        inverse=0;
        if( bit>=nbx+nby ) { inverse=1;bit-=nbx+nby; }
        seqy=0;
        if( bit>=nbx ) { seqy=1; bit-=nbx; }

        // now we have bit = 0..nbx-1 or 0..nby-1, inverse and seqy

        printf("nbx=%d nby=%d  bit=%d inverse=%d seqy=%d\n",nbx,nby,bit,inverse,seqy);

        i=0;
        mask=1<<bit;
        for(y=0;y<I->ys;y++)
        for(x=0;x<I->xs;x++,i++) {
                gx=encode(x);
                gy=encode(y);
                if( seqy ) {
                        if( inverse )   PIXEL(I,x,y,0)=(gy&mask)?0:65535;
                        else            PIXEL(I,x,y,0)=(gy&mask)?65535:0;
                }else{
                        if( inverse )   PIXEL(I,x,y,0)=(gx&mask)?0:65535;
                        else            PIXEL(I,x,y,0)=(gx&mask)?65535:0;
                }
        }
        return(0);
}



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

int makePattern(imgu *I,int p)
{
int i;
	for(i=0;i<I->xs*I->ys*I->cs;i++) I->data[i]=(i+p)*256;
	return 0;
}


int bit=0;
int blackout=0;

int callback(capture *cap,imgu **I,int i,int err)
{
	printf("Got image %d (err=%d)!\n",i,err);
	if( err ) return(1);

    if (blackout>0) {
        blackout--; 
        return 1;
    }

	// on sait que c'est du 12bit... donc on doit faire <<4
	int j;
	unsigned short *p=I[i]->data;
	j=I[i]->xs*I[i]->ys*I[i]->cs;
	//while( --j ) *p++<<=4;
	while( --j ) *p++<<=8; // des bytes en short

	IVcam->updateViewImage(viewCam,I[i]);

	// update le pattern STL
    printf("Bit = %i\n", bit);
	stlPattern(IP,bit);
	bit=(bit+1)%((getNbBit(IP->xs)+getNbBit(IP->ys))*2);
	IVproj->updateViewImage(viewProj,IP);

    blackout=5;

	return(1); // remettre dans la file
}
        
       




int main( int argc, char **argv )
{
char InName[100];
//imgu *IC; // camera
int i,k;

	IVcam=new ImageViews(10);
	IVproj=new ImageViews(10);

	if( InName[0]==0 ) exit(0);

	/*** Camera ***/

	if( imguCaptureInit(DEVICE) ) { printf("A!!\n");exit(0); }


	char **camNames;
        camNames=imguCaptureList(DEVICE);
        if( camNames==NULL || camNames[0]==NULL ) {
                printf("no camera available\n");
                imguCaptureUninit(DEVICE);
                exit(0);
        }

	for(i=0;camNames[i];i++) printf("cam %d is '%s'\n",i,camNames[i]);


	capture cap;
        if( imguCaptureOpen(DEVICE,camNames[1],&cap) ) {
                printf("unable to start capture\n");
                imguCaptureUninit(DEVICE);
                exit(0);
        }

		// set la resolution de la camera
		char buf[100];
        
		imguCaptureGetParam(&cap,"resolution",buf,sizeof(buf));
		printf("Get resolution (before) : '%s'\n",buf);
        
		imguCaptureSetParam(&cap,"resolution","768x480",0);
        
		imguCaptureGetParam(&cap,"resolution",buf,sizeof(buf));
		printf("Get resolution (after): '%s'\n",buf);
		

	sleep(1);

	/*** affichage ****/
	viewCam=IVcam->addView("cam",0.0,0.0,1.0,1.0);
	viewProj=IVproj->addView("stl",0.0,0.0,1.0,1.0);


	// start cam

        imgu *I[20];
        for(i=0;i<20;i++) I[i]=NULL;

        k=imguCaptureMany(&cap,I,20,(int (*)())callback);


	/*** Pattern ***/

	IP=imguCreate(1024,768,1);

	makePattern(IP,0);

	IVproj->updateViewImage(viewProj,IP);

    // construct the viewer.
    osgViewer::Viewer viewerProj;
    osgViewer::Viewer viewerCam;

    IVproj->setViewer(&viewerProj);
    IVcam->setViewer(&viewerCam);

    int initW,initH;
    initW=256+IP->xs;
    initH=256;
    if( initH<IP->ys ) initH=IP->ys;

    printf("trying size %d,%d\n",initW,initH);

    IVcam->adjustViewer(0,0,256,256,true);
    IVproj->adjustViewer(1024,0,1024,768,false);


   viewerCam.addEventHandler(new osgViewer::StatsHandler);

   viewerCam.setSceneData(IVcam->getScene());
   viewerProj.setSceneData(IVproj->getScene());

   if (!viewerCam.isRealized()) { viewerCam.realize(); }
   if (!viewerProj.isRealized()) { viewerProj.realize(); }

   IVcam->resetAllViews();
   IVproj->resetAllViews();

        pthread_t threadsCam;
        pthread_attr_t pthread_custom_attrCam;

        pthread_t threadsProj;
        pthread_attr_t pthread_custom_attrProj;

	pthread_attr_init(&pthread_custom_attrCam);
	pthread_create(&threadsCam, &pthread_custom_attrCam, (void* (*)(void*))background, &viewerCam);

	pthread_attr_init(&pthread_custom_attrProj);
	pthread_create(&threadsProj, &pthread_custom_attrProj, (void* (*)(void*))background, &viewerProj);

	for(i=0;!viewerCam.done();i++) {
		//printf("Zzzzz... %d done=%d\n",i,viewer.done());
		sleep(1);
	}


	viewerCam.setDone(true);
	viewerProj.setDone(true);

	//pthread_cancel(threads);

	printf("waiting to be done.\n");

	pthread_join(threadsCam,NULL);
    pthread_join(threadsProj,NULL);
   
	printf("we are done.\n");

	// done
	imguCaptureClose(&cap);
	imguCaptureUninit(DEVICE);

	delete IVcam;
	delete IVproj;

}

