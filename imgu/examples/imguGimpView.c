
//
// Ce programme capture une camera et envoie l'information comme un stream de commandes
// 'paint' pour informer un gimp de l'information et mettre a jour son image.
//


#include <sys/time.h>

#include <imgu/imgu.h>

//#define USE_PROFILER
//#include <profiler.h>

// pour voir...
//#define VOIR


#include <bmc/udpcast.h>

#define HEADERSIZE  27

#define UDP_PACKET_SIZE 1000
#define UDP_TILE_SIZE(bpp)  ((int)(floor(sqrt((double)((UDP_PACKET_SIZE-HEADERSIZE)/bpp)))))

#define MULTICAST   "226.0.0.1"


typedef struct {
	int x,y,w,h;
	int diff;	// difference moyenne avec l'image gimp
	int age;	// age moyen des pixels
	int rand;
} tile;

/// current gimp image
imgu *Igimp;
imgu *Iage; // age of pixels
tile *T; // contient au moins toutes les tuiles



unsigned char data[UDP_PACKET_SIZE];

udpcast udp;

void sendImage(imgu *IA);
void sendImageSort(imgu *IA,int howMany); // utilise Igimp et Iage

void sendImage(imgu *IA)
{
int i,j;
int width=IA->xs;
int height=IA->ys;
int bpp=IA->cs;

int size=(width*height*bpp)+HEADERSIZE+10; // 10 comme marge
int tsz=UDP_TILE_SIZE(bpp);
int xb,yb;
int iw,ih,wb,hb,xx,yy,c;

	//printf("Tile size is %d\n",tsz);

	// nb de tuiles necessaire pour couvrir
	iw=(width+tsz-1)/tsz;
	ih=(height+tsz-1)/tsz;
	//printf("nb tuiles for (%d,%d) is (%d,%d) = %d\n",width,height,iw,ih,iw*ih);

	// toute les tuiles (i,j)
	for(i=0;i<iw;i++)
	for(j=0;j<ih;j++) {
		xb=i*tsz;
		yb=j*tsz;
		wb=width-i*tsz;if( wb>tsz ) wb=tsz;
		hb=height-j*tsz;if( hb>tsz ) hb=tsz;

		//printf("tuile (%d,%d) : @(%d,%d) w=%d h=%d\n",i,j,xb,yb,wb,hb);

		sprintf((char *)data,"pain %d%5d%5d%5d%5d",bpp,xb,yb,wb,hb);

		for(yy=0;yy<hb;yy++)
		for(xx=0;xx<wb;xx++) {
			for(c=0;c<bpp;c++) {
				data[HEADERSIZE+(yy*wb+xx)*bpp+c]=PIXEL(IA,xb+xx,yb+yy,c)>>8;
			}
		}
		udp_send_data(&udp,data,wb*hb*bpp+HEADERSIZE);
        usleep(200);
	}
}


int cmpTile(tile *A,tile *B)
{
int as=A->diff+A->age;
int bs=B->diff+B->age;
	if( as < bs ) return(1);
	if( as > bs ) return(-1);
	return(0);
}

int cmpTileXY(tile *A,tile *B)
{
/*
int as=A->y*1000+A->x;
int bs=B->y*1000+B->x;
*/
int as=A->rand;
int bs=B->rand;
	if( as < bs ) return(1);
	if( as > bs ) return(-1);
	return(0);
}

// send IA, but sort and send some tiles...

