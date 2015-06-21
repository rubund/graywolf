/*
 *   Copyright (C) 1989-1992 Yale University
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
DESCRIPTION:TimberWolfSC's main function.
CONTENTS:   main( argc , argv )
		INT argc ;
		char *argv[] ;
	    initialize_global_router1()
	    initialize_global_router()
	    execute_global_router()
	    init_utemp()
	    install_swap_pass_thrus( netptr )
		PINBOXPTR netptr ;
	    incorporate_ECOs()
DATE:	    Mar 27, 1989 
REVISIONS:  Oct 20, 1990 - fixed problem with graphics close.
	    Sat Dec 15 22:08:21 EST 1990 - modified pinloc values
		so that it will always be positive.
	    Sun Jan 20 21:47:52 PST 1991 - ported to AIX.
	    Wed Jan 23 02:41:07 PST 1991 - output now at density
		if requested.
	    Fri Jan 25 23:48:09 PST 1991 - avoid crashing if empty
		rows exist.
	    Fri Feb 15 15:15:52 EST 1991 - place pads here so that
		initialize_global_router1 save correct state.
	    Thu Mar  7 01:53:39 EST 1991 - now call SGGR automatically.
	    Tue Mar 12 17:10:47 CST 1991 - fixed DN10000 warnings.
	    Fri Mar 15 00:39:41 CST 1991 - now pick best global
		route based on area.
	    Thu Apr 18 01:48:52 EDT 1991 - now output feed percentage
		so user can use this in the next run.
	    Wed Jun  5 16:37:52 CDT 1991 - during global routing
		maintain side.
	    Thu Aug  1 16:12:05 CDT 1991 - now save orient
		correctly during global routing.
	    Thu Aug 22 22:12:58 CDT 1991 - now let gate_array mode
		run both row evenning and not.
	    Fri Sep  6 15:14:47 CDT 1991 - fixed graphics calls
		to make it less confusing to user.
	    Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.
	    Fri Sep 27 20:54:48 EDT 1991 - fixed ECO problem.
	    Thu Nov  7 23:03:57 EST 1991 - fixed problem with picking
		best global route criteria and added new row evener.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) main.c (Yale) version 4.38 5/15/92" ;
#endif

#define MAIN_VARS  
#include "standard.h"
#include "main.h"
#include "groute.h"
#include "readpar.h"
#include "parser.h"
#include "feeds.h"
#include "pads.h"

#include <string.h>
#include <signal.h>
#include <yalecad/cleanup.h>
#include <yalecad/message.h>

#include "config-build.h"

#define EXPECTEDMEMORY         (512 * 1024) 

/* global variables definitions */
INT row_extentG ;
INT **bin_configG ;
INT left_row_boundaryG ;
INT save_abs_min_flagG ;
INT actual_feed_thru_cells_addedG = 0 ;
BOOL orientation_optimizationG = FALSE ;

/* global variable references */
extern BOOL do_fast_globalG ;
extern BOOL no_row_lengthsG ;
extern BOOL do_not_even_rowsG ;
extern BOOL ignore_crossbusesG ;
extern BOOL output_at_densityG ;
extern BOOL absolute_minimum_feedsG ;
extern BOOL glob_route_only_crit_netsG ;
extern BOOL vertical_track_on_cell_edgeG ;
extern BOOL route_padnets_outsideG ;
extern BOOL call_row_evenerG ;
extern BOOL placement_improveG ;
extern BOOL ignore_feedsG ;
extern INT ECOs_existG ;
extern INT spacer_widthG ;
extern INT longest_row_lengthG ;
extern INT largest_delta_row_lenG ;
extern INT total_row_lengthG ;

/* static variables */
static INT routing_loopS ;
static INT *save_cell_xS ;
static INT *save_cell_yS ;
static INT *save_cell_bS ;
static INT *save_cell_cS ;
static INT *save_desireS ;
static INT *save_orientS ;
static INT *save_orig_desireS ;
static char *twdirS ; 
static INT num_feeds_addedS ;/* number of feeds added on best iteration */
static DOUBLE ave_row_sepS ; /* the row separation for a run */


