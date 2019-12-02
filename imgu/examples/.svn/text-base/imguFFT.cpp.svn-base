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

/**
 * @file
 *
 * This is a simple non-localized motion tracker
 *
 */

// affiche le best
#define BEST

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <imgu.h>
#include <math.h>

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


#include <ImageViews.h>


#define THEREALTHING

#define CPIXEL(I,x,y,c)  ((complex_t *)((I)->complex+INDEX(I,x,y,c)))

// capture
#define CAM_DEVICE	"V4L"
#define CAM_NAME	"/dev/video0"


// pour masquer les bords d'image
#define FENETRE

//#define AFFICHE

#define VMAX_SIZE	41
#define VMAX_STEP	1

#define REF_MAX_VOTE	(11380798697028472.000000)

double globalmax=0;

/////////////////// les images

// On conserve ici les 8 images
// 0,1,2,3,4,5,6,7 : on insere a la fin (7) et on perd le 0
#define TEMPS	4
imgu *Itab[TEMPS];

imgu *Ifft;
imgu *Ivitesse;

int meterNum; // affichage de l'image


////////////////////////// visualisation

ImageViews* IV=NULL;


////////////////////////// calculs

void create_imgage_sequence(imgu **pIA,int xs,int ys,int cs)
{
imgu *IA;
int x,y,c;
double xn,yn,cn;
int v;
	imguAllocate(pIA,xs,ys,cs);
	IA=*pIA;
	for(y=0;y<IA->ys;y++)
	for(x=0;x<IA->xs;x++)
	for(c=0;c<IA->cs;c++) {
		xn=(double)x/(IA->xs)*2*M_PI;
		yn=(double)y/(IA->ys)*2*M_PI;
		cn=(double)c/(IA->cs)*2*M_PI;
//		v = (int) (sin((xn+yn)*2+(cn))*32768+32768);
		v = (int) (cos(xn*8 + yn*16 + cn*1)*32768+32768);
		//v = (int) (cos(yn*4)*32768+32768);
		//v = (int) (cos(cn*4)*32768+32768);

//		v = (int) (cos(xn*4 - yn*2 + cn*1)*32768+32768);
//		v += (int) (cos(xn*1 + yn*1 + cn*3)*32768+32768);
//		v /= 2;

		if( v<0 ) v=0;
		else if( v>65535 ) v=65535;
		PIXEL(IA,x,y,c) = v;
	}

#ifdef SKIP
#ifdef FENETRE
	/// fenetrage!
	{
	imgu *Iw=NULL;
	imguGaussianWindow(&Iw,xs); //imguRaisedCosineWindow(&Iw,xs);
	imguWindow(pIA,*pIA,Iw);
	imguGaussianWindow(&Iw,ys); //imguRaisedCosineWindow(&Iw,ys);
	Iw->ys=Iw->xs;Iw->xs=1; // rend la fenetre en y
	imguWindow(pIA,*pIA,Iw);
	imguGaussianWindow(&Iw,cs); //imguRaisedCosineWindow(&Iw,ys);
	Iw->cs=Iw->xs;Iw->xs=1; // rend la fenetre en c
	imguWindow(pIA,*pIA,Iw);
	imguFree(&Iw);
	}
#endif
#endif
}





