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
FILE:	    mergeseg.c                                       
DESCRIPTION:segment code.
CONTENTS:   mergeseg( netptr )
		NBOX *netptr ;
	    mergedge( netptr, direction )
		NBOX *netptr ;
		INT direction ;
	    rplacseg( netptr, oldnode, newnode )
		PINBOXPTR netptr, oldnode, newnode ;
	    set_steiner_flag( ptr1, ptr2, ptr3 , stptr )
		PINBOXPTR ptr1, ptr2, ptr3, stptr ;
	    recheck_steiner_flag( stptr )
		NBOX *stptr ;
	    PINBOXPTR xmedianfun( aptr, bptr, cptr )
		PINBOXPTR aptr, bptr, cptr ;
	    PINBOXPTR ymedianfun( aptr, bptr, cptr )
		PINBOXPTR aptr, bptr, cptr ;
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) mergeseg.c (Yale) version 4.4 12/15/90" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#define NORTH 1
#define SOUTH 2
#define WEST  3
#define EAST  4

/* global variables */
extern INT *add_st_flagG ;

/* external routines */
extern INT cmpr_sx(), cmpr_lx(), cmpr_sy(), cmpr_ly() ;
extern SEGBOX *makeseg() ;
extern PINBOXPTR makeSTpt() , xmedianfun(), ymedianfun() ;

/* static definitions */
static PINBOXPTR nthptrS[30], sthptrS[30], wstptrS[30], estptrS[30] ;
static INT nthS, sthS, wstS, estS ;


/*------------------------------------------------------------------* 
 *    The function mergeseg() tries to combines the edges incident  *
 *  on the same nodes in the minimum spanning trees  by introducing *
 *  new nodes.                                                      *
 *------------------------------------------------------------------*/

mergeseg( netptr )
PINBOXPTR netptr ;
{
PINBOXPTR ptr ;
ADJASEG *adj ;
SEGBOX *segptr ;
INT x, y ;

wstS = estS = nthS = sthS = 0 ;
x = netptr->xpos ;
y = netptr->newy ;
adj = netptr->adjptr->next ;
if( adj->next == NULL ) {
    return ; /* only one edge is incident on this node, return */
}
for( ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( segptr->pin1ptr == netptr ) {
	ptr = segptr->pin2ptr ;
    } else {
	ptr = segptr->pin1ptr ;
    }
    if( ptr->newy < y ) {
	sthptrS[ ++sthS ] = ptr ;
    } else if( ptr->newy > y ) {
	nthptrS[ ++nthS ] = ptr ;
    } else if( ptr->xpos + average_feed_sepG <= x ) {
	wstptrS[ ++wstS ] = ptr ;
    } else if( ptr->xpos - average_feed_sepG >= x ) {
	estptrS[ ++estS ] = ptr ;
    }
}
mergedge( netptr, NORTH ) ;
mergedge( netptr, SOUTH ) ;
mergedge( netptr, WEST  ) ;
mergedge( netptr, EAST  ) ;
}


mergedge( netptr, direction )
PINBOXPTR netptr ;
INT direction ;
{
PINBOXPTR stptr, astptr, *dirptr, xmedian, ymedian ;
INT i, n, (*funcptr)() ;

if( direction == NORTH ) {
    dirptr = nthptrS ;
    funcptr = cmpr_sy ;
    n = nthS ;
} else if( direction == SOUTH ) {
    dirptr = sthptrS ;
    funcptr = cmpr_ly ;
    n = sthS ;
} else if( direction == WEST ) {
    dirptr = wstptrS ;
    funcptr = cmpr_lx ;
    n = wstS ;
} else if( direction == EAST ) {
    dirptr = estptrS ;
    funcptr = cmpr_sx ;
    n = estS ;
}
Yquicksort( (char *)(dirptr+1), n, sizeof(PINBOXPTR), funcptr ) ;
if( n == 0 ) {
    return ;
} else if( n == 1 ) {
    if( direction == WEST || direction == EAST ) {
        return ;
    }
    if( dirptr[1]->xpos + average_feed_sepG < netptr->xpos ) {
	wstptrS[ ++wstS ] = dirptr[1] ;
    } else if( dirptr[1]->xpos - average_feed_sepG > netptr->xpos ) {
	estptrS[ ++estS ] = dirptr[1] ;
    } else { /* straight vertical segment */
	return ;
    }
} else {
    astptr = dirptr[n] ;
    for( i = n - 1 ; i >= 1 ; i-- ) {
        xmedian = xmedianfun( astptr, dirptr[i], netptr ) ;
        ymedian = ymedianfun( astptr, dirptr[i], netptr ) ;
        if( ABS( dirptr[i]->xpos - xmedian->xpos ) <= average_feed_sepG
	    && dirptr[i]->newy == ymedian->newy ){
            stptr = dirptr[i] ;
            rplacseg( astptr, netptr, dirptr[i] ) ;
	    if( stptr->terminal == 0 && stptr->flag == 0 ) {
		recheck_steiner_flag( stptr ) ;
	    }
        } else if(
	    ABS( astptr->xpos - xmedian->xpos ) <= average_feed_sepG &&
				    astptr->newy == ymedian->newy ) {
            stptr = astptr ;
            rplacseg( dirptr[i], netptr, astptr ) ;
	    if( stptr->terminal == 0 && stptr->flag == 0 ) {
		recheck_steiner_flag( stptr ) ;
	    }
        } else {
            stptr = makeSTpt( netptr->net, xmedian, ymedian ) ;
            rplacseg( astptr, netptr, stptr ) ;
            rplacseg( dirptr[i], netptr, stptr ) ;
            makeseg( netptr, stptr ) ;
	    if( add_st_flagG[stptr->row] ) {
		stptr->flag = 1 ;
	    } else {
		set_steiner_flag( dirptr[i], astptr, netptr,stptr ) ;
	    }
        }
        astptr = stptr ;
    }
    if( direction == NORTH || direction == SOUTH ) {
	if( astptr->xpos + average_feed_sepG < netptr->xpos ) {
	    wstptrS[ ++wstS ] = astptr ;
	} else if( astptr->xpos - average_feed_sepG > netptr->xpos ) {
	    estptrS[ ++estS ] = astptr ;
	}
    }
}
}


