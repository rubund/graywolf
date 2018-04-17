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
FILE:	    buildimp.c                                       
DESCRIPTION:build implicit feedthrus.
CONTENTS:   buildimp( )
	    link_imptr( )
	    decide_boundary( )
	    fixwolf( )
	    addfeed( row , pos , feednum ) 
		INT row , pos , feednum ;
DATE:	    Mar 27, 1989 
REVISIONS:  Aug 30, 1989 - removed lastimp error.
	    Tue Jan 15 20:31:33 PST 1991 - removed constant number
		of implicit feeds in a cell and instead made it
		dynamic.
	    Wed Jan 16 14:26:44 PST 1991 - removed re_buildimp
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) buildimp.c (Yale) version 4.9 11/7/91" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "feeds.h"

static INT max_impfeed_in_cellS ; /* max no. of feeds in a cell */
INT comparetxpos() ;
extern BOOL ignore_feedsG ;	/* Treat feedthroughs as width 0 */


buildimp( )
{

IPBOXPTR *impinptr , imptr ;
CBOXPTR cellptr ;
INT i , j , *Aray , row , count , total ;
INT row_rite , longest_row , longest_blk_length ;
INT *feasible_feednum ;

/* first calculate the maximum implicit feeds in a cell */
max_impfeed_in_cellS = 0 ;
for( i = 1; i <= numcellsG; i++ ){
    cellptr = carrayG[ i ] ;
    count = 0 ;
    for( imptr = cellptr->imptr ; imptr ; imptr = imptr->next ) {
	count++ ;
    }
    if( count > max_impfeed_in_cellS ){
	max_impfeed_in_cellS = count ;
    }
}
FeedInRowG = (INT *)Ysafe_calloc( numRowsG+1, sizeof(INT)) ;
impinptr = (IPBOXPTR *)
	Ysafe_malloc( (max_impfeed_in_cellS+1)* sizeof(IPBOXPTR) ) ;
impFeedsG = (IPBOXPTR *)Ysafe_malloc( ( numRowsG+1 ) * sizeof( IPBOXPTR ));
feasible_feednum = (INT *)Ysafe_malloc( ( numRowsG+1 ) * sizeof( INT ));
for( row = 1 ; row <= numRowsG ; row++ ) {
    total = 0 ;
    impFeedsG[row] = (IPBOXPTR)Ysafe_calloc( 1, sizeof(IPBOX)) ;
    Aray = pairArrayG[row] ;
    for( i = 1 ; i <= Aray[0] ; i++ ) {
	cellptr = carrayG[ Aray[i] ] ;
	count = 0 ;
	for( imptr = cellptr->imptr ; imptr ; imptr = imptr->next ) {
	    impinptr[ ++count ] = imptr ;
	}
	if( count <= 1 ) {
	    continue ;
	}
	Yquicksort( (char *)(impinptr+1) , count , sizeof(PINBOXPTR),
	    comparetxpos ) ;
	if( cellptr->corient <= 1 ) {
	    cellptr->imptr = impinptr[1] ;
	    for( j = 1 ; j <= count - 1 ; j++ ) {
		impinptr[j]->next = impinptr[j+1] ;
		impinptr[j+1]->prev = impinptr[j] ;
	    }
	    impinptr[count]->next = NULL ;
	} else {
	    cellptr->imptr = impinptr[count] ;
	    for( j = count ; j >= 2 ; j-- ) {
		impinptr[j]->next = impinptr[j-1] ;
		impinptr[j-1]->prev = impinptr[j] ;
	    }
	    impinptr[1]->next = NULL ;
	}
	total += count ;
    }
    FeedInRowG[row] = total ;
    if( impFeedsG[row]->next != NULL ) {
	impFeedsG[row]->next->prev = NULL ;
    }
}
Ysafe_free( impinptr ) ;
blk_most_riteG = 0 ;
longest_row = 1 ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
    row_rite = cellptr->cxcenter + cellptr->tileptr->right ;
    if( row_rite > blk_most_riteG ) {
	blk_most_riteG = row_rite ;
	longest_row = row ;
    }
}
longest_blk_length = blk_most_riteG - blkleftG ;
printf("\n block left edge is at %d\n", blkleftG ) ;
printf(" the longest block length is %d\n" , longest_blk_length ) ;

if( longest_blk_length < barrayG[ longest_row ]->desire ) {
    for( row = 1 ; row <= numRowsG ; row++ ) {
	cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
	row_rite = cellptr->cxcenter + cellptr->tileptr->right ;
	feasible_feednum[row] =
	    ( barrayG[row]->desire - row_rite ) / fdWidthG ;
    }
} else {
    for( row = 1 ; row <= numRowsG ; row++ ) {
	cellptr = carrayG[ pairArrayG[row][ pairArrayG[row][0] ] ] ;
	row_rite = cellptr->cxcenter + cellptr->tileptr->right ;
	if( fdWidthG > 0 ) {
	    feasible_feednum[row] =
			    ( blk_most_riteG - row_rite ) / fdWidthG ;
	} else {
	    feasible_feednum[row] = ( blk_most_riteG - row_rite ) ;
	}
    }
}
/*
for( row = 1 ; row <= numRowsG ; row++ ) {
    FeedInRowG[row] += feasible_feednum[row] ;
}
*/
Ysafe_free( feasible_feednum ) ;
}


