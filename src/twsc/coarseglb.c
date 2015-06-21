/*
 *   Copyright (C) 1989-1991 Yale University
 *   Copyright (C) 2013 Tim Edwards <tim@opencircuitdesign.com
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
FILE:	    coarseglb.c                                       
DESCRIPTION:coarse global router code.
CONTENTS:   coarseglb() 
	    assign_row_to_pin()
	    set_up_grid( )
	    initialize_feed_need()
	    feed_config( )
	    set_node( x )
		INT x ;
	    compute_feed_diff( iteration )
	    INT iteration ;
	    space_for_feed( )
	    update_feed_config( iteration )
		INT iteration ;
	    no_of_feedthru_cells()
	    addin_feedcell()
	    final_feed_config( )
	    free_cglb_data()
DATE:	    Mar 27, 1989 
REVISIONS:  Aug 27, 1990 - modified shift so it only shifts if not
		enough room for pads.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) coarseglb.c (Yale) version 4.10 12/9/91" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "main.h"
#include "feeds.h"
#include "parser.h"
#include "pads.h"
#include "readpar.h"

/* static definitions */
static INT *accumulate_feedS , *feed_diffS , *diff_in_rowfeedS ;
static INT *feed_shortS , half_hzsepS , right_Pads_left_edgeS ;

/* global definitions */
INT longest_row_lengthG ;

/* global references */
extern INT add_Lcorner_feedG ;
extern INT extra_cellsG ;
extern INT actual_feed_thru_cells_addedG ;
extern BOOL no_feed_at_endG ;
extern BOOL ignore_feedsG ;

coarseglb() 
{

INT shift ;
INT iteration = 0 ;

assign_row_to_pin() ;
if( case_unequiv_pinG ) {
    unequiv_pin_pre_processing() ;
}
set_up_grid( )  ; /* set up the bin for coarse global routing       */
buildimp( )     ; /* link up all the implicit feed through pins     */
feedest( )      ; /* estimate the number of feed needed in a row    */
printf(" building the steiner trees\n" ) ;
steiner( )      ; /* make steiner tree for all pins in the same net */
feed_config( )  ; /* compute the needed and actual available feed
		     through pins in each bins                      */
cglb_initial( ) ; /* set up some parameter values                   */
proj_tree_to_grid( ) ;
set_cbucket( )   ;

if( SGGRG ) {
    seagate_input() ;
    return ; /* finished with this func. if SGR is to be used */
}
#define EVEN_ROW
#ifdef EVEN_ROW
cglbroute() ;
compute_feed_diff(1) ;
rowevener() ;
printf(" rebuilding the steiner tree\n" ) ;
redo_steiner() ;
initialize_feed_need() ;
reinitial_Hdensity() ;
update_switchvalue() ;
proj_tree_to_grid() ;
rebuild_cbucket() ;
#endif

printf( "\n----start doing coarse global routing ------ \n") ;
while(1) {
    printf(" ITERATION %2d\n", ++iteration ) ;
    cglbroute()  ;
    compute_feed_diff( iteration ) ;
    shift = space_for_feed( ) ;
    if( shift ) {
	update_feed_config(iteration) ;
	reinitial_Hdensity() ;
	update_switchvalue() ;
	proj_tree_to_grid() ;
	rebuild_cbucket() ;
    } else {
	break ;
    }
}
free_cglb_initial( ) ;
fdthrusG = no_of_feedthru_cells( ) ;
fixwolf() ;
addin_feedcell() ;
final_feed_config( ) ;
free_cglb_data( ) ;

}


