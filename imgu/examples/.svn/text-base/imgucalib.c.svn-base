/*
 * This file is part of IMGU.
 *
 * @Copyright 2008 Université de Montréal, Laboratoire Vision3D
 *   Sébastien Roy (roys@iro.umontreal.ca)
 *   Vincent Chapdelaine-Couture (chapdelv@iro.umontreal.ca)
 *   Lucie Bélanger
 *   Jamil Draréni
 *   Louis Bouchard (lwi.bouchard@gmail.com)
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
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

/******************************************************************************/

static double tv2sec( struct timeval* tv ) {
    return tv->tv_sec + (tv->tv_usec/1000000.0);
}

/******************************************************************************/

static int doCalibCV( const char *camid ) {

    rqueue* Qrecycle;
    int i, k;
    int tid_cam;    // thread id of pattern_camera plugin
    int tid_calib;    // thread id of pattern_camera plugin
    int tid_view;   // thread id of view_sink plugin
    char buf[200];

    struct timeval start, now;
    printf("doCalibCV......\n");
    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("calib");
    imguRegisterQueue("cam");

    sprintf( buf, "-in recycle -out cam -camid %s -fps 0 -viewnum 0", camid );
    tid_cam = imguStartPlugin( "gige", "camera", buf );
    //tid_cam = imguStartPlugin("gige","camera","-in recycle -out cam -camid 36134 -fps 0 -viewnum 0 ");
    if( tid_cam < 0 ) { printf("Could not start the cam plugin\n");exit(-1); }

    tid_calib = imguStartPlugin("grid_stereo_calib", "filter", "-in0 cam -out calib -w 9 -h 6 -n 20 -kill 1 -skip 4");
              //imguStartPlugin("grid_stereo_calib", "filter", "-in0 cam -out calib -w 9 -h 6 -n 30 -kill 1 -skip 4");
    if( tid_calib < 0 ) { printf("Could not start the calib plugin\n");exit(-1); }

    tid_view = imguStartPlugin("viewer","sink","-in calib -view0 cam0 0 0 1 1 -geom 0 0 800 600 true");
    if( tid_view < 0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }


    // get access to plugin parameters
    paramlist *pl_view=imguGetPluginParameters(tid_view);
    paramlist *pl_cam=imguGetPluginParameters(tid_cam);

    // this is the maximum number of images circulating at a single time
    imgu *IA=NULL;
    for(i=0;i<20;i++) RQueueAddLast(Qrecycle,(void *)&IA);

    gettimeofday( &start, NULL );

    // main loop: do nothing except check the if the viewer finished
    double start_s, now_s;
    start_s = tv2sec(&start);

    paramInvokeCommand(pl_cam,"START");

    for(;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }

        gettimeofday( &now, NULL );
        now_s = tv2sec(&now);

        if( now_s - start_s >= 0.4 ) {
            gettimeofday( &start, NULL );
            start_s = tv2sec(&start);
            printf("snapshot!!!!!!!!!!!!!!!!!!!!\n");
            paramInvokeCommand(pl_cam,"SNAPSHOT");

        }
        usleep(1000);
    }
    // we are done!
    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam);
    imguStopPlugin(tid_calib);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("cams",NULL);
    imguUnregisterQueue("calib",NULL);

    return 0;
}

/******************************************************************************/

