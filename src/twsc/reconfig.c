/*
 *   Copyright (C) 1989-1992 Yale University
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
FILE:	    reconfig.c                                       
DESCRIPTION:cell topology configuration with feed estimation.
CONTENTS:   INT reconfig()
DATE:	    Mar 27, 1989 
REVISIONS:  Thu Jan 31 15:56:39 EST 1991 - now only call findcostf
		if criteria is met.  Instead call recompute_wirecosts
		and recompute_timecosts.
	    Wed Jul 24 20:58:00 CDT 1991 - reset the MIN_FEED_RATIO number.
	    Fri Sep  6 15:19:05 CDT 1991 - now calculate the feed
		taper from previous run.
	    Wed Sep 11 11:27:46 CDT 1991 - added user
		output info for blocks.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) reconfig.c (Yale) version 4.11 4/2/92" ;
#endif
#endif

#include "standard.h"
#include "readpar.h"
#include "main.h"
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <yalecad/debug.h>

#if SIZEOF_VOID_P == 64
#define INTSCANSTR "%ld"
#else
#define INTSCANSTR "%d"
#endif

#define MIN_FEED_RATIO     0.66

/* global definitions */
extern BOOL no_feed_estG ;
extern INT fdWidthG ;
extern INT totalRG ;
extern INT extra_cellsG ;
extern INT *feeds_in_rowG ;

/* static definitions */
static BOOL feed_length_setS = TRUE ;
static INT feed_lengthS ;             /* the current feed length */
static INT old_feed_lengthS = 0 ; /* the feed length from the last iteration */
static BOOL print_desiredS = TRUE ;

INT reconfig()
{

    INT block ;
    INT total_desire ;

    if( print_desiredS ) {
	print_desiredS = FALSE ;

	total_desire = 0 ;
	for( block = 1 ; block <= numRowsG ; block++ ) {
	    total_desire += barrayG[block]->desire ;
	    fprintf(fpoG,"block:%d desire:%d\n",block,barrayG[block]->desire);
	}
	fprintf(fpoG,"Total Desired Length: %d\n", total_desire ) ;
    }

    /* place the pads */
    placepads() ;

    /* call this since pad placement may change costs */
    funccostG = recompute_wirecost() ;
    timingcostG = recompute_timecost() ;

    return( 0 ) ;

} /* end reconfig */


static configuref()
{
    INT row ;
    INT cell ;
    INT core_left ;
    INT core_right ;
    INT shift_amount ;
    INT extra_shift , tmp ;


    /* see if any rows would be completely filled with feeds */
    extra_shift = 0 ;
    for( row = 1 ; row <= numRowsG ; row++ ) {
	tmp = feeds_in_rowG[row] * fdWidthG ;
	if( tmp > barrayG[row]->orig_desire ) {
	    extra_shift += tmp - barrayG[row]->orig_desire ;
	}
    }

    feed_lengthS -= extra_shift ;

    shift_amount = (feed_lengthS / numRowsG) - (old_feed_lengthS / numRowsG) ;
    if( shift_amount == 0 ) {
	for( row = 1 ; row <= numRowsG ; row++ ) {
	    barrayG[row]->oldsize  = 0 ;
	}
	return ;
    }

    core_right = INT_MIN ;
    core_left  = INT_MAX ;
    for( row = 1 ; row <= numRowsG ; row++ ) {
	if( barrayG[row]->bxcenter + barrayG[row]->bleft < core_left ) {
	    core_left = barrayG[row]->bxcenter + barrayG[row]->bleft ;
	}
	totalRG += shift_amount ;
	barrayG[row]->bright  += shift_amount ;
	barrayG[row]->blength += shift_amount ;
	barrayG[row]->desire  += shift_amount ;
	barrayG[row]->oldsize  = 0 ;
	if( barrayG[row]->bxcenter + barrayG[row]->bright > core_right ) {
	    core_right = barrayG[row]->bxcenter + barrayG[row]->bright ;
	}
    }

    for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
	if( carrayG[cell]->cclass < 0 ) {
	    if( carrayG[cell]->border < 0 ) {
		carrayG[cell]->cxcenter += shift_amount ;
	    }
	}
    }

    old_feed_lengthS = feed_lengthS ;


} /* end configuref */

read_feeds( fp )
FILE *fp ;
{
    fscanf( fp , INTSCANSTR , &feed_lengthS ) ;
    feed_length_setS = TRUE ;
} /* end read_feeds */

save_feeds( fp )
FILE *fp ;
{
    fprintf( fp, "%d\n", feed_lengthS ) ;
} /* end save_feeds */

/* *********************************************************** 
    Use .pl2 to find the number of rows in the output .pl1 file.
    Read the .pl1 to find out the number of explicit feeds.
    We can then set the feed taper for each row.  First
    we assume that we have the same number of rows.  If
    we don't, then we will bin the rows to smooth the
    function and approximate.
   *********************************************************** */
