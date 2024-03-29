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


#define NB_INVERSE  8

#include <imgu.h>
#include <unistd.h>
#include <string.h>

//
// localprototypes
//
int doSimple(void);
int doMovie( void );
int do1( void);
int do2( void);
int doRadialdist( void);
int doScannerEffect(void);
int doFrameBlendFilter(void);
int doKeyingFilter(void);
int doCrayon(void);
int doSave(void);
int doDoubleCapture(int v1,int v2,int fps);



// pour ne pas avoir de vue
// ca ne sauve pas de cpu, alors...
//#define SKIPVIEW

/**
 * ouvre deux plugins camera (v4l2)
 * et un viewer pour voir les deux en meme temps
 * et un save pour les images.
 * le truc, c'est de faire on/off sur le save...
**/
int doDoubleCapture(int v1,int v2,int fps) {
	rqueue* Qrecycle;
	int tid_cam0;
	int tid_cam1;
	int tid_view;
	int tid_saved1;
	int tid_saved2;
	int i,k;
	char buf[400];

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    imgu *IA=NULL;
    for(i=0;i<300;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cams");
    imguRegisterQueue("saved");

    // start the camera
    sprintf(buf,"-in recycle -out cams -device /dev/video%d -fps %d -width 640 -height 480 -viewnum 0",v1,fps);
    tid_cam0=imguStartPlugin("v4l2","camera",buf);
    if( tid_cam0<0 ) { printf("Unable to start /dev/video%d\n",v1);exit(-1); }

    sprintf(buf,"-in recycle -out cams -device /dev/video%d -fps %d -width 640 -height 480 -viewnum 1",v2,fps);
    tid_cam1=imguStartPlugin("v4l2","camera",buf);
    if( tid_cam1<0 ) { printf("Unable to start /dev/video%d\n",v2);exit(-1); }


#ifndef SKIPVIEW
    // start the viewer
    tid_view=imguStartPlugin("viewer","sink","-in saved -geom 0 0 1280 480 false -view0 cam0 0 0 0.5 1 -view1 cam1 0.5 0 1 1");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }
#endif

    ///#define USAGE "-in Q -out Q -compression 1 -filename out%06d.png -singlefilename out.png -start 0 -nb 100000 -keynum FRAMENUM -viewnum VIEWNUM"

#ifndef SKIPVIEW
    tid_saved1=imguStartPlugin("save","sink","-in cams -out saved -compression 0 -filename a%08d-%d.png -start 0 -nb 1000000 -viewnum VIEWNUM");
    tid_saved2=imguStartPlugin("save","sink","-in cams -out saved -compression 0 -filename b%08d-%d.png -start 0 -nb 1000000 -viewnum VIEWNUM");
#else
    tid_saved1=imguStartPlugin("save","sink","-in cams -compression 0 -filename a%08d-%d.png -start 0 -nb 1000000 -viewnum VIEWNUM");
    tid_saved2=imguStartPlugin("save","sink","-in cams -compression 0 -filename b%08d-%d.png -start 0 -nb 1000000 -viewnum VIEWNUM");
#endif
    if( tid_saved1<0 ) { printf("Unable to start save plugin\n");exit(-1); }
    if( tid_saved2<0 ) { printf("Unable to start save plugin\n");exit(-1); }


    // get access to plugin parameters
    paramlist *pl_cam0=imguGetPluginParameters(tid_cam0);
    paramlist *pl_cam1=imguGetPluginParameters(tid_cam1);
#ifndef SKIPVIEW
    paramlist *pl_view=imguGetPluginParameters(tid_view);
#endif
    paramlist *pl_saved1=imguGetPluginParameters(tid_saved1);
    paramlist *pl_saved2=imguGetPluginParameters(tid_saved2);

    paramInvokeCommand(pl_cam0,"START");
    paramInvokeCommand(pl_cam1,"START");

    sleep(10);
    paramInvokeCommand(pl_saved1,"START");
    paramInvokeCommand(pl_saved2,"START");

    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time


    // main loop: do nothing except check the if the viewer finished
    int n;
    for(n=0;;n++) {
#ifndef SKIPVIEW
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }
#else
	printf("Zzz\n");
#endif
        sleep(1);
    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam0);
    imguStopPlugin(tid_cam1);
    imguStopPlugin(tid_saved1);
    imguStopPlugin(tid_saved2);
