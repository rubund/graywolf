/*
 *   Copyright (C) 1988-1991 Yale University
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
FILE:	    sortpad.c
DESCRIPTION:This file contains the utility routines to sort pads
	    for the new pad routines.
CONTENTS:   
DATE:	    Aug 12, 1988 
REVISIONS:  Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
	    Sat Feb 23 00:17:28 EST 1991 - added placepads algorithm.
	    Tue Mar 12 17:05:03 CST 1991 - fixed initialization problem
		with permutation.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) sortpad.c version 3.6 3/12/91" ;
#endif

#include <custom.h>
#include <pads.h>
#include <yalecad/debug.h>



static INT compare_pads();
static INT sort_by_pos();
static install_pad_groups();
static permute_pads();



sort_pads()
{
    INT i ;                /* pad counter */
    INT pos ;              /* position in place array */
    INT compare_pads() ;   /* how to sort the pads initially */
    INT sort_by_pos() ;    /* how to sort the pads */
    PADBOXPTR pad ;        /* current pad */

    /* first perform an initial sort to order the pads by side, hierarchy, */
    /* and position on the side. */
    Yquicksort( &(sortarrayG[1]), totalpadsG, sizeof(PADBOXPTR), compare_pads );
    D( "placepads/after_sort", 
	print_pads("pads after initial sort\n",sortarrayG, totalpadsG ) ;
    ) ;

    /* Now make sure the pads are permuted correctly */
    for( i = 1; i <= totalpadsG; i++ ){
	pad = sortarrayG[i];
	if( pad->hierarchy == ROOT ){
	    permute_pads( pad ) ;
	}
    }

    /* NOW INSTALL THE PAD GROUPS IN THEIR PROPER ORDER. There are 2 cases: */
    /* CASE I CONTIGUOUS INSURE THAT GROUP REMAIN INTACT */
    pos = 1 ;
    for( i = 1; i <= totalpadsG; i++ ){
	pad = sortarrayG[i];
	if( pad->hierarchy == ROOT || pad->hierarchy == NONE ){
	    install_pad_groups( pad, &pos ) ;
	}
    }
    D( "placepads/after_install",
	print_pads("pads after install\n",placearrayG,numpadsG);
    ) ;

    if(!(contiguousG)){
	/* CASE II -  LEAVES ARE ALIGNED LIKE ORDINARY PADS IF THEY HAVE NO */
	/* CONSTRAINTS SUCH AS ORDER OR PERMUTE.  **/
	Yquicksort( &(placearrayG[1]), numpadsG, sizeof(PADBOXPTR), sort_by_pos ) ;
	D( "placepads/after_ncontsort",
	    print_pads("pads after noncontiguous sort\n",placearrayG,numpadsG);
	) ;
    }

} /* end SortPads */
/* ***************************************************************** */

/*** compare_pads() RETURNS TRUE IF ARG1 > ARG2 BY ITS OWN RULES **/
static INT compare_pads( padptr1, padptr2 )
PADBOXPTR *padptr1, *padptr2 ;
{
    PADBOXPTR pad1, pad2;

    pad1 = *padptr1 ;
    pad2 = *padptr2 ;

    if( pad1->padside != pad2->padside) {
	return( pad1->padside - pad2->padside ) ;
    }
    if( pad1->hierarchy != pad2->hierarchy ){
	/*** MOVE ROOTS TO THE BEGINNING OF ARRAY MOVE */
	/* LEAVES ARE SEPARATED, ROOTS ARE MERGED **/
	if( pad1->hierarchy == SUBROOT ){
	    return( 1 ) ;
	} else if( pad2->hierarchy == SUBROOT ){
	    return( 0 ) ;
	} else if( pad1->hierarchy == LEAF ){
	    return( 1 ) ;
	} else if( pad2->hierarchy == LEAF ){
	    return( 0 ) ;
	}
    }

    if( pad1->position == pad2->position ){
	return( pad1->tiebreak - pad2->tiebreak ) ;
    } else {
	return( pad1->position - pad2->position ) ;
    }

} /* end compare_pads */
/* ***************************************************************** */

static INT sort_by_pos( padptr1, padptr2 )
PADBOXPTR *padptr1, *padptr2 ;
{
    PADBOXPTR pad1, pad2;
    BOOL pad1fixed, pad2fixed ;

    pad1 = *padptr1 ;
    pad2 = *padptr2 ;

    if( pad1->padside != pad2->padside) {
	return( pad1->padside - pad2->padside ) ;
    }
    if( pad1->ordered || pad1->permute ){
	pad1fixed = TRUE ;
    } else {
	pad1fixed = FALSE ;
    }
    if( pad2->ordered || pad2->permute ){
	pad2fixed = TRUE ;
    } else {
	pad2fixed = FALSE ;
    }
    if( pad1fixed && pad2fixed && !(contiguousG) ){
	return( 0 ) ;
    } else if( pad1fixed && contiguousG ){
	return( 0 ) ;
    } else if( pad2fixed && contiguousG ){
	return( 1 ) ;
    } else if( pad1->position == pad2->position ){
	return( pad1->tiebreak - pad2->tiebreak ) ;
    } else {
	return( pad1->position - pad2->position ) ;
    }

} /* end sort_by_pos */
/* ***************************************************************** */

