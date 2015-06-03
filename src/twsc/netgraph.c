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
FILE:	    netgraph.c                                       
DESCRIPTION:network graph code.
CONTENTS:   postFeedAssgn()
	    rebuild_netgraph( net )
		INT net ;
	    remove_unnecessary_feed( net , flag )
		INT net , flag ;
	    find_set_name( v )
		INT v ;
	    do_set_union( i , j )
		INT i , j ;
	    switchable_or_not()
	    free_z_memory()
DATE:	    Mar 27, 1989 
REVISIONS:  Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
	    Tue Jan 15 20:30:05 PST 1991 - changed frees to Ysafe_frees.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) netgraph.c (Yale) version 4.7 1/15/91" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "main.h"

typedef struct graph_edge_cost {
    SHORT node1 ;
    SHORT node2 ;
    INT cost ;
    INT channel ;
}
*EDGE_COST ,
EDGE_COST_BOX ;

/* global variable definitions */
INT *count_G ;
INT *father_G ;
INT *root_G ;
INT *stack_G ;
PINBOXPTR *vertex_G ;

/* global variable references */
extern INT Max_numPinsG ;

/* static definitions */
static INT maxpin_numberS ;
static INT *pins_at_rowS ;
static INT *first_indexS ;
static PINBOXPTR **z_S ;
static EDGE_COST *edge_dataS ;


postFeedAssgn()
{

INT net , i , row , botrow , toprow , last_i ;
SEGBOXPTR segptr , nextseg ;
PINBOXPTR netptr , nextptr , st_head , stptr ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ; segptr ; segptr = nextseg ) {
	nextseg = segptr->next ;
	Ysafe_free( segptr ) ;
    }
    netsegHeadG[net]->next = NULL ;
    netptr = netarrayG[net]->pins ;
    st_head = steinerHeadG[net] ;
    if( st_head->next ) { /* there are steiner point for this net */
	for( stptr = st_head ; stptr->next ; ) {
	    nextptr = stptr->next ;
	    if( nextptr->terminal == 0 || !nextptr->flag ) {
		/* steiner point in bottom pads or top pads
		   or they are pseudo steiner point.         */
		stptr->next = nextptr->next ;
		Ysafe_free( nextptr ) ;
	    } else {
		stptr = nextptr ;
	    }
	}
	stptr->next = netptr ;
	netarrayG[net]->pins = st_head->next ;
	/* put all the netbox of the steiner point into the
	   netarrayG linked lists.                           */
    }
    Ysafe_free( steinerHeadG[net] ) ;
}
Ysafe_free( steinerHeadG ) ;

maxpin_numberS = 0 ;
pins_at_rowS = (INT *)Ysafe_calloc( numChansG + 1, sizeof(INT) ) ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    if( netarrayG[net]->numpins <= 2 ) {
	continue ;
    }
    botrow = numChansG ;
    toprow = 0 ;
    for( netptr = netarrayG[net]->pins ; netptr ; netptr = netptr->next ) {
	row = netptr->row ;
	pins_at_rowS[ row ]++ ;
	if( row < botrow ) {
	    botrow = row ;
	}
	if( row > toprow ) {
	    toprow = row ;
	}
    }
    for( row = botrow ; row <= toprow ; row++ ) {
	if( pins_at_rowS[row] > maxpin_numberS ) {
	    maxpin_numberS = pins_at_rowS[row] ;
	}
	pins_at_rowS[row] = 0 ;
    }
}

maxpin_numberS += 3 ;
count_G    = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
father_G   = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
root_G     = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
stack_G    = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
first_indexS = (INT *)Ysafe_malloc( ( numChansG + 1 ) * sizeof( INT ) ) ;
vertex_G   = (PINBOXPTR *)Ysafe_malloc( 2 * Max_numPinsG * sizeof(PINBOXPTR) );
last_i = maxpin_numberS * maxpin_numberS * numRowsG - 1 ;
edge_dataS = (EDGE_COST *)Ysafe_malloc( ( last_i + 1 )
				       * sizeof(EDGE_COST) ) ;
for( i = 1 ; i <= last_i ; i++ ) {
    edge_dataS[i] = (EDGE_COST)Ysafe_malloc( sizeof(EDGE_COST_BOX) );
}
z_S = (PINBOXPTR **)Ysafe_malloc( ( numChansG + 1 ) * sizeof(PINBOXPTR *) ) ;
for( i = 0 ; i <= numChansG ; i++ ) {
    z_S[i] = (PINBOXPTR *)Ysafe_malloc( maxpin_numberS * sizeof(PINBOXPTR) ) ;
}