main( argc , argv )
INT argc ;
char *argv[] ;
{


FILE *fp ;
DOUBLE quality_value();
char filename[LRECL] ;
INT ll, rr, bb, tt ;
INT bdxlen , bdylen ;
INT block ;
INT yaleIntro() ;
INT cx, cy, cl, cr, cb, ct, cell ;
char *ptr ;
char *Ystrclone() ;
BOOL debug ;
BOOL parasite ;
BOOL windowId ;
BOOL verbose ;
INT arg_count ;
char *Ygetenv() ;

/* ********************** start initialization *********************** */
/* start up cleanup handler */
YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

Yinit_memsize( EXPECTEDMEMORY ) ;

USER_INITIALIZATION() ;

if( argc < 2 || argc > 4 ){
    syntax() ;
} else {
    debug      = FALSE ;
    parasite   = FALSE ;
    verbose    = FALSE ;
#ifndef NOGRAPHICS
    windowId   = 0 ;
    doGraphicsG = TRUE ;
#else /* NOGRAPHICS case */
    doGraphicsG = FALSE ;
#endif /* NOGRAPHICS */
    arg_count = 1 ;
    if( *argv[1] == '-' ){
	for( ptr = ++argv[1]; *ptr; ptr++ ){
	    switch( *ptr ){
	    case 'd':
		debug = TRUE ;
	        break ;
	    case 'n':
	        doGraphicsG = FALSE ;
	        break ;
	    case 'v':
	        verbose = TRUE ;
	        break ;
	    case 'w':
	        parasite = TRUE ;
	        break ;
	    default:
		sprintf( YmsgG,"Unknown option:%c\n", *ptr ) ;
		M(ERRMSG,"main",YmsgG);
		syntax() ;
	    }
	}
	YdebugMemory( debug ) ;

	cktNameG = Ystrclone( argv[++arg_count] );

	/* now tell the user what he picked */
	M(MSG,NULL,"\n\nTimberWolfSC switches:\n" ) ;
	if( debug ){
	    YsetDebug( TRUE ) ;
	    M(MSG,NULL,"\tdebug on\n" ) ;
	} 
	if( doGraphicsG ){
	    M(MSG,NULL,"\tGraphics mode on\n" ) ;
	} else {
	    M(MSG,NULL,"\tGraphics mode off\n" ) ;
	}
	if( parasite ){
	    M(MSG,NULL,"\tTimberWolfSC will inherit window\n" ) ;
	    /* look for windowid */
	    if(argc != 4){
		M(ERRMSG,"main","Need to specify windowID\n" ) ;
		syntax() ;

	    } else {
		G( windowId = atoi( argv[++arg_count] ) ) ;
	    } 
	}
	if( verbose ){
	    M(MSG,NULL,"\tVerbose mode on\n" ) ;
	    Ymessage_mode( M_VERBOSE ) ;
	}
	M(MSG,NULL,"\n" ) ;
    } else if( argc == 2 ){
	/* order is important here */
	YdebugMemory( FALSE ) ;
	cktNameG = Ystrclone( argv[1] );

    } else {
	syntax() ;
    }
}
sprintf( filename , "%s.out" , cktNameG ) ;
fpoG = TWOPEN( filename, "w", ABORT ) ;
Ymessage_init( fpoG ) ;  /* send all the messages to a file normally */
YinitProgram( "TimberWolfSC", "v6.0", yaleIntro );
/* ********************** end initialization ************************* */

readParFile();

/* check to see if twdir is set so we can run SGGR */
if( SGGRG ){
    if( !(twdirS = TWFLOWDIR)){
       M( ERRMSG,main,"TWDIR environment variable not set.\n");
       M( ERRMSG,NULL,"Please set it to TimberWolf root directory\n");
       M( ERRMSG,NULL,"SGGR cannot automatically be run after annealing\n");
    }
}
G( initGraphics( argc, argv, windowId ) ) ;

Yset_random_seed( (INT) randomSeedG ) ; 
fprintf( fpoG, "\nThe random number generator seed is: %u\n\n\n", 
						randomSeedG ) ;

/* 
    get a pointer to blockfile
*/
sprintf( filename , "%s.blk" , cktNameG ) ;
fp = TWOPEN( filename , "r" , ABORT ) ;
readblck( fp ) ;
TWCLOSE( fp ) ;

maxCellOG = 0 ;
for( block = 1 ; block <= numRowsG ; block++ ) {
    if( barrayG[block]->borient == 2 ) {
	maxCellOG = 1 ;
	break ;
    }
}


if( rowsG > 0 ) {
    sprintf( filename , "%s.scel" , cktNameG ) ;
} else {
    sprintf( filename , "%s.cel" , cktNameG ) ;
}
/* avoid crashes later on */
if( spacer_widthG == -1 && (rowsG > 0 || gate_arrayG) ){
    M( ERRMSG, "main", 
    "Neither spacer width nor feedthru width was specified in .par file\n" ) ;
    M( ERRMSG, NULL, "FATAL:must exit\n\n" ) ;
}

fp = TWOPEN( filename , "r" , ABORT ) ;
readcell( fp ) ;
TWCLOSE( fp ) ;

/* 
    get a pointer to the netlist input file 
*/
sprintf( filename , "%s.net" , cktNameG ) ;
fp = TWOPEN( filename , "r", NOABORT  ) ;
readnets( fp ) ;
if( fp ) {
    TWCLOSE( fp ) ;
}


if ( Equal_Width_CellsG && file_conversionG ) {
     closegraphics() ;
     calc_cells_width();
     fprintf(stderr,"\n***************************************************");
     fprintf(stderr,"\nThe file stdcell.comp has the converted equal cells");
     fprintf(stderr,"\n***************************************************");
     fprintf(stderr,"\n\n");
     YexitPgm(PGMOK);     

}

iterationG = 0 ;

tt = INT_MIN ;
bb = INT_MAX ;
rr = INT_MIN ;
ll = INT_MAX ;
for( block = 1 ; block <= numRowsG ; block++ ) {
    cx = barrayG[block]->bxcenter ;
    cy = barrayG[block]->bycenter ;
    cl = barrayG[block]->bleft;
    cr = barrayG[block]->bright;
    cb = barrayG[block]->bbottom;
    ct = barrayG[block]->btop;
    if( cx + cr > rr ) {
	rr = cx + cr ;
    }
    if( cx + cl < ll ) {
	ll = cx + cl ;
    }
    if( cy + ct > tt ) {
	tt = cy + ct ;
    }
    if( cy + cb < bb ) {
	bb = cy + cb ;
    }
}
blkxspanG = rr - ll ;
blkyspanG = tt - bb ;

left_row_boundaryG = ll ;
row_extentG = rr - ll ;

fprintf(fpoG,"block x-span:%d  block y-span:%d\n",blkxspanG,blkyspanG);

for( cell = 1 ; cell <= lastpadG ; cell++ ) {
    if( cell > numcellsG - extra_cellsG && cell <= numcellsG ) {
	continue ;
    }
    cx = carrayG[cell]->cxcenter ;
    cy = carrayG[cell]->cycenter ;
    cl = carrayG[cell]->tileptr->left;
    cr = carrayG[cell]->tileptr->right;
    cb = carrayG[cell]->tileptr->bottom;
    ct = carrayG[cell]->tileptr->top;
    if( cx + cr > rr ) {
	rr = cx + cr ;
    }
    if( cx + cl < ll ) {
	ll = cx + cl ;
    }
    if( cy + ct > tt ) {
	tt = cy + ct ;
    }
    if( cy + cb < bb ) {
	bb = cy + cb ;
    }
}
bdxlen = rr - ll ;
bdylen = tt - bb ;

lrtxspanG = rr ;
lrtyspanG = tt ;



if( Equal_Width_CellsG ) {
    binpenConG = 0 ;
    roLenConG  = 0 ;
} else {
    binpenConG = 1.0 ;
    roLenConG  = 6.0 ;
}
calc_init_timeFactor() ;
fprintf(fpoG,"Using default value of bin.penalty.control:%f\n",
					    binpenConG ) ;
pairArrayG = NULL ;

funccostG = findcost() ;

fprintf( fpoG , "bdxlen:%d  bdylen:%d\n", bdxlen , bdylen ) ;
fprintf( fpoG , "l:%d  t:%d  r:%d  b:%d\n", ll , tt , rr , bb ) ;
			    
fprintf( fpoG, "\n\n\nTHIS IS THE ROUTE COST OF THE ");
fprintf( fpoG, "CURRENT PLACEMENT: %d\n" , funccostG ) ;
fprintf( fpoG, "\n\n\nTHIS IS THE PENALTY OF THE ") ;
fprintf( fpoG , "CURRENT PLACEMENT: %d\n" , penaltyG ) ;
fflush( fpoG ) ;

if( intelG && !ignore_crossbusesG ) {
    handle_crossbuses() ;
}

G( init_heat_index() ) ;
G( check_graphics(TRUE) ) ;

fflush(fpoG);
fflush(stdout);

if( orientation_optimizationG ) {
    costonlyG = FALSE ;
}

if( ECOs_existG ) {
    costonlyG = TRUE ;
    printf("ECOs are being incorporated as requested\n");
    fprintf(fpoG,"ECOs are being incorporated as requested\n");
    incorporate_ECOs() ;
}

if( costonlyG ) {
    orientation_optimizationG = TRUE ;
    utemp() ;
} else {
    init_utemp() ;
    utemp() ;
}


fprintf( fpoG , "\nStatistics:\n");
fprintf( fpoG , "Number of Cells: %d\n", numcellsG );
fprintf( fpoG , "Number of Pads: %d \n", numtermsG - numpadgrpsG );
fprintf( fpoG , "Number of Nets: %d \n", numnetsG ) ;
fprintf( fpoG , "Number of Pins: %d \n", maxtermG ) ;
fprintf( fpoG , "Number of PadGroups: %d \n", numpadgrpsG );
fprintf( fpoG , "Number of Implicit Feed Thrus: %d\n",
				implicit_feed_countG++ ) ;
fprintf( fpoG , "Number of Feed Thrus Added: %d\n", num_feeds_addedS ) ;
fprintf( fpoG , "Feed Percentage: %4.2f%%\n",
    100.0 * (DOUBLE) (num_feeds_addedS * fdWidthG) /
    (DOUBLE) total_row_lengthG ) ;
fprintf( fpoG , "Average Row Separation: %4.2f\n",
    ave_row_sepS ) ;

if( intelG ) {
    fprintf( fpoG , "Checking violations at the end\n");
    check_violations() ;
}

Yprint_stats(fpoG);
/* TWCLOSE(fpoG) ; */  /* Handled by Ymessage_close() in YexitPgm() */

closegraphics() ;

YexitPgm(PGMOK);

} /* end main */


