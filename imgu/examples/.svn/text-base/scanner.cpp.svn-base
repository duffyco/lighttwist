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
#include <osg/Transform>
#include <osg/Group>
#include <osg/TexMat>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osgText/Text>
#include <osgDB/Registry>
#include <osg/Camera>
#include <osg/io_utils>
#include <osg/GraphicsContext>
#include <osgGA/TrackballManipulator>
#include <osg/BoundingSphere>


#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>



#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>


#include <imgu.h>

#include <pthread.h>

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


// images pour la capture
	// test de capture...
#define NBI	20
imgu *I[NBI];

// on a ici une switch pour voir l'image ou le scan
osg::Switch* swi;




/*
#define NB_SAVE	200
imgu *ISaved[NB_SAVE];
int nbSaved;
*/

// une image de reference...
int ReferenceMode=0; // 0=pas de reference, 1=please get next image as reference, 2=use reference
double fade; 
imgu *Iref;

#define STEP 0.1

/////////////////////////////
//
// pour la switch
//

void toggleVisible()
{
int which=swi->getValue(0);
   swi->setValue(0,1-which); // geode
   swi->setValue(1,which); // image
}

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
				if( ReferenceMode!=2 ) fade=0.0;
				ReferenceMode=1; // get net image!
			}
			if( ea.getKey()=='t' ) {
				if( ReferenceMode==2 ) { fade=1.0; ReferenceMode=0; }
				else if( ReferenceMode==0 && Iref!=NULL ) ReferenceMode=2;
			}
			if( ea.getKey()=='v' ) {
				toggleVisible();
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

int ComputeDiff(imgu *I,imgu *Ir,double fade)
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
		//if( i==200*I->xs+300 ) printf("a=%6d b=%6d R=%12.6f L=%12.6f\n",a,b,ratio,L);
		v=(L-LMIN)/(LMAX-LMIN)*65535;
		if( v<0 ) v=0;
		if( v>65535 ) v=65535;
		//I->data[i]=(b-a)/2+32768;
		if( fade<1.0 ) I->data[i]=I->data[i]*(1-fade)+fade*v;
		else I->data[i]=v;
	}
}