assign_row_to_pin()
{

CBOXPTR cellptr ;
PINBOXPTR pinptr ;
INT i , block ;

for( i = 1 ; i <= numcellsG ; i++ ) {
    cellptr = carrayG[i] ;
    block = cellptr->cblock ;
    for( pinptr = cellptr->pins; pinptr ; pinptr = pinptr->nextpin ){
	pinptr->row = block ;
    }
}
}


    
set_up_grid( )
{

INT i , j , x , row_rite ;
INT left, right, bottom, top ;
INT row , cell , last_cell , class ;
INT curr_row_rite , max_desire, prev_row_rite , padside ;
TIBOXPTR tptr ;

numChansG = numRowsG + 1 ;
accumulate_feedS = (INT *)Ysafe_malloc( numChansG * sizeof(INT) ) ;
/* set up the grid points in each rows for coarse routing */

if( average_pin_sepG <= 3.25 * average_pin_sepG ) {
    hznode_sepG = 2.3 * average_feed_sepG ;
} else {
    hznode_sepG = 7.5 * average_pin_sepG ;
}
		/* horizontal node separation */
half_hzsepS = hznode_sepG / 2 ;
blk_most_leftG = barrayG[1]->bxcenter + barrayG[1]->bleft ;
for( row = 2 ; row <= numRowsG ; row++ ) {
    if( barrayG[row]->bxcenter + barrayG[row]->bleft < blk_most_leftG ) {
	blk_most_leftG = barrayG[row]->bxcenter + barrayG[row]->bleft ;
    }
}

blk_most_riteG = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    last_cell = pairArrayG[row][ pairArrayG[row][0] ] ;
    row_rite  = carrayG[ last_cell ]->cxcenter
		+ carrayG[ last_cell ]->tileptr->right ;
    if( row_rite > blk_most_riteG ) {
	blk_most_riteG = row_rite ;
    }
}
max_desire = barrayG[1]->desire ;
for( row = 2 ; row <= numRowsG ; row++ ) {
    if( max_desire < barrayG[row]->desire ) {
	max_desire = barrayG[row]->desire ;
    }
}
if( blk_most_riteG - blk_most_leftG >= max_desire ) {
    chan_node_noG = ( blk_most_riteG - blk_most_leftG) / hznode_sepG + 1 ;
    chan_node_noG += chan_node_noG ;
} else {
    chan_node_noG = max_desire / hznode_sepG + 1 ;
    chan_node_noG += chan_node_noG ;
}
/* chan_node_noG is the number of horizontal edges in each row.
   Make one more node in case the row length is longer
   because of feedthrough cells add in                         */

feedpptrG  = (FEED_DATA **)Ysafe_malloc( 
	    numChansG * sizeof(FEED_DATA *) );
for( i = 1 ; i <= numRowsG ; i++ ) {
    feedpptrG[i]  = (FEED_DATA *) Ysafe_malloc( 
		( chan_node_noG + 1 ) * sizeof( FEED_DATA ) ) ;
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	feedpptrG[i][j] = (FEED_DATA)Ysafe_calloc( 1,sizeof(FEED_DBOX) ) ;
    }
}
diff_in_rowfeedS  = (INT *)Ysafe_malloc( numChansG * sizeof(INT) ) ;
feed_diffS = (INT *)Ysafe_calloc( chan_node_noG + 1, sizeof( INT )) ;
fdcel_addedG  = (INT *)Ysafe_calloc( numChansG, sizeof( INT ) ) ;
feed_shortS = (INT *)Ysafe_calloc( numChansG, sizeof( INT ) ) ;
fdcel_needG  = (INT **)Ysafe_calloc( numChansG, sizeof(INT *) );
for( i = 1 ; i <= numRowsG ; i++ ) {
    fdcel_needG[i]  = (INT *)Ysafe_calloc( chan_node_noG+1, sizeof(INT) ) ;
}

/* in order to take care of the circuits with 
	    macros we need to do the following */
row_rite_classG = (INT *) Ysafe_malloc( numChansG * sizeof(INT) ) ;
right_most_in_classG = (INT *)Ysafe_malloc( numChansG * sizeof(INT) ) ;
class = 0 ;
prev_row_rite = -1 ;
row_rite_classG[1] = class ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    curr_row_rite = barrayG[row]->bxcenter
		+ barrayG[row]->bleft + barrayG[row]->desire ;
    if( curr_row_rite == prev_row_rite ) {
	row_rite_classG[row] = class ;
    } else {
	row_rite_classG[row] = ++class ;
	prev_row_rite = curr_row_rite ;
    }
}
right_Pads_left_edgeS = INFINITY ;
for( cell = numcellsG + 1 ; cell <= lastpadG ; cell++ ) {
    padside = carrayG[cell]->padptr->padside ;
    if( padside == R ) {
	tptr = carrayG[cell]->tileptr ;
	left = tptr->left ;
	right = tptr->right ;
	bottom = tptr->bottom ;
	top = tptr->top ;
	YtranslateT( &left, &bottom, &right, &top, (INT)carrayG[cell]->corient ) ;
	x = carrayG[cell]->cxcenter + left ;
	if( x < right_Pads_left_edgeS ) {
	    right_Pads_left_edgeS = x ;
	}
    }
}
}