initialize_global_router1()
{

INT cell , row ;

save_cell_xS = (INT *) 
	Ysafe_malloc( (1 + numcellsG + numtermsG) * sizeof(INT));
save_cell_yS = (INT *) 
	Ysafe_malloc( (1 + numcellsG + numtermsG) * sizeof(INT));
save_cell_bS = (INT *) 
	Ysafe_malloc( (1 + numcellsG + numtermsG) * sizeof(INT));
save_cell_cS = (INT *) 
	Ysafe_malloc( (1 + numcellsG + numtermsG) * sizeof(INT));
save_orientS = (INT *) 
	Ysafe_malloc( (1 + numcellsG + numtermsG) * sizeof(INT));
save_desireS = (INT *) 
		Ysafe_malloc( (1 + numRowsG) * sizeof(INT));
save_orig_desireS = (INT *) 
		Ysafe_malloc( (1 + numRowsG) * sizeof(INT));
for( cell = 1 ; cell <= lastpadG ; cell++ ) {
    save_cell_xS[cell] = carrayG[cell]->cxcenter ; 
    save_cell_yS[cell] = carrayG[cell]->cycenter ; 
    save_cell_bS[cell] = carrayG[cell]->cblock ; 
    save_cell_cS[cell] = carrayG[cell]->cclass ; 
    save_orientS[cell] = carrayG[cell]->corient ; 
}
for( row = 1 ; row <= numRowsG ; row++ ) {
    save_desireS[row] = barrayG[row]->desire ;
    save_orig_desireS[row] = barrayG[row]->orig_desire ;
}

return ;
}


