/*
 *   Copyright (C) 1988-1991 Yale University
 *   Copyright (C) 2014 Ruben Undheim <ruben.undheim@gmail.com>
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
FILE:	    main.c                                       
DESCRIPTION:main routine
DATE:	    Jan 29, 1988 
REVISIONS:  Sep  7, 1988 - fixed argument mismatch to utemp.
	    Sep 24, 1988 - eliminated testloop.c
	    Nov 25, 1988 - set coreError to 0 for partition case.
	    Dec  3, 1988 - added timing driven calculation.
	    Jan 15, 1989 - modifications for curve-fit controller.
		Also added lap.history file for debugging overlap.
	    Jan 29, 1989 - changed msg to YmsgG and added \n's.
	    Feb 23, 1989 - added X11 graphics to MC and pass arguments
		to writeResults which gathers info.
	    Feb 25, 1989 - removed condition compile for MEMORYMAN.
		    - changed iteration numbers to negative.
	    Mar 02, 1989 - moved all negative feedback code to penalties.c
	    Mar 07, 1989 - added compute_attprcel to calculate correct
		attempts per cell.
	    Mar 11, 1989 - added graphics conditional compile.
	    May  9, 1989 - added command line options.
	    May 17, 1989 - modified graphics - partition code.
	    May 24, 1989 - updated control.  Deleted doPlacementG.
	    Jun 21, 1989 - made verbose a global.
	    Sep 25, 1989 - made movebox dynamic and hence needed to 
		move it.   Fixed unsigned randvar print statement.
		Added NOGRAPHICS conditional compile.
	    Apr 23, 1990 - Modified timing constant determination
		and moved graph routines to the library.
	    May  2, 1990 - Moved print_netinfo to cleanupReadcells
		in order for verify_pad_pins to work properly.
	    May  4, 1990 - moved print_stats to library.
	    Sun Dec 16 00:29:00 EST 1990 - now look at .net file
		if .mnet file doesn't exist.
	    Thu Jan 17 00:47:43 PST 1991 - changed silent mode to
		to verbose mode to make consistent with other progs.
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
	    Thu Jan 24 17:12:04 PST 1991 - now when TWMC is called
		recursively .mout file will be correct.
	    Fri Jan 25 18:07:09 PST 1991 - started on quickroute.
	    Mon Feb  4 02:12:46 EST 1991 - working quickroute for non
		partition case.
	    Thu Feb  7 00:17:32 EST 1991 - moved placement of .mest
		calculation.
	    Thu Mar  7 01:47:38 EST 1991 - added Xdebug statement.
	    Thu Mar 14 16:11:13 CST 1991 - avoid reading .net file
		during partitioning.
	    Sat Apr 27 01:13:33 EDT 1991 - moved wirest code so
		now we can use readcells to tell whether we have
		a partitioning case.  Also added initialize_aspect_ratios
	    Wed May  1 19:20:03 EDT 1991 - renamed lap.history.
	    Wed Jun  5 16:28:05 CDT 1991 - added condition for
		initializing aspect ratios.
----------------------------------------------------------------- */
#include "allheaders.h"

/* turn on condition compile for globals */
#define  MAIN_DEFS
#define  CUSTOM_DEFS

#define  NOREDUCTION        -1000000.0 ; 

FILE *fpoG;

char *argv0G;

double saveLapFactorG;

int bdxlengthG , bdylengthG;
int scale_dataG; 
int attmaxG;
int TrybinG;        /* used in setBin calculations */
int coreG[2][2];

BOOL doGraphicsG;
BOOL quickrouteG;
BOOL new_wire_estG;
BOOL verboseG;

static BOOL padsOnlyS;  /* whether to place on pads */
static BOOL batchS;     /* is TW in batch mode partition case */
static BOOL debugS ;     /* whether to enable debug code */
static double wire_red_ratioS = NOREDUCTION ; /* wire reduction */

int readnets(char *filename);
int yaleIntro();