initialize_feed_need()
{

INT i , row ;
FEED_DATA *feedptr ;

for( row = 1 ; row <= numRowsG ; row++ ) {
    fdcel_addedG[row] = 0 ;
    feedptr = feedpptrG[row] ;
    for( i = 1 ; i <= chan_node_noG ; i++ ) {
	feedptr[i]->needed = 0 ;
	feedptr[i]->actual = 0 ;
    }
}
feed_config() ;
}


feed_config( )
{

INT row , cell , cxcenter , k ;
CBOXPTR cellptr ;
IPBOXPTR imptr ;

for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    cellptr = carrayG[ cell ] ;
    cxcenter = cellptr->cxcenter ;
    row = cellptr->cblock ;
    for( imptr = cellptr->imptr ; imptr ; imptr = imptr->next ) {
	if( cellptr->corient <= 1 ) {
	    imptr->xpos = cxcenter + imptr->txpos ;
	} else {
	    if( cellptr->clength % 2 == 0 ) {
		imptr->xpos = cxcenter - imptr->txpos ;
	    } else {
		imptr->xpos = cxcenter - imptr->txpos - 1 ;
	    }
	}
	k = set_node( imptr->xpos ) ;
	feedpptrG[row][k]->actual++ ;
    }
}
}


set_node( x )
INT x ;
{

DOUBLE h ;

h = (DOUBLE)( x - blk_most_leftG ) / (DOUBLE)hznode_sepG + 1.5 ;
if( h < 1 ) {
    return( 1 ) ;
} else if( h > chan_node_noG ) {
    return( chan_node_noG ) ;
} else {
    return( (INT)h ) ;
}
}


compute_feed_diff( iteration )
INT iteration ;
{

INT i , j , k , range , left_node , rite_node ;

if( ignore_feedsG || try_not_to_add_explicit_feedsG ) {
    range = chan_node_noG ;
} else if( iteration <= 1 ) {
    if( add_Lcorner_feedG ) {
	range = 5 + iteration ;
    } else {
	range = iteration ;
    }
} else {
    range = chan_node_noG ;
}
for( i = 1 ; i <= numRowsG ; i++ ) {
    feed_shortS[i] = 0 ;
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	feed_diffS[j] = feedpptrG[i][j]->actual - feedpptrG[i][j]->needed ;
	feed_shortS[i] -= feed_diffS[j] ;
    }
    for( j = 1 ; j <= chan_node_noG ; j++ ) {
	if( feed_diffS[j] < 0 ) {
	    left_node = rite_node = j ;
	    for( k = 1 ; k <= range ; k++ ) {
		if( left_node > 1 ) {
		    if( feed_diffS[--left_node] > 0 ) {
			if( feed_diffS[left_node] + feed_diffS[j] < 0 ) {
			    feed_diffS[j] += feed_diffS[left_node] ;
			    feed_diffS[left_node] = 0 ;
			} else {
			    feed_diffS[left_node] += feed_diffS[j] ;
			    feed_diffS[j] = 0 ;
			    break ;
			}
		    }
		}
		if( rite_node < chan_node_noG ) {
		    if( feed_diffS[++rite_node] > 0 ) {
			if( feed_diffS[rite_node] + feed_diffS[j] < 0 ) {
			    feed_diffS[j] += feed_diffS[rite_node] ;
			    feed_diffS[rite_node] = 0 ;
			} else {
			    feed_diffS[rite_node] += feed_diffS[j] ;
			    feed_diffS[j] = 0 ;
			    break ;
			}
		    }
		}
	    }
	}
	if( feed_diffS[j] < 0 ) {
	    fdcel_needG[i][j] = - feed_diffS[j] ;
	} else {
	    fdcel_needG[i][j] = 0 ;
	}
    }
}
}