initialize_global_router()
{

CBOXPTR ptr ;
PINBOXPTR pinptr, pin, cnetptr , netptr ;
INT cell , row , net ;
BOOL coreConnection ;


rebuild_nextpin() ;
insert_row(0) ;

for( cell = 1 ; cell <= lastpadG; cell++ ) {
    carrayG[cell]->cxcenter = save_cell_xS[cell] ;
    row = carrayG[cell]->cblock = save_cell_bS[cell] ;
    if( row != 0 ){
	carrayG[cell]->cycenter = barrayG[row]->bycenter ;
    } else {
	carrayG[cell]->cycenter = save_cell_yS[cell] ;
    }
    carrayG[cell]->cclass = save_cell_cS[cell] ;
    carrayG[cell]->corient = (char) save_orientS[cell] ;
}

for( cell = 1 ; cell <= lastpadG ; cell++ ) {
    ptr = carrayG[cell] ;
    if( cell > numcellsG ) {
	for( pinptr = ptr->pins; pinptr ; pinptr = pinptr->nextpin ) {
	    if( route_padnets_outsideG ){
		/* look at each net on the pads to see if it has a */
		/* connection in the core */
		coreConnection = FALSE ;
		for( pin = netarrayG[pinptr->net]->pins; pin; pin = pin->next ){
		    if( pin->cell <= numcellsG ){
			coreConnection = TRUE ;
			break ;
		    }
		}
		if(!(coreConnection )){
		    /* no connection to the core we can route this in */
		    /* the top channel */
		    pinptr->row = numRowsG + 1 ;
		    pinptr->pinloc = BOTCELL ;
		    /* now go on to the next pin */
		    continue ;
		}
	    }
	    /* set the pinlocation for the pads and macros */
	    if( ptr->padptr->padside ) {
		if( pinptr->row > numRowsG ) {
		    pinptr->pinloc = BOTCELL ;
		} else {
		    pinptr->pinloc = TOPCELL ;
		}
	    }
	}
    } else {
	for( pinptr = ptr->pins; pinptr ; pinptr = pinptr->nextpin ) {
	    if( pinptr->typos[(ptr->corient)%2] > 0 ) {
		pinptr->pinloc = TOPCELL ;
	    } else if(pinptr->typos[(ptr->corient)%2] < 0 ){
		pinptr->pinloc = BOTCELL ;
	    } else {
		pinptr->pinloc = NEITHER ;
	    }
	}
    }
}
for( row = 1 ; row <= numRowsG ; row++ ) {
    barrayG[row]->desire = save_desireS[row] ;
    barrayG[row]->orig_desire = save_orig_desireS[row] ;
}
no_row_lengthsG = 1 ;
findunlap(0) ;
no_row_lengthsG = 0 ;

return ;
}


