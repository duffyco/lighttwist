

/*
 * imgu_recycle
 *
 * This module manage the automatic recycling of images in a multi-thread context.
 * In this model, all images are stored into queues (rqueue.c)
 *
 * All images, empty or not, start in a special "queue of available images", say Qrecycle
 * A process 1 gets an image from Qrecycle, process it, and put it into another queue, Q1.
 * A process 2 gets an image from Q1, process it, put it into Q2, etc...
 * Eventually, the image is not needed anymore. It must be returned to Qrecycle.
 *
 * the imgu struct contains a field saying to which queue it should be sent for recycling.
 *
 * You can set the recycling queue of an image with imguSetRecycleQueue()
 * You can recycle an image with imguRecycle()
 *
 * If no recycling queue is defined, nothing happens.
 *
 */

#include "imgu.h"
#include <rqueue.h>

void imguSetRecycleQueue(imgu *I,rqueue *Q)
{
    if( I==NULL ) { fprintf(stderr,"[imguSetRecycleQueue] WARNING! NULL image!!!!!!!\n"); }
    while( I ) {
        I->Qrecycle=Q;
        I=I->next;
    }
}

// support MULTI images
int imguRecycle(imgu *I)
{
imgu *J;
    while( I!=NULL ) {
        J=I->next;
        I->next=NULL; // make this image a "single" image
        if( I->Qrecycle ) RQueueAddFirst(I->Qrecycle,(void *)&I);
        else imguFree( &I );
        I=J;
    }
    return(0);
}




