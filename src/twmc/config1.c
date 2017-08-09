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
FILE:	    config1.c                                       
DESCRIPTION:configures initial macro placement
CONTENTS:   config1( )
DATE:	    Jan 30, 1988 
REVISIONS:
	    Feb 13, 1988 - passed modified number of bins to routine
		This takes into account only number of cell bins
		not border bins.
	    Feb 27, 1988 - no longer pass number of bins to routine
		This makebins calculates it and this routine now calls
		makebins.  However, we still must watch border bins.
	    Jun 27, 1988 - add calcPinFactor for initializing feedback
		controller for overfilled pin site penalty.
	    Aug 16, 1988 - fixed sqrt domain error by add temp variables.
		var changed to double because wraparound problem.
	    Aug 26, 1988 - fixed sqrt domain error correctly by adding
		double temp variables.
	    Jan 29, 1989 - fixed \n problem with message macros.
	    Mar 02, 1989 - moved calcLapFactor and calcPinFactor 
		to penalties.c
	    Mar 30, 1989 - changed tile datastructure.
	    Apr 30, 1989 - added max_shortSide calculation for
		partition case.
	    Apr 20, 1990 - Now take aspect ration into account.
	    Fri Jan 25 18:02:00 PST 1991 - fixed problem with
		recursive sizing of data.
	    Mon Feb  4 02:01:57 EST 1991 - added call to update wire estimator
		after each configuration change.
	    Thu Oct 17 11:47:32 EDT 1991 - added initialization.
----------------------------------------------------------------- */
#include <allheaders.h>

#define NUMBINSPERCELL  4   /* we want average cell to be in 4 bins
				for accuracy */
#define WIREAREAUNKNOWN 0   /* at this time we don't know wire area */
#define UPDATE  (BOOL)  FALSE /* don't initialize updateFixedCells */

void makebins( int numbins );
int loadbins(BOOL wireAreaKnown);
void placepads();
void resize_wire_params();

