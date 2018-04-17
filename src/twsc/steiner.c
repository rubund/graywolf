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
FILE:	    steiner.c                                       
DESCRIPTION:steiner tree functions.
CONTENTS:   steiner()
	    make_net_Tree( startptr )
		PINBOXPTR startptr ;
	    redo_steiner()
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
	    Sun Jan 20 21:47:52 PST 1991 - ported to AIX.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) steiner.c (Yale) version 4.7 1/20/91" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "feeds.h"

/* global variables */
INT Max_numPinsG ;
INT *add_st_flagG ;
INT enough_built_in_feedG ;

/* global references */
extern INT *rowfeed_penaltyG ;
extern BOOL absolute_minimum_feedsG ;
SEGBOXPTR makeseg() ;

/* static definitions */
static PINBOXPTR   *vertexS ;
static INT **costS , *lowcostS , *closestS , *components , max_pinS ;

steiner()
{


DBOXPTR dptr ;
PINBOXPTR netptr ;
SEGBOXPTR segptr ;
INT net , row , i ;

max_pinS = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    if( netarrayG[net]->numpins > max_pinS ) {
	max_pinS = netarrayG[net]->numpins ;
    }
}
max_pinS++ ;
Max_numPinsG = 2 * (max_pinS + 3 * numRowsG) ;

netsegHeadG = (SEGBOXPTR *)Ysafe_malloc(
			( numnetsG+1 ) * sizeof( SEGBOXPTR ) ) ;
steinerHeadG = (PINBOXPTR *)Ysafe_malloc(
			( numnetsG+1 ) * sizeof( PINBOXPTR ) ) ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    netsegHeadG[net] = (SEGBOXPTR)Ysafe_calloc( 1, sizeof(SEGBOX) ) ;
    steinerHeadG[net]= (PINBOXPTR)Ysafe_calloc( 1, sizeof(PINBOX) ) ;
}

costS = (INT **)Ysafe_malloc( max_pinS * sizeof(INT *) ) ;
for( i = 1 ; i < max_pinS ; i++ ) {
    costS[i] = (INT *)Ysafe_malloc( max_pinS * sizeof( INT ) ) ;
}

vertexS    = (PINBOXPTR *)Ysafe_malloc( max_pinS * sizeof( PINBOXPTR ) ) ;
lowcostS   = (INT *)Ysafe_calloc( max_pinS, sizeof(INT) ) ;
closestS   = (INT *)Ysafe_calloc( max_pinS, sizeof(INT) ) ;
components = (INT *)Ysafe_malloc( max_pinS * sizeof(INT) ) ;


for( net = 1 ; net <= numnetsG ; net++ ) {
    dptr = netarrayG[net] ;
    if(!(dptr->pins)){
	continue ;
    }
    switch( dptr->numpins ) {
    case 0 :
	break ;
    case 1 :
	break ;
    case 2 :
	netptr = dptr->pins ;
	makeseg( netptr , netptr->next ) ;
	break ;
    default :
	make_net_Tree( dptr->pins ) ;
	update_feedest( net ) ;
	break ;
    }
}

#ifdef CARL_NEW
add_st_flagG = (int *)Ysafe_malloc( ( numChans + 1 ) * sizeof(int) ) ;
if( !absolute_minimum_feeds ) {
    enough_built_in_feed = 1 ;
    for( row = 1 ; row <= numRows ; row++ ) {
	add_st_flagG[row] = TRUE ;
	if( FeedInRow[row] == 0 ) {
	    enough_built_in_feed = 0 ;
	}
    }
} else {
    enough_built_in_feed = 0 ;
    for( row = 1 ; row <= numRows ; row++ ) {
	add_st_flagG[row] = FALSE ;
    }
}
#else
add_st_flagG = (INT *)Ysafe_malloc( ( numChansG + 1 ) * sizeof(INT) ) ;
if( !absolute_minimum_feedsG ) {
    enough_built_in_feedG = TRUE ;
    for( row = 1 ; row <= numRowsG ; row++ ) {
	if( FeedInRowG[row] > 0 ) {
	    add_st_flagG[row] = TRUE ;
	} else {
	    add_st_flagG[row] = FALSE ;
	    enough_built_in_feedG = FALSE ;
	}
    }
} else {
    enough_built_in_feedG = FALSE ;
    for( row = 1 ; row <= numRowsG ; row++ ) {
	add_st_flagG[row] = FALSE ;
    }
}
#endif

