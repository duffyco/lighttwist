#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Renderer>
#include <iostream>
#include <sstream>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <sys/time.h>

#include <imgu/imgu.h>

#define USE_PROFILER
#include <profiler.h>

//#include "DistortionRadiale.h"

char *mapp = "/home/roys/svn3d/vision3d/tracking/powell/pixelmap0.png";
char *imp = "/home/roys/svn3d/vision3d/tracking/powell/image_exemple.png";
char *imp_mono = "/home/roys/svn3d/vision3d/tracking/powell/image_exemple_mono.png";

//
// create a distorsion map
//
imgu *makeMap(int w,int h)
{
int x,y,i;
double *dx,*dy; // [w*h] access [y*w+x]
imgu *IA;
double max,scale,offset;
	dx=(double *)malloc(w*h*sizeof(double));
	dy=(double *)malloc(w*h*sizeof(double));

	IA=NULL;
	imguAllocate(&IA,w,h,3);

	double cx=w/2.0;
	double cy=h/2.0;
	double L;
	double vx,vy;

	for(y=0;y<IA->ys;y++)
	for(x=0;x<IA->xs;x++) {
		vx=(x-cx);
		vy=(y-cy);
		L=sqrt(vx*vx+vy*vy);
		L=pow(L,0.9)/L;
		dx[y*w+x]=((x-cx)*L+cx)-x;
		dy[y*w+x]=((y-cy)*L+cy)-y;
	}

	max=0.0;
	for(i=0;i<w*h;i++) {
		if( dx[i]>max ) max=dx[i];
		if( -dx[i]>max ) max=-dx[i];
		if( dy[i]>max ) max=dy[i];
		if( -dy[i]>max ) max=-dy[i];
	}

	scale=(2*max)/65535; // on va de -max a max
	offset= -max;

	printf("max=%f scale=%f offset=%f\n",max,scale,offset);

	char buf[100];
	sprintf(buf,"%f",scale);
	imguReplaceAddText(IA,"scale",buf);
	sprintf(buf,"%f",offset);
	imguReplaceAddText(IA,"offset",buf);

	for(y=0;y<IA->ys;y++)
	for(x=0;x<IA->xs;x++) {
		PIXEL(IA,x,y,0)=(dx[y*w+x]-offset)/scale;
		PIXEL(IA,x,y,1)=(dy[y*w+x]-offset)/scale;
		PIXEL(IA,x,y,2)=0;
	}

	free(dx);
	free(dy);
	return(IA);
}

//
// create an input image
//
imgu *makeInput(int w,int h)
{
imgu *IA;
int x,y,c;
	IA=NULL;
	imguAllocate(&IA,w,h,3);

	for(y=0;y<IA->ys;y++)
	for(x=0;x<IA->xs;x++) {
		c=((x/32)%2) ^ ((y/32)%2);
		if( c ) {
			PIXEL(IA,x,y,0)=PIXEL(IA,x,y,1)=PIXEL(IA,x,y,2)=65535;
		}else{
			PIXEL(IA,x,y,0)=x*65535/(IA->xs-1);
			PIXEL(IA,x,y,1)=y*65535/(IA->ys-1);
			PIXEL(IA,x,y,2)=65535-(x+y)*65535/(IA->xs-1+IA->ys-1);
		}
	}
	imguSave(IA,"in.png",9,SAVE_AS_IS);
	return(IA);
}


// class to handle events with a pick                                                                                                                     

void doBlur(GPU *gpu)
{
	gpu->setupBlur(5);
}

void doDistortion(GPU *gpu)
{
	imgu *Imap=NULL;
	//if( imguLoad(&Imap,mapp,LOAD_16_BITS)) { printf("Unable to load map\n");exit(0); }
	Imap=makeMap(768,480);

	imguSave(Imap,"map.png",9,SAVE_AS_IS);

	double scale=atof(imguGetText(Imap,"scale"));
	double offset=atof(imguGetText(Imap,"offset"));

	gpu->setupDistortion();

	gpu->setInputImage(2,Imap);
	gpu->setUniform(0,(float)(scale*65535));
	gpu->setUniform(1,(float)(offset));
}


void doDerivee(GPU *gpu)
{
	gpu->setupDerivee(1);
}

void doFluxNormal(GPU *gpu,int w,int h)
{
//int x,y;
//double xx,yy;

	gpu->setupFluxNormal();

	// image 0 est l'image courante
	// image 2 est l'image precedente

/****
	imgu *IA=NULL;
	imgu *IB=NULL;

	imguAllocate(&IA,w,h,3);
	imguAllocate(&IB,w,h,3);

	for(y=0;y<IA->ys;y++)
	for(x=0;x<IA->xs;x++) {
		PIXEL(IA,x,y,0)=(int)((sin(x/20.0)+cos(y/15.0)+2)/4.0*65535.0);
		PIXEL(IA,x,y,1)=PIXEL(IA,x,y,0);
		PIXEL(IA,x,y,2)=PIXEL(IA,x,y,0);

		xx=x+0.5;
		yy=y+0.5;
		PIXEL(IB,x,y,0)=(int)((sin(xx/20.0)+cos(yy/15.0)+2)/4.0*65535.0);
		PIXEL(IB,x,y,1)=PIXEL(IB,x,y,0);
		PIXEL(IB,x,y,2)=PIXEL(IB,x,y,0);
	}

	imguSave(IA,"input_now.png",1,SAVE_AS_IS);
	imguSave(IB,"input_prev.png",1,SAVE_AS_IS);

	gpu->setInputImage(0,IA); // image 0 is now
	gpu->setInputImage(2,IB); // image 2 is prev

****/
	// pas de free... ia et ib sont perdus...

}

