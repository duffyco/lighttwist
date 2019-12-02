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

//
// test le tiling video
//
// concept: on donne une queue de recyclage avec des images
// pre-allouee d'une taille plus grande que le video.
// le decodage va se faire dans l'image trop grande.
//
// pour specifier un offset pour le video (ou 3 offsets pour yuv)
// on utilise l'option -offsetkey toto123
// et ensuite dans une image de la queue de recyclage on ajoute
// les offset dans une paire de cle toto123:  +34+50
// ou +34+50+100+234+84+34  si yuv
// Et voila!
// on peut donc faire un pipeline de decodage tres efficace.
// 
//
//

#include <imgu.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "tileinfo.h"

// global stuff

#define LUT

#define LOOKAHEAD 9

#define MAXNBTILE  50

int nbTile;

// eventuellement tout est ici
tile_info tiles[MAXNBTILE];

int tid_view;   // viewer
//int tid_cam[MAXNBTILE];    // [nbtile] ffmpeg player

paramlist *pl_view;
//paramlist *pl_camera[MAXNBTILE]; // [nbtile]

vector4 lutbb; // LUT bounding box for this projector.
int lutbbpixels[4];

#ifdef LUT

#ifdef IMGU8
imgu *IlutH=NULL;
imgu *IlutL=NULL;
#else
imgu *Ilut=NULL;
#endif


rqueue *Qmini;
#endif


// viewer access
rqueue *Qdisplay;


// definir pour jouer un seul movie complet
// sinon, on va jouer deux moities (left/right)
//#define SINGLE
//#define DUAL
//#define QUAD
#define OCTO4

/**
#ifdef SINGLE
   #define NBTILE  1
char* videonames[NBTILE]={"/home/roys/Videos/pbsice/ice+0+0x1280x720.avi"};
char* offsetnames[NBTILE]={"+0+0"};
	#define BASEX 1280
	#define BASEY 720
#endif
#ifdef DUAL
   #define NBTILE  2
char* videonames[NBTILE]={
	"/home/roys/Videos/pbsice/ice+0+0x640x720.avi",
	"/home/roys/Videos/pbsice/ice+640+0x640x720.avi"};
char* offsetnames[NBTILE]={"+0+0","+640+0"};
	#define BASEX 1280
	#define BASEY 720
#endif
#ifdef QUAD
   #define NBTILE  4
char* videonames[NBTILE]={
	"/home/roys/Videos/pbsice/ice+0+0x640x360.avi",
	"/home/roys/Videos/pbsice/ice+0+360x640x360.avi",
	"/home/roys/Videos/pbsice/ice+640+0x640x360.avi",
	"/home/roys/Videos/pbsice/ice+640+360x640x360.avi"};
char* offsetnames[NBTILE]={"+0+0","+0+360","+640+0","+640+360"};
	#define BASEX 1280
	#define BASEY 720
#endif


#ifdef OCTO
   #define NBTILE  8
char* videonames[NBTILE]={
	"/home/projo/lighttwist/data/bassin-dynamic-20-right_Tile00000x00000x00256x00584.avi",
	"/home/projo/lighttwist/data/bassin-dynamic-20-right_Tile00256x00000x00256x00584.avi",
	"/home/projo/lighttwist/data/bassin-dynamic-20-right_Tile00512x00000x00256x00584.avi",
	"/home/projo/lighttwist/data/bassin-dynamic-20-right_Tile00768x00000x00256x00584.avi"
	"/home/projo/lighttwist/data/bassin-dynamic-20-right_Tile01024x00000x00256x00584.avi",
	"/home/projo/lighttwist/data/bassin-dynamic-20-right_Tile01280x00000x00256x00584.avi",
	"/home/projo/lighttwist/data/bassin-dynamic-20-right_Tile01536x00000x00256x00584.avi",
	"/home/projo/lighttwist/data/bassin-dynamic-20-right_Tile01792x00000x00256x00584.avi"
	};
char* offsetnames[NBTILE]={"+0+0","+256+0","+512+0","+768+0","+1024+0","+1280+0","+1536+0","+1792+0"};
	#define BASEX (NBTILE*256)
	#define BASEY 584
#endif
#ifdef OCTO4
   #define NBTILE  16
char* videonames[NBTILE]={
	"bassin-dynamic-20-right_Tile00000x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile00256x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile00512x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile00768x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile01024x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile01280x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile01536x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile01792x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile02048x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile02304x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile02560x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile02816x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile03072x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile03328x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile03584x00000x00256x00584.mp4",
	"bassin-dynamic-20-right_Tile03840x00000x00256x00584.mp4"
	};
char* offsetnames[NBTILE]={
"+0+0","+256+0","+512+0","+768+0","+1024+0","+1280+0","+1536+0","+1792+0",
"+2048+0","+2304+0","+2560+0","+2816+0","+3072+0","+3328+0","+3584+0","+3840+0"
};
	#define BASEX (NBTILE*256)
	#define BASEY 584
#endif
**/