space_for_feed( )
{

PINBOXPTR pinptr ;
CBOXPTR cellptr ;
INT row , i , Flag , shiftFlag , *Aray ;
INT nodex , node , shift , patch_shift ;
INT locFdWidth = fdWidthG;

// if ( ignore_feedsG ) locFdWidth = 0;

shiftFlag = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    Flag = 1 ;
    Aray = pairArrayG[row] ;
    accumulate_feedS[row] = 0 ;
    for( node = 1 ; node <= chan_node_noG ; node++ ) {
	if( fdcel_needG[row][node] != 0 ) {
	    Flag = 0 ;
	    /* the first position need to add feed thru cell */
	    break ;
	}
    }
    if( Flag ) {
	continue ;  /* no need to add actual feed thru cells */
    }
    shiftFlag = 1 ;
    nodex = blk_most_leftG + ( node - 1 ) * hznode_sepG ;
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    if( Aray[0] == 1 ) {
	patch_shift = locFdWidth * feed_shortS[row] ;
	cellptr->cxcenter += patch_shift ;
	for( pinptr = cellptr->pins;pinptr; pinptr = pinptr->nextpin ) {
	    pinptr->xpos += patch_shift ;
	}
	fdcel_addedG[row] += feed_shortS[row] ;
	continue ;
    }
    if( nodex >= cellptr->cxcenter ) { 
	if( feed_shortS[row] > 0 ) {
	    patch_shift = locFdWidth * feed_shortS[row] ;
	    cellptr->cxcenter += patch_shift ;
	    for( pinptr = cellptr->pins;pinptr;pinptr=pinptr->nextpin ) {
		pinptr->xpos += patch_shift ;
	    }
	    fdcel_addedG[row] += feed_shortS[row] ;
	}
	continue ;
    }
    for( i = 1 ; i <= Aray[0] ; i++ ) {
	cellptr = carrayG[ Aray[i] ] ;
	if( cellptr->cxcenter > nodex && cellptr->cclass != -2 ) {
	    break ;
	}
    }
    nodex += hznode_sepG ;
    for( ; node <= chan_node_noG ; node++ , nodex += hznode_sepG ) {
	accumulate_feedS[row] += fdcel_needG[row][node] ;
	shift = accumulate_feedS[row] * locFdWidth ;
	for( ; i <= Aray[0] ; i++ ) {
	    cellptr = carrayG[ Aray[i] ] ;
	    if( cellptr->cclass == -3 ) {
		break ;
	    } else if( cellptr->cxcenter < nodex ) {
		cellptr->cxcenter += shift ;
		for( pinptr=cellptr->pins;pinptr;pinptr=pinptr->nextpin ){
		    pinptr->xpos += shift ;
		}
	    } else {
		break ;
	    }
	}
	if( i > Aray[0] ) {
	    break ;
	}
    }
    for( ; i <= Aray[0] ; i++ ) {
	carrayG[ Aray[i] ]->cxcenter += shift ;
	for( pinptr = carrayG[ Aray[i] ]->pins ;pinptr;
				pinptr = pinptr->nextpin ) {
	    pinptr->xpos += shift ;
	}
    }
    if( feed_shortS[row] > accumulate_feedS[row] ) {
	patch_shift = locFdWidth *
	    ( feed_shortS[row] - accumulate_feedS[row] ) ;
	if( carrayG[ Aray[ Aray[0] ] ]->cclass == -3 ) {
	    for( i = Aray[0] - 1 ; i >= 1 ; i-- ) {
		if( carrayG[ Aray[i] ]->cclass != -3 ) {
		    i++ ;
		    break ;
		}
	    }
	} else {
	    i = Aray[0] ;
	}
	for( ; i <= Aray[0] ; i++ ) {
	    cellptr = carrayG[ Aray[i] ] ;
	    cellptr->cxcenter += patch_shift ;
	    for( pinptr = cellptr->pins;pinptr;pinptr=pinptr->nextpin ) {
		pinptr->xpos += patch_shift ;
	    }
	}

	fdcel_addedG[row] += feed_shortS[row] ;
    } else {
	fdcel_addedG[row] += accumulate_feedS[row] ;
    }
}