#ifndef SKIPVIEW
    imguStopPlugin(tid_view);
#endif

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("cams",NULL);
    imguUnregisterQueue("saved",NULL);
    return(0);
}

//
// FrameBlend test
//
// plugins used: frameblend_filter, gige_camera, viewer_sink
//

int doSave(void) {

rqueue* Qrecycle;
int tid_cam;    // thread id of pattern_camera plugin
int tid_inv;   // thread id of view_sink plugin
int tid_save;     // keyuing effect
int i,k;

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cam");
    imguRegisterQueue("inv");
    imguRegisterQueue("crap");

    // start the camera
    tid_cam=imguStartPlugin("gige","camera","-in recycle -out cam -camid 13137 -mcast 1 -viewnum 0 "); // -fps 01
    if( tid_cam<0 ) { printf("Could not start the gige plugin\n");exit(-1); }

    // start the scan
    //tid_inv=imguStartPlugin("inverse","filter","-in cam -out inv");
    //if( tid_inv<0 ) { printf("Could not start the inv plugin\n");exit(-1); }
    ///#define USAGE "-in Q -out Q -compression 1 -filename out%06d.png -singlefilename out.png -start 0 -nb 100000 -keynum FRAMENUM -viewnum VIEWNUM"
    tid_save=imguStartPlugin("save","sink","-in cam -out recycle -compression 1 -filename cap%08d.png -start 0 -nb 1000000 -viewnum 0");
    if( tid_save<0 ) { printf("Could not start the save plugin\n");exit(-1); }

    // start the viewer
    //tid_view=imguStartPlugin("viewer","sink","-in keying -view0 cam0 0 0 1 1 -geom 0 0 800 600 true ");
    //if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    paramlist *pl_camera=imguGetPluginParameters(tid_cam);
    //paramlist *pl_inv=imguGetPluginParameters(tid_inv);
    paramlist *pl_save=imguGetPluginParameters(tid_save);

    paramInvokeCommand(pl_camera,"START");
    paramInvokeCommand(pl_save,"START");

    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time


    // main loop: do nothing except check the if the viewer finished
    int n;
    for(n=0;;n++) {
        //paramGetInt(pl_view,"done",&k);
        //if( k ) { printf("We are done!\n");break; }
        sleep(1);
    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_save);
    imguStopPlugin(tid_inv);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("cam",NULL);
    imguUnregisterQueue("inv",NULL);
    return(0);
}


int doKeyingFilter(void) {

rqueue* Qrecycle;
int tid_cam;    // thread id of pattern_camera plugin
int tid_view;   // thread id of view_sink plugin
int tid_keying;     // keyuing effect
int i,k;

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cam");
    imguRegisterQueue("keying");

    // start the camera
    tid_cam=imguStartPlugin("gige","camera","-in recycle -out cam -camid 12143 -fps 10 -viewnum 0 ");
    if( tid_cam<0 ) { printf("Could not start the gige plugin\n");exit(-1); }

    // start the scan
    //tid_keying=imguStartPlugin("keying","filter","-in cam -out keying -l 0.40");
    tid_keying=imguStartPlugin("keying","filter","-in cam -out keying -l 0.4");
    if( tid_keying<0 ) { printf("Could not start the keying plugin\n");exit(-1); }

    // start the viewer
    tid_view=imguStartPlugin("viewer","sink","-in keying -view0 cam0 0 0 1 1 -geom 0 0 800 600 true ");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    paramlist *pl_camera=imguGetPluginParameters(tid_cam);
    paramlist *pl_view=imguGetPluginParameters(tid_view);
    paramlist *pl_keying=imguGetPluginParameters(tid_keying);

    paramInvokeCommand(pl_camera,"START");

    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    // main loop: do nothing except check the if the viewer finished
    int n;
    for(n=0;;n++) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }
        sleep(1);
    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_view);
    imguStopPlugin(tid_keying);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("a",NULL);
    imguUnregisterQueue("b",NULL);
    return(0);
}

