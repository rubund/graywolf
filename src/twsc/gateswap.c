/*
 *   Copyright (C) 1989-1992 Yale University
 *   Copyright (C) 2015 Tim Edwards <tim@opencircuitdesign.com>
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
FILE:	    gateswap.c                                       
DESCRIPTION:evaluates proposed gate swaps
CONTENTS:   gateswap()
DATE:	    May 19, 1989 
REVISIONS:  Apr  1, 1990 - fixed gateswap routine by changing sortpins
		for new data structure.  Also added missing swap of
		txpos fields.  Needed to pass the beginning of
		the cell pin listx instead of pina and pinb.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) gateswap.c (Yale) version 4.6 2/23/92" ;
#endif
#endif

#include <string.h>
#include "ucxxglb.h"
#include "parser.h"
#include <yalecad/debug.h>
#include "readnets.h"

gate_swap( between_two_cells, sgidxa, sgidxb )
INT between_two_cells ;
INT sgidxa, sgidxb ; 
{

CBOXPTR cell1ptr , cell2ptr ;
PINLIST *term1 , *term2 , *term_list1 , *term_list2 , *term ;
PINBOXPTR pin1 , pin2 , pina, pinb ;
char *tmp_char_ptr ;
struct equiv_box *tmp_eqptr ;
INT cost , cell1 , cell2 , length ;
INT tmp , i, swap_group ;
INT truth , count ;
INT pg1 , pg2 ;  /* pg stands for 'pin group' */
INT newtimepenal ;
SGLISTPTR cell1sgl, cell2sgl;

term_list1 = NULL;
term_list2 = NULL;

if( between_two_cells ) {
    cell1 = aG ;
    cell2 = bG ;
    cell1ptr = carrayG[cell1] ;
    cell2ptr = carrayG[cell2] ;
    cell1sgl = cell1ptr->swapgroups + sgidxa;
    cell2sgl = cell2ptr->swapgroups + sgidxb;
    pg1 = XPICK_INT( 1 , cell1sgl->num_pin_group , 0 ) ;
    pg2 = XPICK_INT( 1 , cell2sgl->num_pin_group , 0 ) ;
    swap_group = cell1sgl->swap_group;
    
    term_list1 = (PINLISTPTR) Yhash_search(
		swap_group_listG[swap_group].pin_grp_hash,
		cell1ptr->cname, NULL, FIND );

    while (pg1 > 1) {
	if (term_list1->next_grp == NULL) break;
	term_list1 = term_list1->next_grp;
    }

    term_list2 = (PINLISTPTR) Yhash_search(
		swap_group_listG[swap_group].pin_grp_hash,
		cell2ptr->cname, NULL, FIND );

    while (pg2 > 1) {
	if (term_list2->next_grp == NULL) break;
	term_list2 = term_list2->next_grp;
    }

} else {
    cell1 = aG ;
    cell2 = aG ;
    cell1ptr = carrayG[cell1] ;
    cell1sgl = cell1ptr->swapgroups + sgidxa;
    swap_group = cell1sgl->swap_group ;
    pg1 = XPICK_INT( 1 , cell1sgl->num_pin_group , 0 ) ;
    pg2 = XPICK_INT( 1 , cell1sgl->num_pin_group , pg1 ) ;

    if( pg1 > pg2 ) {
	/*  we want to ensure that pg1 < pg2  */
	i = pg1 ;
	pg1 = pg2 ;
	pg2 = i ;
    }

    term = (PINLISTPTR) Yhash_search(
		swap_group_listG[swap_group].pin_grp_hash,
		cell1ptr->cname, NULL, FIND );

    term_list1 = term;
    while (pg1 > 1) {
	term_list1 = term_list1->next_grp;
    }

    term_list2 = term;
    while (pg2 > 1) {
	term_list2 = term_list2->next_grp;
    }
}

for( term1 = term_list1 ; term1 != NULL ; term1 = term1->next ) {
    pin1 = term1->swap_pin ;
    netarrayG[ pin1->net ]->dflag = 1 ;
    pin1->flag = 1 ;
    length = strcspn( pin1->pinname , "/" ) ;

    for( term2 = term_list2 ; term2 != NULL ; term2 = term2->next ) {
	pin2 = term2->swap_pin ;
	if( length == strcspn( pin2->pinname , "/" ) ) {
	    if( strncmp( pin1->pinname , 
			pin2->pinname , length ) == 0 ) {
		/*  then this is the pair of pins to swap  */
		netarrayG[ pin2->net ]->dflag = 1 ;
		pin2->flag = 1 ;
		pin1->newx = pin2->xpos ;
		pin1->newy = pin2->ypos ;
		pin2->newx = pin1->xpos ;
		pin2->newy = pin1->ypos ;
	    }
	}
    }
}
ASSERT( cell1 == term_list1->swap_pin->cell, NULL, "trouble\n" ) ;
pina = carrayG[cell1]->pins ;
ASSERT( cell2 == term_list2->swap_pin->cell, NULL, "trouble\n" ) ;
pinb = carrayG[cell2]->pins ;

cost = funccostG ; 
clear_net_set() ; /* reset set to mark nets that have ch'ged position */
/* dimbox routines mark the nets that have changed */
if( cell1 != cell2 ) {
    init_dbox_pos_swap( pina ) ;
    init_dbox_pos_swap( pinb ) ;
    new_dbox2( pina , pinb , &cost ) ;
} else {
    init_dbox_pos_swap( pina ) ;
    new_dbox( pina , &cost ) ;
}

