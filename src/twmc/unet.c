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
FILE:	    unet.c                                       
DESCRIPTION:Incremental bounding box routines.
CONTENTS:   INT unet( antrmptr )
		PINBOXPTR antrmptr ;
	    statice check_validbound( dimptr , termptr , nextptr )
		DBOXPTR dimptr ;
		TEBOXPTR termptr , nextptr ;
	    static wire_boundary1( netptr )
		NETBOXPTR netptr ;
	    static wire_boundary2( c , netptr )
		NETBOXPTR dimptr ;
		INT c ;
	    init_unet() ;
DATE:	    Mar 27, 1989 
REVISIONS:  Oct 21, 1989 - changed memoptrS to a pointer rather than
		a fixed array.  Added init_unet to initialize this 
		array.
	    Apr 23, 1990 - added assertion to make sure half perimeter
		does not go negative.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) unet.c (Yale) version 3.6 11/23/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>
#define BREAK_PT 5
static INT prev_netS , curr_netS , test_newnetS ;
static INT validLS , validRS , validBS , validTS ;
static PINBOXPTR  *memoptrS ;
static INT kS ;





static check_validbound();
static wire_boundary2();
static wire_boundary1();




init_unet()
{
    INT maxpin, get_max_pin() ;

    maxpin = get_max_pin() ;
    memoptrS = (PINBOXPTR *)
	Ysafe_malloc( (++maxpin) * sizeof(PINBOXPTR) ) ;

} /* end init_unet */

/* returns the incremental cost for a single cell */
INT unet( antrmptr )
PINBOXPTR antrmptr ;
{

    register NETBOXPTR  netptr ;
    register PINBOXPTR  pinptr ;
    register INT x, y ;
    PINBOXPTR termptr , nextptr ;
    INT cost ;
    /*  INT test ;  */

    prev_netS = -1 ;
    cost = 0 ;
    validRS = validLS = validBS = validTS = 0 ;
    for( termptr = antrmptr ; termptr ; termptr = termptr->nextpin ) {
	curr_netS = termptr->net ;
	test_newnetS = ( prev_netS == curr_netS ) ? 0 : 1 ;
	if( test_newnetS ) {
	    memoptrS[ kS = 0 ] = termptr ;
	}
	prev_netS = curr_netS ;
	netptr = netarrayG[ curr_netS ] ;
	memoptrS[ ++kS ] = nextptr = termptr->nextpin ;
	if( netptr->nflag == 0 || netptr->skip == 1 ) { 
	    continue ;
	}
	if( netptr->numpins <= BREAK_PT ) {
	    netptr->nflag = 0 ;

	    /* ***** CASE of small number of pins - normal case ***** */
	    if( pinptr = netptr->pins ) {
		if( pinptr->flag == 1 ) {
		    netptr->newxmin = netptr->newxmax = pinptr->newx ;
		    netptr->newymin = netptr->newymax = pinptr->newy ;
		    pinptr->flag = 0 ;
		} else {
		    netptr->newxmin = netptr->newxmax = pinptr->xpos ;
		    netptr->newymin = netptr->newymax = pinptr->ypos ;
		}
	    }
	    for( pinptr = pinptr->next ;pinptr; pinptr = pinptr->next ) {
		if( pinptr->flag == 1 ) {
		    x = pinptr->newx ;
		    y = pinptr->newy ;
		    pinptr->flag = 0 ;
		} else {
		    x = pinptr->xpos ;
		    y = pinptr->ypos ;
		}
		if( x < netptr->newxmin ) {
		    netptr->newxmin = x ;
		} else if( x > netptr->newxmax ) {
		    netptr->newxmax = x ;
		}
		if( y < netptr->newymin ) {
		    netptr->newymin = y ;
		} else if( y > netptr->newymax ) {
		    netptr->newymax = y ;
		}
	    }
	    /* *********** end case of small number of pins ******** */
	} else { 
	    check_validbound( netptr , termptr , nextptr ) ;
	    /* if we have another pin on same net continue since we */
	    /* have already taken care of it */
	    if( nextptr && curr_netS == nextptr->net ) {
		    continue ;
	    }
	    netptr->nflag = 0 ;
	    wire_boundary2( validLS + 2*validBS + 4*validRS + 8*validTS,
		netptr );
	    validRS = validLS = validBS = validTS = 0 ;
	}
	/* add2net_set remembers which nets are modified by this move */
	add2net_set( curr_netS ) ;

	netptr->newhalfPx = netptr->newxmax - netptr->newxmin ;
	netptr->newhalfPy = netptr->newymax - netptr->newymin ;
	ASSERT( netptr->newhalfPx >= 0,"unet","Half perimeter negative\n");
	ASSERT( netptr->newhalfPy >= 0,"unet","Half perimeter negative\n");

	/* new wire cost */
	cost += ( netptr->newhalfPx - netptr->halfPx +
	    (INT)(vertical_wire_weightG * (DOUBLE)netptr->newhalfPy) -
	    (INT)(vertical_wire_weightG * (DOUBLE)netptr->halfPy) );

    }
    return( cost ) ;
} /* end unet */

