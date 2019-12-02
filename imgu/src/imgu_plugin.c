

/**
 * @file
 *
 * This file contains the plugin management function.
 *
 * imguLoadPlugin("toto.so")
 *    -> load the shared library, then call the function toto_init()
 *    -> the function toto_init() is responsable to register the plugin and its functions
 *
 * imguUnloadPlugin("toto.so");
 *    -> libere le plugin
 *    (Tue tous les threads en cours d'execution)
 *
 */

#include "imgu_plugin.h"

#include <string.h>
#include <unistd.h>


//#define VERBOSE

//
// list of all loaded plugins information
//
static imguPluginInfo *pluginList = NULL;

//
// list of all currently running plugin threads
//

//
// Each thread created by imguStartPluginFunction is registered here.
//
// the parameter structure here is created by the master. It contains the predefined
// parameter InitialParameters with the intial parameter string.
// Otherwise, the new thread will define its favorite parameters itself.
//
// Standard parameters are:
// args (String) contains the initial parameters of the plugin
//
#define MAX_Q 20
typedef struct {
    imguPluginInfo *plugin; // in the pluginList, contains the name/category/start, NULL=unused
    pthread_t t; // pthread id
    int inq[MAX_Q]; // index in queueTab[] , queues that are read (normally only 1), -1=unused
    int outq[MAX_Q]; // index in queueTab[], queues that are not read, -1=unused
    paramlist *PL; // contains all parameters of this plugin.
        // standard parameters:
        // 'args' contains the initial parameter string (that must match the param_template)
        // 'initialized' contains the return status of the initialization of the thread.
} threadInfo;


//
// global queue information
// each queue has a name...
// we keep a trace of which tread in pthreadTab[] use this queue
// if we deregister this queue, then we must kill all the treads first.
//
typedef struct {
    char *name;
    rqueue *q;
    //int users[MAX_USERS]; // thread index in threadTab[]
} qregister;


// not thread safe! Load and Unload must be called in the main thread.
// The start/stop plugin will be unsafe also... :-(
#define MAX_THREADS 100
static threadInfo threadTab[MAX_THREADS];

#define MAX_QUEUES 50
static qregister queueTab[MAX_QUEUES];

static int pluginInitialized=0;

//
// un mutex pour l'acces aux queues (BeginQueue/EndQueue)
//
static pthread_mutex_t mutexQueues;

//
// un mutex global pour usage dans les plugins (avec imguPluginGlobalLock / Unlock)
//
static pthread_mutex_t mutexPlugins;


//
// support... find the thread id
//
static int findMyThread(void)
{
int i;
pthread_t me = pthread_self();
    for(i=0;i<MAX_THREADS;i++) {
        if( threadTab[i].plugin==NULL ) continue;
        if( pthread_equal(threadTab[i].t,me) ) return(i);
    }
    return(-1);
}

/**
 * must be called once at the start of the program
 *
 * too many static global structures here... :-(
 *
 */
static void initPlugins(void)
{
int i,j;

    pluginList=NULL; // should be already initialized.

    pluginInitialized=1; // so that RegisterAll works

    // this is in plugins/allplugins.c
    imguRegisterAllPlugins();

    for(i=0;i<MAX_THREADS;i++) {
        threadTab[i].plugin=NULL;
        for(j=0;j<MAX_Q;j++) {
            threadTab[i].inq[j]=-1;
            threadTab[i].outq[j]=-1;
        }
    }

    for(i=0;i<MAX_QUEUES;i++) {
        queueTab[i].name=NULL;
        queueTab[i].q=NULL;
    }

    pthread_mutex_init(&mutexQueues,NULL);
    pthread_mutex_init(&mutexPlugins,NULL);


#ifdef VERBOSE
    printf("PLUGIN SYSTEM INITIALIZED\n");
#endif
}

#ifndef PLUGIN_PREFIX
    #define PLUGIN_PREFIX   "./"
#endif
#ifndef PLUGIN_SUFFIX
    #define PLUGIN_SUFFIX   ".so"
