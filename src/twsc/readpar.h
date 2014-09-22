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

/* global variables defined for main */
#ifdef READPAR_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif


EXTERN BOOL addFeedsG ;
EXTERN BOOL connection_machineG ;
EXTERN BOOL costonlyG ;
EXTERN BOOL cswapsG ;
EXTERN BOOL doglobalG ; 
EXTERN BOOL doGraphicsG ;
EXTERN BOOL estimate_feedsG ;
EXTERN BOOL gate_arrayG ;
EXTERN BOOL gate_array_specialG ;
EXTERN BOOL intelG ;
EXTERN BOOL restartG  ;
EXTERN BOOL SGGRG ;
EXTERN BOOL try_not_to_add_explicit_feedsG ;
EXTERN BOOL Equal_Width_CellsG ;
EXTERN BOOL file_conversionG ;
EXTERN BOOL even_rows_maximallyG ;

EXTERN INT feedLayerG ;
EXTERN INT no_feeds_side_netsG ;
EXTERN INT pin_layers_givenG ;
EXTERN INT tw_fastG ;
EXTERN INT tw_slowG ;

#undef EXTERN

#endif /* READPAR_VARS */