for( net = 1 ; net <= numnetsG ; net++ ) {
    if( netarrayG[net]->numpins <= 1 ) {
	continue ;
    }
    rebuild_netgraph( net ) ;
    /* find minimum cost tree connection from all the pins of a net */
}
}


rebuild_netgraph( net )
INT net ;
{

PINBOXPTR netptr ;
SEGBOXPTR segptr ;
ADJASEGPTR adj ;
INT row , botrow , toprow , i , j , k , x ;
INT node1 , node2 , last_j , last_k ;
INT num_edge , tree_edge , vtx , j_rite ;
INT comparepinx(), compare_cost() ;

netptr = netarrayG[net]->pins ;
toprow = 0 ;
botrow = numRowsG + 1 ;
for( ; netptr ; netptr = netptr->next ) {
    row = netptr->row ;
    if( row > toprow ) {
	toprow = row ;
    }
    if( row < botrow ) {
	botrow = row ;
    }
    z_S[ row ][ ++pins_at_rowS[row] ] = netptr ;
}
for( i = botrow ; i <= toprow ; i++ ) {
    if( pins_at_rowS[i] == 0 ) {
	printf("there are no pin in row %d for net %d\n", i, net );
	dbx_terminal( net , net ) ;
	YexitPgm(PGMFAIL) ;
    } else {
	if( pins_at_rowS[i] > 1 ) {
	    Yquicksort( (char *)(z_S[i]+1),
		pins_at_rowS[i], sizeof(PINBOXPTR), comparepinx );
	}
    }
}

vtx = 0 ;
num_edge = 0 ;
first_indexS[botrow] = 0 ;
for( i = 1 ; i <= pins_at_rowS[botrow] ; i++ ) {
    vertex_G[ ++vtx ] = z_S[botrow][i] ;
}
for( i = botrow + 1 ; i <= toprow ; i++ ) {
    first_indexS[i] = first_indexS[i-1] + pins_at_rowS[i-1] ;
    for( j = 1 ; j <= pins_at_rowS[i] ; j++ ) {
	vertex_G[ ++vtx ] = z_S[i][j] ;
    }
}
/* build complete graph for pins in adjacent rows */
for( i = botrow ; i < toprow ; i++ ) {
    last_j = first_indexS[i] + pins_at_rowS[i] ;
    last_k = first_indexS[i+1] + pins_at_rowS[i+1] ;
    for( j = first_indexS[i] + 1 ; j <= last_j ; j++ ) {
	if( vertex_G[j]->pinloc < NEITHER ) {
	    continue ;
	}
	for( k = first_indexS[i+1] + 1 ; k <= last_k ; k++ ) {
	    if( vertex_G[k]->pinloc > NEITHER ) {
		continue ;
	    }
	    edge_dataS[ ++num_edge ]->node1 = j ;
	    edge_dataS[   num_edge ]->node2 = k ;
	    x = ABS( vertex_G[j]->xpos - vertex_G[k]->xpos ) ;
	    if( x < average_pin_sepG ) {
		edge_dataS[ num_edge ]->cost = x ;
	    } else {
		edge_dataS[ num_edge ]->cost = 5 * x / 2 ;
	    }
	}
    }
}
/* hook up the pins in the same row one by one */
for( i = botrow ; i <= toprow ; i++ ) {
    last_j = first_indexS[i] + pins_at_rowS[i] ;
    for( j = first_indexS[i] + 1 ; j < last_j ; j++ ) {
	if( vertex_G[j]->pinloc > NEITHER ) {
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc >= NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  = 5 *
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	} else if( vertex_G[j]->pinloc < NEITHER ) {
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc <= NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  = 5 *
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	} else {
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc < NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  = 5 * 
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc > NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  = 5 *
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc == NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  =
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	}
    }
}
/* find the minimum spanning tree for the graph build above   */
/* -----------------------------------------------------------*
 *                                                            *
 *             Kruskal's minimum spanning tree                *
 *                                                            *
 * -----------------------------------------------------------*/
for( i = 1 ; i <= vtx ; i++ ) {
    count_G[i]    = 1 ;
    father_G[i]   = 0 ;
    root_G[i]     = i ;
}
tree_edge = 0 ;
Yquicksort( (char *)(edge_dataS+1), num_edge, sizeof(EDGE_COST), 
    compare_cost ) ;

segptr = netsegHeadG[net] ;
for( i = 1 ; i <= num_edge ; i++ ) {
    node1 = edge_dataS[i]->node1 ;
    node2 = edge_dataS[i]->node2 ;
    j = find_set_name( node1 ) ;
    k = find_set_name( node2 ) ;
    if( j != k ) { /* node1 and node2 belong to two different set*/
	do_set_union( j , k ) ;
	segptr->next =
	    (SEGBOXPTR)Ysafe_calloc( 1, sizeof(SEGBOX) ) ;
	segptr->next->prev = segptr ;
	segptr = segptr->next ;
	if( vertex_G[node1]->xpos <= vertex_G[node2]->xpos ) {
	    segptr->pin1ptr = vertex_G[node1] ;
	    segptr->pin2ptr = vertex_G[node2] ;
	} else {
	    segptr->pin1ptr = vertex_G[node2] ;
	    segptr->pin2ptr = vertex_G[node1] ;
	}

	adj = (ADJASEGPTR)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;
	adj->next = vertex_G[node1]->adjptr->next ;
	adj->segptr = segptr ;
	vertex_G[node1]->adjptr->next = adj ;

	adj = (ADJASEGPTR)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;
	adj->next = vertex_G[node2]->adjptr->next ;
	adj->segptr = segptr ;
	vertex_G[node2]->adjptr->next = adj ;

	if( ++tree_edge >= vtx ) {
	    break ;
	}
    }
}

remove_unnecessary_feed( net , 0 ) ;
    /* remove the feed through pins that has only one edge
       incident on it.                                     */

for( i = botrow ; i <= toprow ; i++ ) {
    pins_at_rowS[i] = 0 ;
}
return ;
}

