
//
// Parameter library
//
// This manage a parameter list that is shared between an "owner" thread and a bunch
// of other "user" threads.
//
// One can read or write parameters, which can have string,enum,int, or double values.
// Values are read/changed directly and without synchronization
//
// If a parameter is tagged "ASK", then only the owner can read/write it directly.
// Any other thread attempting to read/write will generate a request, which will be signaled to
// the owner. The owner will decide what to do, set the value, then signal back that the
// request has been processed.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "param.h"


// assume 20 param to start
#define INIT_NB_PARAM	20

// how many params should we add when we are out of param?
#define ADD_NB_PARAM	10

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// static functions...............

//
// return the parameter handle, or -1 if not found or problem
//
// optionally specify an access mask. 0 = any is good. READ|WRITE -> on of the two if good
// optionally specify a type to impose (ANY if not important)
//
static int paramFind(paramlist *PL,const char *name,int accessMask,int typeRequired)
{
int i;
	if( PL==NULL ) return(-2);

	for(i=0;i<PL->nb;i++) {
		if( PL->p[i].type==__UNUSED ) continue;
		if( strcmp(PL->p[i].name,name)==0 ) break;
	}
	if( i==PL->nb ) return(-1); // parameter not found
	if( accessMask && ((PL->p[i].access & accessMask)==0) ) return(-2); // no access
	if( typeRequired!=ANY && (PL->p[i].type!=typeRequired) ) return(-3); // wrong type
	return(i); // OK!
}


//
// return the parameter handle for an unused parameter, -1 if problem
// reallocate in case of problem...
//
// ASSUME we are locked.
//
static int paramFindUnused(paramlist *PL)
{
int i;
paraminfo *p,*q;
int nnb;
	if( PL==NULL ) return(-2);

	for(i=0;i<PL->nb;i++) {
		if( PL->p[i].type==__UNUSED ) return(i);
	}

	//printf("reallocating...\n");

	p=PL->p;
	nnb=PL->nb+ADD_NB_PARAM;
	q=(paraminfo *)malloc(nnb*sizeof(paraminfo));
	if( q==NULL ) { printf("[PARAM] unable to reallocate!!!!!!!!\n");return(-1); }

	// index must never change.
	for(i=0;i<PL->nb;i++) q[i]=p[i];
	for(;i<nnb;i++) { q[i].type=__UNUSED;q[i].name=NULL; }

	PL->p=q;
	PL->nb=nnb;

	free(p); // only p, not what it contains (its been passed to q)!!!

	return(paramFindUnused(PL));
}

//
// pour depaner... access flag -> string representation
//
static char *acc2s(param_access access)
{
int i;
static char buf[20];
	i=0;
	buf[i++]='[';
	if( access & READ ) buf[i++]='R'; else buf[i++]=' ';
	if( access & WRITE ) buf[i++]='W'; else buf[i++]=' ';
	if( access & VOLATILE ) buf[i++]='V'; else buf[i++]=' ';
	if( access & CONST ) buf[i++]='C'; else buf[i++]=' ';
#ifdef PARALLEL
	if( access & ASK ) buf[i++]='A'; else buf[i++]=' ';
#endif
	buf[i++]=']';
	buf[i]=0;
	return(buf);
}


//
// verify that the value v is valid with respect to the enumeration def
// def is comma separated values (no blanks please.)
// -1 : invalid enum.
// 0,1,... index of enum value
//
static int whichEnumValue(char *def,char *v)
{
char *p; // inside the value
char *q; // inside the definition
int i;
	if( def==NULL || def[0]==0 ) return(0); // no def means always good.
	for(q=def,i=0;*q;) {
		// scan for the start of next definition
		while( *q && *q==',' ) { q++;i++; }
		if( *q==0 ) break; // no more pattern to test
		for(p=v;*p && *q && *p==*q && *q!=',';p++,q++) ;
		if( *p==0 ) {
			// we are at the end of the word to test. we have a match... possibly
			if( *q==0 || *q==',' ) return(i);
		}
		// scan to the end of current definition
		while( *q && *q!=',' ) q++;
	}
	return(-1); // not a match.
}