// send IA, but sort and send some tiles...
// utilise Igimp et Iage
void sendImageSort(imgu *IA,int howMany)
{
int i,j;
int width=IA->xs;
int height=IA->ys;
int bpp=IA->cs;

int size=(width*height*bpp)+HEADERSIZE+10; // 10 comme marge
int tsz=UDP_TILE_SIZE(bpp);
int xb,yb;
int iw,ih,wb,hb,xx,yy,c;


	printf("Tile size is %d\n",tsz);

	// nb de tuiles necessaire pour couvrir
	iw=(width+tsz-1)/tsz;
	ih=(height+tsz-1)/tsz;
	printf("nb tuiles for (%d,%d) is (%d,%d) = %d\n",width,height,iw,ih,iw*ih);

	if( Igimp==NULL || Iage==NULL ) {
		imguAllocate(&Igimp,IA->xs,IA->ys,IA->cs);
		imguAllocate(&Iage,IA->xs,IA->ys,1);
		imguClear(Igimp);
		imguClear(Iage);

		T=(tile *)malloc(iw*ih*sizeof(tile));
	}

	// toute les tuiles (i,j)
	int n=0;
	for(i=0;i<iw;i++)
	for(j=0;j<ih;j++) {
		xb=i*tsz;
		yb=j*tsz;
		wb=width-i*tsz;if( wb>tsz ) wb=tsz;
		hb=height-j*tsz;if( hb>tsz ) hb=tsz;

		T[n].x=xb;
		T[n].y=yb;
		T[n].w=wb;
		T[n].h=hb;

		// calcule score (diff et age)
		int diff=0;
		int age=0;
		int d;
		for(yy=0;yy<hb;yy++)
		for(xx=0;xx<wb;xx++) {
			age+=PIXEL(Iage,xb+xx,yb+yy,0);
			for(c=0;c<IA->cs;c++) {
				d=(int)PIXEL(IA,xb+xx,yb+yy,c)-(int)PIXEL(Igimp,xb+xx,yb+yy,c);
				if( d<0 ) d=-d;
				diff+=d;
			}
		}
		T[n].age=(age*100)/(wb*hb);
		T[n].diff=(diff)/(wb*hb);
		T[n].rand=rand();
		n++;
	}

	// increment l'age
	for(i=0;i<Iage->xs*Iage->ys;i++) Iage->data[i]+=1;

	// sort les tuiles
	qsort(T,n,sizeof(tile),(void *)cmpTile);

	qsort(T,howMany,sizeof(tile),(void *)cmpTileXY);

	// send some tiles..........
	// toute les tuiles (i,j)
	for(i=0;i<howMany && i<n;i++) {
		xb=T[i].x;
		yb=T[i].y;
		wb=T[i].w;
		hb=T[i].h;

		//printf("tuile (%d,%d) : @(%d,%d) w=%d h=%d dif=%d age=%d\n",i,j,xb,yb,wb,hb,T[i].diff,T[i].age);

		sprintf((char *)data,"pain %d%5d%5d%5d%5d",bpp,xb,yb,wb,hb);

		// we copy the image into the Igimp at the same time, and reset the age
		for(yy=0;yy<hb;yy++)
		for(xx=0;xx<wb;xx++) {
			for(c=0;c<bpp;c++) {
				int v=PIXEL(IA,xb+xx,yb+yy,c);
				data[HEADERSIZE+(yy*wb+xx)*bpp+c]=v>>8;
				PIXEL(Igimp,xb+xx,yb+yy,c)=v;
			}
			PIXEL(Iage,xb+xx,yb+yy,0)=0;
		}
		udp_send_data(&udp,data,wb*hb*bpp+HEADERSIZE);
        usleep(200);
	}

}


void flip(imgu *IA)
{
	int i,x,y,yy;
	pix_t v;
	pix_t *p,*q;
	for(y=0;y<IA->ys/2;y++) {
		yy=IA->ys-1-y;
		p=IA->data+y*IA->xs*IA->cs;
		q=IA->data+yy*IA->xs*IA->cs;
		for(i=0;i<IA->xs*IA->cs;i++) { v=*p;*p=*q;*q=v;p++;q++; }
	}
}




static int cropHomography(double px[4],double py[4],matrix **homo,matrix **homoinv)
{
  matrix *pts1,*pts2;

  pts1=NULL;
  pts2=NULL;

  matAllocate(&pts1,4,2);
  matAllocate(&pts2,4,2);

  pts1->values[0]=px[0];
  pts1->values[1]=py[0];
  pts1->values[2]=px[1];
  pts1->values[3]=py[1];
  pts1->values[4]=px[2];
  pts1->values[5]=py[2];
  pts1->values[6]=px[3];
  pts1->values[7]=py[3];

  pts2->values[0]=0.0;
  pts2->values[1]=0.0;
  pts2->values[2]=1.0;
  pts2->values[3]=0.0;
  pts2->values[4]=1.0;
  pts2->values[5]=1.0;
  pts2->values[6]=0.0;
  pts2->values[7]=1.0;

  if( homo ) matHomographySolve(pts1,pts2,homo);
  if( homoinv ) matInverse(homoinv,*homo);

matPrint(*homo);
matPrint(*homoinv);


/***
  if( homoinv ) {
	matPrint(*homo);
	matAllocate(homoinv,3,3);
    mat3Inverse((*homo)->values,(*homoinv)->values);
	matPrint(*homoinv);
  }
***/


  matFree(&pts1);
  matFree(&pts2);

  return 0;
}



matrix *homo,*homoinv;