#endif

// this is called from allplugins.c, usually.
int imguRegisterPlugin(imguPluginInfo *pi)
{
imguPluginInfo *p;
    if( !pluginInitialized ) initPlugins();
    //
    // find the plugin (i)
    //
#ifdef VERBOSE
    printf("[REGISTER] plugin='%s' category='%s' usage='%s'\n",pi->name,pi->category,pi->usage);
#endif

    for(p=pluginList;p;p=p->next) if( strcmp(pi->name,p->name)==0 && strcmp(pi->category,p->category)==0 ) break;
    if( p ) { printf("already registered.\n");return(-1); }

    // initialize the plugin, if needed. (usually not)
    if( pi->init ) (pi->init)();

    pi->next=pluginList;
    pluginList=pi;

    return(0);
}


//
// return a handle to the threadTab[] (>=0)
//
// <0 -> error!
//
// When the plugin is started, we ask for parameter "initialized".
// If value >= 0, then all is OK. The thread id is returned.
// If value < 0, then error. thread is assumed dead. we return this code
//
// The value can be checked using Get on "initialized" anytime after this function is done.
//
int imguStartPlugin(const char *name, const char *category, char *params)
{
int j;
imguPluginInfo *p;
    if( !pluginInitialized ) initPlugins();

    for(p=pluginList;p;p=p->next) if( strcmp(name,p->name)==0 && (strcmp(category,p->category)==0 || category==NULL)) break;

    if( p==NULL ) return(-1000);

    //
    // find a spot in the thread table
    //
    for(j=0;j<MAX_THREADS;j++) {
        if( threadTab[j].plugin==NULL ) break;
    }
    if( j==MAX_THREADS ) return(-1001); // no more space for threads!

    //
    // start thread
    //
    threadTab[j].plugin=p;

    // thread parameters
    threadTab[j].PL=paramCreate();
    paramDefineString(threadTab[j].PL,"args",READ|WRITE|CONST); // we must allow WRITE
                                                                // since we're not the owner
    paramSetString(threadTab[j].PL,"args",params);

    // to synchronize the start, we make a 'initialized' parameter, with ASK
    // we GET the value. this is the return status of the plugin initialization.
    paramDefineInt(threadTab[j].PL,"initialized",READ|ASK,-100000,100000,1);

    pthread_attr_t pthread_custom_attr;
    pthread_attr_init(&pthread_custom_attr);
    pthread_create( &threadTab[j].t,
                    &pthread_custom_attr,
                    (void *)(p->start), threadTab[j].PL);


    //
    // TODO: here we need to sleep 1 second
    // to avoid problems with 'initialized' read too soon
    //
    // This needs to be fixed : IT IS FIXED!!!
    //
    //usleep(100000);

    //
    // wait for the plugin to finish initialization
    //
    int status;
    paramGetInt(threadTab[j].PL,"initialized",&status);
	printf("imguStartPLugin, initialized=%i\n", status);

    //  change the access of initialized so we can check it directly.
    paramDefineInt(threadTab[j].PL,"initialized",READ|CONST,-100000,100000,1);

    // it status is <0, then assume the thread is dead. return the code.
    // if >=0, then return the thread id.
    if( status < 0 ) {
        imguStopPlugin(j); // kill if not already dead. Cleanup the thread table
        return(status);
    }
    return(j);
}


//
// stop the thread
//
int imguStopPlugin(int tid)
{
    if( !pluginInitialized ) initPlugins();

#ifdef VERBOSE
    printf("KILLING %d\n",tid);
#endif
    if( tid<0 || tid>=MAX_THREADS ) return(-1);
    if( threadTab[tid].plugin==NULL ) return(-1);

    //
    // stop the thread (nicely), if still running
    //

#ifdef VERBOSE
    printf("CANCELLING %d '%s'\n",tid,threadTab[tid].plugin->name);
#endif
    pthread_cancel(threadTab[tid].t);
#ifdef VERBOSE
    printf("JOINING %d\n",tid);
#endif
    pthread_join(threadTab[tid].t,NULL);

#ifdef VERBOSE
    printf("DONE cancelling %d\n",tid);
#endif

    // free the parameter list
    paramFree(threadTab[tid].PL);

    //
    // free the thread structure
    //
    threadTab[tid].plugin=NULL;

#ifdef VERBOSE
    printf("DONE Stopping plugin %d\n",tid);
#endif

    return(0);
}


