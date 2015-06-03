/*
 *   Copyright (C) 1988-1992 Yale University
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
FILE:	    uloop.c                                       
DESCRIPTION:inner loop of the simulated annealing algorithm.
CONTENTS:   uloop( limit )
		INT limit ;
	    make_movebox() ;
	    save_uloop( fp )
		FILE *fp ;
	    INT read_uloop( fp )
		FILE *fp ;
DATE:	    Feb 5, 1988 
REVISIONS:  July 21, 1988 - reversed order of softpin and aspect ratio
		moves to work with controller.
	    Sept 12, 1988 - rewrote cell move generation strategy.
	    Sept 24, 1988 - added testloop code that is added
		initStatCollection and getStatistics ;
	    Oct 20, 1988 - turn off bin controller when percentDone = 1.0
	    Oct 27, 1988 - handle 3 of 4 fixed cases.  Modified controller
		Now use only active cells in choice of cell.
	    Nov 14, 1988 - changed output format for easier reading.
	    Nov 25, 1988 - changed sequence of aspect ratio changes.
			 - modified negative feedback for better control.
	    Dec  3, 1988 - added timing driven code.
	    Dec  6, 1988 - fixed a bug in updating penalty.
	    Jan 15, 1988 - added curve fit controller,  changed overfill
		negative feedback control, and fixed GRAPHHEADING for
		new CRICKETGRAPH program.
	    Jan 25, 1989 - added softPinArrayG to make soft pin
		moves more efficient.  Cleaned up move selection
		strategy.  Changed msg to YmsgG.  Softpin moves
		are used during partitioning but overfill is not
		used.  Removed extraneous call to placepads.
	    Feb 25, 1989 - changed cell selection to get more cell
		moves at lower temperatures.  Made percentDone a
		static variable so it could be used in calc_new_core
		which was added so that the same code could be called
		at any time to change the aspect ratio.  Also added
		X11 graphics calls. Fixed problem with negative iterations
	    Feb 26, 1989 - added wait for user feature.
	    Feb 28, 1989 - updated schedule constants.
	    Mar 02, 1989 - moved schedule constants to temp.h for 
		consistency.  Removed extraneous variables.  Removed
		iter_count from call-now global.  Moved negative feedback
		code to penalties.c.
	    Mar 11, 1989 - added graphics conditional compile and always
		leave controller on.
	    Mar 14, 1989 - began instance coding.
	    Apr 20, 1989 - added partition coding.
	    May  1, 1989 - fixed problem with cell in different classes
		trying to be swapped.  Now we try another move correctly.
	    May 18, 1989 - update forceGrid.
	    May 24, 1989 - moved forceGrid to prboard.
	    Jun 21, 1989 - fixed problem with move_box initialization.
	    Sep 25, 1989 - made move box structure dynamic with respect
		to number of tiles.
	    Apr 23, 1990 - now correctly handle the timing constant
		thru the use of delta change in timing cost.
	    May 15, 1990 - fixed problem with standard cell macros.
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
	    Mon Feb  4 02:23:33 EST 1991 - added MINTUPDATE def.
	    Sun May  5 14:59:39 EDT 1991 - now all moves influence
		temperature scheduler.
	    Wed Jun  5 16:31:51 CDT 1991 - fixed problem with controller
		initialization
	    Mon Aug  5 18:06:17 CDT 1991 - eliminated special
		partitioning case.
	    Sat Nov 23 21:21:49 EST 1991 - began working with automatically
		setting move strategy.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) uloop.c version 3.14 4/6/92" ;
#endif

#include <custom.h>
#include <temp.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>

#define TMIN      1E-6
#define HOWMANY   0

extern INT pick_position() ;
/* ----------------------------------------------------------------- 
   important global definitions - defined in custom.h 
   MOVEBOXPTR *old_aposG, *new_aposG, *old_bposG, *new_bposG ;
   MOVEBOXPTR old_apos0G, new_apos0G, old_bpos0G, new_bpos0G ;
   usite routines use these globals - used arguments are commented.
*/

/* define the cell moves */
#define NUMSELECT                            16
     /* the cell move general categories */
#define CELL_MOVE                             1
#define ROTATION_MOVE                         2
#define ASPECT_MOVE                           3
#define PIN_MOVE                              4
#define INSTANCE_MOVE                         5
#define GROUP_MOVE                            6
#define NUMBER_MOVE_TYPES                     6
#define MOVE_ARRAY_SIZE                       NUMBER_MOVE_TYPES+1
     /* the possible cell moves */
#define NULL_MOVE                             0 
#define SINGLE_CELL_MOVE                      1 
#define SINGLE_CELL_MOVE_AND_ROTATE           2
#define PAIRWISE_CELL_SWAP                    3
#define PAIRWISE_CELL_SWAP_AND_ROTATION       4
#define ROTATION                              5
#define ASPECT_RATIO_CHANGE                   6
#define SOFT_PIN_MOVE                         7
#define GROUP_CELL_MOVE                       8
#define CELL_INSTANCE_CHANGE                  9
#define NUMBER_MOVES                         10


static INT  move_cntS = 0 ;
static INT  acc_cntS = 0 ;
static INT  dumpRatioS = 1 ; /* update screen after each outerloop */
static INT  dumpCountS = 0 ; /* count number of outer loops */
static DOUBLE total_costS, a_ratioS = 1.0 ;
static DOUBLE desired_ratioS = 1.0 ;
static BOOL   controlOnS = TRUE ;
static BOOL   firstTimeS = TRUE ;  /* for graphics control */
/* statistic collection */
static DOUBLE totalwireS ;
static DOUBLE totalpenalS ;
static DOUBLE num_dtimeS ;  /* number of delta time samples */
static DOUBLE avg_dtimeS ;  /* average random delta time */
static DOUBLE num_dfuncS ;  /* number of delta wirelength samples */
static DOUBLE avg_dfuncS ;  /* average random delta wirelength */




