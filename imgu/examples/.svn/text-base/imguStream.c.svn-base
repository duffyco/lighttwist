//
//
//  test de streaming
//
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <imgu.h>
#include <udpcast.h>

#include "imgu_stream.h"

//#include "ImageViews.h"

#define IMGNAME	"/home/roys/Bureau/videos_cube/cube_agora_2/cube_grab_ir_%05d.png"
#define IMGMIN	0
#define IMGMAX	200


#define OUTNAME "out%05d.png"

int send_stream(int port)
{
char buf[100];
imgu *IA=NULL;
int i,k;
imgustream is;

	// opening stream
	k=init_send_stream(&is,"192.168.1.255",port,TYPE_BROADCAST);
	//k=init_send_stream(&is,"127.0.0.1",12345,TYPE_BROADCAST);
	//k=init_send_stream(&is,"226.0.0.1",12345,TYPE_MULTICAST);
	//k=init_send_stream(&is,"192.168.1.203",12345,TYPE_NORMAL);
	if( k ) { printf("unable to access the network\n");return(-1); }


	for(i=IMGMIN;i<=IMGMAX;i++) {
		sprintf(buf,IMGNAME,i);
		k=imguLoad(&IA,buf,LOAD_16_BITS);
		if( k ) { printf("Unable to load '%s'\n",buf);return(-1); }
		printf("-- sending %d --\n",i);

		stream_send_img(&is,IA,STREAM_8_BITS_HIGH); // flag: STREAM_16_BITS ou _8_BITS_HIGH
	}
	uninit_send_stream(&is);
	return(0);
}



int receive_stream(void)
{
imgu *IA=NULL;
imgustream is;
int i,k;
char buf[100];
	// opening stream
	//if( init_receive_stream(&is,NULL,12345) ) { printf("ERR\n");return(-1); }
	if( init_receive_stream(&is,"192.168.1.202",12345) ) { printf("ERR\n");return(-1); }

	for(i=0;;i++) {
	  stream_receive_img_part(&is,&IA, 50);
	  sprintf(buf,OUTNAME,i);
	  imguSave(IA,"out.png",1,SAVE_16_BITS);
	  //imguClear(IA);
	  usleep(100);
	}
	uninit_receive_stream(&is);
	return(0);
}

capture cap[5];
imgu *I[5*4]; // 4 images par stream



// on recoit une image ici! (pour l'instant toujours la meme
int callback(imgu *I)
{
        if( err ) {
                printf("Got image %d (err=%d)!\n",i,err);
                return(1);
        }

	printf("recu image %d du stream %d\n",i,cap->userNumber);

/*
        if( nbSaved<NB_SAVE ) {
                printf("copy image %d to %d\n",i,nbSaved);
                imguCopy(&ISaved[nbSaved],I[i]);
                nbSaved++;
        }
*/

        //IV->updateViewImage(cap->userNumber,I[i]);

		imguRecycle(I);

        return(1); // remettre dans la file
}




// utilise le device imgucapture
int receive_istream(void)
{
int i,k;
char camName[100];
	printf("-- receive ISTREAM --\n");

	if( imguCaptureInit("inet") ) { printf("Le device '%s' ne peut pas etre initialise...\n","inet");exit(0); }

	// start 5 stream
	for(i=0;i<5;i++) {

		I[i]=NULL;

		A ARRANGER!!!!

		sprintf(camName,"%d",12345+i);
		if( imguCaptureOpen("inet",camName,cap+i,NULL,NULL) ) {
			printf("unable to start capture %d\n",i);
			imguCaptureUninit("inet");
			exit(0);
		}
		cap[i].userNumber=i;

		k=imguCaptureMany(cap+i,(int (*)())callback);
	}

	for(i=0;;i++) { printf("ZzzzZZZZZZzzZZzZZZzz %d\n",i);sleep(1); }

	for(i=0;i<5;i++) imguCaptureClose(cap+i);
	imguCaptureUninit("inet");
	return(0);
}


int main(int argc,char *argv[])
{
	printf("- imgu stream test-\n");

	if( argc<2 ) {
		printf("master | slave | islave\n");
		exit(0);
	}

	if( strcmp(argv[1],"master")==0 ) {
		send_stream(atoi(argv[2]));
	}else if( strcmp(argv[1],"slave")==0 ) {
		receive_stream();
	}else if( strcmp(argv[1],"islave")==0 ) {
		receive_istream();
	}else printf("master | slave\n");
	

	exit(0);
}


