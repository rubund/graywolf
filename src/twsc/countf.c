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
FILE:	    countf.c                                       
DESCRIPTION:feed estimation code.
CONTENTS:   countf()
	    prep_feed_count_1()
	    prep_feed_count()
	    insert_row( flag )
		INT flag ;
	    feed_situation( row , net )
		INT row , net ;
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) countf.c (Yale) version 4.10 2/23/92" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "feeds.h"
#include "main.h"
#include "ucxxglb.h"
#include "readpar.h"
#include "parser.h"
#include "pads.h"


/* global definitions */
INT *xfeeds_in_rowG ;
INT est_fdsG ;

/* static variables */
static INT offsetS ;
static INT est_final_feedS ;
static INT **row_mapS ;
static BOOL num_callS = FALSE ;

countf()
{

PINBOXPTR netptr ;
INT check , a , value ;
INT net , i , row , toprow , botrow ;
INT TOP_ROW_FOR_NET ;
INT orig_toprow , orig_botrow ;
DOUBLE fd_reduction = 0.0 ;   /* needed initialization WPS */


if( num_callS == FALSE ) {
    num_callS = TRUE ;

    prep_feed_count_1() ;
    row_mapS = ( INT ** ) Ysafe_malloc( (numRowsG + 2) * 
					    sizeof(INT *));
    for( row = 0 ; row <= numRowsG + 1 ; row++ ) {
	row_mapS[row] = (INT *) Ysafe_malloc( 3 *
 				    sizeof( INT ) );
    }
    insert_row( 0 ) ;
} else {
    prep_feed_count() ;
    insert_row( 1 ) ;
}


for( net = 1 ; net <= numnetsG ; net++ ) {
    if(!(netptr = netarrayG[net]->pins)) {
	continue ;
    }
    for( i = 0 ; i <= numRowsG + 1 ; i++ ) {
	row_mapS[ i ][ RITE_MOST ] = NO_PINS ;
	row_mapS[ i ][ FEED_FLAG ] = NOT_DONE ;
    }
    for( ; netptr ; netptr = netptr->next ) {
	row = netptr->row ;
	row_mapS[ row ][ RITE_MOST ] = netptr->xpos ;
	if( row_mapS[ row ][ FEED_FLAG ] == NOT_DONE ) {
	    row_mapS[ row ][ FEED_FLAG ] = 
				feed_situation( row , net ) ;
	}
    }
    botrow = 0 ;
    toprow = numRowsG + 1 ;
    for( ; botrow <= numRowsG + 1 ; botrow++ ) {
	if( row_mapS[ botrow ][ RITE_MOST ] != NO_PINS ) {
	    break ;
	}
    }
    for( ; toprow >= 0 ; toprow-- ) {
	if( row_mapS[ toprow ][ RITE_MOST ] != NO_PINS ) {
	    break ;
	}
    }

    orig_toprow = toprow ;
    orig_botrow = botrow ;
    for( netptr = netarrayG[net]->pins ;netptr; netptr=netptr->next ) {
	if( netptr->cell <= numcellsG ) {
	    continue ;
	}
	row = netptr->row ;
	if( row <= orig_toprow && row >= orig_botrow ) {
	    continue ;
	}
	if( row > toprow ) {
	    toprow = row ;
	}
	if( row < botrow ) {
	    botrow = row ;
	}
	row_mapS[ row ][ RITE_MOST ] = netptr->xpos ;
	if( row_mapS[ row ][ FEED_FLAG ] == NOT_DONE ) {
	    row_mapS[ row ][ FEED_FLAG ] = feed_situation( row , net ) ;
	}
    }

    if( toprow != botrow ) {
	row = toprow ;
	TOP_ROW_FOR_NET = YES ;
	for( row-- ; row >= botrow ; row-- ) {
	    if( row_mapS[ row ][ RITE_MOST ] == NO_PINS ) {
		continue ;
	    }

	    if( TOP_ROW_FOR_NET == YES ) {
		if( row_mapS[ toprow ][ FEED_FLAG ] == 1 || 
			    row_mapS[ toprow ][ FEED_FLAG ] == 3 ) {
		    feeds_in_rowG[toprow]++ ;
		    xfeeds_in_rowG[toprow]++ ;
		}
	    }
	    if( row > botrow ) {
		if( row_mapS[row][ FEED_FLAG ] != 4 ) {
		    feeds_in_rowG[row]++ ;
		    xfeeds_in_rowG[row]++ ;
		}
	    }
	    if( row == botrow ) {
		if( row_mapS[botrow][ FEED_FLAG ] == 2 || 
			     row_mapS[botrow][ FEED_FLAG ] == 3 ) {
		    feeds_in_rowG[botrow]++ ;
		    xfeeds_in_rowG[botrow]++ ;
		}
	    }
	    check = toprow - row - 1 ;
	    if( check != 0 ) {
		for( a = toprow - 1 ; a > row ; a--){
		    feeds_in_rowG[a]++ ;
		    xfeeds_in_rowG[a]++ ;
		}
	    }
	    toprow = row ;
	    TOP_ROW_FOR_NET = NO ;
	}
    } else {   /* toprow == botrow */
	if( row_mapS[toprow][ FEED_FLAG ] == 3 ) {
	    feeds_in_rowG[toprow]++ ;
	    xfeeds_in_rowG[toprow]++ ;
	}
    }
}

est_fdsG = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    if( xfeeds_in_rowG[row] > 0 ) {
	est_fdsG += xfeeds_in_rowG[row] ;
    }
}
if( iterationG == 3 ) {
    est_final_feedS = est_fdsG * 
		    /*
		    exp( -(log(250000.0) - log(22000.0)));
		    */
		    exp( -(log(280000.0) - log(22000.0)));
    /* these are the numbers for the dec12k circuit   --Carl  */
    if( est_final_feedS - offsetS < 0 ) {
	est_final_feedS = offsetS ;
    }
}
if( iterationG >= 3 ) {
    if( est_fdsG > 0 ) {
	if( est_fdsG - offsetS > 0 ) {
	    fd_reduction = ((DOUBLE) est_final_feedS - (DOUBLE) offsetS) / 
				((DOUBLE) est_fdsG - (DOUBLE) offsetS);
	} else {
	    fd_reduction = 0.0 ;
	}
	if( fd_reduction < 1.0 ) {
	    if( iterationG <= 100 ) {
		fd_reduction += (1.0 - fd_reduction) * (((double)iterationG / 100.0 ) *
							    ((double)iterationG / 100.0) );
	    } else {
		fd_reduction = 1.0 ;
	    }
	} else {
	    fd_reduction = 1.0 ;
	}
    } else {
	fd_reduction = 0.0 ;
    }
    /* else fd_reduction = 0.0 ; */
}


