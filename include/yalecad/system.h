/* ----------------------------------------------------------------- 
FILE:	    system.h                                       
CONTENTS:   definitions for system functions.
DATE:	    Tue Oct 29 15:05:57 EST 1991
REVISIONS:  
----------------------------------------------------------------- */
#ifndef YSYSTEM_H
#define YSYSTEM_H

#ifndef lint
static char Ysystem_HId[] = "@(#) system.h version 1.1 3/5/92" ;
#endif

/*
Ysystem -
Function:
    An abstract interface to the system call.  The first argument is
    the name of the program to be executed.  It is used in the error
    message when the system call fails for some reason.  The second
    argument is whether to abort the program or not.  The third argument
    is the actual command that is to be processed.  The fourth argument
    is the function to call upon failure.  It has no arguments.
    Ysystem return the system error code if failure; 0 otherwise.
*/
extern INT Ysystem( P4(char *program,BOOL abortFlag,char *exec_statement,
    INT (*abort_func)() ) ) ;

/*
YcopyFile -
Function:
    Copy a file from the source to the destination name.
    Aborts pgm if a failure.
*/
extern VOID YcopyFile( P2(char *sourcefile, char *destfile) ) ;

/*
YmoveFile -
Function:
    Move a file from the source to the destination name.
    Aborts pgm if a failure.
*/
extern VOID YmoveFile( P2(char *sourcefile, char *destfile) ) ;

/*
Yrm_files -
Function:
    Remove files from system.  Wildcards are available.  May be system
    dependent however.
    Never aborts pgm if a failure.
*/
extern VOID Yrm_files( P1(char *files) ) ;

/*
Ygetenv -
Function:
    Interface to getenv system function.
*/
extern char *Ygetenv( P1(char *env_var) ) ;

#endif /* YSYSTEM_H */
