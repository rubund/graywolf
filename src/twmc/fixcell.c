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
FILE:	    fixcell.c                                       
DESCRIPTION:places fixed cells in the correct position during a 
	    configuration.  
CONTENTS:   updateFixedCells( initializeFlag )
		BOOL initializeFlag ;
	    update_fixed_record( fptr ) 
		FIXEDBOXPTR fptr ;
	    build_active_array()
	    build_soft_array()
	    determine_origin( x, y, left_not_right, bottom_not_top )
		INT *x, *y ;
		BOOL *left_non_right, *bottom_not_top ;
DATE:	    Sept 28, 1988 
REVISIONS:  Oct  27, 1988 - modified update fixed cells so that it
		could handle the following cases: fixed at a point,
		cell fixed in a neighborhood, group neighborhood
		fixed at a point, group neighborhood free.
		Added build_active_array which holds all active core
		cell including supercells and groupcells to make uloop
		more efficient.
	    Jan 20, 1989 - added build_soft_array() to make soft pin
		moves more efficient.
	    Feb 26, 1989 - added determine_origin for graphics interface.
	    Mar 13, 1989 - split updateFixedCells to update_fixed_record
		to anticipate group cell moves.
	    Apr 27, 1989 - added stdcell to list of active cells.
	    May 10, 1989 - now use the HOWMANYORIENT field for valid
		orientation calculation.
	    Apr 17, 1990 - added error checking.
	    May  4, 1990 - fixed for standard cell case.
	    Mon Feb  4 02:06:08 EST 1991 - updated determine origin
		since arguments changed from BOOL to char *.
	    Sun May  5 14:25:12 EDT 1991 - added prototypes. Fixed
		missing argument to update_fix_record.  Fixed error
		checking.
	    Wed Jul 24 20:43:22 CDT 1991 - added delete function 
		for fixing cells.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) fixcell.c version 3.9 11/23/91" ;
#endif

#include <string.h>
#include <custom.h>
#include <yalecad/base.h>
#include <yalecad/debug.h>


/* defined routes for error checking */
VOID updateFixedCells( P1(BOOL initializeFlag) ) ;
static VOID update_fixed_record( P3(CELLBOXPTR ptr,FIXEDBOXPTR fptr,
				BOOL initFlag ) ) ;
VOID init_fixcell( P4(INT left, INT bottom, INT right, INT top ) ) ;
VOID build_active_array() ;
VOID build_soft_array() ;
VOID determine_origin( P4(INT *x, INT *y, 
    char *left_not_right, char *bottom_not_top ) ) ;

static INT initialS = FALSE ;
static INT newxspanS ;
static INT newyspanS ;
static INT oldxspanS ;
static INT oldyspanS ;

VOID updateFixedCells( initializeFlag )
BOOL initializeFlag ;
{

    INT i ;
    CELLBOXPTR ptr ;
    FIXEDBOXPTR fptr ;

    /* calculate new span of the core region */
    newxspanS = blockrG - blocklG ;
    newyspanS = blocktG - blockbG ;

    /* update positions of fixed cells */
    for( i=1;i<=endsuperG;i++ ){
	ptr = cellarrayG[i] ;
	/* for core cells check fixed record */
	/* for group cells check group record */
	if( ptr->celltype == GROUPCELLTYPE ){
	    ASSERTNCONT( ptr->group, "updateFixedCells", "pointer null");
	    fptr = ptr->group->fixed ;
	} else {
	    fptr = ptr->fixed ;
	}
	update_fixed_record( ptr, fptr, initializeFlag ) ;

	if( initializeFlag && ptr->celltype == GROUPCELLTYPE ){
	    /* now we can add tiles to group */
	    initializeCorner( i ) ;
	    addCorner( fptr->x1, fptr->y1 ) ; /* lower left */
	    addCorner( fptr->x1, fptr->y2 ) ; /* upper left */
	    addCorner( fptr->x2, fptr->y2 ) ; /* upper right */
	    addCorner( fptr->x2, fptr->y1 ) ; /* lower right */
	    processCorners( 4 ) ;
	}
    }
} /* end updateFixCells */

