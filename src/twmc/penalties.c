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
FILE:	    penalties.c                                       
DESCRIPTION:calculation of the penalty coefficients
CONTENTS:   
DATE:	    Mar 1, 1989 
REVISIONS:  May 16, 1989 - removed most doPartitionG conditions.
	    Apr 23, 1990 - modified for new graph routines and 
		modified timing constant update to use a constant
		derived at high temperatures.
	    Mon Feb  4 02:16:26 EST 1991 - added overlap penalty
		override for testing purposes.
	    Thu Feb  7 00:20:00 EST 1991 - reworked graph data.
	    Thu Apr 18 01:40:16 EDT 1991 - refit overlap parameters.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) penalties.c version 3.8 9/16/91" ;
#endif

#include <custom.h>
#include <temp.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#define DEBUGLAPFACTOR

#define DAMPFACTOR       0.015    /* damping factor on overlap penalty */
#define COREDAMPFACTOR   0.10     /* damping factor on core oversize */
#define CORECAP          1.30     /* maximum penalty for core error */
#define COREMIN          1.00     /* min error for core at end */
#define LAPMIN           1.00     /* min penalty factor for binpenalty */
#define LAPCAPFACTOR   100.00     /* lap penalty max is 100x funccost */
#define TIMEFACTOR       3.00     /* ration of time to funccost */
#define STARTCORE        0.10     /* start core for placement case */
#define ENDCORE          0.05     /* end core for placement case */
#define STARTOVERLAP     0.70     /* start overlap for placement case */
#define ENDOVERLAP       0.15     /* end overlap for placement case */
#define	INITFACTOR       1.00     /* initial penalty factors */
#define INITNETPENAL  1000.00     /* dummy value to avoid / 0 */



/* ----------------------------------------------------------------- 
   Initial weight of various penalties to wirelength cost Penalties
   are INITRELXXXX% of the wirelength part.  This is not critical 
   since negative feedback will correct our possible mistake.
   -------------------------------------------------------------- */
#define INITRELLAP       0.40     /* overlap relative to funccost */

static  DOUBLE coreAreaS ;        /* the area of the core */
static  DOUBLE start_core_errorS ;/* the start and stop target */
static  DOUBLE end_core_errorS ;  /* ratios for the various */
static  DOUBLE start_overlapS ;   /* penalties. */
static  DOUBLE end_overlapS ;
static  BOOL   firstLapS = TRUE; /* 1st time calc_init_lapFactor called */
static  BOOL   firstTimeS = TRUE;/* 1st time calc_init_timeFactor called*/

/* ----------------------------------------------------------------- 
   Use negative feedback to control relative ratio between penalties
*/

/* **** overlap penalty controller **** */
DOUBLE calc_lap_factor( percentDone ) 
DOUBLE percentDone ;
{

    DOUBLE diff_lap, target_bin_penalty, bin_deviation ;
    DOUBLE sqrtCoreArea, sqrtBinPenal, lapCap ;
    INT iter ;
    char filename[LRECL] ;
    FILE *fp ;

#define A  0.84379
#define B -1.7294E-4
#define C -2.3867E-5
    /* - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -
        Initialize beginning average to one so that in beginning 
        controller can change alot to adjust to conditions.  Later the
	average will diminish the result. 
    - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - */
#   define NUMSAMPLE 15
#   define RECIP_SAMPLE 0.06666666
    static  DOUBLE avg_devS[NUMSAMPLE] = { 0.0 } ;
    static  INT    avgCountS = 0 ;
    DOUBLE  running_avg ;
    INT     i ;


    /* **** overlap penalty controller **** */
    /* no damping at the start so that controller can react quickly */

    /* calculate target penalty for overlap penalty. */
    sqrtCoreArea = sqrt( coreAreaS ) ;
    iter = iterationG + 2 ;
    diff_lap = A + B*iter + C*iter*iter ;
    if( percentDone > 2.0 ){
	diff_lap = 0.01 ;
    }
    target_bin_penalty = diff_lap * sqrt( coreAreaS ) ;
    sqrtBinPenal = sqrt( (DOUBLE) binpenalG ) ;

    /* bin_deviation is percent error relative to target */
    bin_deviation = 
	(sqrtBinPenal - target_bin_penalty) / target_bin_penalty ;

    Yplot_heading( 0, "graph_lap", "iter","binpenal","sqrtbin",
	    "coreArea","sqrtCore", "percent","sqrtpercent", 
	    "target", "lapFactor", "bin_dev", NULL ) ;
    Yplot( 0, "graph_lap", "%d", iterationG, "%4.4le %4.4le %4.4le %4.4le", 
	(DOUBLE) binpenalG, sqrtBinPenal, coreAreaS, sqrtCoreArea ) ;

    /* save running average of deviations */
    avg_devS[avgCountS++ % NUMSAMPLE] = bin_deviation ;
    running_avg = 0.0 ;
    for( i = 0; i < NUMSAMPLE; i++ ){
	running_avg += avg_devS[i] ;
    }
    running_avg *= RECIP_SAMPLE ;

    if( avgCountS >= NUMSAMPLE ){
	/* use running average to bound maximum change in lapFactor */
	if( running_avg > 0.0 ){
	    /* bound between 0 and running average */
	    bin_deviation = MAX( bin_deviation, 0.0 ) ;
	    bin_deviation = MIN( bin_deviation, running_avg ) ;
	} else {
	    /* bound between 0 and - running average */
	    bin_deviation = MIN( bin_deviation, 0.0 ) ;
	    bin_deviation = MAX( bin_deviation, running_avg ) ;
	}
    }

    lapFactorG *= (1.0 + bin_deviation) ; 
    Yplot( "graph_lap", "%d", iterationG, 
	"%4.4le %4.4le %4.4le %4.4le %4.4le", 
	(DOUBLE) binpenalG / coreAreaS, 
	sqrtBinPenal / sqrtCoreArea,
	target_bin_penalty, lapFactorG, bin_deviation ) ;
    Yplot_flush( "graph_lap" ) ;

    lapFactorG = (lapFactorG > LAPMIN) ? lapFactorG : LAPMIN ; 
    lapCap = LAPCAPFACTOR * (DOUBLE) funccostG / sqrtBinPenal ;
    lapFactorG = (lapFactorG < lapCap) ? lapFactorG : lapCap ; 

    /* this is an override mechanism to setting parameters */
    sprintf( filename, "%s.lap", cktNameG ) ;
    if( fp = TWOPEN( filename, "r", NOABORT )){
	HPI( fp, &lapFactorG ) ;
	TWCLOSE( fp ) ;
    }
    return( lapFactorG ) ;

} /* ********* end overlap penalty controller ******** */