static output_move_table();



static INT TempSelectSoftS[11]  = {
    0,
    GROUP_MOVE, GROUP_MOVE, CELL_MOVE, CELL_MOVE,
    ASPECT_MOVE, ASPECT_MOVE, ROTATION_MOVE, PIN_MOVE,
    CELL_MOVE, CELL_MOVE
} ;

static INT TempSelectHardS[11]  = {
    0,
    GROUP_MOVE, GROUP_MOVE, CELL_MOVE, CELL_MOVE,
    GROUP_MOVE, CELL_MOVE, CELL_MOVE, ROTATION_MOVE,
    ROTATION_MOVE, ROTATION_MOVE
} ;

/* ***************************************************************** 
   uloop - inner loop of simulated annealing algorithm.
*/
uloop( limit )
INT limit ;
{

CELLBOXPTR acellptr , bcellptr ;
DOUBLE range , newAspect ;
DOUBLE calc_core_factor(), calc_lap_factor() ;
DOUBLE calc_time_factor() ;
DOUBLE percentDone ;
DOUBLE coin_toss ;
INT a , b ;
INT attempts, i ;
INT move_acceptted ;
INT binX , binY, bpos, numcells_in_bin ;
INT xb , yb , axcenter , aycenter ;
INT newaor, newbor, aorient , borient ;
INT proposed_move, selection, *moveSelection ;
INT lowerBound, upperBound ;
INT flip[NUMBER_MOVES] ;
INT att[NUMBER_MOVES] ;
DOUBLE move_size[MOVE_ARRAY_SIZE] ;
BOOL acc_move ;
BOOL asp_move_possible ;
FIXEDBOXPTR fixptr ;
BOOL checkbinList() ;

/* temperature control definitions */
INT         m1 = 1, m2 = 1;
DOUBLE      dCp = 0.0;
DOUBLE      temp, eval_ratio() ;

INT temp_timer, time_to_update ; /* keeps track of when to update T */
DOUBLE iter_time, accept_deviation, calc_acceptance_ratio() ;

INT old_time, old_func ; /* keep track of previous value of penalties */
INT delta_time, delta_func ; /* delta penalties */

if( activecellsG <= 0 ){
    M( WARNMSG, "uloop", "No active cells found. Aborting placement\n" ) ;
    cost_onlyG = TRUE ;
    restartG = TRUE ;
    return ;
}

flipsG   = 0 ;
attempts = 0 ;
for( i = 1; i < NUMBER_MOVES; i++ ){
    flip[i] = 0 ;
    att[i] = 0 ;
    if( i <= NUMBER_MOVE_TYPES ){
	move_size[i] = 0.0 ;
    }
} /* end for( i = 0;... */ 

/* number of moves before temperature update */
time_to_update = attmaxG / NUMTUPDATES ;
if( time_to_update < MINTUPDATE ) {
    time_to_update = MINTUPDATE ;
}
temp_timer = 0 ; /* initialize timer */
acc_cntS = 0 ;
/* initialize the counters if necessary */
if( iterationG <= 0 ){
    old_time = timingpenalG ;
    old_func = funccostG ;
}


/* ----------------------------------------------------------------- 
    DETERMINATION OF MOVE SELECTION POSSIBILITES.
    Determin move selection based on whether we have custom cells(hard)
    or undetermined cells( soft).  Hard cells can't change aspect ratio
    or move pins around so these moves possibilities should be removed
    from consideration.  However, if a single soft cell is present we
    must allow these possibilities.
*/
if( numsoftG > 0 || numstdcellG > 0 ){
    moveSelection = TempSelectSoftS ;
} else {
    moveSelection = TempSelectHardS ;
}

/* determine lowerBound based on temp */
/* skip over group moves */
if( ratioG > 0.34 ){
    lowerBound = 1 ;
} else if( ratioG > 0.17 ){
    lowerBound = 2 ;
} else {
    lowerBound = 3 ;
}
/* determine upperBound based on temp */
/* skip over group moves */
/* linearize so moves turn on slowly */
upperBound = 11 - (INT) (10.0 * ratioG)  ;
upperBound = (upperBound <= 10) ? upperBound : 10 ; 
upperBound = (upperBound >= 4) ? upperBound : 4 ; 
/* end move selection possibilities */

/* ----------------------------------------------------------------- 
    BEGINNING OF ANNEALING INNER LOOP.
*/

while( attempts < limit ) {

    D( "uloop/bin", checkbinList() ; ) ;

    /* randomly pick cell an active cell a */
    a = PICK_INT( 1, activecellsG ) ;

    /* initialize position records for use in overlap.c */
    /* delay assignment to b record until needed */
    /* see below */
    acellptr = activeCellarrayG[ a ]   ;
    /* now that we have a point to a cell record */
    /* get cell number from record */
    a = acellptr->cellnum ;
    new_apos0G->cell = old_apos0G->cell = a ;
    old_apos0G->orient = acellptr->orient ;
    axcenter = new_apos0G->xcenter = 
	old_apos0G->xcenter = acellptr->xcenter ;
    aycenter = new_apos0G->ycenter = 
        old_apos0G->ycenter = acellptr->ycenter ;

    MOVE_PICK:
    do {
	if( quickrouteG ){
	    proposed_move = PICK_INT( 1, NUMBER_MOVE_TYPES ) ;
	} else {
	    /* randomly pick a number 1..number elements in moveSelection */
	    selection = PICK_INT( lowerBound, upperBound ) ;
	    /* find out what the move is */
	    proposed_move = moveSelection[selection] ;
	}

	switch( proposed_move ){

	case CELL_MOVE:
	    if( acellptr->group ){
		if( acellptr->celltype == GROUPCELLTYPE ){
		    /* pick new group position using */
		    /* exponential distribution */
		    pick_position(&xb,&yb,axcenter,aycenter);
		    /* load move record with coordinates to move to */
		    /* first keep same orientation */
		    new_apos0G->xcenter = xb ;
		    new_apos0G->ycenter = yb ;
		    new_apos0G->orient = acellptr->orient ;
		    /* group moves not implemented yet */
		    proposed_move = 0 ;
		    break ; /* go to end of case statement */

		} else {
		    /* CELL OF A GROUP */
		    /* we must keep cell with group */
		    fixptr = acellptr->group->fixed ;
		    /* pick new cell position using */
		    /* exponential distribution */
		    pick_neighborhood(&xb,&yb,axcenter,aycenter,fixptr) ;
		}
		
	    } else if( fixptr = acellptr->fixed ){
		/* two cases - fixed at point ii)fixed in neighborhood */
		if( fixptr->fixedType == POINTFLAG ){
		    /* if a cell is fixed at a point, */
		    /* try a rotation only */
		    /* calculate bin for move determination */
		    binX = SETBINX(axcenter) ; 
		    binY = SETBINY(aycenter) ;
		    newCellListG = binptrG[binX][binY]->cells ;
		    proposed_move = ROTATION ;
		    /* we are finished break out of case */
		    break ;

		} else {  /* cell center is fixed in a neighborhood */
		    /* pick new cell position using */
		    /* exponential distribution */
		    pick_neighborhood(&xb,&yb,axcenter,aycenter,fixptr) ;
		}

	    } else { /* a normal cell move */
		/* pick new cell position using */
		/* exponential distribution */
		pick_position(&xb,&yb,axcenter,aycenter);

	    } /* end pick cell position */

	    /* load move record with coordinates to move to */
	    /* first keep same orientation */
	    new_apos0G->xcenter = xb ;
	    new_apos0G->ycenter = yb ;
	    new_apos0G->orient = acellptr->orient ;

	    /* calculate bin for move determination */
	    /* if bin is empty - single cell move */
	    binX = SETBINX(xb) ; 
	    binY = SETBINY(yb) ;
	    newbptrG = binptrG[binX][binY] ;
	    newCellListG = newbptrG->cells ;
	    numcells_in_bin = newCellListG[0] ;

	    /* if no cells in this bin attempt single cell move */
	    /* or cell a is the only cell in this bin - note 2 is */
	    /* first bin which holds cells */
	    if( numcells_in_bin == 0 || 
		(numcells_in_bin == 1 && newCellListG[1] == a) ){
		proposed_move = SINGLE_CELL_MOVE ;
	    } else {
		proposed_move = PAIRWISE_CELL_SWAP ;
	    }
	    break ;

	case ROTATION_MOVE:
	    /* calculate bin for move determination */
	    binX = SETBINX(axcenter) ; 
	    binY = SETBINY(aycenter) ;
	    newCellListG = binptrG[binX][binY]->cells ;
	    proposed_move = ROTATION ;
	    break ;

	case ASPECT_MOVE:
	    if( acellptr->softflag && acellptr->aspUB > 0.01 + acellptr->aspLB ){
		proposed_move = ASPECT_RATIO_CHANGE ;
	    } else {
		proposed_move = 0 ;
	    }
	    break ;

	case PIN_MOVE:
	    /* to make pin moves more efficient, use softPinArrayG */
	    /* which keeps track of all softcells which have pins */
	    /* which can move. softPinArrayG[0] holds size of array */
	    if( (INT) softPinArrayG[HOWMANY] > 0 ){
		proposed_move = SOFT_PIN_MOVE ;
		/* pick one of the soft cells */
		selection = PICK_INT( 1, (INT) softPinArrayG[HOWMANY] );
		/* now get cellptr */
		acellptr = softPinArrayG[selection] ;

	    } else { /* if no soft pins are available give up */
		proposed_move = 0 ;
	    }
	    break ;

	case INSTANCE_MOVE:
	    if( acellptr->instptr ){
		proposed_move = CELL_INSTANCE_CHANGE ;
	    } else {
		proposed_move = 0 ;
	    }
	    break ;

	case GROUP_MOVE:
	    /* not implemented yet */
	    proposed_move = 0 ;
	    break ;

	} /* end switch */

	
    } while( !(proposed_move) ) ;

    /* *********** NOW EVALUATE THE PROPOSED MOVE ********* */

    MOVE_START:switch( proposed_move ){

    case SINGLE_CELL_MOVE:
	/* first try moving cell */
	if( acc_move = usite1( /* old_apos, new_apos */ ) ) {
	    flipsG++ ;
	    flip[SINGLE_CELL_MOVE]++ ;
	    acc_cntS++ ;
	} 
	att[SINGLE_CELL_MOVE]++ ;
	if( quickrouteG ){
	    move_size[CELL_MOVE] += ABS(d_costG) ;
	}
	break ;

    case SINGLE_CELL_MOVE_AND_ROTATE:
	/* try a rotation of cell and move  */
	new_apos0G->orient = newOrient( acellptr , 4 ) ;
	if( new_apos0G->orient >= 0 ) {
	    if( acc_move = usite1( /* old_apos, new_apos */ ) ) {
		flipsG++ ;
		flip[SINGLE_CELL_MOVE_AND_ROTATE]++ ;
		acc_cntS++ ;
	    }
	    att[SINGLE_CELL_MOVE_AND_ROTATE]++ ;
	    if( quickrouteG ){
		move_size[CELL_MOVE] += ABS(d_costG) ;
	    }
	}
	break ;

    case PAIRWISE_CELL_SWAP:
	/* make sure we don't pick cell a if a exists in this bin */
	do {
	    bpos = PICK_INT( 1, newCellListG[0] ) ;
	    b = newCellListG[bpos] ;
	} while ( a == b ) ;

	bcellptr = cellarrayG[b] ;

	/* now check classes */
	if( acellptr->class != bcellptr->class ){
	    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	        This is an invalid move but don't want to count as 
	        a move.  Because break won't get us out of loop
	        we need to use goto.  Try another type of move for
		this cell.
	    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	    goto MOVE_PICK ;
	}
	new_bpos0G->cell = old_bpos0G->cell = b ;
	old_bpos0G->xcenter = bcellptr->xcenter ;
	old_bpos0G->ycenter = bcellptr->ycenter ;
	old_bpos0G->orient = new_bpos0G->orient = bcellptr->orient ;

	new_apos0G->xcenter = bcellptr->xcenter ;
	new_apos0G->ycenter = bcellptr->ycenter ;
	new_bpos0G->xcenter = acellptr->xcenter ;
	new_bpos0G->ycenter = acellptr->ycenter ;
	/* first try exchanging a and b positions */
	if( acc_move = usite2( /*old_apos,new_apos,old_bpos,new_bpos*/ )) {
	    acc_cntS++ ;
	    flipsG++ ;
	    flip[PAIRWISE_CELL_SWAP]++ ;
	} 
	att[PAIRWISE_CELL_SWAP]++ ;
	if( quickrouteG ){
	    move_size[CELL_MOVE] += ABS(d_costG) ;
	}
	break ;

    case PAIRWISE_CELL_SWAP_AND_ROTATION:
	/*
	    try again with opposite orientation types. Remember
	    old orientation in case of neworient failure.
	*/
	aorient = acellptr->orient ;
	borient = bcellptr->orient ;
	newaor = new_apos0G->orient = newOrient( acellptr , 4 ) ;
	newbor = new_bpos0G->orient = newOrient( bcellptr , 4 ) ;
	if( newaor >= 0 || newbor >= 0 ) {
	    if( newaor < 0 ) {
		new_apos0G->orient = aorient ;
	    } else if( newbor < 0 ) {
		new_bpos0G->orient = borient ;
	    }

	    /* try rotation and move of both cells */
	    /* make sure new_apos is correct not changed by */
	    /* previous call of usite2 */
	    new_apos0G->xcenter = bcellptr->xcenter ;
	    new_apos0G->ycenter = bcellptr->ycenter ;
	    new_bpos0G->xcenter = acellptr->xcenter ;
	    new_bpos0G->ycenter = acellptr->ycenter ;
	    if( acc_move = usite2( /* old_apos, new_apos, old_bpos, 
		    new_bpos */ )){
		acc_cntS++ ;
		flipsG++  ;
		flip[PAIRWISE_CELL_SWAP_AND_ROTATION]++ ;
	    }
	    att[PAIRWISE_CELL_SWAP_AND_ROTATION]++ ;
	    if( quickrouteG ){
		move_size[CELL_MOVE] += ABS(d_costG) ;
	    }

	} /* end pairwise_cell_swap_and rotation */	
	break ;

    case ROTATION:
	/* try rotation of cell only */
	/* set coordinates back to original place */
	new_apos0G->xcenter = acellptr->xcenter ;
	new_apos0G->ycenter = acellptr->ycenter ;
	/* calculate bin for move determination */
	binX = SETBINX(axcenter) ; 
	binY = SETBINY(aycenter) ;
	newCellListG = binptrG[binX][binY]->cells ;
	new_apos0G->orient = newOrient( acellptr , 8 ) ;
	if( new_apos0G->orient >= 0 ) {
	    /* set coordinates back to original place */
	    if( acc_move = usite1( /* old_apos, new_apos */ ) ) {
		flipsG++ ;
		acc_cntS++ ;
		flip[ROTATION]++ ;
	    }
	    att[ROTATION]++ ;
	    if( quickrouteG ){
		move_size[ROTATION_MOVE] += ABS(d_costG) ;
	    }
	}
	break ;

    case ASPECT_RATIO_CHANGE:
	/*
	 *   The goal here is to generate a new
	 *   aspect ratio for the cell.
	 */
	range = acellptr->aspUB - acellptr->aspLB;
	newAspect = range * ((DOUBLE)RAND / 
		(DOUBLE) 0x7fffffff) + acellptr->aspLB ;
	/* insure cell center and orientation is correct */
	new_apos0G->orient = acellptr->orient ;
	new_apos0G->xcenter = acellptr->xcenter ;
	new_apos0G->ycenter = acellptr->ycenter ;
	/* need to make sure we count right number of tiles */
	/* see overlap.c - calc_wBins for details */
	new_apos0G->numtiles = acellptr->numtiles ;
	if( acc_move = uaspect( a , newAspect ) ) {
	    flipsG++ ; 
	    acc_cntS++ ;
	    flip[ASPECT_RATIO_CHANGE]++ ;
	}
	att[ASPECT_RATIO_CHANGE]++ ;
	if( quickrouteG ){
	    move_size[ASPECT_MOVE] += ABS(d_costG) ;
	}
	break ;

    case SOFT_PIN_MOVE:
	if( upinswap( acellptr->cellnum ) ){
	    flipsG++ ; 
	    flip[SOFT_PIN_MOVE]++ ;
	    acc_cntS++ ;
	    if( d_costG == 0 ){
		/* there are two many of these moves make them */
		/* track with acceptance rate */
		coin_toss = ROLL_THE_DICE() ;
		if( coin_toss > desired_ratioS ){
		    /* make this a reject instead */
		    acc_cntS-- ;
		    flip[SOFT_PIN_MOVE]-- ;
		    flipsG-- ; 
		}
	    }
	}
	att[SOFT_PIN_MOVE]++ ;
	if( quickrouteG ){
	    move_size[PIN_MOVE] += ABS(d_costG) ;
	}
	break ;

    case GROUP_CELL_MOVE:
	/* to be implemented */
	break ;

    case CELL_INSTANCE_CHANGE:
	new_apos0G->orient  = acellptr->orient ;
	new_apos0G->xcenter = acellptr->xcenter ;
	new_apos0G->ycenter = acellptr->ycenter ;
	D( "twmc/instance", checkcost() ) ;
	if( acc_move = uinst( /* old_apos, new_apos */) ) {
	    flipsG++ ;
	    flip[CELL_INSTANCE_CHANGE]++ ;
	    acc_cntS++ ;
	}
	att[CELL_INSTANCE_CHANGE]++ ;
	if( quickrouteG ){
	    move_size[INSTANCE_MOVE] += ABS(d_costG) ;
	}
	D( "twmc/instance", checkcost() ) ;
	break ;


    } /* end switch */

    /* *********** END OF PROPOSED MOVE  EVALUATION ********* */
    attempts++ ;

    /* see if user has requested an interuption for graphics */
    if( doGraphicsG && iterationG >= -1 ){
	/* at iteration = -1 we wait for user to continue if requested */
	if( iterationG == -1  && wait_for_userG && firstTimeS ){
	    G( TWmessage( "TimberWolfMC waiting for your response" ) ) ;
	    G( process_graphics() ) ;
	    firstTimeS = FALSE ;

	/* otherwise see if user has interupted program */
	/* G( ) is NOGRAPHICS conditional compile */
	} else G( if( TWinterupt() ) ){
	    G( process_graphics() ) ;
	}
    }
    /* ----------------------------------------------------------- 
	Update annealing schedule 
    */
    /* if debug on check cost after each move */
    D( "uloop/incr.", checkcost() );
    D( "uloop/every_move",
	static BOOL debugVarL = FALSE ;
	if( debugVarL ){
	    G( process_graphics() ) ;
	} else {
	    G( draw_the_data() ) ; 
	}
    ) ;

    if (iterationG <= 0 ) {
	/* statistic collection for overlap and pinFactor determination */
	avg_funcG = (avg_funcG * avgsG + funccostG) / (avgsG + 1.0) ;
	avgsG += 1.0 ;
	totalwireS += (DOUBLE) funccostG ;
	totalpenalS += (DOUBLE) penaltyG ;

	delta_time = ABS( old_time - timingpenalG ) ;
	if( delta_time != 0 ){
	    /* calculate a running average of the delta timing penalty */
	    num_dtimeS += 1.0 ;
	    avg_dtimeS = (avg_dtimeS * (num_dtimeS - 1.0) +
		(DOUBLE) delta_time) / num_dtimeS ;

	    /* calculate a running average of the delta wiring penalty */
	    delta_func = ABS( old_func - funccostG ) ;
	    num_dfuncS += 1.0 ;
	    avg_dfuncS = (avg_dfuncS * (num_dfuncS - 1.0) +
		(DOUBLE) delta_func) / num_dfuncS ;
	}
	/* now update the old values of the timing and wirelength */
	old_time = timingpenalG ;
	old_func = funccostG ;

	if (iterationG != 0.0 ){
	    /* stay at a high temperature */
	    continue;
	}

	/* lower temperature to starting point from infinite temp. */
	/* do this to get starting temperature correct */
	if (d_costG >= 0){
	    /*** d_cost is the -ve of the actual d_cost ***/
	    m1 ++;
	} else {
	    dCp -= d_costG;
	    m2 ++;
	}
	temp = (init_accG * attempts - m1) / m2;
	if (temp <= 0.0) {
	    TG *= 0.9;
	} else {
	    TG = -dCp / (m2 * log(temp));
	}
	continue;		/*** initialization phase. JL **/
    }

    /* ----------------------------------------------------------------- 
       Update temperature using negative feedback to control the
       acceptance ratio in accordance to curve fit schedule.  
       Calc_acceptance_ratio returns desired acceptance ratio give
       the iteration.  The damped error term (deviation) is then applied 
       to correct the temperature T.  Update_window_size controls the 
       range limiter.  We avoid updating T during initialization, 
       we use exact schedule to compute starting T.  The temp_timer 
       allows us to update temperature inside the inner loop
       of the annealing algorithm. We use counter to avoid use of mod
       function for speed.  We can either pick from incremental a_ratio
       or cummulative ratio.
     ------------------------------------------------------------------ */
    if( ++temp_timer >= time_to_update || attempts >= attmaxG ) {
	a_ratioS = (DOUBLE) acc_cntS / (DOUBLE) temp_timer;/*incremental*/
	temp_timer = 0 ; /* reset counter */
	acc_cntS = 0;     /* reset incremental timer */
	iter_time = (DOUBLE) iterationG +
		    (DOUBLE) attempts / (DOUBLE) attmaxG ;
	/* maintain desired ratio from iteration to iteration */
	desired_ratioS = calc_acceptance_ratio( iter_time ) ;
	accept_deviation = desired_ratioS - a_ratioS ;

#ifdef NEEDED
	Yplot_heading( 0, "graph_T", "iter","pre_def","post_dev",
	    "run_avg","calc_dev", "meas_dev","T", NULL ) ;
	Yplot( 0, "graph_T", "%d", iterationG, "%4.2le", accept_deviation ) ;

	Yplot( 0, "graph_T", "%d", iterationG,
	    "%4.4le %4.4le %4.4le %4.4le %4.4le", accept_deviation,
	    running_avg, calc_acceptance_ratio(iter_time), a_ratioS, TG );
	Yplot_flush( "graph_T" ) ;
#endif
	if( iterationG <= (INT) HIGHTEMP ) {
	    /* no change to damping factor */
	} else if( iterationG <= (INT) TURNOFFT ) {
	    accept_deviation *= ACCEPTDAMPFACTOR ; 
	} else {
	    accept_deviation *= ACCEPTDAMPFACTOR2 ;
	}
	TG *= 1.0 + accept_deviation ;
	TG = MAX( TG, TMIN ) ; /* to avoid floating point errors */
	update_window_size( iter_time ) ;
    }
    /*
	End of annealing schedule update
     ----------------------------------------------------------- */

    /* *******************************************************
       Now if moves were rejected and they were single cell 
       or pairwise cell moves, try the same moves but with
       rotations.  We think it makes sense to try another move
       with rotation if the move without rotation failed.
       Goto takes us not to beginning of loop but to top of
       move selection switch.  We need to fall to bottom of
       loop so that the statistics are updated. 
       ******************************************************** */
    if( !(acc_move) ){ /* move wasn't accepted */
	switch( proposed_move ){
	    case SINGLE_CELL_MOVE:
		proposed_move = SINGLE_CELL_MOVE_AND_ROTATE ;
		goto MOVE_START ;
	    case SINGLE_CELL_MOVE_AND_ROTATE:
		/* - - - - - - - - - - - - - - - - - - - - - - - - - -
		    If we fail rotation and move, propose to try either
		    aspect ratio change or/and instance change depending
		    on what moves are allowed.  If both are allowed
		    roll the dice.
		 - - - - - - - - - - - - - - - - - - - - - - - - - - - */
		if( acellptr->softflag &&
		    acellptr->aspUB > 0.01 + acellptr->aspLB ){
		    asp_move_possible = TRUE ;
		} else {
		    asp_move_possible = FALSE ;
		}
		if( asp_move_possible && acellptr->instptr ){
		    /* roll the dice */
		    if( PICK_INT( 0, 1 ) ){
			proposed_move = ASPECT_RATIO_CHANGE ;
		    } else {
			proposed_move = CELL_INSTANCE_CHANGE ;
		    }
		    goto MOVE_START ;
		} else if( acellptr->instptr ){
		    proposed_move = CELL_INSTANCE_CHANGE ;
		    goto MOVE_START ;
		} else if( asp_move_possible){
		    proposed_move = ASPECT_RATIO_CHANGE ;
		    goto MOVE_START ;
		} else {
		    /* done sequence */
		    break ;
		}
	    case PAIRWISE_CELL_SWAP:
		proposed_move = PAIRWISE_CELL_SWAP_AND_ROTATION ;
		goto MOVE_START ;
	    case PAIRWISE_CELL_SWAP_AND_ROTATION:
		/* done sequence */
		break ;
	    case ROTATION:
		/* done sequence */
		break ;
	    case ASPECT_RATIO_CHANGE:
		/* if we fail aspect ratio change propose pin change */
		if( acellptr->numpins > 0 ){
		    /* end of sequence try only one so we don't due */
		    /* too much work at high T. */
		    /* proposed_move = SOFT_PIN_MOVE ; */
		    proposed_move = 0 ;
		    goto MOVE_START ;
		} else {
		    /* done sequence */
		    break ;
		}
	    case SOFT_PIN_MOVE:
		/* done sequence */
		break ;
	    case GROUP_CELL_MOVE:
		/* done sequence */
		break ;
	    case CELL_INSTANCE_CHANGE:
		/* done sequence */
		break ;
	} /* end switch */
    }	


} /* ****** END OF ANNEALING INNER LOOP **************** */

/* verify incremental and current costs after each iteration */
D( "uloop", checkcost() ) ;

if( !(++dumpCountS % dumpRatioS) ){
    G( setGraphicWindow() ) ;
    G( draw_the_data() ) ;
}

/* ----------------------------------------------------------------- 
   Use negative feedback to control relative ratio between penalties
*/
/* first get percentage of the task we have completed - temp */
percentDone = eval_ratio( iterationG ) ;

if( percentDone >= FINISHED ){
    /* turn off controller when percent done is 1.0 */
    /* controlOn is initialize to TRUE and once turned off cannot */
    /* get turned back on */
    /* controlOnS = FALSE ; */
}

if( controlOnS ){
    /* **** core area controller **** */
    coreFactorG = calc_core_factor( percentDone ) ;

    /* **** overlap penalty controller **** */
    lapFactorG = calc_lap_factor( percentDone ) ;

    /* **** timing penalty controller **** */
    timeFactorG = calc_time_factor( percentDone ) ;

} /* end negative feedback controller code */

/* reset penalties */
penaltyG  = (INT) (lapFactorG * sqrt( (DOUBLE) binpenalG ) ) ;
timingcostG = (INT) (timeFactorG * (DOUBLE) timingpenalG ) ;

total_costS = (DOUBLE) (funccostG + penaltyG + timingcostG ) ;
ratioG = ( (DOUBLE) flipsG / (DOUBLE) attempts ) ;
FLUSHOUT() ;

if( quickrouteG ){
    output_move_table( flip, att, move_size ) ;
}

/* ----------------------------------------------------------------- 
   now output statistics for this temperature.
*/

OUT1("\nI     T     funccost  binpen x lapFact = penalty  cost coreFactor\n");
OUT2("%3d ",iterationG ); 
OUT2("%4.2le ",TG ); 
OUT2("%4.2le ",(DOUBLE) funccostG ); 
OUT2("%4.2le ",(DOUBLE) binpenalG ); 
OUT2("%4.2le ",lapFactorG ); 
OUT2("%4.2le ",(DOUBLE) penaltyG ); 
OUT2("%4.2le ",total_costS ); 
OUT2("%4.2le\n",coreFactorG ); 
OUT1("timeFactor timepenal timecost  var perDone\n");
OUT2("%4.2le ",(DOUBLE) timeFactorG ); 
OUT2("%4.2le ",(DOUBLE) timingpenalG ); 
OUT2("%4.2le ",(DOUBLE) timingcostG ); 
OUT2("%4.2le\n ",percentDone ); 
OUT1(" flip1   flipo   flip0   flipp   flipa   flip2   flipo2  flipi   flips ratio\n");
OUT3("%3d/%3d ",flip[SINGLE_CELL_MOVE],att[SINGLE_CELL_MOVE] ); 
OUT3("%3d/%3d ",flip[SINGLE_CELL_MOVE_AND_ROTATE],
                att[SINGLE_CELL_MOVE_AND_ROTATE] ); 
OUT3("%3d/%3d ",flip[ROTATION],att[ROTATION] ); 
OUT3("%3d/%3d ",flip[SOFT_PIN_MOVE],att[SOFT_PIN_MOVE] ); 
OUT3("%3d/%3d ",flip[ASPECT_RATIO_CHANGE],att[ASPECT_RATIO_CHANGE] ); 
OUT3("%3d/%3d ",flip[PAIRWISE_CELL_SWAP],att[PAIRWISE_CELL_SWAP] ); 
OUT3("%3d/%3d ",flip[PAIRWISE_CELL_SWAP_AND_ROTATION],
		att[PAIRWISE_CELL_SWAP_AND_ROTATION] ); 
OUT3("%3d/%3d ",flip[CELL_INSTANCE_CHANGE],att[CELL_INSTANCE_CHANGE] ); 
OUT3(" %3d/%3d ",flipsG,attempts ); 
OUT2(" %4.2f\n\n",ratioG ); 
FLUSHOUT() ;


/* GRAPH( graphFileName, xVarformat, xVar, yVarformat, yVars... ) */ 

Yplot_heading( 0, "graph", "iter","funccost","binpenal",
    "percentDone","lapFactor","coreFactor", "ratio",
    "T", "totalcost", NULL ) ;
Yplot( 0, "graph", "%d", iterationG,
    "%d %d %d %4.2le %4.2le %4.2le %4.2le %4.2le %4.2le",
    funccostG,binpenalG,percentDone,lapFactorG,
    coreFactorG, ratioG, TG, total_costS ) ;
Yplot_flush( "graph" ) ;

return ;
} /* end uloop */
/* ***************************************************************** */


