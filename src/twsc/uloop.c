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
FILE:	    uloop.c                                       
DESCRIPTION:inner loop of simulated annealing algorithm.
CONTENTS:   
	    save_control( fp )
		FILE *fp ;
	    read_control( fp )
		FILE *fp ;
	    INT eval_ratio(t)
		DOUBLE *t;
	    init_uloop()
	    init_control(first)
		INT first;
	    pick_position(x,y,ox,oy,scale)
		INT *x,*y,ox,oy;
		DOUBLE scale ;
	    update_window_size( iternum )
		DOUBLE iternum ;
	    uloop()
	    rowcon()
	    DOUBLE partition( C_initial,k_initial,p_initial,R_initial )
		INT C_initial , k_initial , p_initial , R_initial ;
	    DOUBLE expected_value(C_initial,k_initial,p_initial,R_initial)
		INT C_initial , k_initial , p_initial , R_initial ;
	    DOUBLE expected_svalue(C_initial,k_initial,
		p_initial,R_initial ) 
	    INT C_initial , k_initial , p_initial , R_initial ; 
	    DOUBLE compute_and_combination( C , k , p , R )
		INT C , k , p , R ;
	    DOUBLE combination( numerator , denominator )
		INT numerator , denominator ;
	    sanity_check()
	    sanity_check2()
	    sanity_check3()
DATE:	    Mar 27, 1989 
REVISIONS:  Mar 29, 1989 - removed vertical / horz wire weighting.
	    Apr  1, 1990 - removed vertical wire weight from table.
	    Wed Jun  5 16:48:31 CDT 1991 - fixed problem with T
		controller initialization for zero delta costs.
	    Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) uloop.c (Yale) version 4.14 3/23/92" ;
#endif
#endif

#define UCXXGLB_VARS

#include "standard.h"
#include "ucxxglb.h"
#include "main.h"
#include "readpar.h"
#include "parser.h"
#include "yalecad/message.h"
#include "yalecad/debug.h"

#define INITRATIO 0.95
#define AC0 0.90
#define AC1 0.44
#define AC2 0.06
#define AC3 0.10
#define AC4 0.10

#define PT1 0.15
#define PT2 0.52
#define PT3 1.00

#define LAC1 (log(AC1))
#define LAC2 (log(AC2))

#define STEPSPERCEL 0.01

#define TABLIMIT 4096			/* simple table lookup for log. */
#define TABSHIFT 19
#define TABMASK 0xfff
#define TABOFFSET 0x40000
#define RANDFACT (1.0 / MAXINT)
#define CHANCE(n) (!(RAND % n))

/* #define START_ITER 81  if you change this one, */
			/* do the same in utemp    */

/* Note: If you change definitions here, check newtemp.c */
#define ACCEPTDAMPFACTOR  0.025  /* damping factor for acceptance rate*/
#define ACCEPTDAMPFACTOR2  0.25  /* damping factor for low temp's */
#define HIGHTEMP  23.0   /* (15.0) end of high temperature regime */
#define MEDTEMP   81.0   /* (52.0) end of range limiter regime */
#define TURNOFFT 125.0    /* (100) iterationG of negative feedback turnoff */
#define LASTTEMP       155.00 /* last iterationG */
#define NUMTUPDATES   400 /* maximum number of T updates per iterationG */

#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

/* global variables */
BOOL fences_existG ;
DOUBLE avg_timeG ; /* average random time penalty */
DOUBLE avg_funcG ; /* average random wirelength penalty */
DOUBLE start_timeG ;/* initial target value for the time penalty */
DOUBLE end_timeG ;  /* final target value for the time penalty - obviously zero */
DOUBLE ratioG = 1.0 ;
DOUBLE finalRowControlG ;
DOUBLE initialRowControlG ;

/* global references */
extern INT totalRG ;
extern INT minxspanG ;
extern BOOL pairtestG ;
extern BOOL no_feed_estG ;
extern BOOL good_initial_placementG ;

/* function calls */
DOUBLE expected_svalue() ;
DOUBLE expected_value() ;
DOUBLE partition() ;
DOUBLE compute_and_combination() ;
DOUBLE combination() ;
INT eval_ratio() ;

/* static variables */
static INT acc_cntS = 0 ;
static INT move_cntS = 0 ;
static INT f_cntS = 0;
static INT cost_vectorS[13] = { 0,0,0,0,0,0,0,0,0,0,0,0,0 } ;
static BOOL not_doneS = TRUE ;
static BOOL first_fdsS = TRUE ;
static BOOL first_capS = TRUE ;
static BOOL called_rowconS = FALSE ;
static BOOL P_limit_setS = FALSE ;
static DOUBLE xadjustmentS,xalS,min_xalphaS,max_xalphaS;/* x control */
static DOUBLE yadjustmentS,yalS,min_yalphaS,max_yalphaS;/* y control */
static DOUBLE tauXS, tauYS ; /* exp. decay time constants for window */
static DOUBLE stepS ;
static DOUBLE num_penalS ;
static DOUBLE avg_rowpenalS ;
static DOUBLE f_meanS = 0.0;
static DOUBLE total_stepS ;
static DOUBLE fp_ratioS = 1.0 ;
static DOUBLE log_tabS[TABLIMIT];
static DOUBLE bin_capS = 99.0 ;
static DOUBLE row_capS = 99.0 ;
static DOUBLE a_ratioS;
static DOUBLE total_costS;

init_uloop()
{
    not_doneS = 1;
    acc_cntS = move_cntS ;
    ratioG = 1.0;
} /* end init_uloop */