static int validValueInt(paraminfo *p,int val)
{
	if( val<p->u.i.min || val>p->u.i.max ) return(0); // out of range
	if( (val-p->u.i.min)%p->u.i.step != 0 )  return(0); // illegal step
	return(1);
}


static int validValueDouble(paraminfo *p,double val)
{
	if( val<p->u.d.min || val>p->u.d.max ) return(0); // out of range
	return(1);
}



#ifdef PARALLEL
//
// Put a request to SET or GET a parameter in the request queue, then signal everyone.
// wait for the request to be processed.
// return the status of the GET/SET down externaly (0 is good)
//
static int askForParameter(paramlist *PL,int h,int req,multival v)
{
int i;
//printf("[main]-> NEED TO ASK!!!\n");
	// assume h is ok
	// assume we are locked
	// assume we have to ask
	// assume we are NOT the owner (or the owner is not defined)
    // super important si jamais on recoit un cancel. Must match a pop()
    pthread_cleanup_push((void *)pthread_mutex_unlock,&PL->lock);

	// ATTENTION: PL->p can be changed if someone adds a new parameter

	// wait until this parameter is not involved in any request
	//printf("[main]-> waiting for current request to end\n");
	while( PL->p[h].request!=NONE ) {
		// loop on wait for the cond_signal until req=NONE
		//printf("[main]-> (cond_wait)...\n");
		pthread_cond_wait(&PL->processed, &PL->lock);
		//printf("[main]-> (cond_wait) done\n");
	}
	//printf("[main]-> setting request\n");
	// set the request for the owner
	PL->p[h].request=req; // GET or SET
	PL->p[h].newv=v;
	// add to request queue
	PL->p[h].next=PL->first_request;
	PL->first_request=h;
	// signal the owner that something has come up
	//if( PL->owner_defined ) pthread_kill(PL->owner,SIGALRM);
	// wake up any condition attached to this PL
	//printf("[main]-> signaling to wakeup\n");
	for(i=0;i<MAX_COND;i++) {
		if( PL->conds[i] ) pthread_cond_broadcast(PL->conds[i]);
	}
	//printf("[main]-> waiting for parameter to be processed\n");
	// sleep until or change is done
	while( PL->p[h].request!=NONE ) {
		//printf("[main] -> loop : request [%d] is %d\n",h,PL->p[h].request);
		// loop on wait for the cond_signal until req=NONE
		pthread_cond_wait(&PL->processed, &PL->lock);
	}
	//printf("[main]-> got it. parameter has been processed. Our status is %d\n",PL->p[h].status);
	// we are done! value might have changed.
	pthread_cleanup_pop(0);
	return(PL->p[h].status);
}
#endif




//
// Convert a multival into a multival
// if buf is NULL, then return a strdup()
// return NULL on error
// Type can be ENUM|INT|DOUBLE, otherwise error
//
static int multivalToMultival(int inType,multival in,int outType,multival *out)
{
int k;
char inbuf[50];
	k=0;
	//printf("M2M: in type %d, out type %d\n",inType,outType);
	switch( inType ) {
	  case ENUM:
		switch( outType ) {
		  case ENUM:	out->eval=strdup(in.eval);break;
		  case INT:		out->ival=atoi(in.eval);break;
		  case DOUBLE:	out->dval=atof(in.eval);break;
		  default: k=-1;
		}
		break;
	  case INT:
		switch( outType ) {
		  case ENUM:	sprintf(inbuf,"%d",in.ival); out->eval=strdup(inbuf); break;
		  case INT:		out->ival=in.ival;break;
		  case DOUBLE:	out->dval=(double)in.ival;break;
		  default: k=-1;
		}
		break;
	  case DOUBLE:
		switch( outType ) {
		  case ENUM:	sprintf(inbuf,"%12.12g",in.dval); out->eval=strdup(inbuf); break;
		  case INT:		out->ival=(int)in.dval;break;
		  case DOUBLE:	out->dval=in.dval;break;
		  default: k=-1;
		}
		break;
	  case CMD:
	  default:	k=-1;
	}
	//printf("M2M returning %d\n",k);
	return(k);
}

