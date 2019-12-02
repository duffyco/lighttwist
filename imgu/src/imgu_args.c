
//
// manage arguments from a string
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imgu.h"

//#define VERBOSE

/**
 * format simple:
 * -xyz (q1)
 *
 *  supporte les "" et '' autour des strings...
 *
 */



#define MODE_NORMAL		0
#define MODE_GUILL		1
#define MODE_APO		2

typedef struct {
	char *nom; // ex: -in
	int nbarg; // 1,2,3 following arguments
	int offset; // offset in the final parametre table
} swi;


/**
 *
 * argvFromString
 *
 * retourne un tableau de taille n+1, ou n est le nb de tokens
 * Le premier element args[0] contient une string avec le nombre d'arguments (n)
 * Chaque element ne peut etre NULL
 * SVP rappelle argsFree pour liberer la memoire apres utilisation.
 *
 */
char **argvFromString(const char *param) { return argvFromStringX(param," "); }
char **argvFromStringX(const char *param,const char *separator)
{
int nb;
char *token,*save,*string;
char **argv;
char buf[20];
	if( param==NULL ) return(NULL);
	//
	// compte le nb de parametres
	//	
	nb=0;
    string=strdup(param);
    token = strtok_r(string,separator,&save);
    while( token ) {
		nb++;
		if( save==NULL ) break; // special mac...
		token = strtok_r(NULL, separator,&save);
    }
	free(string);

#ifdef VERBOSE
	printf("nb de param=%d\n",nb);
#endif

	argv=(char **)malloc((nb+1)*sizeof(char *));
	sprintf(buf,"%d",nb);
	argv[0]=strdup(buf);

    string=strdup(param);
    token = strtok_r(string,separator,&save);
	nb=0;
    while( token ) {
		nb++;
		argv[nb]=strdup(token);
		if( save==NULL ) break; // special mac...
		token = strtok_r(NULL, separator,&save);
    }
	free(string);
	
	return(argv);
}

/**
 *
 * stringToArgs
 *
 * retourne un tableau de taille n+1, ou n est le nb de switch '-'
 * Le premier element args[0] contient une string avec le nombre d'arguments (n)
 * Chaque element peut etre NULL si l'argument n'est pas specifie
 * SVP rappelle freeArgs pour liberer la memoire apres utilisation.
 *
 */