DOUBLE calc_time_factor( percentDone ) 
DOUBLE percentDone ;
{
    /* **** timing penalty controller **** */
    /* don't change it from initial value */
    return( timeFactorG ) ;
    /* ********* end timing penalty controller ******** */

} /* end function calc_time_penalty */


/* ----------------------------------------------------------------- 
   Calculate the new core area as a function of time.  Reconfigure 
   the core according to this new target compensating overlap penalty
   to account for change in cell areas.  Calculate cell areas with
   routing areas = TRUE. Core_deviation varies as the negative of
   the error measurement. That is why target_core_error and core_error
   in core_deviation calculation are reversed.
*/
DOUBLE calc_core_factor( percentDone ) 
DOUBLE percentDone ;
{
    INT binArea, cellArea ;
    DOUBLE diff_core, target_core_error, core_deviation ;
    DOUBLE core_error ;

    binArea = get_bin_area() ;
    cellArea = calc_cellareas( TRUE ) ;
    diff_core = start_core_errorS - end_core_errorS ;
    core_error = (DOUBLE) (binArea - cellArea) / (DOUBLE) cellArea ;
    target_core_error = (start_core_errorS - diff_core * percentDone ) ; 
    core_deviation =
	COREDAMPFACTOR * (DOUBLE) (target_core_error - core_error) ;
    coreFactorG *= 1.0 + core_deviation ;
    coreFactorG = (coreFactorG > COREMIN ) ? coreFactorG : COREMIN ; 
    coreFactorG = (coreFactorG < CORECAP) ? coreFactorG : CORECAP ; 
    coreAreaS = coreFactorG * (DOUBLE) cellArea ;
    /* reconfigure area and place pads */
    reconfigure( maxBinXG-1,maxBinYG-1, coreAreaS ) ;
    return( coreFactorG ) ;
} /* end calc_core_factor */



/* ***************************************************************** 
   INITIAL LAP FACTOR CALCULATION
   Currently, just set lapFactor initially to 40% of wirelength.  This
   could use move investigation in the future.
*/
DOUBLE calc_init_lapFactor( totFunc, totPen ) 
DOUBLE totFunc ;
DOUBLE totPen ;
{
    DOUBLE factor ;
#ifdef DEBUGLAPFACTOR
    extern DOUBLE saveLapFactorG ;
#endif

    /* first iteration, we set all factors to 1 */
    /* since every move is acceptted */
    if( firstLapS ){
	firstLapS = FALSE ;
	start_overlapS = INITFACTOR ; end_overlapS = INITFACTOR ;
	return( INITFACTOR ) ;
    }
    if( totPen > EPSILON ){
	factor = INITRELLAP * totFunc / totPen ;
    }

    /* -----------------------------------------------------------
	The penalty controller forces overlap to go from :
	start_overlap*binArea to end_overlap*binArea
       ----------------------------------------------------------- */

#ifdef DEBUGLAPFACTOR
    start_overlapS = STARTOVERLAP ; end_overlapS = saveLapFactorG ;
#else
    start_overlapS = STARTOVERLAP ; end_overlapS = ENDOVERLAP ;
#endif
    OUT2("\n\nOVERLAP FACTOR (COMPUTED) : %f\n", lapFactorG ) ;
    OUT3("start_overlap: %4.2le end_overlap: %4.2le\n\n", 
	start_overlapS, end_overlapS ) ;

    return( factor ) ;

} /* end calc_init_lapFactor */


/* ***************************************************************** 
   INITIAL TIME FACTOR CALCULATION
   Currently, just set timeFactor initially to 40% of wirelength.  This
   could use move investigation in the future.
*/
DOUBLE calc_init_timeFactor( avgdFunc, avgdTime ) 
DOUBLE avgdFunc ;
DOUBLE avgdTime ;
{
    if( firstTimeS ){
	firstTimeS = FALSE ;
	return( INITFACTOR ) ;
    } else {
	if( avgdTime > EPSILON ){
	    return( TIMEFACTOR * avgdFunc / avgdTime ) ;
	} else {
	    return( 0.0 ) ;
	}
    }

} /* end calc_init_timeFactor */

DOUBLE calc_init_coreFactor( ) 
{

    /* -------------------------------------------------------------- 
	The penalty controller forces corearea to go from :
	start_core_error*cellArea to end_core_error*cellArea
	initialize coreFactor to binArea / cellarea.
       ----------------------------------------------------------- */
    coreFactorG = (DOUBLE) get_bin_area() /
		 (DOUBLE) calc_cellareas(TRUE ) ;

    start_core_errorS = STARTCORE ; end_core_errorS = ENDCORE ;

    OUT3("start_core: %4.2le end_overcore: %4.2le\n\n", 
	start_core_errorS, end_core_errorS ) ;
    return( coreFactorG ) ;
} /* end calc_init_coreFactor */