void fft_image_sequence(imgu **pIA,imgu **pIV)
{
int x,y,c;
imgu *IA,*IV;
complex_t *q;

/****
	if( imguConvertToComplex(pIA,*pIA,0) ) { printf("complex???\n");exit(0); }

	imguClearComplex(*pIA,1); // clear imaginary
****/

#ifdef FENETRE
	/// fenetrage!
	{
	imgu *Iw=NULL;
	// on a deja applique a l'entree
/***
	imguGaussianWindow(&Iw,(*pIA)->xs); //imguRaisedCosineWindow(&Iw,xs);
	imguWindow(pIA,*pIA,Iw);
	imguGaussianWindow(&Iw,(*pIA)->ys); //imguRaisedCosineWindow(&Iw,ys);
	Iw->ys=Iw->xs;Iw->xs=1; // rend la fenetre en y
	imguWindow(pIA,*pIA,Iw);
***/
	imguGaussianWindow(&Iw,(*pIA)->cs); //imguRaisedCosineWindow(&Iw,ys);
	Iw->cs=Iw->xs;Iw->xs=1; // rend la fenetre en c
	imguWindow(pIA,*pIA,Iw);
	}
#endif


	//
	// imguGaussianWindow(&xwindow,I->xs);
	//       imguGaussianWindow(&ywindow,I->ys);
	//


	imguFFTForward(pIA,*pIA);


	double n;
	IA=*pIA;
#ifdef AFFICHE
	for(c=0;c<IA->cs;c++) {
		printf("--- c=%d ---\n",c);
		for(y=0;y<IA->ys;y++) {
			for(x=0;x<IA->xs;x++) {
				q=CPIXEL(IA,x,y,c);
				n=sqrt(q[0]*q[0]+q[1]*q[1]);
				if( n>1.0 ) 
				printf("(%3d,%3d,%3d) [%15.2f + %15.2f i] %15.2f\n",x,y,c,q[0],q[1],n);
			}
			//printf("\n");
		}
	}
#endif

	double sx,sy,sc;
	double px,py,pc,pxy;
	double max=0.0;
#ifdef BEST
	// trouve les plus grande frequences
	int bestx,besty,bestc;
	double bestphase;
	for(c=0;c<IA->cs;c++) {
		for(y=0;y<IA->ys;y++) {
			for(x=0;x<IA->xs;x++) {
				q=CPIXEL(IA,x,y,c);
				n=sqrt(q[0]*q[0]+q[1]*q[1]);
				// si on fenetre, alors on ne doit pas considerer les 0 en x,y,c
				if( n>max && (x+y)>0 && c>0) { bestx=x;besty=y;bestc=c;max=n;
					bestphase=atan2(-q[1],-q[0])+M_PI;
				}
			}
		}
	}
	printf("BEST is (%3d,%3d,%3d) = %f\n",bestx,besty,bestc,max);
		/***
		q=CPIXEL(IA,IA->xs-bestx,besty,bestc); n=sqrt(q[0]*q[0]+q[1]*q[1]);
		printf("        (%3d,%3d,%3d) = %f\n",IA->xs-bestx,besty,bestc,n);
		q=CPIXEL(IA,bestx,IA->ys-besty,bestc); n=sqrt(q[0]*q[0]+q[1]*q[1]);
		printf("        (%3d,%3d,%3d) = %f\n",bestx,IA->ys-besty,bestc,n);
		q=CPIXEL(IA,bestx,besty,IA->cs-bestc); n=sqrt(q[0]*q[0]+q[1]*q[1]);
		printf("        (%3d,%3d,%3d) = %f\n",bestx,besty,IA->cs-bestc,n);
		***/
	sx=sy=sc=1.0;
	if( bestx>IA->xs/2 ) { bestx=IA->xs-bestx;sx=-1; }
	if( besty>IA->ys/2 ) { besty=IA->ys-besty;sy=-1; }
	if( bestc>IA->cs/2 ) { bestc=IA->cs-bestc;sc=-1; }
	printf("BEST is (%3d,%3d,%3d) = %f phase=%f\n",bestx,besty,bestc,max,bestphase*180/M_PI);
	printf("FREQ is (%12.6f, %12.6f %12.6f)\n",1.0/bestx,1.0/besty,1.0/bestc);
	double dir;
	px=(double)(IA->xs)/bestx*sx;
	py=(double)(IA->ys)/besty*sy;
	pc=(double)(IA->cs)/bestc*sc;
	if( bestc==0 ) { px=py=0.0;pc=1.0; } // cas ou il n'y a pas de mouvement
	if( bestx==0 ) { pxy=py; dir=M_PI/2.0; }
	else if( besty==0 ) { pxy=px;dir=0.0; }
	else {
		pxy=sqrt(px*px+py*py);
		dir=atan2(-py,-px)+M_PI; // rotate -180, then add 180
	}
	printf("periode spatiale X est %f pixels\n",px);
	printf("periode spatiale Y est %f pixels\n",py);
	printf("periode spatiale XY est %f pixels\n",pxy);
	printf("periode temporelle C est %f frame\n",pc);
	printf("La vitesse est donc de %f pixel/frame dans la direction %d deg\n",pxy/pc,(int)(dir*180/M_PI));
#endif


	///////////////////////////////////
	///////////////////////////////////
	///////////////////////////////////

	double vMax;
	if( IA->xs > IA->ys ) vMax=IA->xs-1; else vMax=IA->ys-1;


	imguAllocate(pIV,VMAX_SIZE,VMAX_SIZE,3);
	IV=*pIV;
	// mapping :  -vMax .. vMax -> 0..xs-1
	// i = (v/vMax+1)/2*(xs-1)

	vMax=(IV->xs-1)/2*VMAX_STEP;
	printf("VMAX is %f pix/frame\n",vMax);

	imguClear(IV);

	double *vote=(double *)malloc(IV->xs*IV->ys*sizeof(double));
	int i;
	for(i=0;i<IV->xs*IV->ys;i++) vote[i]=0.0;

	// vote!!!
	int outside=0;
	for(c=1;c<IA->cs;c++) {
		for(y=0;y<IA->ys;y++) {
			for(x=0;x<IA->xs;x++) {
				if( x==0 && y==0 ) continue; // seul cas refuse
				double vx,vy;
				int ix,iy,xx,yy,cc;
				q=CPIXEL(IA,x,y,c);
				//n=sqrt(q[0]*q[0]+q[1]*q[1]);
				n=q[0]*q[0]+q[1]*q[1];
				// direction
				xx=x;yy=y;cc=c;
				sx=sy=sc=1.0;
				if( x>IA->xs/2 ) { xx=IA->xs-xx;sx=-1; }
				if( y>IA->ys/2 ) { yy=IA->ys-yy;sy=-1; }
				if( c>IA->cs/2 ) { cc=IA->cs-cc;sc=-1; }
				// si on fenetre, alors on ne doit pas considerer les 0 en x,y,c
				px=(double)(IA->xs)/xx*sx;
				py=(double)(IA->ys)/yy*sy;
				pc=(double)(IA->cs)/cc*sc;
				if( x==0 ) px=0;
				if( y==0 ) py=0;
				vx=-px/pc;
				vy=-py/pc;
				/// test une vitesse minimum
				//if( vx>-3 && vx<3 && vy>-3 && vy<3 ) continue;

				ix=(vx/vMax+1)/2*(IV->xs-1)+0.5;
				iy=(vy/vMax+1)/2*(IV->ys-1)+0.5;
				//printf("(%d,%d,%d) vx=%f vy=%f ix=%d iy=%d n=%f\n",x,y,c,vx,vy,ix,iy,n);
				if( ix<0 || iy<0 || ix>=IV->xs || iy>=IV->ys ) { outside++;continue; }
				//PIXEL(IV,ix,iy,0)+=(int)(sqrt(n));
				vote[iy*IV->xs+ix]+=n;
			}
		}
	}

	printf("OUTSIDE = %d votes, %f%% du total\n",outside,outside*100.0/(IA->xs*IA->ys*IA->cs));

	max=0;
	for(i=0;i<IV->xs*IV->ys;i++) if( vote[i]>max ) max=vote[i];
	if( max>globalmax ) globalmax=max;
	printf("maximum vote = %f  %f\n",max,globalmax);
	//max/=10; // juste pour voir mieux
	//max=REF_MAX_VOTE;
	double frac,f;
	int val;
	for(i=0;i<IV->xs*IV->ys;i++) {
		frac=vote[i]/globalmax;
		val=(vote[i]<max)?(vote[i]/max*65535):65535;
		if( frac<0.5 ) f=0; else f=(frac-0.5)/(1-0.5);
		IV->data[i*IV->cs+0]=val*f; // .66 .. 1
		if( frac<0.5 ) f=(frac)*2; else f=(0.5-frac)*2+1;
		IV->data[i*IV->cs+1]=val*f; // .33 .. .66
		if( frac>0.5 ) f=0; else f=1-frac*2;
		IV->data[i*IV->cs+2]=val*f; // 0  .. .33
	}
	//imguSave(IV,"out_vitesses.png",1,SAVE_16_BITS);
	free(vote);
}


