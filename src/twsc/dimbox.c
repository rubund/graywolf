/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    dimbox.c                                       
DESCRIPTION:Incremental bounding box routines.
CONTENTS:   new_dbox( antrmptr , costptr )
		TEBOXPTR antrmptr ;
		INT *costptr ;
	    new_dbox2( antrmptr , bntrmptr , costptr )
		TEBOXPTR antrmptr , bntrmptr ;
		INT *costptr ;
	    wire_boundary1( dimptr )
		DBOXPTR dimptr ;
	    check_validbound( dimptr , termptr , nextptr )
		DBOXPTR dimptr ;
		TEBOXPTR termptr , nextptr ;
	    wire_boundary2( c , dimptr )
		DBOXPTR dimptr ;
		INT c ;
	    dbox_pos( antrmptr ) 
		TEBOXPTR antrmptr ;
	    dbox_pos_swap( antrmptr ) 
		TEBOXPTR antrmptr ;
DATE:	    Mar 27, 1989 
REVISIONS:  May 19, 1989 by Carl --- for gate swaps
	    Nov  4, 1989 - made memoptr dynamic instead of fixed.
	    Mon Nov 18 16:28:30 EST 1991 - fixed initialization problem
		with check_validbound.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) dimbox.c (Yale) version 4.5 11/18/91" ;
#endif
#endif

#include "standard.h"
#define break_pt 5

static INT prev_netS , curr_netS , test_newnetS ;
static INT validLS , validRS , validBS , validTS ;
static INT kS ;
static PINBOXPTR *memoptrS ;

init_dimbox()
{
    INT maxpin, get_max_pin() ;

    maxpin = get_max_pin() ;
    memoptrS = (PINBOXPTR *)
	Ysafe_malloc( (++maxpin) * sizeof(PINBOXPTR) ) ;

} /* end init_dimbox */


new_dbox( antrmptr , costptr )
PINBOXPTR antrmptr ;
INT *costptr ;
{

register DBOXPTR  dimptr   ;
PINBOXPTR termptr , nextptr ;

prev_netS = -1 ;
validRS = validLS = validBS = validTS = 0 ;
for( termptr = antrmptr ; termptr ; termptr = termptr->nextpin ) {
    curr_netS = termptr->net ;
    test_newnetS = ( prev_netS == curr_netS ) ? 0 : 1 ;
    if( test_newnetS ) {
	memoptrS[ kS = 0 ] = termptr ;
    }
    prev_netS = curr_netS ;
    dimptr = netarrayG[ curr_netS ] ;
    memoptrS[ ++kS ] = nextptr = termptr->nextpin ;
    if( dimptr->dflag == 0 || dimptr->ignore == 1 ) { 
	continue ;
    }
    if( dimptr->numpins <= break_pt ) {
	dimptr->dflag = 0 ;
	wire_boundary1( dimptr ) ;
    } else { 
	check_validbound( dimptr , termptr , nextptr ) ;
	if( nextptr && curr_netS == nextptr->net ) {
		continue ;
	}
	dimptr->dflag = 0 ;
	wire_boundary2( validLS + 2*validBS + 4*validRS + 8*validTS  
			                             , dimptr ) ;
        validRS = validLS = validBS = validTS = 0 ;
    }
    /* add2net_set remembers which nets are modified by this move */
    add2net_set( curr_netS ) ;

    dimptr->newhalfPx = dimptr->newxmax - dimptr->newxmin ; 
    dimptr->newhalfPy = dimptr->newymax - dimptr->newymin ;

    *costptr = *costptr + ( dimptr->newhalfPx - dimptr->halfPx +
	    (INT)(vertical_wire_weightG * (DOUBLE)dimptr->newhalfPy) -
	    (INT)(vertical_wire_weightG * (DOUBLE)dimptr->halfPy) );
}
}


