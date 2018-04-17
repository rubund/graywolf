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
FILE:	    finalout.c                                       
DESCRIPTION:control loop after end of the simulated annealing algorithm.
CONTENTS:   finalout()
DATE:	    Feb 5, 1988 
REVISIONS:  Dec 3, 1988 - added end of program for partition strategy.
			- added timing driven info to output.
	    Dec 4, 1988 - added prnt_cost to make output easier
			- put addition and subtraction of pins in 
			    conditional compile.
	    Mar 2, 1989 - removed annealing code.
			- added graphics wait loop.
	    Mar 11,1989 - added graphics conditional compile.
	    Nov  5,1989 - consolidated output routines and return
		early in partition case.
	    Feb 7, 1990 - added final pin anneal at low temperature.
		Removed doborder now use routing tiles.
	    Apr 28,1990 - stop updating pin positions halfway thru
		compaction cycle.
	    Mon Jan  7 18:29:53 CST 1991 - removed annoying
		wait state which confuses people.
	    Thu Jan 17 00:52:38 PST 1991 - added adapt_wire_estimator.
	    Fri Jan 25 18:03:22 PST 1991 - added quickroute and
		removed unnecessary variables.
	    Mon Feb  4 02:04:59 EST 1991 - let quickroute perform global
		routing.
	    Tue Mar 12 17:02:16 CST 1991 - now no longer determine side
		of pad in partitioning case.
	    Thu Apr 18 01:31:34 EDT 1991 - flush output.
	    Sun May  5 14:26:56 EDT 1991 - replace config3 with reorigin.
	    Tue Jun 11 13:59:27 CDT 1991 - need to insure that
		we update softpin final positions.
	    Wed Jul 24 20:48:13 CDT 1991 - now always wait for
		user at the end of annealing if requested.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) finalout.c (Yale) version 3.15 7/24/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

/* redefine flags for easier reading */
#define VIOLATIONSONLY   FALSE
#define COMPACT          TRUE
#define CHANNELGRAPH     FALSE
#define UPDATE_ROUTING   TRUE 
#define NOCONSTRAINTS    FALSE
#define CONSTRAINTS      TRUE

