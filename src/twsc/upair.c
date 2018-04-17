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
FILE:	    upair.c                                       
DESCRIPTION:pairwise flips of cells.
CONTENTS:   upair()
	    eval_range( acellptr , bcellptr, axc , anxc , bxc , bnxc )
		CBOXPTR acellptr , bcellptr ;
		INT axc , anxc , bxc , bnxc ;
DATE:	    Mar 27, 1989 
REVISIONS:  Fri Mar 22 16:23:46 CST 1991 - now avoid upair
		if there are no moveable cells.
	    Fri Sep  6 15:20:48 CDT 1991 - now place pads during
		pairwise swaps.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) upair.c (Yale) version 4.12 5/12/92" ;
#endif
#endif

#include "standard.h"
#include "main.h"
#include <yalecad/debug.h>
#include <yalecad/message.h>
#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

/* global references */
extern INT **pairArrayG ;
extern BOOL orientation_optimizationG ;
extern BOOL placement_improveG ;

upair()
{

CBOXPTR acellptr, bcellptr ; 
BBOXPTR ablckptr ;
INT a , b , ablock , aorient ;
INT flips , attempts , oattempts ;
INT axcenter,anxcenter, bnxcenter ;
INT aleft , aright ; 
INT startx1, endx1;
INT cellleft, cellrite;
INT leftEdge, riteEdge;
INT aptr , one_cell_per_row , row ;
INT free_cells, cells_in_row ;

if(!(placement_improveG)){
    return ;
}
flips = 0 ;
attempts = 0 ;
oattempts = 0 ;

/* ************** place the pads ****************** */
setVirtualCore( TRUE ) ;
placepads() ;
funccostG = recompute_wirecost() ;
timingcostG = recompute_timecost() ;

/* assume conditions are true - prove otherwise */
one_cell_per_row = TRUE ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    cells_in_row = pairArrayG[row][0] ;
    if( cells_in_row > 1 ) {
	/* at least one row has more than one cell */
	/* check to see if we have moveable cells in row */
	free_cells = 0 ;
	for( a = 1; a <= cells_in_row; a++ ){
	    aptr = pairArrayG[row][a] ;
	    acellptr = carrayG[ aptr ]  ;
	    if( acellptr->cclass >= 0 ) {
		free_cells++ ;
	    }
	}
	if( free_cells >= 3 ){
	    /* need at least three cells to do meaningful work. */
	    /* otherwise we will spend more time looking for the cells */
	    one_cell_per_row = FALSE ;
	    break ;
	}
    }
}
if( one_cell_per_row ) {
    return ;
}