/* ------------------------------------------------------------------*
*    the do - while loop below try to remove all the feed through    *
*   pins such that there are only one edge incident on them          *
*--------------------------------------------------------------------*/

remove_unnecessary_feed( net , flag )
INT net , flag ;
{

DBOXPTR dimptr ;
PINBOXPTR netptr , nextptr , ptr , ptr1 , ptr2 ;
PINBOXPTR straight_ptr ;
SEGBOXPTR segptr , segptr1 , segptr2 ;
SEGBOXPTR asegptr[10] , straight_seg ;
ADJASEGPTR adj , adjnext ;
INT remove_feed_flag ;
INT i, n, fixFlag, channel ;

if( !(netarrayG[net]->pins) ) {
    return ;
}
do {
    remove_feed_flag = 0 ;
    dimptr = netarrayG[net] ;
    netptr = dimptr->pins ;
    for( ; netptr->next ; netptr = netptr->next ){
	nextptr = netptr->next ;
	if( nextptr->terminal > TotRegPinsG ) {
	    if( nextptr->adjptr->next == NULL ) {
		netptr->next = nextptr->next ;
		Ysafe_free( nextptr->eqptr ) ;
		Ysafe_free( nextptr->adjptr ) ;
		/*   added by Carl 7/14/89       */
		tearrayG[nextptr->terminal] = NULL ;
		/*   end of addition             */
		Ysafe_free( nextptr ) ;
		implicit_pins_usedG-- ;
		remove_feed_flag = 1 ;
	    } else if( nextptr->adjptr->next->next == NULL ) {
		segptr = nextptr->adjptr->next->segptr ;
		segptr->prev->next = segptr->next ;
		if( segptr->next != NULL ) {
		    segptr->next->prev = segptr->prev ;
		}
		if( segptr->pin1ptr == nextptr ) {
		    ptr = segptr->pin2ptr ;
		} else {
		    ptr = segptr->pin1ptr ;
		}
		for( adj = ptr->adjptr ; adj->next ; adj = adj->next ) {
		    adjnext = adj->next ;
		    if( adjnext->segptr == segptr ) {
			adj->next = adjnext->next ;
			Ysafe_free( adjnext ) ;
			break ;
		    }
		}
		Ysafe_free( segptr ) ;
		netptr->next = nextptr->next ;
		Ysafe_free( nextptr->eqptr ) ;
		Ysafe_free( nextptr->adjptr->next ) ;
		Ysafe_free( nextptr->adjptr ) ;
		/*   added by Carl 7/14/89       */
		tearrayG[nextptr->terminal] = NULL ;
		/*   end of addition             */
		Ysafe_free( nextptr ) ;
		implicit_pins_usedG-- ;
		remove_feed_flag = 1 ;
	    } else if( flag && 
			nextptr->adjptr->next->next->next == NULL ) {
		segptr1 = nextptr->adjptr->next->segptr ;
		segptr2 = nextptr->adjptr->next->next->segptr ;
		if( segptr1->flag == segptr2->flag ) { /*same channel*/
		    segptr2->prev->next = segptr2->next ;
		    if( segptr2->next != NULL ) {
			segptr2->next->prev = segptr2->prev ;
		    }
		    if( segptr1->pin1ptr == nextptr ) {
			ptr1 = segptr1->pin2ptr ;
		    } else {
			ptr1 = segptr1->pin1ptr ;
		    }
		    if( segptr2->pin1ptr == nextptr ) {
			ptr2 = segptr2->pin2ptr ;
		    } else {
			ptr2 = segptr2->pin1ptr ;
		    }
		    if( ptr1->xpos <= ptr2->xpos ) {
			segptr1->pin1ptr = ptr1 ;
			segptr1->pin2ptr = ptr2 ;
		    } else {
			segptr1->pin1ptr = ptr2 ;
			segptr1->pin2ptr = ptr1 ;
		    }
		    if( ptr1->row == ptr2->row &&
			ptr1->pinloc == NEITHER && ptr2->pinloc == NEITHER ) {
			if( segptr1->flag == ptr1->row + 1 ) {
			    segptr1->switchvalue = swUP ;
			} else {
			    segptr1->switchvalue = swDOWN ;
			}
		    } else {
			segptr1->switchvalue = nswLINE ;
		    }
		    /* old way: segptr1->switchvalue = nswLINE ; */

		    for( adj = ptr2->adjptr->next ; adj ; 
					    adj = adj->next ) {
			if( adj->segptr == segptr2 ) {
			    adj->segptr = segptr1 ;
			    break ;
			}
		    }
		    Ysafe_free( segptr2 ) ;
		    netptr->next = nextptr->next ;
		    Ysafe_free( nextptr->eqptr ) ;
		    Ysafe_free( nextptr->adjptr->next->next ) ;
		    Ysafe_free( nextptr->adjptr->next ) ;
		    Ysafe_free( nextptr->adjptr ) ;
		    /*   added by Carl 7/14/89       */
		    tearrayG[nextptr->terminal] = NULL ;
		    /*   end of addition             */
		    Ysafe_free( nextptr ) ;
		    implicit_pins_usedG-- ;
		    remove_feed_flag = 1 ;
		}
	    } else if( flag ) {
		fixFlag = 0 ;
		n = 0 ;
		adj = nextptr->adjptr->next ;
		channel = adj->segptr->flag ;
		straight_seg = NULL ;
		for( ; adj ; adj = adj->next ) {
		    segptr = adj->segptr ;
		    if( segptr->flag != channel ) {
			fixFlag = 1 ;
			break ;
		    } else if( segptr->pin1ptr->xpos 
			    == segptr->pin2ptr->xpos && 
				straight_seg == NULL ) {
			straight_seg = segptr ;
			/* straight vertical segment */
		    } else {
			asegptr[ ++n ] = segptr ;
			/* switchable segment */
		    }
		}
		if( fixFlag ) {
		 /* not all the adjcent segments are in the same channel */
		 /* nothing need to be done */
		    continue ;
		}
		if( straight_seg->pin1ptr == nextptr ) {
		    straight_ptr = straight_seg->pin2ptr ;
		} else {
		    straight_ptr = straight_seg->pin1ptr ;
		}
		/* straight_ptr is the pointer to the terminal which is straight
		   above the feed-through pin */
		for( i = 1 ; i <= n ; i++ ) {
		    if( asegptr[i]->pin1ptr == nextptr ) {
			asegptr[i]->pin1ptr = straight_ptr ;
		    } else {
			asegptr[i]->pin2ptr = straight_ptr ;
		    }
		    asegptr[i]->switchvalue = nswLINE ;
		}
		straight_seg->prev->next = straight_seg->next ;
		if( straight_seg->next ) {
		    straight_seg->next->prev = straight_seg->prev ;
		}  /* delete straight segment from the segment linked lists */
		for( adj = straight_ptr->adjptr->next ;
				      adj ; adj = adj->next ) {
		    if( adj->segptr == straight_seg ) {
			adj->segptr = asegptr[1] ;
			break ;
		    }
		}
		for( i = 2 ; i <= n ; i++ ) {
		    adj = (ADJASEG *)Ysafe_calloc( 1 , sizeof(ADJASEG) );
		    adj->segptr = asegptr[i] ;
		    adj->next = straight_ptr->adjptr->next ;
		    straight_ptr->adjptr->next = adj ;
		}
		netptr->next = nextptr->next ;
		Ysafe_free( straight_seg ) ;
		Ysafe_free( nextptr->eqptr ) ;
		for( adj = nextptr->adjptr ; adj ; adj = adjnext ) {
		    adjnext = adj->next ;
		    Ysafe_free( adj ) ;
		}
		/*   added by Carl 7/14/89       */
		tearrayG[nextptr->terminal] = NULL ;
		Ysafe_free( nextptr ) ;
		/*   end of addition             */
		implicit_pins_usedG-- ;
		remove_feed_flag = 1 ;
	    }
	} else {
	    break ;
	}
    }
    netptr = dimptr->pins ;
    if( netptr->terminal > TotRegPinsG ) {
	if( netptr->adjptr->next == NULL ) {
	    dimptr->pins = netptr->next ;
	    Ysafe_free( netptr->eqptr ) ;
	    Ysafe_free( netptr->adjptr ) ;
	    /*   added by Carl 7/14/89       */
	    tearrayG[netptr->terminal] = NULL ;
	    /*   end of addition             */
	    Ysafe_free( netptr ) ;
	    remove_feed_flag = 1 ;
	} else if( netptr->adjptr->next->next == NULL ) {
	    segptr = netptr->adjptr->next->segptr ;
	    segptr->prev->next = segptr->next ;
	    if( segptr->next != NULL ) {
		segptr->next->prev = segptr->prev ;
	    }
	    if( segptr->pin1ptr == netptr ) {
		ptr = segptr->pin2ptr ;
	    } else {
		ptr = segptr->pin1ptr ;
	    }
	    for( adj = ptr->adjptr ; adj->next ; adj = adj->next ) {
		adjnext = adj->next ;
		if( adjnext->segptr == segptr ) {
		    adj->next = adjnext->next ;
		    Ysafe_free( adjnext ) ;
		    break ;
		}
	    }
	    Ysafe_free( segptr ) ;
	    dimptr->pins = netptr->next ;
	    Ysafe_free( netptr->eqptr ) ;
	    Ysafe_free( netptr->adjptr ) ;
	    /*   added by Carl 7/14/89       */
	    tearrayG[netptr->terminal] = NULL ;
	    /*   end of addition             */
	    Ysafe_free( netptr ) ;
	    remove_feed_flag = 1 ;
	} else if( flag && netptr->adjptr->next->next->next == NULL ) {
	    segptr1 = netptr->adjptr->next->segptr ;
	    segptr2 = netptr->adjptr->next->next->segptr ;
	    if( segptr1->flag == segptr2->flag ) { /*same channel*/
		segptr2->prev->next = segptr2->next ;
		if( segptr2->next != NULL ) {
		    segptr2->next->prev = segptr2->prev ;
		}
		if( segptr1->pin1ptr == netptr ) {
		    ptr1 = segptr1->pin2ptr ;
		} else {
		    ptr1 = segptr1->pin1ptr ;
		}
		if( segptr2->pin1ptr == netptr ) {
		    ptr2 = segptr2->pin2ptr ;
		} else {
		    ptr2 = segptr2->pin1ptr ;
		}
		if( ptr1->xpos <= ptr2->xpos ) {
		    segptr1->pin1ptr = ptr1 ;
		    segptr1->pin2ptr = ptr2 ;
		} else {
		    segptr1->pin1ptr = ptr2 ;
		    segptr1->pin2ptr = ptr1 ;
		}
		if( ptr1->row == ptr2->row &&
		    ptr1->pinloc == NEITHER && ptr2->pinloc == NEITHER ) {
		    if( segptr1->flag == ptr1->row + 1 ) {
			segptr1->switchvalue = swUP ;
		    } else {
			segptr1->switchvalue = swDOWN ;
		    }
		} else {
		    segptr1->switchvalue = nswLINE ;
		}
		/* old way: segptr1->switchvalue = nswLINE ; */
		for( adj = ptr2->adjptr->next ; adj ; 
					adj = adj->next ) {
		    if( adj->segptr == segptr2 ) {
			adj->segptr = segptr1 ;
			break ;
		    }
		}
		Ysafe_free( segptr2 ) ;
		dimptr->pins = netptr->next ;
		Ysafe_free( netptr->eqptr ) ;
		Ysafe_free( netptr->adjptr->next->next ) ;
		Ysafe_free( netptr->adjptr->next ) ;
		Ysafe_free( netptr->adjptr ) ;
		/*   added by Carl 7/14/89       */
		tearrayG[netptr->terminal] = NULL ;
		/*   end of addition             */
		Ysafe_free( netptr ) ;
		implicit_pins_usedG-- ;
		remove_feed_flag = 1 ;
	    }
	} else if( flag ) {
	    fixFlag = 0 ;
	    n = 0 ;
	    adj = netptr->adjptr->next ;
	    channel = adj->segptr->flag ;
	    straight_seg = NULL ;
	    for( ; adj ; adj = adj->next ) {
		segptr = adj->segptr ;
		if( segptr->flag != channel ) {
		    fixFlag = 1 ;
		    break ;
		} else if( segptr->pin1ptr->xpos 
			== segptr->pin2ptr->xpos &&
			straight_seg == NULL ) {
		    straight_seg = segptr ; /* straight vertical segment */
		} else {
		    asegptr[ ++n ] = segptr ;/* switchable segment */
		}
	    }
	    if( fixFlag ) {
	     /* not all the adjcent segments are in the same channel */
	     /* nothing need to be done */
		continue ;
	    }
	    if( straight_seg->pin1ptr == netptr ) {
		straight_ptr = straight_seg->pin2ptr ;
	    } else {
		straight_ptr = straight_seg->pin1ptr ;
	    }
	    /* straight_ptr is the pointer to the terminal which is straight
	       above the feed-through pin */
	    for( i = 1 ; i <= n ; i++ ) {
		if( asegptr[i]->pin1ptr == netptr ) {
		    asegptr[i]->pin1ptr = straight_ptr ;
		} else {
		    asegptr[i]->pin2ptr = straight_ptr ;
		}
		asegptr[i]->switchvalue = nswLINE ;
	    }
	    straight_seg->prev->next = straight_seg->next ;
	    if( straight_seg->next ) {
		straight_seg->next->prev = straight_seg->prev ;
	    }  /* delete the straight segment form the segment linked lists */
	    for( adj = straight_ptr->adjptr->next ;
				  adj ; adj = adj->next ) {
		if( adj->segptr == straight_seg ) {
		    adj->segptr = asegptr[1] ;
		    break ;
		}
	    }
	    for( i = 2 ; i <= n ; i++ ) {
		adj = (ADJASEG *)Ysafe_calloc( 1 , sizeof(ADJASEG) ) ;
		adj->segptr = asegptr[i] ;
		adj->next = straight_ptr->adjptr->next ;
		straight_ptr->adjptr->next = adj ;
	    }
	    dimptr->pins = netptr->next ;
	    Ysafe_free( straight_seg ) ;
	    Ysafe_free( netptr->eqptr ) ;
	    for( adj = netptr->adjptr ; adj ; adj = adjnext ) {
		adjnext = adj->next ;
		Ysafe_free( adj ) ;
	    }
	    /*   added by Carl 7/14/89       */
	    tearrayG[netptr->terminal] = NULL ;
	    Ysafe_free( netptr ) ;
	    /*   end of addition             */
	    implicit_pins_usedG-- ;
	    remove_feed_flag = 1 ;
	}
    }
} while( remove_feed_flag ) ;
return ;
}


