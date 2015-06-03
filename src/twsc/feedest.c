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
FILE:	    feedest.c                                       
DESCRIPTION:feedthru estimation.
CONTENTS:   feedest()
	    re_estimate_feed_penalty( )
	    estimate_pass_thru_penalty( row1 , row2 )
		INT row1 , row2 ;
	    update_feedest( net )
		INT net ;
	    free_up_feedest_malloc()
	    update_segment_data( segptr )
		SEGBOXPTR segptr ;
	    PINBOXPTR makeSTpt( net , ptr1 , ptr2 )
		INT net ;
		PINBOXPTR ptr1 , ptr2 ;
	    SEGBOXPTR makeseg( lowptr , highptr )
		PINBOXPTR lowptr , highptr ;
	    dbg_cost()
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
	    Tue Mar 12 17:08:44 CST 1991 - added back missing
		computation.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) feedest.c (Yale) version 4.9 3/12/91" ;
#endif
#endif

#define FEEDS_VARS

#include "standard.h"
#include "groute.h"
#include "feeds.h"
#include "main.h"

/* global definitions */
INT *rowfeed_penaltyG ;

extern BOOL absolute_minimum_feedsG ;

/* static definitions */
static DOUBLE *fd_estimateS ;
static INT *min_feedS ;
static INT *row_flagS ;
static INT chip_width_penaltyS ;
static INT *est_min_ratioS ;


feedest()
{

DOUBLE ratio ;
INT net , row , toprow , botrow, maxdesire ;
DBOXPTR dimptr ;
PINBOXPTR ptr ;

fd_estimateS = (DOUBLE *)Ysafe_calloc( numChansG + 1, sizeof(DOUBLE) ) ;
min_feedS = (INT *)Ysafe_calloc( numChansG + 1, sizeof(INT) ) ;
row_flagS = (INT *)Ysafe_calloc( numChansG + 1, sizeof(INT) ) ;
rowfeed_penaltyG = (INT *)Ysafe_malloc( ( numChansG + 1 ) * sizeof(INT) ) ;
est_min_ratioS = (INT *)Ysafe_malloc( numChansG * sizeof(INT) ) ;

maxdesire = barrayG[1]->desire ;
for( row = 2 ; row <= numRowsG ; row++ ) {
    if( maxdesire < barrayG[row]->desire ) {
	maxdesire = barrayG[row]->desire ;
    }
}

ratio = (DOUBLE)(maxdesire) / (DOUBLE)( barrayG[numRowsG]->bycenter -
			  barrayG[1]->bycenter + rowHeightG ) ;
ratio = 1.0 / ratio ;  /* to fix Kai-Win's bug */

if( absolute_minimum_feedsG ) {
    if( ratio >= 1.0 ) {
	chip_width_penaltyS = barrayG[numRowsG]->bycenter -
			  barrayG[1]->bycenter + rowHeightG ;
    } else {
	chip_width_penaltyS = maxdesire ;
    }
} else {
    /* chip_width_penaltyS = 2.0 * (DOUBLE) rowHeightG ; */
    chip_width_penaltyS = 6.9 * (DOUBLE) rowHeightG ;
}



if( !absolute_minimum_feedsG ) {
    add_Lcorner_feedG = TRUE ;
    /* Carl */
    for( row = 1 ; row <= numRowsG ; row++ ) {
	if( FeedInRowG[row] == 0 ) {
	    add_Lcorner_feedG = FALSE ;
	    break ;
	}
    }
    /* Carl */
} else {
    add_Lcorner_feedG = FALSE ;
}
for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr = netarrayG[net] ;
    if( !(dimptr->pins) ) {
	continue ;
    }
    switch( dimptr->numpins ) {
    case 0 :
    case 1 :
	break ;
    case 2 :
	ptr = dimptr->pins ;
	toprow = ptr->row ;
	botrow = ptr->next->row ;
	if( toprow > botrow ) {
	    for( row = botrow + 1 ; row < toprow ; row++ ) {
		fd_estimateS[row]++ ;
		min_feedS[row]++ ;
	    }
	    if( ABS( ptr->xpos - ptr->next->xpos ) >=
				    average_feed_sepG ) {
		fd_estimateS[botrow] += 0.5 ;
		fd_estimateS[toprow] += 0.5 ;
	    }
	} else if( toprow < botrow ) {
	    for( row = toprow + 1 ; row < botrow ; row++ ) {
		fd_estimateS[row]++ ;
		min_feedS[row]++ ;
	    }
	    if( ABS( ptr->xpos - ptr->next->xpos ) >=
				    average_feed_sepG ) {
		fd_estimateS[botrow] += 0.5 ;
		fd_estimateS[toprow] += 0.5 ;
	    }
	}
	row_flagS[botrow] = 0 ;
	row_flagS[toprow] = 0 ;
	break ;
    case 3 :
    case 4 :
    case 5 :
	ptr = dimptr->pins ;
	toprow = botrow = ptr->row ;
	row_flagS[toprow] = 1 ;
	for( ptr = ptr->next ; ptr ; ptr = ptr->next ) {
	    row = ptr->row ;
	    if( row < botrow ) {
		botrow = row ;
	    }
	    if( row > toprow ) {
		toprow = row ;
	    }
	    row_flagS[row] = 1 ;
	}
	for( row = botrow + 1 ; row < toprow ; row++ ) {
	    if( row_flagS[row] ) {
		fd_estimateS[row]++ ;
		row_flagS[row] = 0 ;
	    } else {
		fd_estimateS[row] += 1.5 ;
		min_feedS[row]++ ;
	    }
	}
	if( toprow > botrow ) {
	    fd_estimateS[botrow] += 0.5 ;
	    fd_estimateS[toprow] += 0.5 ;
	}
	row_flagS[botrow] = 0 ;
	row_flagS[toprow] = 0 ;
	break ;
    default :
	ptr = dimptr->pins ;
	toprow = botrow = ptr->row ;
	row_flagS[toprow] = 1 ;
	for( ptr = ptr->next ; ptr ; ptr = ptr->next ) {
	    row = ptr->row ;
	    if( row < botrow ) {
		botrow = row ;
	    }
	    if( row > toprow ) {
		toprow = row ;
	    }
	    row_flagS[row] = 1 ;
	}
	for( row = botrow + 1 ; row < toprow ; row++ ) {
	    if( row_flagS[row] ) {
		fd_estimateS[row]++ ;
		row_flagS[row] = 0 ;
	    } else {
		fd_estimateS[row] += 1.5 ;
		min_feedS[row]++ ;
	    }
	}
	if( toprow > botrow ) {
	    fd_estimateS[botrow]++ ;
	    fd_estimateS[toprow]++ ;
	}
	row_flagS[botrow] = 0 ;
	row_flagS[toprow] = 0 ;
	break ;
    }
}