void project(imgu *I)
{
pix_t *p;
double vx,vy;
int sz=I->xs*I->ys;
double sumx=0.0;
double sumy=0.0;
double maxx,maxy,minx,miny;
int i,n;
	p=I->data;
	n=0;
	maxx=maxy=-1.0;
	minx=miny=1.0;
	for(i=sz;i>0;i--,p+=I->cs) {
		if( p[0]==13107 && p[1]==13107 ) continue;
		if( p[0]==0 ) continue; // vide
		vx=((int)p[0]-32768)/32768.0;
		vy=((int)p[1]-32768)/32768.0;
		sumx+=vx;
		sumy+=vy;
		if( vx<minx ) minx=vx;
		if( vx>maxx ) maxx=vx;
		if( vy<miny ) miny=vy;
		if( vy>maxy ) maxy=vy;
		n++;
	}
	sumx/=n;
	sumy/=n;
	//printf("n=%6d  vx = %7.2f ,  vy = %7.2f,  x: %f .. %f, y: %f .. %f\n",n,sumx,sumy,minx,maxx,miny,maxy);
	printf("n=%6d  vx = %7.2f ,  vy = %7.2f,  x: %f .. %f, y: %f .. %f%c",n,sumx,sumy,minx,maxx,miny,maxy,10);
}

/****
void eclaire(imgu *I,imgu *IE)
{
pix_t *p;
double vx,vy;
int sz=I->xs*I->ys;
double sumx=0.0;
double sumy=0.0;
double maxx,maxy,minx,miny;
double px=0.0;
double py=0.0;
int i,n,x,y;
	p=I->data;
	n=0;
	maxx=maxy=-1.0;
	minx=miny=1.0;
	i=0;
	for(y=0;y<I->ys;y++)
	for(x=0;x<I->xs;x++,i++,p+=I->cs) {
		if( p[0]==13107 && p[1]==13107 ) continue;
		if( p[0]==0 ) continue; // vide
		vx=abs((int)p[0]-32768)/32768.0;
		vy=abs((int)p[1]-32768)/32768.0;
		if( vx>0.2 || vy>0.2 ) {
			sumx+=vx;
			sumy+=vy;
			px+=x*vx;
			py+=y*vy;
			n++;
		}
	}
	if( n>0 ) {
		px/=sumx;
		py/=sumy;
		//printf("n=%6d  vx = %7.2f ,  vy = %7.2f,  x: %f .. %f, y: %f .. %f\n",n,sumx,sumy,minx,maxx,miny,maxy);
	}
	printf("n=%6d  px=%12.6f   py=%12.6f%c",n,px,py,10);

	// elimine l'eclairage
	for(p=IE->data,i=IE->xs*IE->ys;i>0;i--,p++) if( *p>1024 ) *p-=1024;
	vector3 rgb;
	rgb[0]=rgb[1]=rgb[2]=65535;
	imguDrawCross(IE,px/I->xs*IE->xs,py/I->ys*IE->ys,20,rgb);
}
******/


// I est couleur (la vitesse), IE est mono
void eclaire(imgu *I,imgu *IE)
{
pix_t *p,*q;
int v,i;
double vx,vy;
	//printf("(%d,%d) (%d,%d)\n",I->xs,I->ys,IE->xs,IE->ys);
	// suppose que I et IE sont x et y similaires
	for(q=IE->data,p=I->data,i=IE->xs*IE->ys;i>0;i--,q++,p+=I->cs) {
		v=*q-100; // -256
		if( !((p[0]==13107 && p[1]==13107) || p[0]==0) ) {
			vx=abs((int)p[0]-32768)/32768.0;
			vy=abs((int)p[1]-32768)/32768.0;
			if( vx>0.1 || vy>0.1 ) {
				v+=20000;
			}
		}
		if( v<0 ) v=0;
		else if( v>65535 ) v=65535;
		*q=v;
	}
	imguFastBlur5x5(&IE,IE,1);
}