static int do2CamsCalibCV( const char *camid0, const char* camid1 ) {

    rqueue* Qrecycle;
    int tid_cam0;    // thread id of pattern_camera plugin
    int tid_cam1;    // thread id of pattern_camera plugin
    int tid_calib0;
    int tid_calib1;
    int tid_view;   // thread id of view_sink plugin
    int i,k;
    struct timeval start, now;

    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");

    // create the intermediate image queue
    // no need to keep a pointer to it. Creating it is enough
    imguRegisterQueue("cam0");
    imguRegisterQueue("cam1");
    imguRegisterQueue("calib");

    char buf[200];
    // start the pattern
    sprintf( buf, "-in recycle -out cam0 -camid %s -fps 0 -viewnum 0  -ignoreErrors 1", camid0 );
    tid_cam0 = imguStartPlugin("gige","camera", buf );
    //tid_cam0 = imguStartPlugin( "load", "camera", "-in recycle -out cam0 -filename sample/img0_%03d.png -nb 100 -fps 4 -viewnum 0" );
    if( tid_cam0 < 0 ) { printf("Could not start the pattern plugin\n");exit(-1); }

    sprintf( buf, "-in recycle -out cam1 -camid %s -fps 0 -viewnum 1  -ignoreErrors 1", camid1 );
    tid_cam1 = imguStartPlugin("gige","camera", buf );
    //tid_cam1 = imguStartPlugin( "load", "camera", "-in recycle -out cam1 -filename sample/img1_%03d.png -nb 100 -fps 4 -viewnum 1" );
    if( tid_cam1 < 0 ) { printf("Could not start the pattern plugin\n");exit(-1); }

    //tid_calib0 = imguStartPlugin("grid_stereo_calib", "filter", "-in0 cam0 -in1 cam1 -out calib -w 8 -h 6 -n 100 -kill 1 ");
    tid_calib0 = imguStartPlugin("grid_stereo_calib", "filter", "-in0 cam0 -in1 cam1 -out calib -w 8 -h 6 -n 30 -kill 1 -skip 4");
    if( tid_calib0 < 0 ) { printf("Could not start the calib plugin\n");exit(-1); }

    // start the viewer
    //tid_view = imguStartPlugin("viewer","sink","-in calib -view0 one 0.5 0 0 1 -view1 two 1 0 0.5 1 -geom 0 0 1920 1080 false");
    tid_view = imguStartPlugin("viewer","sink","-in calib -view0 one 0 0 0.5 1 -view1 two 0.5 0 1 1 -geom 0 0 1200 600 true");
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");exit(-1); }

    // get access to plugin parameters
    paramlist *pl_view=imguGetPluginParameters(tid_view);
    paramlist *pl_cam0=imguGetPluginParameters(tid_cam0);
    paramlist *pl_cam1=imguGetPluginParameters(tid_cam1);


    // create a couple of empty images in the recycling queue
    // this is the maximum number of images circulating at a single time
    imgu *IA=NULL;
    for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);


    gettimeofday( &start, NULL );

    // main loop: do nothing except check the if the viewer finished
    double start_s, now_s;
    start_s = tv2sec(&start);

    paramInvokeCommand(pl_cam0,"START");
    paramInvokeCommand(pl_cam1,"START");

    for(;;) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");break; }

        gettimeofday( &now, NULL );
        now_s = tv2sec(&now);
        //printf("diff = %f\n", difftime( start, now ));

        if( now_s - start_s >= 0.4 ) {
            gettimeofday( &start, NULL );
            start_s = tv2sec(&start);
            printf("snapshot!!!!!!!!!!!!!!!!!!!!\n");
            paramInvokeCommand(pl_cam0,"SNAPSHOT");
            paramInvokeCommand(pl_cam1,"SNAPSHOT");

        }
        printf("\a");
        usleep(1000);
    }
    // we are done!
    // output statistics (use "dot -Tpdf -o out.pdf out.dot" to see the plugin stats)
    imguPluginDumpDot("out.dot");

    imguStopPlugin(tid_cam0);
    imguStopPlugin(tid_cam1);
    imguStopPlugin(tid_view);

    imguUnregisterQueue("recycle",NULL);
    imguUnregisterQueue("cams",NULL);
    return(0);


}

/******************************************************************************/

int main( int argc, char** argv )
{

    int i;
    char *cam0 = NULL;
    char *cam1 = NULL;

    for ( i = 1; i < argc; i++ ) {

        if ( strcmp(argv[i], "-h") == 0 ) {

            printf("imgucalib\n");
            printf("   -cam0 <camid> REQUIRED\n");
            printf("   -cam1 <camid>\n");
            printf("\n");

        } else if ( strcmp(argv[i], "-cam0") == 0 ) {
            i++;
            if ( !argv[i] ) {
                printf("invalid arguments\n");
                exit(-1);
            }
            cam0 = argv[i];

        } else if ( strcmp(argv[i], "-cam1") == 0 ) {
            i++;
            if ( !argv[i] ) {
                printf("invalid arguments\n");
                exit(-1);
            }
            cam1 = argv[i];

        } else {
            printf("invalid argument : %s\n", argv[i]);
            exit(-1);
        }

    }

    if ( !cam0 ) {
        printf("invalid arguments.  First camera (-cam0) not specified\n");
        exit(-1);
    }

    if ( !cam1 ) {

        doCalibCV( cam0 );

    } else { // 2 cams!

        do2CamsCalibCV( cam0, cam1 );

    }

    return 0;

}
