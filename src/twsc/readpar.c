/*
 *   Copyright (C) 1989-1992 Yale University
 *   Copyright (C) 2013 Tim Edwards <tim@opencircuitdesign.com>
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
DESCRIPTION:read parameter file.
CONTENTS:   readParFile()
	    yaleIntro(message) 
		char *message ;
DATE:	    Mar 27, 1989 
REVISIONS:  Nov 23, 1990 - now use new readpar library function.
		Only need to read one of two .par or .spar
	    Thu Feb  7 00:11:36 EST 1991 - added new.pin.format.
	    Sun Feb 17 21:05:03 EST 1991 - added min_pad_spacing.
	    Sat Feb 23 00:30:06 EST 1991 - now handle wildcarding.
	    Wed Mar 13 13:46:21 CST 1991 - made the new format
		the default.
	    Thu Apr 18 01:56:45 EDT 1991 - added new parameter
		functions for design rules. 
	    Wed Jun  5 16:47:12 CDT 1991 - added default for
		vertical_wire_weight.
	    Thu Jun 13 11:48:40 CDT 1991 - changed to no.graphics.
	    Wed Jul  3 13:51:21 CDT 1991 - added print_pins.
	    Fri Sep  6 15:18:00 CDT 1991 - added no_feed_est for
		emergencies.
	    Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.
	    Fri Nov  8 01:13:18 EST 1991 - added even the rows
		maximally.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) readpar.c (Yale) version 4.26 5/12/92" ;
#endif
#endif

#define READPAR_VARS
#define NOTSPECIFIED -1
#define COMMENT '#'

#include "standard.h"
#include "main.h"
#include "parser.h"
#include "readpar.h"
#include "groute.h"
#include "feeds.h"
#include "config.h"
#include "pads.h"
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <yalecad/yreadpar.h>

/* globals variable definitions */
INT attprcelG ;
INT core_widthG ;
INT core_heightG ;
INT core_xstartG ;
INT core_ystartG ;
INT spacer_widthG = -1 ;
INT *spacer_feedsG ;
INT vertical_pitchG = 0 ;
INT total_row_lengthG ;
INT vertical_track_pitchG = 0 ;
INT horizontal_track_pitchG = 0 ;
INT approximately_fixed_factorG = 1 ;
INT global_routing_iterationsG = 0 ;
BOOL no_feed_estG = TRUE ;
BOOL placement_improveG = TRUE ;
BOOL intel_debugG = FALSE ;
BOOL do_fast_globalG = FALSE ;
BOOL new_row_formatG = FALSE ;
BOOL no_feed_at_endG ;
BOOL call_row_evenerG = FALSE ;
BOOL turn_off_checksG = FALSE ;
BOOL min_peak_densityG = FALSE ;
BOOL do_not_even_rowsG = FALSE ;
BOOL min_total_densityG = FALSE ;
BOOL ignore_crossbusesG = FALSE ;
BOOL output_at_densityG = FALSE ;
BOOL spacer_name_twfeedG = FALSE ;
BOOL create_new_cel_fileG = FALSE ;
BOOL rigidly_fixed_cellsG = FALSE ;
BOOL exclude_noncrossbus_padsG = TRUE ;
BOOL stand_cell_as_gate_arrayG = FALSE ;
BOOL good_initial_placementG = FALSE ;
BOOL glob_route_only_crit_netsG = FALSE ;
BOOL unused_feed_name_twspacerG = FALSE ;
BOOL absolute_minimum_feedsG = FALSE ;
BOOL ignore_feedsG = FALSE ;
BOOL vertical_track_on_cell_edgeG ;
BOOL route_padnets_outsideG ;
BOOL prop_rigid_cellsG = FALSE ;
BOOL even_rows_maximallyG = FALSE ;
DOUBLE indentG ;
DOUBLE metal2_pitchG ;

/* globals variable references */
extern BOOL orientation_optimizationG ;
extern BOOL doubleback_rows_start_at_oneG ;

/* static variables */
static BOOL abortS = FALSE ;
static BOOL readparamS = FALSE ;

static init_read_par();
static readparam();
static process_readpar();
static err_msg();

readParFile()
{
    init_read_par() ;
    readparam( TWSC ) ;
    readparam( USER ) ;
    process_readpar() ;
}