rplacseg( netptr, oldnode, newnode )
PINBOXPTR netptr, oldnode, newnode ;
{
ADJASEG *adj, *tmpadj ;
SEGBOX *segptr ;

for( adj = netptr->adjptr->next ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( segptr->pin1ptr == oldnode ) {
        segptr->pin1ptr = newnode ;
        break ;
    } else if( segptr->pin2ptr == oldnode ) {
        segptr->pin2ptr = newnode ;
        break ;
    }
}
for( adj = oldnode->adjptr ; adj->next ; adj = adj->next ) {
    if( adj->next->segptr == segptr ) {
        tmpadj = adj->next ;
        adj->next = adj->next->next ;
	Ysafe_free( tmpadj ) ;
        break ;
    }
}
add_adj( segptr, newnode ) ;
update_segment_data( segptr ) ;
}


set_steiner_flag( ptr1, ptr2, ptr3 , stptr )
PINBOXPTR ptr1, ptr2, ptr3, stptr ;
{
PINBOXPTR hiptr, loptr ;

hiptr = loptr = ptr1 ;
if( loptr->newy > ptr2->newy ) {
    loptr = ptr2 ;
}
if( hiptr->newy < ptr2->newy ) {
    hiptr = ptr2 ;
}
if( loptr->newy > ptr3->newy ) {
    loptr = ptr3 ;
}
if( hiptr->newy < ptr3->newy ) {
    hiptr = ptr3 ;
}
if( hiptr->row > stptr->row ) {
    if( stptr->row > loptr->row ) {
	stptr->flag = 1 ;
    } else { /* stptr->row == loptr->row */
	if( (INT) loptr->pinloc == BOTCELL ) {
	    stptr->flag = 1 ;
	} else {
	    stptr->flag = 0 ;
	}
    }
} else { /* hiptr->row == stptr->row */
    if( stptr->row > loptr->row ) {
	if( hiptr->pinloc == TOPCELL ) {
	    stptr->flag = 1 ;
	} else {
	    stptr->flag = 0 ;
	}
    } else { /* hiptr->row == stptr->row == loptr->row */
	if( hiptr->pinloc == TOPCELL && (INT) loptr->pinloc == BOTCELL ) {
	    stptr->flag = 1 ;
	} else {
	    stptr->flag = 0 ;
	}
    }
}
}


recheck_steiner_flag( stptr )
PINBOXPTR stptr ;
{
ADJASEG *adj ;
SEGBOX *segptr ;
PINBOXPTR ptr, loptr, hiptr ;

loptr = hiptr = stptr ;
for( adj = stptr->adjptr->next ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( segptr->pin1ptr == stptr ) {
	ptr = segptr->pin2ptr ;
    } else {
	ptr = segptr->pin1ptr ;
    }
    if( ptr->newy > hiptr->newy ) {
	hiptr = ptr ;
    }
    if( ptr->newy < loptr->newy ) {
	loptr = ptr ;
    }
}
if( hiptr->row > stptr->row ) {
    if( stptr->row > loptr->row ) {
	stptr->flag = 1 ;
    } else { /* stptr->row == loptr->row */
	if( (INT) loptr->pinloc == BOTCELL ) {
	    stptr->flag = 1 ;
	}
    }
} else { /* hiptr->row == stptr->row */
    if( stptr->row > loptr->row ) {
	if( hiptr->pinloc == TOPCELL ) {
	    stptr->flag = 1 ;
	}
    } else { /* hiptr->row == stptr->row == loptr->row */
	if( hiptr->pinloc == TOPCELL && (INT) loptr->pinloc == BOTCELL ) {
	    stptr->flag = 1 ;
	}
    }
}
}


PINBOXPTR xmedianfun( aptr, bptr, cptr )
PINBOXPTR aptr, bptr, cptr ;
{

if( aptr->xpos <= bptr->xpos &&
    aptr->xpos <= cptr->xpos ) {
    if( bptr->xpos <= cptr->xpos ) {
        return( bptr ) ;
    } else {
        return( cptr ) ;
    }
} else if( bptr->xpos <= aptr->xpos &&
           bptr->xpos <= cptr->xpos ) {
    if( aptr->xpos <= cptr->xpos ) {
        return( aptr ) ;
    } else {
        return( cptr ) ;
    }
} else {
    if( aptr->xpos <= bptr->xpos ) {
        return( aptr ) ;
    } else {
        return( bptr ) ;
    }
}
}


PINBOXPTR ymedianfun( aptr, bptr, cptr )
PINBOXPTR aptr, bptr, cptr ;
{

if( aptr->newy <= bptr->newy &&
    aptr->newy <= cptr->newy ) {
    if( bptr->newy <= cptr->newy ) {
        return( bptr ) ;
    } else {
        return( cptr ) ;
    }
} else if( bptr->newy <= aptr->newy &&
           bptr->newy <= cptr->newy ) {
    if( aptr->newy <= cptr->newy ) {
        return( aptr ) ;
    } else {
        return( cptr ) ;
    }
} else {
    if( aptr->newy <= bptr->newy ) {
        return( aptr ) ;
    } else {
        return( bptr ) ;
    }
}
}
