/*
 *   Copyright (C) 1989-1990 Yale University
 *
 *   This work is distributed in the hope that it will be useful; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation;
 *   either version 2 of the License,
 *   or any later version, on the following conditions:
 *
 *   (a) YALE MAKES NO, AND EXPRESSLY DISCLAIMS
 *   ALL, REPRESENTATIONS OR WARRANTIES THAT THE MANUFACTURE, USE, PRACTICE,
 *   SALE OR
 *   OTHER DISPOSAL OF THE SOFTWARE DOES NOT OR WILL NOT INFRINGE UPON ANY
 *   PATENT OR
 *   OTHER RIGHTS NOT VESTED IN YALE.
 *
 *   (b) YALE MAKES NO, AND EXPRESSLY DISCLAIMS ALL, REPRESENTATIONS AND
 *   WARRANTIES
 *   WHATSOEVER WITH RESPECT TO THE SOFTWARE, EITHER EXPRESS OR IMPLIED,
 *   INCLUDING,
 *   BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *   PARTICULAR
 *   PURPOSE.
 *
 *   (c) LICENSEE SHALL MAKE NO STATEMENTS, REPRESENTATION OR WARRANTIES
 *   WHATSOEVER TO
 *   ANY THIRD PARTIES THAT ARE INCONSISTENT WITH THE DISCLAIMERS BY YALE IN
 *   ARTICLE
 *   (a) AND (b) above.
 *
 *   (d) IN NO EVENT SHALL YALE, OR ITS TRUSTEES, DIRECTORS, OFFICERS,
 *   EMPLOYEES AND
 *   AFFILIATES BE LIABLE FOR DAMAGES OF ANY KIND, INCLUDING ECONOMIC DAMAGE OR
 *   INJURY TO PROPERTY AND LOST PROFITS, REGARDLESS OF WHETHER YALE SHALL BE
 *   ADVISED, SHALL HAVE OTHER REASON TO KNOW, OR IN FACT SHALL KNOW OF THE
 *   POSSIBILITY OF THE FOREGOING.
 *
 */

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