/* initializes random wiring and overlap statistics */
initStatCollection()
{
    avgsG = 0.0 ;
    avg_funcG = 0.0 ;
    totalwireS  = 0.0 ;
    totalpenalS = 0.0 ;
    avg_dtimeS = 0.0 ;
    num_dtimeS = 0.0 ;
    avg_dfuncS = 0.0 ;
    num_dfuncS = 0.0 ;
    ratioG = 1.0 ;
} /* end initStatCollection */
/* ***************************************************************** */

getStatistics( totalWire, totalPenalty, avg_time, avg_func )
DOUBLE *totalWire, *totalPenalty, *avg_time, *avg_func ;
{
    *totalWire = totalwireS ;
    *totalPenalty = totalpenalS ;
    *avg_time = avg_dtimeS ;
    *avg_func = avg_dfuncS ;
} /* end getStatistics */

/* ***************************************************************** 
   make_movebox - ALLOCATE MEMORY FOR MOVE RECORDS
   move boxes - old_aposG[0] holds info for old cell ie orient cell
		old_aposG[1..n] holds tile information for old cell.
		new_aposG[1..n] holds tile information for new cell.
		softcells only use old_aposG[1] and new_aposG[1]
		    since they can have only one tile.
*/
make_movebox() 
{
    INT i ;
    INT maxtiles ;

    maxtiles = get_tile_count() ;

    /* allocate four arrays of moveboxs */
    old_aposG = (MOVEBOXPTR *) 
	    Ysafe_malloc( maxtiles * sizeof(MOVEBOXPTR) ); 
    for( i = 0; i< maxtiles ; i++ ){
	old_aposG[i] = (MOVEBOXPTR) Ysafe_malloc( sizeof(MOVEBOX) ) ;
	old_aposG[i]->loaded_previously = FALSE ;
    }
    old_apos0G = old_aposG[0] ;

    new_aposG = (MOVEBOXPTR *) 
	    Ysafe_malloc( maxtiles * sizeof(MOVEBOXPTR) ); 
    for( i = 0; i< maxtiles ; i++ ){
	new_aposG[i] = (MOVEBOXPTR) Ysafe_malloc( sizeof(MOVEBOX) ) ;
	new_aposG[i]->loaded_previously = FALSE ;
    }
    new_apos0G = new_aposG[0] ;

    old_bposG = (MOVEBOXPTR *) 
	    Ysafe_malloc( maxtiles * sizeof(MOVEBOXPTR) ); 
    for( i = 0; i< maxtiles ; i++ ){
	old_bposG[i] = (MOVEBOXPTR) Ysafe_malloc( sizeof(MOVEBOX) ) ;
	old_bposG[i]->loaded_previously = FALSE ;
    }
    old_bpos0G = old_bposG[0] ;

    new_bposG = (MOVEBOXPTR *) 
	    Ysafe_malloc( maxtiles * sizeof(MOVEBOXPTR) ); 
    for( i = 0; i< maxtiles ; i++ ){
	new_bposG[i] = (MOVEBOXPTR) Ysafe_malloc( sizeof(MOVEBOX) ) ;
	new_bposG[i]->loaded_previously = FALSE ;
    }
    new_bpos0G = new_bposG[0] ;

} /* end function make_movebox */