link_imptr( )
{
INT i , row , *Aray ;
CBOXPTR cellptr ;
IPBOXPTR imptr , lastimp ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    Aray = pairArrayG[row] ;
    lastimp = impFeedsG[row] ;
    for( i = 1 ; i <= Aray[0] ; i++ ) {
	cellptr = carrayG[ Aray[i] ] ;
	if( cellptr->imptr == NULL ) {
	    continue ;
	}
	imptr = cellptr->imptr ;
	lastimp->next = imptr ;
	imptr->prev = lastimp ;
	for( lastimp = imptr ; lastimp->next ; lastimp = lastimp->next);
    }
}
}


decide_boundary( )
{

INT row , fcx , fcl , lcx , lcr , lcl ;
INT fcell , lcell , cell ;
INT xstart , xstop ;

numChansG = numRowsG + 1 ;
xstart = INT_MAX ;
xstop  = INT_MIN ;
for( row = 1 ; row <= numRowsG ; row++ ) {
    fcell = pairArrayG[row][1] ;
    lcell = pairArrayG[row][ pairArrayG[row][0] ] ;
    fcx = carrayG[fcell]->cxcenter ;
    fcl = carrayG[fcell]->tileptr->left ;
    lcx = carrayG[lcell]->cxcenter ;
    lcr = carrayG[lcell]->tileptr->right ;
    if( lcx + lcr > xstop ) {
	xstop = lcx + lcr ;
    }
    if( fcx + fcl < xstart ) {
	xstart = fcx + fcl ;
    }
}
for( cell = numcellsG + 1  ; cell <= lastpadG ; cell++ ) {
    lcx = carrayG[cell]->cxcenter ;
    lcl = carrayG[cell]->tileptr->left ;
    lcr = carrayG[cell]->tileptr->right ;
    if( lcx + lcr > xstop ) {
	xstop = lcx + lcr ;
    }
    if( lcx + lcl < xstart ) {
	xstart = lcx + lcl ;
    }
}
gxstartG = xstart ;
gxstopG  = xstop  ;
}


fixwolf( )
{

INT j , last_j ;

maxtermG = implicit_feed_countG + fdthrusG + TotRegPinsG ;
last_j = maxtermG + 2 * numRowsG + 2 ;

carrayG = (CBOXPTR *)Ysafe_realloc( carrayG, (1 + numcellsG + numtermsG + 
			    2 * numRowsG + fdthrusG) * sizeof(CBOXPTR) );

tearrayG = ( PINBOXPTR * ) Ysafe_realloc( tearrayG , ( last_j + 1 )
					    * sizeof(PINBOXPTR) );
for( j = TotRegPinsG + 1 ; j <= last_j ; j++ ) {
    tearrayG[j] = PINNULL ;
}
return ;
}


addfeed( row , pos , feednum ) 
INT row , pos , feednum ;
{
CBOXPTR cellptr ;
TIBOXPTR tileptr ;
IPBOXPTR imptr ;

int locFdWidth = fdWidthG;

/* This seems to work to remove explicit feedthroughs. . . */
if (ignore_feedsG) locFdWidth = 0;

cellptr = ( CBOXPTR )Ysafe_malloc( sizeof( CBOX ) ) ;
carrayG[ numcellsG + numtermsG + feednum ] = cellptr ;
cellptr->cname = (char *) Ysafe_malloc( 16 * sizeof( char ) ) ;
#ifdef FEED_INSTANCES
    sprintf( cellptr->cname , "twfeed%d", feednum ) ;
#else
    sprintf( cellptr->cname , "twfeed" ) ;
#endif
cellptr->cclass   = 0 ;
if( barrayG[row]->borient == 2 ) {
    cellptr->corient = 1 ;
} else {
    cellptr->corient  = 0 ;
}
/*
if( gate_array_special ) {
    ptr->orflag  = 1 ;
} else {
    ptr->orflag  = 0 ;
}
*/
cellptr->orflag  = 0 ;
cellptr->cxcenter = pos ;
cellptr->cycenter = barrayG[row]->bycenter ;
cellptr->cheight  = barrayG[row]->bheight  ;
cellptr->clength  = locFdWidth ;
cellptr->cblock   = row ;
cellptr->numterms = 1   ;
cellptr->pins    = NULL ;
cellptr->padptr  = NULL ;
cellptr->paths   = NULL ;
tileptr = cellptr->tileptr = ( TIBOXPTR )Ysafe_malloc( sizeof( TIBOX ) );
tileptr->left   = -locFdWidth / 2 ;
tileptr->right  = locFdWidth + tileptr->left ;
tileptr->bottom = -( barrayG[row]->bheight / 2 ) ;
tileptr->top    = barrayG[row]->bheight + tileptr->bottom ;
cellptr->imptr  = imptr = ( IPBOXPTR )Ysafe_calloc( 1,sizeof(IPBOX));
imptr->txpos    = 0 ;
imptr->xpos     = pos ;
imptr->cell     = numcellsG + numtermsG + feednum ;
imptr->terminal = TotRegPinsG + implicit_feed_countG + feednum ;
imptr->pinname    = (char *)Ysafe_malloc( 2 ) ;
imptr->eqpinname  = (char *)Ysafe_malloc( 2 ) ;
sprintf( imptr->pinname, "%s", "1" ) ;
sprintf( imptr->eqpinname, "%s", "2" ) ;

return ;
}
