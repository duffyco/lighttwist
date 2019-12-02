
#include <imgu.h>
#include <unistd.h>
#include <string.h>


#include <rqueue.h>

int doStreamOut(int port, char *dev);
int doStreamIn(int port);

int doStreamOut(int port, char *dev)
{
    int i,k;
    int tid_cam,tid_view,tid_radial;
    rqueue *Qrecycle;
    char plugStr[200];

    // create a send and receive queue
    // the Qfree queue is local to us, for recycling.
    Qrecycle    =imguRegisterQueue("recycle");
    imguRegisterQueue("cam");

    // create a couple of empty images for the plugin to use
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);


    //tid_cam=imguStartPlugin("ffmpeg","camera", "-in recycle -out cam -file /home/vision/lighttwist/media/test/plantspano2.mpg -fps 10");

    ///dev/video0
    sprintf( plugStr, "-in recycle -out cam -device %s  -viewnum 0 -width 320 -height 240 -fps 10", dev);
    // tid_cam=imguStartPlugin("v4l2", "camera", "-in recycle -out cam -device  -viewnum 0 -width 320 -height 240 -fps 10");
tid_cam=imguStartPlugin("v4l2", "camera", plugStr);
    if( tid_cam<0 ) exit(-1);

    sprintf( plugStr, "-in cam -out recycle -host 226.0.0.1 -port %i -type m -bits 8 -compression 0", port);
    tid_view=imguStartPlugin("streamout","sink", plugStr);
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO streamout PLUGIN!\n");exit(-1); }

    paramlist *pl_view=imguGetPluginParameters(tid_view);


    for(;;) {
        sleep(1);
    }

    // output statistics
    imguPluginDumpDot("out.dot");

    //
    // we are done!!!
    //
    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("cam",NULL);

    return(0);
}




/*******************************************************************************/




int doStreamIn(int port)
{
    rqueue* Qrecycle;
    int tid_cam;    // thread id of pattern_camera plugin
    int tid_view;   // thread id of view_sink plugin
    int i,k;
char plugStr[200];
    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cam");

    sprintf(plugStr, "-in recycle -out cam -viewnum 0 -host 226.0.0.1 -port %i -mtu 1450", port);
    tid_cam=imguStartPlugin("streamin","camera", plugStr);
    if( tid_cam<0 ) { printf("Could not start the gige plugin\n");exit(-1); }

    // start the viewer


    tid_view=imguStartPlugin("viewer","sink","-in cam -view0 cam0 0 0 1 1 -geom 1440 0 800 600 false ");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    paramlist *pl_camera=imguGetPluginParameters(tid_cam);
    paramlist *pl_view=imguGetPluginParameters(tid_view);

    paramInvokeCommand(pl_camera,"START");

    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    // main loop: do nothing except check the if the viewer finished
    int n;
    for(n=0;;n++) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }
        sleep(1);
    }

    // we are done!

    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("cam",NULL);
    return(0);
}



/*******************************************************************************/



int main( int argc, char **argv ) {

    if (argc > 2) {
        if (strcmp(argv[1],"-in")==0) {
            doStreamIn( atoi(argv[2]) );
        } else if (strcmp(argv[1],"-out")==0) {
            if ( argc <= 3 ) {
                printf( "specifiy device plz\n" );
            }
            doStreamOut(atoi(argv[2]), argv[3]);
        } else {
            printf("Please specify an option: -in <port> or -out <port> <dev>\n");
        }
    } else {
        printf("Please specify an option: -in <port> or -out <port> <dev>\n");
    }

    return 0;
}