INT unet2( antrmptr, bntrmptr )
PINBOXPTR antrmptr , bntrmptr ;
{

    NETBOXPTR  netptr   ;
    PINBOXPTR termptr , nextptr ;
    INT anet , bnet ;
    INT aflag ;
    INT deltaP ; /* change in half perimeter bounding box */ 
    INT cost ;
    /*  INT test ;  */

    if( !antrmptr ) {
	return( unet( bntrmptr ) ) ;
    } 
    if( !bntrmptr ) {
	return( unet( antrmptr ) ) ;
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
    cost = 0 ;
    for( ; termptr ; termptr = nextptr ) { 
	curr_netS = termptr->net ;
	test_newnetS = ( prev_netS == curr_netS ) ? 0 : 1 ;
	if( test_newnetS ) {
	    memoptrS[ kS = 0 ] = termptr ;
	}
	prev_netS = curr_netS ;
	netptr = netarrayG[ curr_netS ] ;
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
	} else if( !antrmptr && bntrmptr ) {
	    nextptr = bntrmptr ;
	    bntrmptr = bntrmptr->nextpin ;
	} else if( antrmptr && !bntrmptr ) {
	    nextptr = antrmptr ;
	    antrmptr = antrmptr->nextpin ;
	} else {
	    nextptr = NIL(PINBOXPTR) ;
	}
	memoptrS[ ++kS ] = nextptr ;
	if( netptr->nflag == FALSE || netptr->skip == TRUE ) { 
	    continue ;
	}
	if( netptr->numpins <= BREAK_PT ) {
	    netptr->nflag = FALSE ;
	    wire_boundary1( netptr ) ;
	} else { /* The pin No. of the net is greater than break_pt */
	    check_validbound( netptr , termptr , nextptr ) ;
	    if( nextptr && curr_netS == nextptr->net ) {
		continue ;
	    }
	    netptr->nflag = FALSE ;
	    wire_boundary2( validLS + 2*validBS + 4*validRS + 8*validTS,
		netptr ) ;
	    validRS = validLS = validBS = validTS = 0 ;
	}
	/* add2net_set remembers which nets are modified by this move */
	add2net_set( curr_netS ) ;
	netptr->newhalfPx = netptr->newxmax - netptr->newxmin ;
	netptr->newhalfPy = netptr->newymax - netptr->newymin ;
	ASSERT( netptr->newhalfPx >= 0,"unet","Half perimeter negative\n");
	ASSERT( netptr->newhalfPy >= 0,"unet","Half perimeter negative\n");

	/* new wire cost */
	cost += ( netptr->newhalfPx - netptr->halfPx +
	    (INT)(vertical_wire_weightG * (DOUBLE)netptr->newhalfPy) -
	    (INT)(vertical_wire_weightG * (DOUBLE)netptr->halfPy) );
    }
    return( cost ) ;
} /* end unet2 */

