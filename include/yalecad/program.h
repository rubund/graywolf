/* ----------------------------------------------------------------- 
"@(#) program.h version 1.5 3/5/92"
FILE:	    program.h                                       
DESCRIPTION:Insert file for program utility routines.
DATE:	    Mar 16, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef PROGRAM_H
#define PROGRAM_H

#include <yalecad/base.h>

extern char *YinitProgram( P3(char *name, char *version, 
    VOID  (*introTextFunction)() ) ) ;
/*
Function:
    Returns string with program name, version, and compile date.
    This string is built with YmsgG (actually a global note G suffix ).
    The user may perform various initialization in introTextFunction. 
    There are no arguments to introTextFunction but user may
    get at the return string by peeking at YmsgG.  This routine also
    starts the elapsed timer.
*/

extern YexitPgm( P1(INT status) ) ;
/* 
Function:
    Exit a program gracefully.  It always outputs a message with
    the number of errors and warnings during the execution.  A
    nonzero status means the execution failed.  Status follows the
    normal UNIX rules.  Exit program also calls the debug write
    function to create a dbg file if debug has been set.
*/

extern char *YgetProgName() ;
/* 
Function:
    Returns the program name given in YinitProgram.
*/

#endif /* PROGRAM_H */