find_set_name( v )
INT v ;
{

INT i , k ;

k = 0 ;
while( father_G[v] ) {
    stack_G[ ++k ] = v ;
    v = father_G[v] ;
}
for( i = 1 ; i <= k ; i++ ) {
    father_G[ stack_G[i] ] = v ;
}
/* path compression */

return( v ) ;
}


/*--------------------------------------------------------------*
 * see "The Design and Analysis of Computer Algorithm" by Aho , *
 * Hopcroft and Ullman page 129 to 139 for this algorithm of    *
 * Union and Find problem.                                      *
 *--------------------------------------------------------------*/
do_set_union( i , j )
INT i , j ;
{

INT large , small ;

if( count_G[ root_G[i] ] <= count_G[ root_G[j] ] ) {
    large = root_G[j] ;
    small = root_G[i] ;
} else {
    large = root_G[i] ;
    small = root_G[j] ;
}
father_G[small] = large ;
count_G[large] += count_G[small] ;
}


switchable_or_not()
{

SEGBOXPTR segptr ;
PINBOXPTR ptr1 , ptr2 ;
DOUBLE r ;
INT net ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ; segptr ;
			    segptr = segptr->next ){
	ptr1 = segptr->pin1ptr ;
	ptr2 = segptr->pin2ptr ;
	if( ptr1->row == ptr2->row ) {
	    if( ptr1->row == 0 || ptr1->row == numRowsG + 1 ) {
		segptr->switchvalue = nswLINE ;
	    } else if( ptr1->pinloc != NEITHER || ptr2->pinloc != NEITHER ) {
		segptr->switchvalue = nswLINE ;
	    } else {
		r = (DOUBLE) RAND / (DOUBLE) 0x7fffffff ;
		if( r >= 0.5 ) {
		    segptr->switchvalue = swUP ;
		} else {
		    segptr->switchvalue = swDOWN ;
		}
	    }
	} else {
	    segptr->switchvalue = nswLINE ;
	}
    }
}
}