// for anyone that wich to access a plugin's parameters...
paramlist *imguGetPluginParameters(int tid)
{
    if( !pluginInitialized ) initPlugins();

    //printf("GetParamlist tid=%d\n",tid);
    if( tid<0 || tid>=MAX_THREADS ) return(NULL);
    if( threadTab[tid].plugin==NULL ) return(NULL);

    return(threadTab[tid].PL);
}



imguPluginInfo* imguGetPluginInfo(int tid) {
    if( !pluginInitialized ) initPlugins();

    ///printf("GetParamlist tid=%d\n",tid);
    if( tid<0 || tid>=MAX_THREADS ) return(NULL);
    if( threadTab[tid].plugin==NULL ) return(NULL);

    return(threadTab[tid].plugin);
}



//
// Register a queue globally
// this is thread safe.
// The queue itsef is not managed. Only the name.
//
rqueue *imguRegisterQueueX(const char *name,int nb,int inc)
{
int i,j;
rqueue *q;
    if( !pluginInitialized ) initPlugins();
    //
    // look if already defined, find an empty spot
    //
    j=-1;
    for(i=0;i<MAX_QUEUES;i++) {
        if( queueTab[i].name==NULL ) { if( j<0 ) j=i; continue; }
        if( strcmp(queueTab[i].name,name)==0 ) return(queueTab[i].q); // already registered. OK.
    }
    if( j<0 ) { printf("No more space for queue!!!!!!\n");return(NULL); } // no more space!
    //
    // allocate a new queue
    //
    q=(rqueue *)malloc(sizeof(rqueue));
    RQueueInit(q,10,sizeof(imgu *),40);

    queueTab[j].name=strdup(name);
    queueTab[j].q=q;

#ifdef VERBOSE
    printf("Registered queue '%s'\n",name);
#endif

    return(q);
}


rqueue *imguRegisterQueue(const char *name)
{
 return imguRegisterQueueX(name,10,40);
}


//
// Unregister a queue globally
// (remember to kill all threads using this queue)
// Queue MUST HAVE BEEN ALLOCATED BY imguRegisterQueue
//
int imguUnregisterQueue(const char *name,rqueue *q)
{
int i,j,k;
    if( !pluginInitialized ) initPlugins();
    //
    // find the queue
    //
    for(i=0;i<MAX_QUEUES;i++) {
        if( queueTab[i].name==NULL ) continue;
        if( name && strcmp(queueTab[i].name,name)==0 ) break;
        if( q && queueTab[i].q==q ) break;
    }
    if( i==MAX_QUEUES ) return(-1); // queue not found!

    //
    // stop any thread using queue i
    //
    for(j=0;j<MAX_THREADS;j++) {
            if( threadTab[j].plugin==NULL ) continue;
            for(k=0;k<MAX_Q;k++) {
                if( threadTab[j].inq[k]>=0 && threadTab[j].inq[k]==i ) imguStopPlugin(j);
                if( threadTab[j].outq[k]>=0 && threadTab[j].outq[k]==i ) imguStopPlugin(j);
            }
    }
    free(queueTab[i].name);
    queueTab[i].name=NULL;
    q=queueTab[i].q;
    queueTab[i].q=NULL;

    //
    // Empty (and free) all images from this queue
    //
    imgu *IA=NULL;
    while( RQueueRemoveFirst(q,(void *)&IA)==0 ) imguFree(&IA);

    RQueueFree(q); // free the elements of the queue
    free(q);    // free the queue itself
    return(0);
}









