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
FILE:	    usite2.c                                       
DESCRIPTION:two cell flip and/or orientation change
CONTENTS:   BOOL usite2( )
DATE:	    Jan 30, 1988 
REVISIONS:  Feb  5, 1988 - changed old_apos, new_apos, old_bpos,
	        new_bpos to global variables. Old method remains
		commented out.
	    Aug 25, 1988 - calls to ufixpin and usoftpin changed.
	    Oct 21, 1988 - changed to sqrt for overlap penalty.
	    Nov 25, 1988 - added timing driven code.
	    May  3, 1989 - added unet2 for two cell swaps.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) usite2.c version 3.4 9/10/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

static INT acellS, bcellS, mod_cellS ;
static CELLBOXPTR acellptrS ,    bcellptrS ;

BOOL usite2( /* old_apos, new_apos, old_bpos, new_bpos */ )
/* MOVEBOXPTR  *old_apos, *new_apos, *old_bpos, *new_bpos ; */
{

PINBOXPTR anewtermptr , bnewtermptr ;
MOVEBOXPTR pos ;         /* temp pointer for easier access */
BINBOXPTR  modptr ;      

INT cost , newpenalty, newbinpenal, newtimepenalty, newtimepenal ;
INT oldBinX, oldBinY, limit, *oldCellList ;
INT modBinX, modBinY, *modCellList ;
INT i ;
INT wire_chg ;
DOUBLE delta_wire ;

/* ----------------------------------------------------------------- 
   global information is stored in element zero of position arrays 
   new position records have been initialized in uloop(). 
*/
acellS = new_apos0G->cell ;
acellptrS = cellarrayG[acellS] ;
anewtermptr  = acellptrS->pinptr ;

bcellS = new_bpos0G->cell ;
bcellptrS = cellarrayG[bcellS] ;
bnewtermptr = bcellptrS->pinptr ;

clear_net_set() ; /* reset set to mark nets that have changed position */

/* remove overlap so that pairwise exchange has better chance at low T*/
mod_cellS = 0 ;
/* mod_cell_positions() ;  */

/* save state of penalty */
newbinpenal = binpenalG ;

/* adjust overlap penalty */
newbinpenal += overlap2( /* old_apos, new_apos, old_bpos, new_bpos */ ) ;

/* scale newpenalty for feedback circuit */
newpenalty = (INT) ( lapFactorG * sqrt( (DOUBLE) newbinpenal ) )  ;

upin_test(  anewtermptr, new_apos0G ) ;
upin_test(  bnewtermptr, new_bpos0G ) ;

cost = funccostG ;

cost += unet2(  anewtermptr, bnewtermptr ) ;

wire_chg = cost - funccostG ;

newtimepenal = timingpenalG ;
newtimepenal += calc_incr_time2( acellS, bcellS ) ;

ASSERT( newtimepenal == dcalc_full_penalty(),"usite2","Timing woes\n" ) ;

/* scale new timing penalty */
newtimepenalty = (INT) ( timeFactorG * (DOUBLE) newtimepenal ) ;

if( acceptt( penaltyG - newpenalty - wire_chg + 
    timingcostG - newtimepenalty )){

    upin_accept( anewtermptr ) ;
    upin_accept( bnewtermptr ) ;

    update_overlap2( /* old_aposG, old_bposG */ ) ;

    update_time2( acellS, bcellS ) ;

    /* update new position and orientation of a cell */
    acellptrS->xcenter = new_apos0G->xcenter ;
    acellptrS->ycenter = new_apos0G->ycenter ;
    acellptrS->orient  = new_apos0G->orient  ;

    bcellptrS->xcenter = new_bpos0G->xcenter ;
    bcellptrS->ycenter = new_bpos0G->ycenter ;
    bcellptrS->orient  = new_bpos0G->orient  ;

    /* ************ BEGIN UPDATE OF BIN CELL LIST ************** */
    if( !(mod_cellS) ){
	/* a and b are swapped - assert old_apos0 has b new position */
	/* and newcellList has new a position calculated in uloop.c */
	oldBinX = SETBINX(old_apos0G->xcenter) ;
	oldBinY = SETBINY(old_apos0G->ycenter) ;
	oldCellList = binptrG[oldBinX][oldBinY]->cells ;

	/* do nothing if cell a remains in same bin */
	if( oldCellList != newCellListG ){
	    
	    /* swap a for b in oldcellList */
	    /* find a 's position in array */
	    limit = oldCellList[0] ;
	    for( i=1;i<=limit;i++ ){
		if( oldCellList[i] == acellS ){
		    oldCellList[i] = bcellS ;
		    break ;
		}
	    } /* assert i now has correct value of b */
	    ASSERT( oldCellList[i] == bcellS,"usite2",
		"Problem in oldbin cell lists\n" ) ;

	    /* swap b for a in newcellList */
	    /* find a 's position in array */
	    limit = newCellListG[0] ;
	    for( i=1;i<=limit;i++ ){
		if( newCellListG[i] == bcellS ){
		    newCellListG[i] = acellS ;
		    break ;
		}
	    } /* assert i now has correct value of a */
	    ASSERT( newCellListG[i] == acellS,"usite2",
		"Problem in newbin cell lists\n" ) ;
	}

    } else if( mod_cellS == acellS ){
	/* a's bin is modified by the moving cells apart */
	oldBinX = SETBINX(old_apos0G->xcenter) ;
	oldBinY = SETBINY(old_apos0G->ycenter) ;
	oldCellList = binptrG[oldBinX][oldBinY]->cells ;
	modBinX = SETBINX(new_apos0G->xcenter) ;
	modBinY = SETBINY(new_apos0G->ycenter) ;
	modptr = binptrG[modBinX][modBinY] ;
	modCellList = modptr->cells ;

	/* add b to oldCellList - find a 's position in array */
	limit = oldCellList[0] ;
	for( i=1;i<=limit;i++ ){
	    if( oldCellList[i] == acellS ){
		oldCellList[i] = bcellS ;
		break ;
	    }
	}
	/* assert i now has correct value of b */
	ASSERT( oldCellList[i] == bcellS, "usite2",
	    "Problem in oldbin cell lists\n");

	/* delete b in newCellList */
	limit = newCellListG[0] ;
	for( i=1;i<=limit;i++ ){
	    if( newCellListG[i] == bcellS ){
		break ;
	    }
	}
	ASSERT( newCellListG[i] == bcellS, "usite2",
	    "Problem in newbin cell lists\n" ) ;

	if( i != limit ){
	    newCellListG[i] = newCellListG[ limit ] ;
	}
	newCellListG[0]-- ;

	/* now add a to modified bin */
	limit = ++modCellList[0] ;
	if( limit >= modptr->space ){
	    modptr->space += EXPCELLPERBIN ;
	    modCellList = (INT *) Ysafe_realloc( modCellList,
		modptr->space * sizeof(INT) ) ;
	}
	modCellList[limit] = acellS ;

    } else if( mod_cellS == bcellS ){
	/* b's bin is modified by the moving cells apart */
	oldBinX = SETBINX(old_apos0G->xcenter) ;
	oldBinY = SETBINY(old_apos0G->ycenter) ;
	oldCellList = binptrG[oldBinX][oldBinY]->cells ;
	modBinX = SETBINX(new_bpos0G->xcenter) ;
	modBinY = SETBINY(new_bpos0G->ycenter) ;
	modptr = binptrG[modBinX][modBinY] ;
	modCellList = modptr->cells ;

	/* add a to newCellList - find b 's position in array */
	/* this delete's b's old position */
	limit = newCellListG[0] ;
	for( i=1;i<=limit;i++ ){
	    if( newCellListG[i] == bcellS ){
		newCellListG[i] = acellS ;
		break ;
	    }
	}
	/* assert i now has correct value of a */
	ASSERT( newCellListG[i] == acellS, "usite2",
	    "Problem in newbin cell lists\n" ) ;

	/* delete a in oldCellList */
	limit = oldCellList[0] ;
	for( i=1;i<=limit;i++ ){
	    if( oldCellList[i] == acellS ){
		break ;
	    }
	}
	ASSERT( oldCellList[i] == acellS, "usite2",
	    "Problem in oldbin cell lists\n" ) ;

	if( i != limit ){
	    oldCellList[i] = oldCellList[ limit ] ;
	}
	oldCellList[0]-- ;

	/* now add b to modified bin */
	limit = ++modCellList[0] ;
	if( limit >= modptr->space ){
	    modptr->space += EXPCELLPERBIN ;
	    modCellList = (INT *) Ysafe_realloc( modCellList,
		modptr->space * sizeof(INT) ) ;
	}
	modCellList[limit] = bcellS ;

    } /* ********** END UPDATE OF BIN CELL LIST ************** */

    /* check that everything is ok */
    ASSERT( checkbinList(), "usite2",
	"We have a problem at checkbinList()\n") ;

    funccostG = cost ;
    penaltyG = newpenalty ;
    binpenalG = newbinpenal ;
    timingcostG = newtimepenalty ;
    timingpenalG = newtimepenal ;

    return (ACCEPT) ;
} else {
    return (REJECT) ;
}
}
