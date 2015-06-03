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
FILE:	    mergecell.c
DESCRIPTION:This file contains the utility routines to merge two
	    custom cells or pads into one cell.
CONTENTS:   
DATE:	    Aug 17, 1988 
REVISIONS:  Jan 29, 1989 - changed msg to YmsgG and added \n's.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) mergecell.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>


/* ######################  STATIC definitions ############################# */
/* ################## END STATIC definitions ############################# */

/* mergeCells takes the contents of child and parent and merges them together */
/* to form a new parent */
mergeCells( cptr, pptr ) 
CELLBOXPTR cptr ;  /* pointer to child cell box */
CELLBOXPTR pptr ;  /* pointer to parent cell box */
{
#ifdef LATER
    BOUNBOXPTR cbounptr, pbounptr ;
    PINBOXPTR cterm, pterm, saveterm ;
    INT        pl, pr, pb, pt ;
    INT        cl, cr, cb, ct ;
    INT        xcenter, ycenter ;
    /* --------------------- perform error checking ------------------- */
    if( cptr->softflag ){
	sprintf(YmsgG,"softcells can't be merged-cell:%s\n",
	    cptr->cname ) ;
	M( ERRMSG,"mergeCells",YmsgG ) ;
	return ;
    }
    if( pptr->softflag ){
	sprintf(YmsgG,"softcells can't be merged-cell:%s\n",
	    pptr->cname ) ;
	M( ERRMSG,"mergeCells",YmsgG ) ;
	return ;
    }

    /* make sure cell types match */
    if( pptr->celltype == PADCELLTYPE || pptr->celltype == PADGROUPTYPE ){
	if( cptr->celltype == SUPERCELLTYPE || cptr->celltype == CUSTOMCELLTYPE ){
	    sprintf(YmsgG,"pads and custom cells can't be merged-pad:%s cell:%s\n",
		pptr->cname, cptr->cname ) ;
	    M( ERRMSG,"mergeCells",YmsgG ) ;
	    return ;
	}
    } else if( pptr->celltype == CUSTOMCELLTYPE ){
	if( cptr->celltype == PADCELLTYPE || cptr->celltype == PADGROUPTYPE ){
	    sprintf(YmsgG,"pads and custom cells can't be merged-pad:%s cell:%s\n",
		cptr->cname, pptr->cname ) ;
	    M( ERRMSG,"mergeCells",YmsgG ) ;
	    return ;
	}
    } 

    if(!( cptr->tiles) ){
	sprintf(YmsgG,"No tile data is present for cell:%s. Aborting merge.\n",
	    cptr->cname ) ;
	M( ERRMSG,"mergeCells",YmsgG ) ;
	return ;
    }
    if(!( cptr->bounBox) ){
	sprintf(YmsgG,"No bounding box data is present for cell:%s. Aborting merge.\n",
	    cptr->cname ) ;
	M( ERRMSG,"mergeCells",YmsgG ) ;
	return ;
    }
    /* ------------------end perform error checking ------------------- */

    /* now begin merge */
    pptr->numsides += cptr->numsides ;

    /* first merge bounding boxes of two cells */
    if( pptr->bounBox ){ 
	/* parents data has already been created */
	/* avoid memory allocation */

	cbounptr=cptr->bounBox[cptr->orient] ;
	pbounptr=pptr->bounBox[pptr->orient] ;
	/* get global positions of both cells */
	pl = pptr->xcenter + pbounptr->l ;
	pr = pptr->xcenter + pbounptr->r ;
	pb = pptr->ycenter + pbounptr->t ;
	pt = pptr->ycenter + pbounptr->t ;

	cl = cptr->xcenter + cbounptr->l ;
	cr = cptr->xcenter + cbounptr->r ;
	cb = cptr->ycenter + cbounptr->t ;
	ct = cptr->ycenter + cbounptr->t ;

	/* determine new x and y center */
	pl = MIN( pl, cl ) ;
	pr = MAX( pr, cr ) ;
	pb = MIN( pb, cb ) ;
	pt = MAX( pr, ct ) ;

	pptr->xcenter = xcenter = ( pl + pr ) / 2 ;
	pptr->ycenter = ycenter = ( pb + pt ) / 2 ;

	/* now save new bounding box cell relative */
	pbounptr->l = pl - xcenter ;
	pbounptr->r = pr - xcenter ;
	pbounptr->b = pb - ycenter ;
	pbounptr->t = pt - ycenter ;

    } else {  /* parent bounding box has not been allocated */
	cbounptr=cptr->bounBox[cptr->orient] ;
	pptr->bounBox = (BOUNBOXPTR *) 
	    Ysafe_malloc( sizeof( BOUNBOXPTR ) ) ;
	pbounptr = pptr->bounBox[0] = (BOUNBOXPTR) 
	    Ysafe_malloc( sizeof( BOUNBOX ) ) ;
	pbounptr->l = cbounptr->l ;
	pbounptr->r = cbounptr->r ;
	pbounptr->b = cbounptr->b ;
	pbounptr->t = cbounptr->t ;
	pptr->xcenter = cptr->xcenter ;
	pptr->ycenter = cptr->ycenter ;
    }
    /* end bounding box merge */

    /* now merge pins */
    for( cterm=cptr->pinptr;cterm;cterm=cterm->nextterm ){

	if( saveterm = pptr->pinptr ) {
	    pterm = pptr->pinptr = 
		(TERMBOXPTR) Ysafe_malloc( sizeof( TERMBOX ) ) ;
	    pterm->nextterm = saveterm ;
	} else {
	    pterm = pptr->termptr =  
		(TERMBOXPTR) Ysafe_malloc( sizeof( TERMBOX ) ) ;
	    pterm->nextterm = NULL ;
	}
	pterm->terminal = cterm->terminal ;
	pterm->xpos = pterm->orig_xpos = cterm->xpos + cptr->xcenter - xcenter ;
	pterm->ypos = pterm->orig_ypos = cterm->ypos + cptr->ycenter - ycenter;
    } /* end terminal loop */
    pptr->numterms += cptr->numterms ;

    /* merge the tiles by making copy of the new tiles */
    /* this won't work I need to find the minimum containing figure */
    /* of the set of tile. Note I need to add xcenter ycenter to get */
    /* global position, find all the corners, use buster to break into */
    /* tiles and then call weight routines */
    for( ctileptr=cptr->config[0];ctileptr;ctileptr=ctileptr->next ){ 
	if( savetile = pptr->config[0] ){
	    pptr->config[0] = tile = 
		(TILEBOXPTR) Ysafe_malloc( sizeof( TILEBOX ) ) ;
	    tile->next = savetile ;
	} else { /* first addition to supercell */
	    pptr->config[0] = tile = 
		(TILEBOXPTR) Ysafe_malloc( sizeof( TILEBOX ) ) ;
	    tile->next = NULL ;
	}
	/* copy contents of tile */
	tile->lweight     = ctileptr->lweight ;
	tile->rweight     = ctileptr->rweight ;
	tile->bweight     = ctileptr->bweight ;
	tile->tweight     = ctileptr->tweight ;
	tile->lborder     = ctileptr->lborder ;
	tile->rborder     = ctileptr->rborder ;
	tile->bborder     = ctileptr->bborder ;
	tile->tborder     = ctileptr->tborder ;
	tile->left        = ctileptr->left ;
	tile->right       = ctileptr->right ;
	tile->bottom      = ctileptr->bottom ;
	tile->top         = ctileptr->top ;
	tile->orig_left   = ctileptr->orig_left ;
	tile->orig_right  = ctileptr->orig_right ;
	tile->orig_bottom = ctileptr->orig_bottom ;
	tile->orig_top    = ctileptr->orig_top ;
    }
#endif


} /* end mergecells */
/* ***************************************************************** */