/* update all the variables in the fixed box record - that is the */
/* the neighborhood and/or the center of the cell for a fixed point */
static VOID update_fixed_record( ptr, fptr, initFlag ) 
CELLBOXPTR ptr ;
FIXEDBOXPTR fptr ;
BOOL initFlag ;
{
    INT x1, x2, y1, y2 ;
    BOOL error_flag = FALSE ;
    DOUBLE temp ;

    if( fptr ){
	D( "update_fixed_record", 
	    fprintf( stderr, "cell:%s\n", ptr->cname ) ;
	    fprintf( stderr, "\tinitialS:%d\n", initialS ) ;
	    fprintf( stderr, "\toldxspanS:%d ", oldxspanS ) ;
	    fprintf( stderr, "\toldyspanS:%d\n", oldyspanS ) ;
	    fprintf( stderr, "\tnewxspanS:%d ",  newxspanS ) ;
	    fprintf( stderr, "\tnewyspanS:%d\n", newyspanS ) ;
	    fprintf( stderr, "\txc:%d yc:%d x1:%d y1:%d x2:%d y2:%d\n", 
		fptr->xcenter, fptr->ycenter,
		fptr->xloc1,fptr->yloc1,fptr->xloc2,fptr->yloc2 ) ;
	    fprintf( stderr, "\tx1:%d y1:%d x2:%d y2:%d\n", 
		fptr->leftNotRight,fptr->bottomNotTop,
		fptr->leftNotRight2,fptr->bottomNotTop2 ) ;
	) ; /* end debug macro */

	if( fptr->fixedType != POINTFLAG ){
	    if( initialS ){
		/* get two points of neighborhood */
		/* POINT ONE */
		temp = (DOUBLE) fptr->xloc1 / oldxspanS * newxspanS ;
		if( fptr->leftNotRight ){
		    /* from left edge  */
		    x1 = blocklG + (INT) temp ;
		} else {
		    /* from right edge = blockx */
		    x1 = blockrG - (INT) temp ;
		}
		temp = (DOUBLE) fptr->yloc1 / oldyspanS * newyspanS ;
		if( fptr->bottomNotTop ){
		    /* from bottom edge = 0 */
		    y1 = blockbG + (INT) temp ;
		} else {
		    /* from top edge = blocky */
		    y1 = blocktG - (INT) temp ;
		}
		/* POINT TWO */
		temp = (DOUBLE) fptr->xloc2 / oldxspanS * newxspanS ;
		if( fptr->leftNotRight2 ){
		    /* from left edge  */
		    x2 = blocklG + (INT) temp ;
		} else {
		    /* from right edge = blockx */
		    x2 = blockrG - (INT) temp ;
		}
		temp = (DOUBLE) fptr->yloc2 / oldyspanS * newyspanS ;
		if( fptr->bottomNotTop2 ){
		    /* from bottom edge = 0 */
		    y2 = blockbG + (INT) temp ;
		} else {
		    /* from top edge = blocky */
		    y2 = blocktG - (INT) temp ;
		}
	    } else {
		/* get two points of neighborhood */
		/* POINT ONE */
		if( fptr->leftNotRight ){
		    /* from left edge  */
		    x1 = blocklG + fptr->xloc1 ;
		} else {
		    /* from right edge = blockx */
		    x1 = blockrG - fptr->xloc1 ;
		}
		if( fptr->bottomNotTop ){
		    /* from bottom edge = 0 */
		    y1 = blockbG + fptr->yloc1 ;
		} else {
		    /* from top edge = blocky */
		    y1 = blocktG - fptr->yloc1 ;
		}
		/* POINT TWO */
		if( fptr->leftNotRight2 ){
		    /* from left edge  */
		    x2 = blocklG + fptr->xloc2 ;
		} else {
		    /* from right edge = blockx */
		    x2 = blockrG - fptr->xloc2 ;
		}
		if( fptr->bottomNotTop2 ){
		    /* from bottom edge = 0 */
		    y2 = blockbG + fptr->yloc2 ;
		} else {
		    /* from top edge = blocky */
		    y2 = blocktG - fptr->yloc2 ;
		}
	    }
	    /* sort points */
	    fptr->x1 = MIN( x1, x2 ) ;
	    fptr->x2 = MAX( x1, x2 ) ;
	    fptr->y1 = MIN( y1, y2 ) ;
	    fptr->y2 = MAX( y1, y2 ) ;
	    if(!(initFlag)){
		/* check to make sure neighborhood is inside core */
		if( fptr->x1 < blocklG ){
		    if( iterationG >= 2 ) fptr->x1 = blocklG ;
		    error_flag = TRUE ;
		}
		if( fptr->x1 > blockrG ){
		    if( iterationG >= 2 ) fptr->x1 = blockrG ;
		    error_flag = TRUE ;
		}
		if( fptr->x2 < blocklG ){
		    if( iterationG >= 2 ) fptr->x2 = blocklG ;
		    error_flag = TRUE ;
		}
		if( fptr->x2 > blockrG ){
		    if( iterationG >= 2 ) fptr->x2 = blockrG ;
		    error_flag = TRUE ;
		}
		if( fptr->y1 < blockbG ){
		    if( iterationG >= 2 ) fptr->y1 = blockbG ;
		    error_flag = TRUE ;
		}
		if( fptr->y1 > blocktG ){
		    if( iterationG >= 2 ) fptr->y1 = blocktG ;
		    error_flag = TRUE ;
		}
		if( fptr->y2 < blockbG ){
		    if( iterationG >= 2 ) fptr->y2 = blockbG ;
		    error_flag = TRUE ;
		}
		if( fptr->y2 > blocktG ){
		    if( iterationG >= 2 ) fptr->y2 = blocktG ;
		    error_flag = TRUE ;
		}
	    }

	    /* now perform calculations for speed */
	    fptr->xcenter = (fptr->x1 + fptr->x2) / 2 ;
	    fptr->ycenter = (fptr->y1 + fptr->y2) / 2 ;
	    fptr->xspan = ABS(fptr->x1 - fptr->x2) ;
	    fptr->yspan = ABS(fptr->y1 - fptr->y2) ;

	    if( error_flag ){
		sprintf( YmsgG, "cell:%s neighborhood larger than core\n",
		    ptr->cname ) ;
		    
		M( WARNMSG,"update_fixed_cells", YmsgG ) ;

		if( iterationG >= 2 ){
		    /* we can't do anything with this constraint */
		    /* throw it out */
		    if( ptr->celltype == GROUPCELLTYPE ){
			fptr = ptr->group->fixed ;
			Ysafe_free( fptr ) ;
			ptr->group->fixed = NIL(FIXEDBOXPTR) ;
		    } else {
			fptr = ptr->fixed ;
			Ysafe_free( fptr ) ;
			ptr->fixed = NIL(FIXEDBOXPTR) ;
		    }
		    M( WARNMSG,NULL,"\tconstraint ignored...\n" ) ;
		}
	    }

	} else {  /* cell is fixed at a point */
	    if( initialS ){
		temp = (DOUBLE) fptr->xcenter / oldxspanS * newxspanS ;
		if( fptr->leftNotRight ){
		    /* from left edge  */
		    ptr->xcenter = blocklG + (INT) temp ;
		} else {
		    /* from right edge = blockx */
		    ptr->xcenter = blockrG - (INT) temp ;
		}
		if( fptr->bottomNotTop ){
		    /* from bottom edge = 0 */
		    ptr->ycenter = blockbG + fptr->ycenter ;
		} else {
		    /* from top edge = blocky */
		    ptr->ycenter = blocktG - fptr->ycenter ;
		}
	    } else {
		if( fptr->leftNotRight ){
		    /* from left edge  */
		    ptr->xcenter = blocklG + fptr->xcenter ;
		} else {
		    /* from right edge = blockx */
		    ptr->xcenter = blockrG - fptr->xcenter ;
		}
		if( fptr->bottomNotTop ){
		    /* from bottom edge = 0 */
		    ptr->ycenter = blockbG + fptr->ycenter ;
		} else {
		    /* from top edge = blocky */
		    ptr->ycenter = blocktG - fptr->ycenter ;
		}
	    }
	    if(!(initFlag)){
		/* now perform error checking to make sure inside core */
		if( ptr->xcenter < blocklG ){
		    if( iterationG >= 2 ) ptr->xcenter = blocklG ;
		    error_flag = TRUE ;
		}
		if( ptr->xcenter > blockrG ){
		    if( iterationG >= 2 ) ptr->xcenter = blockrG ;
		    error_flag = TRUE ;
		}
		if( ptr->ycenter < blockbG ){
		    if( iterationG >= 2 ) ptr->ycenter = blockbG ;
		    error_flag = TRUE ;
		}
		if( ptr->ycenter > blockrG ){
		    if( iterationG >= 2 ) ptr->ycenter = blockrG ;
		    error_flag = TRUE ;
		}
	    }
	    if( error_flag ){
		sprintf( YmsgG, "cell:%s fix point larger than core\n",
		    ptr->cname ) ;
		    
		M( WARNMSG,"update_fixed_cells", YmsgG ) ;

		if( iterationG >= 2 ){
		    /* we can't do anything with this constraint */
		    /* throw it out */
		    if( ptr->celltype == GROUPCELLTYPE ){
			fptr = ptr->group->fixed ;
			Ysafe_free( fptr ) ;
			ptr->group->fixed = NIL(FIXEDBOXPTR) ;
		    } else {
			fptr = ptr->fixed ;
			Ysafe_free( fptr ) ;
			ptr->fixed = NIL(FIXEDBOXPTR) ;
		    }
		    M( WARNMSG,NULL,"\tconstraint ignored...\n" ) ;
		}
	    }
	}
    } /* end for loop */
} /* end function update_fixed_record */