estimate_pass_thru_penalty( 1 , numRowsG ) ;

}

re_estimate_feed_penalty()
{

INT i , n , row , row_rite , excess_fd , *old_penalty ;
DOUBLE ratio , factor ;
FEED_DATA *feedptr ;
CBOXPTR cellptr ;

old_penalty = (INT *)Ysafe_malloc( numChansG * sizeof(INT) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    old_penalty[row] = rowfeed_penaltyG[row] ;
    feedptr = feedpptrG[row] ;
    n = 0 ;
    for( i = 1 ; i <= chan_node_noG ; i++ ) {
	n += feedptr[i]->needed ;
    }
    fd_estimateS[row] = n ;
}
estimate_pass_thru_penalty( 1 , numRowsG ) ;

if( !enough_built_in_feedG ) {
    return ;
}
for( row = 1 ; row <= numRowsG ; row++ ) {
    rowfeed_penaltyG[row] = 
	( 2 * old_penalty[row] + 3 * rowfeed_penaltyG[row] ) / 5 ;
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    row_rite = cellptr->cxcenter + cellptr->tileptr->right ;
    if( fdWidthG > 0 ) {
	excess_fd = fdcel_addedG[row] + 1 - 
	    ( right_most_in_classG[ row_rite_classG[row] ] - row_rite ) /
								fdWidthG;
    } else {
	excess_fd = fdcel_addedG[row] + 1 - 
	    ( right_most_in_classG[ row_rite_classG[row] ] - row_rite ) ;
    }
    if( excess_fd <= 0 ) {
	continue ;
    }
    ratio = (DOUBLE)(fdcel_addedG[row]) / (DOUBLE)(FeedInRowG[row]) ;
    if( ratio <= 0.01 ) {
	factor = 4 ;
    } else if( ratio <= 0.03 ) {
	factor = 8 ;
    } else if( ratio <= 0.05 ) {
	factor = 15 ;
    } else if( ratio <= 0.08 ) {
	factor = 25 ;
    } else if( ratio <= 0.10 ) {
	factor = 30 ;
    } else if( ratio <= 0.20 ) {
	factor = 40 ;
    } else {
	factor = 50 ;
    }
    rowfeed_penaltyG[row] += factor * ratio * (DOUBLE)(rowHeightG) ;
}
/*
fp = TWOPEN("vcost.dat", "a", ABORT ) ;
fprintf(fp, " row newcost oldcost\n" ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    fprintf(fp," %3d %7d %7d\n", row,
	rowfeed_penaltyG[row], old_penalty[row] ) ;
}
TWCLOSE(fp) ;
*/
}

