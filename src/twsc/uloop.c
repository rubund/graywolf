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
	    int eval_ratio(t)
		double *t;
	    init_uloop()
	    init_control(first)
		int first;
	    pick_position(x,y,ox,oy,scale)
		int *x,*y,ox,oy;
		double scale ;
	    update_window_size( iternum )
		double iternum ;
	    uloop()
	    rowcon()
	    double partition( C_initial,k_initial,p_initial,R_initial )
		int C_initial , k_initial , p_initial , R_initial ;
	    double expected_value(C_initial,k_initial,p_initial,R_initial)
		int C_initial , k_initial , p_initial , R_initial ;
	    double expected_svalue(C_initial,k_initial,
		p_initial,R_initial ) 
	    int C_initial , k_initial , p_initial , R_initial ; 
	    double compute_and_combination( C , k , p , R )
		int C , k , p , R ;
	    double combination( numerator , denominator )
		int numerator , denominator ;
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
#define UCXXGLB_VARS

#include <globals.h>
#include "standard.h"
#include "ucxxglb.h"
#include "main.h"
#include "readpar.h"
#include "parser.h"
#include "uloop.h"
#include "ucxx1.h"
#include "ucxxo1.h"
#include "ucxx2.h"
#include "ucxxo2.h"
#include "uc0.h"
#include "graphics.h"
#include "reconfig.h"
#include "gateswap.h"
#include "debug2.h"
#include "savewolf.h"

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
#define RANDFACT (1.0 / INT_MAX)
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

#define PICK_int(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

/* global variables */
BOOL fences_existG ;
double avg_timeG ; /* average random time penalty */
double avg_funcG ; /* average random wirelength penalty */
double start_timeG ;/* initial target value for the time penalty */
double end_timeG ;  /* final target value for the time penalty - obviously zero */
double ratioG = 1.0 ;
double finalRowControlG ;
double initialRowControlG ;

/* global references */
extern int totalRG ;
extern int minxspanG ;
extern BOOL pairtestG ;
extern BOOL no_feed_estG ;
extern BOOL good_initial_placementG ;

/* function calls */
double expected_svalue() ;
double expected_value() ;
double partition() ;
double compute_and_combination() ;
double combination() ;
int eval_ratio() ;

/* static variables */
static int acc_cntS = 0 ;
static int move_cntS = 0 ;
static int f_cntS = 0;
static int cost_vectorS[13] = { 0,0,0,0,0,0,0,0,0,0,0,0,0 } ;
static BOOL not_doneS = TRUE ;
static BOOL first_fdsS = TRUE ;
static BOOL first_capS = TRUE ;
static BOOL called_rowconS = FALSE ;
static BOOL P_limit_setS = FALSE ;
static double xadjustmentS,xalS,min_xalphaS,max_xalphaS;/* x control */
static double yadjustmentS,yalS,min_yalphaS,max_yalphaS;/* y control */
static double tauXS, tauYS ; /* exp. decay time constants for window */
static double stepS ;
static double num_penalS ;
static double avg_rowpenalS ;
static double f_meanS = 0.0;
static double total_stepS ;
static double fp_ratioS = 1.0 ;
static double log_tabS[TABLIMIT];
static double bin_capS = 99.0 ;
static double row_capS = 99.0 ;
static double a_ratioS;
static double total_costS;

void init_uloop()
{
    not_doneS = 1;
    acc_cntS = move_cntS ;
    ratioG = 1.0;
} /* end init_uloop */