for( net = 1 ; net <= numnetsG ; net++ ) {
    dptr = netarrayG[net] ;
    if( dptr->numpins <= 2 ) {
	continue ;
    }
    for( netptr = dptr->pins ; netptr ; netptr = netptr->next ) {
	netptr->newy = 3 * netptr->row + netptr->pinloc - PINLOC_OFFSET ;
    }
    for( netptr = dptr->pins ; netptr ; netptr = netptr->next ) {
	if( netptr->adjptr->next->next != NULL ) {
	    mergeseg( netptr ) ;
	}
    }
    for( segptr = netsegHeadG[net] ; segptr->next ;
			    segptr = segptr->next ) {
	segptr->next->prev = segptr ;
    }
}

#define EVEN_ROW
#ifndef EVEN_ROW
for( i = 1 ; i < max_pinS ; i++ ) {
    Ysafe_free( costS[i] ) ;
}
Ysafe_free( costS ) ;
Ysafe_free( vertexS ) ;
Ysafe_free( lowcostS ) ;
Ysafe_free( closestS ) ;
Ysafe_free( components ) ;
Ysafe_free( add_st_flagG ) ;

#endif

}


make_net_Tree( startptr )
PINBOXPTR startptr ;
{

PINBOXPTR netptr , iptr , jptr ;
INT i , j , k , n , c , row , irow , jrow ;
INT x_diff , min_costS , first_new ;

n = 0 ;
for( netptr = startptr ; netptr ; netptr = netptr->next ) {
    vertexS[ ++n ] = netptr ;
}
for( i = 1 ; i <= n ; i++ ) {
    costS[i][i] = INFINITY ;
    iptr = vertexS[i] ;
    for( j = i+1 ; j <= n ; j++ ) {
	jptr = vertexS[j] ;
	x_diff   = ABS( iptr->xpos - jptr->xpos ) ;
	if( iptr->row == jptr->row ) {
	    if( iptr->pinloc == NEITHER && jptr->pinloc == NEITHER ) {
		costS[i][j] = x_diff ;
	    } else {
		if( ABS( iptr->pinloc - jptr->pinloc ) <= 1 ) {
		    costS[i][j] = x_diff * 5 / 2 ;
		} else {
		    costS[i][j] = x_diff * 5 / 2 +
				rowfeed_penaltyG[iptr->row] ;
		}
	    }
	} else {
	    c = 0 ;
	    irow = iptr->row ;
	    jrow = jptr->row ;
	    if( irow < jrow ) {
		for( row = irow + 1 ; row < jrow ; row++ ) {
		    c += rowfeed_penaltyG[row] ;
		}
	    } else {
		for( row = jrow + 1 ; row < irow ; row++ ) {
		    c += rowfeed_penaltyG[row] ;
		}
	    }
	    if( iptr->pinloc == NEITHER && jptr->pinloc == NEITHER ) {
		if( x_diff < average_feed_sepG ) {
		    costS[i][j] = x_diff  + c ;
		} else {
		    costS[i][j] = x_diff + c +
			( rowfeed_penaltyG[irow] +
			  rowfeed_penaltyG[jrow] ) / 2 ;
		}
	    } else {
		if( irow < jrow ) {
		    if( iptr->pinloc >= NEITHER && jptr->pinloc <= NEITHER ) {
			if( jptr->pinloc <= NEITHER ) {
			    costS[i][j] = x_diff * 5 / 2 + c ;
			} else {
			    costS[i][j] = x_diff * 5 / 2 + c
					+ rowfeed_penaltyG[jrow] ;
			}
		    } else {
			if( jptr->pinloc <= NEITHER ) {
			    costS[i][j] = x_diff * 5 / 2 + c
					+ rowfeed_penaltyG[irow] ;
			} else {
			    costS[i][j] = x_diff * 5 / 2 + c
					+ rowfeed_penaltyG[irow]
					+ rowfeed_penaltyG[jrow] ;
			}
		    }
		} else {
		    if( jptr->pinloc >= NEITHER && iptr->pinloc <= NEITHER ) {
			if( iptr->pinloc <= NEITHER ) {
			    costS[i][j] = x_diff * 5 / 2 + c ;
			} else {
			    costS[i][j] = x_diff * 5 / 2 + c
					+ rowfeed_penaltyG[irow] ;
			}
		    } else {
			if( iptr->pinloc <= NEITHER ) {
			    costS[i][j] = x_diff * 5 / 2 + c
					+ rowfeed_penaltyG[jrow] ;
			} else {
			    costS[i][j] = x_diff * 5 / 2 + c
					+ rowfeed_penaltyG[irow]
					+ rowfeed_penaltyG[jrow] ;
			}
		    }
		}
	    }
	}
	costS[j][i] = costS[i][j] ;
    }
}
/* ****************************************************************
 *  suppose V = { 1 , 2 , ... , n } , initially U is set to {1}   *
 *  At each step, it finds a shortest edge ( u, v ) that connects *
 *  U and V-U and then adds v , the vertexS in V-U , to U.         *
 *  It repeats this step until U = V.                             *
 *        This is the Prim's algorithm for Minimum Spanning Tree. *
 ******************************************************************/

components[1] = OLD ;
for( i = 2 ; i <= n ; i++ ) {
    lowcostS[i] = costS[1][i] ;
    closestS[i] = 1 ;
    components[i] = NEW ;
    /* initialize with only vertexS 1 in the set U.                *
     * closestS[i] gives the vertexS in U that is currently closestS *
     * to vertexS i in V-U . lowcostS[i] gives the costS of the edge *
     * ( i , closestS[i] ).                                        */
}
for( i = 2 ; i <= n ; i++ ) {
    for( j = 2 ; j <= n ; j++ ) {
	if( components[j] ) {
	    min_costS = lowcostS[j] ;
	    first_new = k = j ;
	    break ;
	}
    }
    for( j++ ; j <= n ; j++ ) {
	if( components[j] && lowcostS[j] < min_costS ) {
	    min_costS = lowcostS[j] ;
	    k = j ;
	}
    }
    c = closestS[k] ;
	/* for vertexS k in V-U , c is the closestS vertexS in U to k */
    components[k] = OLD ;
    makeseg( vertexS[c] , vertexS[k] ) ;
    for( j = first_new ; j <= n ; j++ ) {
	if( components[j] && costS[k][j] < lowcostS[j] ) {
	    lowcostS[j] = costS[k][j] ;
	    closestS[j] = k ;
	}
    }
}
/* ******************* Prim's algorithm ends ******************** */
}