free_z_memory()
{

INT i , j , last_i ;

j = numRowsG + 1 ;
for( i = 0 ; i <= j ; i++ ) {
    Ysafe_free( z_S[i] ) ;
}
Ysafe_free( z_S ) ;
Ysafe_free( pins_at_rowS ) ;
Ysafe_free( first_indexS ) ;
last_i = maxpin_numberS * maxpin_numberS * numRowsG - 1 ;
for( i = 1 ; i <= last_i ; i++ ) {
    Ysafe_free( edge_dataS[i] ) ;
}
Ysafe_free( edge_dataS ) ;
}




postFeedAssgn_carl()
{

INT net , i , row , botrow , toprow , last_i ;
SEGBOXPTR segptr , nextseg ;
PINBOXPTR netptr , nextptr , st_head , stptr , ptr ;
ADJASEGPTR adj , nextadj ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    for( segptr = netsegHeadG[net]->next ; segptr ; segptr = nextseg ) {
	nextseg = segptr->next ;
	Ysafe_free( segptr ) ;
    }
    netsegHeadG[net]->next = NULL ;

    for( ptr = netarrayG[net]->pins ; ptr ; ptr = ptr->next ) {
	for( adj = ptr->adjptr->next ; adj ; adj = nextadj ) {
	    nextadj = adj->next ;
	    Ysafe_free( adj ) ;
	}
	ptr->adjptr->next = NULL ;
    }
}