/* added by Carl 12/7/91 */
for( row = 1 ; row <= numRowsG ; row++ ) {
    if( fdcel_addedG[row] < 0 ) {
	fdcel_addedG[row] = 0 ;
    }
}
/* added by Carl 12/7/91 */

return( shiftFlag ) ;
}


update_feed_config( iteration )
INT iteration ;
{

INT cell , padside , shift ;
INT *Aray , i , k , row , row_left , row_rite ;
INT last , orient , curr_rite , next_left , lastcell_rite ;
INT feedx , last_feedx , first_cell_left ;
CBOXPTR cellptr , nextptr , currptr ;
PINBOXPTR pin ;
FEED_DATA *feedptr ;
IPBOXPTR imptr ;

INT locFdWidth = fdWidthG;

// if ( ignore_feedsG ) locFdWidth = 0;

/*********************************************************************
*   if we need to add in feed through cells , we first create a      *
* gap space between adjacent cells where real feed through cells     *
* will be placed later on. Thus the new available number of feed     *
* through cells in each bins will be equal to the number of implicit *
* feed through pins in this bin plus the gap space in this bin       *
* divide by the width of feed through cells.                         *
**********************************************************************/
for( row = 1 ; row <= numRowsG ; row++ ) {
    Aray = pairArrayG[row] ;
    feedptr = feedpptrG[row] ;
    for( k = 1 ; k <= chan_node_noG ; k++ ) {
	feedptr[k]->actual = 0 ;
	feedptr[k]->needed = 0 ;
    }
    currptr = carrayG[ Aray[1] ] ;
    first_cell_left = currptr->cxcenter + currptr->tileptr->left ;
    row_left = barrayG[row]->bxcenter + barrayG[row]->bleft ;
    if( row_left != first_cell_left ) {
	feedx = row_left + locFdWidth / 2 ;
	for( ; feedx < first_cell_left ; feedx += locFdWidth ) {
	    k = set_node( feedx ) ;
	    feedptr[k]->actual++ ;
	}
    }
    last = Aray[0] - 1 ;
    for( i = 1 ; i <= last ; i++ ) {
	nextptr = carrayG[ Aray[i+1] ] ;
	orient = currptr->corient ;
	for( imptr = currptr->imptr ; imptr ; imptr = imptr->next ) {
	    if( orient <= 1 ) {
		imptr->xpos = currptr->cxcenter + imptr->txpos ;
	    } else {
		if( currptr->clength % 2 == 0 ) {
		    imptr->xpos = currptr->cxcenter - imptr->txpos ;
		} else {
		    imptr->xpos = currptr->cxcenter - imptr->txpos - 1 ;
		}
	    }
	    k = set_node( imptr->xpos ) ;
	    feedptr[k]->actual++ ;
	}
	curr_rite = currptr->cxcenter + currptr->tileptr->right ;
	next_left = nextptr->cxcenter + nextptr->tileptr->left ;
	if( curr_rite != next_left ) {
	    for( feedx = curr_rite + locFdWidth / 2 ;
		feedx < next_left ; feedx += locFdWidth ) {
		k = set_node( feedx ) ;
		feedptr[k]->actual++ ;
	    }
	}
	currptr = nextptr ;
    }
    orient = currptr->corient ;
    for( imptr = currptr->imptr ; imptr ; imptr = imptr->next ) {
	if( orient <= 1 ) {
	    imptr->xpos = currptr->cxcenter + imptr->txpos ;
	} else {
	    if( currptr->clength % 2 == 0 ) {
		imptr->xpos = currptr->cxcenter - imptr->txpos ;
	    } else {
		imptr->xpos = currptr->cxcenter - imptr->txpos - 1 ;
	    }
	}
	k = set_node( imptr->xpos ) ;
	feedptr[k]->actual++ ;
    }
}
blk_most_riteG = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    row_rite = cellptr->cxcenter + cellptr->tileptr->right ;
    if( row_rite > blk_most_riteG ) {
	blk_most_riteG = row_rite ;
    }
}

