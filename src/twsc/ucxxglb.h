/* ----------------------------------------------------------------- 
"@(#) ucxxglb.h (Yale) version 4.4 9/19/91"
FILE:	    ucxxglb.h                                       
DESCRIPTION:insert file for cell moves.
CONTENTS:   macros and global defs for cell moves.
DATE:	    Mar 27, 1989 
REVISIONS:  Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.
----------------------------------------------------------------- */
#ifndef UCXXGLB_H
#define UCXXGLB_H

#include "standard.h"

#define MASK 0x3ff 
#define remv_cell( cellptr , post ) (\
( *cellptr != post ) ? (cellptr[ post ] = cellptr[ *cellptr ],\
(*cellptr)-- ) : (*cellptr)-- )
/*
#define add_cell( cellptr , c ) ( cellptr[ ++(*cellptr) ] = c )
*/

/* global variables defined for main */
#ifdef UCXXGLB_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

EXTERN int ablockG , bblockG ;
EXTERN int aG , bG ;
EXTERN int ApostG , BpostG ;
EXTERN int *cellaptrG , *cellbptrG ;
EXTERN int earlyRejG , Rej_errorG ;
EXTERN int axcenterG , aycenterG , bxcenterG , bycenterG ;
EXTERN int aleftG , arightG , bleftG , brightG ;
EXTERN int attemptsG ;
EXTERN int potential_errorsG , error_countG , P_limitG ;
EXTERN DOUBLE total_wire_chgG ;
EXTERN DOUBLE sigma_wire_chgG ;
EXTERN DOUBLE mean_wire_chgG ;
EXTERN int wire_chgsG ;
EXTERN DOUBLE fraction_doneG ;

#undef EXTERN  

#endif /* UCXXGLB_H */
