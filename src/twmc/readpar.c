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
FILE:	    readpar.c                                       
DESCRIPTION:read parameter file
CONTENTS:   readpar( )
DATE:	    Jan 30, 1988 
REVISIONS:  Mar  8, 1988 - added lambda and init_acc ;
	    Oct 27, 1988 - changed to minimal standard random number
		generator.
	    Nov 25, 1988 - added timeFactor initialization.
	    Feb 20, 1989 - added no.graphics keyword
	    Mar 01, 1989 - added no.graphics.wait keyword.
	    Mar 13, 1989 - added tw_fast and tw_slow parameters.
	    May 11, 1989 - moved doGraphicsG initialization to main.c
	    May 18, 1989 - modified pad placement parameters.
	    May 24, 1989 - removed doPlacementG.
	    May 25, 1989 - added graphFilesG switch.
	    Sep 19, 1989 - added no.graphics.update switch.
	    Oct  3, 1989 - added scale_dataG and .mpar output fix.
	    Oct 20, 1989 - changed track spacing to track pitch to
		make it easier on the user.
	    Nov 28, 1989 - made scaling user data automatic.
	    Apr 23, 1990 - rewrote parsing to read one line at a time.
	    May 15, 1990 - made error messages easier to understand.
	    Nov 23, 1990 - now use library utility to read par
		files.
	    Fri Jan 25 18:09:20 PST 1991 - removed unnecessary globals.
	    Sat Feb 23 00:27:37 EST 1991 - now handle wildcarding.
	    Thu Mar  7 03:59:10 EST 1991 - added check for core.
	    Tue Mar 12 17:06:56 CST 1991 - add check for track spacing.
	    Thu Apr 18 01:27:23 EDT 1991 - added new design rule
		functions and cleaned up unused keywords.
	    Sun May  5 14:22:52 EDT 1991 - now user can set origin.
	    Wed Jun  5 16:29:10 CDT 1991 - added test for vertical_path
		weight.
	    Fri Oct 18 00:15:59 EDT 1991 - now scale block when calling
		TimberWolf recursively.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readpar.c version 3.20 11/23/91" ;
#endif

#include <string.h>
#include <custom.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>
#include <yalecad/yreadpar.h>

#define DEFAULTLAMBDA 0.1
#define INIT_ACCEPTANCE 0.99999
#define COMMENT '#'

static BOOL abortS = FALSE ;
static BOOL readparamS = FALSE ;
static BOOL graphFileS ;
static BOOL originGivenS = FALSE ;
static BOOL sc_outputS = FALSE ;
static INT gOffsetXS = INT_MIN ;
static INT gOffsetYS = INT_MIN ;
static INT gridXS = INT_MIN ;
static INT gridYS = INT_MIN ;




static init_read_par();
static readparam();
static process_readpar();
static err_msg();



readpar()
{
    init_read_par() ;
    readparam( TWMC ) ;
    readparam( USER ) ;
    process_readpar() ;
}

static init_read_par()
{
    /* set the default values */
    offsetG     = 0  ;
    layersFactorG = 1 ;
    totNetLenG  = 0  ;
    randVarG = -1 ;
    wireEstimateOnlyG = 0 ;
    lapFactorG  = 1.0;
    timeFactorG  = 1.0;
    cost_onlyG  = FALSE ;
    maxWeightG  = 2  ;
    baseWeightG = 1  ;
    chipaspectG = -1.0 ;
    track_spacingXG = -1 ;
    track_spacingYG = -1 ;
    doPartitionG = FALSE ;
    doChannelGraphG = FALSE ;
    doGlobalRouteG = FALSE ;
    doCompactionG = FALSE ;
    coreGivenG = FALSE ;
    wait_for_userG = FALSE ;
    defaultTracksG = 0 ;
    restartG = FALSE ;
    contiguousG = TRUE ;
    gridCellsG = TRUE ;
    min_pad_spacingG = 0 ;
    init_accG = INIT_ACCEPTANCE ;
    padspacingG = UNIFORM_PADS ;
    external_pad_programG = FALSE ;
    graphFileS = FALSE ;
    vertical_wire_weightG = -1.0 ;
    vertical_path_weightG = 1.0 ;

    /* set default speed */
    set_tw_speed( 1.0 ) ;

    /* initialize routing tiles to NULL */
    init_routing_tiles() ;

    /* assume no origin given default to 0, 0 */
    x_originG = 0 ; y_originG = 0 ;
} /* end init_read_par */