#ifdef Carl
estimate_pass_thru_penalty( row1 , row2 )
INT row1 , row2 ;
{

INT row ;
DOUBLE ratio ;
if( row1 < 1 ) {
    row1 = 1 ;
}
if( row2 > numRowsG ) {
    row2 = numRowsG ;
}
ratio = (DOUBLE) implicit_feed_count / (DOUBLE) TotRegPins ;
if( ratio > 0.0 ) {
    ratio += 0.1 ;
}
if( ratio > 1.0 ) {
    ratio = 1.0 ;
}
for( row = row1 ; row <= row2 ; row++ ) {
    if( !absolute_minimum_feeds ) {
	rowfeed_penaltyG[row] = (2.0 - ratio * 1.2) * rowHeightG ;
    } else {
	rowfeed_penaltyG[row] = chip_width_penaltyS ;
    }
}
}
#else

estimate_pass_thru_penalty( row1 , row2 )
INT row1 , row2 ;
{

INT row ;
DOUBLE actual , estimate , act_est_ratio ;
if( row1 < 1 ) {
    row1 = 1 ;
}
if( row2 > numRowsG ) {
    row2 = numRowsG ;
}

for( row = row1 ; row <= row2 ; row++ ) {
    actual = FeedInRowG[row] ;
    estimate = fd_estimateS[row] ;
    if( actual > min_feedS[row] && !absolute_minimum_feedsG ) {
	if( estimate > 0.0 ) {
	    act_est_ratio = actual / estimate ;
	} else {
	    act_est_ratio = INT_MAX ;
	    /*  fix by Carl 1/25/89  */
	}
	if( act_est_ratio >= 4 ) {
	    rowfeed_penaltyG[row] = rowHeightG * 0.3 ;
	} else if( act_est_ratio >= 1.0 ) {
	    rowfeed_penaltyG[row] =
	    ( 0.9 - 0.2 * ( act_est_ratio - 1.0 ) ) * rowHeightG ;
	} else if( act_est_ratio <= 0.5 ) {
	    rowfeed_penaltyG[row] = 6.9 * rowHeightG ;
	} else {
	    rowfeed_penaltyG[row] = 
	    ( 0.9 + 12.0 * ( 1.0 - act_est_ratio ) ) * rowHeightG ;
	}
    } else {
	rowfeed_penaltyG[row] = chip_width_penaltyS ;
    }
}
}
#endif


