/*
 *   Copyright (C) 1988-1990 Yale University
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
FILE:	    window.c                                       
DESCRIPTION:new window limiter routines due to Jimmy Lamm
CONTENTS:   DOUBLE eval_ratio(percentWindow)
		DOUBLE *percentWindow ;
	    init_control()
	    pick_position(INT *, INT *, int, int)
	    update_control(a)
	    fix_window()
	    update_window_size( iteration )
		DOUBLE iteration ;
DATE:	    Feb 29, 1988 
REVISIONS:  Apr 23, 1988 - added fix_window for low temp anneal 
	    Oct 21, 1988 - changed steps per cell and add graph func.
	    Oct 26, 1988 - add pick_neighborhood and fixed bug in
		pick_position.
	    Nov 20, 1988 - fixed chip aspect ratio.
	    Jan 15, 1989 - added update_window_size, and new eval ratio
		for curve-fit controller.
	    Jan 29, 1989 - changed to YmsgG.
	    Mar 02, 1989 - move temp defintions to temp.h
	    Apr 09, 1989 - fixed bug in pick_position and 
		pick_neighborhood so that cells can't jump outside region.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) window.c version 3.6 11/26/90" ;
#endif

#include <custom.h>
#include <temp.h>
#include <yalecad/debug.h>

#define AC0 0.90		/*** 0.75 ***/
#define AC1 0.44		/*** 0.44 ***/
#define AC2 0.06		/*** 0.08 ***/

#define PT1 0.15		/*** 0.15 ***/
#define PT2 0.52		/*** 0.52 ***/
#define PT3 1.00		/*** 1.00 ***/

#define LAC1 (log(AC1))
#define LAC2 (log(AC2))

#define STEPSPERCEL 0.01           /* was 0.01 */

#define TABLIMIT 4096		/*** simple table lookup for log. ***/
#define TABSHIFT 19
#define TABMASK 0xfff
#define TABOFFSET 0x40000
#define RANDFACT (1.0 / INT_MAX)
#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

static DOUBLE xadjustmentS,xalS,min_xalphaS,max_xalphaS;/** x control **/
static DOUBLE yadjustmentS,yalS,min_yalphaS,max_yalphaS;/** y control **/
static DOUBLE total_stepS;
static DOUBLE log_tabS[TABLIMIT];
static DOUBLE tauXS, tauYS ; /* exp. decay time constants for window */

DOUBLE eval_ratio( iteration )
INT iteration ;
{
    if( iteration >= TURNOFFT ){
	return( (DOUBLE) 1.0 ) ;
    } else if( iteration < 0 ){
	return( (DOUBLE) 0.0 ) ;
    } else {
	return( (DOUBLE) iteration / TURNOFFT ) ;
    }
}

/* ***************************************************************** 
   init_control - initialize range limiter.
*/
init_control(first)
BOOL first ;
{
    INT i;
    DOUBLE area ;

#define FRACTION  0.10

    /*** initialize move generation parameters ***/
    /* minxspan = mean_width + 3 sigma variation  */
    area = mean_cellAreaG + 3 * dev_cellAreaG ;

    /*** average min. window size ***/
    min_xalphaS = 0.5 * sqrt( area / chipaspectG ) ;
    min_yalphaS = 0.5 * sqrt( chipaspectG * area ) ;

    min_xalphaS = MIN( min_xalphaS, FRACTION * (DOUBLE) bdxlengthG ) ;
    min_yalphaS = MIN( min_yalphaS, FRACTION * (DOUBLE) bdylengthG ) ;
    OUT2( "min_xalpha:%4.2lf\n", min_xalphaS ) ;
    OUT2( "min_yalpha:%4.2lf\n", min_yalphaS ) ;

    if (init_accG >= 0.44) {
	max_xalphaS = bdxlengthG;	/*** average max. window size ***/
	max_yalphaS = bdylengthG;	/*** average max. window size ***/
    } else {
	max_xalphaS = min_xalphaS;	/*** no adjustments ***/
	max_yalphaS = min_yalphaS;
    }

    total_stepS = STEPSPERCEL * numcellsG;
    xadjustmentS = (max_xalphaS - min_xalphaS) / total_stepS;
    yadjustmentS = (max_yalphaS - min_yalphaS) / total_stepS;

    xalS = max_xalphaS;
    yalS = max_yalphaS;

    if (first) {
	xalS = max_xalphaS;
	yalS = max_yalphaS;
	/* determine tauXS */
	tauXS = - log( (min_xalphaS/max_xalphaS)) / (MEDTEMP - HIGHTEMP) ;
	tauYS = - log( (min_yalphaS/max_yalphaS)) / (MEDTEMP - HIGHTEMP) ;
    }

    /*** prepare lookup table ***/
    for (i=0; i<TABLIMIT; i++)
	log_tabS[i] = log(((i << TABSHIFT) + TABOFFSET) * RANDFACT);
}

