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

// pour le usleep
#include <unistd.h>
#include <string.h>

#include <imgu.h>

//#define SAVE_SEQUENCE
//#define SAVE_SINGLE_SEQUENCE
//#define LOAD_SEQUENCE
#define LOAD_SINGLE_SEQUENCE

int main( int argc, char **argv )
{
int i,k;
int tid_camera,tid_save,tid_save2,tid_view,tid_view2,tid_view3;
rqueue *Qrecycle,*Qblub;

	//
	// Create a few queues.
	//
	// recycle -> [gige] -> a -> [save] -> b -> [view]
	//
	Qrecycle=imguRegisterQueue("recycle");
	Qblub=imguRegisterQueue("blub");
	imguRegisterQueue("a");
	imguRegisterQueue("b");

	imgu *IA=NULL;
	for(i=0;i<80;i++) RQueueAddLast(Qrecycle,(void *)&IA);

	// USAGE "-in free -out q -viewnum 0 -width 256 -height 256 -channels [1|2|3|4] -fps 0"

#ifdef SAVE_SEQUENCE
	tid_camera=imguStartPlugin("pattern","camera",
		"-in recycle -out a -width 256 -height 128 -channels 3 -fps 30 -viewnum 0");
	if( tid_camera<0 ) exit(-1);

	tid_save=imguStartPlugin("save","sink","-in a -out b -filename blub%03d.png -keynum FRAMENUM");
	if( tid_save<0 ) exit(-1);
#endif

#ifdef SAVE_SINGLE_SEQUENCE
	tid_camera=imguStartPlugin("pattern","camera",
		"-in recycle -out a -width 256 -height 128 -channels 3 -fps 30 -viewnum 0");
	if( tid_camera<0 ) exit(-1);

	tid_save=imguStartPlugin("save","sink","-in a -out b -singlefilename allblub.png");
	if( tid_save<0 ) exit(-1);
#endif

#ifdef LOAD_SEQUENCE
	tid_camera=imguStartPlugin("load","camera",
		"-in recycle -out b -filename blub%03d.png -nb 100000 -fps 4 -viewnum 0");
	if( tid_camera<0 ) exit(-1);
#endif

#ifdef LOAD_SINGLE_SEQUENCE
	tid_camera=imguStartPlugin("load","camera",
		"-in recycle -out b -singlefilename allblub.png -multi 5 -fps 1 -viewnum 0");
	if( tid_camera<0 ) exit(-1);
#endif

//
// one or three viewers... 3 viewer crash on Apple.
//

	tid_view=imguStartPlugin("viewer","sink","-in b -pos 100 100 100 100 false -view0 cam0 0 0 1 1");
	if( tid_view<0 ) exit(-1);

#ifdef MULTI_VIEW
	tid_view2=imguStartPlugin("viewer","sink","-in b -pos 250 100 100 100 false -view0 cam0 0 0 1 1");
	if( tid_view2<0 ) exit(-1);

	tid_view3=imguStartPlugin("viewer","sink","-in b -pos 400 100 100 100 false -view0 cam0 0 0 1 1");
	if( tid_view3<0 ) exit(-1);

	paramlist *pl_view2=imguGetPluginParameters(tid_view2);
	paramlist *pl_view3=imguGetPluginParameters(tid_view3);
#endif

	paramlist *pl_camera=imguGetPluginParameters(tid_camera);
	paramlist *pl_view=imguGetPluginParameters(tid_view);

	//paramDump(pl_camera);

	//paramInvokeCommand(pl_camera,"START");

	//
	// main loop!
	//

	//
	// create a couple of empty images for the plugin to use
	// The camera plugin will block when the queue is empty, waiting for some recycling.
	//
//	for(i=0;i<10;i++) RQueueAddLast(Qblub,(void *)&IA);

	//
	// wait for the viewer to tell us we are done.
	//
	//

	int fini=0;
	for(;;) {
		char cmd[150];
		printf("----------\n");
		printf("start\n");
		printf("stop\n");
		printf("!  (snapshot)\n");
		printf("i [gige parameter] [<int>]\n");
		printf("d [gige parameter] [<double>]\n");
		printf("s [gige parameter] [<string>]\n");
		printf("quit\n");
		printf("----------\n");
		printf("CMD> ");
		fflush(stdout);
		do {
			i=scanf(" %[^\n]",cmd);

			paramGetInt(pl_view,"done",&k);
			if( k ) { fini=1;printf("DONEDONE!\n");break; }

#ifdef MULTI_VIEW
			paramGetInt(pl_view2,"done",&k);
			if( k ) { fini=1;printf("DONEDONE!\n");break; }

			paramGetInt(pl_view3,"done",&k);
			if( k ) { fini=1;printf("DONEDONE!\n");break; }
#endif
		} while( i<1 && !fini );

		if( fini ) break;

		printf("Vous avez tape '%s'\n",cmd);

		if( strcmp(cmd,"start")==0 ) {
			printf("START! time=%f\n",getTimeNow());
			paramInvokeCommand(pl_camera,"START");
		}else if( strcmp(cmd,"stop")==0 ) {
			printf("STOP!\n");
			paramInvokeCommand(pl_camera,"STOP");
		}else if( strcmp(cmd,"!")==0 ) {
			paramInvokeCommand(pl_camera,"SNAPSHOT");
		}else if( strncmp(cmd,"i",1)==0 ) {
			int k,val,err;
			char param[100];
			k=sscanf(cmd,"i %s %d",param,&val);
			if( k==1 ) {
				err=paramGetInt(pl_camera,param,&val);
				printf("%s is %d (err=%d)!\n",param,val,err);
			}else if( k==2 ) {
				err=paramSetInt(pl_camera,param,val);
				printf("%s is %d (err=%d)!\n",param,val,err);
			}else{ printf("???????????????\n"); }
		}else if( strncmp(cmd,"d",1)==0 ) {
			int k,err;
			double val;
			char param[100];
			k=sscanf(cmd,"d %s %lf",param,&val);
			if( k==1 ) {
				err=paramGetDouble(pl_camera,param,&val);
				printf("%s is %f (err=%d)!\n",param,val,err);
			}else if( k==2 ) {
				err=paramSetDouble(pl_camera,param,val);
				printf("%s is %f (err=%d)!\n",param,val,err);
			}else{ printf("???????????????\n"); }
		}else if( strncmp(cmd,"s",1)==0 ) {
			int k,err;
			char val[512];
			char param[100];
			k=sscanf(cmd,"s %s %s",param,val);
			if( k==1 ) {
				err= (paramGetString(pl_camera,param,val,512)!=val);
				printf("%s is '%s' err=%d!\n",param,val,err);
			}else if( k==2 ) {
				err=paramSetString(pl_camera,param,val);
				printf("%s is '%s' (err=%d)!\n",param,val,err);
			}else{ printf("???????????????\n"); }
		}else if( strcmp(cmd,"dump")==0 ) {
			imguPluginDump();
		}else if( strcmp(cmd,"quit")==0 ) {
			break;
		}
/***
		// wait random time 0 to 2sec
		usleep(random()%2000000);

		printf("START time %f\n",getTimeNow());
		paramInvokeCommand(pl_camera,"START");

		sleep(3);

		printf("STOP\n");
		paramInvokeCommand(pl_camera,"STOP");

		sleep(1);
****/
		// timing test
		//paramGetInt(pl_view,"done",&k);
		//if( k ) { fini=1;printf("DONEDONE!\n");break; }
		
	}

	//
	// Output some statistics
	//
	imguPluginDumpDot("out.dot");

	//
	// we are done!!!
	//

	imguStopPlugin(tid_view);
#ifdef MULTI_VIEW
	imguStopPlugin(tid_view2);
	imguStopPlugin(tid_view3);
#endif

	imguStopPlugin(tid_camera);
	//imguStopPlugin(tid_movie);

#ifdef SAVE_SEQUENCE
imguStopPlugin(tid_save);
#endif
#ifdef SAVE_SINGLE_SEQUENCE
imguStopPlugin(tid_save);
#endif

	//imguUnregisterQueue("a",NULL);
	//imguUnregisterQueue("b",NULL);
	//imguUnregisterQueue("recycle",NULL);

	return(0);
}