static void freeMultival(int type,multival *v)
{
	if( type==ENUM && v->eval!=NULL ) { free(v->eval);v->eval=NULL; }
}


// return 0 if ok, -1 on error
static int paramGetGenericValue(paramlist *PL,const char *name,int type,multival *v)
{
int h,k;
multival dummy;
int ownerCalling;
	if( PL==NULL ) return(-999);
#ifdef PARALLEL
	pthread_mutex_lock(&PL->lock);
	ownerCalling=PL->owner_defined && pthread_equal(PL->owner,pthread_self());
#endif
	k=0;
	if( (h=paramFind(PL,name,ownerCalling?0:READ,type))<0 ) k=h; // param not found? access? type?
	else{
#ifdef PARALLEL
		// do we have to ask? we are not the owner?
		// we do not need to make sure there is an owner PL->ownerDefined
		if( (PL->p[h].access & ASK) && !ownerCalling ) {
			dummy.ival=0;
			// ATTENTION PL->p can change while we wait
			k=askForParameter(PL,h,GET,dummy); // for GET, the dummy is not used
		}
#endif
		k=multivalToMultival(PL->p[h].type,PL->p[h].v,type,v);
	}
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


//
// Convert a multival into a string
// if buf is NULL, then return a strdup()
// return NULL on error
//
static char *multivalToString(int type,multival v,char *buf,int size)
{
char *b;
char inbuf[50];
	b=NULL;
	switch( type ) {
	  case ENUM:	b=v.eval; break;
	  case INT:		sprintf(inbuf,"%d",v.ival); b=inbuf; break;
	  case DOUBLE:	sprintf(inbuf,"%12.12g",v.dval); b=inbuf; break;
	  case CMD:
	  default:		;
	}
	if( b==NULL ) return(b);
	if( buf ) { strncpy(buf,b,size-1);buf[size-1]=0;return(buf); }
	return( strdup(b) );
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//
// Create a new paramlist
//
paramlist *paramCreate(void)
{
int i;
paramlist *PL;
	PL=(paramlist *)malloc(sizeof(paramlist));
	if( PL==NULL ) { printf("[PARAM] PL OOM!\n");return(NULL); }

	PL->nb=INIT_NB_PARAM;
	PL->p=(paraminfo *)malloc(PL->nb * sizeof(paraminfo));
	if( PL->p==NULL ) { printf("[PARAM] P OOM!\n");return(NULL); }

	// init as unused
	for(i=0;i<PL->nb;i++) {
		PL->p[i].name=NULL;
		PL->p[i].type=__UNUSED;
#ifdef PARALLEL
		PL->p[i].next=-1;
		PL->p[i].request=NONE;
		PL->p[i].status=0;
#endif
	}

#ifdef PARALLEL
	PL->first_request=-1;
	PL->owner_defined=0;
    pthread_mutex_init(&PL->lock,NULL);
    pthread_cond_init(&PL->processed,NULL);

	for(i=0;i<MAX_COND;i++) PL->conds[i]=NULL;
#endif

	return(PL);
}

//
// define the owner of the paramlist, for signaling
//
#ifdef PARALLEL
void paramSetOwner(paramlist *PL,pthread_t tid)
{
	if( PL==NULL ) return;
	PL->owner_defined=1;
	PL->owner=tid;
}
#endif

//
// Free a paramlist
//
// make sure the owner thread is dead before freeing a paramlist
//
void paramFree(paramlist *PL)
{
int i;
	if( PL==NULL ) return;

	for(i=0;i<PL->nb;i++) {
		if( PL->p[i].type!=__UNUSED ) {
			if( PL->p[i].name==NULL ) {
				printf("paramFree : NAME IS NULL!!!!!!!!!!!!!!! \n");
			}
			paramUndefine(PL,PL->p[i].name);
		}
	}

#ifdef PARALLEL
    pthread_mutex_destroy(&PL->lock);
    pthread_cond_destroy (&PL->processed);
#endif

	free(PL->p);
	free(PL);
}


//
// Dump a parameter list
//
void paramDump(paramlist *PL)
{
int i,nbu;
paraminfo *p;
	if( PL==NULL ) return;

#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
#endif

	// count unused
	nbu=0;
	for(i=0;i<PL->nb;i++) {
		if( PL->p[i].type==__UNUSED ) nbu++;
	}

#ifdef PARALLEL
	printf("--- dump nb=%d (%d used, %d unused) owner=0x%08lx ---\n",PL->nb,PL->nb-nbu,nbu,
		(long unsigned int)(PL->owner_defined?PL->owner:0));
#else
	printf("--- dump nb=%d (%d used, %d unused) ---\n",PL->nb,PL->nb-nbu,nbu);
#endif

	p=PL->p;
	for(i=0;i<PL->nb;i++,p++) {
		if( PL->p[i].type==__UNUSED ) continue;

		printf("(param %3d) ",i);
		
		switch(p->type) {
		  case ENUM:
			printf("E%s %-20s [%s]",acc2s(p->access),p->name,p->u.e.def?p->u.e.def:(NULL));
#ifdef PARALLEL
			if( !(p->access&ASK) ) printf(" = %s\n",p->v.eval?p->v.eval:"(NULL)");
#endif
			printf("\n");
			break;
		  case INT:
			printf("I%s %-20s [%d;;%d;;%d]",acc2s(p->access),p->name,p->u.i.min,p->u.i.max,p->u.i.step);
#ifdef PARALLEL
			if( !(p->access&ASK) ) printf(" = %d",p->v.ival);
#endif
			printf("\n");
			break;
		  case DOUBLE:
			printf("D%s %-20s [%g;;%g]",acc2s(p->access),p->name,p->u.d.min,p->u.d.max);
#ifdef PARALLEL
			if( !(p->access&ASK) ) printf(" = %g\n",p->v.dval);
#endif
			printf("\n");
			break;
		  case CMD:
			printf("C%s %-20s\n",acc2s(p->access),p->name);
			break;
		  default:
			printf("? Unknown type %d\n",p->type);
			break;
		}
	}
#ifdef PARALLEL
    pthread_mutex_unlock(&PL->lock);
#endif
}

//
// Define a parameter
//
// it is allowed ro redefine a parameter, to change its access or constraints
//
// (return 0 if ok, -1=already exists, -2=OOM)
//
//
int paramDefineString(paramlist *PL,const char *name,param_access access)
{
	return( paramDefineEnum(PL,name,access,NULL) );
}


// def can be null.
int paramDefineEnum(paramlist *PL,const char *name,param_access access,char *def)
{
int h,k;
	if( PL==NULL ) return(-999);
#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,ANY))<0 ) {
		// new parameter!
		if( (h=paramFindUnused(PL))<0 ) k=-2; // no more space (impossible!)
		else{
			PL->p[h].name=strdup(name);
			PL->p[h].type=ENUM;
			PL->p[h].access=access;
			PL->p[h].v.eval=NULL;
			if( def )	PL->p[h].u.e.def=strdup(def);
			else		PL->p[h].u.e.def=NULL;;
		}
	}else{
		if( PL->p[h].type!=ENUM ) k=-3;
		else{
			// already defined parameter. changing def.
			if( PL->p[h].u.e.def ) free( PL->p[h].u.e.def );
			if( def )	PL->p[h].u.e.def=strdup(def);
			else		PL->p[h].u.e.def=NULL;;
		}
	}
#ifdef PARALLEL
    pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


int paramDefineInt(paramlist *PL,const char *name,param_access access,int min,int max,int step)
{
int h,k;
	if( PL==NULL ) return(-999);
#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,ANY))<0 ) {
		// new parameter!
		if( (h=paramFindUnused(PL))<0 ) k=-2;
		else{
			PL->p[h].name=strdup(name);
			PL->p[h].type=INT;
			PL->p[h].access=access;
			PL->p[h].v.ival=min;
			PL->p[h].u.i.min=min;
			PL->p[h].u.i.max=max;
			PL->p[h].u.i.step=step;
		}
	}else{
		if( PL->p[h].type!=INT ) k=-3;
		else{
			// already defined. changing limits and access
			PL->p[h].access=access;
			PL->p[h].u.i.min=min;
			PL->p[h].u.i.max=max;
			PL->p[h].u.i.step=step;
		}
	}
	// on pourrait avoir a revalider la valeur courante du parametre ici...
#ifdef PARALLEL
    pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}

int paramDefineDouble(paramlist *PL,const char *name,param_access access,double min,double max)
{
int h,k;
	if( PL==NULL ) return(-999);
#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,ANY))<0 ) {
		// new parameter!
		if( (h=paramFindUnused(PL))<0 ) k=-2; // no more space (impossible!)
		else{
			PL->p[h].name=strdup(name);
			PL->p[h].type=DOUBLE;
			PL->p[h].v.dval=min;
			PL->p[h].access=access;
			PL->p[h].u.d.min=min;
			PL->p[h].u.d.max=max;
		}
	}else{
		if( PL->p[h].type!=DOUBLE ) k=-3;
		else{
			// already defined. changing limits and access
			PL->p[h].access=access;
			PL->p[h].u.d.min=min;
			PL->p[h].u.d.max=max;
			// on pourrait avoir a revalider la valeur courante du parametre ici...
		}
	}
#ifdef PARALLEL
    pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


int paramDefineCommand(paramlist *PL,const char *name,param_access access)
{
int h,k;
	if( PL==NULL ) return(-999);
#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,ANY))<0 ) {
		// new parameter!
		if( (h=paramFindUnused(PL))<0 ) k=-2; // no more space (impossible!)
		else{
			PL->p[h].name=strdup(name);
			PL->p[h].type=CMD;
			PL->p[h].access=access;
		}
	}else{
		if( PL->p[h].type!=CMD ) k=-3;
		else{
			// already defined. changing access
			PL->p[h].access=access;
		}
	}
#ifdef PARALLEL
    pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}