static check_validbound( netptr , termptr , nextptr )
NETBOXPTR netptr ;
PINBOXPTR termptr , nextptr ;
{

    INT nux , nuy , x , y ;

    nux = termptr->newx ;
    nuy = termptr->newy ;
    x   = termptr->xpos ;
    y   = termptr->ypos ;
    if( test_newnetS ) {
	netptr->newxmax = netptr->xmax ;
	netptr->newxmin = netptr->xmin ;
	netptr->newymax = netptr->ymax ;
	netptr->newymin = netptr->ymin ;
	netptr->newRnum = netptr->Rnum ;
	netptr->newLnum = netptr->Lnum ;
	netptr->newBnum = netptr->Bnum ;
	netptr->newTnum = netptr->Tnum ;
    } 

    if( nux > netptr->newxmax ) {
	netptr->newxmax = nux ;
	netptr->newRnum = 1 ;
	validRS = 1 ;
    } else if( x != nux && nux == netptr->newxmax ) {
	netptr->newRnum++ ;
	validRS = 1 ;
    } else if( x == netptr->newxmax && nux < netptr->newxmax ) {
	netptr->newRnum-- ;
    }

    if( nux < netptr->newxmin ) {
	netptr->newxmin = nux ;
	netptr->newLnum = 1 ;
	validLS = 1 ;
    } else if( x != nux &&  nux == netptr->newxmin ) {
	netptr->newLnum++ ;
	validLS = 1 ;
    } else if( x == netptr->newxmin && nux > netptr->newxmin ) {
	netptr->newLnum-- ;
    }

    if( nuy > netptr->newymax ) {
	netptr->newymax = nuy ;
	netptr->newTnum = 1 ;
	validTS = 1 ;
    } else if( y != nuy &&  nuy == netptr->newymax ) {
	netptr->newTnum++ ;
	validTS = 1 ;
    } else if( y == netptr->newymax && nuy < netptr->newymax ) {
	netptr->newTnum-- ;
    }

    if( nuy < netptr->newymin ) {
	netptr->newymin = nuy ;
	netptr->newBnum = 1 ;
	validBS = 1 ;
    } else if( y != nuy &&  nuy == netptr->newymin ) {
	netptr->newBnum++ ;
	validBS = 1 ;
    } else if( y == netptr->newymin && nuy > netptr->newymin ) {
	netptr->newBnum-- ;
    }

    if( !nextptr || curr_netS != nextptr->net ) { 
	if( !validRS && netptr->newxmax == netptr->xmax
			       && netptr->newRnum > 0 ) {
	    validRS = 1 ;
	}
	if( !validLS && netptr->newxmin == netptr->xmin
			       && netptr->newLnum > 0 ) {
	    validLS = 1 ;
	}
	if( !validTS && netptr->newymax == netptr->ymax
			       && netptr->newTnum > 0 ) {
	    validTS = 1 ;
	}
	if( !validBS && netptr->newymin == netptr->ymin
			       && netptr->newBnum > 0 ) {
	    validBS = 1 ;
	}
    }
} /* end check_validbound */