VOID init_fixcell( left, bottom, right, top )
INT left, bottom, right, top ;
{
    initialS = TRUE ; 
    oldxspanS = right - left ;
    oldyspanS = top - bottom ;
    if( oldxspanS <= 0 ){
	M( ERRMSG, "init_fixcell", "initial x core span <= 0\n" ) ;
	initialS = FALSE ; 
    }
    if( oldyspanS <= 0 ){
	M( ERRMSG, "init_fixcell", "initial y core span <= 0\n" ) ;
	initialS = FALSE ; 
    }
} /* end init_fixcell */

/* build active cell list from cells that aren't fixed */
VOID
build_active_array()
{
    INT i, cell ;
    CELLBOXPTR cptr ;

    /* first determine number of active cells */
    activecellsG = 0 ;
    for( i=1;i<=totalcellsG;i++ ){
	cptr = cellarrayG[i] ;
	if( cptr->fixed ){
	    if( cptr->celltype == CUSTOMCELLTYPE ||
		cptr->celltype == SOFTCELLTYPE ||
		cptr->celltype == STDCELLTYPE ){
		/* must check to orientations are all fixed also */
		if( cptr->orientList[HOWMANYORIENT] >= 2 ){
		    /* cell is not totally inactive it may rotate */
		    activecellsG++ ;
		}
	    }

	} else if( cptr->celltype == CUSTOMCELLTYPE ||
	    cptr->celltype == SOFTCELLTYPE ||
	    cptr->celltype == SUPERCELLTYPE ||
	    cptr->celltype == STDCELLTYPE ||
	    cptr->celltype == GROUPCELLTYPE ){
	    activecellsG++ ;
	}
    }
    /* now build array */
    activeCellarrayG = (CELLBOXPTR *)
	Ysafe_malloc( (activecellsG+1) * sizeof(CELLBOXPTR) ) ;
    /* now loop again filling in array */
    cell = 0 ;
    for( i=1;i<=totalcellsG;i++ ){
	cptr = cellarrayG[i] ;
	if( cptr->fixed ){
	    if( cptr->celltype == CUSTOMCELLTYPE ||
		cptr->celltype == SOFTCELLTYPE ||
		cptr->celltype == STDCELLTYPE ){
		if( cptr->orientList[HOWMANYORIENT] >= 2 ){
		    /* cell is not totally inactive it may rotate */
		    activeCellarrayG[++cell] = cptr ;
		}
	    }

	} else if( cptr->celltype == CUSTOMCELLTYPE ||
	    cptr->celltype == SOFTCELLTYPE ||
	    cptr->celltype == SUPERCELLTYPE ||
	    cptr->celltype == STDCELLTYPE ||
	    cptr->celltype == GROUPCELLTYPE ){
	    activeCellarrayG[++cell] = cptr ;
	}
    }
    ASSERT( cell == activecellsG,"build_active_array",
	"Problem with count" ) ;
    
} /* end build_active_array */

