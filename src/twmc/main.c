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
#ifndef lint
static char SccsId[] = "@(#) main.c version 3.27 11/23/91" ;
#endif

#include <string.h>

/* turn on condition compile for globals */
#define  MAIN_DEFS
#define  CUSTOM_DEFS
#include <custom.h>

#include <yalecad/debug.h>
#include <yalecad/cleanup.h>
#include <yalecad/file.h>

#define  VERSION            "v2.2"
#define  EXPECTEDMEMORY     (1024 * 1024) 
#define  NOREDUCTION        -1000000.0 ; 

DOUBLE saveLapFactorG ;
static BOOL parasiteS;   /* whether window is a parasite */
static BOOL padsOnlyS;  /* whether to place on pads */
static BOOL batchS;     /* is TW in batch mode partition case */
static BOOL debugS ;     /* whether to enable debug code */
static INT  windowIdS ;  /* the master window id if given */
static DOUBLE  wire_red_ratioS = NOREDUCTION ; /* wire reduction */

/* Forward declarations */

VOID syntax();
INT closegraphics();

main( argc , argv )
INT argc ;
char *argv[] ;
{

    FILE    *fp ;
    char    filename[LRECL],
	    arguments[LRECL], /* pointer to argument options */
	    *ptr,             /* pointer to argument options */
	    *Ystrclone() ;
    INT     yaleIntro(),
	    attempts,
	    arg_count ;       /* argument counter */
    INT     rememberWire, /* variables for writing history of run */
	    rememberPenal,
	    rememberRand ;
    BOOL    get_arg_string( P1(char *arguments) ) ;
    DOUBLE  calc_init_lapFactor() ,
	    calc_init_timeFactor() ,
	    calc_init_coreFactor() ,
	    analyze() ,
	    totFunc,
	    totPen,
	    avgdTime,
	    avgdFunc ;

    /* ********************** start initialization *********************** */
#ifdef DEBUGX
    extern int _Xdebug ;
    _Xdebug = TRUE ;
#endif

    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

    Yinit_memsize( EXPECTEDMEMORY ) ;

    if( argc < 2 || argc > 5 ){
	syntax() ;
    } else {
	debugS      = FALSE ;
	verboseG    = FALSE ;
	parasiteS   = FALSE ;
	quickrouteG = FALSE ;
	windowIdS   = 0 ;
	scale_dataG = 0 ;
	batchS      = FALSE ;
#ifndef NOGRAPHICS
	doGraphicsG = TRUE ;
#else /* NOGRAPHICS case */
	doGraphicsG = FALSE ;
#endif /* NOGRAPHICS */
	arg_count = 1 ;
	if( *argv[1] == '-' ){
	    for( ptr = ++argv[1]; *ptr; ptr++ ){
		switch( *ptr ){
		case 'b':
		    batchS = TRUE ;
		    doGraphicsG = FALSE ;
		    break ;
		case 'd':
		    debugS = TRUE ;
		    break ;
		case 'n':
		    doGraphicsG = FALSE ;
		    break ;
		case 'o': /* overflow */
		    scale_dataG = atoi( argv[++arg_count] ) ;
		    break ;
		case 'p':
		    padsOnlyS = TRUE ;
		    break ;
		case 'q':
		    quickrouteG = TRUE ;
		    break ;
		case 'v':
		    verboseG = TRUE ;
		    break ;
		case 'w':
		    parasiteS = TRUE ;
		    break ;
		default:
		    sprintf( YmsgG,"Unknown option:%c\n", *ptr ) ;
		    M(ERRMSG,"main",YmsgG);
		    syntax() ;
		}
	    }
	    YdebugMemory( debugS ) ;

	    /* handle I/O requests */
	    argv0G = Ystrclone( argv[0] ) ;
	    cktNameG = Ystrclone( argv[++arg_count] );
	    sprintf( filename, "%s.mout" , cktNameG ) ;
	    if( scale_dataG ){
		fpoG = TWOPEN( filename, "a", ABORT ) ;
	    } else {
		fpoG = TWOPEN( filename, "w", ABORT ) ;
	    }
	    Ymessage_init(fpoG) ;
	    Ymessage_mode( verboseG  ) ;

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
	    if( parasiteS ){
		M(MSG,NULL,"\tTimberWolfMC will inherit window\n" ) ;
		/* look for windowid */
		if((scale_dataG && argc != 5) || (!scale_dataG && argc != 4)){
		    M(ERRMSG,"main","Need to specify windowID\n" ) ;
		    syntax() ;

		} else {
		    windowIdS = atoi( argv[++arg_count] ) ;
		} 
	    }
	    M(MSG,NULL,"\n" ) ;
	} else if( argc == 2 ){
	    /* order is important here */
	    YdebugMemory( FALSE ) ;
	    cktNameG = Ystrclone( argv[1] );
	    argv0G = Ystrclone( argv[0] ) ;

	    sprintf( filename, "%s.mout" , cktNameG ) ;
	    fpoG = TWOPEN( filename, "w", ABORT ) ;
	    Ymessage_init(fpoG) ;
	    Ymessage_mode( verboseG  ) ;

	    YinitProgram( "TimberWolfMC", VERSION, yaleIntro );

	} else {
	    syntax() ;
	}
    }
    /* ********************** end initialization ************************* */

    readpar() ;

    G( initMCGraphics( argc, argv, windowIdS  ) ) ;

    while( TRUE ){

	/* initialize annealing exp. table */
	init_table() ;
	init_acceptance_rate() ; /* initialize heuristic annealing schedule */

	if( randVarG == -1 ) {
	    randVarG = Yrandom_seed() ;
	    Yset_random_seed( randVarG ) ;
	}
	OUT2("\nThe rand generator seed was: %u\n\n\n", (unsigned) randVarG );
	rememberRand = randVarG ;
	FLUSHOUT();


	/* assume first that this is a mixed mode case - .mcel otherwise */
	/* it is a macro cell only case so look for .cel */
	sprintf(filename, "%s.mcel" , cktNameG ) ;
	if(!(fp = TWOPEN( filename , "r", NOABORT ))){
	    sprintf(filename, "%s.cel" , cktNameG ) ;
	    fp = TWOPEN( filename , "r", ABORT ) ;
	}
	readcells( fp ) ;
	TWCLOSE(fp);

	/* now see if we need to perform a quickroute to get */
	/* wiring estimate */
	sprintf( filename, "%s.mest", cktNameG ) ;
	if( !(quickrouteG) && !(cost_onlyG) && !(scale_dataG) &&
	    !(doPartitionG)){
	    if(!(YfileExists( filename ))){
		/* perform a quickroute if file doesn't exist */
		quickrouteG = TRUE ;
		parasiteS = get_arg_string( arguments ) ;
		M( MSG, NULL, arguments ) ;
		M( MSG, NULL, "\n" ) ;
		Ysystem( "TimberWolfMC",ABORT,arguments,closegraphics ) ;
		if( parasiteS ){
		    /* if we save the graphics state we need to restore it */
		    G( TWrestoreState() ) ;
		}
		quickrouteG = FALSE ;
	    }
	}
	/* check to see if .mest file was created */
	new_wire_estG = FALSE ;
	if( fp = TWOPEN( filename, "r", NOABORT )){
	    if( read_wire_est( fp ) ){
		new_wire_estG = TRUE ;
	    }
	    TWCLOSE( fp ) ;
	}

	/* make move box structure base on the largest number of tiles */
	make_movebox() ;

	/* first check existence of .mnet file, for .net file */
	sprintf(filename, "%s.mnet", cktNameG ) ;
	if(!(fp = TWOPEN( filename , "r", NOABORT ))){
	    if(!(doPartitionG)){ 
		sprintf(filename, "%s.net", cktNameG ) ;
		fp = TWOPEN( filename , "r", NOABORT ) ;
	    }
	}
	readnets( fp ) ;
	if( fp ) TWCLOSE( fp ) ;

	if( doPartitionG && numcellsG == numstdcellG ){
	    /* no macro cells to be placed - perform cost only from now on */
	    /* that is just place pads */
	    cost_onlyG = TRUE ;
	}

	/* call config1 to get initial configuration */
	config1() ;

	/* see if we can open save file for writing */
	sprintf(filename, "%s.msav", cktNameG ) ;
	fp = TWOPEN( filename , "w", ABORT ) ;
	TWCLOSE( fp ) ;

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
	    if( !wait_for_userG ){
		G( draw_the_data() ) ;
	    } else G( if( TWinterupt() ) ){
		G( process_graphics() ) ;
	    }
	}

	OUT2("\n\n\nTHE ROUTE COST OF THE CURRENT PLACEMENT: %d",funccostG );
	OUT2("\nTHE PENALTY OF THE CURRENT PLACEMENT: %d\n", penaltyG ) ;
	FLUSHOUT();


	if( wire_red_ratioS < -1000.0 ){
	    
	    /* if wire_red_ratioS is greater that -1000 it was set in readpar */
	    /* this is only for debug purposes */
	    if( !(doPartitionG) && numcellsG > 1 ){
		wire_red_ratioS = analyze() ;

		if( wireEstimateOnlyG ) {
		    closegraphics() ;
		    YexitPgm(OK);
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

	Yplot_init( 0, "graph", "graph_par","graph_stp","graph_lap","graph_T",
	    "graph_prob",NULL ) ;

	attmaxG = compute_attprcel() ;


	/* try a restart if requested */
	sprintf(filename, "%s.mres", cktNameG ) ;
	fp = TWOPEN( filename , "r", NOABORT ) ;
	if( !( restartG && fp) ) {

	    /* ****** NORMAL CASE - NO RESTART ********** */
	    if( restartG ){
		ASSERT( restartG && !fp, "main","restart logic error\n" ) ;
		sprintf(YmsgG,
		    "restart file:%s wasn't present but restart requested!\n",
		    filename ) ;
		M(ERRMSG,"main",YmsgG );
		M(MSG,NULL,"Must exit...\n\n" );
		closegraphics() ;
		YexitPgm( PGMFAIL ) ;
	    } else if (fp){
		ASSERT( !restartG && fp, "main","restart logic error\n" ) ;
		sprintf(YmsgG,
		"<%s.mres> present but restart was not requested in par file.\n",
		cktNameG ) ;
		M(WARNMSG,"main",YmsgG ) ;
	    }

	    /*** initialization. JL ***/
	    init_control(TRUE);     /*** set move generation controller. ***/
	    initStatCollection() ;  /* init funccost penalty recorder */

	    OUT1("High temperature randomization and " ) ;
	    OUT1("controller initialization...\n") ;
	    iterationG = -2 ;
	    TG = 1.0e30;	/*** set to VERY HIGH temperature. ***/
	    attempts = (attmaxG > 3 * numcellsG) ? attmaxG : 3 * numcellsG ; 
	    /* call all controllers to set factors to initial points */
	    /* statistics should all be zero */
	    getStatistics( &totFunc, &totPen, &avgdTime, &avgdFunc ) ;
	    coreFactorG = calc_init_coreFactor() ;
	    lapFactorG = calc_init_lapFactor( totFunc, totPen ) ;
	    timeFactorG = calc_init_timeFactor( avgdFunc, avgdTime ) ;

	    uloop( attempts ) ;


	    /* -----------------------------------------------------------
	       Call config2 to reconfigure core based on estimated routing 
	       area. Take border bins into account. 
	       ----------------------------------------------------------- */
	    config2( maxBinXG-1, maxBinYG-1, wire_red_ratioS );


	    /* since we now are configured scrap long nets */
	    scrapnet() ;

	    funccostG = findcost() ;
	    OUT1("\n\n\n");
	    OUT3("parameter adjust: route:%d  penalty:%d\n",
		funccostG, penaltyG ) ;
	    FLUSHOUT();

	    /* call controllers a second time to update factors */
	    /* set negative feedback controllers */
	    getStatistics( &totFunc, &totPen, &avgdTime, &avgdFunc ) ;
	    lapFactorG = calc_init_lapFactor( totFunc, totPen ) ;
	    timeFactorG = calc_init_timeFactor( avgdFunc, avgdTime ) ;
	    funccostG = findcost() ; /* calc initial costs with factors */

	    attempts = attmaxG ;
	    iterationG = -1 ;
	    uloop( attempts ) ;

	    /* Tell the user the expected outcome */
	    OUT2("\n\nThe average  random  wirelength is: %10.0f\n",
		avg_funcG ) ;
	    OUT2("The expected optimum wirelength is: %10.0f\n\n",
		avg_funcG/wire_red_ratioS ) ;

	    funccostG  = findcost() ;
	    OUT1("\n\n\nThe New Cost Values after readjustment:\n\n");
	    OUT3("route:%d  penalty:%d\n\n\n", funccostG, penaltyG ) ;
	    FLUSHOUT();


	    OUT1("Statistic collection...\n") ;
	    attempts = attmaxG ;
	    iterationG = 0 ;
	    uloop( attempts ) ;

	    G( draw_the_data() ) ;

	} else {

	    /* ****** RESTART CASE ******* */
	    OUT2("reading data from %s\n", filename ) ;
	    restartG = TW_oldinput( fp ) ;
	    if( !(restartG) ){
		M(ERRMSG,"main","Restart aborted because of error\n");
		M(MSG,NULL, "Catastrophic error.  Must exit...\n");
		closegraphics() ;
		YexitPgm( PGMFAIL ) ;
	    }
	    funccostG = findcost() ;
	    OUT2("\n\n\nTHE ROUTE COST OF THE CURRENT PLACEMENT: %d\n"
						      , funccostG ) ;
	    OUT2("\n\nTHE PENALTY OF THE CURRENT PLACEMENT: %d\n" ,
							 penaltyG ) ;
	    FLUSHOUT() ;

	    /* startup graphics */
	    G( set_graphic_context( PLACEMENT ) ) ;
	    G( draw_the_data() ) ;

	}   /* end RESTART CASE */

	if( !cost_onlyG ) {
	    OUT1("\n\nTimberWolf Cell Placement Ready for Action\n\n");
	    /* allow multi cell moves */
	    G( set_graphic_context( PLACEMENT ) ) ;
	    utemp( attmaxG, TRUE ) ;
	}

	funccostG = findcost() ;
	rememberWire = funccostG ;
	rememberPenal = binpenalG ;


	finalout() ;
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
	closegraphics() ;
    }
    OUT1("\n\n************************************ \n\n");
    OUT1("TimberWolf has completed its mission\n");
    OUT1("\n\n************************************ \n\n");

    if( verboseG ){
	Yprint_stats( stdout ) ;
    }
    Yprint_stats( fpoG ) ;
    Yplot_close() ;
    writeResults( rememberWire, rememberPenal, rememberRand );
    if( sc_output() ){
	create_sc_output() ;
    }
    Ymessage_close() ;
    YexitPgm(OK) ;

} /* end main routine */

INT yaleIntro() 
{
    fprintf(fpoG,"\n%s\n",YmsgG) ;
    fprintf(fpoG,"Authors: Carl Sechen, Bill Swartz, Kai-Win Lee\n");
    fprintf(fpoG,"         Dahe Chen, and Jimmy Lam\n"); 
    fprintf(fpoG,"         Yale University\n");

    fprintf(stdout,"\n%s\n",YmsgG) ;
    fprintf(stdout,"Authors: Carl Sechen, Bill Swartz, Kai-Win Lee\n");
    fprintf(stdout,"         Dahe Chen, and Jimmy Lam\n"); 
    fprintf(stdout,"         Yale University\n");

} /* end yaleIntro */

/* this routine takes information about run and write to history file */
/* to accumulate data about runs */
writeResults( wire, penal, rand )
INT wire, penal, rand ;
{
    FILE *fpdebug ;
    INT left_side, right_side, bottom_side, top_side ;
    char filename[LRECL] ;

    funccostG = findcost() ;
    sprintf( filename,"%s.history", cktNameG ) ;
    fpdebug = TWOPEN( filename, "a", ABORT ) ;
    /* find core region */
    find_core_boundary( &left_side, &right_side, &bottom_side, &top_side);
    bdxlengthG = right_side - left_side ;
    bdylengthG = top_side - bottom_side ;
    fprintf( fpdebug, 
	"%4.2le\t%4.2le\t%4.2le\t%4.2le\t%d\t%d\t%d\t%4.2le\t%34.32le\n", 
	(DOUBLE) wire, (DOUBLE) penal,
	(DOUBLE) funccostG, (DOUBLE) penaltyG, bdxlengthG, bdylengthG, 
	rand, (DOUBLE) bdxlengthG * bdylengthG, wire_red_ratioS ) ;
    TWCLOSE( fpdebug ) ;
} /* end writeResults */

/* close graphics window on fault */

INT closegraphics( )
{
    if( doGraphicsG ){
	G( TWcloseGraphics() ) ;
    }
} /* end closegraphics */

/* give user correct syntax */

VOID syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, 
       "\nTimberWolfMC [-dnpvw] designName [windowId] \n" ) ;
   M(MSG,NULL,YmsgG ) ; 
   M(MSG,NULL,"\twhose options are zero or more of the following:\n");
   M(MSG,NULL,"\t\td - prints debug info and performs extensive\n");
   M(MSG,NULL,"\t\t    error checking\n");
   M(MSG,NULL,"\t\tn - no graphics - the default is to open the\n");
   M(MSG,NULL,"\t\t    display and output graphics to an Xwindow\n");
   M(MSG,NULL,"\t\tp - place pads only - read core placement.\n");
   M(MSG,NULL,"\t\tv - verbose mode - writes to screen\n");
   M(MSG,NULL,"\t\tw - parasite mode - user must specify windowId\n");
   YexitPgm(PGMFAIL);
} /* end syntax */

