#ifndef PARAM_H
#define PARAM_H


//
// Parameter system, shared between threads.
//
// A paramlist is owned by a specific thread (the owner).
// The concept is that the owner thread can require to be "asked" when parameters are read or written
// by anyone. This allows the owner to obtain or update internal values in accordance with the
// request.
//
// For example, the owner is a thread controling a video4linux camera. The owner creates parameters
// brightness, contrast, ...
// Anyone can ask to get the value or to change such a parameter. The owner will be signaled,
// he will relay the information to the v4l device and return the result.
//
// There is a condition variable on a parameterlist for anyone who ask for requests and is waiting
// for the answer.
//
// Access flags:
// ( the owner has always the right to READ and WRITE any value directly )
// READ : anyone can read the parameter value
// WRITE : anyone can write a new value to the parameter
// VOLATILE : the parameter value can change value anytime (by the owner)
// CONST : the parameter is never changed (by the owner)
// ASK : anyone intending to read or write the parameter must send a request to the owner.
//
//


/* #undef HAVE_RQUEUE */

#ifdef HAVE_RQUEUE
	#define PARALLEL
#endif


#ifdef PARALLEL
  #include <pthread.h>
  #include <signal.h>

  #define MAX_COND	20
#endif



typedef struct {
	int min,max,step;
} param_int;

typedef struct {
	double min,max;
} param_double;

typedef struct {
	char *def; // list of possible values, comma separated. (strdup. must free when done)
				// simply set to NULL for a regular unconstrained string
} param_enum;



//
// internal structure. No one use it directly.
// 

typedef enum {
	ANY			= 0, // not a type... means absence of type
	ENUM		= 1, // String goes here also. simply set def=NULL.
	INT			= 2,
	DOUBLE		= 3,
	CMD			= 4,
	__UNUSED	= 999,
	} param_type;

typedef enum {
	READ		= 1,
	WRITE		= 2,
	VOLATILE	= 4,
	CONST		= 8,
#ifdef HAVE_RQUEUE
	ASK			= 16,
#endif
	} param_access;

typedef enum {
	NONE	= 0,
	GET		= 1,
	SET		= 2
	} param_request;

typedef union {
		char *eval;		// enum (or string) value
		int ival;		// int value
		double dval;	// double value
} multival;

typedef struct {
	char *name;		// nom du parametre. Always defined or NULL
	param_type type; // enum, int, double, command, or __UNUSED
	union {
		param_enum e;	// .e.val .e.def
		param_int i;	// .i.val .i.min .i.max .i.step
		param_double d; // .d.val .d.min .d.max
	} u; // current information

	// current value of the field (eval,ival,dval) sval is good for enum
	multival v;	// value we are asking for

	// information for SET, when asking for modification
	multival newv;	// value we are asking for

	param_access access; // Read|Write|Volatile|Const|ASK

#ifdef PARALLEL
	// the next parameter in the list (could be the request list or the done list)
	int next; // -1 is end of list

	// is this parameter currently in a list, pending a GET or SET request?
	// if SET or GET, then this param is in the request list.
	param_request request; // NONE | SET | GET

	int status; // this contains the result of the SET or GET request (0=ok, -1=failed, etc..)

#endif
} paraminfo;


typedef struct {
	paraminfo *p; // [nb] contenant les parametres (any UNUSED can be reused)
	int nb; // nb de parametres dans p
	// ici on va avoir des handler et process a signaler quand on veut changer des choses...
	//
	// The main client sets up a request in a parameter, and add the parameter to the request list.
	// Then he signal the condition of all INPUT queues of the owner of the paramlist. The owner can process
	// the request immediately in the signal handler.
	//
#ifdef PARALLEL
	int first_request;	// first node of a list of requests (-1 = empty)

	// owner thread (for signaling ASK requests)
	int owner_defined; // 0=no, 1=yes we know the owner
	pthread_t owner;
	// mutex to lock access to the paramlist
    pthread_mutex_t lock;  // pour acces general a la queue
    pthread_cond_t processed; // condition that request has been processed


	// where to signal when we need to ASK
	pthread_cond_t *conds[MAX_COND]; // each condition, or NULL. Use paramAddCond / paramRemoveCond
#endif

	// condition for the client to wait a for reply to his request
} paramlist;

