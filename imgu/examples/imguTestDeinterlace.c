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
// QrecycleCam -> [camera] -> Qa -> [deinterlace] -> Qb -> [view]
//


#include <imgu.h>
#include <unistd.h>

int main( int argc, char **argv )
{
int i,k;
int tid_camera,tid_crop,tid_deinter,tid_view,tid_color;
int tid_save1,tid_save2,tid_save3;
int tid_split,tid_merge;
int tid_convx,tid_convy,tid_convt;
rqueue *QrecycleCam;

	// the Qfree queue is local to us, for recycling.
	QrecycleCam		=imguRegisterQueue("recycleCam");
	imguRegisterQueue("a");
	imguRegisterQueue("b");
	imguRegisterQueue("c");
	imguRegisterQueue("d");
	imguRegisterQueue("dx");
	imguRegisterQueue("dy");
	imguRegisterQueue("dt");
	imguRegisterQueue("e");
	imguRegisterQueue("ex");
	imguRegisterQueue("ey");
	imguRegisterQueue("et");
	imguRegisterQueue("f");

	imguRegisterQueue("s1");
	imguRegisterQueue("s2");
	imguRegisterQueue("s3");


	imgu *IA=NULL;

	tid_camera=imguStartPlugin("ffmpeg","camera",
		"-in recycleCam -out a -file /home/roys/Bureau/MOV023.TOD -start 390 -end 1560");
		//"-in recycleCam -out a -file /home/roys/Bureau/Video/HDmovies/MOV077.TOD -start 100 -end 1500");
	if( tid_camera<0 ) exit(-1);

	tid_crop=imguStartPlugin("crop","filter","-in a -out b -crop 780 30 1382 860");
	if( tid_crop<0 ) exit(-1);

	tid_color=imguStartPlugin("color","filter","-in b -out c -to gray");
	if( tid_color<0 ) exit(-1);

	tid_deinter=imguStartPlugin("deinterlace","filter","-in c -out d -keep odd");
	if( tid_deinter<0 ) exit(-1);

	tid_split=imguStartPlugin("split","filter","-in d -out1 dx -out2 dy -out3 dt -op copy");
	if( tid_split<0 ) exit(-1);

	tid_convx=imguStartPlugin("convolve","filter","-in dx -out ex -xkernel dgauss -xstd 2.0 -ykernel gauss -ystd 1.0 -tkernel gauss -tstd 2.0 -k 0.9");
	if( tid_convx<0 ) exit(-1);
	tid_convy=imguStartPlugin("convolve","filter","-in dy -out ey -xkernel gauss -xstd 2.0 -ykernel dgauss -ystd 1.0 -tkernel gauss -tstd 2.0 -k 0.9");
	if( tid_convy<0 ) exit(-1);
	tid_convt=imguStartPlugin("convolve","filter","-in dt -out et -xkernel gauss -xstd 2.0 -ykernel gauss -ystd 1.0 -tkernel dgauss -tstd 2.0 -k 0.9");
	if( tid_convt<0 ) exit(-1);

	tid_merge=imguStartPlugin("merge","filter","-in1 ex -in2 ey -in3 et -out s1 -op rgb -syncframenum 1");
	if( tid_merge<0 ) exit(-1);

	tid_save1=imguStartPlugin("save","sink","-in s1 -out e -filename grad%04d.png -keynum FRAMENUM");
	if( tid_save1<0 ) exit(-1);
	tid_save2=imguStartPlugin("save","sink","-in s1 -out e -filename grad%04d.png -keynum FRAMENUM");
	if( tid_save2<0 ) exit(-1);
	tid_save3=imguStartPlugin("save","sink","-in s1 -out e -filename grad%04d.png -keynum FRAMENUM");
	if( tid_save3<0 ) exit(-1);


	tid_view=imguStartPlugin("viewer","sink","-in e");
	if( tid_view<0 ) exit(-1);

	paramlist *pl_view=imguGetPluginParameters(tid_view);
	paramlist *pl_convx=imguGetPluginParameters(tid_convx);
	paramlist *pl_convy=imguGetPluginParameters(tid_convy);
	paramlist *pl_convt=imguGetPluginParameters(tid_convt);

	//
	// All plugin started. Go!
	//

	// create a couple of empty images for the ffmpeg plugin to use
	// (otherwise he will wait for images)
	// MAKE SURE YOU HAVE ENOUGH TO COVER THE CONVOLVE IN T LAG!!
	IA=NULL;
	int xsz,ysz,tsz,sz;
	paramGetInt(pl_convx,"Tsize",&xsz);
	paramGetInt(pl_convy,"Tsize",&ysz);
	paramGetInt(pl_convt,"Tsize",&tsz);
	sz=xsz;
	if( ysz>sz ) sz=ysz;
	if( tsz>sz ) sz=tsz;
	printf("!!!!!!!!!!!!!!!!!!!! size = %d\n",sz);
	// we need at least tw images to account for the accumulation of images for Dt
	for(i=0;i<(sz+10);i++) RQueueAddLast(QrecycleCam,(void *)&IA);

	printf("added images to QrecycleCam\n");


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
	imguStopPlugin(tid_deinter);
	imguStopPlugin(tid_split);
	imguStopPlugin(tid_convx);
	imguStopPlugin(tid_convy);
	imguStopPlugin(tid_convt);
	imguStopPlugin(tid_merge);
	imguStopPlugin(tid_save1);
	imguStopPlugin(tid_save2);
	imguStopPlugin(tid_save3);
	imguStopPlugin(tid_view);


	// Normalement, il faut vider les queues....
	imguUnregisterQueue("recycleCam",NULL);
	imguUnregisterQueue("a",NULL);
	imguUnregisterQueue("b",NULL);
	imguUnregisterQueue("c",NULL);
	imguUnregisterQueue("d",NULL);
	imguUnregisterQueue("dx",NULL);
	imguUnregisterQueue("dy",NULL);
	imguUnregisterQueue("dt",NULL);
	imguUnregisterQueue("e",NULL);
	imguUnregisterQueue("ex",NULL);
	imguUnregisterQueue("ey",NULL);
	imguUnregisterQueue("et",NULL);
	imguUnregisterQueue("f",NULL);
	imguUnregisterQueue("s1",NULL);
	imguUnregisterQueue("s2",NULL);
	imguUnregisterQueue("s3",NULL);

    return 0;
}

