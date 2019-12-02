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
 * Test multiple cameras
 *
 * if -camid is <30, then it is assumed it is the order... 0,1,2,3,4,5...
 * (cameras are sorted by id number)
 *
 */

#include <imgu.h>
#include <string.h>

#define NBCAM	4

#define NBSAVE	3

int camId[5] = { 13136,13137,13138,13139 };

int main( int argc, char **argv )
{
int i,k;
int tid_camera[NBCAM],tid_save[NBSAVE],tid_view,tid_group,tid_save0;
rqueue *Qrecycle;
int sync;
double fps;
int save;
int exposure;

	
	fps=20.0;
	sync=0;
	save=0;
	exposure=11000; // en micro secondes. pour 10fps, <100000, pour 20fps, <50000

	for(i=1;i<argc;i++) {
		if( strcmp("-sync",argv[i])==0 ) { sync=1;continue; }
		if( strcmp("-save",argv[i])==0 ) { save=1;continue; }
		if( strcmp("-expo",argv[i])==0 && i+1<argc ) { exposure=atoi(argv[i+1]);i++;continue; }
		if( strcmp("-fps",argv[i])==0 && i+1<argc ) { fps=atof(argv[i+1]);i++;continue; }
	}

	if( sync ) fps=0; // software sync!!

	//
	// Create a few queues.
	//
	// recycle -> [gige] -> a -> [save] -> b -> [view]
	//
	Qrecycle=imguRegisterQueue("recycle");
	imguRegisterQueue("a");
	imguRegisterQueue("aa");
	imguRegisterQueue("b");
	imguRegisterQueue("c");


	//
	// create a couple of empty images for the plugin to use
	// The camera plugin will block when the queue is empty, waiting for some recycling.
	//
	imgu *IA=NULL;
	for(i=0;i<1200;i++) RQueueAddLast(Qrecycle,(void *)&IA);


	tid_group=imguStartPlugin("group","filter","-in a -out b -nb 4 -tolerance 0.08");
	if( tid_group<0 ) exit(-1);

	for(i=0;i<NBSAVE;i++) {
		//tid_save[i]=imguStartPlugin("save","sink","-in c -compression 1 -filename blub%04d.png -keynum FRAMENUM");
		tid_save[i]=imguStartPlugin("save","sink","-in c -compression 1 -filename blub%04d_%d.png -keynum FRAMENUM -viewnum VIEWNUM");
		if( tid_save[i]<0 ) exit(-1);
	}

	//tid_save0=imguStartPlugin("save","sink","-in a -out aa -filename zap%04d.png");
	//if( tid_save<0 ) exit(-1);


	if( save ) {
		tid_view=imguStartPlugin("viewer","sink","-in b -out c -view0 cam0 0 0 0.5 0.5 -view1 cam1 0.5 0 1 0.5 -view2 cam2 0 0.5 0.5 1 -view3 cam3 0.5 0.5 1 1");
	}else{
		tid_view=imguStartPlugin("viewer","sink","-in b -view0 cam0 0 0 0.5 0.5 -view1 cam1 0.5 0 1 0.5 -view2 cam2 0 0.5 0.5 1 -view3 cam3 0.5 0.5 1 1");
	}
	if( tid_view<0 ) exit(-1);


	//	sleep(5);

	for(i=0;i<NBCAM;i++) {
		char blub[100];
		sprintf(blub,"-in recycle -out a -camid %d -fps %f -viewnum %d",i,fps,i);
		tid_camera[i]=imguStartPlugin("gige","camera",blub);
		if( tid_camera[i]<0 ) { printf("Unable to start camera %d id=%d\n",i,camId[i]);exit(0); }
	}


/***
	tid_camera[0]=imguStartPlugin("ffmpeg","camera",
		"-in recycle -out c -file http://192.168.2.118/vdata.v -format nc -viewnum 0");
	if( tid_camera[0]<0 ) exit(-1);

	tid_camera[1]=imguStartPlugin("ffmpeg","camera",
		"-in recycle -out c -file http://192.168.2.119/vdata.v -format nc -viewnum 1");
	if( tid_camera[1]<0 ) exit(-1);

	tid_camera[2]=imguStartPlugin("ffmpeg","camera",
		"-in recycle -out c -file http://192.168.2.114/vdata.v -format nc -viewnum 2");
	if( tid_camera[2]<0 ) exit(-1);
***/


        /**
           tid_camera[0]=imguStartPlugin("gige","camera",
           "-in recycle -out a -camid 0 -fps 1 -viewnum 0");
           if( tid_camera[0]<0 ) exit(-1);

           tid_camera[1]=imguStartPlugin("gige","camera",
           "-in recycle -out b -camid 1 -fps 1 -viewnum 1");
           if( tid_camera[1]<0 ) exit(-1);
        **/


        /**
           tid_save1=imguStartPlugin("save","sink","-in a -out c -filename left%03d.png -keynum FRAMENUM");
           if( tid_save1<0 ) exit(-1);
           tid_save2=imguStartPlugin("save","sink","-in b -out c -filename right%03d.png -keynum FRAMENUM");
           if( tid_save2<0 ) exit(-1);
        */



	paramlist *pl_camera[NBCAM];
	for(i=0;i<NBCAM;i++) {
		pl_camera[i]=imguGetPluginParameters(tid_camera[i]);
	}
	paramlist *pl_save[NBSAVE];
	for(i=0;i<NBSAVE;i++) {
		pl_save[i]=imguGetPluginParameters(tid_save[i]);
	}
	paramlist *pl_view=imguGetPluginParameters(tid_view);

	//
	// main loop!
	//

	// set certains parametres...
	//
	// c'est a peu pres 1MB/image, donc 15fps -> 15MB/sec
	//
	for(i=0;i<NBCAM;i++) {
		paramSetInt(pl_camera[i],"StreamBytesPerSecond",20000000);
	}

	for(i=0;i<NBCAM;i++) {
		double v;
		//paramSetDouble(pl_camera[i],"FrameRate",1.0);
		paramGetDouble(pl_camera[i],"FrameRate",&v);
		printf("FrameRate is now %f\n",v);
	}

	// ajuste l'exposure
	printf("Exposure set to %d!\n",exposure);
	for(i=0;i<NBCAM;i++) paramSetInt(pl_camera[i],"ExposureValue",exposure);


	//
	// wait for the viewer to tell us we are done.
	//

	int fini=0;
	for(;;) {

		char cmd[150];
		printf("----------\n");
		printf("start\n");
		printf("stop\n");
		printf("savestart\n");
		printf("savestop\n");
		printf("!  (snapshot)\n");
		printf("exposure <int>\n");
		printf("quit\n");
		printf("----------\n");
		printf("CMD> ");
		fflush(stdout);
		do {
			i=scanf(" %[^\n]",cmd);

			paramGetInt(pl_view,"done",&k);
			if( k ) { fini=1;printf("DONEDONE!\n");break; }
		} while( i<1 && !fini );

		if( fini ) break;

		printf("Vous avez tape '%s'\n",cmd);

		if( strcmp(cmd,"start")==0 ) {
			printf("START!\n");
			for(i=0;i<NBCAM;i++) paramInvokeCommand(pl_camera[i],"START");
		}else if( strcmp(cmd,"stop")==0 ) {
			printf("START SAVING!\n");
			for(i=0;i<NBSAVE;i++) paramInvokeCommand(pl_save[i],"START");
			printf("STOP!\n");
			for(i=0;i<NBCAM;i++) paramInvokeCommand(pl_camera[i],"STOP");
		}else if( strcmp(cmd,"savestart")==0 ) {
			printf("START SAVING!\n");
			for(i=0;i<NBSAVE;i++) paramInvokeCommand(pl_save[i],"START");
		}else if( strcmp(cmd,"savestop")==0 ) {
			printf("STOP SAVING!\n");
			for(i=0;i<NBSAVE;i++) paramInvokeCommand(pl_save[i],"STOP");
		}else if( strcmp(cmd,"!")==0 ) {
			for(i=0;i<NBCAM;i++) paramInvokeCommand(pl_camera[i],"SNAPSHOT");
		}else if( strncmp(cmd,"exposure",8)==0 ) {
			int exp=atoi(cmd+8);
			printf("Exposure %d!\n",exp);
			for(i=0;i<NBCAM;i++) paramSetInt(pl_camera[i],"ExposureValue",exp);
		}else if( strcmp(cmd,"dump")==0 ) {
			imguPluginDump();
		}else if( strcmp(cmd,"quit")==0 ) {
			break;
		}
	}

	//
	// Output some statistics
	//
	imguPluginDumpDot("out.dot");

	//
	// we are done!!!
	//
	for(i=0;i<NBCAM;i++) {
		imguStopPlugin(tid_camera[i]);
	}
	for(i=0;i<NBSAVE;i++) {
		imguStopPlugin(tid_save[i]);
	}
	//imguStopPlugin(tid_save0);
	imguStopPlugin(tid_view);

	imguUnregisterQueue("recycle",NULL);
	imguUnregisterQueue("a",NULL);
	imguUnregisterQueue("aa",NULL);
	imguUnregisterQueue("b",NULL);
	imguUnregisterQueue("c",NULL);

    return 0;
}

