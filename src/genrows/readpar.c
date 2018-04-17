/*
 *   Copyright (C) 1991-1992 Yale University
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
FILE:	    readpar.c                                       
DESCRIPTION:read parameter file
CONTENTS:   readpar( )
DATE:	    Sat Jan 26 12:48:32 PST 1991
REVISIONS:  Sat Feb 23 00:35:01 EST 1991 - added wildcarding.
	    Thu Mar  7 03:13:07 EST 1991 - added numrows keyword.
	    Wed Apr 17 22:34:32 EDT 1991 - added rowSep keyword for
		consistency.
	    Wed Jun 12 13:34:30 CDT 1991 - now read previous .out
		file for feed percentage.
	    Fri Sep  6 15:13:23 CDT 1991 - now read previous row
		separation.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readpar.c (Yale) version 1.9 5/14/92" ;
#endif

#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>
#include <yalecad/yreadpar.h>
#include <yalecad/message.h>
#include <globals.h>

#define COMMENT '#'

static BOOL abortS = FALSE ;


static err_msg(); 
static get_defaults();
static int getnumRows();


readpar()
{

    INT line ;
    INT temp ;
    INT numtokens ;
    INT numrows ;
    BOOL onNotOff ;
    BOOL wildcard ;
    BOOL feed_percent_default ;
    BOOL row_sep_default ;
    char design[LRECL] ;
    char **tokens ;
    char *lineptr ;
    DOUBLE tempf ;

    num_rowsG = 0 ;  /* means number of rows not specified */ 
    wait_for_userG = TRUE ;
    last_chanceG = TRUE ;
    flip_alternateG = 0 ;
    feed_percent_default = TRUE ;
    row_sep_default = TRUE ;
    no_outputG = FALSE ;
    M( MSG, NULL, "\n" ) ;

    Yreadpar_init( cktNameG, USER, GENR, FALSE ) ;

    while( tokens = Yreadpar_next( &lineptr, &line, &numtokens, 
	&onNotOff, &wildcard )){
	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( strcmp( tokens[0], "feed_percentage" ) == STRINGEQ ){
	    if( numtokens == 2 ){
		tempf = atof( tokens[1] ) ;
		if( tempf < 0.0 ){
		    err_msg("feed_percentage") ;
		    continue ;
		}
		set_feed_length( tempf ) ;
		feed_percent_default = FALSE ;
	    } else {
		err_msg("feed_percentage") ;
	    }
	} else if( strcmp( tokens[0], "minimum_row_len" ) == STRINGEQ ){
	    if( numtokens == 2 ){
		temp = atoi( tokens[1] ) ;
		if( temp <= 0 ){
		    err_msg("minimum_row_len") ;
		    continue ;
		}
		set_minimum_length( temp ) ;
	    } else {
		err_msg("minimum_row_len") ;
	    }
	} else if( strcmp( tokens[0], "rowSep" ) == STRINGEQ ){
	    if( numtokens >= 2 ){
		tempf = atof( tokens[1] ) ;
		if( tempf < 0 ){
		    err_msg("rowSep") ;
		    continue ;
		}
		temp = (numtokens == 3) ? (INT)atof(tokens[2]) : 0;
		set_row_separation( tempf, temp ) ;
		row_sep_default = FALSE ;
	    } else {
		err_msg("rowSep") ;
	    }
	} else if( strcmp( tokens[0], "row_to_tile_spacing" ) == STRINGEQ ){
	    if( numtokens == 2 ){
		temp = atoi( tokens[1] ) ;
		if( temp < 0 ){
		    err_msg("row_to_tile_spacing") ;
		    continue ;
		}
		spacingG = temp ;
		set_spacing() ;
	    } else {
		err_msg("row_to_tile_spacing") ;
	    }
	} else if( strcmp( tokens[0], "numrows" ) == STRINGEQ ){
	    if( numtokens == 2 ){
		temp = atoi( tokens[1] ) ;
		if( temp < 0 ){
		    err_msg("numrows") ;
		    continue ;
		}
		num_rowsG = temp ;
	    } else {
		err_msg("numrows") ;
	    }
	} else if( strcmp( tokens[0], "flip_alternate_rows" ) == STRINGEQ ){
	    if( numtokens == 2 ){
		temp = atoi( tokens[1] ) ;
		if( temp < 0 || temp > 2 ){
		    err_msg("flip_alternate_rows") ;
		    continue ;
		}
		flip_alternateG = temp ;
	    } else {
		err_msg("flip_alternate_rows") ;
	    }
	} else if( strcmp( tokens[0], "graphics.wait" ) == STRINGEQ ){
	    if( onNotOff ){
		wait_for_userG = TRUE ;
	    } else {
		wait_for_userG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "last_chance.wait" ) == STRINGEQ ){
	    if( onNotOff ){
		last_chanceG = TRUE ;
	    } else {
		last_chanceG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "no_blk_file" ) == STRINGEQ ){
	    if( onNotOff ){
		no_outputG = TRUE ;
	    } else {
		no_outputG = FALSE ;
	    }
	/*** catch all ***/
	} else if(!(wildcard)){
	    sprintf( YmsgG, 
	    "unexpected keyword in the %s.par file at line:%d\n\t%s\n", 
	    cktNameG, line, lineptr );
	    M( ERRMSG, "readpar", YmsgG ) ;
	    Ymessage_error_count() ;
	    abortS = TRUE ;
	}
    } /* end of parsing loop */

    if( abortS ){
	YexitPgm(PGMFAIL) ;
    }
    get_defaults( feed_percent_default, row_sep_default ) ;

    numrows = getnumRows() ;
    if( numrows ){
	if( numrows != num_rowsG && num_rowsG ){
	    sprintf( YmsgG, 
	    "Number of row discrepancy between .row file(%d) and .par file(%d)\n",
	    numrows, num_rowsG ) ;
	    M( ERRMSG, "readpar", YmsgG ) ;
	    M( ERRMSG, NULL, "Using value found in .row file\n" ) ;
	}
	num_rowsG = numrows ;
    }

    /* now check for user errors */
    check_user_data() ;

    if( no_outputG ){
	M( WARNMSG, "readpar", "No .blk file will be output.\n\n" ) ;
    }

} /* end readpar */

