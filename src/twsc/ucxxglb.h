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

extern int ablockG , bblockG ;
extern int aG , bG ;
extern int ApostG , BpostG ;
extern int *cellaptrG , *cellbptrG ;
extern int earlyRejG , Rej_errorG ;
extern int axcenterG , aycenterG , bxcenterG , bycenterG ;
extern int aleftG , arightG , bleftG , brightG ;
extern int attemptsG ;
extern int potential_errorsG , error_countG , P_limitG ;
extern double total_wire_chgG ;
extern double sigma_wire_chgG ;
extern double mean_wire_chgG ;
extern int wire_chgsG ;
extern double fraction_doneG ;

#endif /* UCXXGLB_H */
