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
FILE:	    getftime.c 
DESCRIPTION:This file contains a routine which returns the last time
	    a file was modified.
CONTENTS:   
DATE:	    May  8, 1989 - original coding.
REVISIONS:  Apr 29, 1990 - added message.h
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) getftime.c version 1.3 8/28/90" ;
#endif

#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ERROR    -1             /* return code for error */

INT YgetFileTime( filename )
char *filename ;
{
    struct stat info ;               /* info record */
    INT    status    ;               /* return code from stat */

    if( filename ){
	status = stat( filename, &info ) ;
	if( status == ERROR ){
	    sprintf( YmsgG, "ERROR[YgetFileTime] - %s", filename ) ;
	    perror( YmsgG ) ;
	    return( ERROR ) ;
	}
	return( info.st_mtime ) ;
    }
    return( ERROR ) ;

} /* end YgetFileTime */