static readparam( parfile )
INT parfile ;
{

    INT test ;
    INT speed ;
    INT pins ;
    INT line ;
    INT length ;
    INT numtokens ;
    BOOL onNotOff ;
    BOOL wildcard ;
    char design[LRECL] ;
    char **tokens ;
    char *lineptr ;

    /* always read the user parameter file - avoid having two of */
    /* them so chop off the _io suffix in the case of partitioning */
    if( parfile == USER ){
	length = strlen( cktNameG ) ;
	/* look for _io as the last 3 characters special case */
	strcpy( design, cktNameG ) ; 
	if( strcmp( cktNameG+length-3, "_io" ) == STRINGEQ ){
	    design[length-3] = EOS ;
	}
	Yreadpar_init( design, parfile, TWMC, FALSE ) ;
    } else {
	Yreadpar_init( cktNameG, parfile, TWMC, FALSE ) ;
    }


    OUT1( "\n\n" ) ;

    while( tokens = Yreadpar_next( &lineptr, &line, &numtokens, 
	&onNotOff, &wildcard )){
	readparamS = TRUE ;
	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( strcmp( tokens[0], "initialT" ) == STRINGEQ ){
	    if( numtokens == 2 ){
		TG = atof( tokens[1] ) ;
	    } else {
		err_msg("initialT") ;
	    }
	} else if( strcmp( tokens[0], "cost_only" ) == STRINGEQ ){
	    if( onNotOff ){
		cost_onlyG = TRUE ;
	    } else {
		cost_onlyG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "restart" ) == STRINGEQ ){
	    if( onNotOff ){
		restartG = TRUE ;
	    } else {
		restartG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "debug" ) == STRINGEQ ){
	    if( onNotOff ){
		YsetDebug( TRUE ) ;
	    } else {
		YsetDebug( FALSE ) ;
	    }
	} else if( strcmp( tokens[0], "graph" ) == STRINGEQ ){
	    if( onNotOff ){
		graphFileS = TRUE ;
	    } else {
		graphFileS = FALSE ;
	    }
	} else if( strcmp( tokens[0], "random.seed" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		randVarG = atoi(tokens[1]) ;
		Yset_random_seed( randVarG ) ;
	    } else {
		err_msg("random.seed") ;
	    }
	} else if( strcmp( tokens[0], "chip.aspect.ratio" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		chipaspectG = atof( tokens[1] ) ;
	    } else {
		err_msg("chip.aspect.ratio") ;
	    }
	} else if( strcmp( tokens[0],"default.tracks.per.channel")==STRINGEQ){
	    if( numtokens == 2 ) {
		defaultTracksG = atoi( tokens[1] ) ;
	    } else {
		err_msg("default.tracks.per.channel") ;
	    }
	} else if( strcmp( tokens[0], "wire.estimation.only" ) == STRINGEQ ){
	    if( onNotOff ){
		wireEstimateOnlyG = TRUE ;
	    } else {
		wireEstimateOnlyG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "do.channel.graph" ) == STRINGEQ ){
	    if( onNotOff ){
		doChannelGraphG = TRUE ;
	    } else {
		doChannelGraphG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "do.global.route" ) == STRINGEQ ){
	    if( onNotOff ){
		doGlobalRouteG = TRUE ;
	    } else {
		doGlobalRouteG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "do.partition" ) == STRINGEQ ){
	    if( onNotOff ){
		doPartitionG = TRUE ;
	    } else {
		doPartitionG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "no.graphics" ) == STRINGEQ ){
	    if( onNotOff ){
		doGraphicsG = FALSE ;
	    } else {
		doGraphicsG = TRUE ;
	    }
	} else if( strcmp( tokens[0], "graphics.wait" ) == STRINGEQ ){
	    if( onNotOff ){
		wait_for_userG = TRUE ;
	    } else {
		wait_for_userG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "sc_output_files" ) == STRINGEQ ){
	    if( onNotOff ){
		sc_outputS = TRUE ;
	    } else {
		sc_outputS = FALSE ;
	    }
	} else if( strcmp( tokens[0], "no.graphics.update" ) == STRINGEQ ){
	    if( onNotOff ){
		set_dump_ratio( 160 ) ;
	    } else {
		set_dump_ratio( 1 ) ;
	    }
	} else if( strcmp( tokens[0], "do.compaction" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		doCompactionG = atoi( tokens[1] ) ;
	    } else {
		err_msg("do.compaction") ;
	    }
	    
	} else if( strcmp( tokens[0], "core" ) == STRINGEQ ){
	    if( numtokens == 5 ){
		blocklG = atoi( tokens[1] ) ;
		blockbG = atoi( tokens[2] ) ;
		blockrG = atoi( tokens[3] ) ;
		blocktG = atoi( tokens[4] ) ;
		if( blocklG > blockrG ){
		    M( ERRMSG, "readpar", "core left is greater than core right\n" ) ;
		    abortS = TRUE ;
		}
		if( blockbG > blocktG ){
		    M( ERRMSG, "readpar", "core bottom is greater than core top\n" ) ;
		    abortS = TRUE ;
		}
		coreGivenG = TRUE ;
	    } else if( numtokens == 6 && 
		strcmp( tokens[1], "initially" ) == STRINGEQ ){
		blocklG = atoi( tokens[2] ) ;
		blockbG = atoi( tokens[3] ) ;
		blockrG = atoi( tokens[4] ) ;
		blocktG = atoi( tokens[5] ) ;
		init_fixcell( atoi( tokens[2] ), atoi( tokens[3] ),
		    atoi( tokens[4] ), atoi( tokens[5] ) ) ;
	    } else {
		err_msg("the core definition") ;
	    }
	} else if( strcmp( tokens[0], "origin" ) == STRINGEQ ){
	    if( numtokens == 3 ){
		x_originG = atoi( tokens[1] ) ;
		y_originG = atoi( tokens[2] ) ;
		originGivenS = TRUE ;
	    } else {
		err_msg("the core definition") ;
	    }
	} else if( strcmp( tokens[0], "gridOffsetX" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		gOffsetXS = atoi( tokens[1] ) ;
	    } else {
		err_msg("gridOffsetX") ;
	    }
	} else if( strcmp( tokens[0], "gridOffsetY" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		gOffsetYS = atoi( tokens[1] ) ;
	    } else {
		err_msg("gridOffsetY") ;
	    }
	} else if( strcmp( tokens[0], "gridX" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		gridXS = atoi( tokens[1] ) ;
	    } else {
		err_msg("gridX") ;
	    }
	} else if( strcmp( tokens[0], "gridY" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		gridYS = atoi( tokens[1] ) ;
	    } else {
		err_msg("gridY") ;
	    }
	} else if( strcmp( tokens[0], "fast" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		speed = atoi( tokens[1] ) ;
	    } else {
		err_msg("fast") ;
		continue ;
	    } 
	    if( speed == 0 ){
		err_msg("fast") ;
	    } else {
		set_tw_speed( 1.0 / (DOUBLE) speed ) ;
	    }
	} else if( strcmp( tokens[0], "slow" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		speed = atoi( tokens[1] ) ;
	    } else {
		err_msg("slow") ;
		continue ;
	    }
	    if( speed == 0 ){
		err_msg("slow") ;
	    } else {
		set_tw_speed( (DOUBLE) speed ) ;
	    }

	} else if( strcmp( tokens[0], "init_acc" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		init_accG = atof( tokens[1] ) ;
	    } else {
		err_msg("init_acc") ;
	    }
	} else if( strcmp( tokens[0], "print_pins" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		pins = atoi( tokens[1] ) ;
		set_print_pin( pins ) ;
	    } else {
		err_msg("print_pins") ;
	    }
	} else if( strcmp( tokens[0], "placepads" ) == STRINGEQ ){
	    if( onNotOff ){
		external_pad_programG = TRUE ;
	    } else {
		external_pad_programG = FALSE ;
	    }

	} else if( strcmp( tokens[0], "contiguous_pad_groups" ) == STRINGEQ ){
	    if( onNotOff ){
		contiguousG = TRUE ;
	    } else {
		contiguousG = FALSE ;
	    }

	} else if( strcmp( tokens[0], "minimum_pad_space" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		min_pad_spacingG = atoi( tokens[1] );
	    } else {
		err_msg("minimum space between pads") ;
	    }

	} else if( strcmp( tokens[0], "padspacing" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		if( strcmp( tokens[1] , "uniform" ) == STRINGEQ ) {
		    padspacingG = UNIFORM_PADS ;
		} else if( strcmp( tokens[1] , "variable" ) == STRINGEQ ) {
		    padspacingG = VARIABLE_PADS ;
		} else if( strcmp( tokens[1] , "abut" ) == STRINGEQ ) {
		    padspacingG = ABUT_PADS ;
		} else if( strcmp( tokens[1] , "exact" ) == STRINGEQ ) {
		    padspacingG = EXACT_PADS ;
		} else {
		    OUT1("Unexpected padspacing keyword in the .mpar file\n");
		    abortS = TRUE ;
		}
	    } else {
		err_msg("padspacing") ;
	    }
	} else if( strcmp( tokens[0],"vertical_wire_weight")==STRINGEQ){
	    if( numtokens == 2 ) {
		vertical_wire_weightG = atof( tokens[1] ) ;
	    } else {
		err_msg("vertical_wire_weight") ;
	    }

	} else if( strcmp( tokens[0],"vertical_path_weight")==STRINGEQ){
	    if( numtokens == 2 ) {
		vertical_path_weightG = atof( tokens[1] ) ;
		if( vertical_wire_weightG < 0.0 ){
		    /* this means vertical wire_weight has not been */
		    /* assign yet set to path weight as default */
		    vertical_wire_weightG = vertical_path_weightG ;
		}
	    } else {
		err_msg("vertical_wire_weight") ;
	    }
	} else if( strcmp( tokens[0],"wiring_reduction")==STRINGEQ){
	    if( numtokens == 2 ) {
		set_wiring_reduction( atof( tokens[1] ) ) ;
	    } else {
		err_msg("wiring_reduction") ;
	    }
	/*** catch all ***/
	} else if(!(wildcard)) {
	    if( parfile == USER ){
		sprintf( YmsgG, 
		"unexpected keyword in the %s.par file at line:%d\n\t%s\n", 
		cktNameG, line, lineptr );
		M( ERRMSG, "readpar", YmsgG ) ;
	    } else {
		sprintf( YmsgG, 
		"Unexpected keyword in the %s.mpar file at line:%d\n\t%s\n", 
		cktNameG, line, lineptr );
		M( ERRMSG, "readpar", YmsgG ) ;
	    }
	    Ymessage_error_count() ;
	    abortS = TRUE ;
	}
    }
} /* end readpar */