uloop()
{

FENCEBOXPTR fence ;
CBOXPTR acellptr, bcellptr ; 
BBOXPTR ablckptr , bblckptr ;
INT flips , rejects , do_single_cell_move , bit_class ;
INT axcenter , bxcenter , bycenter ; 
INT aorient , borient ;
INT blk , pairflips ;
INT i , j , t , count , swaps, index, shift ;
INT abin , bbin , fds , done , single_swap ;
DOUBLE target_row_penalty ;
DOUBLE target_bin_penalty ;
DOUBLE temp , percent_error ;
DOUBLE dCp, delta , gswap_ratio ;
INT m1,m2, trials ;
INT num_accepts , gate_switches , gate_attempts ;
INT last_flips , delta_func , delta_time ;
INT temp_timer, time_to_update ; /* keeps track of when to update T */
DOUBLE iter_time, accept_deviation, calc_acceptance_ratio() ;
DOUBLE num_time, num_func ;
DOUBLE calc_time_factor() ; 
/* 
    commented out variables 
    INT reset_T ;
    DOUBLE old_T ;
*/


attemptsG  = 0 ;
flips     = 0 ;
rejects   = 0 ;
pairflips = 0 ;
earlyRejG  = 0 ;
Rej_errorG = 0 ;

G( reset_heat_index() ) ;
potential_errorsG = 0 ;
error_countG = 0 ;
if( !P_limit_setS || iterationG <= 0 ) {
    P_limitG = 999999;
    if( iterationG > 0 ) {
	P_limit_setS = TRUE ;
    }
} else {
    if( wire_chgsG > 0 ) {
	mean_wire_chgG = total_wire_chgG / (DOUBLE) wire_chgsG ;
	if( iterationG > 1 ) {
	    sigma_wire_chgG = sqrt( sigma_wire_chgG / (DOUBLE) wire_chgsG);
	} else {
	    sigma_wire_chgG = 3.0 * mean_wire_chgG ;
	}
    } else {
	mean_wire_chgG  = 0.0 ;
	sigma_wire_chgG = 0.0 ;
    }
    P_limitG = mean_wire_chgG + 3.0 * sigma_wire_chgG + TG ;
    if (P_limitG > 999999) P_limitG = 999999;
}

sigma_wire_chgG = 0.0 ;
total_wire_chgG = 0.0 ;
wire_chgsG = 0 ;
m1 = m2 = 1;
dCp = 0.0;


fds = reconfig() ;

avg_rowpenalS = 0.0 ;
num_penalS = 0.0 ;
if( iterationG < 0 ) {
    avg_timeG = 0.0 ;
    num_time = 0.0 ;
    avg_funcG = 0.0 ;
    num_func = 0.0 ;
}

/* number of moves before temperature update */
time_to_update = attmaxG / NUMTUPDATES ;
if( time_to_update <= 14 ) {
    time_to_update = 14 ;
}
temp_timer = 0 ; /* initialize timer */
num_accepts = 0 ;
last_flips = 0 ;

gate_switches = 0 ;
gate_attempts = 0 ;

/* ------------------------------------------------------------------------
        The back bone of the program the "while-loop" begins from here ...
   ------------------------------------------------------------------------ */ 
while( attemptsG < attmaxG ) {

    /* ------------- pick up a number at random --------------- */
    aG = PICK_INT( 1 , numcellsG - extra_cellsG ) ;

    /* ------------- get the structure for cell# aG ------------- */
    acellptr = carrayG[ aG ] ;
    ablockG   = acellptr->cblock ;
    axcenter = acellptr->cxcenter ;

/* ------------------------------------------------------------------------
    If two cells have the same swap_group then parts of the cells are
    interchangable. Below we check if the cell#aG belongs to a swap_group
   ------------------------------------------------------------------------ */
    if( acellptr->num_swap_group > 0 ) {
	INT sgroup;
	SGLISTPTR sglistptr;
	i = PICK_INT( 0 , acellptr->num_swap_group ) ;
	sglistptr = acellptr->swapgroups + i;
	sgroup = sglistptr->swap_group;
	

	trials = 0 ;   /*--- number of max trials (heuristically)=50 ---*/
	do {
	 /* -----------------------------------------------------------------
            pick_position picks up a new position for the cell#aG, it returns
	    the bxcenter and blk where the cell could be moved ...
            -----------------------------------------------------------------*/
	    pick_position(&bxcenter,&blk,axcenter,ablockG,8.0);

	    bbin = SetBin( bxcenter ) ;
	 /* -----------------------------------------------------------------
            the field "cell" in binptrG[blk][bbin]->cell contains a list of 
	    all the cells in that bin. But cell[0] contains the total number
	    of cells in the bin (hard-coded) isn't it ...
	    -----------------------------------------------------------------*/
	    cellbptrG = binptrG[blk][bbin]->cell ;

	    if( *cellbptrG > 0 ) {
		if( Equal_Width_CellsG ){
		    BpostG = 1 ;
		} else {
		    BpostG = PICK_INT( 1 , *cellbptrG ) ;
		}
		bG = cellbptrG[ BpostG ] ;
		bcellptr  = carrayG[bG] ;
	    } else {
		bG = 0 ;
	    }
	    if( bG != 0 && aG != bG ) {
		for (j = 0; j < bcellptr->num_swap_group; j++) {
		    sglistptr = bcellptr->swapgroups + j;
		    if (sglistptr->swap_group == sgroup)
			break;
		}
		if (j < bcellptr->num_swap_group) {
		    break ;
		} else {
		    trials++ ;
		}
	    } else {
		trials++ ;
	    }
	} while( trials <= 50 ) ;

	if( trials <= 50 ) {
	    for( swaps = 1 ; swaps <= 4 ; swaps++ ) {
	      /* -- gate_swap evaluates the proposed gate swaps --*/
		gate_switches += gate_swap( 1, i, j ) ;
		gate_attempts++ ;
	    }
	}

	sglistptr = acellptr->swapgroups + i;

	// If a cell has more than one pin group in the same
	// swap group, then it can permute pin groups within
	// itself.

	if( sglistptr->num_pin_group > 1 ) {
	    for( swaps = 1 ; swaps <= 4 ; swaps++ ) {
		gate_swap( 0, i, j ) ;
	    }
	}
    }
    if( acellptr->cclass < -1 ) { /*---- for rigid cells, continue ----*/
	continue ;
    }

    ablckptr = barrayG[ ablockG ] ;
    aorient = acellptr->corient ;
    abin = SetBin( axcenter ) ;
    cellaptrG = binptrG[ablockG][abin]->cell ;
    if( Equal_Width_CellsG ){
	ApostG = 1 ;
    } else {
	for( i = 1 ; i <= *cellaptrG ; i++ ) {
	    if( cellaptrG[i] == aG ) {
		 ApostG = i ;
		 break ;
	    }
	}
    }

    /* 
     *  select block for cell a to be placed in 
     */
    if( acellptr->fence == NULL ) { /* cclass -1 cells won't enter here */
	bblockG = 0 ;
	for (i=0; i<10; i++) {
	    pick_position(&bxcenter,&blk,axcenter,ablockG,1.0);
	    bblckptr = barrayG[ blk ] ;
	    /* if cell "a" can't legally enter this row, keep looking */
	    if( ablockG == blk || acellptr->cclass == 0 ) {
		/*  cell "a" will remain in the same row, or:  */
		/*  cell "a" can go anywhere it pleases  */
		break ;
	    } else if( acellptr->cclass > 0 ) {  /*  it cannot go anywhere it pleases  */
		bit_class = 1 ;
		index = (bblckptr->bclass - 1) / 32 ;
		shift = bblckptr->bclass - 32 * index ;
		bit_class <<= (shift - 1) ;
		if( bit_class & acellptr->cbclass[index] ) {
		    /*  "a" is allowed in a row with this block_class */
		    break ;
		}  /* else, keep searching for a legal row  */
	    }
	}
    } else {
	/* 
	 *  select block for cell a to be placed in 
	 */
	bblockG = 0 ;
	for (i=0; i<10; i++) {
	    fence = acellptr->fence ;
	    if( fence->next_fence != NULL ) {
		count = 0 ;
		for( ; fence; fence = fence->next_fence){
		    count++ ;
		}
		j = PICK_INT( 1 , count ) ;
		count = 0 ;
		fence = acellptr->fence ;
		for( ; fence; fence = fence->next_fence ) {
		    if( ++count == j ) {
			break ;
		    }
		}
	    }
	    pick_fence_position(&bxcenter,&blk,fence) ;
	    bblckptr = barrayG[ blk ] ;

	    /* if cell "a" can't legally enter this row, keep looking */
	    if( ablockG == blk || acellptr->cclass <= 0 ) {
		/*  cell "a" will remain in the same row, or:  */
		/*  cell "a" can go anywhere it pleases  */
		break ;
	    } else if( acellptr->cclass > 0 ) {  /*  it cannot go anywhere it pleases  */
		bit_class = 1 ;
		index = (bblckptr->bclass - 1) / 32 ;
		shift = bblckptr->bclass - 32 * index ;
		bit_class <<= (shift - 1) ;
		if( bit_class & acellptr->cbclass[index] ) {
		    /*  "a" is allowed in a row with this block_class */
		    break ;
		}  /* else, keep searching for a legal row  */
	    }
	}
    }/* end else -- acellptr->fence != NULL -- cclass -1 cells won't enter here */

    if (i == 10) continue;  /*-- get out of the while-loop --*/

    /*------- Now get the target block's structure and target bin -------*/
    bblockG = blk;
    bycenter = bblckptr->bycenter ;

    bbin = SetBin( bxcenter ) ;
    cellbptrG = binptrG[bblockG][bbin]->cell ;

    if( *cellbptrG > 0 ) {
	count = 0 ;

get_b:  if( Equal_Width_CellsG ){
	    BpostG = 1 ;
	} else {
	    BpostG = PICK_INT( 1 , *cellbptrG ) ;
	}
	bG = cellbptrG[ BpostG ] ;
	if( aG == bG ) {
	    continue ;
	}
	bcellptr  = carrayG[bG] ;

	if( fences_existG ) {
	    done = 0 ;
	    if( (fence = bcellptr->fence) != NULL ) {
		while(1) {
 /*------- make sure cell aG's block is outside the block bG's fence -------*/ 
		    if( ablockG >= fence->min_block && 
					ablockG <= fence->max_block &&
					axcenter >= fence->min_xpos &&
					axcenter <= fence->max_xpos ) {
			done = 1 ;
			break ;
		    } else {
			fence = fence->next_fence ;
			if( fence == NULL ) {
			    if( ++count < *cellbptrG ) {
				goto get_b ;
			    } else {
				break ;
			    }
			}
		    }
		}
		if( !done ) {
		    continue ;
		}
	    }
	}
	if( bcellptr->cclass < -1 ) {
	    do_single_cell_move = 1 ;
	} else if( ablockG != bblockG && bcellptr->cclass > 0 ) {
	    bit_class = 1 ;
	    index = (ablckptr->bclass - 1) / 32 ;
	    shift = ablckptr->bclass - 32 * index ;
	    bit_class <<= (shift - 1) ;
	    if( !(bit_class & bcellptr->cbclass[index]) ) {
		/*  "b" is not allowed in a row with this block_class */
		continue ;
	    }
	    do_single_cell_move = 0 ;
	    borient  = bcellptr->corient ;
	} else {
	    do_single_cell_move = 0 ;
	    borient  = bcellptr->corient ;
	}
    } else {
	do_single_cell_move = 1 ;
    }

    delta_func = funccostG ;
    delta_time = timingcostG ;

    if( do_single_cell_move ) {
	if( Equal_Width_CellsG ) {
	    continue ;
	}
	/*
	reset_T = 0 ;
	if( acellptr->fence != NULL ) {
	    if( ablockG < acellptr->fence->min_block ||
				ablockG > acellptr->fence->max_block ) {
		reset_T = 1 ;
		old_T = T ;
		T = 1000000.0 ;
	    }
	}
	*/
	if( ablckptr->borient == 1 ) {
	    if( bblckptr->borient == 1 ) {
		t = ucxx1( bxcenter, bycenter) ;
		if( t != 1 ) {
		    rejects++ ;
		    if( rejects % 6 == 0 && acellptr->orflag != 0){
			uc0( aG , (aorient == 0) ? 2 : 0 );
		    }
		} else {  /* if( t == 1 ) */
		    flips++ ;
		    acc_cntS ++;
		}
	    } else {  /* bblckptr->borient == 2 */
		t = ucxxo1( bxcenter,bycenter,(aorient == 0) ? 1 : 3 ) ;
		if( t != 1 ) {
		    rejects++ ;
		    if( rejects % 6 == 0 && acellptr->orflag != 0){
			uc0( aG , (aorient == 0) ? 2 : 0 );
		    }
		} else {  /* if( t == 1 ) */
		    flips++ ;
		    acc_cntS ++;
		}
	    }
	} else {  /* ablockGptr->borient == 2 */	
	    if( bblckptr->borient == 1 ) {
		t = ucxxo1( bxcenter, bycenter, (aorient == 1) ? 0 : 2) ;
		if( t != 1 ) {
		    rejects++ ;
		    if( rejects % 6 == 0 && acellptr->orflag != 0){
			uc0( aG , (aorient == 1) ? 3 : 1 );
		    }
		} else {  /* if( t == 1 ) */
		    flips++ ;
		    acc_cntS ++;
		}
	    } else {  /*  bblckptr->borient == 2 */
		t = ucxx1( bxcenter, bycenter) ;
		if( t != 1 ) {
		    rejects++ ;
		    if( rejects % 6 == 0 && acellptr->orflag != 0){
			uc0( aG , (aorient == 1) ? 3 : 1 );
		    }
		} else {  /* if( t == 1 ) */
		    flips++ ;
		    acc_cntS ++;
		}
	    }
	}
	/*
	if( reset_T ) {
	    T = old_T ;
	}
	*/
    } else {
	/*   pairwise interchange */
	if( ablckptr->borient == 1 ) {
	    if( bblckptr->borient == 1 ) {
		t = ucxx2() ;
		if( t != 1 ) {
		    rejects++ ;
		    if( rejects % 6 == 0 && acellptr->orflag != 0){
			uc0( aG , (aorient == 0) ? 2 : 0 );
		    }
		} else {  /* if( t == 1 ) */
		    pairflips++ ;
		    acc_cntS ++;
		}
	    } else {  /* bblockG->orient == 2 */
		t = ucxxo2( (aorient == 0) ? 1:3, (borient == 1) ? 0:2);
		if( t != 1 ) {
		    rejects++ ;
		    if( rejects % 6 == 0 && acellptr->orflag != 0){
			uc0( aG , (aorient == 0) ? 2 : 0 );
		    }
		} else {  /* if( t == 1 ) */
		    pairflips++ ;
		    acc_cntS ++;
		}
	    }
	} else { /* ablockG->borient == 2 */
	    if( bblckptr->borient == 1 ) {
		t = ucxxo2( (aorient == 1) ? 0:2, (borient == 0) ? 1:3);
		if( t != 1 ) {
		    rejects++ ;
		    if( rejects % 6 == 0 && acellptr->orflag != 0){
			uc0( aG , (aorient == 1) ? 3 : 1 );
		    }
		} else {  /* if( t == 1 ) */
		    pairflips++ ;
		    acc_cntS ++;
		}
	    } else { /* bblockG->borient == 2 */  
		t = ucxx2( ) ;
		if( t != 1 ) {
		    rejects++ ;
		    if( rejects % 6 == 0 && acellptr->orflag != 0){
			uc0( aG , (aorient == 1) ? 3 : 1 );
		    }
		} else {  /* if( t == 1 ) */
		    pairflips++ ;
		    acc_cntS ++;
		}
	    }
	}
    }

    num_penalS += 1.0 ;
    avg_rowpenalS = (avg_rowpenalS * (num_penalS - 1.0) + 
			(DOUBLE) rowpenalG) / num_penalS ;
    
    attemptsG++ ;

    /* draw the data */
    G( check_graphics(FALSE) ) ;

    if (iterationG <= 0) {

	if( iterationG == 0 )  continue;
	
	/* calculate a running average of (delta) timing penalty */
	delta_time = abs( delta_time - timingcostG ) ;
	if( delta_time != 0 ) {
	    num_time += 1.0 ;
	    avg_timeG = (avg_timeG * (num_time - 1.0) + 
			    (DOUBLE) delta_time) / num_time ;
	
	    /* calculate a running average of (delta) wirelength penalty */
	    delta_func = abs( delta_func - funccostG ) ;
	    num_func += 1.0 ;
	    avg_funcG = (avg_funcG * (num_func - 1.0) + 
				(DOUBLE) delta_func) / num_func ;
	}


	if (d_costG >= 0){
	    m1 ++;	/* d_cost is the -ve of the actual d_cost */
	} else {
	    dCp -= d_costG;
	    m2 ++;
	}
	temp = (INITRATIO * attemptsG - m1) / m2;
	if (temp <= 0.0) {
	    TG *= 0.9;
	} else {
	    TG = -dCp / (m2 * log(temp));
	}
	continue;		/* initialization phase */
    }

    /* ----------------------------------------------------------------- 
       Update temperature using negative feedback to control the
       acceptance ratio in accordance to curve fit schedule.  
       Calc_acceptance_ratio returns desired acceptance ratio give
       the iterationG.  The damped error term (deviation) is then applied 
       to correct the temperature T.  Update_window_size controls the 
       range limiter.  We avoid updating T during initialization, 
       we use exact schedule to compute starting T.  The temp_timer 
       allows us to update temperature inside the inner loop
       of the annealing algorithm. We use counter to avoid use of mod
       function for speed.
     ------------------------------------------------------------------ */
    num_accepts += pairflips + flips - last_flips ;
    last_flips = pairflips + flips ;

    if( ++temp_timer >= time_to_update || 
			(attemptsG >= attmaxG && temp_timer >= 50) ) {
	ratioG = ((DOUBLE)(num_accepts)) / (DOUBLE) temp_timer ;
	temp_timer = 0 ; /* reset counter */
	num_accepts = 0 ;
	iter_time = (DOUBLE) iterationG +
		    (DOUBLE) attemptsG / (DOUBLE) attmaxG ;
	accept_deviation = 
	    (calc_acceptance_ratio( iter_time ) - ratioG ) ;
	if( (DOUBLE) iterationG < TURNOFFT ) {
	    accept_deviation *= ACCEPTDAMPFACTOR ; 
	} else {
	    accept_deviation *= ACCEPTDAMPFACTOR2 ;
	}
	TG *= 1.0 + accept_deviation ;
	update_window_size( (DOUBLE) iterationG +
			    (DOUBLE) attemptsG / (DOUBLE) attmaxG ) ;
    }


}   /* end of inner loop */

D( "uloop",
    check_cost() ;
) ;
f_meanS = 0.0;
f_cntS = 0;

if( ratioG < AC3 ){
    if( iterationG >= LASTTEMP || (tw_frozen( funccostG )  &&
				(iterationG >= LASTTEMP - 5)) ) {
	pairtestG = TRUE ;
    }
}

if( potential_errorsG > 0 ) {
    percent_error = (DOUBLE) error_countG / (DOUBLE) potential_errorsG ;
} else {
    percent_error = 0.0 ;
}
percent_error *= 100.0 ;

if( pairflips > 0.0001 ) {
    fp_ratioS = (DOUBLE)flips/(DOUBLE)pairflips ;
} else {
    fp_ratioS = 1.0 ;
}

ratioG = ((DOUBLE)(pairflips+flips)) / attemptsG;
if(iterationG >= 0 || good_initial_placementG ) {
    if( !gate_arrayG ) {
	sprintf(YmsgG,"%3d: %6.2le %6ld %-8ld %-6ld %-8ld",
	    iterationG,TG,fds,funccostG,rowpenalG,timingcostG);
	M( MSG, NULL, YmsgG ) ;
    } else {
	sprintf(YmsgG,"%3d: %6.2le %6ld %-8ld %-6ld %-8ld",
	    iterationG,TG,fds,funccostG,rowpenalG,timingcostG);
	M( MSG, NULL, YmsgG ) ;
    }
    sprintf(YmsgG,"%6ld %4.2lf %4.2lf %5.2lf %5.2lf ",
	P_limitG,percent_error/100.0,binpenConG,roLenConG,timeFactorG);
    M( MSG, NULL, YmsgG ) ;
    if( swappable_gates_existG ) {
	if( gate_attempts > 0 ) {
	    gswap_ratio = (DOUBLE) gate_switches / (DOUBLE) gate_attempts ;
	} else {
	    gswap_ratio = 0 ;
	}
	sprintf(YmsgG,"%4.2lf %4.2lf %5.3lf %4.2lf\n",
	    fp_ratioS,((DOUBLE) earlyRejG)/attemptsG,ratioG,
	    gswap_ratio );
	M( MSG, NULL, YmsgG ) ;

    } else {
	sprintf(YmsgG,"%4.2lf %4.2lf %5.3lf\n",
	    fp_ratioS,((DOUBLE) earlyRejG)/attemptsG,ratioG );
	M( MSG, NULL, YmsgG ) ;
    }
    Ymessage_flush();
}


if( !called_rowconS && !(Equal_Width_CellsG)) {
    rowcon() ;
    called_rowconS = TRUE ;
} 

/*  A Negative-Feedback Approach */
if( not_doneS || good_initial_placementG ) {
    not_doneS = eval_ratio(&fraction_doneG);
    if (first_capS && (!first_fdsS)) {
	first_capS = 0;
    }

    if( Equal_Width_CellsG ){
	/*----------------- No row or bin penalty -----------------*/
	target_bin_penalty = 0 ; 
	target_row_penalty = 0 ;
	binpenConG = 0 ;
	roLenConG = 0 ;
	penaltyG = 0 ;
	/*---------------------------------------------------------*/
    } else {

	target_bin_penalty = (1.25 - 1.00 * fraction_doneG) * (DOUBLE)totalRG ;
	target_row_penalty = (initialRowControlG - 
		    (initialRowControlG - finalRowControlG) * 
		    (fraction_doneG) ) * (DOUBLE) totalRG ;
	binpenConG += ((DOUBLE) binpenalG - target_bin_penalty) / 
						(DOUBLE) totalRG ;
	binpenConG = (binpenConG > 1.0) ? binpenConG : 1.0 ;
	if( good_initial_placementG ) {
	    binpenConG = (binpenConG > 4.0) ? 4.0 : binpenConG ;
	}
	delta = ((DOUBLE) rowpenalG - target_row_penalty) / 
						target_row_penalty ;
	if( fraction_doneG > PT1 && fraction_doneG < PT2 ) {
	    roLenConG += (  (fraction_doneG - PT1) / (PT2 - PT1)  ) * delta;
	} else if( fraction_doneG <= PT1 && roLenConG + delta > 6.0 ) {
	    roLenConG = 6.0 ;
	} else {
	    roLenConG += delta ;
	    if( good_initial_placementG ) {
		roLenConG = (roLenConG > 5.0) ? 5.0 : roLenConG ;
	    }
	}
	roLenConG = (roLenConG > 1.0) ? roLenConG : 1.0 ;
	penaltyG = (INT)( binpenConG * (DOUBLE) binpenalG + 
				    roLenConG * (DOUBLE) rowpenalG ) ;
    } /* end !Equal_Width_CellsG */

    timeFactorG = calc_time_factor() ;
}

return ;
} /* end uloop */