/* ***************************************************************** 
   save_uloop - save uloop parameters for restart
*/
save_uloop( fp )
FILE *fp ;
{
    fprintf(fp,"# uloop parameters:\n") ;
    fprintf(fp,"%d %d %d\n",iterationG,acc_cntS,move_cntS);
    fprintf(fp,"%f %f\n",a_ratioS,total_costS);
} /* end save_uloop */

/* ***************************************************************** 
   read_uloop - read uloop parameters for restart
*/
INT read_uloop( fp )
FILE *fp ;
{
    INT error = 0 ;

    fscanf(fp,"%[ #:a-zA-Z]\n",YmsgG ); /* throw away comment */
    fscanf(fp,"%ld %ld %ld\n",&iterationG,&acc_cntS,&move_cntS);
    fscanf(fp,"%lf %lf\n",&a_ratioS,&total_costS);
    /* try to detect errors in file */
    if( acc_cntS < 0 ){
	M(ERRMSG,"read_uloop","Restart file: acc_cntS negative\n") ;
	error++ ;
    }
    if( move_cntS < 0 ){
	M(ERRMSG,"read_uloop","Restart file: move_cnt negative\n") ;
	error++ ;
    }
    if( a_ratioS < 0.0 ){
	M(ERRMSG,"read_uloop","Restart file: a_ratio negative\n") ;
	error++ ;
    }
    if( a_ratioS > 1.0 ){
	M(ERRMSG,"read_uloop","Restart file: a_ratio >1\n") ;
	error++ ;
    }
    if( iterationG < 1 ){
	M(ERRMSG,"read_uloop","Restart file: iteration count <1\n") ;
	error++ ; /* one error */
    }
    return(error) ;

} /* end read_uloop */

