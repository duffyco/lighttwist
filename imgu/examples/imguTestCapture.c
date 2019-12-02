/*
 * Test
 * - image creation
 * - image save
 *
 */


/*** deux version, pour test... selon IMGU8 ***/

//
// Si IMGU8 est utilise pour choisir 8bits vs 16bits, alors on peut faire
// include imgu.h directement (parce qu'il utilise IMGU8)
//
// On peut aussi forcer 8bits en utilisant include imgu8.h
//

/*
#define DEVICE	"ffmpeg"
#define CAMERA	"/home/roys/Bureau/Video/mov077.avi"
*/

#define DEVICE	"ffmpeg"
         //#define CAMERA	"http://192.168.2.114/vdata.v"
#define CAMERA	"/home/roys/Bureau/MOV023.TOD"

// pour utiliser la queue externe
#define USEQ

#define OUTPUT_NAME   "out%04d.png"


#include <imgu.h>

int doit(char *dev,char *cam) {
	imgu *I=NULL;
	int i;
	char buf[100];

#ifdef USEQ
	rqueue Qvid;
	RQueueInit(&Qvid,10,sizeof(imgu *),10);
#endif

        capture cap;

		//
		// Open "camera device" ffmpeg
		//
        if( imguCaptureInit(dev) ) {
                printf("Unable to open capture device %s\n",dev);
                return(-1);
        }

		//
		// Get the list of cameras (not relevant for ffmpeg)
		//
        char **cams=imguCaptureList(dev);
        if( cams==NULL || cams[0]==NULL || cams[0][0]==0 ) {
        printf("no cams available...\n");
    }else{
                int i;
                for(i=0;cams[i];i++) printf("cam %d is '%s'\n",i,cams[i]);
        }

		//
		// Open a camera
		//
#ifdef USEQ
        if( imguCaptureOpen(dev,cam,&cap,NULL,&Qvid) ) {
#else
        if( imguCaptureOpen(dev,cam,&cap,NULL,NULL) ) {
#endif
                printf("unable to start capture\n");
                imguCaptureUninit(dev);
                return(-1);
        }

		printf("open was done correctly\n");

		//
		// get/set parameters for the camera
		//

/***
        imguCaptureSetParam(&cap,NULL,buf,sizeof(buf));
        printf("ALL Set parameters are : '%s'\n",buf);

        imguCaptureGetParam(&cap,NULL,buf,sizeof(buf));
        printf("ALL Get parameters are : '%s'\n",buf);

        imguCaptureGetParam(&cap,"resolution",buf,sizeof(buf));
        printf("Get resolution (before) : '%s'\n",buf);
***/

		for(i=0;i<1600;i++) {
			printf("capturing %d\n",i);
			//
			// Get a single image
			//
#ifdef USEQ
			while( RQueueRemoveFirstWait(&Qvid,(void *)&I) ) ;
#else
			imguCaptureOne(&cap,&I);
#endif

			imguExtractRectangle(&I,I,876,100,1338-876+1,832-100+1);

			sprintf(buf,OUTPUT_NAME,i);
			imguSave(I,buf,1,SAVE_AS_IS);
			//
			// Recycle the image
			//
			imguRecycle(I);
		}

        imguCaptureClose(&cap);
        imguCaptureUninit(dev);

#ifdef USEQ
	while( RQueueRemoveFirst(&Qvid,(void *)&I)==0 ) {
		printf("Qfree freeing one image\n");
		imguFree(&I);
	}
	RQueueFree(&Qvid);
#endif

		return(0);
}








int main( int argc, char **argv )
{
int k;
	if( argc>1 ) k=doit(DEVICE,argv[1]);
	else k=doit(DEVICE,CAMERA);
	printf("doit returned %d\n",k);
	printf("OUTPUT is in %s\n",OUTPUT_NAME);

    return 0;
}