//
// free a parameter. return 0 if ok, -1 if param does not exists
//
int paramUndefine(paramlist *PL,const char *name)
{
int h,k;
paraminfo *p;
	if( PL==NULL ) return(-999);
	if( name==NULL ) return(-998);
#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,ANY))<0 ) k=-1; // unknown param
	else{
		p=PL->p+h;

		if( p->name ) { free(p->name);p->name=NULL; }
		switch( p->type ) {
		  case ENUM:
			if( p->v.eval ) { free(p->v.eval);p->v.eval=NULL; }
			if( p->u.e.def ) { free(p->u.e.def);p->u.e.def=NULL; }
			break;
		  case INT:
		  case DOUBLE:
		  case CMD:
		  case __UNUSED:
		  default: ;
		}
		p->type=__UNUSED;
	}
#ifdef PARALLEL
    pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


//
// generic value SET
// the value val has a type which can be different than the parameter.
// It that case, there is a conversion.
//
static int paramSetGenericValue(paramlist *PL,const char *name,int type,multival val)
{
int h,k;
int ownerCalling;
int wakeupNeeded;
	if( PL==NULL ) return(-999);
#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
	ownerCalling= PL->owner_defined && pthread_equal(PL->owner,pthread_self());
#endif
	wakeupNeeded=0;
	k=0;
	if( (h=paramFind(PL,name,ownerCalling?0:WRITE,ANY))<0 ) k=h; // param not found? access?
	else{
#ifdef PARALLEL
		// do we have to ask? we are not the owner?
		// don't care if owner is not defined. important for initialization
		if( (PL->p[h].access & ASK) && !ownerCalling ) {
			k=multivalToMultival(type,val,PL->p[h].type,&PL->p[h].newv);
			if( k==0 ) k=askForParameter(PL,h,SET,PL->p[h].newv);
		}else{
#endif
			freeMultival(PL->p[h].type,&PL->p[h].v);
			k=multivalToMultival(type,val,PL->p[h].type,&PL->p[h].v);
#ifdef PARALLEL
			if( PL->p[h].request!=NONE ) {
				// we are setting the value of a request... (we must be the owner)
				// reset the request and note that wakeup will be needed when done
				PL->p[h].request=NONE;
				wakeupNeeded=1;
			}
		}
#endif
	}
#ifdef PARALLEL
    pthread_mutex_unlock(&PL->lock);
#endif
	// signal anyone waiting for requests to be processed.
	// this means that processing requests requires that paramSet* be called for each parameter,
	// using SET or GET does not matter.
	if( wakeupNeeded ) {
#ifdef PARALLEL
		if( !ownerCalling ) printf("[PARAM] WAKUP and not OWNER!!!!!!!!! IMPOSSIBLE!!!!!!!!!! BUG!!!!!!!!\n");
		pthread_cond_broadcast(&PL->processed);
#endif
	}
	return(k);
}


//
// Set the value of a parameter
//
// return 0 if ok, -1 if invalid, -2,...
//
// also good for Enum, Int or Double (uses sscanf on val)
int paramSetString(paramlist *PL,const char *name,char *val)
{
multival v;
	v.eval=val;
	return( paramSetGenericValue(PL,name,ENUM,v) );
}

int paramSetInt(paramlist *PL,const char *name,int val)
{
multival v;
	v.ival=val;
	return( paramSetGenericValue(PL,name,INT,v) );
}


int paramSetDouble(paramlist *PL,const char *name,double val)
{
multival v;
	v.dval=val;
	return( paramSetGenericValue(PL,name,DOUBLE,v) );
}

// to invoke, a command must grant read or write access
int paramInvokeCommand(paramlist *PL,const char *name)
{
int h,k;
int wakeupNeeded;
int ownerCalling;
	if( PL==NULL ) return(-999);
#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
	wakeupNeeded=0;
	ownerCalling=PL->owner_defined && pthread_equal(PL->owner,pthread_self());
	k=0;
	if( (h=paramFind(PL,name,READ|WRITE,CMD))<0 ) k=h; // param not found? access? type?
	else{
		// do we have to ask? we are not the owner? owner_defined not essential
		if( (PL->p[h].access & ASK) && !ownerCalling ) {
			multival v;
			v.ival=0; // not important.
			k=askForParameter(PL,h,SET,v);
		}else{
			// for a command, nothing happens if we do not need to ask or if we are the owner.
			if( PL->p[h].request!=NONE ) {
				// reset the request and note that wakeup will be needed when done
				PL->p[h].request=NONE;
				wakeupNeeded=1;
			}
		}
	}
    pthread_mutex_unlock(&PL->lock);
	if( wakeupNeeded ) {
		pthread_cond_broadcast(&PL->processed);
	}
	return(k);
#else
	return(-1); // nothing to do if parallel is not used.
#endif
}


#ifdef PARALLEL
// to invoke, a command must grant read or write access
//
// this simple reset the request field and signal the main, if needed (usually needed).
//
// this should be called by the owner.
//
int paramRequestDone(paramlist *PL,char *name)
{
int h,k;
int wakeupNeeded;
	if( PL==NULL ) return(-999);
    pthread_mutex_unlock(&PL->lock);
	wakeupNeeded=0;
	k=0;
	if( (h=paramFind(PL,name,READ|WRITE,ANY))<0 ) k=h; // param not found? access? type?
	else{
		// do we have to ask? we are not the owner?
		if( PL->owner_defined && pthread_equal(PL->owner,pthread_self()) ) {
			// for a command, nothing happens if we do not need to ask or if we are the owner.
			if( PL->p[h].request!=NONE ) {
				PL->p[h].request=NONE;
				wakeupNeeded=1;
			}
		}
	}
    pthread_mutex_unlock(&PL->lock);
	if( wakeupNeeded ) {
		pthread_cond_broadcast(&PL->processed);
	}
	return(k);
}
#endif










//
// Get the value of a parameter
//
//
// if buf==NULL -> return the value as a strdup string, or NULL on error
// if buf!=NULL -> fill the buffer upto size-1 char (nul terminated), return buf or NULL on error
// also good for enum, int, double
char *paramGetString(paramlist *PL,const char *name,char *buf,int size)
{
int h;
char *b;
int ownerCalling;
	if( PL==NULL ) return(NULL);
#ifdef PARALLEL
    pthread_mutex_lock(&PL->lock);
	ownerCalling= PL->owner_defined && pthread_equal(PL->owner,pthread_self());
#endif
	b=NULL;
	if( (h=paramFind(PL,name,ownerCalling?0:READ,ANY))<0 ) b=NULL; // param not found? access?
	else{
#ifdef PARALLEL
		// do we have to ask? we are not the owner? no need to have an owner yet
		if( (PL->p[h].access & ASK) && !ownerCalling ) {
			multival dummy;
			dummy.ival=0;
			if( askForParameter(PL,h,GET,dummy) ) b=NULL; // for GET, the dummy is not used
		}
#endif
		b=multivalToString(PL->p[h].type,PL->p[h].v,buf,size);
	}
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(b);
}



// return 0 if ok, -1 on error
int paramGetInt(paramlist *PL,const char *name,int *val)
{
int k;
multival w;
	if( (k=paramGetGenericValue(PL,name,INT,&w)) ) return(k);
	*val=w.ival;

	return(0);
}

int paramGetDouble(paramlist *PL,const char *name,double *val)
{
int k;
multival w;
	if( (k=paramGetGenericValue(PL,name,DOUBLE,&w)) ) return(k);
	*val=w.dval;
	return(0);
}


//
// Check the existence, availability, and validity
//
// 1 if exists, 0 if does not exists
int paramExists(paramlist *PL,char *name)
{
int k;
	if( PL==NULL ) return(0);
#ifdef PARALLEL
	pthread_mutex_lock(&PL->lock);
#endif
	k=paramFind(PL,name,0,ANY);
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(k>=0); // param foun?
}


// 1 if READ or WRITE access is true (works for commands)
int paramAvailable(paramlist *PL,char *name)
{
int h,k;
	if( PL==NULL ) return(0);
#ifdef PARALLEL
	pthread_mutex_lock(&PL->lock);
#endif
	k=1; // assume all is good
	if( (h=paramFind(PL,name,READ|WRITE,ANY))<0 ) k=0; // param not found? access?
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


// get the type, return 0 on error
param_type paramType(paramlist *PL,char *name)
{
int h,k;
	if( PL==NULL ) return(ANY);
#ifdef PARALLEL
	pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,ANY))<0 ) k=ANY; // param not found, type ANY
	else k=PL->p[h].type;
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