rowcon()
{

INT C , R , p_first , totalCells , cellsPerRow , temp_R ;
INT over, under ;
DOUBLE states , value , expect , variance ; 
DOUBLE expectedExtraRowLength , rowControl , x , minDev ;

if( numcellsG > 5000 ) {
    rowControl = 0.008 ;
    /* rowControl = 0.012 ; */

    initialRowControlG =  6.0 * rowControl ;
    /* initialRowControlG =  5.5 * rowControl ; */
    finalRowControlG   =  rowControl ;
} else {
    totalCells = numcellsG - extra_cellsG ;
    /*
    totalCells = 0 ;
    for( i = 1 ; i <= numcellsG - extra_cellsG ; i++ ) {
	if( carrayG[i]->cclass >= 0 ) {
	    totalCells++ ;
	}
    }
    */
    R = numRowsG ;

    cellsPerRow = totalCells / R ;

    temp_R = R - R / 3 ;
    rowControl = 0.0 ;
    do {
	rowControl += 0.001 ;
	C = (INT)( rowControl * (DOUBLE) totalCells / 2.0 ) ;
	if( 2.0 * (DOUBLE)(C+1) - rowControl * totalCells <=
			rowControl * totalCells - 2.0 * (DOUBLE) C ) {
	    C++ ;
	}
    } while( 2 * C < temp_R ) ;

    p_first = C / R ;
    if( p_first * R < C ) {
	p_first++ ;
    }
    R -= R / 3 ;

    under = 0 ;
    over  = 0 ;
    for( ; ; ) {
	value = expected_value( C , 0 , p_first , R ) ;
	states = combination( C+R-1, C ) ;
	expect = value / states ;
	expectedExtraRowLength = 100.0 * expect / (DOUBLE) cellsPerRow ;
	value = expected_svalue( C , 0 , p_first , R ) ;
	variance = value / states - (expect * expect) ;
	x = 100.0 * sqrt( variance ) / (DOUBLE) cellsPerRow ; 
	minDev = 100.0 / (DOUBLE) cellsPerRow ;
	value = x+expectedExtraRowLength - minDev ;

	if( !resume_runG ) {
	    fprintf(fpoG,"rowControl:%6.3f     ", rowControl ) ;
	    fprintf(fpoG,"expected deviation above minimum:%6.2f\n",value);
	}
	if( value > 2.7 && value < 3.3 ) {
	    rowControl += 0.001 ;
	    break ;
	} else if( value <= 2.7 ) {
	    rowControl += 0.001 ;
	    if( !over ) {
		under = 1 ;
	    } else {
		break ;
	    }
	} else if( value >= 3.3 ) {
	    if( !under ) {
		rowControl -= 0.001 ;
		over = 1 ;
	    } else {
		break ;
	    }
	}
	C = (INT)( rowControl * (DOUBLE) totalCells / 2.0 ) ;
	if( 2.0 * (DOUBLE)(C+1) - rowControl * totalCells <=
			rowControl * totalCells - 2.0 * (DOUBLE) C ) {
	    C++ ;
	}
	p_first = C / R ;
	if( p_first * R < C ) {
	    p_first++ ;
	}
    }

    initialRowControlG =  5.5 * rowControl ;
    /* initialRowControlG =  5.5 * rowControl ; */
    finalRowControlG   =  rowControl ;
}
return ;
}