char **argsFromString(const char *def,const char *param)
{
int i;
int nb;
int off;
char *token,*save,*string;
swi *args; // compte des arguments pour chaque option
int nbvalues;
char **values;
char buf[20];
	if( def==NULL || param==NULL ) return(NULL);
	//
	// compte le nb de parametres
	//	
	nb=0;
    string=strdup(def);
    token = strtok_r(string," ",&save);
    while( token ) {
		if( token[0]=='-' ) nb++;
		if( save==NULL ) break; // special mac...
		switch( save[0] ) {
		  case '"': token = strtok_r(NULL, "\"",&save);break;
		  case '\'': token = strtok_r(NULL, "'",&save);break;
		  case '<': token = strtok_r(NULL, "<>",&save);break;
		  case '[': token = strtok_r(NULL, "[]",&save);break;
		  default: token = strtok_r(NULL, " ",&save);break;
		}
    }
    free(string);

#ifdef VERBOSE
	printf("nb de param=%d\n",nb);
#endif
	args=(swi *)malloc(nb*sizeof(swi));

	//
	// structure interne pour compte le nb d'arguments par parametre
	//
	nb=0;off=1;
    string=strdup(def);
    token = strtok_r(string," ",&save);
    while( token ) {
		if( token[0]=='-' ) {
			args[nb].nom=strdup(token);
			args[nb].nbarg=0;
			args[nb].offset=off;
			nb++;
		}else{
			if( nb>0 ) {
				args[nb-1].nbarg++;
				off++;
			}else printf("Definition must begin with - switch (skipping)\n");
		}
		if( save==NULL ) break;
		switch( save[0] ) {
		  case '"': token = strtok_r(NULL, "\"",&save);break;
		  case '\'': token = strtok_r(NULL, "'",&save);break;
		  case '<': token = strtok_r(NULL, "<>",&save);break;
		  case '[': token = strtok_r(NULL, "[]",&save);break;
		  default: token = strtok_r(NULL, " ",&save);break;
		}
    }
    free(string);

	nbvalues=0;
	for(i=0;i<nb;i++) nbvalues+=args[i].nbarg;
	values=(char **)malloc((nbvalues+1)*sizeof(char *));

	sprintf(buf,"%d",nbvalues);
	values[0]=strdup(buf); // the number of arguments in [0]

	for(i=1;i<=nbvalues;i++) values[i]=NULL;

#ifdef VERBOSE
	printf("total nb of values is %d\n",nbvalues);
	for(i=0;i<nb;i++) printf("arg %d is '%s' with %d arguments, off=%d\n",i,args[i].nom,args[i].nbarg,args[i].offset);
#endif

	//
	// process param according to args[]
	//
	int j;
	int current=-1;
	int err=0;
    string=strdup(param);
    token = strtok_r(string," ",&save);
    while( token ) {
		if( current<0 ) {
			// expecting a switch
			if( token[0]=='-' ) {
				// trouve le token dans arg...
				for(i=0;i<nb;i++) if( strcmp(args[i].nom,token)==0 ) { current=i;j=0;break; }
				if( i==nb ) {
					printf("Unkown switch '%s'. Must be one of:",token);
					for(i=0;i<nb;i++) printf(" %s",args[i].nom); printf("\n");
					err=1;
				}
			}else{
				printf("Expected a switch and got '%s'. Must be one of:",token);
				for(i=0;i<nb;i++) printf(" %s",args[i].nom); printf("\n");
				err=1;
			}
		}else{
			// expecting an argument of switch 'current', arg number is j
			if( j>=args[current].nbarg ) {
				// this should not happen since we reset j when large enough
				printf("Expected only %d arguments after '%s', got '%s'\n",
					args[current].nbarg,args[current].nom,token);
				err=1;
			}else{
#ifdef VERBOSE
				printf("Switch %s has value %d [param %d] set to '%s'\n",
					args[current].nom,j,args[current].offset+j,token);
#endif
				values[args[current].offset+j]=strdup(token);
				j++;
				if( j==args[current].nbarg ) { current=-1; } // end of this switch.
			}
		}
		if( err ) break;
		if( save==NULL ) break;
		switch( save[0] ) {
		  case '"': token = strtok_r(NULL, "\"",&save);break;
		  case '\'': token = strtok_r(NULL, "'",&save);break;
		  default: token = strtok_r(NULL, " ",&save);break;
		}
    }
    free(string);

	if( current!=-1 ) {
		printf("Expected %d more arguments to the switch '%s'\n",args[current].nbarg-j,args[current].nom);
		err=1;
	}

	// cleanup
	for(i=0;i<nb;i++) free(args[i].nom);
	free(args);

	// if any error happenned, free all values and return NULL
	if( err ) { argsFree(values); values=NULL; }

	return(values);
}

//
// free whatever was returned by stringsToArgs()
//
void argsFree(char **args)
{
int n;
	if( args==NULL ) return;
	n=atoi(args[0]);

	int i;
	for(i=0;i<n+1;i++) if( args[i] ) free(args[i]);
	free(args);
	return;
}


/*****
int main(int argc,char *argv[])
{
char **args;

	args=argsFromString(
		"-in <q1 q2> -out q2 -comp-ress 1 -flag [SAVE_16_BITS | SAVE_8_BITS] -test 'x \"y z ' -info \"a b c' d e\" -scale 0.567 0.4 0.7 0.8",
		"-in Q1 -out Q2 -flag SAVE -info 'bonjour blub' -scale 0.77 0.1 0.2 99");

	if( args ) {
		int n=atoi(args[0]);
		int i;
		for(i=0;i<=n;i++) {
			printf("value %d is '%s'\n",i,args[i]?args[i]:"(NULL)");
		}
	}

	argsFree(args);
	return(0);
}
*****/