//
// look for queue name or pointer
//
// return index of queue 'name' or queue 'q', whatever fits first
// return -1 if not found
//
static int findQueue(char *name,rqueue *q)
{
int i;
    for(i=0;i<MAX_QUEUES;i++) {
        if( queueTab[i].name==NULL ) continue;
        if( name && strcmp(queueTab[i].name,name)==0 ) break;
        if( q && queueTab[i].q==q ) break;
    }
    if( i==MAX_QUEUES ) return(-1); // queue not found!
    return(i);
}


//
// Get access to a queue (and register the thread using the queue!)
// we need the tid... look for pid then into threadTab[].. slow but ok
//
// WARNING: This function is called by plugin threads!!! Mutex required!
//
// read: 1=this queue will be used for reading, 0=never used for reading (only writing)
//
rqueue *imguBeginUseQueue(char *name,int read)
{
int i,j;
int tid;
    if( name==NULL ) return(NULL);
    if( !pluginInitialized ) initPlugins();

    tid=findMyThread();
#ifdef VERBOSE
    printf("BeginUseQueue '%s' thread=%d for_reading=%d\n",name,tid,read);
#endif

    //
    // find the queue
    //
    if( (i=findQueue(name,NULL))<0 ) return(NULL); // not found

    //
    // we are a thread... so we must protect access to the structures...
    //
    pthread_mutex_lock (&mutexQueues);

    //
    // add the queue i as an in ou out queue of thread tid
    //
    // we could add both in read and write Q lists... why not? eventually if needed.
    //
    if( read==FOR_READING ) {
        for(j=0;j<MAX_Q && threadTab[tid].inq[j]>=0 ;j++) ;
        if( j==MAX_Q ) { printf("OUT OF Q! MAX_Q too small!!!!!!!!!!!!!!!!!!!!\n"); }
        else threadTab[tid].inq[j]=i;
        //
        // Any 'ASK' parameter will broadcast on this condition, in case the thread is waiting on this queue
        // This gives a chance to check if any parameter request is pending.
        //
        paramAddCond(threadTab[tid].PL,&queueTab[i].q->not_empty);
    }else{
        // assume FOR_WRITING
        for(j=0;j<MAX_Q && threadTab[tid].outq[j]>=0 ;j++) ;
        if( j==MAX_Q ) { printf("OUT OF Q! MAX_Q too small!!!!!!!!!!!!!!!!!!!!\n"); }
        else threadTab[tid].outq[j]=i;
    }

    pthread_mutex_unlock (&mutexQueues);



    return(queueTab[i].q);
}


//
// to help plugins which have a non thread safe part.
//
void imguPluginGlobalLock(int ref,long pt) {
	pthread_mutex_lock (&mutexPlugins);
	printf("*** plugin global lock from %d  0x%08lx ***\n",ref,pt);
	fflush(stdout);
	sleep(2);
}
void imguPluginGlobalUnlock(int ref,long pt) {
	printf("*** plugin global unlock from %d 0x%08lx ***\n",ref,pt);
	fflush(stdout);
	sleep(2);
	pthread_mutex_unlock (&mutexPlugins);
}


// could be the name or the actual queue, either one is fine
//
// Warning: this is called by thread! use mutex
//
int imguEndUseQueue(char *name,rqueue *q)
{
int i,j;
int tid;
    if( name==NULL ) return(-1);
    if( !pluginInitialized ) initPlugins();

    tid=findMyThread();
    //printf("EndUseQueue '%s' tid=%d\n",name,tid);

    if( (i=findQueue(name,q))<0 ) return(-1);  // not found

    //
    // we are a thread... so we must protect access to the structures...
    //
    pthread_mutex_lock (&mutexQueues);

    //
    // Look for our thread
    //
    for(j=0;j<MAX_Q;j++) {
        if( threadTab[tid].inq[j]==i ) {
            threadTab[tid].inq[j]=-1;
            // remove any condition for ASK parameters
            paramRemoveCond(threadTab[tid].PL,&queueTab[i].q->not_empty);
        }
        if( threadTab[tid].outq[j]==i ) threadTab[tid].outq[j]=-1;
    }

    pthread_mutex_unlock (&mutexQueues);

#ifdef VERBOSE
    printf("removed user tid=%d from queue '%s'\n",tid,queueTab[i].name);
#endif
    return(0);
}