DOUBLE partition( C_initial , k_initial , p_initial , R_initial )
INT C_initial , k_initial , p_initial , R_initial ;
{

INT R , C , k , p , k_limit , p_limit ;
DOUBLE states , equivs ;

states = 0.0 ;
R = k_limit = R_initial - k_initial ;
C = C_initial - p_initial * k_initial ;

for( k = 1 ; k <= k_limit ; k++ ) {
    equivs = combination( R , k ) ;
    p_limit = C / k ;
    for( p = p_initial ; p <= p_limit ; p++ ) {
	if( C - (p-1)*R > k ) { ;
	    continue ;
	}
	states += equivs * (compute_and_combination( C , k , p , R ) -
					partition( C , k , p , R ) ) ;
	continue ;
    }
}

return( states ) ;
}


DOUBLE expected_value( C_initial , k_initial , p_initial , R_initial )
INT C_initial , k_initial , p_initial , R_initial ;
{

INT R , C , k , p , k_limit , p_limit ;
DOUBLE value , equivs ;

value = 0.0 ;
R = k_limit = R_initial - k_initial ;
C = C_initial - p_initial * k_initial ;

for( k = 1 ; k <= k_limit ; k++ ) {
    equivs = combination( R , k ) ;
    p_limit = C / k ;
    for( p = p_initial ; p <= p_limit ; p++ ) {
	if( C - (p-1)*R > k ) { ;
	    continue ;
	}
	value += (DOUBLE) p * equivs *
			(compute_and_combination( C , k , p , R ) -
				    partition( C , k , p , R ) ) ;
    }
}

return( value ) ;
}