void save_image_sequence(imgu *IA,char *name)
{
int x,y,c;
imgu *IB=NULL;
char buf[100];
	imguAllocate(&IB,IA->xs,IA->ys,1);
	for(c=0;c<IA->cs;c++) {
		sprintf(buf,name,c);
		for(y=0;y<IA->ys;y++) for(x=0;x<IA->xs;x++) PIXEL(IB,x,y,0)=PIXEL(IA,x,y,c);
		imguSave(IB,buf,1,SAVE_16_BITS);
	}
	imguFree(&IB);
}



void save_fft_sequence(imgu *IA,char *name)
{
int x,y,c;
imgu *IB=NULL;
imgu *IZ;
char buf[100];

	IZ=NULL;
	imguConvertFromComplexFlow(&IZ,IA);

	printf("z is %x %d %d\n",IZ->xs,IZ->ys,IZ->cs);

	imguAllocate(&IB,IZ->xs,IZ->ys,1);
	for(c=0;c<IZ->cs;c++) {
		sprintf(buf,name,c);
		for(y=0;y<IZ->ys;y++) for(x=0;x<IZ->xs;x++) PIXEL(IB,x,y,0)=PIXEL(IZ,x,y,c);
		imguSave(IB,buf,1,SAVE_16_BITS);
	}
	imguFree(&IB);
}