BOOL read_feed_data() 
{

#define EQUAL_CASE        0   /* equal number of rows */
#define LESS_ROWS_CASE    1   /* currently have less rows */
#define MORE_ROWS_CASE    2   /* currently have more rows */

    INT row ;                 /* counter */
    INT numtokens ;           /* number of tokens on the line */
    INT pl1_rows ;            /* the number of row found in .pl2 */
    INT pl1_case ;            /* which of 3 cases we have */
    INT row_in_pl1 ;          /* the feed was found in this row */
    INT start_row ;           /* the first row in case 3 */
    INT end_row ;             /* the last row in case 3 */
    FILE *fp ;                /* current file pointer */
    DOUBLE *smooth ;          /* used to calculate MORE_ROWS case */
    DOUBLE value ;            /* distribute feed over mult. rows */
    char filename[LRECL] ;    /* name of the file */
    char buffer[LRECL] ;      /* read string into buffer */
    char *bufferptr ;         /* start of the buffer */
    char **tokens ;           /* tokens on the line */

	/* only process this file once */
    static BOOL already_processedL = FALSE ;
	/* save the result for future use. Default no feeds */
    static BOOL feed_statusL = FALSE ;

    if( already_processedL ){
	/* 
	    We only need to call routine once.
	    Return the saved result.
	*/
	return( feed_statusL ) ;
    }
    already_processedL = TRUE ;
    /* If we get here, this is the first time try to open .pl2 */
    sprintf( filename, "%s.pl2", cktNameG ) ;
    fp = TWOPEN( filename, "r", NOABORT ) ;
    if(!(fp)){
	/* no feed data present return false */
	return( FALSE ) ;
    }
    /***********************************************************
    * Read from circuitName.pl2 file.
    ***********************************************************/
    pl1_rows = 0 ;
    while( bufferptr = fgets( buffer, LRECL, fp ) ){
	tokens = Ystrparser( bufferptr, " \t\n", &numtokens ) ;
	if( numtokens != 7 ){
	    continue ;
	}
	row = atoi( tokens[0] ) ;
	if( row > 0 ){
	    /* make sure the last two fields are zero */
	    if( atoi(tokens[5]) == 0 && atoi(tokens[6]) == 0 ){
		pl1_rows = row ;
	    }
	}
    }
    TWCLOSE( fp ) ;

    if( pl1_rows <= 0 ){
	/* no valid feed data present return false */
	return( FALSE ) ;
    }

    /***********************************************************
    * Now set the case.  Easy case I. pl1_rows == numRowsG.
    * case II.  pll1_rows > numRowsG.
    * case III. pl1_rows <  numRowsG.
    ***********************************************************/
    if( pl1_rows == numRowsG ){
	pl1_case = EQUAL_CASE ;
    } else if( pl1_rows > numRowsG ){
	pl1_case = LESS_ROWS_CASE ;
	M( WARNMSG, "read_feed_data",
	"The number of rows have decreased since the last run.\n" ) ;
    } else if( pl1_rows < numRowsG ){
	pl1_case = MORE_ROWS_CASE ;
	M( WARNMSG, "read_feed_data",
	"The number of rows have increased since the last run.\n" ) ;
	/* here we need also to make a smoothing array */
	smooth = (DOUBLE *) 
	    Ysafe_malloc( (numRowsG+1) * sizeof(DOUBLE) ) ;
	for( row = 1; row <= numRowsG; row++ ){
	    smooth[row] = 0.0 ;
	}
    }
    if( pl1_case == LESS_ROWS_CASE || pl1_case == MORE_ROWS_CASE ){
	M( WARNMSG, NULL, 
	"This may affect feed estimation.  To insure the best results,\n" ) ;
	M( WARNMSG, NULL, "\trun TimberWolf at least one more time.\n\n" ) ;
    }


    /***********************************************************
    * Now read from circuitName.pl1 file to get explicit feeds.
    ***********************************************************/
    /* Now try to open .pl1 file */
    sprintf( filename, "%s.pl1", cktNameG ) ;
    fp = TWOPEN( filename, "r", NOABORT ) ;
    if(!(fp)){
	/* no feed data present return false */
	return( FALSE ) ;
    }

    /* now allocate space for feeds in row */
    feed_lengthS = 0 ;
    while( bufferptr = fgets( buffer, LRECL, fp ) ){
	tokens = Ystrparser( bufferptr, " \t\n", &numtokens ) ;
	if( numtokens != 7 ){
	    continue ;
	}
	if( strncmp( tokens[0], "twfeed", 6 ) == STRINGEQ ){
	    feed_lengthS += fdWidthG ;
	    row_in_pl1 = atoi( tokens[6] ) ;
	    switch( pl1_case ){
	    case EQUAL_CASE :
		feeds_in_rowG[row_in_pl1]++ ;
		break ;
	    case LESS_ROWS_CASE :
		row = ROUND( (DOUBLE) row_in_pl1 / 
			(DOUBLE) pl1_rows * (DOUBLE) numRowsG ) ;
		feeds_in_rowG[row]++ ;
		break ;
	    case MORE_ROWS_CASE :
		value = (DOUBLE) row_in_pl1 / 
			(DOUBLE) pl1_rows * (DOUBLE) numRowsG ;
		start_row = (INT) floor( value ) ;
		if( start_row <= 0 ){
		    start_row = 1 ;
		}
		end_row = (INT) ceil( value ) ;
		value = 1.0 / (end_row - start_row + 1) ;
		for( row = start_row; row <= end_row; row++ ){
		    smooth[row] += value ;
		}
		break ;
	    } /* end switch( pl1_case... */
	}
    } /* end while( bufferptr... */

    TWCLOSE( fp ) ;

    if( pl1_case == MORE_ROWS_CASE ){
	/* perform the rounding after all feeds have been accounted*/
	for( row = 1; row <= numRowsG; row++ ){
	    feeds_in_rowG[row] = ROUND( smooth[row] ) ;
	}
	Ysafe_free( smooth ) ;
    }
    feed_statusL = TRUE ; /* feed data is valid at this point */

    M( MSG, NULL, "\nFeed Taper:\n" ) ;
    for( row = 1; row <= numRowsG; row++ ){
	sprintf( YmsgG, "\trow:%4d num explicit feeds:%d\n",
	    row, feeds_in_rowG[row] ) ;
	M( MSG, NULL, YmsgG ) ;
    }
    M( MSG, NULL, "\n\n" ) ;

    /* now update the configuration */
    configuref() ;
    funccostG = findcostf() ;

    return( feed_statusL ) ;

} /* end read_feed_data */
