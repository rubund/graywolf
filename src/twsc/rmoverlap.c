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
FILE:	    rmoverlap.c                                       
DESCRIPTION:remove netsegment overlap function.
CONTENTS:   assgn_channel_to_seg()
	    free_chan_seg()
	    remove_overlap_segment( net )
		INT net ;
	    rm_segm_overlap( checkseg , m )
		SEGBOXPTR *checkseg ;
		INT m ;
	    PINBOXPTR depth_first_search( ptr, bptr1, bptr2, aseg, edge )
		PINBOXPTR ptr , bptr1 , bptr2 ;
		SEGBOXPTR aseg , edge ;
	    replace_seg( netptr, oldnode, newnode )
		PINBOXPTR netptr , oldnode , newnode ;
	    add_adj( segptr, node )
		SEGBOXPTR segptr ;
		PINBOXPTR node ;
	    check_overlap_at_pin( ptr )
		PINBOXPTR ptr ;
	    check_connectivity( net )
		INT net ;
	    depth_first_check( ptr , oldedge )
		PINBOXPTR ptr ;
		SEGBOXPTR oldedge ;
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) rmoverlap.c (Yale) version 4.4 12/15/90" ;
#endif
#endif

#include "standard.h"
#include "groute.h"

/* global variable references */
extern BOOL connectFlagG ;
PINBOXPTR depth_first_search() ;

/* static definitions */
static INT *segcountS ;
static SEGBOXPTR **chan_segS ;

assgn_channel_to_seg()
{

PINBOXPTR ptr1 , ptr2 ;
SEGBOXPTR segptr ;
INT i , net , *maxcount ;
INT channel , topchan , botchan ;

segcountS = (INT *)Ysafe_calloc( numChansG+1 , sizeof(INT) ) ;
maxcount = (INT *)Ysafe_calloc( numChansG+1 , sizeof(INT) ) ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    topchan = 0 ;
    botchan = numChansG ;
    for( segptr = netsegHeadG[net]->next ; segptr ; 
					segptr = segptr->next ) {
	ptr1 = segptr->pin1ptr ;
	ptr2 = segptr->pin2ptr ;
	if( ptr1->row == ptr2->row ) {
	    if( segptr->switchvalue == swUP ) {
		segptr->flag = ptr1->row + 1 ;
	    } else if( segptr->switchvalue == swDOWN ) {
		segptr->flag = ptr1->row ;
	    } else if( ptr1->pinloc >= NEITHER && ptr2->pinloc >= NEITHER ) {
		if( ptr1->row == numChansG ) {
		    segptr->flag = ptr1->row ;
		} else {
		    segptr->flag = ptr1->row + 1 ;
		}
	    } else {
		segptr->flag = ptr1->row ;
	    }
	} else {
	    segptr->flag =
		( ptr1->row > ptr2->row ) ? ptr1->row : ptr2->row ;
	}
	segcountS[ ( channel = segptr->flag ) ]++ ;
	if( channel > topchan ) topchan = channel ;
	if( channel < botchan ) botchan = channel ;
    }
    for( i = botchan ; i <= topchan ; i++ ) {
	if( segcountS[i] > maxcount[i] ) {
	    maxcount[i] = segcountS[i] ;
	}
	segcountS[i] = 0 ;
    }
}
chan_segS = (SEGBOXPTR **)Ysafe_calloc( numChansG+1, sizeof(SEGBOXPTR *) );
for( i = 1 ; i <= numChansG ; i++ ) {
    chan_segS[i] = (SEGBOXPTR *)Ysafe_calloc( 
		    maxcount[i] + 10, sizeof(SEGBOXPTR) ) ;
}
Ysafe_free( maxcount ) ;
}


free_chan_seg()
{
INT i ;

for( i = 1 ; i <= numChansG ; i++ ) {
    Ysafe_free( chan_segS[i] ) ;
}
Ysafe_free( chan_segS ) ;
Ysafe_free( segcountS ) ;

}