execute_global_router()
{

PINBOXPTR netptr ;
INT i , j, row ;
INT loop_boundary ;
INT temp ;
INT core_height ;
INT core_width ;
INT total_row_height ;
INT best_tracks ;
DOUBLE area ;
DOUBLE best_area ;
BOOL decision ;

char command[LRECL] ;

if( gate_arrayG && vertical_track_on_cell_edgeG ) {
    findunlap(0) ;
    fprintf(fpoG,"Removed %d redundant implicit feeds\n",
				rm_overlapping_feeds() ) ;
}

loop_boundary = 4 ;
routing_loopS = 8 ;

/* added by Carl 24 Jan 91  */
total_row_height = 0 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    if( pairArrayG[row][0] < 1 ) {
	fprintf(fpoG,"Trying to fix empty row problem ...") ;
	printf("Trying to fix empty row problem ...") ;
	fflush(fpoG);
	fflush(stdout);
	even_the_rows(0,FALSE) ;
	if( pairArrayG[row][0] < 1 ) {
	    /* take even more drastic measures */
	    even_the_rows(0,TRUE) ;
	}
	for( row = 1 ; row <= numRowsG ; row++ ) {
	    if( pairArrayG[row][0] < 1 ) {
		fprintf(fpoG,"Global router cannot handle a row without cells\n");
		printf("Global router cannot handle a row without cells\n");
		YexitPgm(PGMFAIL);
	    }
	}
	fprintf(fpoG," fixed\n");
	printf(" fixed\n");
	fflush(fpoG);
	fflush(stdout);
	break ;
    }
    /* accumulate the total row height */
    total_row_height += barrayG[row]->btop - barrayG[row]->bbottom ;
}
/*  end  */

