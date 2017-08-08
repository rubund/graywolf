/* ----------------------------------------------------------------- 
FILE:	    system.h                                       
CONTENTS:   definitions for system functions.
DATE:	    Tue Oct 29 15:05:57 EST 1991
REVISIONS:  
----------------------------------------------------------------- */
#ifndef YSYSTEM_H
#define YSYSTEM_H

/*
YcopyFile -
Function:
    Copy a file from the source to the destination name.
    Aborts pgm if a failure.
*/
void YcopyFile( P2(char *sourcefile, char *destfile) ) ;

/*
YmoveFile -
Function:
    Move a file from the source to the destination name.
    Aborts pgm if a failure.
*/
void YmoveFile( P2(char *sourcefile, char *destfile) ) ;

/*
Yrm_files -
Function:
    Remove files from system.  Wildcards are available.  May be system
    dependent however.
    Never aborts pgm if a failure.
*/
void Yrm_files( P1(char *files) ) ;

/*
Ygetenv -
Function:
    Interface to getenv system function.
*/
extern char *Ygetenv( P1(char *env_var) ) ;

#endif /* YSYSTEM_H */