while( attempts < attmaxG && oattempts < 2 * attmaxG ) {
    ablock = PICK_INT( 1 , numRowsG ) ;
    if( pairArrayG[ablock][0] <= 1 ) {
	continue ;
    }
    aptr = PICK_INT( 1 , pairArrayG[ablock][0] ) ;

    a = pairArrayG[ablock][aptr] ;
    acellptr = carrayG[ a ]  ;
    if( acellptr->cclass < 0 ) {
	continue ;
    }
    aorient = acellptr->corient ;

    ablckptr = barrayG[ ablock ] ;
    axcenter = acellptr->cxcenter ;

    aleft = acellptr->tileptr->left ;
    aright = acellptr->tileptr->right ;
    startx1 = axcenter + aleft    ; 
    endx1   = axcenter + aright   ; 

    if( orientation_optimizationG ) {
	goto ort ;
    }

    if( aptr > 1 ) {
	cellleft = pairArrayG[ablock][aptr - 1] ;
	if( carrayG[cellleft]->cclass < 0 ) {
	    cellleft = 0 ;
	}
    } else {
	cellleft = 0 ;
    }
    if( aptr < pairArrayG[ablock][0] ) {
	cellrite = pairArrayG[ablock][aptr + 1] ;
	if( carrayG[cellrite]->cclass < 0 ) {
	    cellrite = 0 ;
	}
    } else {
	cellrite = 0 ;
    }

    /*
    if( intel || fences_exist ) {
	if( cellleft != 0 ) {
	    bcellptr = carrayG[cellleft] ;
	    leftEdge = bcellptr->cxcenter + bcellptr->tileptr->left ;
	    anxcenter = leftEdge - aleft ;
	    bnxcenter = endx1 - bcellptr->tileptr->right ;
	    if( eval_range( acellptr, bcellptr, axcenter, anxcenter,
				bcellptr->cxcenter, bnxcenter ) == 0 ) {
		cellleft = 0 ;
	    }
	}
	if( cellrite != 0 ) {
	    bcellptr = carrayG[cellrite] ;
	    riteEdge = bcellptr->cxcenter + bcellptr->tileptr->right ;
	    anxcenter = riteEdge - aright ;
	    bnxcenter = startx1 - bcellptr->tileptr->left ;
	    if( eval_range( acellptr, bcellptr, axcenter, anxcenter,
				bcellptr->cxcenter, bnxcenter ) == 0 ) {
		cellrite = 0 ;
	    }
	}
    }
    */

    if( cellleft == 0 && cellrite == 0 ) {
	continue ;
    }

    if( cellleft != 0 && cellrite != 0 ) {
	if( PICK_INT(1 , 2) == 1 ){
	    /*
	     *   Take the left neighbor first.
	     */
	    b = cellleft ;
	    bcellptr = carrayG[b] ;
	    leftEdge = bcellptr->cxcenter + bcellptr->tileptr->left ;
	    anxcenter = leftEdge - aleft ;
	    bnxcenter = endx1 - bcellptr->tileptr->right ;
	    if( ucxxp( a, b, anxcenter, bnxcenter ) ) {
		flips++ ;
		attempts++ ;
		pairArrayG[ablock][aptr] = b ;
		pairArrayG[ablock][aptr - 1] = a ;
	    } else {
		attempts++ ;
		b = cellrite ;
		bcellptr = carrayG[b] ;
		riteEdge = bcellptr->cxcenter 
				+ bcellptr->tileptr->right ;
		anxcenter = riteEdge - aright ;
		bnxcenter = startx1 - bcellptr->tileptr->left ;
		if( ucxxp( a, b, anxcenter, bnxcenter ) ) {
		    flips++ ;
		    pairArrayG[ablock][aptr] = b ;
		    pairArrayG[ablock][aptr + 1] = a ;
		} 
		attempts++ ;
	    }
	} else {
	    b = cellrite ;
	    bcellptr = carrayG[b] ;
	    riteEdge = bcellptr->cxcenter + bcellptr->tileptr->right ;
	    anxcenter = riteEdge - aright ;
	    bnxcenter = startx1 - bcellptr->tileptr->left ;
	    if( ucxxp( a, b, anxcenter, bnxcenter ) ) {
		flips++ ;
		attempts++ ;
		pairArrayG[ablock][aptr] = b ;
		pairArrayG[ablock][aptr + 1] = a ;
	    } else {
		attempts++ ;
		b = cellleft ;
		bcellptr = carrayG[b] ;
		leftEdge = bcellptr->cxcenter + bcellptr->tileptr->left;
		anxcenter = leftEdge - aleft ;
		bnxcenter = endx1 - bcellptr->tileptr->right ;
		if( ucxxp( a, b, anxcenter, bnxcenter ) ) {
		    flips++ ;
		    pairArrayG[ablock][aptr] = b ;
		    pairArrayG[ablock][aptr - 1] = a ;
		} 
		attempts++ ;
	    }
	}
    } else {
	if( cellleft ) {
	    b = cellleft ;
	    bcellptr = carrayG[b] ;
	    leftEdge = bcellptr->cxcenter + bcellptr->tileptr->left ;
	    anxcenter = leftEdge - aleft ;
	    bnxcenter = endx1 - bcellptr->tileptr->right ;
	    if( ucxxp( a, b, anxcenter, bnxcenter ) ) {
		flips++ ;
		pairArrayG[ablock][aptr] = b ;
		pairArrayG[ablock][aptr - 1] = a ;
	    } 
	    attempts++ ;
	} else if( cellrite != 0 && 
		    carrayG[cellrite]->cclass >= 0 &&
		    acellptr->cclass >= 0 ) {
	    b = cellrite ;
	    bcellptr = carrayG[b] ;
	    riteEdge = bcellptr->cxcenter + bcellptr->tileptr->right ;
	    anxcenter = riteEdge - aright ;
	    bnxcenter = startx1 - bcellptr->tileptr->left ;
	    if( ucxxp( a, b, anxcenter, bnxcenter ) ) {
		flips++ ;
		pairArrayG[ablock][aptr] = b ;
		pairArrayG[ablock][aptr + 1] = a ;
	    } 
	    attempts++ ;
	}
    }
ort:if( ablckptr->borient == 1 ) {
	if( acellptr->orflag != 0 ) {
	    uc0( a , (aorient == 0) ? 2 : 0 ) ;
	    oattempts++ ;
	}
    } else {
	if( acellptr->orflag != 0 ) {
	    uc0( a , (aorient == 1) ? 3 : 1 ) ;
	    oattempts++ ;
	}
    }
    D( "upair",
	check_cost() ;
    ) ;
}
sprintf( YmsgG, " %3d %6.3f %9d  %3d%s  %-8ld\n", iterationG+1, TG, funccostG,
	(INT)( 100.0 * (DOUBLE)(flips) / (DOUBLE)(attmaxG) ) , "%" ,
	timingcostG ) ;