update_feedest( net )
INT net ;
{

DBOXPTR dimptr ;
SEGBOXPTR seg ;
PINBOXPTR ptr ;
INT row , toprow , botrow , row1 , row2 ;


dimptr = netarrayG[net] ;
if( !(dimptr->pins) ) {
    return ;
}
switch( dimptr->numpins ) {
case 0 :
case 1 :
case 2 :
    return ;
case 3 :
case 4 :
case 5 :
    ptr = dimptr->pins ;
    toprow = botrow = ptr->row ;
    row_flagS[toprow] = 1 ;
    for( ptr = ptr->next ; ptr ; ptr = ptr->next ) {
	row = ptr->row ;
	if( row < botrow ) {
	    botrow = row ;
	}
	if( row > toprow ) {
	    toprow = row ;
	}
	row_flagS[row] = 1 ;
    }
    for( row = botrow + 1 ; row < toprow ; row++ ) {
	if( row_flagS[row] ) {
	    fd_estimateS[row]-- ;
	    row_flagS[row] = 0 ;
	} else {
	    fd_estimateS[row] -= 1.5 ;
	}
    }
    if( toprow > botrow ) {
	fd_estimateS[botrow] -= 0.5 ;
	fd_estimateS[toprow] -= 0.5 ;
    }
    row_flagS[botrow] = 0 ;
    row_flagS[toprow] = 0 ;
    break ;
default :
    ptr = dimptr->pins ;
    toprow = botrow = ptr->row ;
    row_flagS[toprow] = 1 ;
    for( ptr = ptr->next ; ptr ; ptr = ptr->next ) {
	row = ptr->row ;
	if( row < botrow ) {
	    botrow = row ;
	}
	if( row > toprow ) {
	    toprow = row ;
	}
	row_flagS[row] = 1 ;
    }
    for( row = botrow + 1 ; row < toprow ; row++ ) {
	if( row_flagS[row] ) {
	    fd_estimateS[row]-- ;
	    row_flagS[row] = 0 ;
	} else {
	    fd_estimateS[row] -= 1.5 ;
	}
    }
    if( toprow > botrow ) {
	fd_estimateS[botrow]-- ;
	fd_estimateS[toprow]-- ;
    }
    row_flagS[botrow] = 0 ;
    row_flagS[toprow] = 0 ;
    break ;
}
for( seg = netsegHeadG[net]->next ; seg ; seg = seg->next ) {
    row1 = seg->pin1ptr->row ;
    row2 = seg->pin2ptr->row ;
    if( row1 == row2 ) {
	if( ABS( seg->pin1ptr->pinloc - seg->pin2ptr->pinloc ) <= 1 ) {
	    continue ;
	} else {
	    fd_estimateS[row1]++ ;
	}
    } else {
	for( row = row1 + 1 ; row < row2 ; row++ ) {
	    fd_estimateS[row]++ ;
	}
	if( seg->pin1ptr->pinloc < NEITHER ) {
	    fd_estimateS[row1]++ ;
	}
	if( seg->pin2ptr->pinloc > NEITHER ) {
	    fd_estimateS[row2]++ ;
	}
	if( add_Lcorner_feedG && seg->switchvalue != nswLINE ) {
	    fd_estimateS[row1] += 0.5 ;
	    fd_estimateS[row2] += 0.5 ;
	}
    }
}
for( ptr = steinerHeadG[net]->next; ptr ; ptr = ptr->next ) {
    fd_estimateS[ ptr->row ]++ ;
}

estimate_pass_thru_penalty( botrow , toprow ) ;

}


free_up_feedest_malloc()
{

Ysafe_free( fd_estimateS ) ;
Ysafe_free( min_feedS ) ;
Ysafe_free( row_flagS ) ;
Ysafe_free( rowfeed_penaltyG ) ;
Ysafe_free( est_min_ratioS ) ;
}


update_segment_data( segptr )
SEGBOXPTR segptr ;
{
PINBOXPTR ptr1 , ptr2 ;

ptr1 = segptr->pin1ptr ;
ptr2 = segptr->pin2ptr ;
segptr->flag = NEW ;
if( ptr1->row < ptr2->row ) {
    segptr->pin1ptr = ptr1 ;
    segptr->pin2ptr = ptr2 ;
    if( ABS( ptr1->xpos - ptr2->xpos ) >= average_feed_sepG ) {
	segptr->switchvalue = swL_up ;
    } else {
	segptr->switchvalue = nswLINE ;
    }
} else if( ptr1->row > ptr2->row ) {
    segptr->pin2ptr = ptr1 ;
    segptr->pin1ptr = ptr2 ;
    if( ABS( ptr1->xpos - ptr2->xpos ) >= average_feed_sepG ) {
	segptr->switchvalue = swL_up ;
    } else {
	segptr->switchvalue = nswLINE ;
    }
} else if( (INT) ptr1->pinloc == BOTCELL && (INT) ptr2->pinloc == TOPCELL ) {
    segptr->pin1ptr = ptr1 ;
    segptr->pin2ptr = ptr2 ;
    segptr->switchvalue = nswLINE ;
} else if( (INT) ptr1->pinloc == TOPCELL && (INT) ptr2->pinloc == BOTCELL ) {
    segptr->pin1ptr = ptr2 ;
    segptr->pin2ptr = ptr1 ;
    segptr->switchvalue = nswLINE ;
} else if( ptr1->xpos <= ptr2->xpos ) {
    segptr->pin1ptr = ptr1 ;
    segptr->pin2ptr = ptr2 ;
    segptr->switchvalue = nswLINE ;
} else {
    segptr->pin1ptr = ptr2 ;
    segptr->pin2ptr = ptr1 ;
    segptr->switchvalue = nswLINE ;
}
}