decide_right_most_in_class() ;

if( iteration >= 2 && !no_feed_at_endG ) {
    for( row = 1 ; row <= numRowsG ; row++ ) {
	cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
	lastcell_rite  = cellptr->cxcenter + cellptr->tileptr->right ;
	feedx = lastcell_rite + locFdWidth / 2 ;
	last_feedx = right_most_in_classG[ row_rite_classG[row] ]
					- ( locFdWidth + 1 ) / 2 ;
	feedptr = feedpptrG[row] ;
	for( ; feedx <= last_feedx ; feedx += locFdWidth ) {
	    k = set_node( feedx ) ;
	    feedptr[k]->actual++ ;
	}
    }
}
if( rowsG == 0 && blk_most_riteG >= right_Pads_left_edgeS ) {
    shift = blk_most_riteG - right_Pads_left_edgeS + 30 ;
    right_Pads_left_edgeS += shift ;

    for( cell = numcellsG + 1 ; cell <= lastpadG ; cell++ ) {
	padside = carrayG[cell]->padptr->padside ;
	if( padside ==   R || padside == MR ||
	    padside == MUR || padside == MLR ) {
	    carrayG[cell]->cxcenter += shift ;

	    for( pin = carrayG[cell]->pins; pin ; pin = pin->nextpin ){
		pin->xpos += shift ;
	    }
	}
    }
}
}


no_of_feedthru_cells()
{

INT i , row , n , difference , lastcell_rite , total_feedthrus ;
CBOXPTR cellptr ;
FEED_DATA *feedptr ;

INT locFdWidth = fdWidthG;

// if (ignore_feedsG) locFdWidth = 0;

total_feedthrus = 0 ;

for( row = 1 ; row <= numRowsG ; row++ ) {
    difference = 0 ;
    feedptr = feedpptrG[row] ;
    for( i = 1 ; i <= chan_node_noG ; i++ ) {
	difference += feedptr[i]->actual - feedptr[i]->needed ;
    }
    if( difference < 0 ) {
	diff_in_rowfeedS[row] = -difference ;
	if( no_feed_at_endG ) {
	    printf(" not enough feed were added in\n" ) ;
	    exit(0) ;
	}
    } else {
	diff_in_rowfeedS[row] = 0 ;
    }
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    lastcell_rite  = cellptr->cxcenter + cellptr->tileptr->right ;
    if( locFdWidth == 0 || cellptr->cclass == -3 ) {
	n = 0 ;
    } else {
	n = ( right_most_in_classG[ row_rite_classG[row] ] 
				- lastcell_rite ) / locFdWidth ;
    }
    total_feedthrus += ( fdcel_addedG[row] + diff_in_rowfeedS[row] + n ) ;
}
return( total_feedthrus ) ;
}