void uloop()
{

FENCEBOXPTR fence ;
CBOXPTR acellptr, bcellptr ; 
BBOXPTR ablckptr , bblckptr ;
int flips , rejects , do_single_cell_move , bit_class ;
int axcenter , bxcenter , bycenter ; 
int aorient , borient ;
int blk , pairflips ;
int i , j , t , count , swaps, index, shift ;
int abin , bbin , fds , done , single_swap ;
double target_row_penalty ;
double target_bin_penalty ;
double temp , percent_error ;
double dCp, delta , gswap_ratio ;
int m1,m2, trials ;
int num_accepts , gate_switches , gate_attempts ;
int last_flips , delta_func , delta_time ;
int temp_timer, time_to_update ; /* keeps track of when to update T */
double iter_time, accept_deviation, calc_acceptance_ratio() ;
double num_time, num_func ;
double calc_time_factor() ; 
/* 
    commented out variables 
    int reset_T ;
    double old_T ;
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
	mean_wire_chgG = total_wire_chgG / (double) wire_chgsG ;
	if( iterationG > 1 ) {
	    sigma_wire_chgG = sqrt( sigma_wire_chgG / (double) wire_chgsG);
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
    aG = PICK_int( 1 , numcellsG - extra_cellsG ) ;

    /* ------------- get the structure for cell# aG ------------- */
    acellptr = carrayG[ aG ] ;
    ablockG   = acellptr->cblock ;
    axcenter = acellptr->cxcenter ;

/* ------------------------------------------------------------------------
    If two cells have the same swap_group then parts of the cells are
    interchangable. Below we check if the cell#aG belongs to a swap_group
   ------------------------------------------------------------------------ */
    if( acellptr->num_swap_group > 0 ) {
	int sgroup;
	SGLISTPTR sglistptr;
	i = PICK_int( 0 , acellptr->num_swap_group - 1 ) ;
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
		    BpostG = PICK_int( 1 , *cellbptrG ) ;
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
		j = PICK_int( 1 , count ) ;
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
	    BpostG = PICK_int( 1 , *cellbptrG ) ;
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
			(double) rowpenalG) / num_penalS ;
    
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
			    (double) delta_time) / num_time ;
	
	    /* calculate a running average of (delta) wirelength penalty */
	    delta_func = abs( delta_func - funccostG ) ;
	    num_func += 1.0 ;
	    avg_funcG = (avg_funcG * (num_func - 1.0) + 
				(double) delta_func) / num_func ;
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
	ratioG = ((double)(num_accepts)) / (double) temp_timer ;
	temp_timer = 0 ; /* reset counter */
	num_accepts = 0 ;
	iter_time = (double) iterationG +
		    (double) attemptsG / (double) attmaxG ;
	accept_deviation = 
	    (calc_acceptance_ratio( iter_time ) - ratioG ) ;
	if( (double) iterationG < TURNOFFT ) {
	    accept_deviation *= ACCEPTDAMPFACTOR ; 
	} else {
	    accept_deviation *= ACCEPTDAMPFACTOR2 ;
	}
	TG *= 1.0 + accept_deviation ;
	update_window_size( (double) iterationG +
			    (double) attemptsG / (double) attmaxG ) ;
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
    percent_error = (double) error_countG / (double) potential_errorsG ;
} else {
    percent_error = 0.0 ;
}
percent_error *= 100.0 ;

if( pairflips > 0.0001 ) {
    fp_ratioS = (double)flips/(double)pairflips ;
} else {
    fp_ratioS = 1.0 ;
}

ratioG = ((double)(pairflips+flips)) / attemptsG;
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
	    gswap_ratio = (double) gate_switches / (double) gate_attempts ;
	} else {
	    gswap_ratio = 0 ;
	}
	sprintf(YmsgG,"%4.2lf %4.2lf %5.3lf %4.2lf\n",
	    fp_ratioS,((double) earlyRejG)/attemptsG,ratioG,
	    gswap_ratio );
	M( MSG, NULL, YmsgG ) ;

    } else {
	sprintf(YmsgG,"%4.2lf %4.2lf %5.3lf\n",
	    fp_ratioS,((double) earlyRejG)/attemptsG,ratioG );
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

	target_bin_penalty = (1.25 - 1.00 * fraction_doneG) * (double)totalRG ;
	target_row_penalty = (initialRowControlG - 
		    (initialRowControlG - finalRowControlG) * 
		    (fraction_doneG) ) * (double) totalRG ;
	binpenConG += ((double) binpenalG - target_bin_penalty) / 
						(double) totalRG ;
	binpenConG = (binpenConG > 1.0) ? binpenConG : 1.0 ;
	if( good_initial_placementG ) {
	    binpenConG = (binpenConG > 4.0) ? 4.0 : binpenConG ;
	}
	delta = ((double) rowpenalG - target_row_penalty) / 
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
	penaltyG = (int)( binpenConG * (double) binpenalG + 
				    roLenConG * (double) rowpenalG ) ;
    } /* end !Equal_Width_CellsG */

    timeFactorG = calc_time_factor() ;
}