static init_read_par()
{
    /* initialization of variables */
    SGGRG = FALSE ;
    gate_arrayG = FALSE ;
    try_not_to_add_explicit_feedsG = FALSE ;
    vertical_track_on_cell_edgeG = FALSE ;
    no_feed_at_endG = TRUE ;
    spacer_feedsG = (INT *) Ysafe_malloc( 101 * sizeof(INT) ) ;
    spacer_feedsG[0] = 0 ;
    metal2_pitchG = 0.0 ;
    core_widthG  = 0 ;
    core_heightG = 0 ;
    core_xstartG = 0 ;
    core_ystartG = 0 ;
    vertical_wire_weightG = -1.0 ;
    vertical_path_weightG = -1.0 ;
    horizontal_path_weightG = 1.0 ;
    swap_netG = FALSE ;
    case_unequiv_pinG = FALSE ;
    swappable_gates_existG = FALSE ;
    min_pad_spacingG = 0 ;
    Equal_Width_CellsG = FALSE ;
    file_conversionG = FALSE ;
} /* end init_read_par */

static readparam( parfile )
INT parfile ;
{

    INT test ;
    INT speed ;
    INT pins ;
    INT spacer_tmp ;
    INT line ;
    INT numtokens ;
    BOOL onNotOff ;
    BOOL wildcard ;
    char **tokens ;
    char *lineptr ;

    Yreadpar_init( cktNameG, parfile, TWSC, FALSE ) ;

    OUT1( "\n\n" ) ;

    while( tokens = Yreadpar_next( &lineptr, &line, &numtokens, 
	&onNotOff, &wildcard )){
	readparamS = TRUE ;
	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;

	} else if( strcmp( tokens[0],"fast") == STRINGEQ ){
	    if( numtokens == 2 ) {
		tw_fastG = atoi( tokens[1] ) ;
	    } else {
		err_msg("fast") ;
	    }
	} else if( strcmp( tokens[0],"slow") == STRINGEQ ){
	    if( numtokens == 2 ) {
		tw_slowG = atoi( tokens[1] ) ;
	    } else {
		err_msg("slow") ;
	    }
        } else if( strcmp( tokens[0], "equal_width_cells" ) == STRINGEQ ){
              if( onNotOff ){
		  Equal_Width_CellsG = TRUE ;
	      } else {
        	  Equal_Width_CellsG = FALSE ;
	      }
	/*--------------------------------------------------------------*/
	} else if( strcmp( tokens[0],"file_conversion_only") == STRINGEQ ){
              if( onNotOff ){
		  file_conversionG = TRUE ;
	      } else {
		  file_conversionG = FALSE ;
	      }
	} else if( strcmp( tokens[0],"even_rows_maximally") == STRINGEQ ){
              if( onNotOff ){
		even_rows_maximallyG = TRUE ;
	      } else {
		even_rows_maximallyG = FALSE ;
	      }

	} else if( strcmp( tokens[0],"vertical_wire_weight") == STRINGEQ ){
	    if( numtokens == 2 ) {
		vertical_wire_weightG = atof( tokens[1] ) ;
	    } else {
		err_msg("vertical_wire_weight") ;
	    }
	} else if( strcmp( tokens[0],"vertical_path_weight") == STRINGEQ ){
	    if( numtokens == 2 ) {
		vertical_path_weightG = atof( tokens[1] ) ;
		if( vertical_wire_weightG < 0.0 ){
		    vertical_wire_weightG = vertical_path_weightG ;
		}
	    } else {
		err_msg("vertical_path_weight") ;
	    }
	} else if( strcmp( tokens[0],"horizontal_path_weight") == STRINGEQ ){
	    if( numtokens == 2 ) {
		horizontal_path_weightG = atof( tokens[1] ) ;
	    } else {
		err_msg("horizontal_path_weight") ;
	    }
	} else if( strcmp( tokens[0],"approximately_fixed_factor") == STRINGEQ ){
	    if( numtokens == 2 ) {
		approximately_fixed_factorG = atoi( tokens[1] ) ;
	    } else {
		err_msg("approximately_fixed_factor") ;
	    }
	    if( approximately_fixed_factorG < 1 ) {
		err_msg("approximately_fixed_factor") ;
	    }
	} else if( strcmp( tokens[0],"turn_off_checks") == STRINGEQ ){
	    if( onNotOff ){
		turn_off_checksG = TRUE ;
	    } else {
		turn_off_checksG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"gr_placement_improve") == STRINGEQ ){
	    if( onNotOff ){
		placement_improveG = TRUE ;
	    } else {
		placement_improveG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"route_only_critical_nets") == STRINGEQ ){
	    if( onNotOff ){
		glob_route_only_crit_netsG = TRUE ;
	    } else {
		glob_route_only_crit_netsG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"min_peak_density") == STRINGEQ ){
	    if( onNotOff ){
		min_peak_densityG = TRUE ;
	    } else {
		min_peak_densityG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"min_total_density") == STRINGEQ ){
	    if( onNotOff ){
		min_total_densityG = TRUE ;
	    } else {
		min_total_densityG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"call_row_evener") == STRINGEQ ){
	    if( onNotOff ){
		call_row_evenerG = TRUE ;
	    } else {
		call_row_evenerG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"create_new_cel_file") == STRINGEQ ){
	    if( onNotOff ){
		create_new_cel_fileG = TRUE ;
	    } else {
		create_new_cel_fileG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"unused_feed_name_twspacer") == STRINGEQ ){
	    if( onNotOff ){
		unused_feed_name_twspacerG = TRUE ;
	    } else {
		unused_feed_name_twspacerG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"spacer_name_twfeed") == STRINGEQ ){
	    if( onNotOff ){
		spacer_name_twfeedG = TRUE ;
	    } else {
		spacer_name_twfeedG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"new_row_format") == STRINGEQ ){
	    if( onNotOff ){
		new_row_formatG = TRUE ;
	    } else {
		new_row_formatG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"good_initial_placement") == STRINGEQ ){
	    if( numtokens == 2 ) {
		good_initial_placementG = atoi( tokens[1] ) ;
	    } else {
		err_msg("good_initial_placement") ;
	    }
	} else if( strcmp( tokens[0],"orientation_optimization") == STRINGEQ ){
	    if( onNotOff ){
		orientation_optimizationG = TRUE ;
	    } else {
		orientation_optimizationG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"do_not_even_rows") == STRINGEQ ){
	    if( onNotOff ){
		do_not_even_rowsG = TRUE ;
	    } else {
		do_not_even_rowsG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"absolute_minimum_feeds") == STRINGEQ ){
	    if( onNotOff ){
		absolute_minimum_feedsG = TRUE ;
	    } else {
		absolute_minimum_feedsG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"ignore_feeds") == STRINGEQ ){
	    if( onNotOff ){
		ignore_feedsG = TRUE ;
	    } else {
		ignore_feedsG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"pin_layers_given") == STRINGEQ ){
	    if( onNotOff ){
		pin_layers_givenG = TRUE ;
	    } else {
		pin_layers_givenG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"no_explicit_feeds") == STRINGEQ ){
	    if( onNotOff ){
		try_not_to_add_explicit_feedsG = TRUE ;
	    } else {
		try_not_to_add_explicit_feedsG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"no_feed_at_end") == STRINGEQ ){
	    if( onNotOff ){
		no_feed_at_endG = TRUE ;
	    } else {
		no_feed_at_endG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"intel_debug") == STRINGEQ ){
	    if( onNotOff ){
		intel_debugG = TRUE ;
	    } else {
		intel_debugG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"exclude_noncrossbus_pads") == STRINGEQ ){
	    if( onNotOff ){
		exclude_noncrossbus_padsG = TRUE ;
	    } else {
		exclude_noncrossbus_padsG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"proportionally_space_rigid_cells") == STRINGEQ ){
	    if( onNotOff ){
		prop_rigid_cellsG = TRUE ;
	    } else {
		prop_rigid_cellsG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"ignore_crossbuses") == STRINGEQ ){
	    if( onNotOff ){
		ignore_crossbusesG = TRUE ;
	    } else {
		ignore_crossbusesG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"connection_machine") == STRINGEQ ){
	    if( numtokens == 2 ) {
		connection_machineG = atoi( tokens[1] ) ;
	    } else {
		err_msg("connection_machine") ;
	    }
	} else if( strcmp( tokens[0],"doubleback_rows_start_at_one") == STRINGEQ ){
	    if( onNotOff ){
		intelG = TRUE ;
		no_feed_estG = TRUE ;
		doubleback_rows_start_at_oneG = TRUE ;
	    } else {
		intelG = FALSE ;
		no_feed_estG = TRUE ;
		doubleback_rows_start_at_oneG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"doubleback_rows_start_at_two") == STRINGEQ ){
	    if( onNotOff ){
		intelG = TRUE ;
		no_feed_estG = TRUE ;
		doubleback_rows_start_at_oneG = FALSE ;
	    } else {
		intelG = FALSE ;
		no_feed_estG = TRUE ;
		doubleback_rows_start_at_oneG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"vertical_track_on_cell_edge") == STRINGEQ ){
	    if( onNotOff ){
		vertical_track_on_cell_edgeG = TRUE ;
	    } else {
		vertical_track_on_cell_edgeG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"cost_only") == STRINGEQ ){
	    if( onNotOff ){
		costonlyG = TRUE ;
	    } else {
		costonlyG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"no_feed_est") == STRINGEQ ){
	    if( onNotOff ){
		no_feed_estG = TRUE ;
	    } else {
		no_feed_estG = TRUE ;
	    }
	} else if( strcmp( tokens[0],"only.do.global.route") == STRINGEQ ){
	    if( onNotOff ){
		doglobalG = TRUE ;
		costonlyG = TRUE ;
		resume_runG = YES ;
	    } else {
		doglobalG = FALSE ;
		costonlyG = FALSE ;
		resume_runG = NO ;
	    }
	} else if( strcmp( tokens[0],"SGGR") == STRINGEQ ){
	    if( onNotOff ){
		SGGRG = TRUE ;
	    } else {
		SGGRG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"global_routing_iterations") == STRINGEQ ){
	    if( numtokens == 2 ) {
		global_routing_iterationsG = atoi( tokens[1] ) ;
	    } else {
		err_msg("global_routing_iterations") ;
	    }
	} else if( strcmp( tokens[0],"do.global.route") == STRINGEQ ){
	    if( onNotOff ){
		doglobalG = TRUE ;
	    } else {
		doglobalG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"standard_cell_as_gate_array") == STRINGEQ ){
	    if( onNotOff ){
		stand_cell_as_gate_arrayG = TRUE ;
	    } else {
		stand_cell_as_gate_arrayG = FALSE ;
	    }
	} else if( strcmp( tokens[0], "print_pins" ) == STRINGEQ ){
	    if( numtokens == 2 ) {
		pins = atoi( tokens[1] ) ;
		set_print_pin( pins ) ;
	    } else {
		err_msg("print_pins") ;
	    }
	} else if( strcmp( tokens[0],"do.fast.global.route") == STRINGEQ ){
	    if( onNotOff ){
		do_fast_globalG = TRUE ;
		doglobalG = TRUE ;
	    } else {
		do_fast_globalG = FALSE ;
		doglobalG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"feedThruWidth") == STRINGEQ ){
	    if( numtokens == 2 ) {
		fdWidthG = atoi( tokens[1] ) ;
	    } else if( numtokens == 4 ) {
		pin_layers_givenG = TRUE ;
		fdWidthG = atoi( tokens[1] ) ;
		feedLayerG = atoi( tokens[3] ) ;
		if( feedLayerG == 0 ){
		    feedLayerG = 1 ;
		} else if( feedLayerG == 0 ){
		    err_msg("layer") ;
		}
	    } else {
		err_msg("feedThruWidth") ;
	    }
	} else if( strcmp( tokens[0],"total_row_length") == STRINGEQ ){
	    if( numtokens == 2 ) {
		total_row_lengthG = atoi( tokens[1] ) ;
	    } else {
		err_msg("total_row_length") ;
	    }
	} else if( strcmp( tokens[0],"spacer_width") == STRINGEQ ){
	    if( numtokens == 2 ) {
		spacer_widthG = atoi( tokens[1] ) ;
	    } else {
		err_msg("spacer_width") ;
	    }
	    gate_arrayG = TRUE ;
	    try_not_to_add_explicit_feedsG = TRUE ;
	} else if( strcmp( tokens[0],"spacer_feed_from_left") == STRINGEQ ){
	    if( numtokens == 2 ) {
		spacer_tmp = atoi( tokens[1] ) ;
	    } else {
		err_msg("spacer_feed_from_left") ;
	    }
	    spacer_feedsG[ ++spacer_feedsG[0] ] = spacer_tmp ;
	} else if( strcmp( tokens[0],"metal2_pitch") == STRINGEQ ){
	    if( numtokens == 2 ) {
		metal2_pitchG = atof( tokens[1] ) ;
	    } else {
		err_msg("metal2_pitch") ;
	    }
	} else if( strcmp( tokens[0],"vertical.pitch") == STRINGEQ ){
	    if( numtokens == 2 ) {
		vertical_pitchG = atoi( tokens[1] ) ; 
	    } else {
		err_msg("vertical.pitch") ;
	    }
	} else if( strcmp( tokens[0],"core_width") == STRINGEQ ){
	    if( numtokens == 2 ) {
		core_widthG = atoi( tokens[1] ) ;
	    } else {
		err_msg("core_width") ;
	    }
	} else if( strcmp( tokens[0],"core_height") == STRINGEQ ){
	    if( numtokens == 2 ) {
		core_heightG = atoi( tokens[1] ) ;
	    } else {
		err_msg("core_height") ;
	    }
	} else if( strcmp( tokens[0],"core_xstart_relative_to_left_edge_of_rows") == STRINGEQ ){
	    if( numtokens == 2 ) {
		core_xstartG = atoi( tokens[1] ) ;
	    } else {
		err_msg("core_xstart_relative_to_left_edge_of_rows") ;
	    }
	} else if( strcmp( tokens[0],"core_ystart_relative_to_bottom_of_first_row") == STRINGEQ ){ if( numtokens == 2 ) {
		core_ystartG = atoi( tokens[1] ) ;
	    } else {
		err_msg("core_ystart_relative_to_bottom_of_first_row") ;
	    }
	} else if( strcmp( tokens[0],"addFeeds") == STRINGEQ ){
	    /* don't do anything for old keyword */
	} else if( strcmp( tokens[0],"indent") == STRINGEQ ){
	    if( numtokens != 2 ) {
		err_msg("indent") ;
	    }
	    indentG = 1.0 ;
	    /*  indent should always be 1.0 now  */
	} else if( strcmp( tokens[0],"random.seed") == STRINGEQ ){
	    if( numtokens == 2 ) {
		randomSeedG = (UNSIGNED_INT) atoi( tokens[1] ) ; 
	    } else {
		err_msg("random.seed") ;
	    }
	} else if( strcmp( tokens[0],"rowSep") == STRINGEQ ){
	    if( numtokens >= 2 ) {
		rowSepG = atof( tokens[1] ) ; 
		rowSepAbsG = (numtokens == 3) ? (INT) atof( tokens[2] ) : 0 ; 
	    } else {
		err_msg("rowSep") ;
	    }
	} else if( strcmp( tokens[0],"restart") == STRINGEQ ){
	    if( onNotOff ){
		resume_runG = YES ;
	    } else {
		resume_runG = NO ;
	    }
	} else if( strcmp( tokens[0],"uneven.cell.height") == STRINGEQ ){
	    if( onNotOff ){
		uneven_cell_heightG = TRUE ;
	    } else {
		uneven_cell_heightG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"route.to.active") == STRINGEQ ){
	    /* route2act is the routing layer to active cell minimum spacing */
	    if( numtokens == 2 ) {
		route2actG = atoi( tokens[1] ) ; 
	    } else {
		err_msg("route.to.active") ;
	    }
	} else if( strcmp( tokens[0],"output.at.density") == STRINGEQ ){
	    if( onNotOff ){
		output_at_densityG = TRUE ;
	    } else {
		output_at_densityG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"one.pin.feedthru") == STRINGEQ ){
	    if( onNotOff ){
		one_pin_feedthruG = TRUE ;
	    } else {
		one_pin_feedthruG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"route_padnets_outside") == STRINGEQ ){
	    if( onNotOff ){
		route_padnets_outsideG = TRUE ;
	    } else {
		route_padnets_outsideG = FALSE ;
	    }
	} else if( strcmp( tokens[0],"lambda") == STRINGEQ ){
	} else if( strcmp( tokens[0],"no.graphics") == STRINGEQ ){
	    if( onNotOff ){
		doGraphicsG = FALSE ;
	    } else {
		doGraphicsG = TRUE ;
	    }
	} else if( strcmp( tokens[0],"no.graphics.update") == STRINGEQ ){
	    if( onNotOff ){
		G( set_update( FALSE ) ) ;
	    } else {
		G( set_update( TRUE ) ) ;
	    }
	} else if( strcmp( tokens[0],"graphics.wait") == STRINGEQ ){
	} else if( strcmp( tokens[0],"old.pin.format") == STRINGEQ ){
	    if( onNotOff ){
		set_pin_format( TRUE ) ;
	    } else {
		set_pin_format( FALSE ) ;
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
		    OUT1("Unexpected padspacing keyword in the .par file\n");
		    abortS = TRUE ;
		}
	    } else {
		err_msg("padspacing") ;
	    }

	/*** catch all ***/
	} else if(!(wildcard)){
	    if( parfile == USER ){
		OUT4("ERROR[readpar]:unexpected keyword in the %s.par file at line:%d\n\t%s\n", 
		cktNameG, line, lineptr );
	    } else {
		OUT4("ERROR[readpar]:Unexpected keyword in the %s.spar file at line:%d\n\t%s\n", 
		cktNameG, line, lineptr );
	    }
	    Ymessage_error_count() ;
	    abortS = TRUE ;
	}
    }
} /* end  readparam */