if( swap_netG > 0 ) {
    /*  there are TW_PASS_THRU pins on a swap-able gate  */
    /*  we must add them to the implicit list on the relevant cell */
    netptr = netarrayG[swap_netG]->pins ;
    for( ; netptr ; netptr = netptr->next ) {
	install_swap_pass_thrus( netptr ) ;
    }
    netarrayG[swap_netG]->ignore = -1 ;
}

if( glob_route_only_crit_netsG ) {
    route_only_critical_nets() ;
} else {
    elim_nets(1) ; /* nets which are NOT to be global routed */
}

actual_feed_thru_cells_addedG = 0 ;

initialize_global_router1() ;
if( SGGRG ) {
    initialize_global_router() ;
    coarseglb() ; /* this function returns "early" if SGGR */
    /* now place pads for the final time using real cell positions for core */
    /* boundary to insure we don't have overlap */
    setVirtualCore( TRUE ) ;
    placepads() ;
    output() ;
    fprintf(fpoG,"\nTimberWolfSC placement is complete; Now ready for execution of SGGR\n");

    Yprint_stats(fpoG);

    closegraphics() ;

    if( twdirS ){
	Ymessage_mode( M_VERBOSE ) ;
	M( MSG, NULL, "\n\nPlacement phase complete...\n\n" ) ;
	M( MSG, NULL, "\n\nNow calling SGGR...\n\n" ) ;

	TWCLOSE(fpoG) ;
	sprintf( command, "%s/bin/SGGR %s", twdirS, cktNameG  ) ;
	Ysystem( "SGGR", ABORT, command, NULL ) ;
    }
    else TWCLOSE(fpoG) ;

    YexitPgm(PGMOK);
}

best_area = DBL_MAX ;

USER_NEXT_METER() ;
USER_SEND_VALUE( routing_loopS ) ;
save_abs_min_flagG = absolute_minimum_feedsG ;

/* now place pads using real cell positions for core */
/* boundary to insure we don't have overlap */
setVirtualCore( TRUE ) ;
placepads() ;
/* draw the data */
G( check_graphics(TRUE) ) ;

if( !(placement_improveG )){
    /* limit the number of cases since we won't move cells */
    routing_loopS = 4 ;
    call_row_evenerG = FALSE ;
}

/* from here on, the pad side must be retained during global routing */
placepads_retain_side( TRUE ) ;
for( j = 1 ; j <= routing_loopS ; j++ ) {

    if( j == routing_loopS || j == loop_boundary ) {
	absolute_minimum_feedsG = TRUE ;
    } else {
	absolute_minimum_feedsG = save_abs_min_flagG ;
    }
    if( j <= loop_boundary ) {
	if( call_row_evenerG ){
	    do_not_even_rowsG = FALSE ;
	    fprintf(fpoG,"TimberWolfSC did call even_the_rows()\n");
	} else {
	    do_not_even_rowsG = TRUE ;
	    fprintf(fpoG,"TimberWolfSC did NOT call even_the_rows()\n");
	} 
    } else {
	do_not_even_rowsG = FALSE ;
	fprintf(fpoG,"TimberWolfSC did call even_the_rows()\n");
    }
    if( absolute_minimum_feedsG ) {
	fprintf(fpoG,"TimberWolfSC using absolute_minimum_feeds\n");
    }

    tracksG = 0 ;
    initialize_global_router() ;

    coarseglb() ;
    if( globe() == 0 ) {
	fprintf(fpoG,"WARNING: refine_fixed_placement failed\n");
	printf("WARNING: refine_fixed_placement failed\n");
	fflush(fpoG);
	fflush(stdout);
	USER_INCR_METER() ;
	continue ;
    }
    if( output_at_densityG ){
	density() ;
    }
    placepads() ;
    /* draw the data */
    G( check_graphics(TRUE) ) ;

    core_width = barrayG[1]->desire ;
    for( row = 2 ; row <= numRowsG ; row++ ) {
	if( core_width < barrayG[row]->desire ) {
	    core_width = barrayG[row]->desire ;
	}
    }
    core_width += largest_delta_row_lenG ;
    core_height = tracksG * track_pitchG + total_row_height ;
    area = (DOUBLE) core_height * (DOUBLE) core_width ;
    if( area < best_area ){
	best_area = area ;
	decision = TRUE ;
	best_tracks = tracksG ;
    } else {
	decision = FALSE ;
    }

    if( decision ) {
	fprintf(fpoG,"THIS G. ROUTING IS BEING SAVED AS BEST SO FAR\n");
	fprintf(fpoG,"\nFINAL NUMBER OF ROUTING TRACKS: %d\n\n", 
						    tracksG);
	for( i = 1 ; i <= numChansG ; i++ ) {
	    fprintf(fpoG,"MAX OF CHANNEL:%3d  is: %3d\n", i 
					    , maxTrackG[i]);
	}
	outpins() ;
	output() ;
	print_paths() ;
	best_tracks = tracksG ;
	/* save this for final statistics */
	num_feeds_addedS = actual_feed_thru_cells_addedG ;
	/* save the effective row separation */
	ave_row_sepS = (DOUBLE) (tracksG * track_pitchG) /
			(DOUBLE) total_row_height ;
    }
    final_free_up() ;
    USER_INCR_METER() ;
}
fprintf(fpoG,"\n\n***********************************************\n");
fprintf(fpoG,"*ACTUAL* FINAL NUMBER OF ROUTING TRACKS: %d\n", 
						    best_tracks);