int main (int argc, char *argv[])
{
int i;
imgu *Iout;
GPU* gpu;
char buf[100];
int tid_cam;
int tid_deinter;
int tid_viewer;
rqueue *Qrecycle,*Qcam,*Qicam;
rqueue *Qviewrecycle,*Qview;
int w,h;

	profiler_init();

	Qrecycle=imguRegisterQueue("recycle");
	Qcam=imguRegisterQueue("cam");
	Qicam=imguRegisterQueue("icam");

	Qviewrecycle=imguRegisterQueue("viewrecycle");
	Qview=imguRegisterQueue("view");

	imgu *IA=NULL;
	for(i=0;i<20;i++) RQueueAddFirst(Qrecycle,(unsigned char *)&IA);



	//tid_cam=imguStartPlugin("v4l2","camera","-in recycle -out cam -fps 10 -width 640 -height 480");
	//tid_cam=imguStartPlugin("v4l2","camera","-in recycle -out cam -device /dev/video1 -fps 30 -width 768 -height 480");
	//tid_cam=imguStartPlugin("v4l2","camera","-in recycle -out cam -device /dev/video0 -fps 10 -width 768 -height 480");
	tid_cam=imguStartPlugin("gige","camera","-in recycle -out cam -camid 0");
	//tid_cam=imguStartPlugin("ffmpeg","camera","-in recycle -out cam -file /home/roys/Bureau/pbs_ice.avi -fps 10");
	if( tid_cam<0 ) { printf("Unable to start camera\n");exit(0); }

	paramlist* PL=imguGetPluginParameters(tid_cam);
	paramInvokeCommand(PL,"START");

	//tid_deinter=imguStartPlugin("deinterlace","filter","-in cam -out icam");
	//if( tid_deinter<0 ) { printf("Unable to start camera\n");exit(0); }

	w=659/2;
	h=493/2;

	char cmd[200];
	sprintf(cmd,"-in view -geom 1280 0 800 600 false -homo  0.349750,0.540667,0.754750,0.540000,0.753750,0.118000,0.344000,0.119000   0,1,1,1,1,0,0,0");
	tid_viewer=imguStartPlugin("viewer","sink",cmd);

	paramlist *pl_viewer=imguGetPluginParameters(tid_viewer);

	//osg::setNotifyLevel(osg::INFO);

	//gpu = new GPU(768,480);

	//gpu = new GPU(768,480,30,30,GPU::NORMAL_WINDOW);
	gpu = new GPU(659/2,493/2,30,30,GPU::NORMAL_WINDOW);

	//gpu = new GPU(768,480,30,30,GPU::NO_DECORATION);
	//gpu = new GPU(768,480,30,30,GPU::NO_WINDOW);

	//doBlur(gpu);
	//doDistortion(gpu);
	//doDerivee(gpu);
	doFluxNormal(gpu,659/2,493/2);

/***
	// we assume that image 0 is the input
	imgu *IA=NULL;
	//if( imguLoad(&IA,imp,LOAD_16_BITS)) { printf("Unable to load\n");exit(0); }
	IA=makeInput(768,480);
	gpu->setInputImage(0,IA); // image 0 is the input
***/

	// we assume that image 1 is the output
	Iout=NULL;
	gpu->setOutputImage(1,&Iout); // image 1 is result

	// what do we want to see?
	gpu->activateViewing(1); // image 1 is the result

	imgu* Iprev=NULL;
	imgu *IZ=NULL;



	int k;
	double s;

	i=0;
	for(i=0;;i++) {
        if( paramGetInt(pl_viewer,"done",&k)==0 && k ) {
            printf("DONE!\n");break;
        }

		//printf("frame %d...\n",i);
		RQueueRemoveFirstWaitForever(Qcam,(unsigned char*)&IA);


		if( Iprev ) {
			gpu->setInputImage(0,IA); // image 0 is now
			gpu->setInputImage(2,Iprev); // image 2 is prev

			profiler_start("process");
			k=gpu->process();
			profiler_stop("process");
			if( k ) break;

			//profiler_start("project");
			//project(Iout);
			//profiler_stop("project");

			int avail;
			if( IZ==NULL ) {
				imguAllocate(&IZ,Iout->xs,Iout->ys,1);
				imguClear(IZ);
				imguSetRecycleQueue(IZ,Qviewrecycle);
				imguReplaceAddText(IZ,"VIEWNUM","0");
				avail=0;
			}else{
				// va chercher une image libre (toujours la meme, en fait)
				avail=RQueueRemoveFirst(Qviewrecycle,(unsigned char*)&IZ);
			}


			if( IZ!=NULL /*avail==0*/ ) {
				profiler_start("eclaire");
				eclaire(Iout,IZ);
				profiler_stop("eclaire");
				RQueueAddLast(Qview,(unsigned char *)&IZ);
			}else{
				usleep(1000);
			}

/**
			if( i==10 ) {
				sprintf(buf,"input%03d.png",i);
				imguSave(IA,buf, 1, SAVE_16_BITS);

				sprintf(buf,"output%03d.png",i);
				imguSave(Iout,buf, 1, SAVE_16_BITS);
			}
**/


		}
		if( Iprev ) imguRecycle(Iprev);
		Iprev=IA;
		IA=NULL;
	}

	profiler_dump();

	printf("fini\n");
	delete gpu;
}


