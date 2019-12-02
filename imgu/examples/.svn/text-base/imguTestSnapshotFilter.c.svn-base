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
int tid_snap=-1;
int tid_view=-1;
int tid_viewnum=-1;
rqueue *Qrecycle,*Qin,*Qout,*Qsnap;
	//
	//
	// recycle -> [v4l2] -> incam -> [snapshot] -> out -> [viewer]
	//                     inproj -> [snapshot] -> out -> [viewer]
	//                               [snapshot] -> snap
	//
	Qrecycle=imguRegisterQueue("recycle");
	Qin=imguRegisterQueue("cam");
	Qout=imguRegisterQueue("out");
	Qsnap=imguRegisterQueue("snap");

	//
	// give a few images to the camera
	//
	imgu *IA=NULL;
	for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

	tid_camera=imguStartPlugin("v4l2","camera","-in recycle -out cam -fps 5 -viewnum 0");
	if( tid_camera<0 ) exit(-1);

	//tid_snap=imguStartPlugin("snapshot","filter","-in cam -out out -outsnap snap -delay 1.0");
	tid_snap=imguStartPlugin("snapshot","filter","-in cam -outsnap snap -delay 1.0");
	if( tid_snap<0 ) exit(-1);

	tid_viewnum=imguStartPlugin("setviewnum","filter","-in snap -out out -viewnum 1");
	if( tid_viewnum<0 ) exit(-1);


	tid_view=imguStartPlugin("viewer","sink","-in out -view0 camera 0 0 1 0.5 -view1 snap 0 0.5 1 1");
	if( tid_view<0 ) exit(-1);


	paramlist *pl_snap=imguGetPluginParameters(tid_snap);
	paramlist *pl_view=imguGetPluginParameters(tid_view);


	int fini=0;
	for(;;) {
		char cmd[150];
		printf("----------\n");
		printf("!  (snapshot)\n");
		printf("delay <double>\n");
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

		if( strcmp(cmd,"!")==0 ) {
			printf("snap!\n");
			paramInvokeCommand(pl_snap,"Snapshot");
		}else if( strncmp(cmd,"delay",5)==0 ) {
			int k,err;
			double val;
			k=sscanf(cmd,"delay %lf",&val);
			if( k==0 ) {
				err=paramGetDouble(pl_snap,"Delay",&val);
				printf("delay is %f (err=%d)!\n",val,err);
			}else if( k==1 ) {
				err=paramSetDouble(pl_snap,"Delay",val);
				printf("delay is %f (err=%d)!\n",val,err);
			}else{ printf("???????????????\n"); }
		}else if( strcmp(cmd,"quit")==0 ) {
			break;
		}

		// timing test
		if( paramGetInt(pl_view,"done",&k)==0 && k ) {
			fini=1;printf("DONEDONE!\n");break;
		}
		
	}

	//
	// Output some statistics
	//
	imguPluginDumpDot("out.dot");

	//
	// we are done!!!
	//
	imguStopPlugin(tid_camera);
	imguStopPlugin(tid_snap);
	imguStopPlugin(tid_view);
	imguStopPlugin(tid_viewnum);


	return(0);
}