remove_overlap_segment( net )
INT net ;
{

PINBOXPTR ptr ;
SEGBOXPTR segptr ;
INT chan , botchan , topchan ; 
INT checkFlag ;

for( ptr = netarrayG[net]->pins ; ptr ; ptr = ptr->next ) {
    ptr->flag = 1 ;
}
do {
    checkFlag = 0 ;
    for( ptr = netarrayG[net]->pins ; ptr ; ptr = ptr->next ) {
	if( ptr->flag == 1 ) {
	    check_overlap_at_pin( ptr ) ;
	    checkFlag = 1 ;
	}
    }
} while( checkFlag == 1 ) ;

topchan = 0 ;
botchan = numChansG ;
for( segptr = netsegHeadG[net]->next ; segptr ; 
			    segptr = segptr->next ) {
    chan = segptr->flag ;
    if( chan > topchan ) topchan = chan ;
    if( chan < botchan ) botchan = chan ;
    chan_segS[chan][ ++segcountS[chan] ] = segptr ;
}
for( chan = botchan ; chan <= topchan ; chan++ ) {
    if( segcountS[chan] <= 1 ) {
	segcountS[chan] = 0 ;
	continue ;
    }
    rm_segm_overlap( chan_segS[chan], segcountS[chan] );
    segcountS[chan] = 0 ;
}

if( check_connectivity( net ) == 0 ) {
    connectFlagG = FALSE ;
}
}


rm_segm_overlap( checkseg , m )
SEGBOXPTR *checkseg ;
INT m ;
{

SEGBOXPTR aseg , bseg , seg ;
PINBOXPTR aptr1 , aptr2 , bptr1 , bptr2 , anode , bnode ;
ADJASEGPTR adj ;
INT i , j , flag ;

do {
    flag = 0 ;
    for( i = 1 ; i <= m ; i++ ) {
	for( j = 1 ; j <= m ; j++ ) {
	    if( i == j ) continue ;
	    aseg  = checkseg[i] ;
	    aptr1 = aseg->pin1ptr ;
	    aptr2 = aseg->pin2ptr ;
	    bseg  = checkseg[j] ;
	    bptr1 = bseg->pin1ptr ;
	    bptr2 = bseg->pin2ptr ;
	    if( aptr1->xpos >= bptr2->xpos || 
		aptr2->xpos <= bptr1->xpos ) {
		continue ;
	    }
	    if( aptr1 == bptr1 ) {
		if( aptr2->xpos <= bptr2->xpos ) {
		    replace_seg( bptr2 , bptr1 , aptr2 ) ;
		} else {
		    replace_seg( aptr2 , aptr1 , bptr2 ) ;
		}
		flag = 1 ;
		continue ;
	    } else if( aptr2 == bptr2 ) {
		if( aptr1->xpos <= bptr1->xpos ) {
		    replace_seg( aptr1 , aptr2 , bptr1 ) ;
		} else {
		    replace_seg( bptr1 , bptr2 , aptr1 ) ;
		}
		flag = 1 ;
		continue ;
	    }
	    if( aptr1->xpos == bptr1->xpos ) {
		for( adj = aptr1->adjptr->next ; adj ; adj = adj->next){
		    seg = adj->segptr ;
		    if( seg->pin1ptr == aptr1 && seg->pin2ptr == bptr1||
			seg->pin1ptr == bptr1 && seg->pin2ptr == aptr1){
			break ;
		    }
		}
		if( adj ) {
		    if( aptr2->xpos <= bptr2->xpos ) {
			replace_seg( bptr2 , bptr1 , aptr2 ) ;
		    } else {
			replace_seg( aptr2 , aptr1 , bptr2 ) ;
		    }
		    flag = 1 ;
		    continue ;
		}
	    }
	    if( aptr2->xpos ==  bptr2->xpos ) {
		for( adj = aptr2->adjptr->next ; adj ; adj = adj->next){
		    seg = adj->segptr ;
		    if( seg->pin1ptr == aptr2 && seg->pin2ptr == bptr2||
			seg->pin1ptr == bptr2 && seg->pin2ptr == aptr2){
			break ;
		    }
		}
		if( adj ) {
		    if( aptr1->xpos <= bptr1->xpos ) {
			replace_seg( aptr1 , aptr2 , bptr1 ) ;
		    } else {
			replace_seg( bptr1 , bptr2 , aptr1 ) ;
		    }
		    flag = 1 ;
		    continue ;
		}
	    }

	    bnode = depth_first_search( aptr1, bptr1, bptr2, aseg, 0 ) ;
	    if( bnode ) {
		anode = aptr1 ;
	    } else {
		anode = aptr2 ;
		bnode = depth_first_search( 
		    aptr2 , bptr1 , bptr2 , aseg, 0 );
		if( bnode == NULL ) {
		    printf("connectivity is lost for net %d\n", 
						    aptr1->net);
		    exit(0) ;
		}
	    }

	    if( aptr1->xpos <= bptr1->xpos ) {
		if( aptr2->xpos <= bptr2->xpos ) {
		    /* a1 b1 a2 b2 */
		    if( anode == aptr1 && bnode == bptr1 ) {
			replace_seg( aptr2 , aptr1 , bptr1 ) ;
			replace_seg( bptr2 , bptr1 , aptr2 ) ;
		    } else if( anode == aptr2 && bnode == bptr2 ) {
			replace_seg( aptr1 , aptr2 , bptr1 ) ;
			replace_seg( bptr1 , bptr2 , aptr2 ) ;
		    } else {
			replace_seg( aptr1 , aptr2 , bptr1 ) ;
			replace_seg( bptr2 , bptr1 , aptr2 ) ;
		    }
		} else {
		    /* a1 b1 b2 a2 */
		    if( anode == aptr1 && bnode == bptr1 ||
			anode == aptr1 && bnode == bptr2 ) {
			replace_seg( aptr2 , aptr1 , bptr2 ) ;
		    } else {
			replace_seg( aptr1 , aptr2 , bptr1 ) ;
		    }
		}
	    } else { /* b1 < a1 */
		if( bptr2->xpos  <= aptr2->xpos ) {
		    /* b1 a1 b2 a2 */
		    if( bnode == bptr1 && anode == aptr1 ) {
			replace_seg( bptr2 , bptr1 , aptr1 ) ;
			replace_seg( aptr2 , aptr1 , bptr2 ) ;
		    } else if( bnode == bptr2 && anode == aptr2 ) {
			replace_seg( bptr1 , bptr2 , aptr1 ) ;
			replace_seg( aptr1 , aptr2 , bptr2 ) ;
		    } else {
			replace_seg( bptr1 , bptr2 , aptr1 ) ;
			replace_seg( aptr2 , aptr1 , bptr2 ) ;
		    }
		} else { /* a2 < b2 */
		    /* b1 a1 a2 b2 */
		    if( bnode == bptr1 && anode == aptr1 ||
			bnode == bptr1 && anode == aptr2 ) {
			replace_seg( bptr2 , bptr1 , aptr2 ) ;
		    } else {
			replace_seg( bptr1 , bptr2 , aptr1 ) ;
		    }
		}
	    }
	    flag = 1 ;
	}
    }
} while( flag ) ;
}


