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
FILE:	    relpath.c                                       
DESCRIPTION:Calculate the path of a file relative to a given known 
    file.  You can access any file in that directory and below.
CONTENTS:  char *Yrelpath( known_path, rel_path )
		char *known_path, *rel_path ;
DATE:	    Apr 18, 1989 
REVISIONS:  May  8, 1989 - updated to handle ../../ constructs.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) relpath.c version 3.2 8/28/90" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/string.h>

char *Yrelpath( known_path, rel_path )
char *known_path, *rel_path ; /* known path and relative path to it */
{

    char known_fpath[LRECL] ; /* full path of known obj */
    char *ptr ;               /* used to replace obj with relative path */
    char *result ;            /* resulting path */
    char *Yfixpath(), *strrchr(), *strcat() ;
    INT  up ;              /* keeps count of backtracking up dir tree */

    /* make a copy of path */
    strcpy( known_fpath, known_path ) ;

    /* look for ./ constructs */
    up = 0 ;
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
       First look for ./  or directory local files.  That is if 
       known path is   :/twolf6/bills/tw/pgms/test 
       and rel_path    :./src
       result should be:/twolf6/bills/tw/pgms/test/src
    - - -- - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    if( strncmp( rel_path,"./", 2 ) == STRINGEQ ){
	/* back up the directory tree */
	/* update rel_path by skipping over ./ */
	rel_path += 2 ;
    }
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
       Next look for ../ or higher directory files.  That is if 
       known path is   :/twolf6/bills/tw/pgms/test 
       and rel_path    :../src
       result should be:/twolf6/bills/tw/pgms/src
    - - -- - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    /* now look for multiple ../../ skip over leading .  */
    while( strncmp( rel_path,"../", 3 ) == STRINGEQ ){
	up++ ; /* back up the directory tree */
	/* update rel_path by skipping over ../ */
	rel_path += 3 ;
    }
    /* now find matching slashes in known path */
    /* find last backslash */
    for(  ; up > 0 ; up-- ){ 
	if( ptr = strrchr( known_fpath, '/' )){
	    *ptr = EOS ;
	} else {
	    return( NULL ) ; /* problem */
	}
    }
    if( known_fpath ){
	strcat( known_fpath, "/" ) ;
	strcat( known_fpath, rel_path ) ;
	result = (char *) Ystrclone(known_fpath);
	return( result ) ;
    }
    return( NULL ) ;

} /* end Yrelpath */