/* ***************************************************************** 
   pick_positon - pick place to move within range limiter.
*/
pick_position(x,y,ox,oy)
INT *x,*y,ox,oy;
{
    register INT i,m,n;

    /* get exponentially distributed random number around old x */
    for (i=0; i<2; i++) {
	m = RAND;
	n = -xalS * log_tabS[(m >> TABSHIFT) & TABMASK];
	if (m & 0x10000){
	    n = -n;
	}
	n += ox;
	/* check for inside core */
	if (n >= blocklG && n <= blockrG){
	    /* hate to use a goto here but it saves another test */
	    goto DONEX ;
	}
    }
    /* by default -we have failed. Use boundary */
    if (n < blocklG) {
	if (ox > blockrG){
	    ox = blockrG;
	} else if (ox < blocklG){
	    ox = blocklG;
	}
	n = PICK_INT(blocklG,ox);
    } else if (n > blockrG) {
	if (ox < blocklG){
	    ox = blocklG;
	} else if (ox > blockrG){
	    ox = blockrG;
	}
	n = PICK_INT(ox,blockrG);
    }
DONEX:  *x = n;

    /* get exponentially distributed random number around old y */
    for (i=0; i<2; i++) {
	m = RAND;
	n = -yalS * log_tabS[(m >> TABSHIFT) & TABMASK];
	if (m & 0x10000){
	    n = -n;
	}
	n += oy;
	/* check for inside core */
	if (n >= blockbG && n <= blocktG){
	    *y = n;
	    return;
	}
    }
    /* if fail use boundary */
    if (n < blockbG) {
	if (oy > blocktG){
	    oy = blocktG;
	} else if (oy < blockbG){
	    oy = blockbG;
	}
	n = PICK_INT(blockbG,oy);
    } else if (n > blocktG) {
	if (oy < blockbG){
	    oy = blockbG;
	} else if (oy > blocktG){
	    oy = blocktG;
	}
	n = PICK_INT(oy,blocktG);
    }
    *y = n;
}

/* ***************************************************************** 
   pick_neighborhood - pick place to move within neighborhood while
   still using range limiter.
*/
pick_neighborhood(x,y,ox,oy,fixptr)
INT *x,*y,ox,oy;
FIXEDBOXPTR fixptr ;
{
    register INT i,m,n;
    INT xjump, yjump ;

#define DIV_2   >> 1 

    /* check if x range limiter is smaller than neighborhood */
    if( xalS > fixptr->xspan ){
	ox = fixptr->xcenter ;  /* jump from center of neighborhood */
	xjump = fixptr->xspan DIV_2 ; /* average jump half the xspan */
    } else {
	/* we must use range limiter and jump from old coordinates */
	xjump = xalS ;
    }

    /* get exponentially distributed random number around old x */
    for (i=0; i<2; i++) {
	m = RAND;
	n = -xjump * log_tabS[(m >> TABSHIFT) & TABMASK];
	if (m & 0x10000){
	    n = -n;
	}
	n += ox;
	/* check for inside neighborhood */
	if (n >= fixptr->x1 && n <= fixptr->x2 ){
	    /* hate to use a goto here but it saves another test */
	    goto DONEX ;
	}
    }
    /* by default - we have failed. Use neighborhood boundary */
    if (n < fixptr->x1 ) {
	if (ox > fixptr->x2 ){
	    ox = fixptr->x2 ;
	} else if (ox < fixptr->x1 ){
	    ox = fixptr->x1 ;
	}
	n = PICK_INT(fixptr->x1 ,ox);
    } else if (n > fixptr->x2 ) {
	if (ox < fixptr->x1 ){
	    ox = fixptr->x1 ;
	} else if (ox > fixptr->x2 ){
	    ox = fixptr->x2 ;
	}
	n = PICK_INT(ox,fixptr->x2 );
    }
    DONEX:  *x = n;

    /* check if y range limiter is smaller than neighborhood */
    if( yalS > fixptr->yspan ){
	oy = fixptr->ycenter ;  /* jump from center of neighborhood */
	yjump = fixptr->yspan DIV_2 ; /* average jump half the yspan */
    } else {
	/* we must use range limiter and jump from old coordinates */
	yjump = yalS ;
    }
    /* get exponentially distributed random number around old y */
    for (i=0; i<2; i++) {
	m = RAND;
	n = -yjump * log_tabS[(m >> TABSHIFT) & TABMASK];
	if (m & 0x10000){
	    n = -n;
	}
	n += oy;
	/* check for inside core */
	if (n >= fixptr->y1 && n <= fixptr->y2){
	    *y = n;
	    return;
	}
    }
    /* if fail use neighborhood boundary */
    if (n < fixptr->y1) {
	if (oy > fixptr->y2){
	    oy = fixptr->y2;
	} else if (oy < fixptr->y1){
	    oy = fixptr->y1;
	}
	n = PICK_INT(fixptr->y1,oy);
    } else if (n > fixptr->y2) {
	if (oy < fixptr->y1){
	    oy = fixptr->y1;
	} else if (oy > fixptr->y2){
	    oy = fixptr->y2;
	}
	n = PICK_INT(oy,fixptr->y2);
    }
    *y = n;
} /* end pick_neighborhood */