new_dbox2( antrmptr , bntrmptr , costptr )
PINBOXPTR antrmptr , bntrmptr ;
INT *costptr ;
{

DBOXPTR  dimptr   ;
PINBOXPTR termptr , nextptr ;
INT anet , bnet ;
INT aflag ;

if( antrmptr == PINNULL ) {
    new_dbox( bntrmptr , costptr ) ;
    return ;
} 
if( bntrmptr == PINNULL ) {
    new_dbox( antrmptr , costptr ) ;
    return ;
}
validRS = validLS = validBS = validTS = 0 ;
aflag = ( antrmptr->net <= bntrmptr->net ) ? 1 : 0 ;
if( aflag ) {
    termptr = antrmptr ;
    antrmptr = antrmptr->nextpin ;
} else {
    termptr = bntrmptr ;
    bntrmptr = bntrmptr->nextpin ;
}
prev_netS = -1 ;
for( ; termptr ; termptr = nextptr ) { 
    curr_netS = termptr->net ;
    test_newnetS = ( prev_netS == curr_netS ) ? 0 : 1 ;
    if( test_newnetS ) {
	memoptrS[ kS = 0 ] = termptr ;
    }
    prev_netS = curr_netS ;
    dimptr = netarrayG[ curr_netS ] ;
    if( antrmptr && bntrmptr ) {
	if( (anet = antrmptr->net)
	    < (bnet = bntrmptr->net) ) {
	    nextptr = antrmptr ;
	    antrmptr = antrmptr->nextpin ;
	    aflag = 1 ;
	} else if( anet > bnet ) {
	    nextptr = bntrmptr ;
	    bntrmptr = bntrmptr->nextpin ;
	    aflag = 0 ;
	} else {  /* anet is equal to bnet */
	    if( aflag ) { /* the current pointer is on antrmptr list*/
		nextptr = antrmptr ;
		antrmptr = antrmptr->nextpin ;
	    } else {      /* the current pointer is on bntrmptr list*/
		nextptr = bntrmptr ;
		bntrmptr = bntrmptr->nextpin ;
	    }
	}
    } else if( antrmptr == PINNULL && bntrmptr != PINNULL ) {
	nextptr = bntrmptr ;
	bntrmptr = bntrmptr->nextpin ;
    } else if( antrmptr && bntrmptr == PINNULL ) {
	nextptr = antrmptr ;
	antrmptr = antrmptr->nextpin ;
    } else {
	nextptr = PINNULL ;
    }
    memoptrS[ ++kS ] = nextptr ;
    if( dimptr->dflag == 0 || dimptr->ignore == 1 ) { 
	continue ;
    }
    if( dimptr->numpins <= break_pt ) {
	dimptr->dflag = 0 ;
	wire_boundary1( dimptr ) ;
    } else { /* The pin No. of the net is greater than break_pt */
	check_validbound( dimptr , termptr , nextptr ) ;
	if( nextptr && curr_netS == nextptr->net ) {
	    continue ;
	}
	dimptr->dflag = 0 ;
	wire_boundary2( validLS + 2*validBS + 4*validRS + 8*validTS  
			     , dimptr ) ;
        validRS = validLS = validBS = validTS = 0 ;
    }
    /* add2net_set remembers which nets are modified by this move */
    add2net_set( curr_netS ) ;
    dimptr->newhalfPx = dimptr->newxmax - dimptr->newxmin ; 
    dimptr->newhalfPy = dimptr->newymax - dimptr->newymin ;

    *costptr = *costptr + ( dimptr->newhalfPx - dimptr->halfPx +
	    (INT)(vertical_wire_weightG * (DOUBLE)dimptr->newhalfPy) -
	    (INT)(vertical_wire_weightG * (DOUBLE)dimptr->halfPy) );
}
}


wire_boundary1( dimptr )
DBOXPTR dimptr ;
{

PINBOXPTR netptr ;
INT x , y ;

if( netptr = dimptr->pins ) {
    if( netptr->flag == 1 ) {
	dimptr->newxmin = dimptr->newxmax = netptr->newx ;
	dimptr->newymin = dimptr->newymax = netptr->newy ;
	netptr->flag = 0 ;
    } else {
	dimptr->newxmin = dimptr->newxmax = netptr->xpos ;
	dimptr->newymin = dimptr->newymax = netptr->ypos ;
    }
}
for( netptr = netptr->next ; netptr; netptr = netptr->next ) {
    if( netptr->flag == 1 ) {
	x = netptr->newx ;
	y = netptr->newy ;
	netptr->flag = 0 ;
    } else {
	x = netptr->xpos ;
	y = netptr->ypos ;
    }
    if( x < dimptr->newxmin ) {
	dimptr->newxmin = x ;
    } else if( x > dimptr->newxmax ) {
	dimptr->newxmax = x ;
    }
    if( y < dimptr->newymin ) {
	dimptr->newymin = y ;
    } else if( y > dimptr->newymax ) {
	dimptr->newymax = y ;
    }
}
}

