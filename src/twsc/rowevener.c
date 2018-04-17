/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    rowevener.c                                       
DESCRIPTION:cell row evener functions.
CONTENTS:   rowevener()
	    decide_right_most_in_class()
	    dump_rowlength( flag )
		int flag ;
	    kick_off_cells_to_next_row( row , length_diff ) 
		int row , length_diff ;
	    borrow_cell_from_other_row( row , last_cell_rite )
		int row , last_cell_rite ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) rowevener.c (Yale) version 4.7 12/5/91" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "feeds.h"
#include "readpar.h"

/* global variables */
INT exact_feedsG ;

/* global references */
extern BOOL do_not_even_rowsG ;
extern BOOL no_row_lengthsG ;
extern BOOL ignore_feedsG ;

/* static definitions */
static INT *expect_row_rightS , *expect_row_lengthS ;
static INT *cell_kickedS ;


rowevener()
{

INT i , n , row , *addnum , add_to_this_row , total_remaining ;
INT row_left , row_right , length_diff , last_cell_rite ;
INT expect_extra , total_extra ;
INT total_cells_length , total_desire_length , total_feed_length ;
CBOXPTR cellptr , lastptr ;


total_feed_length = 0 ;
total_desire_length = 0 ;
total_cells_length = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    n = 0 ;
    addnum = fdcel_needG[row] ;
    for( i = 1 ; i <= chan_node_noG ; i++ ) {
	n += addnum[i] ;
    }
    fdcel_addedG[row] = n ;

    if (!ignore_feedsG)
        total_feed_length += n * fdWidthG ;

    row_left = barrayG[row]->bxcenter + barrayG[row]->bleft ;
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    row_right   = cellptr->cxcenter + cellptr->tileptr->right ;
    total_cells_length += row_right - row_left ;
    total_desire_length += barrayG[row]->desire ;
}

total_remaining = total_feed_length ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    add_to_this_row = total_remaining / (numRowsG - row + 1) ;
    total_remaining -= add_to_this_row ;
    barrayG[row]->desire = barrayG[row]->orig_desire + add_to_this_row ;
}
exact_feedsG = 1 ;
if( !do_not_even_rowsG ) {
    even_the_rows(!ignore_feedsG,FALSE) ;
    if( even_rows_maximallyG ){
	even_the_rows(!ignore_feedsG,TRUE) ;
    }
}
assign_row_to_pin() ;
no_row_lengthsG = 0 ;
findunlap(1) ;
no_row_lengthsG = 0 ;

decide_right_most_in_class() ;
return ;
}


decide_right_most_in_class()
{

INT n , row , row_right ;
CBOXPTR cellptr ;

n = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    row_right = cellptr->cxcenter + cellptr->tileptr->right ;
    if( row_rite_classG[row] > n ) {
	right_most_in_classG[++n] = row_right ;
    } else if( right_most_in_classG[n] < row_right ) {
	right_most_in_classG[n] = row_right ;
    }
}
}


#ifdef NEEDED
dump_rowlength( flag )
int flag ;
{

FILE *fp ;
CBOXPTR cellptr ;
int row , before , after , rowleft ;

if( flag == 0 ) {
    fp = TWOPEN( "rowlen.dat" , "w", ABORT ) ;
} else {
    fp = TWOPEN( "rowlen.dat" , "a", ABORT ) ;
}
fprintf(fp," row before_add  feednum after_add difference");
fprintf(fp," last_cell_length\n" ) ;
for( row = 1 ; row <= numRows ; row++ ) {
    rowleft = barray[row]->bxcenter + barray[row]->bleft ;
    cellptr = carray[ pairArray[row][ pairArray[row][0] ] ] ;
    before = cellptr->cxcenter + cellptr->tileptr->right - rowleft ;
    after  = before + fdcel_added[row] * fdWidth ;
    fprintf(fp," %3d %10d %7d %8d %10d %10d\n", row , before ,
	fdcel_added[row] , after , after - expect_row_length[row] ,
	cellptr->clength ) ;
}
TWCLOSE(fp) ;
}
#endif