return ;
} /* end uloop */

void rowcon()
{

int C , R , p_first , totalCells , cellsPerRow , temp_R ;
int over, under ;
double states , value , expect , variance ; 
double expectedExtraRowLength , rowControl , x , minDev ;

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
	C = (int)( rowControl * (double) totalCells / 2.0 ) ;
	if( 2.0 * (double)(C+1) - rowControl * totalCells <=
			rowControl * totalCells - 2.0 * (double) C ) {
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
	expectedExtraRowLength = 100.0 * expect / (double) cellsPerRow ;
	value = expected_svalue( C , 0 , p_first , R ) ;
	variance = value / states - (expect * expect) ;
	x = 100.0 * sqrt( variance ) / (double) cellsPerRow ; 
	minDev = 100.0 / (double) cellsPerRow ;
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
	C = (int)( rowControl * (double) totalCells / 2.0 ) ;
	if( 2.0 * (double)(C+1) - rowControl * totalCells <=
			rowControl * totalCells - 2.0 * (double) C ) {
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

double partition( int C_initial , int k_initial , int p_initial , int R_initial )
{

int R , C , k , p , k_limit , p_limit ;
double states , equivs ;

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

double expected_value( int C_initial , int k_initial , int p_initial , int R_initial )
{

int R , C , k , p , k_limit , p_limit ;
double value , equivs ;

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
	value += (double) p * equivs *
			(compute_and_combination( C , k , p , R ) -
				    partition( C , k , p , R ) ) ;
    }
}

return( value ) ;
}

double expected_svalue( int C_initial , int k_initial , int p_initial , int R_initial )
{

int R , C , k , p , k_limit , p_limit ;
double value , equivs ;

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
	value += (double)(p * p) * equivs *
			(compute_and_combination( C , k , p , R ) -
				    partition( C , k , p , R ) ) ;
    }
}

return( value ) ;
}


double compute_and_combination( C , k , p , R )
int C , k , p , R ;
{

int numerator , denom1 , denom2 , temp ;
double states ;

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
    states *= (double) numerator ;
    states /= (double) denom1 ;
}

return( states ) ;
}


double combination( numerator , denominator )
int numerator , denominator ;
{

double states ;
int temp , denom1 , denom2 ;

states = 1.0  ;

denom1 = denominator ;
denom2 = numerator - denominator ;
if( denom1 > denom2 ) {
    temp = denom1 ;
    denom1 = denom2 ;
    denom2 = temp ;
}
for( ; numerator > denom2 ; numerator-- , denom1-- ) {
    states *= (double) numerator ;
    states /= (double) denom1 ;
}

return( states ) ;
}