#define NBI	3
imgu *Icap[NBI];

/// utiliset Itab[0..7] pour faire la FFT et calculer les vitesses
void process(void)
{
int sz,i;
	if( Itab[0]==NULL ) return; // on attend que la queue soi pleine
	imguAllocate(&Ivitesse,Itab[0]->xs,Itab[0]->ys,3 /*Itab[0]->cs*/ );
#ifdef SLOWMOTION
unsigned short *q,*r;
	imguClear(Ivitesse);
	for(i=0;i<TEMPS;i++) {
		if( Itab[i]==NULL ) continue;
		sz=Itab[i]->xs*Itab[i]->ys*Itab[i]->cs; // cs devrait etre = 1
		q=Itab[i]->data;
		r=Ivitesse->data;
		while( sz ) { *r++ += (*q++)/TEMPS; sz--;*r++=0;*r++=0; }
	}
#endif
#ifdef SIMPLE_COPIE
	//imguCopy(&Ivitesse,Itab[7]); // ne marche plus
#endif


	// the real thing!!!
	int xs,ys,cs;
	xs=Itab[0]->xs;
	ys=Itab[0]->ys;
	cs=TEMPS;

	imguAllocate(&Ifft,xs,ys,cs);
	imguAllocateComplex(&Ifft,xs,ys,cs);
#ifdef THEREALTHING
	//imguClearComplex(Ifft,1); // clear imaginary

	// copy les 8 images directement dans le buffer complexe
	
	double *q;
	int x,y,c;
	for(c=0;c<cs;c++) {
		for(y=0;y<ys;y++) {
			for(x=0;x<xs;x++) {
				q=CPIXEL(Ifft,x,y,c);
				q[0]=PIXEL(Itab[c],x,y,0);
				q[1]=0.0; // imaginary
			}
		}
	}
#else
	create_imgage_sequence(&Ifft,xs,ys,cs);
	imguConvertToComplex(&Ifft,Ifft,0);
	imguClearComplex(Ifft,1); // clear imaginary
#endif
	fft_image_sequence(&Ifft,&Ivitesse);

	IV->updateViewImage(meterNum,Ivitesse);
}