//
// Une liste de parametres est simplement un paramlist *PL
//



#ifdef __cplusplus
extern "C" {
#endif




//
// Create a new paramlist
// (owner thread)
//
paramlist *paramCreate(void);

#ifdef PARALLEL
void paramSetOwner(paramlist *PL,pthread_t tid);
#endif


//
// Free a paramlist
//
void paramFree(paramlist *PL);

//
// Dump a parameter list
//
void paramDump(paramlist *PL);


//
// Define a parameter
// (return 0 if ok, -1=already exists, -2=OOM)
//
int paramDefineString(paramlist *PL,const char *name,param_access access);
int paramDefineEnum(paramlist *PL,const char *name,param_access access,char *def);
int paramDefineInt(paramlist *PL,const char *name,param_access access,int min,int max,int step);
int paramDefineDouble(paramlist *PL,const char *name,param_access access,double min,double max);
int paramDefineCommand(paramlist *PL,const char *name,param_access access);

int paramUndefine(paramlist *PL,const char *name); // remove the parameter completely

//
// Set the value of a parameter
//
// return 0 if ok, -1 if invalid, -2,...
//
int paramSetString(paramlist *PL,const char *name,char *val); // also good for Enum, Int or Double (uses sscanf on val)
int paramSetInt(paramlist *PL,const char *name,int val);
int paramSetDouble(paramlist *PL,const char *name,double val);
int paramInvokeCommand(paramlist *PL,const char *name); // to invoke, a command must grant read or write access

//
// Get the value of a parameter
//
//
// if buf==NULL -> return the value as a strdup string, or NULL on error
// if buf!=NULL -> fill the buffer upto size-1 char (nul terminated), return buf or NULL on error
char *paramGetString(paramlist *PL,const char *name,char *buf,int size); // also good for enum, int, double

// return 0 if ok, -1 on error
int paramGetInt(paramlist *PL,const char *name,int *val);
int paramGetDouble(paramlist *PL,const char *name,double *val);


//
// Check the existence, availability, and validity
//
int paramExists(paramlist *PL,char *name); // 1 if exists, 0 if does not exists
int paramAvailable(paramlist *PL,char *name); // 1 if READ or WRITE access is true (works for commands)
param_type paramType(paramlist *PL,char *name); // get the type, return 0 on error
param_access paramAccess(paramlist *PL,char *name); // get the type, return 0 on error

//
// Verify if value is allowed for this parameter
// if parameter does not exists, then not allowed...
// return 1 if value is valid, 0 if invalid.
//
// These function never ASK the owner for validation.
// Validation is simple, against min/max values or enumeration definition
//
int paramValidString(paramlist *PL,char *name,char *val); // also good for enum, int, double
int paramValidInt(paramlist *PL,char *name,int val);
int paramValidDouble(paramlist *PL,char *name,double val); // also good for enum, int, double

#ifdef PARALLEL
int paramAddCond(paramlist *PL,pthread_cond_t *c); // return 0 if ok, -1 if oomem
int paramRemoveCond(paramlist *PL,pthread_cond_t *c); // return 0 if ok, -1 if cond not found

// return NONE/SET/GET and fills the name (and val if SET)
// owner must call paramSetValue() on this parameter to notify the main that processing is done
param_request paramNextRequest(paramlist *PL,char **name,multival *val);
int paramRequestDone(paramlist *PL,char *name); // notify that this param request is done
		// very useful when we don't want to do a paramSet() on the parameter
		// because we want to use the current value of the parameter or its a command.
#endif


#ifdef __cplusplus
}
#endif


#endif