M( MSG, NULL, YmsgG ) ;
return;
}


eval_range( acellptr , bcellptr, axc , anxc , bxc , bnxc )
CBOXPTR acellptr , bcellptr ;
INT axc , anxc , bxc , bnxc ;
{

FENCEBOXPTR fence ;
INT a_current_penalty , a_new_penalty ;
INT b_current_penalty , b_new_penalty ;


if( (fence = acellptr->fence) == NULL ) {
    a_current_penalty = 0 ;
} else {
    a_current_penalty = 10000000 ;
}
for( fence = acellptr->fence ; fence && a_current_penalty != 0 ; 
					fence = fence->next_fence ) {
    if( axc < fence->min_xpos ) {
	if( fence->min_xpos - axc < a_current_penalty ) {
	    a_current_penalty = fence->min_xpos - axc ;
	}
    } else if( axc > fence->max_xpos ) {
	if( axc - fence->max_xpos < a_current_penalty ) {
	    a_current_penalty = axc - fence->max_xpos ;
	}
    } else {
	a_current_penalty = 0 ;
    }
}
if( (fence = bcellptr->fence) == NULL ) {
    b_current_penalty = 0 ;
} else {
    b_current_penalty = 10000000 ;
}
for( ; fence && b_current_penalty != 0 ; fence = fence->next_fence ) {
    if( bxc < fence->min_xpos ) {
	if( fence->min_xpos - bxc < b_current_penalty ) {
	    b_current_penalty = fence->min_xpos - bxc ;
	}
    } else if( bxc > fence->max_xpos ) {
	if( bxc - fence->max_xpos < b_current_penalty ) {
	    b_current_penalty = bxc - fence->max_xpos ;
	}
    } else {
	b_current_penalty = 0 ;
    }
}

if( (fence = acellptr->fence) == NULL ) {
    a_new_penalty = 0 ;
} else {
    a_new_penalty = 10000000 ;
}
for( ; fence && a_new_penalty != 0 ; fence = fence->next_fence ) {
    if( anxc < fence->min_xpos ) {
	if( fence->min_xpos - anxc < a_new_penalty ) {
	    a_new_penalty = fence->min_xpos - anxc ;
	}
    } else if( anxc > fence->max_xpos ) {
	if( anxc - fence->max_xpos < a_new_penalty ) {
	    a_new_penalty = anxc - fence->max_xpos ;
	}
    } else {
	a_new_penalty = 0 ;
    }
}
if( (fence = bcellptr->fence) == NULL ) {
    b_new_penalty = 0 ;
} else {
    b_new_penalty = 10000000 ;
}
for( ; fence && b_new_penalty != 0 ; fence = fence->next_fence ) {
    if( bnxc < fence->min_xpos ) {
	if( fence->min_xpos - bnxc < b_new_penalty ) {
	    b_new_penalty = fence->min_xpos - bnxc ;
	}
    } else if( bnxc > fence->max_xpos ) {
	if( bnxc - fence->max_xpos < b_new_penalty ) {
	    b_new_penalty = bnxc - fence->max_xpos ;
	}
    } else {
	b_new_penalty = 0 ;
    }
}

if( a_new_penalty + b_new_penalty <= a_current_penalty +
					    b_current_penalty ) {
    return(1) ;
} else {
    return(0) ;
}

}