void imguPluginDump(void)
{
int i,j,k;
imguPluginInfo *pi;
    if( !pluginInitialized ) initPlugins();

    //imguRefreshQueues();
    printf("--- plugins information ---\n");
    //
    // all plugins
    //
    for(pi=pluginList;pi;pi=pi->next) {
        printf("plugin '%s' category '%s' usage '%s' loaded\n",pi->name,pi->category,pi->usage);
    }

    //
    // all current threads
    //
    for(i=0;i<MAX_THREADS;i++) {
            if( threadTab[i].plugin==NULL ) continue;
            pi=threadTab[i].plugin;
            printf("thread %d : (plugin '%s' cat '%s' usage '%s')",i,pi->name,pi->category,pi->usage);
            printf(" inQ=[");
            for(k=0;k<MAX_Q;k++) if( threadTab[i].inq[k]>=0 )
                    printf("%s,",queueTab[threadTab[i].inq[k]].name);
            printf("] outQ=[");
            for(k=0;k<MAX_Q;k++) if( threadTab[i].outq[k]>=0 )
                    printf("%s,",queueTab[threadTab[i].outq[k]].name);
            printf("]\n");
            printf("-- paramlist --\n");
            paramDump(threadTab[i].PL);
    }

    //
    // all queues
    //
    for(i=0;i<MAX_QUEUES;i++) {
        if( queueTab[i].name==NULL ) continue;
        printf("Queue '%s' (read by thread:",queueTab[i].name);
        for(j=0;j<MAX_THREADS;j++) {
            if( threadTab[j].plugin==NULL ) continue;
            for(k=0;k<MAX_Q;k++) if( threadTab[j].inq[k]==i ) printf("%d,",j);
        }
        printf(") (write by thread:");
        for(j=0;j<MAX_THREADS;j++) {
            if( threadTab[j].plugin==NULL ) continue;
            for(k=0;k<MAX_Q;k++) if( threadTab[j].outq[k]==i ) printf("%d,",j);
        }
        printf(")\n");
    }

    printf("-----------------------------\n");
}