PINBOXPTR makeSTpt( net , ptr1 , ptr2 )
INT net ;
PINBOXPTR ptr1 , ptr2 ;
{

PINBOXPTR ptr ;

ptr = (PINBOXPTR)Ysafe_malloc( sizeof(PINBOX) ) ;
ptr->xpos = ptr1->xpos ;
ptr->ypos = ptr2->ypos ;
ptr->terminal = 0 ;
if( ptr1->terminal == 0 ) {
    ptr->newx = ptr1->newx ;
} else {
    ptr->newx = ptr1->terminal ;
}
    /* the 'newx' field is temporary used for storing the
	y-direction terminal that steiner points refered to */
ptr->row = ptr2->row ;
ptr->pinloc = NEITHER ;
ptr->newy = 3 * ptr->row ;
ptr->net = net ;
ptr->adjptr = (ADJASEGPTR)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;
ptr->next = steinerHeadG[net]->next ;
steinerHeadG[net]->next = ptr ;
return( ptr ) ;
}


SEGBOXPTR makeseg( lowptr , highptr )
PINBOXPTR lowptr , highptr ;
{
ADJASEG *adj1, *adj2 ;
SEGBOXPTR segptr ;

segptr = ( SEGBOXPTR )Ysafe_malloc( sizeof(SEGBOX) ) ;
if( lowptr->row < highptr->row ) {
    segptr->pin1ptr = lowptr ;
    segptr->pin2ptr = highptr ;
    segptr->switchvalue = swL_up ;
} else if( lowptr->row > highptr->row ) {
    segptr->pin2ptr = lowptr ;
    segptr->pin1ptr = highptr ;
    segptr->switchvalue = swL_up ;
} else if( (INT) lowptr->pinloc == BOTCELL && (INT) highptr->pinloc == TOPCELL ) {
    segptr->pin1ptr = lowptr ;
    segptr->pin2ptr = highptr ;
    segptr->switchvalue = swL_up ;
} else if( (INT) lowptr->pinloc == TOPCELL && (INT) highptr->pinloc == BOTCELL ) {
    segptr->pin1ptr = highptr ;
    segptr->pin2ptr = lowptr ;
    segptr->switchvalue = swL_up ;
} else if( lowptr->xpos < highptr->xpos ) {
    segptr->pin1ptr = lowptr ;
    segptr->pin2ptr = highptr ;
    segptr->switchvalue = nswLINE ;
} else {
    segptr->pin2ptr = lowptr ;
    segptr->pin1ptr = highptr ;
    segptr->switchvalue = nswLINE ;
}
adj1 = (ADJASEGPTR)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;
adj2 = (ADJASEGPTR)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;
adj1->segptr = segptr ;
adj2->segptr = segptr ;
adj1->next =  lowptr->adjptr->next ;
adj2->next = highptr->adjptr->next ;
 lowptr->adjptr->next = adj1 ;
highptr->adjptr->next = adj2 ;
segptr->next = netsegHeadG[lowptr->net]->next ;
netsegHeadG[lowptr->net]->next = segptr ;
return( segptr ) ;
}


dbg_cost()
{
FILE *fp ;
INT row ;
DOUBLE cost, ratio ;

fp = TWOPEN("vcost.dat", "w", ABORT ) ;
fprintf(fp, " rowHeightG = %d\n", rowHeightG ) ;
fprintf(fp, " row  cost actual estimate    ratio\n" ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    cost = (DOUBLE)(rowfeed_penaltyG[row]) / (DOUBLE)(rowHeightG) ;
    ratio = (DOUBLE)(FeedInRowG[row]) / fd_estimateS[row] ;
    fprintf(fp, " %3d %5.2f %6d %8.2f %8.2f\n", row, cost,
		FeedInRowG[row], fd_estimateS[row], ratio ) ;
}
TWCLOSE(fp) ;
}


dbx_fdpen()
{

FILE *fp ;
FEED_DATA *feedptr ;
INT row , i , s ;

fp = TWOPEN("pen.dat" , "w", ABORT ) ;
fprintf(fp," row min_feedSS fd_estimateS act_feed needed\n" ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    feedptr = feedpptrG[row] ;
    s = 0 ;
    for( i = 1 ; i <= chan_node_noG ; i++ ) {
	s += feedptr[i]->needed ;
    }
    fprintf(fp," %3d %8d %8d %8d %6d\n", row, min_feedS[row],
		    (INT)fd_estimateS[row] , FeedInRowG[row] , s ) ;
}
TWCLOSE(fp) ;
}
