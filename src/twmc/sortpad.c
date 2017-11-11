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
#include "allheaders.h"

int sort_by_pos_pad( PADBOXPTR *padptr1, PADBOXPTR *padptr2 );

void sort_pads()
{
	int i ;                /* pad counter */
	int pos ;              /* position in place array */
	int compare_pads() ;   /* how to sort the pads initially */
	PADBOXPTR pad ;        /* current pad */

	/* first perform an initial sort to order the pads by side, hierarchy, */
	/* and position on the side. */
	Yquicksort( &(sortarrayG[1]), totalpadsG, sizeof(PADBOXPTR), compare_pads );
	D( "placepads/after_sort", print_pads("pads after initial sort\n",sortarrayG, totalpadsG ) ; ) ;

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
		Yquicksort( &(placearrayG[1]), numpadsG, sizeof(PADBOXPTR), sort_by_pos_pad ) ;
		D( "placepads/after_ncontsort",
		print_pads("pads after noncontiguous sort\n",placearrayG,numpadsG);
		) ;
	}

} /* end SortPads */
/* ***************************************************************** */

/*** compare_pads() RETURNS TRUE IF ARG1 > ARG2 BY ITS OWN RULES **/
int compare_pads( PADBOXPTR *padptr1, PADBOXPTR *padptr2 )
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

int sort_by_pos_pad( PADBOXPTR *padptr1, PADBOXPTR *padptr2 )
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

} /* end sort_by_pos_pad */
/* ***************************************************************** */

void install_pad_groups( PADBOXPTR pad, int *position )
{
    int i ;                      /* pad counter */
    int howmany ;                /* number of pads in group */
    int initial_position ;       /* position of next open place in placearray */
    int sort_by_pos_pad() ;          /* how to sort the pads */
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
	Yquicksort( &(temparray[1]), howmany, sizeof(PADBOXPTR), sort_by_pos_pad ) ;

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

void permute_pads( PADBOXPTR pad )
{
    int tmp ;                 /* used to reverse permutable pads */
    int j, k ;                /* used to reverse pads */
    int i ;                   /* padcounter */
    int howmany ;             /* number of children in current padgroup */
    int max_pos ;             /* max. value of the ideal positions of pad in pg */
    int min_pos ;             /* min. value of the ideal positions of pad in pg */
    int forward_cost ;        /* cost to place pads in current order */
    int bakward_cost ;        /* cost to place pads in reverse order */
    int proposed_fpos ;       /* proposed uniformly spaced pos in forward order */
    int proposed_bpos ;       /* proposed uniformly spaced pos in bakward order */
    int *array ;              /* sort the children */
    double spacing ;          /* spacing if we place pads in pg uniformly */
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
	spacing = (double) (max_pos - min_pos) / (double) (howmany - 1) ;
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