void imguPluginDumpDot(char *filename)
{
int i,j,k;
imguPluginInfo *pi;
int open;
FILE *F;
    open=0;
    F=NULL;
    if( filename ) { F=fopen(filename,"w"); }
    if( F==NULL ) F=stdout; else open=1;

    if( !pluginInitialized ) initPlugins();

    fprintf(F,"digraph g {\n");
    fprintf(F,"  graph [ style=rounded rankdir=LR ];\n");
    fprintf(F,"  node [ fontsize=16 shape=Mrecord ];\n");
    fprintf(F,"  edge [ ]; \n");

    // output all clusters (all plugin names)
    for(i=0,pi=pluginList;pi;pi=pi->next,i++) {
        fprintf(F,"  subgraph cluster%d {\n",i);
        fprintf(F,"    ");
        k=0;
        for(j=0;j<MAX_THREADS;j++) {
            if( threadTab[j].plugin==pi ) { fprintf(F,"thread%d ",j);k++; }
        }
        if( k ) fprintf(F,";\n");
        fprintf(F,"    label=\"%s\\n%s\";\n",pi->name,pi->category);
        fprintf(F,"  }\n");
    }

    //imguRefreshQueues();
    //
    // all plugins
    //
    /*
    for(pi=pluginList;pi;pi=pi->next) {
        printf("plugin '%s' category '%s' usage '%s' loaded\n",pi->name,pi->category,pi->usage);
    }
    */

    //
    // all current threads
    //
    for(i=0;i<MAX_THREADS;i++) {
            if( threadTab[i].plugin==NULL ) continue;
            pi=threadTab[i].plugin;
            fprintf(F,"  thread%d [ label=\"%d\" shape=ellipse ];\n",i,i);
            //printf("thread %d : (plugin '%s' cat '%s' usage '%s')",i,pi->name,pi->category,pi->usage);
            for(k=0;k<MAX_Q;k++) if( threadTab[i].inq[k]>=0 )
                    fprintf(F,"  queue%d -> thread%d;\n",threadTab[i].inq[k],i);
            for(k=0;k<MAX_Q;k++) if( threadTab[i].outq[k]>=0 )
                    fprintf(F,"  thread%d -> queue%d;\n",i,threadTab[i].outq[k]);
            //paramDump(threadTab[i].PL);
    }

    //
    // all queues
    //
    for(i=0;i<MAX_QUEUES;i++) {
        if( queueTab[i].name==NULL ) continue;

#ifdef USE_PROFILER
        rqueue *q=queueTab[i].q;
        countinfo c;

        RQueuePeekLock(q);
        profiler_count_stats(&q->ci);
        c=q->ci;
        RQueuePeekUnlock(q);

        fprintf(F," queue%d [ label=\"{ %s | { size %d .. %d | mean %.2f +/- %.2f | empty %.2f%% } }\" ];\n",i,queueTab[i].name,c.min,c.max,c.mean,c.std,c.percent0*100);
#else
        fprintf(F," queue%d [ label=\"{ %s }\" ];\n",i,queueTab[i].name);
#endif
/*
        for(j=0;j<MAX_THREADS;j++) {
            if( threadTab[j].plugin==NULL ) continue;
            for(k=0;k<MAX_Q;k++) if( threadTab[j].inq[k]==i ) printf("%d,",j);
        }
        printf(") (write by thread:");
        for(j=0;j<MAX_THREADS;j++) {
            if( threadTab[j].plugin==NULL ) continue;
            for(k=0;k<MAX_Q;k++) if( threadTab[j].outq[k]==i ) printf("%d,",j);
        }
*/
    }

    //printf("-----------------------------\n");
    fprintf(F,"}\n");

    if( open ) fclose(F);
}




//
// le nom de camera est le nom du video
// retourne 0 si ok, <0 si error
//
// Args: -in q1 -in q2 -file toto.mpg -format xyz
// Parameters:
//              --
//
// ATTENTION: A plugin CANNOT die before it has answered the initialization request.
//            It should return non zero status and then die.
//
//

// THIS IS USED ONLY INSIDE A PLUGIN.
// DO NOT die if status is 0
void finish_init_and_maybe_die(paramlist *PL,int status)
{
    //
    // We are done with the initialization. Tell the main we are GO.
    // (the main is asking to GET the parameter 'initialization'. lets reply)
    // We MUST loop until there is a request. usleep is required to remove
    // possible race until the master has called paramGetInt()
    //
    char *name;
    for(;;) {
        if( paramNextRequest(PL,&name,NULL)==NONE ) { usleep(10000);continue; }
        if( strcmp(name,"initialized")!=0 ) continue; // not supposed to get here.
        paramSetInt(PL,name,status);
        break;
    }

    if( status<0 ) pthread_exit(0);
}



//
// Fonctions dealing with time
//

#include <sys/time.h>

double getTimeNow(void)
{
struct timeval tv;
    gettimeofday(&tv,NULL);

    return (double)tv.tv_sec+tv.tv_usec/1000000.0;
}

//
// return the timestamp or -1 of no timestamp available or messed up timestamp tag
//
double getTimeStamp(imgu *I)
{
char *buf;
int s,us;
double t;
    buf=imguGetText(I,"TIMESTAMP");
    if( buf==NULL ) return(-1.0);

    if( sscanf(buf," %d %d",&s,&us)!=2 ) {
        if( sscanf(buf," %lf",&t)==1 ) return(t);
        return(-1.0);
    }
    return((double)s+us/1000000.0);
}