int readCrop(char *cropFile)
{
FILE *F;
char type[100];
double px[4],py[4];
	F=fopen(cropFile,"r");
	if( F==NULL ) return(-1);

	fscanf(F,"%s",type);
	if( strcmp(type,"HOMOGRAPHY")==0 ) {
		printf("--- using homography ---\n");
		fscanf(F," %lf , %lf   %lf , %lf   %lf , %lf   %lf , %lf",px+0,py+0,px+1,py+1,px+2,py+2,px+3,py+3);


		homo=NULL;
		homoinv=NULL;
		cropHomography(px,py,&homo,&homoinv);
		
	}
	fclose(F);

	printf("%f %f\n",px[0],py[0]);
	printf("%f %f\n",px[1],py[1]);
	printf("%f %f\n",px[2],py[2]);
	printf("%f %f\n",px[3],py[3]);

	double *p=homo->values;
	printf("%12.6f %12.6f %12.6f\n",p[0],p[1],p[2]);
	printf("%12.6f %12.6f %12.6f\n",p[3],p[4],p[5]);
	printf("%12.6f %12.6f %12.6f\n",p[6],p[7],p[8]);
	p=homoinv->values;
	printf("%12.6f %12.6f %12.6f\n",p[0],p[1],p[2]);
	printf("%12.6f %12.6f %12.6f\n",p[3],p[4],p[5]);
	printf("%12.6f %12.6f %12.6f\n",p[6],p[7],p[8]);
	return(0);
}

// normalement: uv -> H -> xy , xy -> Hinv -> uv
static void cropHomographyFromTo(double from[2],double to[2],matrix *H)
{
    vector3 pfrom,pto;

    pfrom[0]=from[0];
    pfrom[1]=from[1];
    pfrom[2]=1.0;

	//printf("(%f,%f,%f)->\n",pfrom[0],pfrom[1],pfrom[2]);

	double *vv=H->values;
	//printf("H=(%f,%f,%f,%f,%f,%f,%f,%f,%f)->\n",vv[0],vv[1],vv[2],vv[3],vv[4],vv[5],vv[6],vv[7],vv[8]);

    mat3MultiplyVector(H->values,pfrom,pto);
    vect3Homogenize2D(pto);

	//printf("->(%f,%f,%f)\n",pto[0],pto[1],pto[2]);

    to[0]=pto[0];
    to[1]=pto[1];
}


//
// remap image IA to image IB, using homography
// assume that image IA is a camera image in pixel
// assume that image IB is a resulting image in UV (scaled coords)
//
static void remap(imgu *IA,imgu *IB,matrix *H,int inverseY)
{
double uv[2],xy[2];
int x,y,c;
double val[4];
	imguClear(IB);
	for(y=0;y<IB->ys;y++)
	for(x=0;x<IB->xs;x++) {
		uv[0]=(double)x/(IB->xs-1);
		if( inverseY )	uv[1]=1-(double)y/(IB->ys-1);
		else			uv[1]=(double)y/(IB->ys-1);
		cropHomographyFromTo(uv,xy,H);
//		xy[0]*=(IB->xs-1);
//		xy[1]*=(IB->ys-1);
		//printf("(%4d,%4d) -> (%12.6f,%12.6f)\n",x,y,xy[0],xy[1]);
		int nx=(int)(xy[0]);
		int ny=(int)(xy[1]);
		if( nx<0 || nx>=IA->xs || ny<0 || ny>=IA->ys ) {
			for(c=0;c<IB->cs;c++) PIXEL(IB,x,y,c)=128;
			continue;
		}
		//if( imguInterpolateBilinear(IA,xy[0],xy[1],val) ) continue;
		for(c=0;c<IB->cs && c<IA->cs;c++) PIXEL(IB,x,y,c)=PIXEL(IA,nx,ny,c);
	}
}