static process_readpar()
{
    char *layer ;             /* name of layer */
    INT i ;                   /* counter */
    INT pitch ;               /* the pitch of the layer */
    INT numv_layers ;         /* number of vertical   layers */
    INT numh_layers ;         /* number of horizontal layers */
    INT num_layers ;          /* total number of layers */

    if( !(readparamS)){
	M( ERRMSG, "process_readpar", "No parameter files found.  Must exit\n\n" ) ;
	YexitPgm( FAIL ) ;
    }
    Yplot_control( graphFileS ) ;

    if( coreGivenG && scale_dataG ){
	blockbG /= scale_dataG ;
	blocklG /= scale_dataG ;
	blockrG /= scale_dataG ;
	blocktG /= scale_dataG ;
    }
    if( originGivenS && scale_dataG ){
	x_originG /= scale_dataG ;
	y_originG /= scale_dataG ;
    }

    numv_layers = 0 ;
    numh_layers = 0 ;
    num_layers = Yreadpar_numlayers() ;

    for( i = 1; i <= num_layers; i++ ){
	layer = Yreadpar_id2layer( i ) ;
	pitch = (INT) Yreadpar_pitch( layer ) ;

	if( Yreadpar_layer_HnotV( layer ) ){
	    track_spacingYG += pitch ;
	    numh_layers++ ;
	} else {
	    track_spacingXG += pitch ;
	    numv_layers++ ;
	}
    }
    /* get the average pitch of all the layers */
    if( numv_layers > 0 ){
	track_spacingXG++ ; /* to account for the -1 initialization */
	track_spacingXG /= numv_layers ;
	if( track_spacingXG == 0 ){
	    track_spacingXG == 1 ;
	}
	if( scale_dataG ){
	    track_spacingXG = ceil( (DOUBLE) track_spacingXG /
			 (DOUBLE)scale_dataG ) ;
	}
    }
    if( numh_layers > 0 ){
	track_spacingYG++ ; /* to account for the -1 initialization */
	track_spacingYG /= numh_layers ;
	if( track_spacingYG == 0 ){
	    track_spacingYG == 1 ;
	}
	if( scale_dataG ){
	    track_spacingYG = ceil( (DOUBLE) track_spacingYG /
			 (DOUBLE)scale_dataG ) ;
	}
    }

    if( gridXS >= 0 || gridYS >= 0 || gOffsetXS >= 0 || gOffsetYS >= 0 ) {
	gridGivenG = TRUE ;
	if( !(gridXS >= 0 && gridYS >= 0 && 
				gOffsetXS >= 0 && gOffsetYS >= 0 ) ) {
	    OUT1("Error: It appears as though the grid is\n");
	    OUT1("not fully specified\n");
	    abortS = TRUE ;
	}
    } else {
	gridGivenG = FALSE ;
	gridXS = track_spacingXG ;
	gridYS = track_spacingYG ;
	gOffsetXS = 0 ;
	gOffsetYS = 0 ;
    }
    Ygrid_setx( gridXS, gOffsetXS ) ;
    Ygrid_sety( gridYS, gOffsetYS ) ;

    if( track_spacingXG < 0 ) {
	M( ERRMSG, "readpar",
	"Vertical track pitch could not be calculated from design rules\n");
	M( ERRMSG, NULL, "Check the design rules in parameter file\n\n") ;
	abortS = TRUE ;
    }
    if( track_spacingYG < 0 ) {
	M( ERRMSG, "readpar",
	"Horizontal track pitch could not be calculated from design rules\n");
	M( ERRMSG, NULL, "Check the design rules in parameter file\n\n") ;
	abortS = TRUE ;
    }
    if( track_spacingXG == 0 ) {
	M( ERRMSG, "readpar","Vertical track pitch must not be zero\n");
	M( ERRMSG, NULL, "Check the design rules in parameter file\n\n") ;
	abortS = TRUE ;
    }
    if( track_spacingYG == 0 ) {
	M( ERRMSG, "readpar","Horizontal track pitch must not be zero\n");
	M( ERRMSG, NULL, "Check the design rules in parameter file\n\n") ;
	abortS = TRUE ;
    }
    if( vertical_path_weightG < 0.0 || vertical_wire_weightG < 0.0 ){
	M( ERRMSG, "readpar","Vertical_path_weight missing or specified incorrectly\n");
	abortS = TRUE ;
    }
    if( coreGivenG && originGivenS ) {
	M( ERRMSG, "readpar",
	"Both origin and core region have been specified.\n");
	M( ERRMSG, NULL, "Either add the keyword initially to core or\n");
	M( ERRMSG, NULL, "only specify at a time\n\n") ;
	abortS = TRUE ;
    }
    if( chipaspectG < 0.0 ) {
	OUT1("chip.aspect.ratio was not entered ");
	OUT1("in the .mpar file\n");
	abortS = TRUE ;
    }
    if( !cost_onlyG ) {
	if( init_accG < 0.0 ) {
	    OUT1("init_acc must be a positive floating point number\n");
	    abortS = TRUE ;
	} 
    }
    if( restartG ) {
	OUT1("TimberWolf instructed to attempt ");
	OUT2("restart of circuit:<%s>\n", cktNameG ) ;
    }
    if( !cost_onlyG ) {
	OUT1("TimberWolf instructed to do ");
	OUT2("placement of circuit:<%s>\n", cktNameG ) ;
    }
    if( doChannelGraphG ) {
	OUT1("TimberWolf instructed to generate ");
	OUT2("channel graph for circuit:<%s>\n", cktNameG ) ;
	if( cost_onlyG  ) {
	    OUT1("TimberWolf assumes input files: ");
	    OUT3("<%s.geo> and <%s.pin> are present in\n",cktNameG, cktNameG );
	}
    }
    if( doGlobalRouteG ) {

	if( doChannelGraphG ) {
	    if( defaultTracksG < 0 ) {
		OUT1("default.tracks.per.channel \n");
		OUT1("was not entered in .mpar file\n");
		abortS = TRUE ;
	    } else {
		OUT2("default.tracks.per.channel: %d\n",defaultTracksG);
	    }
	}
	OUT1("TimberWolf instructed to do ");
	OUT2("global route for circuit:<%s>\n", cktNameG ) ;
	if( ! doChannelGraphG ) {
	    OUT1("TimberWolf assumes input files: ");
	    OUT3("<%s.gph> and <%s.twf> are present in ",
						    cktNameG, cktNameG );
	    OUT1("the working directory\n") ;
	}
    }
    if( doCompactionG ) {
	OUT1("TimberWolf instructed to do ");
	OUT2("post-placement compaction for circuit:<%s>\n",cktNameG);
	if( !(doChannelGraphG && doGlobalRouteG) ) {
	    OUT1("Error: TimberWolf cannot do compaction ");
	    OUT1("without request to also do:\n");
	    OUT1("\tdoChannelGraph and doGlobalRoute - ");
	    OUT1("Hence, request is cancelled\n");
	    doCompactionG = FALSE ;
	}
    }

    OUT2("track.pitch.x: %d\n" , track_spacingXG ) ;
    OUT2("track.pitch.y: %d\n" , track_spacingYG ) ;
    OUT2("chip.aspect.ratio: %g\n" , chipaspectG ) ;
    OUT2("init_acc: %4.2f\n", init_accG ) ;

    if( abortS ){
	M( ERRMSG, "read_par", "Trouble with parameter file\n" ) ;
	sprintf( YmsgG, "Please read %s.mout for details\n\n",cktNameG ) ;
	M( ERRMSG, NULL, YmsgG ) ;
	YexitPgm( FAIL ) ;
    }

    FLUSHOUT() ;
    return ;
} /* end process_readpar */

static err_msg( keyword ) 
char *keyword ;
{
    OUT2("The value for %s was", keyword );
    OUT1(" not properly entered in the .mpar file\n");
    abortS = TRUE ;
}/* end err_msg */

BOOL sc_output()
{
    return( sc_outputS ) ;
} /* end sc_output */