DOUBLE expected_svalue( C_initial , k_initial , p_initial , R_initial )
INT C_initial , k_initial , p_initial , R_initial ;
{

INT R , C , k , p , k_limit , p_limit ;
DOUBLE value , equivs ;

value = 0.0 ;
R = k_limit = R_initial - k_initial ;
C = C_initial - p_initial * k_initial ;

for( k = 1 ; k <= k_limit ; k++ ) {
    equivs = combination( R , k ) ;
    p_limit = C / k ;
    for( p = p_initial ; p <= p_limit ; p++ ) {
	if( C - (p-1)*R > k ) { ;
	    continue ;
	}
	value += (DOUBLE)(p * p) * equivs *
			(compute_and_combination( C , k , p , R ) -
				    partition( C , k , p , R ) ) ;
    }
}

return( value ) ;
}


DOUBLE compute_and_combination( C , k , p , R )
INT C , k , p , R ;
{

INT numerator , denom1 , denom2 , temp ;
DOUBLE states ;

states = 1.0  ;
numerator = C - k*p + R - k - 1 ;
denom1 = C - k*p ;
denom2 = R - k - 1 ;
if( denom1 > denom2 ) {
    temp = denom1 ;
    denom1 = denom2 ;
    denom2 = temp ;
}
for( ; numerator > denom2 ; numerator-- , denom1-- ) {
    states *= (DOUBLE) numerator ;
    states /= (DOUBLE) denom1 ;
}

return( states ) ;
}