//
// FrameBlend test
//
// plugins used: frameblend_filter, gige_camera, viewer_sink
//
int doFrameBlendFilter(void)
{
rqueue* Qrecycle;
int tid_cam;    // thread id of pattern_camera plugin
int tid_view;   // thread id of view_sink plugin
int tid_frameblend;     // frameblend effect
int i,k;

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cam");
    imguRegisterQueue("frameblend");

    // start the pattern
    //tid_cam=imguStartPlugin("ffmpeg","camera","-in recycle -out cam -file /home/vision/lighttwist/data/test/plantspano2.mpg -fps video");
    //if( tid_cam<0 ) { printf("Could not start the ffmpeg plugin\n");exit(-1); }
    tid_cam=imguStartPlugin("gige","camera","-in recycle -out cam -camid 13136 -fps 10 -viewnum 0 ");
    if( tid_cam<0 ) { printf("Could not start the gige plugin\n");exit(-1); }

    // start the scan
    tid_frameblend=imguStartPlugin("frameblend","filter","-in cam -out frameblend -ratio 0.94");
    if( tid_frameblend<0 ) { printf("Could not start the frameblend plugin\n");exit(-1); }

    // start the viewer
    tid_view=imguStartPlugin("viewer","sink","-in frameblend -view0 cam0 0 0 1 1 -geom 1440 0 800 600 false ");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    paramlist *pl_camera=imguGetPluginParameters(tid_cam);
    paramlist *pl_view=imguGetPluginParameters(tid_view);
    paramlist *pl_frameblend=imguGetPluginParameters(tid_frameblend);

    paramInvokeCommand(pl_camera,"START");

    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    // main loop: do nothing except check the if the viewer finished
    int n;
    for(n=0;;n++) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }
        sleep(1);
        //if( n%10==5 ) paramInvokeCommand(pl_scan,"ON");
        //if( n>0 && n%10==0 ) paramInvokeCommand(pl_scan,"OFF");
    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_view);
    imguStopPlugin(tid_frameblend);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("a",NULL);
    imguUnregisterQueue("b",NULL);
    return(0);
}


//
// Scannereffect test
//
// plugins used: scannereffect_filter, ffmpeg_camera, viewer_sink
//
int doScannerEffect(void)
{
rqueue* Qrecycle;
int tid_cam;    // thread id of pattern_camera plugin
int tid_view;   // thread id of view_sink plugin
int tid_scan;   // scanner effect
int i,k;

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cam");
    imguRegisterQueue("scan");

    // start the pattern
    tid_cam=imguStartPlugin("ffmpeg","camera","-in recycle -out cam -file /home/vision/lighttwist/data/test/plantspano2.mpg -fps video");
    if( tid_cam<0 ) { printf("Could not start the ffmpeg plugin\n");exit(-1); }

    // start the scan
    tid_scan=imguStartPlugin("scannereffect","filter","-in cam -out scan");
    if( tid_scan<0 ) { printf("Could not start the scannereffect plugin\n");exit(-1); }

    // start the viewer
    tid_view=imguStartPlugin("viewer","sink","-in scan");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    paramlist *pl_view=imguGetPluginParameters(tid_view);
    paramlist *pl_scan=imguGetPluginParameters(tid_scan);

    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    paramInvokeCommand(pl_scan,"ON");

    // main loop: do nothing except check the if the viewer finished
    int n;
    for(n=0;;n++) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }
        sleep(1);
        //if( n%10==5 ) paramInvokeCommand(pl_scan,"ON");
        //if( n>0 && n%10==0 ) paramInvokeCommand(pl_scan,"OFF");
    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_view);
    imguStopPlugin(tid_scan);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("a",NULL);
    imguUnregisterQueue("b",NULL);
    return(0);
}