static process_readpar()
{

char *layer ;             /* name of layer */
INT i ;                   /* counter */
INT pitch ;               /* the pitch of the layer */
INT numv_layers ;         /* number of vertical   layers */
INT numh_layers ;         /* number of horizontal layers */
INT num_layers ;          /* total number of layers */

if( abortS ){
    OUT1( "Errors found in the .par file.  Must exit\n\n" ) ;
    YexitPgm(PGMFAIL);
}
if( !(readparamS)){
    M( ERRMSG, "process_readpar", 
	"No parameter files found. Must exit\n\n") ;
    YexitPgm(PGMFAIL);
}


if( vertical_wire_weightG < 0 ) {
    fprintf( fpoG, "vertical_wire_weight ") ;
    fprintf( fpoG, "was NOT found in the .par file\n");
    YexitPgm(PGMFAIL);
}
if( vertical_path_weightG < 0 ) {
    fprintf( fpoG, "vertical_path_weight ") ;
    fprintf( fpoG, "was NOT found in the .par file\n");
    YexitPgm(PGMFAIL);
}


if( spacer_widthG == -1 ) {
    spacer_feedsG[0] = 1 ;
    spacer_feedsG[1] = fdWidthG / 2 ;
    spacer_widthG = fdWidthG ;
}

numv_layers = 0 ;
numh_layers = 0 ;
num_layers = Yreadpar_numlayers() ;
for( i = 1; i <= num_layers; i++ ){
    layer = Yreadpar_id2layer( i ) ;
    pitch = Yreadpar_pitch( layer ) ;
    if( Yreadpar_layer_HnotV( layer ) ){
	horizontal_track_pitchG += pitch ;
	numh_layers++ ;
    } else {
	vertical_track_pitchG += pitch ;
	numv_layers++ ;
    }
}
/* get the average pitch of all the layers */
if( numv_layers > 0 ){
    vertical_track_pitchG /= numv_layers ;
    if( vertical_track_pitchG == 0 ){
	vertical_track_pitchG == 1 ;
    }
}
if( numh_layers > 0 ){
    horizontal_track_pitchG++ ; /* to account for the -1 initialization */
    horizontal_track_pitchG /= numh_layers ;
    if( horizontal_track_pitchG == 0 ){
	horizontal_track_pitchG == 1 ;
    }
    track_pitchG = horizontal_track_pitchG ; /* these are the same var. */
}

if( vertical_track_pitchG == 0 || horizontal_track_pitchG == 0 ) {
    if( vertical_track_pitchG == 0 ) {
	M( ERRMSG, "readpar",
	"Vertical track pitch could not be calculated from design rules\n");
	M( ERRMSG, NULL, "Check the design rules in parameter file\n\n") ;
    }
    if( horizontal_track_pitchG == 0 ) {
	M( ERRMSG, "readpar",
	"Horizontal track pitch could not be calculated from design rules\n");
	M( ERRMSG, NULL, "Check the design rules in parameter file\n\n") ;
    }
    YexitPgm(PGMFAIL);
}

#ifdef INTEL
if( 0 /* intel */ ) {
    doglobalG = 0 ;
    if( metal2_pitchG == 0.0 ) {
	fprintf( fpoG,"metal2_pitch was not entered in the .par file\n");
	YexitPgm(PGMFAIL);
    } else {
	fprintf( fpoG, "metal2_pitch: %f\n" , metal2_pitchG ) ;
    }
}
#endif /* INTEL */

if( doglobalG ) {
    fprintf(fpoG,"TimberWolf will perform a global route step\n");
}
if( doglobalG == TRUE  &&  fdWidthG == -1 ) {
    fprintf(fpoG,"feedThruWidth was not entered in the .par file\n");
    YexitPgm(PGMFAIL);
} else {
    fprintf( fpoG, "feedThruWidth: %d\n" , fdWidthG ) ;
}

/* make sure track pitch has been specified for uneven cell heights */
if( uneven_cell_heightG ){
    if( track_pitchG == NOTSPECIFIED ){
	fprintf( fpoG, "TimberWolfSC cannot perform uneven cell");
	fprintf( fpoG, "height calculations without track pitch\n");
	fprintf( fpoG, "Enter track.pitch in the .par file\n");
    }
}
    
if( track_pitchG != NOTSPECIFIED ) {
    fprintf( fpoG, "track.pitch: %d\n" , track_pitchG ) ;
    if( route2actG == NOTSPECIFIED ){
	fprintf( fpoG, "route2act was not entered in the .par file\n");
	fprintf( fpoG, "route2act defaulted to track.pitch\n");
	route2actG = track_pitchG ;
    }
    fprintf( fpoG, "route2act: %d\n" , route2actG ) ;

} else {
    fprintf( fpoG, "track.pitch was not specified in par file\n" ) ;
    fprintf( fpoG, "TimberWolfSC will output cell locations according\n");
    fprintf( fpoG, "to user supplied row separation.\n" ) ;

}

return ;
} /* end process_readpar */

