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
FILE:	    savewolf.c                                       
DESCRIPTION:restart mechanism routines
CONTENTS:   savewolf()
DATE:	    Jan 29, 1988 
REVISIONS:  Dec  3, 1988 - added forced save flag.
	    Jan 25, 1989 - changed placement of aspect ratio field
		so that softcell restarts will work properly. Also
		changed msg to YmsgG.
	    Dec 17, 1989 - now save instances correctly.
	    Feb  7, 1989 - fixed error in softcell sites.
	    Feb  8, 1989 - force recalculation of boundaries when
		you read in new data.
	    Apr 23, 1990 - save pin positions in restart file.
	    May 15, 1990 - moved cpu time to library.
	    Oct 14, 1990 - fixed restart in annealing.
	    Mon Feb  4 02:19:36 EST 1991 - made HPI more robust.
	    Thu Mar  7 01:50:35 EST 1991 - now save wireest params.
	    Sat Apr 27 01:10:45 EDT 1991 - fixed problem with aspect
		ratio calculation.
	    Wed May  1 19:23:32 EDT 1991 - fixed pin placement and
		aspect ratio initialization in restart.
	    Sun May  5 14:31:12 EDT 1991 - replaced config3 with
		reorigin.
	    Mon Sep 16 22:23:04 EDT 1991 - fixed for R6000.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) savewolf.c version 3.12 9/16/91" ;
#endif

#include <custom.h>
#include <temp.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>

#define MAXTIMEBEFORESAVE 600.0   /* seconds before new save 10min. */

savewolf( forceSave )
BOOL forceSave ; /* if true save parameters regardless of time */
{

INT  m ;
FILE *fp ;
char filename[LRECL] ;
char file1[LRECL] ;
char file2[LRECL] ;
INT cell , terminal ;
INT count ;
CELLBOXPTR cellptr ;
PINBOXPTR  *sarray ;
PINBOXPTR  pin ;

DOUBLE Ycpu_time();
DOUBLE current_time;
static DOUBLE last_time = 0.0;

#ifndef VMS
    /* make sure another checkpoint is worthwhile !! */
    current_time = Ycpu_time();
    if( !(forceSave) ){ /* avoid return when forced to save file */
	if( (current_time - last_time) < MAXTIMEBEFORESAVE ) {
	    return;
	}
    }
    last_time = current_time;
    sprintf( filename , "%s.mtmp" , cktNameG ) ;
    fp = TWOPEN( filename , "w", ABORT ) ;
#else
    sprintf( filename , "%s.msav" , cktNameG ) ;
    fp = TWOPEN( filename , "w", ABORT ) ;
    sprintf( filename , "%s.sav;-2" , cktNameG ) ;
    delete( filename ) ;
#endif
    if( scale_dataG ){
	m = scale_dataG ;
    } else {
	m = 1 ;
    }

    save_uloop( fp ) ;
    save_window( fp ) ;
    save_wireest( fp ) ;
    fprintf(fp,"# configuration parameters:\n") ;
    fprintf(fp,"#numcells %d\n", numcellsG ) ;
    fprintf(fp,"%u\n", Yget_random_var() ) ;
    fprintf(fp,"%d %d\n", m*(blocktG-blockbG), m*(blockrG-blocklG) ) ;
    fprintf(fp,"%d %d\n", m*blockmxG, m*blockmyG ) ;
    fprintf(fp,"%d %d\n",m*binWidthXG,m*binXOffstG ) ;
    fprintf(fp,"%d %d\n",m*binWidthYG,m*binYOffstG ) ;
    HPO(fp,slopeXG) ;
    HPO(fp,slopeYG) ;
    HPO(fp,baseWeightG) ;
    HPO(fp,wireFactorXG) ;
    HPO(fp,wireFactorYG) ;
    HPO(fp,aveChanWidG) ;
    HPO(fp,lapFactorG) ;

    for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	cellptr = cellarrayG[ cell ] ;
	fprintf( fp , "%d %d %d %d\n", cell , cellptr->orient , 
	    cellptr->xcenter*m, 
	    cellptr->ycenter*m ) ;
	if( cellptr->instptr ){
	    fprintf( fp , "%d\n", cellptr->cur_inst ) ;
	}
	if( cellptr->softflag ) {
	    fprintf( fp , " %lf\n" , cellptr->aspect );
	    count = 0 ;
	    for( pin = cellptr->pinptr; pin; pin = pin->nextpin ){
		fprintf( fp, "%d %d ", m*pin->txpos, m*pin->typos ) ;
		if( (++count % 15 ) == 0 ){
		    fprintf( fp, "\n" ) ;
		}
	    }
	    fprintf( fp, "\n" ) ;
	}
    }
    TWCLOSE( fp ) ;

