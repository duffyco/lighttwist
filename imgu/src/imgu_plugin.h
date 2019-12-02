#ifndef IMGU_PLUGIN_H
#define IMGU_PLUGIN_H




/**
 * @defgroup plugins Plugin system
 *
 * @ingroup imgugeneral
 *
 * This is the multithreaded plugin support system.
 *

The plugin system manages threads and image queues in order to make it easy to
create multi-threaded image processing applications.

The system uses image queues (@ref rqueue) for inter-thread communication.
A typical plugin will remove images from an input queue, process them, then put
the resulting images in an output queue. Removing an image can induce wait if the
queue is empty, but adding an image is always done without any wait.

To use a plugin, input and output queues must be created. Queues are managed
globally in the library using names.

@code
rqueue* Qrecycle = QrecycleimguRegisterQueue("recycle");
rqueue* Qresult = QrecycleimguRegisterQueue("result");
@endcode

To start a plugin, specify the name and category and any options.

@code
int tid=imguStartPlugin("pattern","camera","-in recycle -out result -width 512 -height 512 -fps 5");
if( tid<0 ) { printf("Could not start the pattern plugin\n");exit(-1); }
@endcode

Each plugin has a parameter list that can be used to read or write parameter values
of the plugin, or to invoke commands. For example, a gaussian blur plugin would
have a variance parameter to change the filter. Each plugin "category" has a
specific set of "mandatory" parameters. For example, "camera" plugins must have
a "FRAMENUM" parameter and a "START", "STOP", "SNAPSHOT" parameter command.

@code
paramlist *PL=imguGetPluginParameters(tid);
paramInvokeCommand(PL,"START"); // since this is a camera plugin
@endcode

The first plugin will need empty images in its input queue (here the "recycle" queue).
Fill this queue with empty images. The number of images represent the maximum
number of images in the plugin system at any time. After these images are used, the
plugin will wait until an image is recycled. It is allowed to put NULL images in
a queue that is used as an image recycling source. Plugins with category "camera"
expect such a recycling queue. Those plugins produce images automatically.

@code
imgu *IA=NULL;
for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(void *)&IA);
@endcode

The main loop first puts an image in the input queue of the plugin, then wait
for the result on the output queue. Note that a "camera" plugin will generate
images by itself, so we can simply wait for an output

@code
imgu *IR;

for(;;) {
    ...
    RQueueRemoveLastWaitForever(Qresult,(unsigned char*)&IR);
    ...
    (process image IR)
    ...
    imguRecycle(IR);
    ...
}
@endcode

Once you get an image from a Queue, it belongs to you. When you are done with it,
you must either put it on another image queue, or recycle it. When you call
imguRecycle, the image returns to the queue that was set by imgeSetRecycleQueue.
A "camera" plugin will always set the recycle queue of an image it get from its
recycle queue in order to ensure that image eventually return there. Other
plugin types ("filter" or "sink") will not do that.

To stop a plugin:

@code
imguStopPlugin(tid);
@endcode

To unregister a queue registered with imguRegisterQueue, you can do it 2 ways.
Using the queue name only or using the queue pointer. Note that any plugin using
a queue which is unregistered will be automatically terminated.

@code
imguUnregisterQueue("recycle",NULL);
imguUnregisterQueue(NULL,Qresult);
@endcode



 *
 * @example imguTestPlugin.c imguview.cpp
 *
 *@{
 */

#include <rqueue.h>

#include "param.h"
#include "imgu.h"

// structure to define a plugin
typedef struct imguPluginInfo {
//
// each plugin must fill these 4 fields.
//
const char *name;           // name of the plugin function
const char *category;       // camera, filter, ...
const char *usage; // what parameters are expected on start

int (*start) (paramlist *pr);  // the actual thread

int (*init) (void);          // the init at registration of the plugin (NULL is ok)

// private (list of registered plugins with imguRegisterPlugin)
struct imguPluginInfo *next;
} imguPluginInfo;


#ifdef __cplusplus
extern "C" {
#endif


// this is inside plugin/allplugins.c
void imguRegisterAllPlugins(void);

int imguRegisterPlugin(imguPluginInfo *pi);
int imguStartPlugin(const char *name, const char *category, char *params);
int imguStopPlugin(int tid);

// pour les dynamic libraries
//  int imguLoadPlugin(char *pluginName);
//  int imguUnloadPlugin(char *pluginName);

//  int imguRegisterPluginFunction(char *plugin,char *function,char *usage);
//  int imguUnregisterPluginFunction(char *plugin,char *function);

//int imguStartPluginFunction(char *plugin,char *function,char *params);
//int imguStopPluginFunction(int tid);

rqueue *imguRegisterQueueX(const char *name,int nb,int inc);
rqueue *imguRegisterQueue(const char *name);
int imguUnregisterQueue(const char *name,rqueue *Q); // one must be NULL

// used by the plugins. Used to keep track of who is connected to what
#define FOR_READING     1
#define FOR_WRITING     0
rqueue *imguBeginUseQueue(char *name,int read);
int imguEndUseQueue(char *name,rqueue *q);

// for anyone that wich to access a plugin's parameters...
paramlist *imguGetPluginParameters(int tid);
imguPluginInfo* imguGetPluginInfo(int tid);

void imguPluginDump(void);
void imguPluginDumpDot(char *filename);

//
// extra stuff that is simply usefull to plugins
//
// ATTENTION: bad names!!! need to use imgu*, normally
//


 // used inside a plugin
void finish_init_and_maybe_die(paramlist *PL,int status);

double getTimeNow(void);
double getTimeStamp(imgu *I);


//
// to help plugins which have a non thread safe part.
//
void imguPluginGlobalLock(int ref,long pt);
void imguPluginGlobalUnlock(int ref,long pt);





#ifdef __cplusplus
}
#endif

/*@}*/


#endif