void ComputeChange(imgu *I,imgu *Ir,double *errdif,double *errMoy)
{
int x,y,c;
int err,moy,n,a,b;
	err=0;
	moy=0;
	n=0;
	for(y=0;y<I->ys;y+=5)
	for(x=0;x<I->xs;x+=5)
	for(c=0;c<I->cs;c++) {
		a=PIXEL(I,x,y,c);
		b=PIXEL(Ir,x,y,c);
		moy+=a;
		if( a<=b ) err+=(b-a); else err+=(a-b);
		n++;
	}
	*errdif=(double)err/n;
	*errMoy=(double)moy/n;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

// on recoit une image ici! (pour l'instant toujours la meme
int callback(capture *cap,imgu **I,int i,int err)
{
        if( err ) {
		printf("Got image %d (err=%d)!\n",i,err);
		return(1);
	}

	//printf("recu image %d\n",i);
	//if( strcmp(cap->type,"V4L")==0 ) { // videodev.h NOT INCLUDED ANYMORE!!!
	//	printf("pixelformat = %d %d\n",cap->pixelFormat,V4L2_PIX_FMT_BGR32);
	//}

	if( pixelShift ) {
		// on sait que c'est du 12bit... donc on doit faire <<4
		int j;
		unsigned short *p=I[i]->data;
		j=I[i]->xs*I[i]->ys*I[i]->cs;
		while( --j ) *p++<<=pixelShift;
		//while( --j ) *p++<<=8; // des bytes en short
	}

	
/*
	if( nbSaved<NB_SAVE ) {
		printf("copy image %d to %d\n",i,nbSaved);
		imguCopy(&ISaved[nbSaved],I[i]);
		nbSaved++;
	}
*/

/***
	double ed,em;
	if( Iref ) {
		ComputeChange(I[i],Iref,&ed,&em);
		printf("errdif =%12.6f  errmoy=%12.6f\n",ed,em);
	}
***/

	// reference mode
	if( ReferenceMode==1 ) {
		imguCopy(&Iref,I[i]);
		ReferenceMode=2;
	}

	if( ReferenceMode==2 ) {
		ComputeDiff(I[i],Iref,fade);
		fade+=STEP; // fadein
	}else if( fade>0.0 && ReferenceMode==0 ) {
		ComputeDiff(I[i],Iref,fade);
		fade-=STEP; // fadein
	}

	IV->updateViewImage(cap->userNumber,I[i]);

	return(1); // remettre dans la file
}


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




osg::Geode* createShapes()
{
	osg::Geode* geode = new osg::Geode();
	osg::StateSet* stateset = new osg::StateSet();
	osg::Image* image = osgDB::readImageFile( "Images/lz.rgb" );

	if( image ) {
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setImage(image);
		stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
	}

	geode->setStateSet( stateset );

	osg::TessellationHints* hints = new osg::TessellationHints;
	hints->setDetailRatio(0.5f);

	//geode->addDrawable(new osg::ShapeDrawable(new osg::Cone(osg::Vec3(0.0f,0.0f,-5.0f),1.0,2.0),hints));

	osg::HeightField* grid = new osg::HeightField;
	grid->allocate(100,100);
	grid->setXInterval(1.0);
	grid->setYInterval(1.0);

	//grid->setOrigin(osg::Vec3(-1,-2,0));

	for(unsigned int r=0;r<100;++r) 
        for(unsigned int c=0;c<100;++c) {
		grid->setHeight(c,r,drand48()*(r+c)/30);
	}

	//grid->setRotation(osg::Quat(M_PI/2.0,osg::Vec3(0.0,0.0,1.0)));

 //osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));

	geode->addDrawable(new osg::ShapeDrawable(grid));

	const osg::BoundingSphere& bs = geode->getBound();

	printf("center = %f,%f,%f\n",bs.center().x(),bs.center().y(),bs.center().z());
	printf("radius = %f\n",bs.radius());

	return(geode);
}




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

	IV=new ImageViews(NB_IV,1); // avec HUD!


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
			k=imguLoad(imgLoaded+nbCam,imageName,MAKE_16_BITS); // LOAD_AS_IS | MAKE_16_BITS
			if( k ) { printf("unable to load '%s'\n",argv[i]);exit(0); }

			if( nbCam==0 ) {
				initW=imgLoaded[nbCam]->xs;
				initH=imgLoaded[nbCam]->ys;
			}

			if( pixelShift ) {
				int j;
				imgu *I=imgLoaded[nbCam];
				unsigned short *p=I->data;
				j=I->xs*I->ys*I->cs;
				while( --j ) *p++<<=pixelShift;
			}

			int num;
			num=IV->addView(imageName,0.0,0.0,1.0,1.0);
			IV->updateViewImage(num,imgLoaded[nbCam]);
			//IV->updatePosition(num,osg::Matrix::identity());
			//IV->updatePosition(num,osg::Matrix::scale(1,1,1)*osg::Matrix::translate(0.5*nbCam,0.,0.)*osg::Matrix::rotate(M_PI/4*nbCam,0.,0.,1.) );
			nbCam++;
		}
	}

    // construct the viewer.
    osgViewer::Viewer viewer;


    IV->setViewer(&viewer);

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
    

	for(i=0;i<NBI;i++) I[i]=NULL;

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
		if( imguCaptureOpen(camDevice,camName,&cap) ) {
			printf("unable to start capture\n");
			imguCaptureUninit(camDevice);
			exit(0);
		}
		// set un parametre
		char buf[100];
		/*****
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

   swi = new osg::Switch();

   osg::Geode* g=createShapes();

            osg::Camera* hcamera = new osg::Camera;
            hcamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	    hcamera->setProjectionMatrixAsPerspective(30.0f,1.0,0.1,1000.0);
            //camera->setViewMatrix(osg::Matrix::identity());
	    hcamera->setViewMatrixAsLookAt(osg::Vec3(250,50,200),osg::Vec3(50,50,0),osg::Vec3(0,1,0));
            //camera->setClearMask(GL_DEPTH_BUFFER_BIT);
            hcamera->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::ON);

            hcamera->addChild(g);
            swi->addChild(hcamera);



   swi->addChild(IV->getScene());

   swi->setValue(0,0); // geode
   swi->setValue(1,1); // image

   root->addChild(swi);



   viewer.setSceneData(root);

/*
	for(i=0;i<NB_SAVE;i++) ISaved[i]=NULL;
	nbSaved=0;
*/

   if( camnum>=0 || camName[0] ) {
	int k;
	k=imguCaptureMany(&cap,I,NBI,callback);
	printf("Many returned %d\n",k);
    }

  
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


	viewer.setDone(true);

	//pthread_cancel(threads);

	printf("waiting to be done.\n");


	pthread_join(threads,NULL);
   
	printf("we are done.\n");

	delete IV;

}