void config1() {
	CELLBOXPTR cellptr ;
	TILEBOXPTR tileptr ;
	int l , r , b , t , cell ;
	int numbins, numbinX, numbinY ;
	int window ;
	char resfile[LRECL] ;
	char savfile[LRECL] ;
	BOOL parasite ;
	double tileArea ;
	double softArea ;
	double totalArea ;
	double coreArea ;
	double cellArea ;
	double varpercell ;
	double deltaArea, deltaShort ;
	double shortvarpercell ;
	double var = 0.0 ;
	double temp ;
	double var_short = 0.0 ;
	double shortSide, total_shortSide, mean_shortSide, dev_shortSide ;
	double length, height, max_shortSide ;


	/*
	*   Sum the areas of the cells - get total area, average cell area. 
	*/
	totalArea = 0.0 ;
	total_shortSide = 0.0 ;
	softArea = 0.0 ;
	for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	cellptr = cellarrayG[cell] ;
	for( tileptr = cellptr->tiles;tileptr; tileptr = tileptr->next ){
		l = tileptr->left   ;
		r = tileptr->right  ;
		b = tileptr->bottom ;
		t = tileptr->top    ;
		printf("left %d right %d bottom %d top %d \n",l,r,b,t);
		tileArea = (double) (r - l) * (double) (t - b) ;
		totalArea += tileArea ; 
		if( cellptr->celltype == STDCELLTYPE ) {
			softArea += tileArea ;
		}
		/* calculate mean of smallest size of tile */
		length = ABS( r - l ) ;
		height = ABS( t - b ) ;
		total_shortSide += (double) MIN( length , height ) ;
	}
	}
	printf("numcellsG: %d\n",numcellsG);
	printf("totalArea: %f\n",totalArea);

	mean_cellAreaG = (totalArea / (double) numcellsG) ;
	mean_shortSide = total_shortSide / numcellsG ;
	/*
	*   Find the variance in the size of the cells.
	*   Find the variance in the shortest side of the tiles.
	*   Find the largest short side for configuring partition case.
	*/
	max_shortSide = (double) INT_MIN ;
	for( cell = 1 ; cell <= numcellsG ; cell++ ) {
		cellptr = cellarrayG[cell] ;
		/* get total area for one cell */
		cellArea = 0.0 ;
		for( tileptr = cellptr->tiles;tileptr; tileptr = tileptr->next ){
			l = tileptr->left   ;
			r = tileptr->right  ;
			b = tileptr->bottom ;
			t = tileptr->top    ;
			length = ABS( r - l ) ;
			height = ABS( t - b ) ;
			cellArea += (double) length * (double) height ;
			/* calculate variance of smallest size of tile */
			shortSide = MIN( length , height ) ;

			if( cellptr->celltype != STDCELLTYPE ){
				/* find the maximum shortside */
				max_shortSide = MAX( max_shortSide, shortSide ) ;
			}

			deltaShort = shortSide - mean_shortSide ;
			var_short += deltaShort * deltaShort ;
		}
		/* note variance of area is over a cell not tile */
		deltaArea = cellArea - mean_cellAreaG  ;
		var += deltaArea * deltaArea ;
	}
	varpercell = var / (double) numcellsG ;
	dev_cellAreaG = sqrt( varpercell ) ;
	shortvarpercell = var_short / (double) numcellsG ;
	dev_shortSide = sqrt( shortvarpercell ) ;

	printf("\nTotal cell area : %4.2le\n", totalArea ) ;
	printf("mean cell area : %4.2le      std deviation cell area : %4.2le\n", mean_cellAreaG, dev_cellAreaG ) ;
	printf("mean short side : %4.2le      std deviation short side : %4.2le\n", mean_shortSide, dev_shortSide ) ;

	if( coreGivenG == 0 ) {
		blockrG = blocktG = (int) sqrt( (double) totalArea ) + 1 ;
		/* 
		*    Take into account the aspect ratio requested by the user
		*/
		blocktG = (int)( sqrt(chipaspectG) * (double) blocktG ) + 1 ;
		blockrG = (int)( 1.0 / sqrt(chipaspectG) * (double) blockrG ) + 1;
		blocklG = blockbG = 0 ;
		totChanLenG = perimG / 2 - (blockrG + blocktG) ;
		aveChanWidG = 0.0 ;
	} else {
		r = t = (int) sqrt( totalArea ) + 1 ;
		totChanLenG = perimG / 2 - (r + t) ;
		aveChanWidG = 0.0 ;
	}

	slopeXG = (double)(maxWeightG - baseWeightG) / ((double) blockrG * 0.5 ) ;
	slopeYG = (double)(maxWeightG - baseWeightG) / ((double) blocktG * 0.5 ) ;
	basefactorG = (double) baseWeightG ;
	wireFactorXG = wireFactorYG = 0.0 ;

	/* DETERMINE NUMBER OF BINS */
	cellArea = (double) (mean_shortSide + 0 * dev_shortSide ) ;
	cellArea *= cellArea ;
	coreArea = ((double) blocktG - blockbG) * ((double) blockrG - blocklG) ;

	if( 5.0 * coreArea > (double) INT_MAX && !(cost_onlyG) ){
		scale_dataG = (int) sqrt( (10.0 * coreArea / (double) INT_MAX)) ;
		scale_dataG++ ; /* round up always */
		M( MSG,"config1", "Design is too large for integer operations\n");
		sprintf(YmsgG, "Calling TimberWolfMC recursively to scale data by %d\n", scale_dataG ) ;
		M( MSG, NULL, YmsgG ) ;
		TWCLOSE( fpoG ) ;
		/* go to cost only mode */
		cost_onlyG = TRUE ;

		/* read the placement from restart file by moving sav to res */
		sprintf( savfile, "%s.msav", cktNameG ) ;
		sprintf( resfile, "%s.mres", cktNameG ) ;

		/* make a system independent copy */
		YcopyFile( savfile, resfile ) ;

		/*if( parasite ){
			G( TWrestoreState() ) ;
		}*/
		restartG = TRUE ;
		scale_dataG = 1 ;
	}

	numbins = (int) ((double) NUMBINSPERCELL * coreArea / (double) cellArea ) ;

	if( numbins <= 1 ) {
		M(ERRMSG,"config1","number of bins calculated is <= 1. Must exit.\n");
		return;
	}

	/* makebins determines globals maxBinX and maxBinY */
	makebins( numbins ) ;

	/***** calculate the core configuration constants for wire estimation */
	blockmxG = (blockrG + blocklG) / 2 ;
	blockmyG = (blocktG + blockbG) / 2 ;
	halfXspanG = (blockrG - blocklG) / 2 ;
	halfYspanG = (blocktG - blockrG) / 2 ;

	/* take account of border for loadbins */
	numbinX = maxBinXG - 1;
	numbinY = maxBinYG - 1;

	binWidthXG = (blockrG - blocklG) / numbinX ;
	if( (blockrG - blocklG - binWidthXG * numbinX) >= numbinX / 2 ) {
		binWidthXG++ ;
	}
	binXOffstG = blocklG + 1 - binWidthXG ;

	binWidthYG = (blocktG - blockbG) / numbinY ;
	if( (blocktG - blockbG - binWidthYG * numbinY) >= numbinY / 2 ) {
		binWidthYG++ ;
	}
	binYOffstG = blockbG + 1 - binWidthYG ;

	updateFixedCells( UPDATE ) ;  /* place fixed cells */

	if( !(cost_onlyG) ){
		loadbins( WIREAREAUNKNOWN ) ;
	}

	placepads() ;

	bdxlengthG = blockrG - blocklG ;
	bdylengthG = blocktG - blockbG ;
	printf("bdxlength:%d    bdylength:%d\n",bdxlengthG,bdylengthG);

	/* update for wire estimation algorithm */
	resize_wire_params() ;

	return ;
} /* end config1 */