// on recoit une image ici! (pour l'instant toujours la meme
int callback(capture *cap,imgu **I,int i,int err)
{
	if( err ) {
		printf("Got image %d (err=%d)!\n",i,err);
		return(1);
	}
	//printf("recu image %d\n",i);

	// flip l'image
	int j;
	int x;
	unsigned short *p=I[i]->data;
	unsigned short *q=I[i]->data+((I[i]->ys-1)*I[i]->xs*I[i]->cs);
	unsigned short z;
	for(j=0;j<I[i]->ys/2;j++) {
		for(x=0;x<I[i]->xs*I[i]->cs;x++) {
			z=*p;
			*p++ = *q *256;
			*q++ = z *256;
		}
		q-=2*I[i]->xs*I[i]->cs;
	}

/***
	int j;
	unsigned short *p=I[i]->data;
	j=I[i]->xs*I[i]->ys*I[i]->cs;
	while( --j ) *p++<<=8;
***/

	//IV->updateViewImage(cap->userNumber,I[i]);

	// copy l'image dans la queue... 1->0, ... 7->6, 0->7
	imgu *Iz=Itab[0];
	for(j=0;j<TEMPS-1;j++) Itab[j]=Itab[j+1];
	Itab[TEMPS-1]=Iz;

	imguConvertToGray(Itab+TEMPS-1,I[i]);
	imguScale(Itab+TEMPS-1,Itab[TEMPS-1],128.0/Itab[TEMPS-1]->xs,128.5/Itab[TEMPS-1]->ys);
	printf("new size is %d,%d\n",Itab[TEMPS-1]->xs,Itab[TEMPS-1]->ys);

#ifdef FENETRE
	imgu *Iw=NULL;
	imguGaussianWindow(&Iw,Itab[TEMPS-1]->xs); //imguRaisedCosineWindow(&Iw,xs);
	imguWindow(Itab+TEMPS-1,Itab[TEMPS-1],Iw);
	imguGaussianWindow(&Iw,Itab[TEMPS-1]->ys); //imguRaisedCosineWindow(&Iw,ys);
	Iw->ys=Iw->xs;Iw->xs=1; // rend la fenetre en y
	imguWindow(Itab+TEMPS-1,Itab[TEMPS-1],Iw);
	imguFree(&Iw);
#endif

	IV->updateViewImage(cap->userNumber,Itab[TEMPS-1]);

	process();

        return(1); // remettre dans la file
}





int main(int argc,char *argv[])
{
imgu *IA;
imgu *It;
capture cap; // pour la capture d'images
int i,k;

	Ifft=NULL;
	Ivitesse=NULL;
	for(i=0;i<TEMPS;i++) Itab[i]=NULL;
	
	//create_imgage_sequence(&IA,160,120,8);

	//// visualisation
	IV=new ImageViews(11,1); // avec HUD!

	// construct the viewer.
	osgViewer::Viewer viewer;
	IV->setViewer(&viewer);

	IV->adjustViewer(160+120,120);

	/// init capture
	for(i=0;i<NBI;i++) Icap[i]=NULL;
	if( imguCaptureInit(CAM_DEVICE) ) { printf("Le device '%s' ne peut pas etre initialise...\n",CAM_DEVICE);exit(0); }
	if( imguCaptureOpen(CAM_DEVICE,CAM_NAME,&cap) ) {
		printf("unable to start capture\n");
		imguCaptureUninit(CAM_DEVICE);
		exit(0);
	}
	char buf[100];
	imguCaptureGetParam(&cap,"resolution",buf,sizeof(buf));
	printf("Get resolution (before) : '%s'\n",buf);
	imguCaptureSetParam(&cap,"resolution","160x120",0);
	imguCaptureGetParam(&cap,"resolution",buf,sizeof(buf));
	printf("Get resolution (after) : '%s'\n",buf);

	cap.userNumber=IV->addView("webcam",0,0,0.5,1);
	meterNum=IV->addView("vitesse",0.5,0.0,1.0,1.0);
	//IV->updateViewImage(meterNum,image);

	viewer.addEventHandler(new osgViewer::StatsHandler);
	osg::Group* root = new osg::Group();
	root->addChild(IV->getScene());
	viewer.setSceneData(root);

	k=imguCaptureMany(&cap,Icap,NBI,(int (*)())callback);

	if (!viewer.isRealized()) { viewer.realize(); }

	while( !viewer.done() ) {
                usleep(20000);
                viewer.frame();
	}

	delete IV;

	exit(0);


	
#ifdef ORIGINAL


	It=NULL;
	//save_image_sequence(IA,"out%03d.png");
	fft_image_sequence(&IA,&It);
	//save_fft_sequence(IA,"fft%03d.png");
	process_fft_sequence(&IA);

#endif

#ifdef SKIP
	{
	int i;
	double t,c,s,a;
	for(i=0;i<360;i++) {
		t=(double)i*M_PI/180.0;
		c=cos(t);
		s=sin(t);
		//a=atan2(s,c);
		a=atan2(-s,-c)+M_PI; // rotate -180, then add 180
		printf("%3d : %12.6f : (%12.6f,%12.6f) : %12.6f\n",i,t,c,s,a);
	}
	}
#endif





}


