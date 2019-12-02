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
 * si on utilise electric-fence, on doit definir les variables suivantes:
 * export EF_ALIGNMENT=1   
 *       (This controls the allignment of dynamically allocated memory. By default, this alignment is equal to your machines's word size. This means that small overruns might go unnoticed because extra memory has been allocated for certain buffers. To make sure that this does not happen, set alignment to 1. This ensures that even the small overruns will be caught.)
 * export EF_PROTECT_BELOW=1
 *       (When this option is set to 1, it tells Electric Fence to check for buffer underruns in addtion to buffer overruns.)
 * export EF_PROTECT_FREE=1
 *       (When this option is set to 1, Electric Fence will check to be sure that memory is not being accessed after it has been released with the free() system call. )
*/

#include <unistd.h>

#if defined(TRY_IMGU8)
	#include <imgu8.h>
#elif defined(TRY_IMGU16)
	#include <imgu16.h>
#else
	#include <imgu.h>
#endif


#include <imguextra.h>


//#include <iostream>
#include <sstream>

#define MAX_NBVIEW	100

// pour sauvegarder tout
//#define SAVEIT


typedef struct {
	//char name[100];
	char shader[200];
	double xmin,ymin,xmax,ymax; // view settings
} viewData;

// number of view currently in use.
// next image or plugin will be using this new view.
int nbView;
viewData viewTab[MAX_NBVIEW];

//
// keep track of each plugin, its options, ...
// each plugin has its own recycle queue.
//
typedef struct {
	char name[150]; // name of plugin
	char *options; // strdup of the argv[] for this plugin (NULL=not used)
	char params[800]; // plugin parameters
	int tid;	// plugin identifier (-1=not running)
	char recycleQueue[50];
	paramlist *PL;
	rqueue *rQ;
	} pluginData;

#define MAX_PLUGIN	50
int nbPlugin;
pluginData pluginLoaded[MAX_PLUGIN];




//
// Queues
//

rqueue *Qimages;
rqueue *Qdisplay;

//
// viewer
//

int tid_viewer;
paramlist *pl_viewer;

int tid_save;
int tid_sync;



// add a new camera plugin

// newview: 1=add a new view, 0=use current view.

int addCamera(char *opts,char *shader,double xmin,double ymin,double xmax,double ymax,int newView)
{
pluginData *p;
	if( nbView==0 && newView==0 ) {
		printf("Must have at least on view before using -ucamera\n");
		return(-1);
	}
	if( nbPlugin>=MAX_PLUGIN ) { printf("Out of plugin space!\n");return(-1); }
	printf("*** adding plugin %s\n",opts);
	p=&pluginLoaded[nbPlugin];
	p->options=strdup(opts);
	p->params[0]=0;
	p->tid=-1;
	p->PL=NULL;
	sprintf(p->recycleQueue,"recycleQ%d",nbPlugin);

	p->rQ=imguRegisterQueue(p->recycleQueue);

	// tokenize le format par ":"
	// <plugin name> : <option>=<value> : <option>=<value> : ...
	// -<option> <value> -<option> <value> ...
	char **args=argvFromStringX(p->options,":=");
	int i,nb;
	nb=atoi(args[0]);
	for(i=1;i<=nb;i++) printf("** arg[%d] is '%s'\n",i,args[i]);

	// create the options
	p->params[0]=0;
	// basic options are: -in, -out and -viewnum
	sprintf(p->params,"-in %s -out display -viewnum %d ",p->recycleQueue,
		newView?(nbView):(nbView-1));

	for(i=2;i+1<=nb;i+=2) {
		strcat(p->params,"-");
		strcat(p->params,args[i]);
		strcat(p->params," ");
		strcat(p->params,args[i+1]);
		strcat(p->params," ");
	}

	printf("final option is >>>%s<<<\n",p->params);

	strcpy(p->name,args[1]);

	if( newView ) {
		//strcpy(viewTab[nbView].name,args[1]);
		strcpy(viewTab[nbView].shader,shader);
		viewTab[nbView].xmin=xmin;
		viewTab[nbView].ymin=ymin;
		viewTab[nbView].xmax=xmax;
		viewTab[nbView].ymax=ymax;
		nbView++;
	}

	// add a few images for recycling
	imgu *IA=NULL;
	for(i=0;i<40;i++) RQueueAddLast(p->rQ,(unsigned char *)&IA);

	p->tid=imguStartPlugin(strdup(args[1]),"camera",p->params);
	argsFree(args);
	if( p->tid<0 ) {
		printf("Unable to start plugin %s - %i\n",opts, p->tid);
		//imguUnregisterQueue(NULL,p->rQ);
		//free(p->options);
		return(-1);
	}


	p->PL=imguGetPluginParameters(p->tid);
	//paramInvokeCommand(p->PL,"START");

	printf("Plugin initialized!!!!!!\n");

	nbPlugin++;
	return(0);
}

// add a new image
// use nbView and incremente it
// if uniform not null, then it sets the uniform
// if newView is set, then a new view is created
// loadFlags must be LOAD_KEEP16, or LOAD_8_BITS_HIGH or LOAD_8_BITS_LOW

int addImage(char *filename,int pixelShift,char *shader,double xmin,double ymin,double xmax,double ymax,char *uniform,int newView,int loadFlags)
{
int k;
imgu *IA;
	if( uniform!=NULL && !newView && nbView==0 ) {
		printf("Need at least one view before uniform image %s!\n",filename);
		return(-1);
	}
	// get a recycled image if possible
	IA=NULL;
	RQueueRemoveFirst(Qimages,(unsigned char*)&IA); // if there is no image, no worry. IA=NULL

	// a 16 bit image will be loaded as 2 images
	k=imguLoad(&IA,filename,loadFlags); // LOAD_16_BITS fails if IMGU8
	if( k ) { printf("unable to load '%s'\n",filename);return(-1); }

	imguSetRecycleQueue(IA,Qimages);

	if( pixelShift ) {
		int j;
		pix_t *p=IA->data;
		j=IA->xs*IA->ys*IA->cs;
		while( --j ) *p++<<=pixelShift;
	}

	if( newView ) {
		//strcpy(viewTab[nbView].name,filename);
		strcpy(viewTab[nbView].shader,shader);
		viewTab[nbView].xmin=xmin;
		viewTab[nbView].ymin=ymin;
		viewTab[nbView].xmax=xmax;
		viewTab[nbView].ymax=ymax;
		nbView++;
	}

	if( uniform ) imguReplaceAddText(IA,"UNIFORM",uniform);

	// setup which view
	char buf[10];
	sprintf(buf,"%d",nbView-1);
	imguReplaceAddText(IA,"VIEWNUM",buf);

	// send to the viewer
	RQueueAddLast(Qdisplay,(unsigned char *)&IA);
	return(0);
}


int main( int argc, char **argv )
{
// combien de shift a gauche?
int pixelShift;
int i;
//int fullScreen=0;
int x0=60;
int y0=60;
int initW=640;
int initH=480;
int deco=1; // window decoration?
double xmin,ymin,xmax,ymax;
char saveOutName[100];
int save;
int sync;
char syncDest[100];
int syncPort;
int syncDelay; // en ms 
double syncFactor;
int syncDup;
char shader[100];
int nbframes;

    	imguPluginDump();

	// Queue de recyclage pour les images normales
	Qimages=imguRegisterQueue("images");

	// Queue d'affichage pour le viewer
	Qdisplay=imguRegisterQueue("display");

	rqueue *qsave=imguRegisterQueue("save");
	imguRegisterQueue("sync");


	nbPlugin=0;
	for(i=0;i<MAX_PLUGIN;i++) pluginLoaded[i].options=NULL;


	// nom du fichier de sortie
	save=0;
	saveOutName[0]=0;

	sync=0;
	syncDest[0]=0;
	syncPort=-1;
	syncDelay=0;
	syncFactor=1.0;
	syncDup=1;

    nbframes=0;
	pixelShift=0;

	// current view, by default is the whole screen
	xmin=0.0;
	ymin=0.0;
	xmax=1.0;
	ymax=1.0;

	// current shader
	strcpy(shader,"rgb");

	// all plugins of type camera must have the following parameters:
	// -in <input queue>
	// -out <output queue>
	// -viewnum <viewer number> (defines the tag VIEWNUM)
	// -fps <framerate double> (this is not handled by the imguview)

	// -camera gige:camid=12345:fps=5.0
	// -ucamera ffmpeg:uniform=toto  -> start only the plugin, not the view.
	//     (it will use the current view)

	for(i=1;i<argc;i++) {
		if( strcmp("-h",argv[i])==0 ) {
			printf("Usage: %s -geom x0 y0 w h deco(true/false) -shift 8 -shader yuv|rgb|gray -view 0 0 1 1 (xmin,ymin,xmax,ymax) [-image uniname] toto.png -uimage uniname toto.png [-sync localhost 23456 5(ms) | -varsync localhost 23456 1000(ms) 0.9(factor) 2(dup)]  -camera v4l2 -geom 100 100 320 240 true (x0,y0,W,H,deco) -ucamera v4l2 [-p ExposureValue 1000]\n",argv[0]);
printf("       -nb 100   -> used with sync to stop eventually\n");
#ifdef IMGU8
			printf("       -> in 8 bit mode, you also have: -imagelow, -imagehi, -uimagelow, -uimagehi which load the hi or low part of a 16bit image\n");
#endif
			exit(0);
		}else if( strcmp("-shift",argv[i])==0 && i+1<argc ) {
			pixelShift=atoi(argv[i+1]);
			i++;continue;
		}else if( strcmp("-nb",argv[i])==0 && i+1<argc ) {
			nbframes=atoi(argv[i+1]);
			i++;continue;
		}else if( strcmp("-shader",argv[i])==0 && i+1<argc ) {
			strcpy(shader,argv[i+1]);
			i++;continue;
		}else if( strcmp("-view",argv[i])==0 && i+4<argc ) {
			xmin=atof(argv[i+1]);
			ymin=atof(argv[i+2]);
			xmax=atof(argv[i+3]);
			ymax=atof(argv[i+4]);
			i+=4;continue;
		}else if( strcmp("-geom",argv[i])==0 && i+4<argc ) {
			x0=atoi(argv[i+1]);
			y0=atoi(argv[i+2]);
			initW=atoi(argv[i+3]);
			initH=atoi(argv[i+4]);
			deco=(strcmp(argv[i+5],"true")==0);
			i+=5;continue;
		}else if( strcmp("-camera",argv[i])==0 && i+1<argc ) {
			// create a camera in a new view
			if( addCamera(argv[i+1],shader,xmin,ymin,xmax,ymax,1) ) exit(0);
			i++;continue;
		}else if( strcmp("-ucamera",argv[i])==0 && i+1<argc ) {
			// create a camera in the same view (no new view)
			if( addCamera(argv[i+1],shader,xmin,ymin,xmax,ymax,0) ) exit(0);
			i++;continue;
		}else if( strcmp("-p",argv[i])==0 && i+2<argc ) {
			pluginData *cp;
			cp=&pluginLoaded[nbPlugin];
			cp->PL=imguGetPluginParameters(cp->tid);
			if(paramSetString(cp->PL,argv[i+1],argv[i+2])<0)
				printf("INVALID PARAMETER: %s=%s\n", argv[i+1],argv[i+2]);
			i+=2;
		}else if( strcmp("-save",argv[i])==0 && i+1<argc ) {
			// suivi du nom de fichier, style out%04d.png
			strcpy(saveOutName,argv[i+1]);
			save=1;
			i+=1;
		}else if( strcmp("-sync",argv[i])==0 && i+3<argc ) {
			// suivi de la destination et port, et du delai
			sync=1;
			strcpy(syncDest,argv[i+1]);
			syncPort=atoi(argv[i+2]);
			syncDelay=atoi(argv[i+3]);
			syncFactor=1.0;
			syncDup=1;
			i+=3;
		}else if( strcmp("-varsync",argv[i])==0 && i+5<argc ) {
			// suivi de la destination et port, et du delai
			sync=1;
			strcpy(syncDest,argv[i+1]);
			syncPort=atoi(argv[i+2]);
			syncDelay=atoi(argv[i+3]);
			syncFactor=atof(argv[i+4]);
			syncDup=atoi(argv[i+5]);
			i+=5;
		}else if( strcmp("-image",argv[i])==0 && i+2<argc ) {
			if( addImage(argv[i+2],pixelShift,shader,xmin,ymin,xmax,ymax,argv[i+1],1,LOAD_AS_KEEP16) ) exit(0);
			i+=2;
#ifdef IMGU8
		}else if( strcmp("-imagelow",argv[i])==0 && i+2<argc ) {
			if( addImage(argv[i+2],pixelShift,shader,xmin,ymin,xmax,ymax,argv[i+1],1,LOAD_8_BITS_LOW) ) exit(0);
			i+=2;
		}else if( strcmp("-imagehi",argv[i])==0 && i+2<argc ) {
			if( addImage(argv[i+2],pixelShift,shader,xmin,ymin,xmax,ymax,argv[i+1],1,LOAD_8_BITS_HIGH) ) exit(0);
			i+=2;
#endif
		}else if( strcmp("-uimage",argv[i])==0 && i+2<argc ) {
			if( addImage(argv[i+2],pixelShift,shader,xmin,ymin,xmax,ymax,argv[i+1],0,LOAD_AS_KEEP16) ) exit(0);
			i+=2;
#ifdef IMGU8
		}else if( strcmp("-uimagelow",argv[i])==0 && i+2<argc ) {
			if( addImage(argv[i+2],pixelShift,shader,xmin,ymin,xmax,ymax,argv[i+1],0,LOAD_8_BITS_LOW) ) exit(0);
			i+=2;
		}else if( strcmp("-uimagehi",argv[i])==0 && i+2<argc ) {
			if( addImage(argv[i+2],pixelShift,shader,xmin,ymin,xmax,ymax,argv[i+1],0,LOAD_8_BITS_HIGH) ) exit(0);
			i+=2;
#endif
		}else{
			if( addImage(argv[i],pixelShift,shader,xmin,ymin,xmax,ymax,NULL,1,LOAD_AS_KEEP16) ) exit(0);
		}
	}

	// demarre les cameras
	for(i=0;i<nbPlugin;i++) {
		pluginData *p;
		p=&pluginLoaded[i];
		paramInvokeCommand(p->PL,"START");
		printf("START command sent to plugin %d\n",i);
	}

/***
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
***/


#ifdef SKIP
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
#endif

	printf("NBVIEW  is %d\n",nbView);

	//
	// create a viewer
	// (we create the viewer last, only if everything started ok)
	//
	//tid_viewer=imguStartPlugin("viewer","sink","-in display -view0 cam0 0 0 1 1 -view1 cam1 0 0 1 1");
	char buf[500];
	if( sync )  {
		sprintf(buf,"-in display -out sync -geom %d %d %d %d %s",x0,y0,initW,initH,deco?"true":"false");
	}else if( save )  {
		sprintf(buf,"-in display -out save -geom %d %d %d %d %s",x0,y0,initW,initH,deco?"true":"false");
	}else{
		sprintf(buf,"-in display -geom %d %d %d %d %s",x0,y0,initW,initH,deco?"true":"false");
	}
	for(i=0;i<nbView;i++) {
		viewData *p=viewTab+i;
		//sprintf(buf+strlen(buf)," -view%d %s %f %f %f %f",i,p->name,p->xmin,p->ymin,p->xmax,p->ymax);
		sprintf(buf+strlen(buf)," -view%d %s %f %f %f %f",i,p->shader,p->xmin,p->ymin,p->xmax,p->ymax);
	}
	tid_viewer=imguStartPlugin("viewer","sink",buf);
	if( tid_viewer<0 ) { printf("Unable to start viewer\n");exit(0); }

	if( sync ) {
		if( save ) {
			//sprintf(buf,"-in sync -out save -dest %s -port %d -delay %d -nb %d",syncDest,syncPort,syncDelay,nbframes);
			sprintf(buf,"-in sync -out save -dest %s -port %d -delay %d -nb %d -factor %f -dup %d",syncDest,syncPort,syncDelay,nbframes,syncFactor,syncDup);
		}else{
			//sprintf(buf,"-in sync -dest %s -port %d -delay %d -nb %d",syncDest,syncPort,syncDelay,nbframes);
			sprintf(buf,"-in sync -dest %s -port %d -delay %d -nb %d -factor %f -dup %d",syncDest,syncPort,syncDelay,nbframes,syncFactor,syncDup);
		}
		tid_sync=imguStartPlugin("sync","filter",buf);
		if( tid_sync<0 ) { printf("Unable to start sync\n");exit(0); }
	}else	tid_sync=-1;

	if( save ) {
		sprintf(buf,"-in save -filename %s",saveOutName);
		tid_save=imguStartPlugin("save","sink",buf);
		if( tid_save<0 ) { printf("Unable to start save (name %s)\n",saveOutName);exit(0); }
	}else	tid_save=-1;

	paramlist *pl_viewer=imguGetPluginParameters(tid_viewer);
	paramDump(pl_viewer);


	//
	// boucle principale
	//
	int k;
	char event[100];
	double lastEvent=-1.0; // time of last event. New event should be later than this
	double time,x,y;
	char type[50];

	for(;;) {
		if( paramGetInt(pl_viewer,"done",&k)==0 && k ) {
			printf("DONE!\n");break;
		}
		// check tous les autres
		int fini=0;
		for(i=0;i<nbPlugin;i++) {
			if( paramGetInt(pluginLoaded[i].PL,"done",&k)==0 && k ) {
				printf("plugin %d DONE!!!!\n",i);
				fini=1;break;
			}
		}
		if( fini ) break;
		if( paramGetString(pl_viewer,"event",event,sizeof(event)) ) {
			sscanf(event,"%lf:%[^:]:",&time,type);
			if( time>lastEvent ) {
				lastEvent=time;
				if( strcmp(type,"keydown")==0 ) {
					int key;
					sscanf(event,"%lf:%[^:]:%lf:%lf:%d",&time,type,&x,&y,&key);
					//
					// forward/backward seek
					switch(key) {
					  case 'u': // seek forward
						paramSetDouble(pluginLoaded[0].PL,"SEEKTIME",10.0);
						break;
					  case 'i': // seek forward (frames)
						paramSetInt(pluginLoaded[0].PL,"SEEKFRAME",501);
						break;
					  case 'o': // seek forward (frames)
						paramSetInt(pluginLoaded[0].PL,"SEEKFRAME",502);
						break;
					  case 'p': // seek forward (frames)
						paramSetInt(pluginLoaded[0].PL,"SEEKFRAME",503);
					}
				}
			}
		}
		//paramDump(pl_viewer);
		//usleep(2500000);
		usleep(250000);
	}

    fprintf(stderr,"[imgu] Number of images still in saving queue: %d.\n",qsave->n);
    fflush(stderr);
    sleep(4);
    /*while(qsave->n>0) 
    {
      fprintf(stderr,"[imgu] Waiting to save all images (%d).\n",qsave->n);
      fflush(stderr);
      usleep(100);
    }*/
    //
    // Output some statistics
    //
    //imguPluginDumpDot("out.dot");

	// Kill all queues
	imguUnregisterQueue(NULL,Qimages);
	imguUnregisterQueue(NULL,Qdisplay);
	imguUnregisterQueue("save",NULL);
	imguUnregisterQueue("sync",NULL);

    //
    // we are done!!!
    //
    imguStopPlugin(tid_viewer);
    if( save ) imguStopPlugin(tid_save);

    return(0);
}