fd_reduction *= 1.25 ;  /* we no longer add just min. feeds */

for( row = 1 ; row <= numRowsG ; row++ ) {
    value = (INT)(fd_reduction * feeds_in_rowG[row]) ; 
    if( value > 0 && fdWidthG > 0 ) {
	feeds_in_rowG[row] = 1 + value ;
    } else {
	feeds_in_rowG[row] = 0 ;
    }
}
value = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    value += feeds_in_rowG[row] ;
}

return( value * fdWidthG ) ;
}



prep_feed_count_1()
{

INT row ;
DOUBLE total_row_len ;

total_row_len = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    total_row_len += (DOUBLE) barrayG[row]->blength ;
}

xfeeds_in_rowG = (INT *) Ysafe_malloc( (1 + numRowsG) * sizeof(INT) );

offsetS = - (INT)((DOUBLE) implicit_feed_countG * 0.33) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    feeds_in_rowG[row] = (INT)( (DOUBLE) offsetS *
	    ((DOUBLE)(barrayG[row]->blength) / total_row_len) ) ;
    xfeeds_in_rowG[row] = 0 ;
}

return ;
}


prep_feed_count()
{

INT row ;
DOUBLE total_row_len ;

total_row_len = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    total_row_len += (DOUBLE) barrayG[row]->blength ;
}
for( row = 1 ; row <= numRowsG ; row++ ) {
    feeds_in_rowG[row] = (INT)( (DOUBLE) offsetS *
	    ((DOUBLE)(barrayG[row]->blength) / total_row_len) ) ;
    xfeeds_in_rowG[row] = 0 ;
}