int
__attribute__((visibility("default")))
TimberWolfMC(int b, int d, int n, int scale_dataP, int p, int q, int v, char *dName)
{
	printf("Running TimberWolfMC\n");

	FILE *fp ;
	char filename[LRECL];

	int	rememberWire, /* variables for writing history of run */
		rememberPenal,
		rememberRand ;

	/* ********************** start initialization *********************** */
	/* start up cleanup handler */

	debugS      = FALSE ;
	verboseG    = FALSE ;
	quickrouteG = FALSE ;
	scale_dataG = 0 ;
	batchS      = FALSE ;
	doGraphicsG = TRUE ;

	if(b) {
		batchS = TRUE ;
		doGraphicsG = FALSE ;
	}
	if(d) {
		debugS = TRUE ;
	}
	if(n) {
		doGraphicsG = FALSE ;
	}
	if(p) {
		padsOnlyS = TRUE ;
	}
	if(q) {
		quickrouteG = TRUE ;
	}
	if(v) {
		verboseG = TRUE ;
	}
	cktNameG = dName;

#ifdef NOGRAPHICS
	doGraphicsG = FALSE ;
#endif /* NOGRAPHICS */

	scale_dataG = scale_dataP;

	YdebugMemory( debugS ) ;

	/* handle I/O requests */
	sprintf( filename, "%s.mout" , cktNameG ) ;
	if( scale_dataG ){
		fpoG = TWOPEN( filename, "a", ABORT ) ;
	} else {
		fpoG = TWOPEN( filename, "w", ABORT ) ;
	}

	YinitProgram( "TimberWolfMC", VERSION, yaleIntro );

	/* now tell the user what he picked */
	M(MSG,NULL,"\n\nTimberWolfMC switches:\n" ) ;
	if( debugS ){
		YsetDebug( TRUE ) ;
		M(MSG,NULL,"\tdebug on\n" ) ;
	} 
	if( verboseG  ){
		M(MSG,NULL,"\tMessages will be redirected to screen\n" ) ;
	}
	if( doGraphicsG ){
		M(MSG,NULL,"\tGraphics mode on\n" ) ;
	} else {
		M(MSG,NULL,"\tGraphics mode off\n" ) ;
	}

	/* ********************** end initialization ************************* */
	readpar();

	while( TRUE ) {
		/* initialize annealing exp. table */
		init_table() ;
		init_acceptance_rate() ; /* initialize heuristic annealing schedule */

		if( randVarG == -1 ) {
			randVarG = Yrandom_seed() ;
			Yset_random_seed( randVarG ) ;
		}
		printf("\nThe rand generator seed was: %u\n\n\n", (unsigned) randVarG );
		rememberRand = randVarG ;

		/* assume first that this is a mixed mode case - .mcel otherwise */
		/* it is a macro cell only case so look for .cel */

		sprintf(filename, "%s.mcel" , cktNameG ) ;
		if( access( filename, F_OK ) == -1 ) { 
			sprintf(filename, "%s.cel" , cktNameG );
			if( access( filename, F_OK ) != -1 ) {
				readcells(filename);
			} else {
				printf("No cel/mcel file found!\n");
				return 1;
			}
		} else {
			readcells(filename);
		}

		/* now see if we need to perform a quickroute to get */
		/* wiring estimate */
		sprintf( filename, "%s.mest", cktNameG ) ;
		int status;
		if( !(quickrouteG) && !(cost_onlyG) && !(scale_dataG) && !(doPartitionG)){
			if(!(YfileExists( filename ))){
				quickrouteG = TRUE ; // perform a quickroute if file doesn't exist
			} else {
				quickrouteG = FALSE ;
			}
			status=TimberWolfMC(b, d, n, scale_dataP, p, quickrouteG, v, cktNameG);
		}
		/* check to see if .mest file was created */
		new_wire_estG = FALSE ;
		if((fp = TWOPEN( filename, "r", NOABORT))){
			if( read_wire_est( fp ) ){
				new_wire_estG = TRUE ;
			}
			TWCLOSE( fp ) ;
		}

		/* make move box structure base on the largest number of tiles */
		make_movebox() ;

		/* first check existence of .mnet file, for .net file */
		sprintf(filename, "%s.mnet", cktNameG ) ;
		if(YfileExists(filename)) {
			readnets( filename ) ;
		} else {
			if(!(doPartitionG)) {
				sprintf(filename, "%s.net", cktNameG ) ;
				if(YfileExists(filename)){
					readnets(filename) ;
				}
			}
		}

		if( doPartitionG && numcellsG == numstdcellG ) {
			/* no macro cells to be placed - perform cost only from now on */
			/* that is just place pads */
			cost_onlyG = TRUE ;
		}

		/* call config1 to get initial configuration */
		config1() ;

		/* initialize incremental bounding box data structures */
		/* uses count found in prnt_netinfo */
		init_unet() ;

		funccostG = findcost() ;
		initcheck() ;

		/* at this point we can initialize the aspect ratios */
		if(!(cost_onlyG)){
			initialize_aspect_ratios() ;
		}

		/* startup graphics */
		if( doGraphicsG ){
			G( set_graphic_context( PLACEMENT ) ) ;
			if( !wait_for_userG ) {
				G( draw_the_data() ) ;
			} else G( if( TWinterupt() ) ){
				G( process_graphics() ) ;
			}
		}

		printf("\n\n\nTHE ROUTE COST OF THE CURRENT PLACEMENT: %d",funccostG );
		printf("\nTHE PENALTY OF THE CURRENT PLACEMENT: %d\n", penaltyG ) ;

		if( wire_red_ratioS < -1000.0 ){
		
			/* if wire_red_ratioS is greater that -1000 it was set in readpar */
			/* this is only for debug purposes */
			if( !(doPartitionG) && numcellsG > 1 ){
				wire_red_ratioS = analyze() ;

				if( wireEstimateOnlyG ) {
					return 0;
				}

			} else {
				/* tmp */
				wire_red_ratioS = 2.0 ;
			}
		}

		if( cost_onlyG && !restartG ){
			/* done initialization */
			break ;
		}

		Yplot_init( 0, "graph", "graph_par","graph_stp","graph_lap","graph_T","graph_prob",NULL ) ;

		attmaxG = compute_attprcel() ;

		if( !cost_onlyG ) {
			printf("\n\nTimberWolf Cell Placement Ready for Action\n\n");
			/* allow multi cell moves */
			if(doGraphicsG) G( set_graphic_context( PLACEMENT ) ) ;
			utemp( attmaxG, TRUE ) ;
		}

		funccostG = findcost() ;
		rememberWire = funccostG ;
		rememberPenal = binpenalG ;

		finalout();
		finalcheck() ;
		twstats() ;
		break ; /* avoid a loop-loop only for restart failure */

	} /* end doPlacement loop */ 

	if( cost_onlyG && !restartG ){
		finalout() ;
		twstats() ;
	}

	if( doGraphicsG ){
		if( wait_for_userG && !doPartitionG ){
			G( TWmessage( "TimberWolfMC waiting for your response" ) ) ;
			G( set_graphic_context( PLACEMENT ) ) ;
			G( process_graphics() ) ;
		} else {
			G( graphics_dump() ) ;
		}
// 		closegraphics() ;
	}
	printf("\n\n************************************ \n\n");
	printf("TimberWolf has completed its mission\n");
	printf("\n\n************************************ \n\n");

// 	Yprint_stats( fpoG ) ;
// 	Yplot_close() ;
	writeResults( rememberWire, rememberPenal, rememberRand );
	if( sc_output() ){
		create_sc_output() ;
	}
	return 0;
} /* end main routine */