//
// simplest example
//
// display a random pattern in a viewer
//
// plugins used: pattern_camera, viewer_sink
//
int doSimple(void)
{
rqueue* Qrecycle;
int tid_cam;    // thread id of pattern_camera plugin
int tid_view;   // thread id of view_sink plugin
int i,k;

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cam");

    // start the pattern
    tid_cam=imguStartPlugin("pattern","camera","-in recycle -out cam -width 512 -height 512 -fps 15");
    if( tid_cam<0 ) { printf("Could not start the pattern plugin\n");exit(-1); }

    // start the viewer
    tid_view=imguStartPlugin("viewer","sink","-in cam");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    paramlist *pl_view=imguGetPluginParameters(tid_view);

    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    // main loop: do nothing except check the if the viewer finished
    for(;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }
        sleep(1);
    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("a",NULL);
    imguUnregisterQueue("b",NULL);
    return(0);
}

/*******************************************************************************/


/*******************************************************************************/


/*******************************************************************************/


/*******************************************************************************/



//
// simplest example
//
// display a random pattern in a viewer
//
// plugins used: pattern_camera, viewer_sink
//
int doCrayon(void)
{
rqueue* Qrecycle;
int tid_cam;    // thread id of pattern_camera plugin
int tid_view;   // thread id of view_sink plugin
int i,k;

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cam");

    // start the pattern
    tid_cam=imguStartPlugin("crayon","camera","-in recycle -out cam -img ass.pjp");
    if( tid_cam<0 ) { printf("Could not start the crayon plugin\n");exit(-1); }

    // start the viewer
    tid_view=imguStartPlugin("viewer","sink","-in cam");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    paramlist *pl_view=imguGetPluginParameters(tid_view);

    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    // main loop: do nothing except check the if the viewer finished
    for(;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }
        sleep(1);
    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("a",NULL);
    imguUnregisterQueue("b",NULL);
    return(0);
}


/*******************************************************************************/


/*******************************************************************************/


/*******************************************************************************/



//
// Play a film, view and save the images
//
int doMovie( void )
{
int i,k;
int tid_cam,tid_view,tid_radial;
rqueue *Qrecycle;

    // create a send and receive queue
    // the Qfree queue is local to us, for recycling.
    Qrecycle    =imguRegisterQueue("recycle");
    imguRegisterQueue("cam");
    imguRegisterQueue("a");

    tid_cam=imguStartPlugin("ffmpeg","camera",
        "-in recycle -out cam -file /home/vision/lighttwist/media/pbs_ice_10sec.avi -fps 30");
    if( tid_cam<0 ) exit(-1);

    tid_radial=imguStartPlugin("radialdist","gpufilter","-in cam -out a -width 1280 -height 720 -zoom 0 -coeffs coeffDisto_d60.data");
    if( tid_radial<0 ) exit(-1);

    tid_view=imguStartPlugin("viewer","sink","-in a");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    paramlist *pl_view=imguGetPluginParameters(tid_view);

    // create a couple of empty images for the plugin to use
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    for(;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("DONEDONE!\n");break; }

        sleep(1);
    }

    // output statistics
    imguPluginDumpDot("out.dot");

    //
    // we are done!!!
    //
    imguStopPlugin(tid_cam);
    //imguStopPlugin(tid_save);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("a",NULL);
    imguUnregisterQueue("b",NULL);
    return(0);
}