static err_msg( keyword ) 
char *keyword ;
{
    sprintf( YmsgG, "The value for %s was", keyword );
    M( ERRMSG, "readpar", YmsgG ) ;
    sprintf( YmsgG, " not properly entered in the .par file\n");
    M( ERRMSG, NULL, YmsgG ) ;
    abortS = TRUE ;
}/* end err_msg */

static get_defaults( feed_percent_default, row_sep_default )
BOOL feed_percent_default, row_sep_default ;
{
    FILE *fp ;
    char filename[LRECL] ;
    char buffer[LRECL] ;
    char *bufferptr ;
    DOUBLE tempf ;

    sprintf( filename, "%s.out", cktNameG ) ;
    if(!(fp = TWOPEN( filename, "r", NOABORT ))){
	return ;
    }

    /* start at beginning and read till we find feed percentage */
    while( bufferptr = fgets( buffer, LRECL, fp )){
	/* remove leading blanks or tabs */
	bufferptr = Yremove_lblanks( bufferptr ) ;
	if( strncmp( bufferptr, "Feed Percentage:", 16 ) == STRINGEQ ){
	    /* skip over Feed Percentage */
	    bufferptr += 16 ;
	    /* remove any leading blanks */
	    bufferptr = Yremove_lblanks( bufferptr ) ;
	    tempf = atof( bufferptr ) ;
	    sprintf( YmsgG, 
	    "Found previous value for feed percentage:%4.2f\n", tempf ) ;
	    M( MSG, NULL, YmsgG ) ;

	    if( feed_percent_default ){
		set_feed_length( tempf ) ;
	    } else {
		M( MSG,NULL,"Overridden by .par file value for feed percentage\n" ) ;
	    }
	}
	if( strncmp( bufferptr, "Average Row Separation:", 23 ) == STRINGEQ ){
	    /* skip over Feed Percentage */
	    bufferptr += 23 ;
	    /* remove any leading blanks */
	    bufferptr = Yremove_lblanks( bufferptr ) ;
	    tempf = atof( bufferptr ) ;
	    sprintf( YmsgG, 
	    "Found previous value for row separation:%4.2f\n", tempf ) ;
	    M( MSG, NULL, YmsgG ) ;

	    if( row_sep_default ){
		set_row_separation( tempf, (INT)0 ) ;
	    } else {
		M( MSG,NULL,"Overridden by .par file value for row separation.\n" ) ;
	    }
	    break ;
	}
    }
    TWCLOSE( fp ) ;
} /* end get_defaults */

static int getnumRows()
{

    char **tokens ;         /* for parsing menu file */
    INT  numtokens ;        /* number of tokens on the line */
    int  line ;             /* count lines in input file */
    char buffer[LRECL], *bufferptr = buffer ;
    char filename[LRECL];
    FILE* fp;

    /* read net list file */
    sprintf( filename, "%s.row", cktNameG ) ;
    fp = TWOPEN( filename,"r",NOABORT) ;

    if(!(fp)){
	return(0) ;
    }
  
    line = 0 ;  /*--- initialize the line counter ---*/
  
    /*-----------  parse file ------------*/
    while( bufferptr = fgets( buffer, LRECL, fp )){

	tokens = Ystrparser( bufferptr, "\t\n/ ", &numtokens );
	if( numtokens == 0 ){
	    continue ;
	}
	line++;
    } /* end while... */

    return(line) ; 
} /* end int getnumRows() */