DOUBLE combination( numerator , denominator )
INT numerator , denominator ;
{

DOUBLE states ;
INT temp , denom1 , denom2 ;

states = 1.0  ;

denom1 = denominator ;
denom2 = numerator - denominator ;
if( denom1 > denom2 ) {
    temp = denom1 ;
    denom1 = denom2 ;
    denom2 = temp ;
}
for( ; numerator > denom2 ; numerator-- , denom1-- ) {
    states *= (DOUBLE) numerator ;
    states /= (DOUBLE) denom1 ;
}

return( states ) ;
}


sanity_check()
{

INT *cellxptr , cell , center , block , bin , i ;

for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    center = carrayG[cell]->cxcenter ;
    block  = carrayG[cell]->cblock   ;
    bin = SetBin( center ) ;
    cellxptr = binptrG[block][bin]->cell ;
    for( i = 1 ; i <= *cellxptr ; i++ ) {
	if( cellxptr[i] == cell ) {
	     break ;
	}
    }
    if( i > *cellxptr ) {
	return(cell);
    }
}
return(0);
}


sanity_check2()
{

INT *cellxptr , *clist ;
INT block , bin , cell , i ;

clist = (INT *) Ysafe_malloc( (1 + numcellsG - extra_cellsG ) * sizeof( INT ) ) ;
for( i = 1 ; i <= numcellsG - extra_cellsG ; i++ ) {
    clist[i] = 0 ;
}

for( block = 1 ; block <= numRowsG ; block++ ) {
    for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	cellxptr = binptrG[block][bin]->cell ;
	for( i = 1 ; i <= *cellxptr ; i++ ) {
	    cell = cellxptr[i] ;
	    if( clist[cell] == 1 ) {
		printf("cell %d appears twice in clist\n",cell);
		fflush(stdout);
		return(cell);
	    } else {
		clist[cell] = 1 ;
	    }
	}
    }
}
for( i = 1 ; i <= numcellsG - extra_cellsG ; i++ ) {
    if( clist[i] == 0 ) {
	printf("cell %d appears NEVER in clist\n",i);
	fflush(stdout);
	return(i);
    }
}