static wire_boundary2( c , netptr )
NETBOXPTR netptr ;
INT c ;
{

    PINBOXPTR pinptr ;
    INT x , y , i ;

    if( c == 15 ) {
	for( i = 0 ; i <= kS-1 ; i++ ) {
	    memoptrS[i]->flag = 0 ;
	}
	return ;
    }
    if( pinptr = netptr->pins ){
	if( pinptr->flag == 1 ) {
	    pinptr->flag = 0 ;
	    switch( c ) {
	    case  0 :  /* L , R , B , T false */
		netptr->newxmin = netptr->newxmax = pinptr->newx ;
		netptr->newymin = netptr->newymax = pinptr->newy ;
		break ;
	    case  1 :  /* R , B , T false */
		netptr->newxmax = pinptr->newx ;
		netptr->newymin = netptr->newymax = pinptr->newy ;
		break ;
	    case  2 :  /* L , R , T false */
		netptr->newxmin = netptr->newxmax = pinptr->newx ;
		netptr->newymax = pinptr->newy ;
		break ;
	    case  3 :  /* R , T false */
		netptr->newxmax = pinptr->newx ;
		netptr->newymax = pinptr->newy ;
		break ;
	    case  4 :  /* L , B , T false */
		netptr->newxmin = pinptr->newx ;
		netptr->newymin = netptr->newymax = pinptr->newy ;
		break ;
	    case  5 :  /* B , T false */
		netptr->newymin = netptr->newymax = pinptr->newy ;
		break ;
	    case  6 :  /* L , T false */
		netptr->newxmin = pinptr->newx ;
		netptr->newymax = pinptr->newy ;
		break ;
	    case  7 :  /* T false */
		netptr->newymax = pinptr->newy ;
		break ;
	    case  8 :  /* L , R , B false */
		netptr->newxmin = netptr->newxmax = pinptr->newx ;
		netptr->newymin = pinptr->newy ;
		break ;
	    case  9 :  /* R , B false */
		netptr->newxmax = pinptr->newx ;
		netptr->newymin = pinptr->newy ;
		break ;
	    case 10 :  /* L , R false */
		netptr->newxmin = netptr->newxmax = pinptr->newx ;
		break ;
	    case 11 :  /* R false */
		netptr->newxmax = pinptr->newx ;
		break ;
	    case 12 :  /* L , B false */
		netptr->newxmin = pinptr->newx ;
		netptr->newymin = pinptr->newy ;
		break ;
	    case 13 :  /* B false */
		netptr->newymin = pinptr->newy ;
		break ;
	    case 14 :  /* L false */
		netptr->newxmin = pinptr->newx ;
		break ;
	    }
	} else {
	    switch( c ) {
	    case  0 :  /* L , R , B , T false */
		netptr->newxmin = netptr->newxmax = pinptr->xpos ;
		netptr->newymin = netptr->newymax = pinptr->ypos ;
		break ;
	    case  1 :  /* R , B , T false */
		netptr->newxmax = pinptr->xpos ;
		netptr->newymin = netptr->newymax = pinptr->ypos ;
		break ;
	    case  2 :  /* L , R , T false */
		netptr->newxmin = netptr->newxmax = pinptr->xpos ;
		netptr->newymax = pinptr->ypos ;
		break ;
	    case  3 :  /* R , T false */
		netptr->newxmax = pinptr->xpos ;
		netptr->newymax = pinptr->ypos ;
		break ;
	    case  4 :  /* L , B , T false */
		netptr->newxmin = pinptr->xpos ;
		netptr->newymin = netptr->newymax = pinptr->ypos ;
		break ;
	    case  5 :  /* B , T false */
		netptr->newymin = netptr->newymax = pinptr->ypos ;
		break ;
	    case  6 :  /* L , T false */
		netptr->newxmin = pinptr->xpos ;
		netptr->newymax = pinptr->ypos ;
		break ;
	    case  7 :  /* T false */
		netptr->newymax = pinptr->ypos ;
		break ;
	    case  8 :  /* L , R , B false */
		netptr->newxmin = netptr->newxmax = pinptr->xpos ;
		netptr->newymin = pinptr->ypos ;
		break ;
	    case  9 :  /* R , B false */
		netptr->newxmax = pinptr->xpos ;
		netptr->newymin = pinptr->ypos ;
		break ;
	    case 10 :  /* L , R false */
		netptr->newxmin = netptr->newxmax = pinptr->xpos ;
		break ;
	    case 11 :  /* R false */
		netptr->newxmax = pinptr->xpos ;
		break ;
	    case 12 :  /* L , B false */
		netptr->newxmin = pinptr->xpos ;
		netptr->newymin = pinptr->ypos ;
		break ;
	    case 13 :  /* B false */
		netptr->newymin = pinptr->ypos ;
		break ;
	    case 14 :  /* L false */
		netptr->newxmin = pinptr->xpos ;
		break ;
	    }
	}
    }
    if( !validLS ) {
	netptr->newLnum = 1 ;
    }
    if( !validRS ) {
	netptr->newRnum = 1 ;
    }
    if( !validBS ) {
	netptr->newBnum = 1 ;
    }
    if( !validTS ) {
	netptr->newTnum = 1 ;
    }
    for( pinptr = pinptr->next ; pinptr ; pinptr = pinptr->next ) {
	if( pinptr->flag == 1 ) {
	    x = pinptr->newx ;
	    y = pinptr->newy ;
	    pinptr->flag = 0 ;
	} else {
	    x = pinptr->xpos ;
	    y = pinptr->ypos ;
	}
	switch( c ) {
	case  0 :  /* L , R , B , T false */
	    if( x < netptr->newxmin ) {
		netptr->newxmin = x ;
		netptr->newLnum = 1 ;
	    } else if( x > netptr->newxmax ) {
		netptr->newxmax = x ;
		netptr->newRnum = 1 ;
	    } else if( x == netptr->newxmin ) {
		netptr->newLnum++ ;
		if( x == netptr->newxmax ) {
		    netptr->newRnum++ ;
		}
	    } else if( x == netptr->newxmax ) { 
		netptr->newRnum++ ;
	    }
	    if( y < netptr->newymin ) {
		netptr->newymin = y ;
		netptr->newBnum = 1 ;
	    } else if( y > netptr->newymax ) {
		netptr->newymax = y ;
		netptr->newTnum = 1 ;
	    } else if( y == netptr->newymin ) {
		netptr->newBnum++ ;
		if( y == netptr->newymax ) {
		    netptr->newTnum++ ;
		}
	    } else if( y == netptr->newymax ) { 
		netptr->newTnum++ ;
	    }
	    break ;
	case  1 :  /* R , B , T false */
	    if( x > netptr->newxmax ) {
		netptr->newxmax = x ;
		netptr->newRnum = 1 ;
	    } else if( x == netptr->newxmax ) { 
		netptr->newRnum++ ;
	    }
	    if( y < netptr->newymin ) {
		netptr->newymin = y ;
		netptr->newBnum = 1 ;
	    } else if( y > netptr->newymax ) {
		netptr->newymax = y ;
		netptr->newTnum = 1 ;
	    } else if( y == netptr->newymin ) {
		netptr->newBnum++ ;
		if( y == netptr->newymax ) {
		    netptr->newTnum++ ;
		}
	    } else if( y == netptr->newymax ) { 
		netptr->newTnum++ ;
	    }
	    break ;
	case  2 :  /* L , R , T false */
	    if( x < netptr->newxmin ) {
		netptr->newxmin = x ;
		netptr->newLnum = 1 ;
	    } else if( x > netptr->newxmax ) {
		netptr->newxmax = x ;
		netptr->newRnum = 1 ;
	    } else if( x == netptr->newxmin ) {
		netptr->newLnum++ ;
		if( x == netptr->newxmax ) {
		    netptr->newRnum++ ;
		}
	    } else if( x == netptr->newxmax ) { 
		netptr->newRnum++ ;
	    }
	    if( y > netptr->newymax ) {
		netptr->newymax = y ;
		netptr->newTnum = 1 ;
	    } else if( y == netptr->newymax ) { 
		netptr->newTnum++ ;
	    }
	    break ;
	case  3 :  /* R , T false */
	    if( x > netptr->newxmax ) {
		netptr->newxmax = x ;
		netptr->newRnum = 1 ;
	    } else if( x == netptr->newxmax ) { 
		netptr->newRnum++ ;
	    }
	    if( y > netptr->newymax ) {
		netptr->newymax = y ;
		netptr->newTnum = 1 ;
	    } else if( y == netptr->newymax ) { 
		netptr->newTnum++ ;
	    }
	    break ;
	case  4 :  /* L , B , T false */
	    if( x < netptr->newxmin ) {
		netptr->newxmin = x ;
		netptr->newLnum = 1 ;
	    } else if( x == netptr->newxmin ) {
		netptr->newLnum++ ;
	    }
	    if( y < netptr->newymin ) {
		netptr->newymin = y ;
		netptr->newBnum = 1 ;
	    } else if( y > netptr->newymax ) {
		netptr->newymax = y ;
		netptr->newTnum = 1 ;
	    } else if( y == netptr->newymin ) {
		netptr->newBnum++ ;
		if( y == netptr->newymax ) {
		    netptr->newTnum++ ;
		}
	    } else if( y == netptr->newymax ) { 
		netptr->newTnum++ ;
	    }
	    break ;
	case  5 :  /* B , T false */
	    if( y < netptr->newymin ) {
		netptr->newymin = y ;
		netptr->newBnum = 1 ;
	    } else if( y > netptr->newymax ) {
		netptr->newymax = y ;
		netptr->newTnum = 1 ;
	    } else if( y == netptr->newymin ) {
		netptr->newBnum++ ;
		if( y == netptr->newymax ) {
		    netptr->newTnum++ ;
		}
	    } else if( y == netptr->newymax ) { 
		netptr->newTnum++ ;
	    }
	    break ;
	case  6 :  /* L , T false */
	    if( x < netptr->newxmin ) {
		netptr->newxmin = x ;
		netptr->newLnum = 1 ;
	    } else if( x == netptr->newxmin ) {
		netptr->newLnum++ ;
	    }
	    if( y > netptr->newymax ) {
		netptr->newymax = y ;
		netptr->newTnum = 1 ;
	    } else if( y == netptr->newymax ) { 
		netptr->newTnum++ ;
	    }
	    break ;
	case  7 :  /* T false */
	    if( y > netptr->newymax ) {
		netptr->newymax = y ;
		netptr->newTnum = 1 ;
	    } else if( y == netptr->newymax ) { 
		netptr->newTnum++ ;
	    }
	    break ;
	case  8 :  /* L , R , B false */
	    if( x < netptr->newxmin ) {
		netptr->newxmin = x ;
		netptr->newLnum = 1 ;
	    } else if( x > netptr->newxmax ) {
		netptr->newxmax = x ;
		netptr->newRnum = 1 ;
	    } else if( x == netptr->newxmin ) {
		netptr->newLnum++ ;
		if( x == netptr->newxmax ) {
		    netptr->newRnum++ ;
		}
	    } else if( x == netptr->newxmax ) { 
		netptr->newRnum++ ;
	    }
	    if( y < netptr->newymin ) {
		netptr->newymin = y ;
		netptr->newBnum = 1 ;
	    } else if( y == netptr->newymin ) {
		netptr->newBnum++ ;
	    }
	    break ;
	case  9 :  /* R , B false */
	    if( x > netptr->newxmax ) {
		netptr->newxmax = x ;
		netptr->newRnum = 1 ;
	    } else if( x == netptr->newxmax ) { 
		netptr->newRnum++ ;
	    }
	    if( y < netptr->newymin ) {
		netptr->newymin = y ;
		netptr->newBnum = 1 ;
	    } else if( y == netptr->newymin ) {
		netptr->newBnum++ ;
	    }
	    break ;
	case 10 :  /* L , R false */
	    if( x < netptr->newxmin ) {
		netptr->newxmin = x ;
		netptr->newLnum = 1 ;
	    } else if( x > netptr->newxmax ) {
		netptr->newxmax = x ;
		netptr->newRnum = 1 ;
	    } else if( x == netptr->newxmin ) {
		netptr->newLnum++ ;
		if( x == netptr->newxmax ) {
		    netptr->newRnum++ ;
		}
	    } else if( x == netptr->newxmax ) { 
		netptr->newRnum++ ;
	    }
	    break ;
	case 11 :  /* R false */
	    if( x > netptr->newxmax ) {
		netptr->newxmax = x ;
		netptr->newRnum = 1 ;
	    } else if( x == netptr->newxmax ) { 
		netptr->newRnum++ ;
	    }
	    break ;
	case 12 :  /* L , B false */
	    if( x < netptr->newxmin ) {
		netptr->newxmin = x ;
		netptr->newLnum = 1 ;
	    } else if( x == netptr->newxmin ) {
		netptr->newLnum++ ;
	    }
	    if( y < netptr->newymin ) {
		netptr->newymin = y ;
		netptr->newBnum = 1 ;
	    } else if( y == netptr->newymin ) {
		netptr->newBnum++ ;
	    }
	    break ;
	case 13 :  /* B false */
	    if( y < netptr->newymin ) {
		netptr->newymin = y ;
		netptr->newBnum = 1 ;
	    } else if( y == netptr->newymin ) {
		netptr->newBnum++ ;
	    }
	    break ;
	case 14 :  /* L false */
	    if( x < netptr->newxmin ) {
		netptr->newxmin = x ;
		netptr->newLnum = 1 ;
	    } else if( x == netptr->newxmin ) {
		netptr->newLnum++ ;
	    }
	    break ;
	}
    }
} /* end wire_boundary2 */