static install_pad_groups( pad, position )
PADBOXPTR pad ;
INT *position ;
{
    INT i ;                      /* pad counter */
    INT howmany ;                /* number of pads in group */
    INT initial_position ;       /* position of next open place in placearray */
    INT sort_by_pos() ;          /* how to sort the pads */
    PADBOXPTR child ;            /* current child */
    PADBOXPTR *temparray ;       /* temporary array to sort pads */

    initial_position = *position ;
    if( pad->padtype == PADCELLTYPE ){
	placearrayG[initial_position] = pad ;
	*position = ++initial_position ;
    } else {
	howmany = pad->children[HOWMANY] ;
	temparray = (PADBOXPTR *) Yvector_alloc( 1,howmany,sizeof(PADBOXPTR) ) ;
	for( i = 1 ;i <= howmany ; i++ ){
	    child = padarrayG[ pad->children[i] ] ;
	    temparray[i] = child ;
	}
	/* now sort the subroots or leaves to obey both order constraints */
	/* and permutation constraints.  Otherwise try to sort by opt. pos.*/
	Yquicksort( &(temparray[1]), howmany, sizeof(PADBOXPTR), sort_by_pos ) ;

	/* now that we have subroots or leaves in correct order */
	/* look at next level down */
	for( i = 1 ;i <= howmany ; i++ ){
	    child = temparray[ i ] ;
	    install_pad_groups( child, position ) ;
	}
	Yvector_free( temparray, 1, sizeof(PADBOXPTR) ) ;
    }
} /* end install_pad_groups */
/* ***************************************************************** */

static permute_pads( pad )
PADBOXPTR pad ;
{
    INT tmp ;                 /* used to reverse permutable pads */
    INT j, k ;                /* used to reverse pads */
    INT i ;                   /* padcounter */
    INT howmany ;             /* number of children in current padgroup */
    INT max_pos ;             /* max. value of the ideal positions of pad in pg */
    INT min_pos ;             /* min. value of the ideal positions of pad in pg */
    INT forward_cost ;        /* cost to place pads in current order */
    INT bakward_cost ;        /* cost to place pads in reverse order */
    INT proposed_fpos ;       /* proposed uniformly spaced pos in forward order */
    INT proposed_bpos ;       /* proposed uniformly spaced pos in bakward order */
    INT *array ;              /* sort the children */
    DOUBLE spacing ;          /* spacing if we place pads in pg uniformly */
    PADBOXPTR child ;         /* current child */

    if( pad->permute ){
	/* first calculate span of padgroup */
	howmany = pad->children[HOWMANY] ;
	ASSERTNRETURN( howmany >= 2,"permute_pads",
	    "Must have at least 2 pads in a padgroup\n");
	child = padarrayG[pad->children[1]];
	min_pos = child->position ;
	max_pos = child->position ;
	for( i = 2; i <= howmany ; i++ ){
	    child = padarrayG[pad->children[i]];
	    min_pos = MIN( child->position, min_pos ) ;
	    max_pos = MAX( child->position, max_pos ) ;
	}
	/* now find the cost if we evenly space the pads over that region */
	spacing = (DOUBLE) (max_pos - min_pos) / (DOUBLE) (howmany - 1) ;
	forward_cost = 0 ;
	bakward_cost = 0 ;
	for( i = 1; i <= howmany ; i++ ){
	    child = padarrayG[pad->children[i]];
	    proposed_fpos = min_pos + ROUND( (i - 1) * spacing ) ;
	    proposed_bpos = max_pos - ROUND( (i - 1) * spacing ) ;
	    forward_cost += ABS( child->position - proposed_fpos ) ;
	    bakward_cost += ABS( child->position - proposed_bpos ) ;
	}

	if( bakward_cost < forward_cost ) {
	    /* we need to reverse the permutation */
	    array = pad->children + 1;
	    j = howmany - 1;
	    k = 0;
	    while( k < j ){
		tmp        = array[j];
		array[j--] = array[k];
		array[k++] = tmp;
	    }
	}
   }
   /*** NEED TO CHECK THE CHILDREN REGARDLESS OF THE PERMUTABILITY OF
	THE PARENT ROOT */
   for( i = 1; i <= pad->children[HOWMANY]; i++ ){
	child = padarrayG[pad->children[i]];
	if( child->hierarchy == SUBROOT){
	    permute_pads( child ) ;
	}
    }

} /* end permute_pads */
/* ***************************************************************** */