int yaleIntro() 
{
	fprintf(fpoG,"\n%s\n",YmsgG) ;
	fprintf(fpoG,"Authors: Carl Sechen, Bill Swartz, Kai-Win Lee\n");
	fprintf(fpoG,"         Dahe Chen, and Jimmy Lam\n"); 
	fprintf(fpoG,"         Yale University\n");

	printf("\n%s\n",YmsgG) ;
	printf("Authors: Carl Sechen, Bill Swartz, Kai-Win Lee\n");
	printf("         Dahe Chen, and Jimmy Lam\n"); 
	printf("         Yale University\n");

} /* end yaleIntro */

/* this routine takes information about run and write to history file */
/* to accumulate data about runs */
void writeResults( int wire, int penal, int rand )
{
	FILE *fpdebug ;
	int left_side, right_side, bottom_side, top_side ;
	char filename[LRECL] ;

	funccostG = findcost() ;
	sprintf( filename,"%s.history", cktNameG ) ;
	fpdebug = TWOPEN( filename, "a", ABORT ) ;
	/* find core region */
	find_core_boundary( &left_side, &right_side, &bottom_side, &top_side);
	bdxlengthG = right_side - left_side ;
	bdylengthG = top_side - bottom_side ;
	fprintf( fpdebug, "%4.2le\t%4.2le\t%4.2le\t%4.2le\t%d\t%d\t%d\t%4.2le\t%34.32le\n", 
			(double) wire, (double) penal,
			(double) funccostG, (double) penaltyG, bdxlengthG, bdylengthG, 
			rand, (double) bdxlengthG * bdylengthG, wire_red_ratioS ) ;
	TWCLOSE( fpdebug ) ;
} /* end writeResults */

BOOL get_batch_mode()
{
	return( batchS ) ;
} /* end get_batch_mode */

void set_wiring_reduction( double reduction )
{
	wire_red_ratioS = reduction ;
} /* set_wiring_reduction */