static wire_boundary1( netptr )
NETBOXPTR netptr ;
{

    PINBOXPTR pinptr ;
    INT x , y ;

    if( pinptr = netptr->pins ) {
	if( pinptr->flag == 1 ) {
	    netptr->newxmin = netptr->newxmax = pinptr->newx ;
	    netptr->newymin = netptr->newymax = pinptr->newy ;
	    pinptr->flag = 0 ;
	} else {
	    netptr->newxmin = netptr->newxmax = pinptr->xpos ;
	    netptr->newymin = netptr->newymax = pinptr->ypos ;
	}
    }
    for( pinptr = pinptr->next ; pinptr ; pinptr = pinptr->next ) {
	if( pinptr->flag == 1 ) {
	    x = pinptr->newx ;
	    y = pinptr->newy ;
	    pinptr->flag = 0 ;
	} else {
	    x = pinptr->xpos ;
	    y = pinptr->ypos ;
	}
	if( x < netptr->newxmin ) {
	    netptr->newxmin = x ;
	} else if( x > netptr->newxmax ) {
	    netptr->newxmax = x ;
	}
	if( y < netptr->newymin ) {
	    netptr->newymin = y ;
	} else if( y > netptr->newymax ) {
	    netptr->newymax = y ;
	}
    }
} /* wire_boundary1 */
