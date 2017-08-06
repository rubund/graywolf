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

BOOL addFeedsG ;
BOOL connection_machineG ;
BOOL costonlyG ;
BOOL cswapsG ;
BOOL doglobalG ; 
BOOL doGraphicsG ;
BOOL estimate_feedsG ;
BOOL gate_arrayG ;
BOOL gate_array_specialG ;
BOOL intelG ;
BOOL restartG  ;
BOOL SGGRG ;
BOOL try_not_to_add_explicit_feedsG ;
BOOL Equal_Width_CellsG ;
BOOL file_conversionG ;
BOOL even_rows_maximallyG ;

int feedLayerG ;
int no_feeds_side_netsG ;
int pin_layers_givenG ;
int tw_fastG ;
int tw_slowG ;

void init_read_par();
void readparam();
void process_readpar();
void err_msg();
void readParFile();

#endif /* READPAR_VARS */