fprintf(fpoG,"***********************************************\n\n");
fflush(fpoG);
fprintf(stdout,"\n\n***********************************************\n");
fprintf(stdout,"*ACTUAL* FINAL NUMBER OF ROUTING TRACKS: %d\n", 
						    best_tracks);
fprintf(stdout,"***********************************************\n\n");
fflush(stdout);


return ;
}





init_utemp()
{

INT row , bin ;

bin_configG = (INT **) Ysafe_malloc( (1 + numRowsG) * sizeof(INT *) ) ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    bin_configG[row] = (INT *) Ysafe_malloc( (1 + numBinsG) * sizeof(INT) );
    for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	bin_configG[row][bin] = 0 ;
    }
}

return ;
}





install_swap_pass_thrus( netptr )
PINBOXPTR netptr ;
{

CBOXPTR ptr ;
IPBOXPTR imptr ;
PINBOXPTR termptr ;
INT impxpos , impypos , cell ;
char *pname ;

/*  code borrowed from readcell()  */
/*  fscanf( fp , " %d %d " , &impxpos , &impypos ) ;  */
/*  impxpos and impypos are rel. to cell center -- to
    get these we need to find the relevant pin on
    the cell  */
cell = netptr->cell ;
ptr = carrayG[cell] ;
for( termptr = ptr->pins ;termptr ; termptr = termptr->nextpin ) {
    if( termptr == netptr ) {
	impxpos = termptr->txpos[0] ;
	impypos = termptr->typos[0] ;
	fprintf(fpoG,"FOUND the connection in install_swap...\n");
	break ;
    }
}
imptr = ( IPBOXPTR )Ysafe_malloc( sizeof( IPBOX ) ) ;
imptr->pinname = (char *) Ysafe_malloc(
			(strlen( netptr->pinname ) + 1 ) * 
						sizeof( char ) ) ;
imptr->eqpinname = (char *) Ysafe_malloc(
			(strlen( netptr->eqptr->pinname ) + 1 ) * 
						sizeof( char ) ) ;
sprintf( imptr->pinname , "%s" , netptr->pinname ) ;
sprintf( imptr->eqpinname , "%s" , netptr->eqptr->pinname ) ;

imptr->txpos = impxpos ;
imptr->cell  = cell ;
imptr->terminal = ++last_pin_numberG ;
imptr->next = ptr->imptr ;
ptr->imptr = imptr ;

if( impypos > 0 ) { /* swap the pinnames */
    pname = imptr->pinname ;
    imptr->pinname = imptr->eqpinname ;
    imptr->eqpinname = pname ;
}

return ;
}