//
// Play a film, view and save the images
//
int do1(void )
{
int i,k;
int tid_camera,tid_view;
//int tid_save;
int tid_crop;
int tid_inverse[NB_INVERSE];
rqueue *Qrecycle;

    // create a send and receive queue
    // the Qfree queue is local to us, for recycling.
    Qrecycle    =imguRegisterQueue("recycle");
    imguRegisterQueue("a");
    imguRegisterQueue("b");

    imgu *IA=NULL;

    tid_camera=imguStartPlugin("ffmpeg","camera",
        //"-in recycle -out done -file http://192.168.2.114/vdata.v -format nc");
        //"-in recycle -out done -file /home/roys/Bureau/Video/00008.MTS");
        //"-in recycle -out send -file /home/roys/Bureau/MOV023.TOD -format mpegts");
        "-fps 5 -in recycle -out a -file /home/roys/Vidéos/AstroViz__Solar_Storms[SPRP00000160].mkv");
        //"-in recycle -out send -file /home/roys/Vidéos/Heroes/Heroes.S03E10.HDTV.XviD-LOL.[VTV].avi");
        //"-in recyclebin -out send -file /home/roys/Bureau/Video/HDmovies/MOV06B.avi");
    if( tid_camera<0 ) exit(-1);

    //tid_save=imguStartPlugin("save","sink","-in a -out b -filename blub%03d.png");
    //if( tid_save<0 ) exit(-1);

    //tid_view=imguStartPlugin("viewer","sink","-in send -out exit");
    tid_view=imguStartPlugin("viewer","sink","-in a");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }


    paramlist *pl_view=imguGetPluginParameters(tid_view);

    // create a couple of empty images for the plugin to use
    IA=NULL;
    for(i=0;i<100;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    for(;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("DONEDONE!\n");break; }

        sleep(1);
    }

    // output statistics
    imguPluginDumpDot("out.dot");

    //
    // we are done!!!
    //
    imguStopPlugin(tid_camera);
    //imguStopPlugin(tid_save);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("a",NULL);
    imguUnregisterQueue("b",NULL);
    return(0);
}







int do2( void )
{
int i,k;
int tid_camera,tid_save,tid_view;
int tid_crop;
int tid_inverse[NB_INVERSE];
rqueue *Qrecycle;

    //if( imguLoadPlugin("inverse") ) exit(-1);
    //if( imguLoadPlugin("camera_ffmpeg") ) exit(-1);
//  if( imguLoadPlugin("save") ) exit(-1);

    imguPluginDump();

    // create a send and receive queue
    // the Qfree queue is local to us, for recycling.
    Qrecycle    =imguRegisterQueue("recycle");
    imguRegisterQueue("send");
    imguRegisterQueue("receive");
    imguRegisterQueue("done");
    imguRegisterQueue("exit");


    imgu *IA=NULL;
    //imgu *IB=NULL;
    //char buf[100];

    tid_camera=imguStartPlugin("ffmpeg","camera",
        //"-in recycle -out done -file http://192.168.2.114/vdata.v -format nc");
        //"-in recycle -out done -file /home/roys/Bureau/Video/00008.MTS");
        //"-in recycle -out send -file /home/roys/Bureau/MOV023.TOD -format mpegts");
        "-fps 5 -in recycle -out done -file /home/roys/Vidéos/AstroViz__Solar_Storms[SPRP00000160].mkv");
        //"-in recycle -out send -file /home/roys/Vidéos/Heroes/Heroes.S03E10.HDTV.XviD-LOL.[VTV].avi");
        //"-in recyclebin -out send -file /home/roys/Bureau/Video/HDmovies/MOV06B.avi");
    if( tid_camera<0 ) exit(-1);

    tid_crop=imguStartPlugin("crop","filter","-in send -out receive -crop 100 100 200 200");
    if( tid_crop<0 ) exit(-1);


    for(i=0;i<NB_INVERSE;i++) {
        tid_inverse[i]=imguStartPlugin("inverse","filter","-in receive -out done");
        if( tid_inverse[i]<0 ) exit(-1);
    }

    //tid_save=imguStartPlugin("save","sink","-in receive -out done -filename blub%03d.png");
    //if( tid_save<0 ) exit(-1);

    //tid_view=imguStartPlugin("viewer","sink","-in send -out exit");
    tid_view=imguStartPlugin("viewer","sink","-in done");
    if( tid_view<0 ) exit(-1);


    paramlist *pl_view=imguGetPluginParameters(tid_view);

    // normalement, on wait jusqu'a ce que ca soit initialise, mais bon....
    //usleep(100);

    //imguPluginDumpDot("out.dot");

    //
    // main loop!
    //

    // create a couple of empty images for the plugin to use
    IA=NULL;
    for(i=0;i<8;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    //sleep(10);
    //imguPluginDumpDot("out.dot");
    //printf("--------- statistics are out --------------\n");

    for(;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("DONEDONE!\n");break; }

        sleep(1);
    }
    imguPluginDumpDot("out.dot");
    printf("--------- statistics are out --------------\n");

    //
    // we are done!!!
    //
    imguStopPlugin(tid_camera);
    imguStopPlugin(tid_crop);
    for(i=0;i<NB_INVERSE;i++) imguStopPlugin(tid_inverse[i]);
    //imguStopPlugin(tid_save);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("send",NULL);
    imguUnregisterQueue("receive",NULL);
    imguUnregisterQueue("done",NULL);
    imguUnregisterQueue("exit",NULL);

    return 0;
}