// get the access mask, return 0 on error
param_access paramAccess(paramlist *PL,char *name)
{
int h,k;
	if( PL==NULL ) return(0);
#ifdef PARALLEL
	pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,ANY))<0 ) k=0; // param not found, so no access
	else k=PL->p[h].access;
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


//
// Verify if value is allowed for this parameter
// if parameter does not exists, then not allowed...
// return 1 if value is valid, 0 if invalid.
//
// also good for enum, int, double
int paramValidString(paramlist *PL,char *name,char *val)
{
int h,k;
paraminfo *p;
int ival;
double dval;
	if( PL==NULL ) return(0);
	if( val==NULL ) return(0);
#ifdef PARALLEL
	pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,ANY))<0 ) k=0; // param not found?
	else{
		p=PL->p+h;
		switch( p->type ) {
		  case ENUM: k= ( whichEnumValue(p->u.e.def,val)>=0 ); break;
		  case INT:
			k=( sscanf(val," %d",&ival)==1 && validValueInt(p,ival) ); break;
		  case DOUBLE:
			k=( sscanf(val," %lg",&dval)==1 && validValueDouble(p,dval) ); break;
		  case CMD:
		  default:
			;
		}
	}
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


int paramValidInt(paramlist *PL,char *name,int val)
{
int h,k;
	if( PL==NULL ) return(0);
#ifdef PARALLEL
	pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,INT))<0 ) k=0; // param not found? type?
	else{
		k=validValueInt(PL->p+h,val);
	}
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}