PINBOXPTR depth_first_search( ptr , bptr1 , bptr2 , aseg , edge )
PINBOXPTR ptr , bptr1 , bptr2 ;
SEGBOXPTR aseg , edge ;
{
PINBOXPTR next ;
SEGBOXPTR seg ;
ADJASEGPTR adj ;

for( adj = ptr->adjptr->next ; adj ; adj = adj->next ) {
    seg = adj->segptr ;
    if( seg == edge || seg == aseg ) {
	continue ;
    }
    if( seg->pin1ptr == ptr ) {
	next = seg->pin2ptr ;
    } else {
	next = seg->pin1ptr ;
    }
    if( next == bptr1 || next == bptr2 ) {
	return( next ) ;
    } else {
	next = depth_first_search( next, bptr1, bptr2, aseg, seg );
	if( next == bptr1 || next == bptr2 ) {
	    return( next ) ;
	}
    }
}
return( NULL ) ;
}


replace_seg( netptr, oldnode, newnode )
PINBOXPTR netptr , oldnode , newnode ;
{
ADJASEGPTR adj, tmpadj ;
SEGBOXPTR segptr ;

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
if( netptr->row != newnode->row ) {
    segptr->switchvalue = nswLINE ;
} else if( netptr->pinloc != NEITHER || newnode->pinloc != NEITHER ) {
    segptr->switchvalue = nswLINE ;
} else if( segptr->flag > netptr->row ) {
    segptr->switchvalue = swUP ;
} else {
    segptr->switchvalue = swDOWN ;
}
}


add_adj( segptr, node )
SEGBOXPTR segptr ;
PINBOXPTR node ;
{
ADJASEG *adjptr ;

adjptr = (ADJASEG *)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;
adjptr->segptr = segptr ;
adjptr->next = node->adjptr->next ;
node->adjptr->next = adjptr ;
}