newtimepenal = timingcostG ;
if( cell1 != cell2 ) {
    newtimepenal += calc_incr_time2( cell1, cell2 ) ;
} else {
    newtimepenal += calc_incr_time( cell1 ) ;
}

ASSERT( dcalc_full_penalty(newtimepenal),NULL,"time problem") ;


truth = acceptt( 2*(funccostG-cost), 2*(timingcostG-newtimepenal), 0 ) ;

if( truth ) {

    for( term1 = term_list1 ; term1 != NULL ; term1 = term1->next ) {
	pin1 = term1->swap_pin ;
	length = strcspn( pin1->pinname , "/" ) ;
	for( term2 = term_list2; term2 != NULL ; term2 = term2->next ) {
	    pin2 = term2->swap_pin ;
	    if( length == strcspn( pin2->pinname , "/" ) ) {
		if( strncmp( pin1->pinname , 
				pin2->pinname , length ) == 0 ) {
		    /*  this is the pair of pins which swapped  */
	   	    term1->swap_pin = pin2 ; 
	   	    term2->swap_pin = pin1 ; 

		    tearrayG[ pin1->terminal ] = pin2 ;
		    tearrayG[ pin2->terminal ] = pin1 ;

		    tmp_eqptr = pin1->eqptr ;
		    pin1->eqptr = pin2->eqptr ;
		    pin2->eqptr = tmp_eqptr ;

		    tmp_char_ptr = pin1->pinname ;
		    pin1->pinname = pin2->pinname ;
		    pin2->pinname = tmp_char_ptr ;

		    tmp = pin1->terminal         ;
		    pin1->terminal = pin2->terminal ;
		    pin2->terminal = tmp         ;

		    tmp = pin1->cell             ;
		    pin1->cell = pin2->cell   ;
		    pin2->cell = tmp             ;

		    tmp = pin1->pinloc           ;
		    pin1->pinloc = pin2->pinloc ;
		    pin2->pinloc = tmp           ;

		    pin1->xpos = pin1->newx   ;
		    pin1->ypos = pin1->newy   ;
		    pin2->xpos = pin2->newx   ;
		    pin2->ypos = pin2->newy   ;

		    /* swap the cell relative pin positions */
		    tmp = pin1->txpos[0] ;
		    pin1->txpos[0] = pin2->txpos[0] ;
		    pin2->txpos[0] = tmp ;
		    tmp = pin1->txpos[1] ;
		    pin1->txpos[1] = pin2->txpos[1] ;
		    pin2->txpos[1] = tmp ;

		    tmp = pin1->typos[0] ;
		    pin1->typos[0] = pin2->typos[0] ;
		    pin2->typos[0] = tmp ;
		    tmp = pin1->typos[1] ;
		    pin1->typos[1] = pin2->typos[1] ;
		    pin2->typos[1] = tmp ;

		}
	    }
	}
    }
    if( cell1 != cell2 ) {
	sortpin2( cell1, cell2 ) ;
	/* must pass beginning of the list not pina and pinb */
	dbox_pos_swap( carrayG[cell1]->pins ) ;
	dbox_pos_swap( carrayG[cell2]->pins ) ;
	update_time2() ;
	/*  Now we have to put the paths on the correct cells
	    since the cells have swapped some pins  */
	adjust_paths_on_cell( cell1 ) ;
	adjust_paths_on_cell( cell2 ) ;

    } else {
	sortpin1( cell1 ) ;
	dbox_pos_swap( pina ) ;
	update_time( cell1 ) ;
    }
 
    funccostG = cost ; 
    timingcostG  = newtimepenal ;

    return( 1 ) ;
} else {
    return( 0 ) ;
}
}






adjust_paths_on_cell( cell )
INT cell ;
{

INT net_number ;
PSETPTR pathlist, enum_path_set() ;
CBOXPTR ptr ;
GLISTPTR  path_ptr, tempPath ;
DBOXPTR dimptr ;
PINBOXPTR pinptr ;

ptr = carrayG[cell] ;
/*  First, scrap all of the current path list  */
tempPath = ptr->paths ;
ptr->paths = NULL ;
while( tempPath != NULL ) {
    path_ptr = tempPath ;
    tempPath = tempPath->next ;
    Ysafe_free( (GLISTPTR) path_ptr ) ;
}

clear_path_set() ;
/* look for the UNIQUE paths that connects to this cell */
for(pinptr=ptr->pins;pinptr;pinptr=pinptr->nextpin){
    net_number = pinptr->net ;

    /* now go to net array */
    dimptr = netarrayG[net_number] ;

    /* look at all paths that use this net */
    for( path_ptr=dimptr->paths;path_ptr;path_ptr=path_ptr->next){
	add2path_set( path_ptr->p.path ) ;
    }
}
/* now add UNIQUE list of paths to this cell */
for( pathlist=enum_path_set(); pathlist; pathlist=pathlist->next){
    if( tempPath = ptr->paths ){
	path_ptr = ptr->paths = 
	    (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
	path_ptr->next = tempPath ;
    } else {  /* start a new list */
	path_ptr = ptr->paths = 
	    (GLISTPTR) Ysafe_malloc( sizeof(GLISTBOX) ) ;
	path_ptr->next = NULL ;
    }
    /* copy path to cell list of paths */
    path_ptr->p.path = pathlist->path ;
}
return ;
}