yaleIntro() 
{
    INT i ;

    fprintf(fpoG,"\n%s\n",YmsgG) ;
    fprintf(fpoG,"Row-Based Placement and Global Routing Program\n");
    fprintf(fpoG,"Authors: Carl Sechen, Kai-Win Lee, and Bill Swartz,\n");
    fprintf(fpoG,"         Yale University\n");

    printf("%s\n",YmsgG) ;
    printf("Row-Based Placement and Global Routing Program\n");
    printf("Authors: Carl Sechen, Kai-Win Lee, and Bill Swartz,\n");
    printf("         Yale University\n");

    /* inialize variables */
    randomSeedG  = (unsigned) Yrandom_seed() ;

    fixarrayG = (INT *) NULL ;
    ffeedsG = 0 ;
    macspaceG = (DOUBLE *) Ysafe_malloc( 24 * sizeof(DOUBLE) ) ;
    for( i = 1 ; i <= 15 ; i++ ) {
	macspaceG[i] = -1.0 ;
    }
    costonlyG = FALSE ;
    fdthrusG = FALSE ;
    doglobalG = FALSE ;

    attprcelG = 0 ;

    fdWidthG  = -1 ;
    track_pitchG = -1 ; 
    route2actG = -1 ;
    rowSepG = -1.0 ;
    rowSepAbsG = 0 ;
    indentG = 1.0 ;
    numSegsG = 0 ;
    resume_runG = NO ;
    pin_layers_givenG = TRUE ;
    no_feeds_side_netsG = FALSE ;
    feedLayerG = 0 ;
    tw_fastG = 0 ;
    tw_slowG = 0 ;
    estimate_feedsG = TRUE ;
    connection_machineG = 0 ;
    route_padnets_outsideG = FALSE ;

} /* end yaleIntro */

static err_msg( keyword ) 
char *keyword ;
{
    OUT2("The value for %s was", keyword );
    OUT1(" not properly entered in the .par file\n");
    abortS = TRUE ;
}/* end err_msg */
