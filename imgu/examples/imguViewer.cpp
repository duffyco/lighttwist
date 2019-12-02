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


#if defined(TRY_IMGU8)
	#include <imgu8.h>
#elif defined(TRY_IMGU16)
	#include <imgu16.h>
#else
	#include <imgu.h>
#endif


// ne pas utiliser sur MAC!!!!
//#define USE_THREADS


#ifdef USE_THREADS
#include <pthread.h>
#endif

//#include <iostream>
#include <sstream>


#include <imguextra.h>

//#define DEVICE	"V4L"
//#define DEVICE	"gige"
//#define DEVICE	"empty"


// combien de shift a gauche?
int pixelShift;


// 10 vues! (une vue qui contient root=NULL est vide)
// la derniere vue contient root==null
#define NB_IV	51


imgu *imgLoaded[NB_IV]; // il faut garder en memoire tant que la texture n'est pas envoyee dans la carte graphique...



/*
#define NB_SAVE	200
imgu *ISaved[NB_SAVE];
int nbSaved;
*/

// une image de reference...
int ReferenceMode=0; // 0=pas de reference, 1=please get next image as reference, 2=use reference
imgu *Iref;

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


class LocalEventHandler : public osgGA::GUIEventHandler
{
public:

        LocalEventHandler(char *rien) {
		printf("extra keyboard handler stared\n");
        }

        bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
	{
		switch(ea.getEventType()) {
                  case(osgGA::GUIEventAdapter::KEYUP):
			printf("EXTRA KEYUP ::::: %d\n",ea.getKey());
			if( ea.getKey()=='r' ) {
				ReferenceMode=1;
			}
			if( ea.getKey()=='t' ) {
				if( ReferenceMode==2 ) ReferenceMode=0;
				else if( ReferenceMode==0 && Iref!=NULL ) ReferenceMode=2;
			}
                        break;
		  default:
			break;
		}
		return(1);
	}

};



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

// global a cause du callback...
ImageViews* IV=NULL;

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////


#define LMIN -1
#define LMAX 0