incorporate_ECOs()
{

PINBOXPTR termptr , netptr ;
INT *nets, xspot, yspot , i , net , cell , count, maxpins ;
INT orient , row , xmin, xmax, ymin, ymax , x , y ;

xspot = 0 ;
yspot = 0 ;
maxpins = get_max_pin() ;
nets = (INT *) Ysafe_calloc( maxpins+1, sizeof(INT) ) ;
for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    if( carrayG[cell]->ECO_flag == 1 ) {
	printf("ECO added cell being processed: <%s> ",
					    carrayG[cell]->cname ) ;
	fprintf(fpoG,"ECO added cell being processed: <%s> ",
					    carrayG[cell]->cname ) ;
	termptr = carrayG[cell]->pins ;
	for( ; termptr ; termptr = termptr->nextpin ) {
	    net = termptr->net ;
	    for( i = 0 ; nets[i] != 0 ; i++ ) {
		if( nets[i] == net ) {
		    break ;
		}
	    }
	    if( nets[i] == 0 ) {
		nets[i] = net ;
	    }
	}
	/* nets[] is the list of unique nets for the cell */
	count = 0 ;
	for( i = 0 ; nets[i] != 0 ; i++ ) {
	    /* compute bounding boxes of the unique nets,
	       taking care to ignore ECO_added cells      */
	    /* ************ updated on 12/21/90 by Carl */
	    net  = nets[i] ;
	    if( !(netptr = netarrayG[net]->pins)) {
		continue ;
	    }
	    if( carrayG[ netptr->cell ]->ECO_flag == 1 ) {
		/* don't let ECO added cells influence the decision */
		xmin = INT_MAX ;
		xmax = INT_MIN ;
		ymin = INT_MAX ;
		ymax = INT_MIN ;
	    } else {
		xmin = xmax = netptr->xpos ;
		ymin = ymax = netptr->ypos ;
	    }
	    count = 0 ;
	    for( netptr = netptr->next ; netptr ; netptr = netptr->next ) {
		if( carrayG[ netptr->cell ]->ECO_flag == 1 ) {
		    continue ;
		    /* don't let ECO_added cells influence the decision */
		}
		x = netptr->xpos ;
		y = netptr->ypos ;
		if( x < xmin ) {
		    xmin = x ;
		} 
		if( x > xmax ) {
		    xmax = x ;
		}
		if( y < ymin ) {
		    ymin = y ;
		} 
		if( y > ymax ) {
		    ymax = y ;
		}
	    }
	    /* ************ */
	    xspot += (xmax + xmin) / 2 ;
	    yspot += (ymax + ymin) / 2 ; 
	    count++ ;
	}
	if( count >= 1 ){
	    xspot /= count ;
	    yspot /= count ;
	} else {
	    fprintf( stderr, "Incorrectly specified ECO cell:%s", 
		carrayG[cell]->cname ) ;
	}

	for( row = 1 ; row <= numRowsG ; row++ ) {
	    if( barrayG[row]->bycenter > yspot ) {
		break ;
	    }
	}
	if( row > numRowsG ) {
	    row = numRowsG ;
	} else if( row > 1 ) {
	    if( barrayG[row]->bycenter - yspot >
				yspot - barrayG[row-1]->bycenter ) {
		row-- ;
	    }
	}
	yspot = barrayG[row]->bycenter ;
	    
	carrayG[cell]->cxcenter = xspot ;
	carrayG[cell]->cycenter = yspot ;
	carrayG[cell]->cblock   = row   ;
	carrayG[cell]->ECO_flag = 0     ;

	printf(" ... placed in row:%d x:%d\n", row , xspot ) ;
	fprintf(fpoG," ... placed in row:%d x:%d\n", row , xspot ) ;

	orient = carrayG[cell]->corient ;
	for( termptr = carrayG[cell]->pins ; termptr ; 
					termptr = termptr->nextpin ) {
	    termptr->xpos = termptr->txpos[ orient/2 ] + xspot ;
	    termptr->ypos = termptr->typos[ orient%2 ] + yspot ;
	}
    }
}

return ;
}


/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, 
       "\nTimberWolfSC [-dnw] designName [windowId] \n" ) ;
   M(MSG,NULL,YmsgG ) ; 
   M(MSG,NULL,"\twhose options are zero or more of the following:\n");
   M(MSG,NULL,"\t\td - prints debug info and performs extensive\n");
   M(MSG,NULL,"\t\t    error checking\n");
   M(MSG,NULL,"\t\tn - no graphics - the default is to open the\n");
   M(MSG,NULL,"\t\t    display and output graphics to an Xwindow\n");
   M(MSG,NULL,"\t\tv - verbose mode - echo output to the screen\n");
   M(MSG,NULL,"\t\tw - parasite mode - user must specify windowId\n");
   YexitPgm(PGMFAIL);
} /* end syntax */
