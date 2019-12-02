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
// joue le videos fournis en ligne de commande
// options: -audio 55555  -> envoie des commandes pour syncher l'audio
//

#include <imgu.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include <udpcast.h>

#define LUT "lut.png"

int tid_view;   // viewer
paramlist *pl_view;
rqueue *Qdisplay;
rqueue *Qpoubelle;

rqueue* Qrecycle;

udpcast udp;

static void flushLastLine(imgu *I)
{
int x,y,dx,dy,i;
int *sum;
    sum=(int *)malloc(I->xs*I->ys*sizeof(int));
    for(x=1;x<I->xs-1;x++) {
        for(y=1;y<I->ys-1;y++) {
            sum[y*I->xs+x]=0;
            for(dx=-1;dx<=1;dx++)
            for(dy=-1;dy<=1;dy++) {
                sum[y*I->xs+x]+=PIXEL(I,x+dx,y+dy,2);
            }
        }
    }
    for(i=0;i<I->xs*I->ys;i++) {
        if( sum[i]<IMGU_MAXVAL*9 ) {
            if( I->data[i*I->cs+2]>0 ) {
                //printf("reset (%d,%d)\n",i%I->xs,i/I->xs);
                I->data[i*I->cs+2]=0;
            }
        }
    }
    free(sum);
}


static int setupViewer(int audioPort)
{
        Qdisplay=imguRegisterQueue("display");
        Qpoubelle=imguRegisterQueue("poubelle");

    // start the viewer
#ifdef LUT
    //tid_view=imguStartPlugin("viewer","sink","-in display -out poubelle -view0 yuvlutHLblend 0 0 1 1 -geom 0 0 960 540 false");
    tid_view=imguStartPlugin("viewer","sink","-in display -out poubelle -view0 yuvlutHLblend 0 0 1 1 -geom 0 0 1920 1080 false");
#else
    tid_view=imguStartPlugin("viewer","sink","-in display -out poubelle -view0 yuv 0 0 1 1 -geom 0 0 640 480 true");
#endif
    if( tid_view<0 ) { printf("!!!!!!!!!!!! NO VIEWER PLUGIN!\n");return(-1); }

    pl_view=imguGetPluginParameters(tid_view);

    // pout tous les players...
    // create a recycling queue
    Qrecycle=imguRegisterQueue("recycle");
    // we decide the image size
    // each image must be different buffer
    imgu *IA;
    int i;
    for(i=0;i<30;i++) {
        IA=NULL;
        RQueueAddLast(Qrecycle,(void *)&IA);
    }


    if( audioPort>0 ) {
    int k;
     k=udp_init_sender(&udp,"127.0.0.1",audioPort,TYPE_NORMAL);
    printf("UDP init returned %d\n",k);
    }

}

/*
static void flushLastLine(imgu *I)
{
int x,y,dx,dy,i;
int *sum;
    sum=(int *)malloc(I->xs*I->ys*sizeof(int));
    for(x=1;x<I->xs-1;x++) {
        for(y=1;y<I->ys-1;y++) {
            sum[y*I->xs+x]=0;
            for(dx=-1;dx<=1;dx++)
            for(dy=-1;dy<=1;dy++) {
                sum[y*I->xs+x]+=PIXEL(I,x+dx,y+dy,2);
            }
        }
    }
    for(i=0;i<I->xs*I->ys;i++) {
        if( sum[i]<IMGU_MAXVAL*9 ) {
            if( I->data[i*I->cs+2]>0 ) {
                //printf("reset (%d,%d)\n",i%I->xs,i/I->xs);
                I->data[i*I->cs+2]=0;
            }
        }
    }
    free(sum);
    }*/