int ComputeDiff(imgu *I,imgu *Ir)
{
int i;
int a,b,v;
double ratio,L;
	for(i=0;i<I->xs*I->ys*I->cs;i++) {
		a=Ir->data[i];
		b=I->data[i];
		if( a==0 ) a=1;
		if( b==0 ) b=1;
		ratio=(double)b/(double)a;
		L=log2(ratio);
		if( i==200*I->xs+300 ) printf("a=%6d b=%6d R=%12.6f L=%12.6f\n",a,b,ratio,L);
		v=(L-LMIN)/(LMAX-LMIN)*65535;
		if( v<0 ) v=0;
		if( v>65535 ) v=65535;
		//I->data[i]=(b-a)/2+32768;
		I->data[i]=v;
	}
	return(0);
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

// on recoit une image ici! (dans ffmpeg, toujours la meme image...)
int callback(imgu *I)
{
#ifdef SKIP
	if( pixelShift ) {
		// on doit shifter de pixelshift bits <<4
		int j;
		pix_t *p=I->data;
		j=I->xs*I->ys*I->cs;
		if( pixelShift>0 )	while( --j ) *p++<<=pixelShift;
		else 				while( --j ) *p++>>=(-pixelShift);
	}
#endif
	
	// reference mode
	if( ReferenceMode==1 ) {
		imguCopy(&Iref,I);
		ReferenceMode=2;
	}

	if( ReferenceMode==2 ) {
		ComputeDiff(I,Iref);
	}

	// on va supposer que le usernumber est dans les tags de l'image...
	int userNumber=atoi(imguGetText(I,"USERNUMBER"));
	IV->updateViewImage(userNumber,I);

	imguRecycle(I);
	return(0);
}

#ifdef USE_THREADS

static void background(osgViewer::Viewer *v)
{
#ifdef TEST_UPDATE_POS
double x[4],y[4];

	x[0]=0.5;y[0]=1;
	x[1]=0;y[1]=0;
	x[2]=1;y[2]=0;
	x[3]=1;y[3]=1;
#endif

	printf("viewer in thread go.\n");

	while (!v->done()) {
		pthread_testcancel();
		usleep(20000);
		//sleep(1);
		v->frame();
		//printf("yo\n");
#ifdef TEST_UPDATE_POS
		updateViewPos(IV->_ivT+0,x,y);
		
		/**
		x[0]+=0.005;
		if( x[0]>0.5 ) x[0]=0.0;
		x[2]-=0.005;
		if( x[2]<0.5 ) x[2]=1.0;
		**/
#endif
	}
	printf("viewer in thread done.\n");
	pthread_exit(0);
}

#endif


int main( int argc, char **argv )
{
int camnum=0;
char camName[100];
int i;
char imageName[200];
int nbCam;
int initW=640;
int initH=480;
int fullScreen=0;
int x0=0;
int y0=0;
char camDevice[100];

	ReferenceMode=0;
	Iref=NULL;

	camDevice[0]=0;

	// make all views empty
	nbCam=0;
	for(i=0;i<NB_IV;i++) imgLoaded[i]=NULL;

	IV=new ImageViews(NB_IV); // avec HUD!


	imageName[0]=0;
	camnum=-1; // which camera? -1=no camera
	camName[0]=0;

	pixelShift=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-shift",argv[i])==0 && i+1<argc ) {
			pixelShift=atoi(argv[i+1]);
			i++;continue;
		}else if( strcmp("-c",argv[i])==0 && i+1<argc ) {
			strcpy(camDevice,"gige");
			camnum=atoi(argv[i+1]);
			i++;continue;
		}else if( strcmp("-cam",argv[i])==0 && i+2<argc ) {
			strcpy(camDevice,argv[i+1]);
			camnum=atoi(argv[i+2]);
			camName[0]=0;
			i+=2;continue;
		}else if( strcmp("-camera",argv[i])==0 && i+2<argc ) {
			strcpy(camDevice,argv[i+1]);
			strcpy(camName,argv[i+2]);
			camnum=-1;
			i+=2;continue;
		}else if( strcmp("-geom",argv[i])==0 && i+4<argc ) {
			x0=atoi(argv[i+1]);
			y0=atoi(argv[i+2]);
			initW=atoi(argv[i+3]);
			initH=atoi(argv[i+4]);
			fullScreen=2;
			i+=4;continue;
		}else if( strcmp("-fs",argv[i])==0 ) {
			fullScreen=1;
			continue;
		}else{
			if( nbCam>=NB_IV ) { printf("too many images! max=%d\n",NB_IV);exit(0); }
			int k;
			strcpy(imageName,argv[i]);
			k=imguLoad(imgLoaded+nbCam,imageName,LOAD_AS_IS); // LOAD_16_BITS fails if IMGU8
			if( k ) { printf("unable to load '%s'\n",argv[i]);exit(0); }

			if( nbCam==0 ) {
				initW=imgLoaded[nbCam]->xs;
				initH=imgLoaded[nbCam]->ys;
			}

			if( pixelShift ) {
				int j;
				imgu *I=imgLoaded[nbCam];
				pix_t *p=I->data;
				j=I->xs*I->ys*I->cs;
				while( --j ) *p++<<=pixelShift;
			}

			int num;
			num=IV->addView(imageName,0.0,0.0,1.0,1.0);
			//num=IV->addView(imageName,0.0,0.0,1.0,1.0);
			IV->updateViewImage(num,imgLoaded[nbCam]);
			//IV->updatePosition(num,osg::Matrix::identity());
			//IV->updatePosition(num,osg::Matrix::scale(1,1,1)*osg::Matrix::translate(0.5*nbCam,0.,0.)*osg::Matrix::rotate(M_PI/4*nbCam,0.,0.,1.) );
			nbCam++;
		}
	}

    // construct the viewer.
    osgViewer::Viewer viewer;


    IV->setView(&viewer);

	// ajoute pour nos propres touches locales...
	viewer.addEventHandler(new LocalEventHandler(NULL));


    if( fullScreen==2 ) {
	IV->adjustViewer(x0,y0,initW,initH,false);
    }else if( fullScreen==1 ) {
	IV->adjustViewerFullScreen();
    }else if( fullScreen==0 ) {
	    // adjust initial window size
	    // devrait tester avec la taille de l'ecran...
	    while( initW>1220 || initH>708 ) { initW/=2;initH/=2; }
	    while( initW<=1220/2 && initH<=708/2 ) { initW*=2;initH*=2; }
	printf("trying size %d,%d\n",initW,initH);
	IV->adjustViewer(initW,initH);
    }

    capture cap;
    