maxpin_numberS = 0 ;
pins_at_rowS = (INT *)Ysafe_calloc( numChansG + 1, sizeof(INT) ) ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    if( netarrayG[net]->numpins <= 2 ) {
	continue ;
    }
    botrow = numChansG ;
    toprow = 0 ;
    for( netptr = netarrayG[net]->pins ; netptr ; netptr = netptr->next ) {
	row = netptr->row ;
	pins_at_rowS[ row ]++ ;
	if( row < botrow ) {
	    botrow = row ;
	}
	if( row > toprow ) {
	    toprow = row ;
	}
    }
    for( row = botrow ; row <= toprow ; row++ ) {
	if( pins_at_rowS[row] > maxpin_numberS ) {
	    maxpin_numberS = pins_at_rowS[row] ;
	}
	pins_at_rowS[row] = 0 ;
    }
}

maxpin_numberS += 3 ;
count_G    = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
father_G   = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
root_G     = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
stack_G    = (INT *)Ysafe_malloc( 2 * Max_numPinsG * sizeof( INT ) ) ;
first_indexS = (INT *)Ysafe_malloc( ( numChansG + 1 ) * sizeof( INT ) ) ;
vertex_G   = (PINBOXPTR *)Ysafe_malloc( 2 * Max_numPinsG * sizeof(PINBOXPTR) );
last_i = maxpin_numberS * maxpin_numberS * numRowsG - 1 ;
edge_dataS = (EDGE_COST *)Ysafe_malloc( ( last_i + 1 )
				       * sizeof(EDGE_COST) ) ;