// also good for enum, int, double
int paramValidDouble(paramlist *PL,char *name,double val)
{
int h,k;
	if( PL==NULL ) return(0);
#ifdef PARALLEL
	pthread_mutex_lock(&PL->lock);
#endif
	k=0;
	if( (h=paramFind(PL,name,0,DOUBLE))<0 ) k=0; // param not found? type?
	k=validValueDouble(PL->p+h,val);
#ifdef PARALLEL
	pthread_mutex_unlock(&PL->lock);
#endif
	return(k);
}




#ifdef PARALLEL
// return 0 if ok, -1 if oomem
int paramAddCond(paramlist *PL,pthread_cond_t *c)
{
int i;
	if( PL==NULL ) return(-2);
	for(i=0;i<MAX_COND;i++) if( PL->conds[i]==NULL ) break;
	if( i==MAX_COND ) return(-1);
	PL->conds[i]=c;
	return(0);
}

// return 0 if ok, -1 if cond not found
int paramRemoveCond(paramlist *PL,pthread_cond_t *c)
{
int i,k;
	if( PL==NULL ) return(-2);
	k=-1;
	for(i=0;i<MAX_COND;i++) if( PL->conds[i]==c ) { PL->conds[i]=NULL;k=0; }
	return(k);
}


//
// This is called by the owner of the list
// return NONE if no request are to be processed
// return SET if a SET request is available (name and value are defined)
// return GET if a GET request is available (name is defined)
//
// Normally, the owner does some work with the parameter, then calls
// paramSetValue() with the correct value. A signal will be sent to the waiters of this request.
//
// the owner MUST call paramSetValue on the parameter given, otherwise main will wait forever.
// it can also call Done
//
// val can be NULL, but not name
//
//
param_request paramNextRequest(paramlist *PL,char **name,multival *val)
{
param_request k;
	if( PL==NULL ) return(NONE);
	pthread_mutex_lock(&PL->lock);
	k=NONE;
	if( PL->first_request>=0 ) {
		int h=PL->first_request;
		//printf("[owner] we have a request %d(SET=%d) for parameter '%s'\n", PL->p[h].request,SET,PL->p[h].name);
		*name=PL->p[h].name;
		k=PL->p[h].request; // must be SET or GET
		if( val ) {
			if( k==SET ) *val = PL->p[h].newv;
			else		 *val = PL->p[h].v; // copy of the current value of the parameter
		}
		// remove this request
		PL->first_request=PL->p[h].next;
		PL->p[h].next=-1;
		//printf("[owner] removed request from queue. it is processed. k=%d\n",k);
	}
	pthread_mutex_unlock(&PL->lock);
	return(k);
}



#endif