check_overlap_at_pin( ptr )
PINBOXPTR ptr ;
{

PINBOXPTR aptr , bptr ;
SEGBOXPTR aseg , bseg ;
ADJASEGPTR adj , bdj , next_adj , next_bdj ;
INT breakFlag ;

ptr->flag = 0 ;
breakFlag = 0 ;
for( adj = ptr->adjptr ; adj->next ; ) {
    next_adj = adj->next ;
    aseg = next_adj->segptr ;
    if( aseg->pin1ptr == ptr ) {
	aptr = aseg->pin2ptr ;
    } else {
	aptr = aseg->pin1ptr ;
    }
    for( bdj = adj->next ; bdj->next ; bdj = bdj->next ) {
	next_bdj = bdj->next ;
	bseg = next_bdj->segptr ;
	if( aseg->flag != bseg->flag ) {
	    continue ;
	}
	if( bseg->pin1ptr == ptr ) {
	    bptr = bseg->pin2ptr ;
	} else {
	    bptr = bseg->pin1ptr ;
	}
	if( ptr->xpos < aptr->xpos && aptr->xpos < bptr->xpos ) {
	    /* ptr    aptr    bptr */
	    /* move the segment between ptr and bptr
	       to be in between aptr and bptr.       */

	    bseg->pin1ptr = aptr ;
	    bdj->next = next_bdj->next ;
	    /* remove the next_bdj from ptr adjacent list */

	    next_bdj->next = aptr->adjptr->next ;
	    aptr->adjptr->next = next_bdj ;
	    aptr->flag = 1 ;
	    /* add next_bdj to the adjacent list of aptr */

	    bseg->switchvalue = nswLINE ;
	    break ;

	} else if( ptr->xpos > aptr->xpos && aptr->xpos > bptr->xpos ) {
	    /* bptr    aptr    ptr */
	    /* move the segment between bptr and ptr
	       to be in between bptr and aptr.       */

	    bseg->pin2ptr = aptr ;
	    bdj->next = next_bdj->next ;
	    /* remove the next_bdj from ptr adjacent list */

	    next_bdj->next = aptr->adjptr->next ;
	    aptr->adjptr->next = next_bdj ;
	    aptr->flag = 1 ;
	    /* add next_bdj to the adjacent list of aptr */

	    bseg->switchvalue = nswLINE ;
	    break ;

	} else if( ptr->xpos < bptr->xpos && bptr->xpos < aptr->xpos ) {
	    /* ptr    bptr    aptr */
	    /* move the segment between ptr and aptr
	       to be in between bptr and aptr.       */

	    aseg->pin1ptr = bptr ;
	    adj->next = next_adj->next ;
	    /* remove the next_adj from ptr adjacent list */

	    next_adj->next = bptr->adjptr->next ;
	    bptr->adjptr->next = next_adj ;
	    bptr->flag = 1 ;
	    /* add next_adj to the adjacent list of bptr */

	    aseg->switchvalue = nswLINE ;
	    breakFlag = 1 ;
	    break ;

	} else if( ptr->xpos > bptr->xpos && bptr->xpos > aptr->xpos ) {
	    /* aptr    bptr     ptr */
	    /* move the segment between aptr and ptr
	       to be in between aptr and bptr.       */

	    aseg->pin2ptr = bptr ;
	    adj->next = next_adj->next ;
	    /* remove the next_adj from ptr adjacent list */

	    next_adj->next = bptr->adjptr->next ;
	    bptr->adjptr->next = next_adj ;
	    bptr->flag = 1 ;
	    /* add next_adj to the adjacent list of bptr */

	    aseg->switchvalue = nswLINE ;
	    breakFlag = 1 ;
	    break ;
	}
    }
    if( !breakFlag ) {
	adj = adj->next ;
    } else {
	breakFlag = 0 ;
    }
}
}


check_connectivity( net )
INT net ;
{
INT correctness = 1 ;
PINBOXPTR ptr , hdptr ;

hdptr= netarrayG[net]->pins ;
if( !hdptr ) {
    return(1);
}
for( ptr = hdptr ; ptr ; ptr = ptr->next ) {
    ptr->flag = NEW ;
}
hdptr->flag = OLD ;
depth_first_check( hdptr , 0 ) ;
for( ptr =  hdptr ; ptr ; ptr = ptr->next ) {
    if( ptr->flag == NEW ) {
	printf(" the connectivity of net %d after" , net ) ;
	printf(" the remove_overlap operation was lost\n" ) ;
	correctness = 0 ;
	exit(0) ;
    }
}
return( correctness ) ;
}

depth_first_check( ptr , oldedge )
PINBOXPTR ptr ;
SEGBOXPTR oldedge ;
{
PINBOXPTR nextptr ;
SEGBOXPTR segptr ;
ADJASEGPTR adj ;

for( adj = ptr->adjptr->next ; adj ; adj = adj->next ) {
    segptr = adj->segptr ;
    if( segptr == oldedge ) {
	continue ;
    }
    if( segptr->pin1ptr == ptr ) {
	nextptr = segptr->pin2ptr ;
    } else {
	nextptr = segptr->pin1ptr ;
    }
    if( nextptr->flag == NEW ) {
	nextptr->flag = OLD ;
	depth_first_check( nextptr , segptr ) ;
    } else {
	printf(" there is a cycle in net %d after" , ptr->net ) ;
	printf(" the remove overlap function\n" ) ;
	exit(0) ;
    }
}
}