addin_feedcell()
{

INT row , i , k , r , last , feednum , row_left ;
INT curr_rite , next_left , half_fdWidthG , feedx , last_feedx ;
INT *Aray , *nAray , last_rite_edge , first_cell_left ;
CBOXPTR current , nextone , lastptr ;

INT locFdWidth = fdWidthG;

// if ( ignore_feedsG ) locFdWidth = 0;


half_fdWidthG = locFdWidth / 2 ;
feednum = 0 ;
/*
fp = TWOPEN( "rowlen.dat" , "a", ABORT ) ;
fprintf(fp,"\n the row length after coarse global routing are\n" ) ;
fprintf(fp," row  row_length  desire  over/under fdthrus\n" ) ;
*/
for( row = 1 ; row <= numRowsG ; row++ ) {
    Aray = pairArrayG[row] ;
    lastptr = carrayG[ Aray[ Aray[0] ] ] ;
    last_rite_edge = lastptr->cxcenter + lastptr->tileptr->right ;

    if( !no_feed_at_endG ) {
	r = ( right_most_in_classG[ row_rite_classG[row] ] 
			- last_rite_edge ) / locFdWidth + 1 ;
    } else {
	r = 0 ;
    }
    /*
    n = fdcel_addedG[row] + diff_in_rowfeedS[row] ;
    fprintf(fp," %3d %10d  %6d  %10d %6d\n", row, last_rite_edge -
	barrayG[row]->bxcenter - barrayG[row]->bleft ,
	barrayG[row]->desire, last_rite_edge - barrayG[row]->desire -
	barrayG[row]->bxcenter - barrayG[row]->bleft , n ) ;
    */
    if( fdcel_addedG[row] == 0 ) {
	if( r > 1 ) {
	    Aray = (INT *)Ysafe_realloc( Aray ,
	    (2 * ( r + Aray[0] + diff_in_rowfeedS[row] + 1 ) +
	    (extra_cellsG / numRowsG) * 4) * sizeof(INT) ) ;
	    feedx = last_rite_edge + half_fdWidthG ;
	    last_feedx = right_most_in_classG[ row_rite_classG[row] ]
					    - ( locFdWidth + 1 ) / 2 ;
	    for( ; feedx <= last_feedx ; feedx += locFdWidth ) {
		addfeed( row , feedx , ++feednum ) ;
		Aray[ ++Aray[0] ] = numcellsG + numtermsG + feednum ;
	    }
	    last_feedx += diff_in_rowfeedS[row] * locFdWidth ;
	    for( ; feedx <= last_feedx ; feedx += locFdWidth ) {
		addfeed( row , feedx , ++feednum ) ;
		Aray[ ++Aray[0] ] = numcellsG + numtermsG + feednum ;
		k = set_node( feedx ) ;
		feedpptrG[row][k]->actual++ ;
	    }
	}
	pairArrayG[row] = Aray ;
	continue ;
    }

    nAray = (INT *)Ysafe_malloc( (2 * ( Aray[0] + fdcel_addedG[row] +
		diff_in_rowfeedS[row] + r + 1 ) 
		+ (extra_cellsG / numRowsG ) * 4 ) * sizeof( INT ) ) ;
    nAray[0] = 0 ;

    current = carrayG[ Aray[1] ] ;
    first_cell_left = current->cxcenter + current->tileptr->left ;
    row_left = barrayG[row]->bxcenter + barrayG[row]->bleft ;
    if( row_left != first_cell_left ) {
	feedx = row_left + locFdWidth / 2 ;
	for( ; feedx < first_cell_left ; feedx += locFdWidth ) {
	    addfeed( row , feedx , ++feednum ) ;
	    nAray[ ++nAray[0] ] = numcellsG + numtermsG + feednum ;
	}
    }
    last = Aray[0] - 1 ;
    for( i = 1 ; i <= last ; i++ ) {
	nAray[ ++nAray[0] ] = Aray[i] ;
	nextone = carrayG[ Aray[i+1] ] ;
	curr_rite = current->cxcenter + current->tileptr->right ;
	next_left = nextone->cxcenter + nextone->tileptr->left  ;
	if( curr_rite < next_left ) {
	    feedx = curr_rite + half_fdWidthG ;
	    for( ; feedx < next_left ; feedx += locFdWidth ) {
		addfeed( row , feedx , ++feednum ) ;
		nAray[ ++nAray[0] ] = numcellsG + numtermsG + feednum ;
	    }
	}
	current = nextone ;
    }
    nAray[ ++nAray[0] ] = Aray[ Aray[0] ] ;

    if( !no_feed_at_endG ) {
	feedx = last_rite_edge + half_fdWidthG ;
	last_feedx = right_most_in_classG[ row_rite_classG[row] ] 
					    - ( locFdWidth + 1 ) / 2;
	for( ; feedx <= last_feedx ; feedx += locFdWidth ) {
	    addfeed( row , feedx , ++feednum ) ;
	    nAray[ ++nAray[0] ] = numcellsG + numtermsG + feednum ;
	}
	last_feedx += diff_in_rowfeedS[row] * locFdWidth ;
	for( ; feedx <= last_feedx ; feedx += locFdWidth ) {
	    addfeed( row , feedx , ++feednum ) ;
	    nAray[ ++nAray[0] ] = numcellsG + numtermsG + feednum ;
	    k = set_node( feedx ) ;
	    feedpptrG[row][k]->actual++ ;
	}
    }

    pairArrayG[row] = nAray ;
    Ysafe_free( Aray ) ;
}
actual_feed_thru_cells_addedG = feednum ;

/* Added by Tim, 8/26/2013 */
expand_heat_index();

/*
TWCLOSE(fp) ;
*/
}