return ;
}




insert_row( flag )
INT flag ;
{

PINBOXPTR pinptr ;
INT row , cell , pad ;
INT yc , yb , blk , xc ;


for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    row = carrayG[cell]->cblock ;
    for( pinptr = carrayG[cell]->pins ; pinptr ;pinptr=pinptr->nextpin ) {
	pinptr->row = row ;
    }
}
if( flag == 0 ) {
    for( pad = numcellsG + 1 ; pad <= lastpadG ; pad++ ) {
	for( pinptr = carrayG[pad]->pins;pinptr;pinptr=pinptr->nextpin ) {
	    xc = carrayG[pad]->cxcenter + pinptr->txpos[1] ;
	    yc = carrayG[pad]->cycenter + pinptr->typos[1] ;
	    for( blk = 1 ; blk <= numRowsG ; blk++ ) {
		if( barrayG[blk]->bycenter >= yc ) {
		    break ;
		}
	    }
	    if( blk == 1 ) {
		yb = barrayG[1]->bycenter + barrayG[1]->bbottom ;
		if( yc < yb ) {
		    row = 0 ;
		} else {
		    row = 1 ;
		}
	    } else if( blk > numRowsG ) {
		if( yc > barrayG[numRowsG]->bycenter +
			 barrayG[numRowsG]->btop ) {
		    row = numRowsG + 1 ;
		    pinptr->pinloc = BOTCELL ;
		} else {
		    row = numRowsG ;
		}
	    } else {
		yb = barrayG[blk]->bycenter ;
		if( yb - yc < yc - barrayG[blk - 1]->bycenter ) {
		    row = blk ;
		} else {
		    row = blk - 1 ;
		}
	    }
	    pinptr->row = row ;
	}
	if( carrayG[pad]->padptr->padside == 0 && rowsG > 0 ) {
	    /* case of a port on left or right and below 1st row */
	    /* reset 'row' for purposes of this 'if' only */
	    if( row == 0 ){
		row = 1 ;
	    } else if( row == numRowsG + 1 ) {
		row = numRowsG ;
	    }
	    /*  must then be a left or right side port  */
	    /*  Note: there is only one pin per port, obviously  */
	    if( xc < barrayG[row]->bxcenter ) {
		carrayG[pad]->padptr->padside = L ;
	    } else {
		carrayG[pad]->padptr->padside = R ;
	    }
	}
    }
}
return ;
}




feed_situation( row , net )
INT row , net ;
{

PINBOXPTR nptr ;
INT pinup, pindown , pinloc ;

/*
 *   The following code is used for the return value:
 *
 *       pins top,bot but NOT truly equivalent:   0
 *       pins only on top of row for this net:    1
 *       pins only on bot of row for this net:    2
 *       pins top,bot but no passthru for net:    3
 *       passthru exists for this net:            4
 */
pinup   = 0 ;
pindown = 0 ;
for( nptr = netarrayG[net]->pins; nptr; nptr = nptr->next ) {
    if( nptr->row != row ) {
	continue ;
    }
    if( nptr->eqptr ) {
	if( nptr->eqptr->unequiv == 1 ) {
	    continue ;
	}
    }
    pinloc = nptr->pinloc ;
    if( pinloc == NEITHER ) {
	return(4) ;
    }
    if( pinloc > NEITHER ) {
	pinup = 1 ;
    } else {
	pindown = 1 ;
    }
}
if( pinup == 1 && pindown == 0 ) {
    return(1) ;
} else if( pinup == 0 && pindown == 1 ) {
    return(2) ;
} else if( pinup == 1 && pindown == 1 ) {
    return(3) ;
} else {
    return(0);
}
}