update_window_size( iteration )
DOUBLE iteration ;
{
    if( iteration <= HIGHTEMP ){
	xalS = max_xalphaS ;
	yalS = max_yalphaS ;
    } else if( iteration <= MEDTEMP ){
	/* exponential decay xal and yal */
	/* -------------------------------------------------------- 
	    xal = max_xalpha * exp( - tauXS * ( iteration - HIGHTEMP ))
	    yal = max_yalpha * exp( - tauYS * ( iteration - HIGHTEMP ))
	   -------------------------------------------------------- */
	xalS = yalS = iteration - HIGHTEMP ;
	xalS *= - tauXS ;
	xalS = max_xalphaS * exp( xalS ) ;

	yalS *= - tauYS ;
	yalS = max_yalphaS * exp( yalS ) ;

    } else {  /* low temp */
	xalS = min_xalphaS ;
	yalS = min_yalphaS ;
    }
   /*
	OUT5(" xal = %4.2le  yal = %4.2le stepsG = %4.2le T=%4.2le\n",
	   xalS, yalS, stepsG, TG );
    */
   
}

fix_window()
{
    /*** set window to minimum for low temp anneal ***/
    xalS = min_xalphaS;
    yalS = min_yalphaS;
}

/* ***************************************************************** 
   save_window - save window parameters for restart
*/
/* static declaration for restoring state for low temp anneal */
static DOUBLE ws_xalS;
static DOUBLE ws_yalS;
static DOUBLE ws_ratioS ;

save_window( fp )
FILE *fp ;
{
    if( fp ){  /* if a file pointer is given write to file */
	fprintf(fp,"# window parameters:\n") ;
	fprintf(fp,"%f %f %f\n",xalS,yalS,ratioG );
    } else { /* otherwise save in memory */
	ws_xalS = xalS ;
	ws_yalS = yalS ;
	ws_ratioS = ratioG ;
    }
}

/* ***************************************************************** 
   read_window - read window parameters for restart
*/
INT read_window( fp )
FILE *fp ;
{
    INT errors = 0 ;
    if( fp ){  /* if file pointer given restore from file */
	fscanf(fp,"%[ #:a-zA-Z]\n",YmsgG ); /* throw away comment */
	fscanf(fp,"%lf %lf %lf\n",&xalS,&yalS,&ratioG);
	/* try to detect errors */
	if( xalS < 0.0 ){
	    M(ERRMSG,"read_window","Restart file: xal negative\n") ;    
	    errors++ ;
	}
	if( yalS < 0.0 ){
	    M(ERRMSG,"read_window","Restart file: yal negative\n") ;    
	    errors++ ;
	}
	if( ratioG < 0.0 ){
	    M(ERRMSG,"read_window","Restart file: ratio negative\n") ;   
	    errors++ ;
	}
	if( ratioG > 1.0 ){
	    M(ERRMSG,"read_window","Restart file: ratio > 1\n") ;    
	    errors++ ;
	}
    } else {  /* restore from memory */
	xalS = ws_xalS ;
	yalS = ws_yalS ;
	ratioG = ws_ratioS ;
    }
    return(errors) ;
} /* end read_window */
