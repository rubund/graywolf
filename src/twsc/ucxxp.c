/* ----------------------------------------------------------------- 
FILE:	    ucxxp.c                                       
DESCRIPTION:pairwise exchange.
CONTENTS:   ucxxp( a , b , anxcenter , bnxcenter )
		INT a , b , anxcenter , bnxcenter ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) ucxxp.c (Yale) version 4.6 2/23/92" ;
#endif
#endif

#include "ucxxglb.h"
#include <yalecad/debug.h>

ucxxp( a , b , anxcenter , bnxcenter )
INT a , b , anxcenter , bnxcenter ;
{

CBOXPTR acellptr , bcellptr ;
PINBOXPTR atermptr , btermptr ;

INT cost ;
INT aorient , borient ;
INT aycenter , bycenter ;
INT newtimepenal ;

acellptr = carrayG[ a ]    ; 
aycenter = acellptr->cycenter ; 
aorient  = acellptr->corient  ; 
atermptr = acellptr->pins ; 

bcellptr = carrayG[ b ]    ; 
bycenter = bcellptr->cycenter ; 
borient  = bcellptr->corient  ; 
btermptr = bcellptr->pins ; 

term_newpos( atermptr , anxcenter , bycenter , aorient ) ; 
term_newpos( btermptr , bnxcenter , aycenter , borient ) ; 

cost = funccostG ; 

clear_net_set() ; /* reset set to mark nets that have changed position */
/* dimbox routines mark the nets that have changed */
new_dbox2( atermptr , btermptr , &cost ) ;

newtimepenal = timingcostG ;
newtimepenal += calc_incr_time2( a, b ) ;

D( "twsc/check_timing",
    ASSERT( dcalc_full_penalty(newtimepenal),NULL,"time problem") ;
) ;


if( acceptt( funccostG - cost, timingcostG - newtimepenal, 0 ) ){

    dbox_pos( atermptr ) ;
    dbox_pos( btermptr ) ;
    update_time2() ;

    G( erase_a_cell( a, acellptr->cxcenter, aycenter ) ) ;
    G( erase_a_cell( b, bcellptr->cxcenter, bycenter ) ) ;

    acellptr->cxcenter = anxcenter ; 
    acellptr->cycenter = bycenter ; 
    bcellptr->cxcenter = bnxcenter ; 
    bcellptr->cycenter = aycenter ; 

    funccostG = cost ; 
    timingcostG  = newtimepenal ;
    G( graphics_cell_update( a ) ) ;
    G( graphics_cell_update( b ) ) ;
    return( 1 ) ;
} else {
    G( graphics_cell_attempt( a ) ) ;
    G( graphics_cell_attempt( b ) ) ;
    return( 0 ) ;
}
}