/* build softPinArrayG so that soft pin moves are more efficient */
/* softPinArray will contain the cellptrs of all softcells which */
/* have pins that can move.  The zeroth element will contain howmany */
VOID
build_soft_array()
{

#define HOWMANY 0  /* this tells the size of the array */

    INT i, cell ;
    CELLBOXPTR cptr ;
    INT softPins ;

    softPins = 0 ;
    if( numsoftG > 0 || numstdcellG > 0 ){
	/* first determine number of softcell with uncommitted pins */
	for( i=1;i<=totalcellsG;i++ ){
	    cptr = cellarrayG[i] ;
	    if( cptr->celltype == SOFTCELLTYPE ||
	        cptr->celltype == STDCELLTYPE ){
		if( cptr->numpins > 0 ){
		    softPins++ ;
		}
	    }
	}

	/* now build array */
	softPinArrayG = (CELLBOXPTR *)
	    Ysafe_malloc( (softPins+1) * sizeof(CELLBOXPTR) ) ;
	/* now loop again filling in array */
	cell = 0 ;
	for( i=1;i<=totalcellsG;i++ ){
	    cptr = cellarrayG[i] ;
	    if( cptr->celltype == SOFTCELLTYPE ||
	        cptr->celltype == STDCELLTYPE ){
		if( cptr->numpins > 0 ){
		    softPinArrayG[++cell] = cptr ;
		}
	    }
	}
	ASSERT( cell == softPins,"build_soft_array",
	    "Problem with count" ) ;
	softPinArrayG[HOWMANY] = (CELLBOXPTR) softPins ;

    } else {
	softPinArrayG = (CELLBOXPTR *) Ysafe_malloc( sizeof(CELLBOXPTR) ) ;
	softPinArrayG[HOWMANY] = (CELLBOXPTR) softPins ; /* zero */
    }
    
} /* end build_soft_array */

/* In fixing a cell, determine which side of the core to reference */
/* cell so that changes to the position of the cell due to core size */
/* changes will be minimized. */
VOID
determine_origin( x, y, left_not_right, bottom_not_top )
INT *x, *y ; /* point of reference */
char *left_not_right, *bottom_not_top ;
{
    if( *x <= blockmxG ){
	strcpy( left_not_right, "L" ) ;
    } else {
	strcpy( left_not_right, "R" ) ;
	/* reference to right side need to change x coordinate */
	*x = blockrG - *x ;
    }
    if( *y <= blockmyG ){
	strcpy( bottom_not_top, "B" ) ;
    } else {
	strcpy( bottom_not_top, "T" ) ;
	/* reference to top side need to change x coordinate */
	*y = blocktG - *y ;
    }
} /* end determine_origin */

delete_fix_constraint( cell )
INT cell ;
{
    CELLBOXPTR ptr ;

    ptr = cellarrayG[cell] ;
    if( ptr->fixed != (FIXEDBOXPTR)NULL ){
	Ysafe_free(ptr->fixed) ;
    }
    ptr->fixed = NIL(FIXEDBOXPTR) ;
    ptr->class = 0 ;
} /* end delete_fix_constraint */