check_validbound( dimptr , termptr , nextptr )
DBOXPTR dimptr ;
PINBOXPTR termptr , nextptr ;
{

INT nux , nuy , x , y ;

nux = termptr->newx ;
nuy = termptr->newy ;
x   = termptr->xpos ;
y   = termptr->ypos ;
if( test_newnetS ) {
    dimptr->newxmax = dimptr->xmax ;
    dimptr->newxmin = dimptr->xmin ;
    dimptr->newymax = dimptr->ymax ;
    dimptr->newymin = dimptr->ymin ;
    dimptr->newRnum = dimptr->Rnum ;
    dimptr->newLnum = dimptr->Lnum ;
    dimptr->newBnum = dimptr->Bnum ;
    dimptr->newTnum = dimptr->Tnum ;
} 

if( nux > dimptr->newxmax ) {
    dimptr->newxmax = nux ;
    dimptr->newRnum = 1 ;
    validRS = 1 ;
} else if( x != nux && nux == dimptr->newxmax ) {
    dimptr->newRnum++ ;
    validRS = 1 ;
} else if( x == dimptr->newxmax && nux < dimptr->newxmax ) {
    dimptr->newRnum-- ;
}

if( nux < dimptr->newxmin ) {
    dimptr->newxmin = nux ;
    dimptr->newLnum = 1 ;
    validLS = 1 ;
} else if( x != nux &&  nux == dimptr->newxmin ) {
    dimptr->newLnum++ ;
    validLS = 1 ;
} else if( x == dimptr->newxmin && nux > dimptr->newxmin ) {
    dimptr->newLnum-- ;
}

if( nuy > dimptr->newymax ) {
    dimptr->newymax = nuy ;
    dimptr->newTnum = 1 ;
    validTS = 1 ;
} else if( y != nuy &&  nuy == dimptr->newymax ) {
    dimptr->newTnum++ ;
    validTS = 1 ;
} else if( y == dimptr->newymax && nuy < dimptr->newymax ) {
    dimptr->newTnum-- ;
}

if( nuy < dimptr->newymin ) {
    dimptr->newymin = nuy ;
    dimptr->newBnum = 1 ;
    validBS = 1 ;
} else if( y != nuy &&  nuy == dimptr->newymin ) {
    dimptr->newBnum++ ;
    validBS = 1 ;
} else if( y == dimptr->newymin && nuy > dimptr->newymin ) {
    dimptr->newBnum-- ;
}

if( nextptr == PINNULL || curr_netS != nextptr->net ) { 
    if( !validRS && dimptr->newxmax == dimptr->xmax
			   && dimptr->newRnum > 0 ) {
	validRS = 1 ;
    }
    if( !validLS && dimptr->newxmin == dimptr->xmin
			   && dimptr->newLnum > 0 ) {
	validLS = 1 ;
    }
    if( !validTS && dimptr->newymax == dimptr->ymax
			   && dimptr->newTnum > 0 ) {
	validTS = 1 ;
    }
    if( !validBS && dimptr->newymin == dimptr->ymin
			   && dimptr->newBnum > 0 ) {
	validBS = 1 ;
    }
}
}