/* used to TimberWolfMC recursively for the overflow case */
/* returns windowid if graphics are on and window is passed */
BOOL get_arg_string( arguments )
char *arguments ;
{
    char temp[LRECL] ; /* used to build strings */
    INT  window ;      /* current window ID */

    sprintf( arguments, "%s -", argv0G ) ;
    if( scale_dataG ){
	strcat( arguments, "o" ) ;
    }
    if( debugS ){
	strcat( arguments, "d" ) ;
    }
    if( verboseG ){
	strcat( arguments, "v" ) ;
    }
    if( quickrouteG ){
	strcat( arguments, "q" ) ;
    }
    window = 0 ;
    if( doGraphicsG ){
	/* save state of graphics and get window id */
	G( window = TWsaveState() ) ;
	if( window ){
	    strcat( arguments, "w" ) ;
	}
    } else if( batchS ){
	strcat( arguments, "b" ) ;
    } else {
	strcat( arguments, "n" ) ;
    }
    strcat( arguments, " " ) ;
    /* now build the values */
    if( scale_dataG ){
	sprintf( temp, "%d ", scale_dataG ) ;
	strcat( arguments, temp ) ;
    }
    /* now the design name */
    strcat( arguments, cktNameG ) ;
    /* now pass graphics window if necessary */
    if( window ){
	sprintf( temp, " %d", window ) ;
	strcat( arguments, temp ) ;
	return(TRUE) ;
    } else {
	return(FALSE) ;
    }
} /* end function get_arg_string() */

BOOL get_batch_mode()
{
    return( batchS ) ;
} /* end get_batch_mode */

set_wiring_reduction( reduction )
DOUBLE reduction ;
{
    wire_red_ratioS = reduction ;
} /* set_wiring_reduction */
