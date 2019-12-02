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
 * Grab 3 gige cameras with snapshot command
 */
#include <string.h>

#include <sys/time.h>
#include <time.h>

#include <unistd.h>

#include <imgu.h>

#define NBCAM	3

int main( int argc, char **argv )
{
int i,k;
int tid_camera[NBCAM],tid_save[NBCAM],tid_view;
rqueue *Qrecycle;
paramlist *pl_camera[NBCAM];
char buf[256];
imgu *IB;
int fps;

fps=15;

	for(i=1;i<argc;i++) {
        if( strcmp("-fps",argv[i])==0 && i+1<argc ) {
			fps=atoi(argv[i+1]);
			i++;continue;
		}
    }

	//
	// Create a few queues.
	//
	// recycle -> cam[] -> save -> view
	//
	Qrecycle=imguRegisterQueue("recycle");
	imguRegisterQueue("view");

	//
	// create a couple of empty images for the plugin to use
	// The camera plugin will block when the queue is empty, waiting for some recycling.
	//
	imgu *IA=NULL;
	for(i=0;i<100;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    int cid;
    for (i=0;i<NBCAM;i++)
    {
      sprintf(buf,"cam%d",i);
      imguRegisterQueue(buf);

      if (i==0) cid=36134;
      else if (i==1) cid=36133;
      else cid=107245;
      cid=i;

      sprintf(buf,"-in recycle -out cam%d -camid %d -fps 0 -viewnum %d",i,cid,i);
  	  tid_camera[i]=imguStartPlugin("gige","camera",buf);
	  if( tid_camera[i]<0 ) exit(-1);

      pl_camera[i]=imguGetPluginParameters(tid_camera[i]);

      sprintf(buf,"-in cam%d -out view -filename frame_%d_%%05d.png -keynum FRAMENUM",i,i);
	  tid_save[i]=imguStartPlugin("save","sink",buf);
	  if( tid_save[i]<0 ) exit(-1);
    }

    tid_view=imguStartPlugin("viewer","sink","-in view -view0 cam0 0 0 1 0.33 -view1 cam1 0 0.33 1 0.66 -view2 cam2 0 0.66 1 1");
	if( tid_view<0 ) exit(-1);

    paramDump(pl_camera[0]);

    for (i=0;i<NBCAM;i++)
    {
      paramInvokeCommand(pl_camera[i],"START");
      //paramSetString(pl_camera,"ExposureMode","Manual");
      //paramSetInt(pl_camera,"ExposureValue",0.150*1000000);
    }

    int frame_dt,usleep_time;
    struct timeval tv;
  	frame_dt=1000000/fps; //usec    
    k=0;
    while(1)
    {
      for (i=0;i<NBCAM;i++)
      {
        paramInvokeCommand(pl_camera[i],"SNAPSHOT");
      }
   	  gettimeofday(&tv,NULL);
	  usleep_time=frame_dt-tv.tv_usec%frame_dt;
      if (usleep_time>100 ) usleep(usleep_time);
      //printf("%06ld\n",(long int)(tv.tv_usec));
      k++;
    }

    for (i=0;i<NBCAM;i++)
    {
      paramInvokeCommand(pl_camera[i],"STOP");
    }

    //
	// we are done!!!
	//
    for (i=0;i<NBCAM;i++)
    {
  	  imguStopPlugin(tid_camera[i]);
  	  imguStopPlugin(tid_save[i]);
  	  imguStopPlugin(tid_view);
    }
    for (i=0;i<NBCAM;i++)
    {
  	  imguUnregisterQueue("recycle",NULL);
  	  imguUnregisterQueue("view",NULL);
      sprintf(buf,"cam%d",i);
      imguUnregisterQueue(buf,NULL);
    }

	return(0);
}