wire_boundary2( c , dimptr )
DBOXPTR dimptr ;
INT c ;
{

PINBOXPTR netptr ;
INT x , y , i ;

if( c == 15 ) {
    for( i = 0 ; i <= kS-1 ; i++ ) {
        memoptrS[i]->flag = 0 ;
    }
    return ;
}
if( netptr = dimptr->pins ) {
    if( netptr->flag == 1 ) {
	netptr->flag = 0 ;
	switch( c ) {
	case  0 :  /* L , R , B , T false */
	    dimptr->newxmin = dimptr->newxmax = netptr->newx ;
	    dimptr->newymin = dimptr->newymax = netptr->newy ;
	    break ;
	case  1 :  /* R , B , T false */
	    dimptr->newxmax = netptr->newx ;
	    dimptr->newymin = dimptr->newymax = netptr->newy ;
	    break ;
	case  2 :  /* L , R , T false */
	    dimptr->newxmin = dimptr->newxmax = netptr->newx ;
	    dimptr->newymax = netptr->newy ;
	    break ;
	case  3 :  /* R , T false */
	    dimptr->newxmax = netptr->newx ;
	    dimptr->newymax = netptr->newy ;
	    break ;
	case  4 :  /* L , B , T false */
	    dimptr->newxmin = netptr->newx ;
	    dimptr->newymin = dimptr->newymax = netptr->newy ;
	    break ;
	case  5 :  /* B , T false */
	    dimptr->newymin = dimptr->newymax = netptr->newy ;
	    break ;
	case  6 :  /* L , T false */
	    dimptr->newxmin = netptr->newx ;
	    dimptr->newymax = netptr->newy ;
	    break ;
	case  7 :  /* T false */
	    dimptr->newymax = netptr->newy ;
	    break ;
	case  8 :  /* L , R , B false */
	    dimptr->newxmin = dimptr->newxmax = netptr->newx ;
	    dimptr->newymin = netptr->newy ;
	    break ;
	case  9 :  /* R , B false */
	    dimptr->newxmax = netptr->newx ;
	    dimptr->newymin = netptr->newy ;
	    break ;
	case 10 :  /* L , R false */
	    dimptr->newxmin = dimptr->newxmax = netptr->newx ;
	    break ;
	case 11 :  /* R false */
	    dimptr->newxmax = netptr->newx ;
	    break ;
	case 12 :  /* L , B false */
	    dimptr->newxmin = netptr->newx ;
	    dimptr->newymin = netptr->newy ;
	    break ;
	case 13 :  /* B false */
	    dimptr->newymin = netptr->newy ;
	    break ;
	case 14 :  /* L false */
	    dimptr->newxmin = netptr->newx ;
	    break ;
	}
    } else {
	switch( c ) {
	case  0 :  /* L , R , B , T false */
	    dimptr->newxmin = dimptr->newxmax = netptr->xpos ;
	    dimptr->newymin = dimptr->newymax = netptr->ypos ;
	    break ;
	case  1 :  /* R , B , T false */
	    dimptr->newxmax = netptr->xpos ;
	    dimptr->newymin = dimptr->newymax = netptr->ypos ;
	    break ;
	case  2 :  /* L , R , T false */
	    dimptr->newxmin = dimptr->newxmax = netptr->xpos ;
	    dimptr->newymax = netptr->ypos ;
	    break ;
	case  3 :  /* R , T false */
	    dimptr->newxmax = netptr->xpos ;
	    dimptr->newymax = netptr->ypos ;
	    break ;
	case  4 :  /* L , B , T false */
	    dimptr->newxmin = netptr->xpos ;
	    dimptr->newymin = dimptr->newymax = netptr->ypos ;
	    break ;
	case  5 :  /* B , T false */
	    dimptr->newymin = dimptr->newymax = netptr->ypos ;
	    break ;
	case  6 :  /* L , T false */
	    dimptr->newxmin = netptr->xpos ;
	    dimptr->newymax = netptr->ypos ;
	    break ;
	case  7 :  /* T false */
	    dimptr->newymax = netptr->ypos ;
	    break ;
	case  8 :  /* L , R , B false */
	    dimptr->newxmin = dimptr->newxmax = netptr->xpos ;
	    dimptr->newymin = netptr->ypos ;
	    break ;
	case  9 :  /* R , B false */
	    dimptr->newxmax = netptr->xpos ;
	    dimptr->newymin = netptr->ypos ;
	    break ;
	case 10 :  /* L , R false */
	    dimptr->newxmin = dimptr->newxmax = netptr->xpos ;
	    break ;
	case 11 :  /* R false */
	    dimptr->newxmax = netptr->xpos ;
	    break ;
	case 12 :  /* L , B false */
	    dimptr->newxmin = netptr->xpos ;
	    dimptr->newymin = netptr->ypos ;
	    break ;
	case 13 :  /* B false */
	    dimptr->newymin = netptr->ypos ;
	    break ;
	case 14 :  /* L false */
	    dimptr->newxmin = netptr->xpos ;
	    break ;
	}
    }
}
if( !validLS ) {
    dimptr->newLnum = 1 ;
}
if( !validRS ) {
    dimptr->newRnum = 1 ;
}
if( !validBS ) {
    dimptr->newBnum = 1 ;
}
if( !validTS ) {
    dimptr->newTnum = 1 ;
}
for( netptr = netptr->next ; netptr; netptr = netptr->next ) {
    if( netptr->flag == 1 ) {
	x = netptr->newx ;
	y = netptr->newy ;
	netptr->flag = 0 ;
    } else {
	x = netptr->xpos ;
	y = netptr->ypos ;
    }
    switch( c ) {
    case  0 :  /* L , R , B , T false */
	if( x < dimptr->newxmin ) {
	    dimptr->newxmin = x ;
	    dimptr->newLnum = 1 ;
	} else if( x > dimptr->newxmax ) {
	    dimptr->newxmax = x ;
	    dimptr->newRnum = 1 ;
	} else if( x == dimptr->newxmin ) {
	    dimptr->newLnum++ ;
	    if( x == dimptr->newxmax ) {
		dimptr->newRnum++ ;
	    }
	} else if( x == dimptr->newxmax ) { 
	    dimptr->newRnum++ ;
	}
	if( y < dimptr->newymin ) {
	    dimptr->newymin = y ;
	    dimptr->newBnum = 1 ;
	} else if( y > dimptr->newymax ) {
	    dimptr->newymax = y ;
	    dimptr->newTnum = 1 ;
	} else if( y == dimptr->newymin ) {
	    dimptr->newBnum++ ;
	    if( y == dimptr->newymax ) {
		dimptr->newTnum++ ;
	    }
	} else if( y == dimptr->newymax ) { 
	    dimptr->newTnum++ ;
	}
	break ;
    case  1 :  /* R , B , T false */
	if( x > dimptr->newxmax ) {
	    dimptr->newxmax = x ;
	    dimptr->newRnum = 1 ;
	} else if( x == dimptr->newxmax ) { 
	    dimptr->newRnum++ ;
	}
	if( y < dimptr->newymin ) {
	    dimptr->newymin = y ;
	    dimptr->newBnum = 1 ;
	} else if( y > dimptr->newymax ) {
	    dimptr->newymax = y ;
	    dimptr->newTnum = 1 ;
	} else if( y == dimptr->newymin ) {
	    dimptr->newBnum++ ;
	    if( y == dimptr->newymax ) {
		dimptr->newTnum++ ;
	    }
	} else if( y == dimptr->newymax ) { 
	    dimptr->newTnum++ ;
	}
	break ;
    case  2 :  /* L , R , T false */
	if( x < dimptr->newxmin ) {
	    dimptr->newxmin = x ;
	    dimptr->newLnum = 1 ;
	} else if( x > dimptr->newxmax ) {
	    dimptr->newxmax = x ;
	    dimptr->newRnum = 1 ;
	} else if( x == dimptr->newxmin ) {
	    dimptr->newLnum++ ;
	    if( x == dimptr->newxmax ) {
		dimptr->newRnum++ ;
	    }
	} else if( x == dimptr->newxmax ) { 
	    dimptr->newRnum++ ;
	}
	if( y > dimptr->newymax ) {
	    dimptr->newymax = y ;
	    dimptr->newTnum = 1 ;
	} else if( y == dimptr->newymax ) { 
	    dimptr->newTnum++ ;
	}
	break ;
    case  3 :  /* R , T false */
	if( x > dimptr->newxmax ) {
	    dimptr->newxmax = x ;
	    dimptr->newRnum = 1 ;
	} else if( x == dimptr->newxmax ) { 
	    dimptr->newRnum++ ;
	}
	if( y > dimptr->newymax ) {
	    dimptr->newymax = y ;
	    dimptr->newTnum = 1 ;
	} else if( y == dimptr->newymax ) { 
	    dimptr->newTnum++ ;
	}
	break ;
    case  4 :  /* L , B , T false */
	if( x < dimptr->newxmin ) {
	    dimptr->newxmin = x ;
	    dimptr->newLnum = 1 ;
	} else if( x == dimptr->newxmin ) {
	    dimptr->newLnum++ ;
	}
	if( y < dimptr->newymin ) {
	    dimptr->newymin = y ;
	    dimptr->newBnum = 1 ;
	} else if( y > dimptr->newymax ) {
	    dimptr->newymax = y ;
	    dimptr->newTnum = 1 ;
	} else if( y == dimptr->newymin ) {
	    dimptr->newBnum++ ;
	    if( y == dimptr->newymax ) {
		dimptr->newTnum++ ;
	    }
	} else if( y == dimptr->newymax ) { 
	    dimptr->newTnum++ ;
	}
	break ;
    case  5 :  /* B , T false */
	if( y < dimptr->newymin ) {
	    dimptr->newymin = y ;
	    dimptr->newBnum = 1 ;
	} else if( y > dimptr->newymax ) {
	    dimptr->newymax = y ;
	    dimptr->newTnum = 1 ;
	} else if( y == dimptr->newymin ) {
	    dimptr->newBnum++ ;
	    if( y == dimptr->newymax ) {
		dimptr->newTnum++ ;
	    }
	} else if( y == dimptr->newymax ) { 
	    dimptr->newTnum++ ;
	}
	break ;
    case  6 :  /* L , T false */
	if( x < dimptr->newxmin ) {
	    dimptr->newxmin = x ;
	    dimptr->newLnum = 1 ;
	} else if( x == dimptr->newxmin ) {
	    dimptr->newLnum++ ;
	}
	if( y > dimptr->newymax ) {
	    dimptr->newymax = y ;
	    dimptr->newTnum = 1 ;
	} else if( y == dimptr->newymax ) { 
	    dimptr->newTnum++ ;
	}
	break ;
    case  7 :  /* T false */
	if( y > dimptr->newymax ) {
	    dimptr->newymax = y ;
	    dimptr->newTnum = 1 ;
	} else if( y == dimptr->newymax ) { 
	    dimptr->newTnum++ ;
	}
	break ;
    case  8 :  /* L , R , B false */
	if( x < dimptr->newxmin ) {
	    dimptr->newxmin = x ;
	    dimptr->newLnum = 1 ;
	} else if( x > dimptr->newxmax ) {
	    dimptr->newxmax = x ;
	    dimptr->newRnum = 1 ;
	} else if( x == dimptr->newxmin ) {
	    dimptr->newLnum++ ;
	    if( x == dimptr->newxmax ) {
		dimptr->newRnum++ ;
	    }
	} else if( x == dimptr->newxmax ) { 
	    dimptr->newRnum++ ;
	}
	if( y < dimptr->newymin ) {
	    dimptr->newymin = y ;
	    dimptr->newBnum = 1 ;
	} else if( y == dimptr->newymin ) {
	    dimptr->newBnum++ ;
	}
	break ;
    case  9 :  /* R , B false */
	if( x > dimptr->newxmax ) {
	    dimptr->newxmax = x ;
	    dimptr->newRnum = 1 ;
	} else if( x == dimptr->newxmax ) { 
	    dimptr->newRnum++ ;
	}
	if( y < dimptr->newymin ) {
	    dimptr->newymin = y ;
	    dimptr->newBnum = 1 ;
	} else if( y == dimptr->newymin ) {
	    dimptr->newBnum++ ;
	}
	break ;
    case 10 :  /* L , R false */
	if( x < dimptr->newxmin ) {
	    dimptr->newxmin = x ;
	    dimptr->newLnum = 1 ;
	} else if( x > dimptr->newxmax ) {
	    dimptr->newxmax = x ;
	    dimptr->newRnum = 1 ;
	} else if( x == dimptr->newxmin ) {
	    dimptr->newLnum++ ;
	    if( x == dimptr->newxmax ) {
		dimptr->newRnum++ ;
	    }
	} else if( x == dimptr->newxmax ) { 
	    dimptr->newRnum++ ;
	}
	break ;
    case 11 :  /* R false */
	if( x > dimptr->newxmax ) {
	    dimptr->newxmax = x ;
	    dimptr->newRnum = 1 ;
	} else if( x == dimptr->newxmax ) { 
	    dimptr->newRnum++ ;
	}
	break ;
    case 12 :  /* L , B false */
	if( x < dimptr->newxmin ) {
	    dimptr->newxmin = x ;
	    dimptr->newLnum = 1 ;
	} else if( x == dimptr->newxmin ) {
	    dimptr->newLnum++ ;
	}
	if( y < dimptr->newymin ) {
	    dimptr->newymin = y ;
	    dimptr->newBnum = 1 ;
	} else if( y == dimptr->newymin ) {
	    dimptr->newBnum++ ;
	}
	break ;
    case 13 :  /* B false */
	if( y < dimptr->newymin ) {
	    dimptr->newymin = y ;
	    dimptr->newBnum = 1 ;
	} else if( y == dimptr->newymin ) {
	    dimptr->newBnum++ ;
	}
	break ;
    case 14 :  /* L false */
	if( x < dimptr->newxmin ) {
	    dimptr->newxmin = x ;
	    dimptr->newLnum = 1 ;
	} else if( x == dimptr->newxmin ) {
	    dimptr->newLnum++ ;
	}
	break ;
    }
}
}