#ifdef UNIX
    sprintf(file1, "%s.mtmp", cktNameG);
    sprintf(file2, "%s.msav", cktNameG);
    rename(file1, file2);
#endif
    return ;

} /* end savewolf */


/* ***************************************************************** 
   RESTART TIMBERWOLFMC ROUTINE
*/
BOOL TW_oldinput( fp )
FILE *fp ;
{

#define HOWMANY  0
INT i, cell, orient, site, terminal;
INT xcenter , ycenter, error ;
INT oleft , oright , obottom , otop , Hdiv2 , Wdiv2 ;
INT number_of_cells, inst ;
DOUBLE aspect , aspFactor , val ;
DOUBLE percentDone ;
DOUBLE eval_ratio( P1(INT iterationG) ) ;
CELLBOXPTR ptr ;
TILEBOXPTR tileptr1 ;
PINBOXPTR *sarray ; /* array of the current softpins */
PINBOXPTR term ;
INSTBOXPTR instptr ;
BOUNBOXPTR bounptr ;
VERTBOXPTR vert ;
INT *xorig ;
INT *xnew ;
INT *yorig ;
INT *ynew ;
INT howmany ;

error = 0 ;
error += read_uloop( fp ) ;
error += read_window( fp ) ;
error += read_wireest( fp ) ;
fscanf(fp,"%[ #:a-zA-Z]\n",YmsgG ); /* throw away comment */
fscanf( fp, "%[ #:a-zA-Z]%d\n", YmsgG, &number_of_cells ) ; 
if( number_of_cells != numcellsG ){
    M(ERRMSG,"TW_oldinput","Number of cells in restart file in error\n");
    /* abort at this time no sense going on */
    return( FALSE ) ;
}
fscanf( fp, "%u", &randVarG ) ;
Yset_random_seed( randVarG ) ;
fscanf( fp, "%d %d\n", &blocktG, &blockrG ) ; 
/* set blockl and blockb to zero anticipating call to placepads */
blocklG = blockbG = 0 ; 
fscanf( fp, "%d %d\n", &blockmxG, &blockmyG ) ; 
fscanf( fp, "%d %d\n", &binWidthXG, &binXOffstG ) ; 
fscanf( fp, "%d %d\n", &binWidthYG, &binYOffstG ) ; 
HPI(fp,&slopeXG) ;
HPI(fp,&slopeYG) ;
HPI(fp,&baseWeightG) ;
HPI(fp,&wireFactorXG) ;
HPI(fp,&wireFactorYG) ;
HPI(fp,&aveChanWidG) ;
HPI(fp,&lapFactorG) ;

while( fscanf( fp , " %d %d %d %d ", &cell , &orient , 
				    &xcenter , &ycenter ) == 4 ) {
    ptr = cellarrayG[ cell ] ;

    ptr->orient  = orient  ;
    ptr->xcenter = xcenter ;
    ptr->ycenter = ycenter ;
    ptr->boun_valid = FALSE ;
    if( instptr = ptr->instptr ){
	fscanf( fp , "%d", &inst ) ;
	ptr->cur_inst = inst ;
	/* update tiles */
	ptr->tiles = instptr->tile_inst[inst] ;
	ptr->numtiles = instptr->numtile_inst[inst] ;
	/* swap in the new sites */

	/* delay recalculation of bounding box views until needed */
	ptr->bounBox = instptr->bounBox[inst] ;
	ptr->numsides = instptr->numsides[inst] ;
	ptr->vertices = instptr->vert_inst[inst] ;
    } else {
	inst = 0 ;
    }

    if( ptr->softflag ) {
	fscanf( fp , "%lf" , &aspect ) ;
	ptr->aspect = aspect ;
	for( term = ptr->pinptr; term; term = term->nextpin ){
	    fscanf( fp, "%d %d", &(term->txpos), &(term->typos) ) ;
	    term->txpos_orig[inst] = term->txpos ;
	    term->typos_orig[inst] = term->typos ;
	}

	howmany = (INT) ptr->softpins[HOWMANY] ;
	/* set each pin's correct instance */
	for( i = 1 ; i <= howmany; i++ ){
	    term = ptr->softpins[i] ;
	    term->softinfo = term->soft_inst[inst] ;
	}
    }


    if( ptr->softflag == 1 && ptr->aspUB > 0.01 + ptr->aspLB ) {
	/* determine aspect ratio orientation */
	aspFactor = sqrt( aspect / ptr->orig_aspect ) ;
	/* use original bounding box to calculate changes */
	bounptr = ptr->bounBox[8] ; /* original bbox */
	oleft = bounptr->l ;
	obottom = bounptr->b ;
	oright = bounptr->r ;
	otop = bounptr->t ;
	val = (DOUBLE) (otop - obottom) * aspFactor ;
	Hdiv2 = ROUND( val ) / 2 ;
	val = (DOUBLE) (oright - oleft) / aspFactor ;
	Wdiv2 = ROUND( val ) / 2 ;


	/*
	 *   Time to compute the new coordinates for the cell
	 *   since its aspect ratio has changed.  Do it for
	 *   the orient 0 version and then use the routines
	 *   from readcells() to transfer for other orients.
	 */

	for( tileptr1 = ptr->tiles;tileptr1;tileptr1 = tileptr1->next ){
	    val = (DOUBLE)(tileptr1->orig_left - oleft) / aspFactor ;
	    tileptr1->left   = ROUND( val ) - Wdiv2 ;
	    val = (DOUBLE)(tileptr1->orig_right - oleft) / aspFactor ;
	    tileptr1->right  = ROUND( val ) - Wdiv2 ;
	    val = (DOUBLE)(tileptr1->orig_bottom - obottom) * aspFactor;
	    tileptr1->bottom = ROUND( val ) - Hdiv2 ;
	    val = (DOUBLE)(tileptr1->orig_top - obottom) * aspFactor ;
	    tileptr1->top    = ROUND( val ) - Hdiv2 ;
	}
	/* modify the vertices of the cell for the softpin moves */
	vert = ptr->vertices ;
	xorig = vert->x_orig ;
	xnew = vert->x ;
	yorig = vert->y_orig ;
	ynew = vert->y ;
	for( i = 1; i <= ptr->numsides; i++ ){
	    val = (DOUBLE)(xorig[i] - oleft) / aspFactor ;
	    xnew[i] = ROUND( val ) - Wdiv2 ;
	    val = (DOUBLE)(yorig[i] - obottom) * aspFactor ;
	    ynew[i] = ROUND( val ) - Hdiv2 ;
	}
    }

    for( term = ptr->pinptr ; term ; term = term->nextpin ) {
	term->txpos = term->txpos_orig[inst] ;
	term->typos = term->typos_orig[inst] ;
	term->xpos = term->txpos + xcenter ;
	term->ypos = term->typos + ycenter ;
    }
}
if( cell != numcellsG ){ /* abort restart */
    error++ ;
    M(ERRMSG,"TW_oldinput",
    "Number of cell in restart file don't match number of cells in .cel file\n") ;
}
if( error ){ /* abort restart */

    return( FALSE ) ;

} else {
    /* a valid restart - find out where we are */
    if( iterationG <= LASTTEMP ){
	/* we are still annealing */
	init_control(FALSE);  /*** set controller to old values. ***/

	/* reset the annealing controllers and reconfigure bins */
	percentDone = eval_ratio( iterationG ) ;

	/* **** core area controller **** */
	coreFactorG = calc_core_factor( percentDone ) ;

	/* **** overlap penalty controller **** */
	lapFactorG = calc_lap_factor( percentDone ) ;

	/* **** timing penalty controller **** */
	timeFactorG = calc_time_factor( percentDone ) ;

    } else {
	/* we are in placement refinement */
	reorigin() ;
    }
    return( TRUE ) ;
}
} /* end TW_oldinput */

HPO(fp,d)
FILE *fp;			/* high precision output */
DOUBLE d;
{
    fprintf(fp,"%34.32le\n",d);
} /* end HPO */

HPI(fp,d)
FILE *fp;			/* high precision input */
DOUBLE *d;
{
    INT numread ;

    numread = (INT) fscanf(fp,"%s\n",YmsgG);
    if( numread != 1 ){
	M( ERRMSG, "HPI", "Could not read data.  FATAL must abort\n" ) ;
	closegraphics() ;
	YexitPgm( PGMFAIL ) ;
    }
    *d = atof( YmsgG ) ;
} /* end HPI */