for( i = 1 ; i <= last_i ; i++ ) {
    edge_dataS[i] = (EDGE_COST)Ysafe_malloc( sizeof(EDGE_COST_BOX) );
}
z_S = (PINBOXPTR **)Ysafe_malloc( ( numChansG + 1 ) * sizeof(PINBOXPTR *) ) ;
for( i = 0 ; i <= numChansG ; i++ ) {
    z_S[i] = (PINBOXPTR *)Ysafe_malloc( maxpin_numberS * sizeof(PINBOXPTR) ) ;
}

for( net = 1 ; net <= numnetsG ; net++ ) {
    if( netarrayG[net]->numpins <= 1 ) {
	continue ;
    }
    rebuild_netgraph_carl( net ) ;
    /* find minimum cost tree connection from all the pins of a net */
}
}



rebuild_netgraph_carl( net )
INT net ;
{

PINBOXPTR netptr ;
SEGBOXPTR segptr ;
ADJASEGPTR adj ;
INT row , botrow , toprow , i , j , k , x ;
INT node1 , node2 , last_j , last_k ;
INT num_edge , tree_edge , vtx , j_rite ;
INT comparepinx(), compare_cost() ;

netptr = netarrayG[net]->pins ;
toprow = 0 ;
botrow = numRowsG + 1 ;
for( ; netptr ; netptr = netptr->next ) {
    row = netptr->row ;
    if( row > toprow ) {
	toprow = row ;
    }
    if( row < botrow ) {
	botrow = row ;
    }
    z_S[ row ][ ++pins_at_rowS[row] ] = netptr ;
}
for( i = botrow ; i <= toprow ; i++ ) {
    if( pins_at_rowS[i] == 0 ) {
	printf("there are no pin in row %d for net %d\n", i, net );
	dbx_terminal( net , net ) ;
	YexitPgm(PGMFAIL) ;
    } else {
	if( pins_at_rowS[i] > 1 ) {
	    Yquicksort( (char *)(z_S[i]+1),
		pins_at_rowS[i], sizeof(PINBOXPTR), comparepinx );
	}
    }
}

vtx = 0 ;
num_edge = 0 ;
first_indexS[botrow] = 0 ;
for( i = 1 ; i <= pins_at_rowS[botrow] ; i++ ) {
    vertex_G[ ++vtx ] = z_S[botrow][i] ;
}
for( i = botrow + 1 ; i <= toprow ; i++ ) {
    first_indexS[i] = first_indexS[i-1] + pins_at_rowS[i-1] ;
    for( j = 1 ; j <= pins_at_rowS[i] ; j++ ) {
	vertex_G[ ++vtx ] = z_S[i][j] ;
    }
}
/* build complete graph for pins in adjacent rows */
for( i = botrow ; i < toprow ; i++ ) {
    last_j = first_indexS[i] + pins_at_rowS[i] ;
    last_k = first_indexS[i+1] + pins_at_rowS[i+1] ;
    for( j = first_indexS[i] + 1 ; j <= last_j ; j++ ) {
	if( vertex_G[j]->pinloc < NEITHER ) {
	    continue ;
	}
	for( k = first_indexS[i+1] + 1 ; k <= last_k ; k++ ) {
	    if( vertex_G[k]->pinloc > NEITHER ) {
		continue ;
	    }
	    edge_dataS[ ++num_edge ]->node1 = j ;
	    edge_dataS[   num_edge ]->node2 = k ;
	    x = ABS( vertex_G[j]->xpos - vertex_G[k]->xpos ) ;
	    if( x < average_pin_sepG ) {
		edge_dataS[ num_edge ]->cost = x ;
	    } else {
		edge_dataS[ num_edge ]->cost = 5 * x / 2 ;
	    }
	}
    }
}
/* hook up the pins in the same row one by one */
for( i = botrow ; i <= toprow ; i++ ) {
    last_j = first_indexS[i] + pins_at_rowS[i] ;
    for( j = first_indexS[i] + 1 ; j < last_j ; j++ ) {
	if( vertex_G[j]->pinloc > NEITHER ) {
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc >= NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  = 5 *
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	} else if( vertex_G[j]->pinloc < NEITHER ) {
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc <= NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  = 5 *
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	} else {
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc < NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  = 5 * 
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc > NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  = 5 *
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	    for( j_rite = j+1 ; j_rite <= last_j ; j_rite++ ) {
		if( vertex_G[j_rite]->pinloc == NEITHER ) {
		    edge_dataS[ ++num_edge ]->node1 = j ;
		    edge_dataS[   num_edge ]->node2 = j_rite ;
		    edge_dataS[   num_edge ]->cost  =
			ABS( vertex_G[j]->xpos -
			     vertex_G[j_rite]->xpos ) / 2 ;
		    break ;
		}
	    }
	}
    }
}
/* find the minimum spanning tree for the graph build above   */
/* -----------------------------------------------------------*
 *                                                            *
 *             Kruskal's minimum spanning tree                *
 *                                                            *
 * -----------------------------------------------------------*/
for( i = 1 ; i <= vtx ; i++ ) {
    count_G[i]    = 1 ;
    father_G[i]   = 0 ;
    root_G[i]     = i ;
}
tree_edge = 0 ;
Yquicksort( (char *)(edge_dataS+1), num_edge, sizeof(EDGE_COST), 
    compare_cost ) ;

segptr = netsegHeadG[net] ;
for( i = 1 ; i <= num_edge ; i++ ) {
    node1 = edge_dataS[i]->node1 ;
    node2 = edge_dataS[i]->node2 ;
    j = find_set_name( node1 ) ;
    k = find_set_name( node2 ) ;
    if( j != k ) { /* node1 and node2 belong to two different set*/
	do_set_union( j , k ) ;
	segptr->next =
	    (SEGBOXPTR)Ysafe_calloc( 1, sizeof(SEGBOX) ) ;
	segptr->next->prev = segptr ;
	segptr = segptr->next ;
	if( vertex_G[node1]->xpos <= vertex_G[node2]->xpos ) {
	    segptr->pin1ptr = vertex_G[node1] ;
	    segptr->pin2ptr = vertex_G[node2] ;
	} else {
	    segptr->pin1ptr = vertex_G[node2] ;
	    segptr->pin2ptr = vertex_G[node1] ;
	}

	adj = (ADJASEGPTR)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;
	adj->next = vertex_G[node1]->adjptr->next ;
	adj->segptr = segptr ;
	vertex_G[node1]->adjptr->next = adj ;

	adj = (ADJASEGPTR)Ysafe_calloc( 1, sizeof(ADJASEG) ) ;
	adj->next = vertex_G[node2]->adjptr->next ;
	adj->segptr = segptr ;
	vertex_G[node2]->adjptr->next = adj ;

	if( ++tree_edge >= vtx ) {
	    break ;
	}
    }
}

remove_unnecessary_feed( net , 0 ) ;
    /* remove the feed through pins that has only one edge
       incident on it.                                     */

for( i = botrow ; i <= toprow ; i++ ) {
    pins_at_rowS[i] = 0 ;
}
return ;
}
