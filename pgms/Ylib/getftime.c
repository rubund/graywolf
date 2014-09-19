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
