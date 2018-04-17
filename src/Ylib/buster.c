/*
 *   Copyright (C) 1988-1991 Yale University
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
FILE:	    buster.c
DESCRIPTION:This file contains the utility routine to break a 
	    rectilinear cell up into tiles.
CONTENTS:   YBUSTBOXPTR Ybuster()
	    INT Ybuster_init() ;
	    INT Ybuster_addpt( x, y ) ;
	    void Ybuster_free() ;
	    void Ybuster_clear() ;
		INT x, y ;
DATE:	    Aug  7, 1988 - rewrote to match new parser.
REVISIONS:  May  1, 1990 - made sure we cannot match the 0 
		record in the redundancy check for points.
	    May 4, 1990  - moved to the library since it now occurs
		in many files.
	    Aug 23,1990 - added Ybuster_free, Ybuster_clear.
	    Sat Dec 15 22:55:53 EST 1990 - added debug_verify
		code to buster.
	    Wed Apr 17 23:39:20 EDT 1991 - rewrote buster verify
		for more extensive error checking.
	    Thu Oct 17 11:08:18 EDT 1991 - added buster_chcek_rect.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) buster.c version 3.8 12/15/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/buster.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>

#define EXPECTEDPTS  50
/* detect problems with clockwise rotation pattern */
#define E_STATE 0
#define U_STATE 1
#define L_STATE 2
#define R_STATE 3
#define D_STATE 4
#define S_STATE 5  /* start state */

/* ####################### STATIC definitions ######################### */
static INT  cornerCountS ;     /* current number of corners             */
static INT  ptAllocS = 0 ;     /* current allocation for pts            */
static YBUSTBOXPTR ptS = NIL(YBUSTBOXPTR) ;/* array of pts for boundary */
static YBUSTBOXPTR resultS;    /* the array of pts to break into tiles  */
static INT cur_stateS ;        /* current state direction of edge */
static char *user_messageS;    /* output message on error */
/* ################## END STATIC definitions ########################## */
static BOOL check_rect( P4(INT xx1, INT yy1, INT xx2, INT yy2 ) ) ;