// pour le viewer
#define WRESX  1280
#define WRESY  720

//
// test YUV
//


#define OUT_FORMAT   "yuv"
#define RESX  BASEX
#define RESY  (BASEY*3/2)
#define RESC 1


//
// test GRAY
//
/*
#define OUT_FORMAT   "gray"
#define RESX  1280
#define RESY  720
#define RESC 1
*/


//
// RGB
//
/*
#define OUT_FORMAT   "rgb"
#define RESX  1280
#define RESY  720
#define RESC 3
*/



// normalement "30", ou "60", ou "video", ou "fast"
// attention c'est une string
#define SPEED "40"
#define FINALFPS 30.0

static void overflow(int d)
{
	printf("*********** overflow %d \n",d);
	exit(0);
}



static int doVideoTile(int start,const char *moviename)
{
rqueue* Qrecycle;
int tid_drip; // drip filter pour le fps
int i,k;
char buf[200];

	// get all tiles for this video
//vector4 lutbb; // LUT bounding box for this projector.
//int lutbbpixels[4];
/**
	lutbb[XMIN]=0.4;
	lutbb[XMAX]=0.6;
	lutbb[YMIN]=0.0;
	lutbb[YMAX]=1.0;
*/

	printf("****** LUT BB (%f - %f , %f - %f) \n",lutbb[XMIN],lutbb[XMAX],lutbb[YMIN],lutbb[YMAX]);
	nbTile=getTilesCoord(moviename,lutbb,tiles,MAXNBTILE,lutbbpixels);

	printf("** final got nbtile=%d\n",nbTile);

    // compute full image size
    int xmin,xmax,ymin,ymax;
	xmin=99999; xmax=0;
	ymin=99999; ymax=0;
    for(i=0;i<nbTile;i++) {
	if( tiles[i].x<xmin ) xmin=tiles[i].x;
	if( tiles[i].x+tiles[i].width-1>xmax ) xmax=tiles[i].x+tiles[i].width-1;
	if( tiles[i].y<ymin ) ymin=tiles[i].y;
	if( tiles[i].y+tiles[i].height-1>ymax ) ymax=tiles[i].y+tiles[i].height-1;
    }
    int fullW=xmax-xmin+1;
    int fullH=ymax-ymin+1;
    printf("image bounds is (%d-%d,%d-%d) %dx%d\n",xmin,xmax,ymin,ymax,fullW,fullH);

    // create a recycling queue
    Qrecycle=imguRegisterQueueX("recycle",50,100);


    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    for(i=0;i<nbTile;i++) {
	if( snprintf(buf,sizeof(buf),"movie%d",i) >= sizeof(buf) ) overflow(0);
    	imguRegisterQueueX(buf,100,100);
    }

    char b1[200],b2[200];
    char bufin[200],bufout[200];

    for(i=0;i<nbTile;i++) {
	if( i==0 ) 	strcpy(bufin,"recycle");
	else		sprintf(bufin,"movie%d",i-1);
	sprintf(bufout,"movie%d",i);
	if( snprintf(b1,sizeof(b1),"-in %s -out %s -file %s -fps %s -outformat %s -offsetkey %s",bufin,bufout,tiles[i].filename,SPEED,OUT_FORMAT,bufout) >= sizeof(b1) ) return(-2);
	printf("STARTING FFMPEG : %s\n",b1);
        tiles[i].tid_cam=imguStartPlugin("ffmpeg","camera",b1);
        if( tiles[i].tid_cam<0 ) { printf("Could not start the ffmpeg plugin %d\n",i);return(-1); }
	printf("ZZzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n");
	//sleep(2);
    }

    //
    // start the drip
    //
    char v1[200];
    struct timeval tv;
    gettimeofday(&tv,NULL);
    tv.tv_sec+=2; // dans 2 secondes on commence

    if( start>0 ) { tv.tv_sec=start;tv.tv_usec=0; }

    if( snprintf(v1,sizeof(v1),"-in %s -out display -startsec %d -startusec %d -fps %f",bufout,(int)tv.tv_sec,(int)tv.tv_usec,FINALFPS) >=sizeof(v1) ) overflow(5);
    tid_drip=imguStartPlugin("drip","filter",v1);
    if( tid_drip<0 ) { printf("!!!!!!!!!!!! NO DRIP PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    for(i=0;i<nbTile;i++) tiles[i].pl_camera=imguGetPluginParameters(tiles[i].tid_cam); 
    // ne sert a rien pour un movie...
    for(i=0;i<nbTile;i++) paramInvokeCommand(tiles[i].pl_camera,"START");

	// YUV!!!
    fullH=fullH*3/2;

    // we decide the image size
    // each image must be different buffer
    imgu *IA;
    char b3[200];
    for(i=0;i<30;i++) {
		IA=NULL;
		imguAllocate(&IA,fullW,fullH,RESC);
		// ajoute un positionnement du video
		int j;
		for(j=0;j<nbTile;j++) {
			sprintf(b2,"movie%d",j);
			// pour rire on echange les tuiles
			//imguReplaceAddText(IA,b2,offsetnames[NBTILE-1-j]);
			sprintf(b3,"+%d+%d",tiles[j].x-xmin,tiles[j].y-ymin);
			imguReplaceAddText(IA,b2,b3);
			//printf("****** OFFSET %s is >%s<\n",b2,b3);
		}
		RQueueAddLast(Qrecycle,(void *)&IA);
    }

    return 0;
}

void endVideoTile(void)
{
    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    int i;
    for(i=0;i<nbTile;i++) imguStopPlugin(tiles[i].tid_cam);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("display",NULL);

    char b2[200];
    for(i=0;i<nbTile;i++) {
	sprintf(b2,"movie%d",i);
        imguUnregisterQueue(b2,NULL);
    }
}

//
// Le viewer sera accédé par une queue display
//
int buildViewer(int geomx,int geomy,int geomw,int geomh)
{
    Qdisplay=imguRegisterQueueX("display",100,100);

    char v1[200];
    // start the viewer
	// yuvclamp
#ifdef IMGU8
    if( snprintf(v1,sizeof(v1),"-in display -view0 yuvlutHLblend 0 0 1 1 -geom %d %d %d %d false",geomx,geomy,geomw,geomh) >=sizeof(v1) ) return(-1);
#else
    if( snprintf(v1,sizeof(v1),"-in display -view0 yuvlutblend 0 0 1 1 -geom %d %d %d %d false",geomx,geomy,geomw,geomh) >=sizeof(v1) ) return(-1);
#endif
    tid_view=imguStartPlugin("viewer","sink",v1);
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");return(-1); }

    pl_view=imguGetPluginParameters(tid_view);
    return(0);
}


#ifdef LUT

#ifdef IMGU8

int loadLUT(char *lutName, double bb[4],double ucutmin,double ucutmax)
{
int k;
    k=imguLoad(&IlutL,lutName,LOAD_8_BITS_LOW);
    if( k ) { printf("Unable to load LUT %s\n",lutName);return(-1); }
    k=imguLoad(&IlutH,lutName,LOAD_8_BITS_HIGH);
    if( k ) { printf("Unable to load LUT %s\n",lutName);return(-1); }

    bb[XMIN]=0.3;
    bb[XMAX]=0.5;
    bb[YMIN]=0.0;
    bb[YMAX]=1.0;

    return(0);
}

int loadLUTLighttwist(char *lutName,char *blendName,double bb[4],double ucutmin,double ucutmax)
{
imgu *Iblend;
double umin,umax,vmin,vmax;
int k;

    k=imguLoad(&IlutL,lutName,LOAD_8_BITS_LOW);
    if( k ) { printf("Unable to load LUT %s\n",lutName);return(-1); }
    k=imguLoad(&IlutH,lutName,LOAD_8_BITS_HIGH);
    if( k ) { printf("Unable to load LUT %s\n",lutName);return(-1); }

	umin=atof(imguGetText(IlutL,"FMIN_U"));
	umax=atof(imguGetText(IlutL,"FMAX_U"));
	vmin=atof(imguGetText(IlutL,"FMIN_V"));
	vmax=atof(imguGetText(IlutL,"FMAX_V"));

	printf("(%f - %f, %f - %f)\n",umin,umax,vmin,vmax);

int uhi,ulow,vhi,vlow;
int i;

    bb[XMIN]=1000.0;
    bb[XMAX]=-100.0;
    bb[YMIN]=100.0;
    bb[YMAX]=-100.0;

   for(i=0;i<IlutL->xs*IlutL->ys;i++) {
	uhi=IlutH->data[i*IlutH->cs+0];
	vhi=IlutH->data[i*IlutH->cs+1];
	ulow=IlutL->data[i*IlutL->cs+0];
	vlow=IlutL->data[i*IlutL->cs+1];
	double u = (uhi*256+ulow)/65535.0*(umax-umin)+umin;
	double v = (vhi*256+vlow)/65535.0*(vmax-vmin)+vmin;

	if( uhi<ucutmin || uhi>ucutmax ) u=-1;

/**
	while( u<0 ) u=1.0;
	while( u>1 ) u-=1.0;
	while( v<0 ) v=1.0;
	while( v>1 ) v-=1.0;
**/

	//if( u<ucutmin || u>ucutmax ) u=-1;

	if( u>=0 ) {
		if( u<bb[XMIN] ) bb[XMIN]=u;
		if( u>bb[XMAX] ) bb[XMAX]=u;
	}
	if( v>=0 ) {
		if( v<bb[YMIN] ) bb[YMIN]=v;
		if( v>bb[YMAX] ) bb[YMAX]=v;
	}

	if( u<0 ) { uhi=ulow=0; } else {
		uhi= ((int)(u*65535+0.5))/256;
		ulow= ((int)(u*65535+0.5))%256;
	}
	if( v<0 ) { vhi=vlow=0; } else {
		vhi= ((int)(v*65535+0.5))/256;
		vlow= ((int)(v*65535+0.5))%256;
	}

	IlutH->data[i*IlutH->cs+0] = uhi;
	IlutH->data[i*IlutH->cs+1] = vhi;

	IlutL->data[i*IlutL->cs+0] = ulow;
	IlutL->data[i*IlutL->cs+1] = vlow;
   }

    Iblend=NULL;
    if( blendName[0] ) {
	    k=imguLoad(&Iblend,blendName,LOAD_8_BITS_HIGH);
   		for(i=0;i<IlutL->xs*IlutL->ys;i++) {
			IlutH->data[i*IlutH->cs+2]=Iblend->data[i*Iblend->cs];
			IlutL->data[i*IlutL->cs+2]=0;
		}
    }

    return(0);
}



#else
int loadLUT(char *lutName,double bb[4],double ucutmin,double ucutmax)
{
int k;
    k=imguLoad(&Ilut,lutName,LOAD_AS_IS);
    if( k ) { printf("Unable to load LUT %s\n",lutName);return(-1); }
    return(0);
}
int loadLUTLighttwist(char *lutName,char *blendName,double bb[4],double ucutmin,double ucutmax)
{
	exit(0);
}


#endif

int sendLUT(char *uniName,int viewnum,imgu *IZ)
{
char buf[50];
	sprintf(buf,"%d",viewnum);
	imguReplaceAddText(IZ,"UNIFORM",uniName);
        imguReplaceAddText(IZ,"VIEWNUM",buf);
	imguSetRecycleQueue(IZ,Qmini);

	// send to viewer
        RQueueAddLast(Qdisplay,(void *)&IZ);

	printf("Sending LUT to viewer...\n");

	// l'image doit revenir...
	while( RQueueRemoveFirstWait(Qmini,(void *)&IZ) ); 
	return(0);
}

#endif


void mainloop()
{
    int k;
    // loop: do nothing except check the if the viewer finished
    int n;
    for(n=0;;n++) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }
        sleep(1);
        //if( n%10==5 ) paramInvokeCommand(pl_scan,"ON");
        //if( n>0 && n%10==0 ) paramInvokeCommand(pl_scan,"OFF");
    }
}