#ifdef EVEN_ROW
redo_steiner()
{

INT net , i ;
DBOXPTR dptr ;
PINBOXPTR netptr , nextptr ;
ADJASEGPTR adjptr , saveptr ;
SEGBOXPTR segptr , nextseg ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    dptr = netarrayG[net] ;
    if( dptr->numpins <= 1 ) {
	continue ;
    }
    for( segptr = netsegHeadG[net]->next ; segptr ; segptr = nextseg ) {
	nextseg = segptr->next ;
	Ysafe_free( segptr ) ;
    }
    netsegHeadG[net]->next = NULL ;
    for( netptr = dptr->pins ; netptr ; netptr = netptr->next ) {
	for( adjptr = netptr->adjptr->next ; adjptr ;
				    adjptr = saveptr ) {
	    saveptr = adjptr->next ;
	    Ysafe_free( adjptr ) ;
	}
	netptr->adjptr->next = NULL ;
    }
    for( netptr = steinerHeadG[net]->next ; netptr ;
			    netptr = netptr->next ) {
	for( adjptr = netptr->adjptr->next ; adjptr ;
				    adjptr = saveptr ) {
	    saveptr = adjptr->next ;
	    Ysafe_free( adjptr ) ;
	}
	netptr->adjptr->next = NULL ;
    }
    for( netptr = steinerHeadG[net]->next ; netptr ;
				netptr = nextptr ) {
	nextptr = netptr->next ;
	Ysafe_free( netptr ) ;
    }
    steinerHeadG[net]->next = NULL ;
}

re_estimate_feed_penalty() ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    dptr = netarrayG[net] ;
    switch( dptr->numpins ) {
    case 0 :
	break ;
    case 1 :
	break ;
    case 2 :
	netptr = dptr->pins ;
	makeseg( netptr , netptr->next ) ;
	break ;
    default :
	make_net_Tree( dptr->pins ) ;
	break ;
    }
}
for( net = 1 ; net <= numnetsG ; net++ ) {
    dptr = netarrayG[net] ;
    if( dptr->numpins <= 1 ) {
	continue ;
    } else if( dptr->numpins == 2 ) {
	netsegHeadG[net]->next->prev = netsegHeadG[net] ;
	continue ;
    }
    for( netptr = dptr->pins ; netptr ; netptr = netptr->next ) {
	netptr->newy = 3 * netptr->row + netptr->pinloc - PINLOC_OFFSET ;
    }
    for( netptr = dptr->pins ; netptr ; netptr = netptr->next ) {
	if( netptr->adjptr->next->next != NULL ) {
	    mergeseg( netptr ) ;
	}
    }
    for( segptr = netsegHeadG[net] ; segptr->next ;
			    segptr = segptr->next ) {
	segptr->next->prev = segptr ;
    }
}

for( i = 1 ; i < max_pinS ; i++ ) {
    Ysafe_free( costS[i] ) ;
}
Ysafe_free( costS ) ;
Ysafe_free( vertexS ) ;
Ysafe_free( lowcostS ) ;
Ysafe_free( closestS ) ;
Ysafe_free( components ) ;
Ysafe_free( add_st_flagG ) ;

free_up_feedest_malloc() ;
}
#endif
