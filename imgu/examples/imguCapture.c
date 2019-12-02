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
 * @file
 * Test of the generic capture functions
 *
 */

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <imgu.h>
#include <math.h>

#define STR_LEN 255

int DoAbort=0;
int NbCapImgs=0;

// on recoit le tableau I et l'image i est disponible.
// On doit retourner 0 si rien a faire,
// sinon 1 pour remettre en file
// ou encore -1 pour tout arreter (vide la queue)
// Si on veut, on peut prendre le pointeur et remettre I[i] a NULL
// alors une nouvelle image sera reservee.
// err est a 0 si pas d'erreur, sinon il y a un code d'erreur 
int callback(capture *cap,imgu **I,int i,int err)
{
int sum;
	printf("T :Got frame %d (err=%d)!\n",i,err);
	sum=0;
	//for(j=0;j<I[i]->xs*I[i]->ys/20;j++) sum+=I[i]->data[j];
	//printf("bonjour! image %3d received! sum=%10d\n",i,sum);

    NbCapImgs++;

	return(0); // ne pas remettre dans la file
	return(1); // remettre dans la file
}


// CTRL-C handler
void CtrlCHandler(int Signo)
{
    printf("interrupting!\n");

    DoAbort=1;
    signal(SIGINT, CtrlCHandler);
}


int main(int argc,char *argv[])
{
int i,single_frame;
char device[30];
int N;
int k;
char outFilename[STR_LEN];
char **camNames;
capture cap; // capture control structure

    outFilename[0]='\0';
    N=0;
    single_frame=0;
    strcpy(device,DEVICE_V4L);
    for(i=1;i<argc;i++) {
      if( strcmp("-gige",argv[i])==0) {
          strcpy(device,DEVICE_GIGE);
          continue;
      }
      if( strcmp("-v4l",argv[i])==0) {
          strcpy(device,DEVICE_V4L);
          continue;
      }
      if( strcmp("-single",argv[i])==0) {
          single_frame++;
          continue;
      }
      if( strcmp("-multiple",argv[i])==0 && i+1<argc) {
          N=atoi(argv[i+1]);
          i++;
          continue;
      }
      if( strcmp("-o",argv[i])==0 && i+1<argc) {
          strcpy(outFilename,argv[i+1]);
          i++;
          continue;
      }
    }

    if (outFilename[0]=='\0') 
    {
      fprintf(stderr,"Usage: imguCapture -single -o <output filename>\n");
      fprintf(stderr,"Usage: imguCapture -multiple <number of images> -o <output filename>\n");
      exit(0);
    }
    if (N<1) N=1;

	signal(SIGINT, CtrlCHandler);

	printf("-- test capture --\n");

	if( imguCaptureInit(device) ) {
		printf("Module is not available\n");
		exit(0);
	}

	printf("module ok\n");

	camNames=imguCaptureList(device);
	if( camNames==NULL || camNames[0]==NULL ) {
		printf("no camera available\n");
		imguCaptureUninit(device);
		exit(0);
	}

	printf("Got the following cameras:\n");
	for(i=0;camNames[i];i++) {
		printf("cam %d is '%s'\n",i,camNames[i]);
	}
	printf("Using first camera '%s'\n",camNames[0]);

	if( imguCaptureOpen(device,camNames[0],&cap,NULL,NULL) ) {
		printf("unable to start capture\n");
		imguCaptureUninit(device);
		exit(0);
	}

	printf("capturing..\n");

	sleep(1); //donner une chance au device de commencer a ramasser des images

    if (single_frame)
    {
	  imgu *I=NULL;

      k=imguCaptureOne(&cap,&I);
	  printf("capture returned %d\n",k);
	  if( k==0 ) {
	    imguSave(I,outFilename,1,SAVE_8_BITS_LOW);
      }
      imguFree(&I);
    }
    else //multiple frames
    {
    }

	// done
	imguCaptureClose(&cap);

	// done!
	imguCaptureUninit(device);

	exit(0);
}