YBUSTBOXPTR Ybuster()
{

    INT k , Pk[2] , Pl[2] , Pm[2]  ;
    INT xmin , ymin , kmin , found ;

    if( cornerCountS <= 0 ){
	return( NIL(YBUSTBOXPTR) ) ;
    }
    D( "Ybuster/debug", Ybuster_verify( NULL ) ; ) ;

    /*  find Pk  */
    ymin = INT_MAX ;
    for( k = 1 ; k <= cornerCountS ; k++ ) {
	if( ptS[k].y < ymin ) {
	    ymin = ptS[k].y ;
	}
    }  /* we have the lowest y coordinate  */
    xmin = INT_MAX ;
    for( k = 1 ; k <=cornerCountS ; k++ ) {
	if( ptS[k].y == ymin ) {
	    if( ptS[k].x < xmin ) {
		xmin = ptS[k].x ;
		kmin = k ;
	    }
	}
    }  /*  we have the leftmost lowest corner  */
    Pk[0] = xmin ;
    Pk[1] = ymin ;
    xmin = INT_MAX ;
    for( k = 1 ; k <= cornerCountS ; k++ ) {
	if( k == kmin ) {
	    continue ;
	}
	if( ptS[k].y == ymin ) {
	    if( ptS[k].x < xmin ) {
		xmin = ptS[k].x ;
	    }
	}
    }   /*  we have the next leftmost lowest corner  */
    Pl[0] = xmin ;
    Pl[1] = ymin ;
    ymin = INT_MAX ;
    for( k = 1 ; k <= cornerCountS ; k++ ) {
	if( ptS[k].y == Pk[1] ) {
	    continue ;
	}
	if( ptS[k].y < ymin ) {
	    ymin = ptS[k].y ;
	}
    }  /* we have the next lowest y coordinate  */
    xmin = INT_MAX ;
    for( k = 1 ; k <= cornerCountS ; k++ ) {
	if( ptS[k].y == ymin ) {
	    if( ptS[k].x < Pk[0] || ptS[k].x > Pl[0] ) {
		continue ;
	    }
	    if( ptS[k].x < xmin ) {
		xmin = ptS[k].x ;
	    }
	}
    }  /*  we have the leftmost next lowest corner  */
    Pm[0] = xmin ;
    Pm[1] = ymin ;

    /*
     *  According to the instruction sheet I read, we can
     *  output the bounding rectangle of Pk , Pl , Pm.
     */
    resultS[1].x = Pk[0] ;
    resultS[1].y = Pk[1] ;
    resultS[2].x = Pk[0] ;
    resultS[2].y = Pm[1] ;
    resultS[3].x = Pl[0] ;
    resultS[3].y = Pm[1] ;
    resultS[4].x = Pl[0] ;
    resultS[4].y = Pk[1] ;

    /*  
     *  Now weed out those elements of R which are in A and
     *  add those elements of R which are not in A.
     *  Note that index 1 and 4 are necessarily in A, and thus
     *  have to be removed from A.
     */
    for( k = 1 ; k <= cornerCountS ; k++ ) {
	if( resultS[1].x == ptS[k].x && resultS[1].y == ptS[k].y ) {
	    ptS[k].x = ptS[ cornerCountS ].x ;
	    ptS[k].y = ptS[ cornerCountS-- ].y ;
	    break ;
	}
    }
    for( k = 1 ; k <= cornerCountS ; k++ ) {
	if( resultS[4].x == ptS[k].x && resultS[4].y == ptS[k].y ) {
	    ptS[k].x = ptS[ cornerCountS ].x ;
	    ptS[k].y = ptS[ cornerCountS-- ].y ;
	    break ;
	}
    }
    found = 0 ;
    for( k = 1 ; k <= cornerCountS ; k++ ) {
	if( resultS[2].x == ptS[k].x && resultS[2].y == ptS[k].y ) {
	    ptS[k].x = ptS[ cornerCountS ].x ;
	    ptS[k].y = ptS[ cornerCountS-- ].y ;
	    found = 1 ;
	    break ;
	}
    }
    if( found == 0 ) {
	/*
	 *  Add the thing to the list A
	 */
	ptS[ ++cornerCountS ].x = resultS[2].x ;
	ptS[ cornerCountS ].y = resultS[2].y ;
    }
    found = 0 ;
    for( k = 1 ; k <= cornerCountS ; k++ ) {
	if( resultS[3].x == ptS[k].x && resultS[3].y == ptS[k].y ) {
	    ptS[k].x = ptS[ cornerCountS ].x ;
	    ptS[k].y = ptS[ cornerCountS-- ].y ;
	    found = 1 ;
	    break ;
	}
    }
    if( found == 0 ) {
	/*
	 *  Add the thing to the list A
	 */
	ptS[ ++cornerCountS ].x = resultS[3].x ;
	ptS[ cornerCountS ].y = resultS[3].y ;
    }
    return( resultS ) ;

} /* end buster */
/* ***************************************************************** */

Ybuster_addpt( xpos, ypos )
INT xpos, ypos ;
{
    if( xpos == ptS[cornerCountS].x && ypos == ptS[cornerCountS].y ){
	/* avoid redundant points */
	return ;
    }
    /* increase the space if necessary */
    if( ++cornerCountS >= ptAllocS ){
	ptAllocS += EXPECTEDPTS ;
	ptS = YREALLOC( ptS,  ptAllocS, YBUSTBOX ) ;
    }
    ptS[cornerCountS].x = xpos ;
    ptS[cornerCountS].y = ypos ;
} /* end add_arb_pt */
/* ***************************************************************** */

Ybuster_init()
{
    /* allocate memory if needed */
    if(!(ptS)){
	ptAllocS = EXPECTEDPTS ;
	ptS = YMALLOC( ptAllocS, YBUSTBOX );
	resultS = YMALLOC( 5, YBUSTBOX ) ;
    }
    /* make sure we cannot match the 0 record in the redundancy */
    /* check above in add_arb_pt */
    ptS[0].x = INT_MIN ;
    ptS[0].y = INT_MIN ;
    cornerCountS = 0 ;
} /* end Ybuster_init */
/* ***************************************************************** */