dbox_pos( antrmptr ) 
PINBOXPTR antrmptr ;
{

DBOXPTR dimptr ;
PINBOXPTR termptr ;

for( termptr = antrmptr ; termptr != PINNULL;termptr=termptr->nextpin ) {
    dimptr = netarrayG[ termptr->net ] ;
    dimptr->xmin = dimptr->newxmin ;
    dimptr->xmax = dimptr->newxmax ;
    dimptr->ymin = dimptr->newymin ;
    dimptr->ymax = dimptr->newymax ;
    dimptr->halfPx = dimptr->newhalfPx ;
    dimptr->halfPy = dimptr->newhalfPy ;
    if( dimptr->numpins >= break_pt + 1 ) { 
	dimptr->Lnum = dimptr->newLnum ;
	dimptr->Rnum = dimptr->newRnum ;
	dimptr->Bnum = dimptr->newBnum ;
	dimptr->Tnum = dimptr->newTnum ;
    }
    termptr->xpos = termptr->newx ;
    termptr->ypos = termptr->newy ;
}
}


dbox_pos_swap( antrmptr ) 
PINBOXPTR antrmptr ;
{

DBOXPTR dimptr ;
PINBOXPTR termptr ;

for( termptr = antrmptr ; termptr != PINNULL;termptr=termptr->nextpin ) {
    dimptr = netarrayG[ termptr->net ] ;
    dimptr->xmin = dimptr->newxmin ;
    dimptr->xmax = dimptr->newxmax ;
    dimptr->ymin = dimptr->newymin ;
    dimptr->ymax = dimptr->newymax ;
    dimptr->halfPx = dimptr->newhalfPx ;
    dimptr->halfPy = dimptr->newhalfPy ;
    if( dimptr->numpins >= break_pt + 1 ) { 
	dimptr->Lnum = dimptr->newLnum ;
	dimptr->Rnum = dimptr->newRnum ;
	dimptr->Bnum = dimptr->newBnum ;
	dimptr->Tnum = dimptr->newTnum ;
    }
}
}


init_dbox_pos_swap( antrmptr ) 
PINBOXPTR antrmptr ;
{

DBOXPTR dimptr ;
PINBOXPTR termptr ;

for( termptr = antrmptr ; termptr != PINNULL;termptr=termptr->nextpin ) {
    dimptr = netarrayG[ termptr->net ] ;
    dimptr->newxmin = dimptr->xmin ;
    dimptr->newxmax = dimptr->xmax ;
    dimptr->newymin = dimptr->ymin ;
    dimptr->newymax = dimptr->ymax ;
    dimptr->newhalfPx = dimptr->halfPx ;
    dimptr->newhalfPy = dimptr->halfPy ;
    if( dimptr->numpins >= break_pt + 1 ) { 
	dimptr->newLnum = dimptr->Lnum ;
	dimptr->newRnum = dimptr->Rnum ;
	dimptr->newBnum = dimptr->Bnum ;
	dimptr->newTnum = dimptr->Tnum ;
    }
}
}