//
//
//
int doRadialdist( void )
{
int i,k;
int tid_cam,tid_view;
int tid_radial;
rqueue *Qrecycle;

    // create a send and receive queue
    // the Qfree queue is local to us, for recycling.
    Qrecycle=imguRegisterQueue("recycle");
    imguRegisterQueue("a");
    imguRegisterQueue("b");

    imgu *IA=NULL;

    tid_cam=imguStartPlugin("pattern","camera","-in recycle -out a -width 768 -height 480 -fps 30");
    //tid_cam=imguStartPlugin("ffmpeg","camera","-fps 15 -in recycle -out a -file /home/roys/Bureau/pbs_ice.avi");
    //tid_cam=imguStartPlugin("load","camera","-fps 15 -in recycle -out a -filename damier_768_480.png -viewnum 0 -make16 1");
    if( tid_cam<0 ) exit(-1);

    //tid_radial=imguStartPlugin("radialdist","gpufilter","-in a -out b -width 768 -height 480 -zoom 1.02");
    //tid_radial=imguStartPlugin("radialdist","gpufilter","-in a -out b -width 768 -height 480 -zoom 0 -coeffs coeffDisto_d60.data");
    tid_radial=imguStartPlugin("radialdist","gpufilter","-in a -out b -width 768 -height 480 -zoom 0 -coeffs coeffDisto_d60.data");
    if( tid_radial<0 ) { printf("!!!!!!!!!!!! NO RADIALDIST PLUGIN! code=%d\n",tid_radial);exit(-1); }

    //tid_save=imguStartPlugin("save","sink","-in a -out b -filename blub%03d.png");
    //if( tid_save<0 ) exit(-1);

    tid_view=imguStartPlugin("viewer","sink","-in b -geom 20 20 640 360 true");
    if( tid_view<0 ) exit(-1);


    paramlist *pl_view=imguGetPluginParameters(tid_view);
    paramlist *pl_radial=imguGetPluginParameters(tid_radial);

    // create a couple of empty images for the plugin to use
    IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    for(;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("DONEDONE!\n");break; }

        sleep(1);
    }

    // output statistics
    imguPluginDumpDot("out.dot");

    //
    // we are done!!!
    //
//  imguStopPlugin(tid_cam);
    imguStopPlugin(tid_radial);
//  imguStopPlugin(tid_view);

//  imguUnregisterQueue("recycle",NULL);
//  imguUnregisterQueue("a",NULL);
//  imguUnregisterQueue("b",NULL);
    return(0);
}



int main( int argc, char **argv )
{
int v1,v2;
int fps;
int i;

	v1=0;
	v2=1;
	fps=10;
	for(i=1;i<argc;i++) {
		if( strcmp(argv[i],"-fps")==0 && i+1<argc ) {
			fps=atoi(argv[i+1]);
			i++;continue;
		}else if( strcmp(argv[i],"-video")==0 && i+2<argc ) {
			v1=atoi(argv[i+1]);
			v2=atoi(argv[i+2]);
			i+=2;continue;
		}
		printf("Usage: %s -video 0 1 -fps 15\n",argv[0]);
		exit(0);
	}

    //doSimple();
    //doMovie();
    //do1();
    //doRadialdist();
    //doScannerEffect();
    //doFrameBlendFilter();
    //printf("hello patato\n");
    //doCrayon();
    //doKeyingFilter();
    //doSave();
    doDoubleCapture(v1,v2,fps);

    return 0;
}