if( camnum>=0 || camName[0] ) {
	if( nbCam==NB_IV ) { printf("not enough views for a camera!\n");exit(0); }

	if( imguCaptureInit(camDevice) ) { printf("Le device '%s' ne peut pas etre initialise...\n",camDevice);exit(0); }
	char **cams=imguCaptureList(camDevice);
	if( cams==NULL || cams[0]==NULL || cams[0][0]==0 ) {
		printf("no cam\n");
                imguCaptureUninit(camDevice);
		exit(0);
	}
	for(i=0;cams[i];i++) printf("cam %d is '%s'\n",i,cams[i]);
	if( camnum<0 ) {
		printf("using cam %s\n",camName);
	}else{
		strcpy(camName,cams[camnum]);
	}
		printf("Using camera %s\n",camName);
		if( imguCaptureOpen(camDevice,camName,&cap,NULL,NULL) ) {
			printf("unable to start capture\n");
			imguCaptureUninit(camDevice);
			exit(0);
		}
		// set un parametre
		/*****
		char buf[100];
		imguCaptureSetParam(&cap,NULL,buf,sizeof(buf));
		printf("ALL Set parameters are : '%s'\n",buf);

		imguCaptureGetParam(&cap,NULL,buf,sizeof(buf));
		printf("ALL Get parameters are : '%s'\n",buf);

		imguCaptureGetParam(&cap,"resolution",buf,sizeof(buf));
		printf("Get resolution (before) : '%s'\n",buf);

		//imguCaptureSetParam(&cap,"resolution","320x240",0);

		imguCaptureGetParam(&cap,"resolution",buf,sizeof(buf));
		printf("Get resolution (after): '%s'\n",buf);
		*****/

	/**
		imguCaptureGetParam(&cap,"raw",buf,sizeof(buf));
		printf("Get raw (before) : '%s'\n",buf);
		imguCaptureSetParam(&cap,"raw","1",0);
		imguCaptureGetParam(&cap,"raw",buf,sizeof(buf));
		printf("Get raw (after) : '%s'\n",buf);
	**/
	
	   //sprintf(imageName,"camera[%d] (%s:%s)",camnum,camDevice,cams[camnum]);
	   sprintf(imageName,"camera (%s:%s)",camDevice,camName);

	   //cap.userNumber=IV->addView
	   cap.userNumber=IV->addView(imageName,0,0,1,1);
	   nbCam++;
}

   printf("Total nb cam = %d\n",nbCam);

   viewer.addEventHandler(new osgViewer::StatsHandler);

   osg::Group* root = new osg::Group();

   root->addChild(IV->getScene());

   viewer.setSceneData(root);

/*
	for(i=0;i<NB_SAVE;i++) ISaved[i]=NULL;
	nbSaved=0;
*/

   if( camnum>=0 || camName[0] ) {
	int k;
	k=imguCaptureMany(&cap,callback);
	printf("Many returned %d\n",k);
    }

  
   if (!viewer.isRealized()) { viewer.realize(); }


   //IV->resetAllViews();

#ifdef USE_THREADS
	pthread_t threads;
	pthread_attr_t pthread_custom_attr;

	pthread_attr_init(&pthread_custom_attr);
	pthread_create(&threads, &pthread_custom_attr, (void* (*)(void*))background, &viewer);

	// wait for the thread to finish...
	pthread_join(threads,NULL);

#else
	//	viewer.run();

	//IV->getCurrentIView()->getAnnotations()->add( 200.5, 200.5 );

	for(i=0;!viewer.done();i++) {
		viewer.frame();
		usleep(20000);
/***
		if( i==100 ) {
			printf("Add!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			IV->getCurrentIView()->getAnnotations()->add( 100.5, 100.5 );
		}
***/
	}
#endif

/*
	printf("Saving...\n");
	for(i=0;i<nbSaved;i++) {
		char buf[100];
		sprintf(buf,"out%03d.png",i);
		imguSave(ISaved[i],buf,1,SAVE_16_BITS);
		printf("done img %d\n",i);
	}
*/

   if( camnum>=0 || camName[0] ) {
	// you must wait for all frames to be received
	printf("waiting before closing...\n");
	//sleep(3);
	printf("closing...\n");
	imguCaptureClose(&cap);
	imguCaptureUninit(camDevice);
	printf("done.\n");
   }


#ifdef USE_THREADS
	//viewer.setDone(true);

	pthread_cancel(threads);

	printf("waiting to be done.\n");
	pthread_join(threads,NULL);
	printf("we are done.\n");
#endif

	delete IV;

}

