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

int ablockG , bblockG ;
int aG , bG ;
int ApostG , BpostG ;
int *cellaptrG , *cellbptrG ;
int earlyRejG , Rej_errorG ;
int axcenterG , aycenterG , bxcenterG , bycenterG ;
int aleftG , arightG , bleftG , brightG ;
int attemptsG ;
int potential_errorsG , error_countG , P_limitG ;
double total_wire_chgG ;
double sigma_wire_chgG ;
double mean_wire_chgG ;
int wire_chgsG ;
double fraction_doneG ;

#undef EXTERN  

#endif /* UCXXGLB_H */
