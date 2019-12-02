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

#include <imgu.h>
#include <pt.h>

int computeLuminance(imgu *I, double *value, double *norm, double *out_max, double *out_sum) {
	int i, nbpix=1;
	double sum=0;
	double max=0;
	for (i=0 ; i<I->xs*I->ys*I->cs ; i++) {
		if (I->data[i]>5000) {
			sum += I->data[i];
			if (I->data[i]>max) max=I->data[i];
			nbpix++;
		}
	}
	printf("LUMINANCE ------------> sum=%f \n",sum/nbpix);

	*value = sum;
	*out_max = max;
	*norm = sum/nbpix;
	*out_sum = nbpix;
	return 1;
}

int main( int argc, char **argv )
{
int i,k;
int tid_camera,tid_movie,tid_save,tid_view;
rqueue *Qrecycle;

	//
	// Create a few queues.
	//
	// recycle -> [gige] -> a -> [save] -> b -> [view]
	//
	Qrecycle=imguRegisterQueue("recycle");
	rqueue *QA = imguRegisterQueue("a");
	rqueue *QB = imguRegisterQueue("b");
	rqueue *QC = imguRegisterQueue("c");

	tid_camera=imguStartPlugin("gige","camera",
							   "-in recycle -out a -camid 36134 -fps 0 -viewnum 0");

	if( tid_camera<0 ) exit(-1);
	/*
	tid_movie=imguStartPlugin("ffmpeg","camera",
		"-in blub -out a -viewnum 1 -file /home/roys/Vidéos/AstroViz__Solar_Storms[SPRP00000160].mkv -fps 20");
	if( tid_movie<0 ) exit(-1);
	*/
	tid_save=imguStartPlugin("save","sink","-in c -filename wiipen%03d.png -keynum FRAMENUM");
	if( tid_save<0 ) exit(-1);

	//tid_view=imguStartPlugin("viewer","sink","-in a -view0 cam0 0 0 1 0.5 -view1 cam1 0 0.5 1 1");
	tid_view=imguStartPlugin("viewer","sink","-in b -out c -view0 cam0 0 0 1 1");
	if( tid_view<0 ) exit(-1);

	paramlist *pl_camera=imguGetPluginParameters(tid_camera);
	paramlist *pl_view=imguGetPluginParameters(tid_view);


	//
	// main loop!
	//

	//
	// create a couple of empty images for the plugin to use
	// The camera plugin will block when the queue is empty, waiting for some recycling.
	//
	imgu *IA=NULL;
	for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

	//
	// wait for the viewer to tell us we are done.
	//
	//
	sleep(1); // laisser le temps aux cameras de partir

	int fini=0;
	for(;;) {

		char cmd[150];
		printf("----------\n");
		printf("start\n");
		printf("stop\n");
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
			paramInvokeCommand(pl_camera,"START");
			
		} else if( strcmp(cmd,"scan")==0 ) {
			// data file
			FILE *data_lum;
			data_lum = fopen("./luminance.txt","w");
	
			int pt = pt_init(NULL);
			int incr = 500;
			pt_sendcmd_int(pt, "ps", 2500);
			pt_waitloop(pt, "pp", -10000);
			pt_sendcmd_int(pt, "ps", 500);
			int s;
			for (s=-10000 ; s<=10000 ; s+=incr) {
				
				printf("AT PAN: %i\n", s);

				pt_waitloop(pt, "pp", s);
				paramInvokeCommand(pl_camera,"SNAPSHOT");
				while (RQueueRemoveFirstWait(QA,(void*)&IA)) ;
				double val,max,norm,cx,cy,sum;
				computeLuminance(IA, &val,&norm,&max,&sum);
				fprintf(data_lum,"%i %f %f %f %f %f\n",s,P2A(s),val,norm,max,sum);
				RQueueAddLast(QB,(void*)&IA);
			} 
			fclose(data_lum);

		}else if( strcmp(cmd,"stop")==0 ) {
			printf("STOP!\n");
			paramInvokeCommand(pl_camera,"STOP");
		}else if( strcmp(cmd,"!")==0 ) {
			paramInvokeCommand(pl_camera,"SNAPSHOT");
			while (RQueueRemoveFirstWait(QA,(void*)&IA)) ;
			RQueueAddLast(QB,(void*)&IA);
		}else if( strncmp(cmd,"exposure",8)==0 ) {
			int exp=atoi(cmd+8);
			printf("Exposure %d!\n",exp);
			paramSetInt(pl_camera,"ExposureValue",exp);
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
	imguStopPlugin(tid_camera);
	//imguStopPlugin(tid_movie);
	imguStopPlugin(tid_save);
	imguStopPlugin(tid_view);

	imguUnregisterQueue("recycle",NULL);
	imguUnregisterQueue("a",NULL);
	imguUnregisterQueue("b",NULL);
}