Ysafe_free( clist ) ;
return(0);
}


sanity_check3()
{

INT *cellxptr ;
INT block , bin , cell , i ;

for( block = 1 ; block <= numRowsG ; block++ ) {
    for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	cellxptr = binptrG[block][bin]->cell ;
	for( i = 1 ; i <= *cellxptr ; i++ ) {
	    cell = cellxptr[i] ;
	    if( carrayG[cell]->cblock != block ) {
		printf("cell %d has inconsistent block\n",cell);
		fflush(stdout);
		return(cell);
	    }
	    if( SetBin( carrayG[cell]->cxcenter ) != bin ) {
		printf("cell %d has inconsistent bin\n",cell);
		fflush(stdout);
		return(cell);
	    }
	}
    }
}

return(0);
}

/* new evaluate ratio is a linear function of iterationG */
INT eval_ratio( t )
DOUBLE *t;
{
    if( iterationG >= TURNOFFT ){
	*t = 1.0 ;
    } else if( iterationG < 0 ){
	*t = 0.0 ;
    } else {
	*t = (DOUBLE) iterationG / TURNOFFT ;
    }
    return((ratioG < AC4) ? 0 : 1);
}

init_control(first)
INT first;
{
    INT i;
    DOUBLE tmp ;

    /* initialize move generation parameters */
    min_xalphaS = 0.5 * minxspanG;	/* average min. window size */
    min_yalphaS = 0.75;
    if( intelG ) {
	min_yalphaS = 1.5;
    }

    max_xalphaS = blkxspanG;	/* average max. window size */
    max_yalphaS = numRowsG;

    total_stepS = STEPSPERCEL * (numcellsG - extra_cellsG) ;
    xadjustmentS = (max_xalphaS - min_xalphaS) / total_stepS;
    yadjustmentS = (max_yalphaS - min_yalphaS) / total_stepS;

    if( first == 1 ) {
	/* stepS = (init_ratioG >= 0.44) ? 0.0 : 1.0; */
	xalS = max_xalphaS;
	yalS = max_yalphaS;
    }
    /* determine tauXS & tauYS - exponential decay of range limiter */
    tauXS = - log( (min_xalphaS/max_xalphaS) ) / (MEDTEMP - HIGHTEMP) ;
    tauYS = - log( (min_yalphaS/max_yalphaS) ) / (MEDTEMP - HIGHTEMP) ;

    /* prepare lookup table */
    for (i=0; i<TABLIMIT; i++){
	tmp = ((i << TABSHIFT) + TABOFFSET) * RANDFACT;
	log_tabS[i] = log( tmp ) ;
    }
}



pick_fence_position(x,y,fence)
INT *x, *y ;
FENCEBOX *fence ;
{
    register INT left,right;
    BBOXPTR bblckptr ;

    *y = PICK_INT( fence->min_block , fence->max_block ) ;
    bblckptr = barrayG[*y] ;
    
    left = fence->min_xpos ;
    if( left < bblckptr->bxcenter + bblckptr->bleft ) {
	left = bblckptr->bxcenter + bblckptr->bleft ;
    }
    right = fence->max_xpos ;
    if( right > bblckptr->bxcenter + bblckptr->bright ) {
	right = bblckptr->bxcenter + bblckptr->bright ;
    }
    *x = PICK_INT( left , right ) ;
    return;
}

