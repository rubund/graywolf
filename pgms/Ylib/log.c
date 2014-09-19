/* ----------------------------------------------------------------- 
FILE:	    log.c 
DESCRIPTION:This file contains routines for logging the
	    execution times of the program.
CONTENTS:   Ylog_msg( message )
		char *message ;
	    Ylog_start( message )
		char *message ;
DATE:	    May  7, 1989 - original coding.
REVISIONS:  Aug  7, 1989 - Moved to libary.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) log.c version 1.2 8/28/90" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/file.h>

static char cktNameS[LRECL] = " " ; 

/* put a message in the log file */
Ylog_msg( message )
char *message ;
{
    INT timestamp ;           /* seconds since Jan 1, 1970 */
    char *time, *YcurTime() ; /* YcurTime returns ASCII time string */
    char filename[LRECL] ;    /* log file name */
    FILE *fp ;                /* logfile */

    sprintf( filename, "%s.log", cktNameS ) ;    
    fp = TWOPEN( filename, "a", ABORT ) ;
    time = YcurTime( &timestamp ) ;
    fprintf( fp, "time %8d %s:%s\n", timestamp, time, message ) ;
    TWCLOSE( fp ) ;
} /* end log */

Ylog_start( design, message )
char *design ;
char *message ;
{
    INT timestamp ;           /* seconds since Jan 1, 1970 */
    char *time, *YcurTime() ; /* YcurTime returns ASCII time string */
    char filename[LRECL] ;    /* log file name */
    FILE *fp ;                /* logfile */

    strcpy( cktNameS, design ) ;
    sprintf( filename, "%s.log", cktNameS ) ;    
    fp = TWOPEN( filename, "a", ABORT ) ;
    time = YcurTime( &timestamp ) ;
    fprintf( fp,"####################################################\n");
    fprintf( fp, "time %8d %s:%s\n", timestamp, time, message ) ;
    TWCLOSE( fp ) ;
} /* end log */