set_dump_ratio( count )
{
    dumpRatioS = count ;
} /* end dump_ratio */


static output_move_table( flip, att, move_size )
INT *flip, *att ;
DOUBLE *move_size ;
{
    INT  i ;
    INT a[MOVE_ARRAY_SIZE] ;  /* attempts for the individual moves */
    INT f[MOVE_ARRAY_SIZE] ;  /* flips for the individual moves */
    FILE *fp ;
    DOUBLE Qm[MOVE_ARRAY_SIZE] ;
    DOUBLE P[MOVE_ARRAY_SIZE] ;
    DOUBLE Qsum ;
    char filename[LRECL] ;

    sprintf( filename, "%s.mset", cktNameG ) ;
    fp = TWOPEN( filename, "a", ABORT ) ;
    fprintf( fp, "I:%d ", iterationG ) ;

    /* first calculate f (flip) and a (attempts) from indivial vectors */

    f[CELL_MOVE] = flip[SINGLE_CELL_MOVE] + flip[SINGLE_CELL_MOVE_AND_ROTATE] +
	    flip[PAIRWISE_CELL_SWAP_AND_ROTATION] + flip[PAIRWISE_CELL_SWAP] ;
    a[CELL_MOVE] = att[SINGLE_CELL_MOVE] + att[SINGLE_CELL_MOVE_AND_ROTATE] +
	    att[PAIRWISE_CELL_SWAP] + att[PAIRWISE_CELL_SWAP_AND_ROTATION] ;

    f[ROTATION_MOVE] = flip[ROTATION] ;
    a[ROTATION_MOVE] = att[ROTATION] ;

    f[ASPECT_MOVE] = flip[ASPECT_RATIO_CHANGE] ;
    a[ASPECT_MOVE] = att[ASPECT_RATIO_CHANGE] ;

    f[PIN_MOVE] = flip[SOFT_PIN_MOVE] ;
    a[PIN_MOVE] = att[SOFT_PIN_MOVE] ;

    f[GROUP_MOVE] = flip[GROUP_CELL_MOVE] ;
    a[GROUP_MOVE] = att[GROUP_CELL_MOVE] ;

    f[INSTANCE_MOVE] = flip[CELL_INSTANCE_CHANGE] ;
    a[INSTANCE_MOVE] = att[CELL_INSTANCE_CHANGE] ;

    for( i = 1; i <= NUMBER_MOVE_TYPES; i++ ){
	/* only output moves that were attempted */
	fprintf( fp, "%d:%d/%d %4.2le  ", i, f[i], a[i], move_size[i] ) ;
    }
    fprintf( fp, "\n" ) ;

    /* **************************************************************
	Calculate the percentages

	Qm = #accepted_moves   * average_move_size 
	     ---------------------------------------
	       # attempted_moves (for this move type)
	where average_move_size is move_size[i] / a[i].

    ***************************************************************** */
    Qsum = 0.0 ;
    for( i = 1; i <= NUMBER_MOVE_TYPES; i++ ){
	if( a[i] ){
	    Qm[i] = f[i] * move_size[i] / (DOUBLE) a[i] / (DOUBLE) a[i] ;
	} else {
	    Qm[i] = 0.0 ;
	}
	Qsum += Qm[i] ;
    } /* end for( i = 1;... */


    for( i = 1; i <= NUMBER_MOVE_TYPES; i++ ){
	P[i] = Qm[i]/Qsum ;
	fprintf( fp, "Qm[%d]=%4.2le P[%d]=%4.2le  ", i, Qm[i], i, Qm[i]/Qsum ) ;
    } /* end for( i = 1;... */
    fprintf( fp, "\n" ) ;

    /* PLOT THE RESULTS TO SEE TRENDS */
    Yplot_heading( 0, "graph_prob", "iter","P[1]","P[2]","P[3]",
	"P[4]", "P[5]", "P[6]", NULL ) ;

    for( i = 1; i <= NUMBER_MOVE_TYPES; i++ ){
	Yplot( 0, "graph_prob", "%d", iterationG, "%4.2le", P[i] ) ;
    }
    Yplot_flush( "graph_prob" ) ;

    TWCLOSE( fp ) ;
} /* end output_move_table */