pick_position(x,y,ox,oy,scale)
INT *x,*y,ox,oy;
DOUBLE scale ;
{
    register INT i,m,n,bleft,bright;
    DOUBLE tmp ;
    BBOXPTR bblckptr ;

    m = RAND;
    tmp = yalS * scale ;
    if( tmp > max_yalphaS ) {
	tmp = max_yalphaS ;
    }
    n = -tmp * log_tabS[(m >> TABSHIFT) & TABMASK] + 0.5;
    if (m & 0x10000) n = -n;
    n += oy;
    if (n < 1) n = PICK_INT(1,oy);
    else if (n > numRowsG) n = PICK_INT(oy,numRowsG);
    *y = n;
    bblckptr = barrayG[n] ;
    for (i=0; i<2; i++) {
	m = RAND;
	tmp = xalS * scale ;
	if( tmp > max_xalphaS ) {
	    tmp = max_xalphaS ;
	}
	n = -tmp * log_tabS[(m >> TABSHIFT) & TABMASK];
	if (m & 0x10000) n = -n;
	n += ox;
	bleft = bblckptr->bxcenter + bblckptr->bleft ;
	bright = bblckptr->bxcenter + bblckptr->bright ;
	if (n < bleft || n > bright) continue;
	*x = n;
	return;
    }
    if (n < bleft) {
	if (ox > bright) ox = bright;
	else if (ox < bleft) ox = bleft;
	n = PICK_INT(bleft,ox);
    } else if (n > bright) {
	if (ox < bleft) ox = bleft;
	else if (ox > bright) ox = bright;
	n = PICK_INT(ox,bright);
    }
    *x = n;
}

/* change range limiter according to iterationG number */
update_window_size( iternum )
DOUBLE iternum ;
{

/*
    commented out variables 
    CBOXPTR ptr ;
    INT cell ;
*/

    if( iternum <= HIGHTEMP ){
	xalS = max_xalphaS ;
	yalS = max_yalphaS ;
    } else if( iternum <= MEDTEMP ){
	/* exponential decay xalS and yalS */
	/* -------------------------------------------------------- 
	    xalS = max_xalphaS * exp( - tauXS * ( iternum - HIGHTEMP ))
	    yalS = max_yalphaS * exp( - tauYS * ( iternum - HIGHTEMP ))
	   -------------------------------------------------------- */
	xalS = yalS = iternum - HIGHTEMP ;
	xalS *= - tauXS ;
	xalS = max_xalphaS * exp( xalS ) ;

	yalS *= - tauYS ;
	yalS = max_yalphaS * exp( yalS ) ;

    } else {  /* low temp */
	xalS = min_xalphaS ;
	yalS = min_yalphaS ;
    }

    /*
    for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
	ptr = carrayG[cell] ;
	if( ptr->fence != NULL ) {
	    ptr->fence->min_xpos = ptr->orig_xcenter - xalS ;
	    ptr->fence->max_xpos = ptr->orig_xcenter + xalS ;
	    if( yalS != min_yalphaS ) {
		ptr->fence->min_block = ptr->orig_row - (INT) yalS ;
		if( ptr->fence->min_block < 1 ) {
		    ptr->fence->min_block = 1 ;
		}
		ptr->fence->max_block = ptr->orig_row + (INT) yalS ;
		if( ptr->fence->max_block > numRowsG ) {
		    ptr->fence->max_block = numRowsG ;
		}
	    } else {
		ptr->fence->min_block = ptr->orig_row ;
		ptr->fence->max_block = ptr->orig_row ;
	    }
	}
    }
    */
}

save_control( fp )
FILE *fp ;
{
    fprintf(fp,"%d 0 %d\n",pairtestG,not_doneS);
    fprintf(fp,"%d %d %d %d\n",acc_cntS,move_cntS,first_fdsS,first_capS);
    fprintf(fp,"%f %f %f %f\n",stepS,xalS,yalS,a_ratioS);
    fprintf(fp,"%f %f\n",ratioG,total_costS);
    fprintf(fp,"%f %f\n",bin_capS,row_capS);
    fprintf(fp,"%f %f %f\n",avg_timeG, avg_funcG, timeFactorG);
}

read_control( fp )
FILE *fp ;
{
    INT junk ;

    fscanf(fp,"%ld %ld %ld\n",&pairtestG,&junk,&not_doneS);
    fscanf(fp,"%ld %ld %ld %ld\n",&acc_cntS,&move_cntS,&first_fdsS,&first_capS);
    fscanf(fp,"%lf %lf %lf %lf\n",&stepS,&xalS,&yalS,&a_ratioS);
    fscanf(fp,"%lf %lf\n",&ratioG,&total_costS) ;
    fscanf(fp,"%lf %lf\n",&bin_capS,&row_capS);
    fscanf(fp,"%lf %lf %lf\n",&avg_timeG, &avg_funcG, &timeFactorG);
}

tw_frozen( cost )
INT cost ;
{

DOUBLE diff , avg_first_set , avg_second_set ;
INT i ;

if( cost_vectorS[0] >= 12 ) {
    for( i = 2 ; i <= 12 ; i++ ) {
	cost_vectorS[i-1] = cost_vectorS[i] ;
    }
    cost_vectorS[12] = cost ;
    avg_first_set = (DOUBLE)(cost_vectorS[1] + cost_vectorS[2] + 
		    cost_vectorS[3] + cost_vectorS[4]) / 4.0 ;
    avg_second_set = (DOUBLE)(cost_vectorS[9] + cost_vectorS[10] + 
		    cost_vectorS[11] + cost_vectorS[12]) / 4.0 ;
    diff = avg_first_set - avg_second_set ;
    if( diff <= 0.0 ) {
	return(1) ;
    } else {
	if( diff / avg_first_set < 0.004 ) {
	    return(1) ;
	} else {
	    return(0) ;
	}
    }
} else {
    cost_vectorS[ ++(cost_vectorS[0]) ] = cost ;
    return(0) ;
}
}