final_feed_config( )
{

IPBOXPTR imptr ;
FEED_DATA *feedptr ;
CBOXPTR first_cptr , last_cptr ;
INT i , row , longest_row , max_length , k , length , *Aray ;
INT delta_row_len ;

for( row = 1 ; row <= numRowsG ; row++ ) {
    Aray = pairArrayG[row] ;
    for( i = 1 ; i <= Aray[0] ; i++ ) {
	for( imptr = carrayG[ Aray[i] ]->imptr ; imptr ;
				    imptr = imptr->next ) {
	    k = set_node( imptr->xpos ) ;
	    if( feedpptrG[row][k]->firstimp ) {
		feedpptrG[row][k]->lastimp = imptr ;
	    } else {
		feedpptrG[row][k]->firstimp = imptr ;
		feedpptrG[row][k]->lastimp  = imptr ;
	    }
	}
    }
}
total_feed_in_the_rowG = (INT *)Ysafe_malloc( numChansG * sizeof(INT) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    k = 0 ;
    feedptr = feedpptrG[row] ;
    for( i = 1 ; i <= chan_node_noG ; i++ ) {
	k += feedptr[i]->actual ;
    }
    total_feed_in_the_rowG[row] = k ;
}

fprintf(fpoG,"After Feeds are Added:\n");
fprintf(fpoG,"BLOCK      TOTAL CELL LENGTHS      OVER/UNDER TARGET\n");
max_length = 0 ;
for( i = 1 ; i <= numRowsG ; i++ ) {
    Aray = pairArrayG[i] ;
    first_cptr = carrayG[ Aray[1] ] ;
    last_cptr  = carrayG[ Aray[ Aray[0] ] ] ;
    length = last_cptr->cxcenter + last_cptr->tileptr->right -
	     first_cptr->cxcenter - first_cptr->tileptr->left ;
    delta_row_len = length - barrayG[i]->desire ;
    fprintf( fpoG, "%3d            %7d                %6d\n", i,
			length , delta_row_len );
    if( max_length < length ) {
	longest_row = i ;
	max_length = length ;
    }
}
fprintf( fpoG, "\nLONGEST Row is:%d   Its length is:%d\n",
			    longest_row , max_length ) ;
longest_row_lengthG = max_length ;
printf("\n  longest Row is:%d   Its length is:%d\n",
			    longest_row , max_length ) ;
}


free_cglb_data()
{

INT i , net ;
PINBOXPTR netptr ;
ADJASEGPTR adjptr , saveptr ;


for( i = 1 ; i <= numRowsG ; i++ ) {
    Ysafe_free( fdcel_needG[i] ) ;
}
Ysafe_free( fdcel_needG ) ;
Ysafe_free( fdcel_addedG ) ;
Ysafe_free( feed_diffS ) ;
Ysafe_free( feed_shortS ) ;
Ysafe_free( row_rite_classG) ;
Ysafe_free( right_most_in_classG) ;
Ysafe_free( accumulate_feedS ) ;
Ysafe_free( diff_in_rowfeedS ) ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    for( netptr = netarrayG[net]->pins ; netptr ;netptr = netptr->next ) {
	for( adjptr = netptr->adjptr->next ; adjptr ;
				    adjptr = saveptr ) {
	    saveptr = adjptr->next ;
	    Ysafe_free( adjptr ) ;
	}
	netptr->adjptr->next = NULL ;
    }
    for( netptr = steinerHeadG[net]->next;netptr;netptr = netptr->next ) {
	for( adjptr = netptr->adjptr->next ; adjptr ;
				    adjptr = saveptr ) {
	    saveptr = adjptr->next ;
	    Ysafe_free( adjptr ) ;
	}
	netptr->adjptr->next = NULL ;
    }
}
}
