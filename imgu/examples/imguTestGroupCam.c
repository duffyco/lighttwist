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
#include <string.h>

#include <imgu.h>

#define NBCAM	3

int main( int argc, char **argv )
{
int i,k;
int tid_camera[NBCAM],tid_view,tid_group;
rqueue *Qrecycle,*Qgroup;
imgu *IB;

	//
	// Create a few queues.
	//
	// recycle -> [gige] -> a -> [save] -> b -> [view]
	//
	Qrecycle=imguRegisterQueue("recycle");
	imguRegisterQueue("a");
	imguRegisterQueue("b");
	imguRegisterQueue("c");
	imguRegisterQueue("d");
	Qgroup=imguRegisterQueue("g");

	imguBeginUseQueue("g",FOR_READING);

	//
	// create a couple of empty images for the plugin to use
	// The camera plugin will block when the queue is empty, waiting for some recycling.
	//
	imgu *IA=NULL;
	for(i=0;i<100;i++) RQueueAddLast(Qrecycle,(void *)&IA);


	tid_group=imguStartPlugin("group","filter","-in d -out g -nb 3 -tolerance 0.05");
	if( tid_group<0 ) { printf("group plugin returned %d\n",tid_group);exit(-1); }

        /*
	tid_camera[0]=imguStartPlugin("gige","camera",
		"-in recycle -out c -camid 0 -fps 1 -viewnum 0");
	if( tid_camera[0]<0 ) exit(-1);

	tid_camera[1]=imguStartPlugin("gige","camera",
		"-in recycle -out c -camid 1 -fps 1 -viewnum 1");
	if( tid_camera[1]<0 ) exit(-1);
        */

        tid_camera[0]=imguStartPlugin("ffmpeg","camera",
                                      "-in recycle -out c -file http://192.168.2.118/vdata.v -format nc -viewnum 0");
        if( tid_camera[0]<0 ) exit(-1);

        tid_camera[1]=imguStartPlugin("ffmpeg","camera",
                                      "-in recycle -out c -file http://192.168.2.119/vdata.v -format nc -viewnum 1");
        if( tid_camera[1]<0 ) exit(-1);

        tid_camera[2]=imguStartPlugin("ffmpeg","camera",
                                      "-in recycle -out c -file http://192.168.2.114/vdata.v -format nc -viewnum 2");
        if( tid_camera[2]<0 ) exit(-1);


/*
	tid_save1=imguStartPlugin("save","sink","-in a -out c -filename left%03d.png -keynum FRAMENUM");
	if( tid_save1<0 ) exit(-1);
	tid_save2=imguStartPlugin("save","sink","-in b -out c -filename right%03d.png -keynum FRAMENUM");
	if( tid_save2<0 ) exit(-1);
*/

	//tid_view=imguStartPlugin("viewer","sink","-in a -view0 cam0 0 0 1 0.5 -view1 cam1 0 0.5 1 1");
	tid_view=imguStartPlugin("viewer","sink","-in c -out d -view0 cam0 0 0 1 0.33 -view1 cam1 0 0.33 1 0.66 -view2 cam2 0 0.66 1 1");
	if( tid_view<0 ) exit(-1);



	paramlist *pl_camera[NBCAM];
	pl_camera[0]=imguGetPluginParameters(tid_camera[0]);
	pl_camera[1]=imguGetPluginParameters(tid_camera[1]);
	pl_camera[2]=imguGetPluginParameters(tid_camera[2]);
	paramlist *pl_view=imguGetPluginParameters(tid_view);


	int n;
	int done=0;
	for(n=0;;n++) {


		// attend un groupe

		IA=NULL;

		paramGetInt(pl_view,"done",&k);
		if( k ) { done=1;printf("DONEDONE!\n");break; }

		while( RQueueRemoveFirstWait(Qgroup,(void *)&IA) );

		if( done ) break;

#ifdef SAVE_IT
		// on fait un save multi la dessus... ye.
		char buf[100];
		sprintf(buf,"multi%03d.png",n);
		printf("saving- multi %s\n",buf);
		imguSaveMulti(IA,buf,1,SAVE_AS_IS);
#endif

		// on process...
		// ....
		printf("MAIN processing group %d\n",n);

		// on recycle
		while( IA ) {
			IB=IA->next;
			IA->next=NULL;
            imguRecycle(IA);
			IA=IB;
		}

	}

	imguEndUseQueue(NULL,Qgroup);


	//
	// Output some statistics
	//
	printf("Dot!!!!!!!\n");
	imguPluginDump();
	imguPluginDumpDot("out.dot");

	//
	// we are done!!!
	//
	imguStopPlugin(tid_group);
	imguStopPlugin(tid_camera[0]);
	imguStopPlugin(tid_camera[1]);
	imguStopPlugin(tid_camera[2]);
//	imguStopPlugin(tid_save1);
//	imguStopPlugin(tid_save2);
	imguStopPlugin(tid_view);

	imguUnregisterQueue("recycle",NULL);
	imguUnregisterQueue("a",NULL);
	imguUnregisterQueue("b",NULL);
	imguUnregisterQueue("c",NULL);
	imguUnregisterQueue("d",NULL);

	return(0);
}