int main (int argc, char *argv[])
{
int i,k;
imgu *Iout;
char buf[100];
int tid_cam;
rqueue *Qrecycle,*Qcam,*Qview,*Qfree;
int port;
char cropFile[100];
int tid_viewer;
int exposure;
int gimpwidth,gimpheight;
int camid;
int inverseY;
int nbtuile;



	nbtuile=250;
	port=14999;
	cropFile[0]=0;
	exposure=-1;
	gimpwidth=2048;
	gimpheight=1536;
	camid=0;
	inverseY=1;

	strcpy(cropFile,"/home/roys/lighttwist/scan_plancher/out/crop.txt");

	for(i=1;i<argc;i++) {
		if( strcmp(argv[i],"-camid")==0 && i+1<argc ) {
			camid=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-ynormal")==0 ) {
			inverseY=0;
			continue;
		}
		if( strcmp(argv[i],"-yinverse")==0 ) {
			inverseY=1;
			continue;
		}
		if( strcmp(argv[i],"-p")==0 && i+1<argc ) {
			port=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-crop")==0 && i+1<argc ) {
			strcpy(cropFile,argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-nocrop")==0 ) {
			cropFile[0]=0;
			i++;continue;
		}
		if( strcmp(argv[i],"-e")==0 && i+1<argc ) {
			exposure=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-w")==0 && i+1<argc ) {
			gimpwidth=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-h")==0 && i+1<argc ) {
			gimpheight=atoi(argv[i+1]);
			i++;continue;
		}
		if( strcmp(argv[i],"-nbtuile")==0 && i+1<argc ) {
			nbtuile=atoi(argv[i+1]);
			i++;continue;
		}
	}

	printf("port=%d\n",port);

	// homography
	if( cropFile[0] ) readCrop(cropFile);



	// network

	k=udp_init_sender(&udp,MULTICAST,port,TYPE_MULTICAST);
	//k=udp_init_sender(&udp,"127.0.0.1",54321,TYPE_NORMAL);


	Qrecycle=imguRegisterQueue("recycle");
	Qcam=imguRegisterQueue("cam");
	Qfree=imguRegisterQueue("free");
#ifdef VOIR
	Qview=imguRegisterQueue("view");
#endif

	imgu *IA=NULL;
	for(i=0;i<20;i++) RQueueAddFirst(Qrecycle,(unsigned char *)&IA);
	for(i=0;i<20;i++) RQueueAddFirst(Qfree,(unsigned char *)&IA);


	//tid_cam=imguStartPlugin("v4l2","camera","-in recycle -out cam -fps 10 -width 640 -height 480");
	//tid_cam=imguStartPlugin("v4l2","camera","-in recycle -out cam -device /dev/video1 -fps 30 -width 768 -height 480");
	//tid_cam=imguStartPlugin("v4l2","camera","-in recycle -out cam -device /dev/video0 -fps 10 -width 640 -height 480");
	char cmd[200];
	sprintf(cmd,"-in recycle -out cam -camid %d -fps 0",camid);
	tid_cam=imguStartPlugin("gige","camera",cmd);
	if( tid_cam<0 ) { printf("Unable to start camera\n");exit(0); }


	// un viewer
#ifdef VOIR
	tid_viewer=imguStartPlugin("viewer","sink","-in view -view0 cam0 0 0 0.5 1 -view1 crop 0.5 0 1 1");
#endif
	//tid_viewer=imguStartPlugin("viewer","sink","-in view -view0 cam0 0 0 1 1");


	// we assume that image 1 is the output
	Iout=NULL;


	paramlist *PL=imguGetPluginParameters(tid_cam);

	if( exposure>0 ) paramSetInt(PL,"ExposureValue",exposure);

	paramInvokeCommand(PL,"START");

	imgu *IB=NULL;
	//imguAllocate(&IB,512,512,1);

	//imguAddText(IB,"VIEWNUM","1");


	i=0;
	for(i=0;;i++) {

		// vide la queue d'input, au cas ou SNAPSHOT ne serait pas implante
		while( RQueueRemoveFirst(Qcam,(unsigned char*)&IA)==0 ) imguRecycle(IA);

		printf("frame %d...\n",i);
		paramInvokeCommand(PL,"SNAPSHOT");
		RQueueRemoveLastWaitForever(Qcam,(unsigned char*)&IA);
		RQueueRemoveLastWaitForever(Qfree,(unsigned char*)&IB);

		imguAllocate(&IB,gimpwidth,gimpheight,1);
		//imguCopy(&IB,IA);
		imguSetRecycleQueue(IB,Qfree);

#ifdef VOIR
		imguReplaceAddText(IA,"VIEWNUM","1");
		imguReplaceAddText(IB,"VIEWNUM","0");
#endif

		//imguSetRecycleQueue(IA,Qrecycle);
		//imguReplaceAddText(IA,"VIEWNUM","0");

		//imguScale(&IA,IA,1360.0/IA->xs,1024.0/IA->ys);

		//imguFlip(&IA,IA);
		//imguScale(&IA,IA,2048.0/1360.0,1.0);

		remap(IA,IB,homoinv,inverseY);

#ifdef VOIR
		RQueueAddLast(Qview,(unsigned char *)&IA);
		RQueueAddLast(Qview,(unsigned char *)&IB);
#endif
		//RQueueAddLast(Qview,(unsigned char *)&IB);

		//imguRecycle(IB);

		//sendImage(IA);
#ifndef VOIR
		sendImageSort(IB,nbtuile);
		imguRecycle(IA);
		imguRecycle(IB);
#endif
		// save the image
		//sprintf(buf,"output%03d.png",i);
		//imguSave(IA,buf, 1, SAVE_AS_IS);

	}
}