Ybuster_free()
{
    /* free allocate memory */
    if(ptS){
	YFREE( ptS ) ;
	YFREE( resultS ) ;
	ptS = NULL ;
    }
} /* end Ybuster_free */
/* ***************************************************************** */


/*--------------------------------
  run a sanity check on data.
  returns TRUE if data might be ok
  reutrns FALSE if data not ok
  --------------------------------*/
BOOL Ybuster_verify( user_string )
char *user_string ;
{
  INT l;

  cur_stateS = S_STATE ;
  user_messageS = user_string ;

  /* verify corner count */
  
  if (cornerCountS < 4) {
    sprintf(YmsgG," %s : There must be at least 4 corners\n", user_messageS ) ;
    M(ERRMSG,"Ybuster_verify",YmsgG);
    return(FALSE) ;  
  }

  if ( cornerCountS & 1) {
    sprintf(YmsgG," %s : There must be an even # of corners\n", user_messageS ) ;
    M(ERRMSG,"Ybuster_verify",YmsgG);
    return(FALSE) ;  
  }

  /* check all points for consistency */
  
  for (l=1; l < cornerCountS;l++) {
    if( Ybuster_check_rect( ptS[l].x, ptS[l].y, ptS[l+1].x, ptS[l+1].y )){
      return( FALSE ) ;
    }
  }
  /* check the last and the first point */
  if( Ybuster_check_rect( ptS[l].x,ptS[l].y, ptS[1].x, ptS[1].y ) ){
    return( FALSE ) ;
  }
  
  
  /* if we get to this point, everything is ok */
  return(TRUE);
} /* end Ybuster_verify */
/* ***************************************************************** */

/* ***************************************************************** */
/* detect problems with clockwise rotation pattern */

BOOL Ybuster_check_rect( xx1, yy1, xx2, yy2 )
INT xx1, yy1, xx2, yy2 ;
{
    INT next_state ;           /* the next direction of the edge */
    static INT errorArrayL[6] =
    {
	/* E   -    U   -    L   -    R   -    D  -     S   */
	E_STATE, D_STATE, R_STATE, L_STATE, U_STATE, R_STATE
    } ;

    if( xx1 == xx2 && yy1 == yy2 ) {
	M(ERRMSG,"Ybuster_verify","a zero length side was found ");
	sprintf(YmsgG,"%s @(%d,%d)\n", user_messageS, xx1, yy1 );
	M(ERRMSG,NULL,YmsgG);
	return( TRUE ) ;
    } else if( xx1 != xx2 && yy1 != yy2 ) {
	M(ERRMSG,"Ybuster_verify","a non rectilinear side was found");
	sprintf(YmsgG," %s @(%d,%d)\n", 
	    user_messageS, xx1, yy1 );
	M(ERRMSG,NULL,YmsgG);
	return( TRUE ) ;
    } else if( xx2 == xx1 ){
	if( yy2 < yy1 ){
	    next_state = D_STATE ;
	} else {
	    next_state = U_STATE ;
	}
    } else if( yy2 == yy1 ){
	if( xx2 < xx1 ){
	    next_state = L_STATE ;
	} else {
	    next_state = R_STATE ;
	}
    }
    /* check to see if this is an error by looking in error array */
    /* the first state has two bad states explicitly enum. the second */
    if( next_state == errorArrayL[cur_stateS] ||
	(cur_stateS == S_STATE && next_state != U_STATE )){
	sprintf( YmsgG,
	"%s is not specified in a CW direction\n",
	    user_messageS ) ;
	M(ERRMSG,"Ybuster_check_rect", YmsgG ) ;
	return( TRUE ) ;
    } 
    /* update state */
    cur_stateS = next_state ;
    return( FALSE ) ;
} /* end Ybuster_check_rect */

Ybuster_check_rect_init( user_string )
char *user_string ;
{
    cur_stateS = S_STATE ;
    user_messageS = user_string ;
} /* end Ybuster_check_init */
