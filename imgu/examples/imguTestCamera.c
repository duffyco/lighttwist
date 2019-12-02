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


int main( int argc, char **argv )
{
int i,k;
int tid_camera=-1;
int tid_save=-1;
int tid_view=-1;
rqueue *Qrecycle,*Qblub;
int dosave;
int camid;


	dosave=0;
	camid=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-save",argv[i])==0 ) { dosave=1;continue; }
		if( strcmp("-camid",argv[i])==0 && i+1<argc ) { camid=atoi(argv[i+1]);i++;continue; }
	}


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

/*
	tid_camera=imguStartPlugin("ffmpeg","camera",
		//"-in recycle -out a -file blub.nc -format nc -viewnum 0");
		"-in recycle -out a -file http://192.168.2.114/vdata.v -format nc -viewnum 0");
	if( tid_camera<0 ) exit(-1);
*/

	char blub[100];

	sprintf(blub,"-in recycle -out a -camid %d -fps 10 -viewnum 0",camid);
	tid_camera=imguStartPlugin("gige","camera",blub);
	if( tid_camera<0 ) exit(-1);

/**
	tid_movie=imguStartPlugin("ffmpeg","camera",
		"-in blub -out a -viewnum 1 -file /home/roys/Vidéos/AstroViz__Solar_Storms[SPRP00000160].mkv -fps 20");
	if( tid_movie<0 ) exit(-1);
**/


	if( dosave ) {
		tid_save=imguStartPlugin("save","sink","-in a -out b -filename blub%03d.png -keynum FRAMENUM");
		if( tid_save<0 ) exit(-1);
		tid_view=imguStartPlugin("viewer","sink","-in b -view0 cam0 0 0 1 1");
		if( tid_view<0 ) exit(-1);
	}else{
		tid_view=imguStartPlugin("viewer","sink","-in a -view0 cam0 0 0 1 1");
		if( tid_view<0 ) exit(-1);
	}

	//tid_view=imguStartPlugin("viewer","sink","-in a -view0 cam0 0 0 1 0.5 -view1 cam1 0 0.5 1 1");

	paramlist *pl_camera=imguGetPluginParameters(tid_camera);
	paramlist *pl_view=imguGetPluginParameters(tid_view);

	paramDump(pl_camera);

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
#ifndef SKIP
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

			if( paramGetInt(pl_view,"done",&k)==0 && k ) {
				fini=1;printf("DONEDONE!\n");break;
			}
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
#else
		// wait random time 0 to 2sec
		usleep(random()%2000000);

		printf("START time %f\n",getTimeNow());
		paramInvokeCommand(pl_camera,"START");

		sleep(3);

		printf("STOP\n");
		paramInvokeCommand(pl_camera,"STOP");

		sleep(1);
#endif
		// timing test
		if( paramGetInt(pl_view,"done",&k)==0 && k ) { fini=1;printf("DONEDONE!\n");break; }
		
	}

	//
	// Output some statistics
	//
	imguPluginDumpDot("out.dot");

	//
	// we are done!!!
	//
	imguStopPlugin(tid_view);
	imguStopPlugin(tid_camera);
	//imguStopPlugin(tid_movie);
	imguStopPlugin(tid_save);

	//imguUnregisterQueue("a",NULL);
	//imguUnregisterQueue("b",NULL);
	//imguUnregisterQueue("recycle",NULL);

	return(0);
}

