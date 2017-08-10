/* ----------------------------------------------------------------- 
"@(#) readpar.h (Yale) version 4.4 11/8/91"
FILE:	    readpar.h                                       
DESCRIPTION:global definitions for TimberWolfSC
CONTENTS:   
DATE:	    Mar 27, 1989 
REVISIONS:  Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.
----------------------------------------------------------------- */
#ifndef READPAR_H
#define READPAR_H

extern int fdWidthG ;
extern int attprcelG ;
extern int core_widthG ;
extern int core_heightG ;
extern int core_xstartG ;
extern int core_ystartG ;
extern int spacer_widthG;
extern int *spacer_feedsG ;
extern int vertical_pitchG;
extern int total_row_lengthG ;
extern int vertical_track_pitchG;
extern int horizontal_track_pitchG;
extern int approximately_fixed_factorG;
extern int global_routing_iterationsG;
extern int uneven_cell_heightG ;
extern int pin_layers_givenG;
extern int ffeedsG;
extern int track_pitchG;
extern int rowSepAbsG ;
extern int tw_fastG ;
extern int tw_slowG ;
extern int resume_runG ;
extern int feedLayerG ;
extern int *fixarrayG ;
extern int numSegsG ;
extern int no_feeds_side_netsG ;

extern BOOL no_feed_estG;
extern BOOL placement_improveG;
extern BOOL intel_debugG;
extern BOOL do_fast_globalG;
extern BOOL new_row_formatG;
extern BOOL no_feed_at_endG;
extern BOOL call_row_evenerG;
extern BOOL turn_off_checksG;
extern BOOL min_peak_densityG;
extern BOOL do_not_even_rowsG;
extern BOOL min_total_densityG;
extern BOOL ignore_crossbusesG;
extern BOOL output_at_densityG;
extern BOOL spacer_name_twfeedG;
extern BOOL create_new_cel_fileG;
extern BOOL rigidly_fixed_cellsG;
extern BOOL exclude_noncrossbus_padsG;
extern BOOL stand_cell_as_gate_arrayG;
extern BOOL good_initial_placementG;
extern BOOL glob_route_only_crit_netsG;
extern BOOL unused_feed_name_twspacerG;
extern BOOL absolute_minimum_feedsG;
extern BOOL ignore_feedsG;
extern BOOL vertical_track_on_cell_edgeG ;
extern BOOL route_padnets_outsideG ;
extern BOOL prop_rigid_cellsG;
extern BOOL even_rows_maximallyG;
extern BOOL try_not_to_add_explicit_feedsG;
extern BOOL SGGRG;
extern BOOL gate_arrayG ;
extern BOOL estimate_feedsG ;
extern BOOL Equal_Width_CellsG ;
extern BOOL intelG ;
extern BOOL file_conversionG ;
extern BOOL connection_machineG ;
extern BOOL doglobalG ;

extern double indentG ;
extern double metal2_pitchG ;
extern double rowSepG ;

extern unsigned randomSeedG  ;

void init_read_par();
void readparam();
void process_readpar();
void err_msg();
void readParFile();
void yaleIntro();

#endif /* READPAR_VARS */
