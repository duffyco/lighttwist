#ifndef IMGU_ARGS_H
#define IMGU_ARGS_H

////////////////////////////////
////////////////////////////////
//
// Args (managing string arguments)
// (see imgu_args.c)
//
////////////////////////////////
////////////////////////////////
/**
 * @defgroup args Argument processing
 *
 * @ingroup imgugeneral
 *
 * This is the argument support functions
 * This breaks a string containing switch and parameters into strings in an array.
 * A definition is given, and each parameter extracted will be put in the corresponding
 * spot in the array.
 *
 *@{
 */

#ifdef __cplusplus
extern "C" {
#endif


char **argvFromString(const char *param);
char **argvFromStringX(const char *param,const char *separator);
char **argsFromString(const char *def,const char *param);
void argsFree(char **args);


#ifdef __cplusplus
}
#endif

/*@}*/








#endif