finalout()
{

INT c ;
INT bbtop, bbbottom, bbleft, bbright ;

/* dump the results of the placement to graphics file */
G( graphics_dump() ) ;
G( TWsetMode(1) ) ;
G( draw_the_data() ) ;
G( TWsetMode(0) ) ;


/* we known wire area at this point don't need to estimate */
turn_wireest_on(FALSE) ;

/* let the user know which pins we couldn't place */
set_pin_verbosity( TRUE ) ;

/* before channel graph generation and global routing let use tweak */
/* placement if desired */
if( doGraphicsG && wait_for_userG ){
    G( TWmessage( "TimberWolfMC waiting for your response" ) ) ;
    G( process_graphics() ) ;
} 

savewolf( TRUE ) ;  /* for debug purposes force save to occur */
if( scale_dataG > 1 ){
    /* end of the line for scaled case - 
	will return to parent to continue using saved placement. */
    closegraphics() ;
    YexitPgm( PGMOK ) ;
}
grid_cells() ;      /* force cells to grid locations */
compact(VIOLATIONSONLY); /* remove cell overlap */

/* if this is a partitioning run determine row placement */
if( doPartitionG && !(quickrouteG) ){
    set_determine_side( FALSE ) ;  /* allow SC to pick side */
    G( set_graphic_context( PARTITION_PLACEMENT ) ) ;
    config_rows() ;
    print_paths() ; /* print path information */
    Output( 0 ) ;
    return ;
}
/* do final placement of pads using virtual core to insure pads */
/* are outside of core */
setVirtualCore( TRUE ) ;
placepads() ;

/* before channel graph generation and global routing let use tweak */
/* placement if desired */
check_graphics() ;

if( !scale_dataG ){ 
    /* reload bins to get new overlap penalty */
    loadbins(FALSE) ; /* wireArea not known */
}
prnt_cost( "\nFINAL PLACEMENT RESULTS AFTER VIOLATION REMOVAL ARE:\n" ) ;

print_paths() ; /* print path information */
Output( 0 ) ;

if( doCompactionG > 0 || quickrouteG ) {
    gmain( CHANNELGRAPH ) ;
    rmain( NOCONSTRAINTS ) ;
    gmain( UPDATE_ROUTING ) ;
    adapt_wire_estimator() ;
    check_graphics() ;

    if( quickrouteG ){
	return ;
    }

    for( c = 1 ; c <= doCompactionG ; c++ ) {

	funccostG = findcost() ;
	sprintf(YmsgG,"\n\nCompactor Pass Number: %d begins with:\n", c ) ;
	prnt_cost( YmsgG ) ;

	wirecosts() ;


	grid_cells() ;      /* force cells to grid locations */
	compact(COMPACT);   /* remove white space */
	reorigin() ;
	check_graphics() ;

	sprintf(YmsgG,"\n\nCompactor Pass Number: %d after cost:\n", c ) ;
	prnt_cost( YmsgG ) ;

	Output( c ) ;

	gmain( CHANNELGRAPH ) ;

	if( c == doCompactionG ){
	    rmain( CONSTRAINTS ) ;
	} else {
	    rmain( NOCONSTRAINTS ) ;
	    gmain( UPDATE_ROUTING ) ;
	    adapt_wire_estimator() ;
	    check_graphics() ;
	}

    } /* end compaction - global route loop */

} else {
    if( doChannelGraphG ) {
	gmain( CHANNELGRAPH ) ;
    }
    if( doGlobalRouteG ) {
	rmain( CONSTRAINTS ) ;
    }
}


prnt_cost("\nTIMBERWOLFMC FINAL RESULTS ARE:\n" ) ;

return ;
} /* end finalout */




Output( cycle )
INT cycle ;
{

    if( cycle == 0 || cycle < doCompactionG / 2 ){
	update_pins( FALSE ) ;  /* update the _orig fields */
    }
    outgeo() ;
    output(NULL)   ;
    outpin() ;
    if( doPartitionG ){
	output_partition() ;
    }
    return ;
} /* end Output */

/* print out the current cost to the user */
prnt_cost( out_string ) 
char *out_string ;
{
    INT xspan ;
    INT yspan ;

    funccostG = findcost() ;
    OUT2("%s", out_string ) ;
    OUT2("   routing cost        :%d\n", funccostG ) ;
    OUT2("   overlap penalty     :%d\n", binpenalG);
    OUT2("   lapFactor * overlap :%d\n", penaltyG);
    OUT2("   timing penalty      :%d\n", timingpenalG );
    OUT2("+  timeFactor * timepen:%d\n", timingcostG );
    OUT1("-------------------------------------\n" ) ; 
    OUT2("   total cost          :%d\n",
	(funccostG + penaltyG + timingcostG) ) ;
    wirecosts() ;

    find_core_boundary( &blocklG, &blockrG, &blockbG, &blocktG ) ;
    OUT5("\n\nCORE Bounding Box: l:%d r:%d b:%d t:%d\n",
	    blocklG , blockrG , blockbG , blocktG ) ;
    xspan = blockrG - blocklG ;
    yspan = blocktG - blockbG ;
    OUT2( "   xspan     = %d\n", xspan ) ;
    OUT2( "   yspan     = %d\n", yspan ) ;
    OUT2( "   core area = %4.2le\n\n",  (DOUBLE) xspan * (DOUBLE) yspan );
    OUT1("-------------------------------------\n" ) ; 
    Ymessage_flush() ;

}/* end print_current_cost */

check_graphics()
{
    if( doGraphicsG && wait_for_userG ){
	G( TWmessage( "TimberWolfMC waiting for your response" ) ) ;
	G( process_graphics() ) ;
    } else { 
	G( draw_the_data() ) ;
    }
} /* end check_graphics */
