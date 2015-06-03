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
FILE:	    utemp.c                                       
DESCRIPTION:outer loop of the simulated annealing algorithm.
CONTENTS:   utemp( )
DATE:	    Feb 5, 1988 
REVISIONS:  Dec 3, 1988 - added output info for timing.
	    Dec 4, 1988 - moved output info to prnt_cost routine and
			    updated arguments to savewolf.
	    Feb 20,1989 - recoded annealing loop to make it easier 
		to read.
	    Mar 01,1989 - changed iter_countS to iteration a global.
		removed all static code.
	    Mar 02,1989 - removed fuloop from code.
	    Jun 21,1989 - added verbose switch to iteration number.
	    Oct 14,1990 - added overlap iterations.
	    Fri Jan 25 18:09:20 PST 1991 - removed unnecessary globals.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) utemp.c version 3.6 4/6/92" ;
#endif

#include <custom.h>
#include <temp.h>
#include <yalecad/debug.h>

utemp( attempts, multi_cell_moves )
INT attempts ;
BOOL multi_cell_moves ;
{

    INT i ;
    DOUBLE calc_lap_factor( P1(DOUBLE percentDone) ) ;


    /****************** main annealing loop ***************** */
    while( iterationG < (INT) LASTTEMP ){
	iterationG++ ; /* next iteration */
	/* write iteration to the screen if not verbose */
	if(!(verboseG )){
	    fprintf( stdout, "%3d ", iterationG ) ;
	    if(!(iterationG % 15)){
		fprintf( stdout, "\n" ) ;
	    }
	    fflush( stdout ) ;
	}
	uloop( attmaxG ) ;
	savewolf( FALSE ) ; /* don't force save */
	
    }

    /* **** overlap penalty controller **** */
    if( iterationG <= (INT) LASTTEMP ){
	iterationG++ ; /* next iteration */
	lapFactorG = calc_lap_factor( (DOUBLE) 3.0 ) ;
	funccostG = findcost() ;
	for( i = 1; i <= 3 ; i++ ){
	    uloop( attmaxG ) ;
	    savewolf( FALSE ) ; /* don't force save */
	    iterationG++ ; /* next iteration */
	}
    }

    /*************** end main annealing loop ***************** */
    /*
     *  * WE ARE FINISHED *
     */
    prnt_cost("\nPLACEMENT RESULTS AFTER ANNEALING ARE:\n" ) ;
    OUT2("MAX NUMBER OF ATTEMPTED FLIPS PER T:%8d\n", attmaxG ) ;
    /* verify incremental and current costs */
    D( "twmc/utemp", checkcost() ) ;
    return ;

} /* end utemp */