int main( int argc, char **argv )
{
int i;
int start=0;
char blutName[400];
char lutName[400];
char blendName[400];
char movieName[400];
int geom_x,geom_y,geom_w,geom_h;
double ucutmin,ucutmax;
    start=0;
    blutName[0]=0;
    lutName[0]=0;
    blendName[0]=0;
	strcpy(movieName,"bassin-dynamic-20-right.mp4");

ucutmin=-100.;
ucutmax=100.;

	geom_x=0;
	geom_y=0;
	geom_w=1024;
	geom_h=768;

    for(i=1;i<argc;i++) {
	if( strcmp(argv[i],"-ucut")==0 && i+2<argc ) {
		ucutmin=atof(argv[i+1]);
		ucutmax=atof(argv[i+2]);
		i+=2;continue;
	}
	if( strcmp(argv[i],"-geom")==0 && i+4<argc ) {
		geom_x=atoi(argv[i+1]);
		geom_y=atoi(argv[i+2]);
		geom_w=atoi(argv[i+3]);
		geom_h=atoi(argv[i+4]);
		i+=4;continue;
	}
	if( strcmp(argv[i],"-start")==0 && i+1<argc ) {
    		start=atoi(argv[i+1]);
		i++;continue;
	}
	if( strcmp(argv[i],"-lut")==0 && i+1<argc ) {
		strcpy(lutName,argv[i+1]);
		i++;continue;
	}
	if( strcmp(argv[i],"-blend")==0 && i+1<argc ) {
		strcpy(blendName,argv[i+1]);
		i++;continue;
	}
	if( strcmp(argv[i],"-blut")==0 && i+1<argc ) {
		strcpy(blutName,argv[i+1]);
		i++;continue;
	}
	if( strcmp(argv[i],"-movie")==0 && i+1<argc ) {
		strcpy(movieName,argv[i+1]);
		i++;continue;
	}
	printf("unknown option '%s'\n",argv[i]);
	exit(0);
    }

    if( buildViewer(geom_x,geom_y,geom_w,geom_h) ) exit(0);

#ifdef LUT
	Qmini=imguRegisterQueue("mini");

    
    if( blutName[0] ) {
	if( loadLUT(blutName,lutbb,ucutmin,ucutmax) ) { printf("unable to load '%s'\n",lutName);exit(0); }
    }else if( lutName[0] ) {
	if( loadLUTLighttwist(lutName,blendName,lutbb,ucutmin,ucutmax) ) { printf("unable to load '%s'\n",lutName);exit(0); }
    }

	

    if( blutName[0] || lutName[0] ) {
#ifdef IMGU8
	sendLUT("lutH",0,IlutH);
	sendLUT("lutL",0,IlutL);
#else
	sendLUT("lut",0,Ilut);
#endif
    }
#endif

    doVideoTile(start,movieName);

    mainloop();

    endVideoTile();
    return 0;
}