int sanity_check()
{

int *cellxptr , cell , center , block , bin , i ;

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

int sanity_check2()
{

int *cellxptr , *clist ;
int block , bin , cell , i ;

clist = (int *) Ysafe_malloc( (1 + numcellsG - extra_cellsG ) * sizeof( int ) ) ;
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

int sanity_check3()
{

int *cellxptr ;
int block , bin , cell , i ;

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
int eval_ratio(double *t)
{
    if( iterationG >= TURNOFFT ){
	*t = 1.0 ;
    } else if( iterationG < 0 ){
	*t = 0.0 ;
    } else {
	*t = (double) iterationG / TURNOFFT ;
    }
    return((ratioG < AC4) ? 0 : 1);
}

void init_control(int first)
{
    int i;
    double tmp ;

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

void pick_fence_position(int *x, int *y, FENCEBOX *fence)
{
    register int left,right;
    BBOXPTR bblckptr ;

    *y = PICK_int( fence->min_block , fence->max_block ) ;
    bblckptr = barrayG[*y] ;
    
    left = fence->min_xpos ;
    if( left < bblckptr->bxcenter + bblckptr->bleft ) {
	left = bblckptr->bxcenter + bblckptr->bleft ;
    }
    right = fence->max_xpos ;
    if( right > bblckptr->bxcenter + bblckptr->bright ) {
	right = bblckptr->bxcenter + bblckptr->bright ;
    }
    *x = PICK_int( left , right ) ;
    return;
}

void pick_position(int *x, int *y, int ox, int oy,double scale)
{
    register int i,m,n,bleft,bright;
    double tmp ;
    BBOXPTR bblckptr ;

    m = RAND;
    tmp = yalS * scale ;
    if( tmp > max_yalphaS ) {
	tmp = max_yalphaS ;
    }
    n = -tmp * log_tabS[(m >> TABSHIFT) & TABMASK] + 0.5;
    if (m & 0x10000) n = -n;
    n += oy;
    if (n < 1) n = PICK_int(1,oy);
    else if (n > numRowsG) n = PICK_int(oy,numRowsG);
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
	n = PICK_int(bleft,ox);
    } else if (n > bright) {
	if (ox < bleft) ox = bleft;
	else if (ox > bright) ox = bright;
	n = PICK_int(ox,bright);
    }
    *x = n;
}

/* change range limiter according to iterationG number */
void update_window_size( double iternum )
{

/*
    commented out variables 
    CBOXPTR ptr ;
    int cell ;
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
}

void save_control(FILE *fp)
{
    fprintf(fp,"%d 0 %d\n",pairtestG,not_doneS);
    fprintf(fp,"%d %d %d %d\n",acc_cntS,move_cntS,first_fdsS,first_capS);
    fprintf(fp,"%f %f %f %f\n",stepS,xalS,yalS,a_ratioS);
    fprintf(fp,"%f %f\n",ratioG,total_costS);
    fprintf(fp,"%f %f\n",bin_capS,row_capS);
    fprintf(fp,"%f %f %f\n",avg_timeG, avg_funcG, timeFactorG);
}

void read_control( FILE *fp )
{
    int junk ;

    fscanf(fp,"%ld %ld %ld\n",&pairtestG,&junk,&not_doneS);
    fscanf(fp,"%ld %ld %ld %ld\n",&acc_cntS,&move_cntS,&first_fdsS,&first_capS);
    fscanf(fp,"%lf %lf %lf %lf\n",&stepS,&xalS,&yalS,&a_ratioS);
    fscanf(fp,"%lf %lf\n",&ratioG,&total_costS) ;
    fscanf(fp,"%lf %lf\n",&bin_capS,&row_capS);
    fscanf(fp,"%lf %lf %lf\n",&avg_timeG, &avg_funcG, &timeFactorG);
}

int tw_frozen( int cost )
{

double diff , avg_first_set , avg_second_set ;
int i ;

if( cost_vectorS[0] >= 12 ) {
    for( i = 2 ; i <= 12 ; i++ ) {
	cost_vectorS[i-1] = cost_vectorS[i] ;
    }
    cost_vectorS[12] = cost ;
    avg_first_set = (double)(cost_vectorS[1] + cost_vectorS[2] + 
		    cost_vectorS[3] + cost_vectorS[4]) / 4.0 ;
    avg_second_set = (double)(cost_vectorS[9] + cost_vectorS[10] + 
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
