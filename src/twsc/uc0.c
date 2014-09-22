/* ----------------------------------------------------------------- 
FILE:	    uc0.c                                       
DESCRIPTION:orientation change.
CONTENTS:   uc0( a , newaor )
		INT a , newaor ;
DATE:	    Mar 27, 1989 
REVISIONS:  Mon Aug 12 17:01:03 CDT 1991 - changed timing ASSERTIONS
		to D( ) constructs to speed execution time during
		debug mode.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) uc0.c (Yale) version 4.6 2/23/92" ;
#endif
#endif

#include "standard.h"
#include <yalecad/debug.h>


uc0( a , newaor )
INT a , newaor ;
{

CBOXPTR acellptr ;
PINBOXPTR antrmptr ;
INT cost , truth ;
INT newtimepenal ;


acellptr = carrayG[ a ]    ;
antrmptr = acellptr->pins ;

term_newpos( antrmptr, acellptr->cxcenter, acellptr->cycenter, newaor );

cost = funccostG ;

clear_net_set() ; /* reset set to mark nets that have changed position */
/* dimbox routines mark the nets that have changed */
new_dbox( antrmptr , &cost ) ;

newtimepenal = timingcostG ;
newtimepenal += calc_incr_time( a ) ;

D( "twsc/check_timing",
    ASSERT( dcalc_full_penalty(newtimepenal),"uc0","time problem") ;
) ;

truth =  acceptt( funccostG - cost, timingcostG - newtimepenal, 0  ) ; 
if( truth ) {

    dbox_pos( antrmptr ) ;
    update_time( a ) ;
    acellptr->corient = newaor ;
    funccostG = cost ;
    timingcostG  = newtimepenal ;
    G( graphics_cell_update( a ) ) ;
} else {
    G( graphics_cell_attempt( a ) ) ;
}
return ;
}