static int doVideoPlay(char *name,int audioPort)
{
int tid_cam;    // ffmpeg player
int i,k;
char buf[300];

    if( audioPort>0 ) {
    sprintf(buf,"open %s.ogg;\n",name);
    udp_send_data(&udp,(unsigned char *)(buf),strlen(buf));
    }

#ifdef LUT
    printf("Loading LUT %s\n",LUT);
    imgu *IZ=NULL;
    //  int k;
    k=imguLoad(&IZ,LUT,LOAD_8_BITS_HIGH);
    if( k ) { printf("Unable to load LUT %s\n",LUT);return(-1); }
    flushLastLine(IZ);
    flushLastLine(IZ);
    flushLastLine(IZ);

        imguReplaceAddText(IZ,"UNIFORM","lutH");
        imguReplaceAddText(IZ,"VIEWNUM","0");
        RQueueAddLast(Qdisplay,(void *)&IZ);


    while( RQueueRemoveFirstWait(Qpoubelle,(void *)&IZ) ); // attend que l'image revienne

    k=imguLoad(&IZ,LUT,LOAD_8_BITS_LOW);
    if( k ) { printf("Unable to load LUT %s\n",LUT);return(-1); }
    flushLastLine(IZ);
    flushLastLine(IZ);
    flushLastLine(IZ);

        imguReplaceAddText(IZ,"UNIFORM","lutL");
        imguReplaceAddText(IZ,"VIEWNUM","0");
        RQueueAddLast(Qdisplay,(void *)&IZ);

    while( RQueueRemoveFirstWait(Qpoubelle,(void *)&IZ) ); // attend que l'image revienne
    imguFree(&IZ);
    IZ=NULL;
#endif

    return(0);
}
/*
static int doVideoPlay(char *name,int audioPort)
{
int tid_cam;    // ffmpeg player
int i,k;
char buf[300];

    if( audioPort>0 ) {
    sprintf(buf,"open %s.ogg;\n",name);
    udp_send_data(&udp,buf,strlen(buf));
    }


    sprintf(buf,"-in recycle -out display -file %s -fps video -outformat yuv -loop 0",name);

    tid_cam=imguStartPlugin("ffmpeg","camera",buf);
    if( tid_cam<0 ) { printf("Could not start the ffmpeg plugin %d\n",i);return(-1); }

    // get access to plugin parameters
    paramlist *pl_cam=imguGetPluginParameters(tid_cam);

    // ne sert a rien pour un movie...
    paramInvokeCommand(pl_cam,"START");



    //printf("ZZZZzzzzzZZzz...\n");
    //sleep(1); // pour laisser le plugin commencer a decoder...

    // main loop: do nothing except check the if the viewer finished
    int n;
    int status=0;
    imgu *I;
    int newvideo=1;
    for(n=0;;n++) {
    // vide la poubelle dans la queue de recyclage
    while( RQueueRemoveFirst(Qpoubelle,(void *)&I)==0 ) {
        RQueueAddLast(Qrecycle,(void *)&I);
        if( newvideo ) {
            printf("******* SYNC AUDIO!!!!!! *******\n");
            if( audioPort>0 ) {
                strcpy(buf,"start\n");
                udp_send_data(&udp,(unsigned char *)(buf),strlen(buf));
            }
            newvideo=0;
        }
        }

    if( n%100==0 ) {
        paramGetInt(pl_view,"done",&k);
        if( k ) { printf("We are done!\n");status=1;break; }

        paramGetInt(pl_cam,"LOOPNUM",&k);
        //printf("LOOP is %d\n",k);
        if( k>0 ) { status=0;break; } // on a fini ce video!
    }

        usleep(1000);
        //if( n%10==5 ) paramInvokeCommand(pl_scan,"ON");
        //if( n>0 && n%10==0 ) paramInvokeCommand(pl_scan,"OFF");
    }

    // we are done!
   imguStopPlugin(tid_cam);
    //imguUnregisterQueue("recycle",NULL);

    return(status);
    }*/

int main( int argc, char **argv )
{
int audio;
int i,j;
int nbV;
char **videos;

   nbV=1;

   audio=-1;
   for(i=1;i<argc;i++) {
    if( strcmp(argv[i],"-audio")==0 && i+1<argc ) {
        audio=atoi(argv[i+1]);
        i++;continue;
    }
    // on a un fichier video
    nbV=argc-i;
    break;
   }
   printf("audio is %d\n",audio);
   printf("Videos nb=%d\n",nbV);
   videos=(char **)malloc(nbV*sizeof(char *));
   for(j=0;i<argc;i++,j++) videos[j]=argv[i];

   for(j=0;j<nbV;j++) {
    printf("video %d : %s\n",j,videos[j]);
   }

   setupViewer(audio);

   int k;
   for(i=0;;i++) {
    k=doVideoPlay(videos[i%nbV],audio);
    if( k ) break;
    sleep(1);
   }

   imguStopPlugin(tid_view);

   return 0;
}


