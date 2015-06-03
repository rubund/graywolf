/*
 *   Copyright (C) 1988-1992 Yale University
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
FILE:	    output.c                                       
DESCRIPTION:placer output routine
CONTENTS:   output( fp )
		FILE *fp ;
DATE:	    Jan 29, 1988 
REVISIONS:  Aug 10, 1988 - add compile switch for Dahe.
	    Mar 29, 1989 - added translatef for floating numbers.
	    Apr 27, 1989 - removed Dahe's format by adding new
		format.
	    Apr 30, 1989 - fixed missing file pointer.
	    May  8, 1989 - added update to stat file.
	    May 11, 1989 - made output same as input format.
	    May 16, 1989 - removed unneccessary code.
	    Jun 21, 1989 - now output only true pin locations not
		averaged equiv locations.
	    Sep 16, 1989 - fixed problem with unbust.  Multiple tile
		cells should use tile count as argument since tiles
		need to be seamed together.
	    Oct  5, 1989 - now output equivs correctly.
	    Oct 15, 1989 - need to rotate border so output is in correct
		orientation.
	    Nov 30, 1989 - fixed bug in inverse orientation.
	    Apr 23, 1990 - modified for the new softpin algorithm.
	    Apr 26, 1990 - take absolute value of layers in order to
		handle tenatively assigned layers.
	    Jun 21, 1990 - Output the aspect ratio for standard cells.
	    Aug 10, 1990 - moved translation files to library.
	    Sun Dec 16 00:33:09 EST 1990 - Now use the library version
		of buster.
	    Wed Jan 30 14:13:36 EST 1991 - now left justify orientation
		rotations.
	    Wed Feb 13 23:53:54 EST 1991 - added outpins function.
	    Sat Feb 23 00:26:42 EST 1991 - fixed problems with outputing
		padgroups.
	    Tue Mar 12 17:02:16 CST 1991 - now no longer determine side
		of pad in partitioning case.
	    Tue Mar 19 16:33:23 CST 1991 - fixed typo in output_pad_groups
		which accessed the wrong array.
	    Thu Apr 18 01:38:21 EDT 1991 - reversed corners.
	    Sat Apr 27 01:12:42 EDT 1991 - no longer need to output
		number of orientations.
	    Wed Jun  5 16:30:30 CDT 1991 - eliminated unbust.
	    Wed Jun 12 13:43:19 CDT 1991 - fixed problem with ordered pad
		groups.
	    Mon Aug 12 16:57:04 CDT 1991 - create sc output files.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) output.c version 3.16 3/6/92" ;
#endif

#include <custom.h>
#include <pads.h>
#include <initialize.h>
#include <yalecad/buster.h>
#include <yalecad/debug.h>
#include <yalecad/relpos.h>
#include <yalecad/file.h>
#include <yalecad/string.h>

static FILE *outS ;                    /* the output file */
static BOOL determine_sideS = TRUE ;   /* normally determine side */





static output_corners();
static output_class_orient();
static output_pins();
static output_side_n_space();
static output_pad_groups();
static output_fixed();
static create_pl1();
static print_four_corners();
static create_pin();





output( fp )
FILE *fp ;
{

    INT cell ;
    char filename[LRECL] ;
    CELLBOXPTR cellptr ;


    if( fp ){
	/* partition output case */
	outS = fp ;
    } else {
	/* normal case */
	sprintf( filename, "%s.mdat", cktNameG ) ;
	outS = TWOPEN( filename, "w", ABORT ) ;
    }

    for( cell = 1 ; cell <= totalcellsG ; cell++ ) {
	cellptr = cellarrayG[cell] ;
	switch( cellptr->celltype ){
	    case CUSTOMCELLTYPE :
		fprintf( outS,"\nhardcell %d name %s\n", 
		    cellptr->cellnum, cellptr->cname ) ;
		output_fixed( cellptr ) ;
		output_corners( cellptr ) ;
		output_class_orient( cellptr ) ;
		output_pins( cellptr ) ;
		break ;
	    case PADCELLTYPE :
		fprintf( outS,"\npad %d name %s\n",
		    cellptr->cellnum, cellptr->cname ) ;
		output_corners( cellptr ) ;
		fprintf( outS, "orient %d\n", cellptr->orient ) ;
		output_side_n_space( cellptr ) ;
		output_pins( cellptr ) ;
		break ;
	    case SOFTCELLTYPE :
		fprintf( outS,"\nsoftcell %d name %s\n",
		    cellptr->cellnum, cellptr->cname ) ;
		output_fixed( cellptr ) ;
		output_corners( cellptr ) ;
		fprintf( outS,"asplb %lf aspub %lf\n", 
		    cellptr->aspLB, cellptr->aspUB ) ;
		output_class_orient( cellptr ) ;
		output_pins( cellptr ) ;
		break ;
	    case PADGROUPTYPE :
		if( cellptr->padptr->permute ){
		    fprintf( outS,"\npadgroup %s permute\n",
			cellptr->cname );
		} else {
		    fprintf( outS,"\npadgroup %s nopermute\n",
			cellptr->cname);
		}
		output_pad_groups( cellptr ) ;
		output_side_n_space( cellptr ) ;
		break ;
	    case SUPERCELLTYPE :
	    case MERGEDCELLTYPE :
	    case GROUPCELLTYPE :
	    case PADMACROTYPE :
		break ;
	    case STDCELLTYPE :
		if( fp ){
		    /* partition output file does not need this */
		    break ;
		}
		fprintf( outS,"\nstdcell %d name %s\n",
		    cellptr->cellnum, cellptr->cname ) ;
		output_corners( cellptr ) ;
		fprintf( outS,"asplb %lf aspub %lf\n", 
		    cellptr->aspLB, cellptr->aspUB ) ;
		output_class_orient( cellptr ) ;
		break ;
	}
    }
    TWCLOSE( outS ) ;

    return ;

} /* end output */

output_pads( fp )
FILE *fp ;
{

    INT cell ;
    BOOL save_state ;
    BOOL save_orient ;
    CELLBOXPTR cellptr ;


    outS = fp ;
    save_state = determine_sideS ;
    determine_sideS = FALSE ;       /* let placepads pick side */

    for( cell = endsuperG ; cell <= totalcellsG ; cell++ ) {
	cellptr = cellarrayG[cell] ;
	save_orient = cellptr->orient ;
	cellptr->orient = 0 ;
	/* pads should be output in orient 0 */
	switch( cellptr->celltype ){
	    case PADCELLTYPE :
		fprintf( outS,"\npad %d name %s\n",
		    cellptr->cellnum, cellptr->cname ) ;
		output_corners( cellptr ) ;
		output_side_n_space( cellptr ) ;
		output_pins( cellptr ) ;
		break ;
	    case PADGROUPTYPE :
		if( cellptr->padptr->permute ){
		    fprintf( outS,"\npadgroup %s permute\n",
			cellptr->cname );
		} else {
		    fprintf( outS,"\npadgroup %s nopermute\n",
			cellptr->cname);
		}
		output_pad_groups( cellptr ) ;
		output_side_n_space( cellptr ) ;
		break ;
	}
	/* now put orientation back */
	cellptr->orient = save_orient ;
    }
    determine_sideS = save_state ;
    return ;

} /* end output_pads */

static output_corners( cellptr )
CELLBOXPTR cellptr ;
{
    fprintf( outS, "corners %d\n", cellptr->numsides ) ;
    output_vertices( outS, cellptr ) ;
} /* end output_corners */

static output_class_orient( cellptr )
CELLBOXPTR cellptr ;
{
    INT i ;

    fprintf( outS, "class %d ", cellptr->class ) ;
    fprintf( outS, "orientations " ) ;
    for( i = 0; i <= 7 ; i++ ){
	if( cellptr->orientList[i] ){
	    fprintf( outS, "%d ", i ) ;
	}
    }
    fprintf( outS, "\n" ) ;
    fprintf( outS, "orient %d\n", cellptr->orient ) ;

} /* end output_class_orient */


static output_pins( cellptr )
CELLBOXPTR cellptr ;
{
    INT i ;                  /* counter */
    INT x,  y ;              /* absolute coordinates */
    INT instance ;           /* current instance */
    INT howmany ;            /* number of equivalent pins */
    PINBOXPTR pinptr ;       /* current pin */
    PINBOXPTR child ;        /* softcell child */
    SOFTBOXPTR spin ;        /* current softpin information */
    EQUIVPTR eqptr ;         /* current equiv */

    instance = cellptr->cur_inst ;

    for( pinptr=cellptr->pinptr;pinptr;pinptr = pinptr->nextpin ){

	if( pinptr->type == SOFTEQUIVTYPE ){
	    /* delay the equivalent softpins until below */
	    continue ;
	} else {

	    /* rel position is a macro which calculates */
	    /* absolute pin loc - defined in relpos.h */
	    REL_POST( cellptr->orient, 
		x, y,                               /* result */
		pinptr->txpos_orig[instance],
		pinptr->typos_orig[instance],     /* cell relative */
		cellptr->xcenter, cellptr->ycenter ) ;  /* cell center */

	    fprintf(outS,"pin name %s signal %s layer %d %d %d\n",
		pinptr->pinname, netarrayG[ pinptr->net ]->nname, 
		ABS(pinptr->layer), x , y ) ;
	}
	/* ******** HARD PIN EQUIVALENT CASE ****** */
	if( pinptr->eqptr && !(pinptr->softinfo)){ 
	    /* occurs only for hard pins */
	    /* now process the equivalent pins */
	    for( eqptr=pinptr->eqptr; eqptr; eqptr=eqptr->next ){
		REL_POST( cellptr->orient, 
		    x, y,                               /* result */
		    eqptr->txpos[instance],
		    eqptr->typos[instance],          /* cell relative */
		    cellptr->xcenter, cellptr->ycenter ) ; /* center */
		fprintf(outS,"equiv name %s layer %d %d %d\n",
		    pinptr->pinname,  ABS(pinptr->layer), x , y ) ;
	    }
	}
	/* ******** SOFT PIN EQUIVALENT CASE ****** */
	/* now look for equivalent pins are children */
	if( pinptr->type == SOFTPINTYPE ){
	    spin = pinptr->softinfo ;
	    if( spin->children ){
		howmany = (INT) spin->children[HOWMANY] ;
	    } else {
		howmany = 0 ;
	    }
	    for( i = 1; i <= howmany; i++ ){
		child = spin->children[i] ;
		REL_POST( cellptr->orient, 
		    x, y,                               /* result */
		    child->txpos, child->typos,     /* cell relative */
		    cellptr->xcenter, cellptr->ycenter ) ; /* center */

		fprintf(outS,"equiv name %s layer %d %d %d\n",
		    child->pinname,  ABS(pinptr->layer), x , y ) ;
	    }
	}
    }
} /* end output_pins */
static output_side_n_space( cellptr )
CELLBOXPTR cellptr ;
{
    INT i ;           /* counter */
    PADBOXPTR pad ;

    pad = cellptr->padptr ;
    if(!(pad->valid_side[ALL] )){
	if( determine_sideS ){
	    switch( pad->padside ){
	    case L:
		fprintf( outS, "restrict side L\n" ) ;
		break ;
	    case T:
		fprintf( outS, "restrict side T\n" ) ;
		break ;
	    case R:
		fprintf( outS, "restrict side R\n" ) ;
		break ;
	    case B:
		fprintf( outS, "restrict side B\n" ) ;
		break ;
	    } /* end switch */

	} else {
	    /* pass constraints to the output file */
	    fprintf( outS, "restrict side " ) ;
	    if( pad->valid_side[L] ){
		fprintf( outS, "L" ) ;
	    }
	    if( pad->valid_side[T] ){
		fprintf( outS, "T" ) ;
	    }
	    if( pad->valid_side[R] ){
		fprintf( outS, "R" ) ;
	    }
	    if( pad->valid_side[B] ){
		fprintf( outS, "B" ) ;
	    } 
	    fprintf( outS, "\n" ) ;

	}

    }
    if( pad->fixed ){
	    fprintf( outS, "sidespace %lf %lf\n", pad->lowerbound,
		pad->upperbound ) ;
    }
} /* end output_side_n_space */

static output_pad_groups( cellptr )
CELLBOXPTR cellptr ;
{
    INT i, child, padnum ;
    PADBOXPTR pad ;
    CELLBOXPTR cptr ;       /* temporary pointer to child cell rec */

    pad = cellptr->padptr ;
    for( i = 1; i <= pad->children[HOWMANY]; i++ ){
	padnum = pad->children[i] ;
	child = padarrayG[padnum]->cellnum ;
	ASSERTNCONT( child > 0 && child <= totalcellsG,"output",
	    "pad child out of bounds\n" ) ;
	cptr = cellarrayG[child] ;
	fprintf( outS, "%s ", cellarrayG[child]->cname ) ; 
	if( cptr->padptr->ordered ){
	    fprintf( outS, "fixed\n" ) ;
	} else {
	    fprintf( outS, "nonfixed\n" ) ;
	}
    }
    
} /* end output_pad_groups */

static output_fixed( cellptr )
CELLBOXPTR cellptr ;
{
    FIXEDBOXPTR fixptr ;

    if(!(fixptr = cellptr->fixed)){
	return ;
    }
    if( fixptr->fixedType == POINTFLAG ){
	fprintf( outS, "fixed at " ) ;
	if( fixptr->leftNotRight == TRUE ){
	    fprintf( outS, "%d from L ", fixptr->xcenter ) ;
	} else {
	    fprintf( outS, "%d from R ", fixptr->xcenter ) ;
	}
	if( fixptr->bottomNotTop == TRUE ){
	    fprintf( outS, "%d from B ", fixptr->ycenter ) ;
	} else {
	    fprintf( outS, "%d from T ", fixptr->ycenter ) ;
	}
    } else {
	fprintf( outS, "fixed neighborhood " ) ;
	if( fixptr->leftNotRight == TRUE ){
	    fprintf( outS, "%d from L ", fixptr->xloc1 ) ;
	} else {
	    fprintf( outS, "%d from R ", fixptr->xloc1 ) ;
	}
	if( fixptr->bottomNotTop == TRUE ){
	    fprintf( outS, "%d from B ", fixptr->yloc1 ) ;
	} else {
	    fprintf( outS, "%d from T ", fixptr->yloc1 ) ;
	}
	if( fixptr->leftNotRight2 == TRUE ){
	    fprintf( outS, "%d from L ", fixptr->xloc2 ) ;
	} else {
	    fprintf( outS, "%d from R ", fixptr->xloc2 ) ;
	}
	if( fixptr->bottomNotTop2 == TRUE ){
	    fprintf( outS, "%d from B ", fixptr->yloc2 ) ;
	} else {
	    fprintf( outS, "%d from T ", fixptr->yloc2 ) ;
	}
    }
} /* end output_fixed */

set_determine_side( flag )
BOOL flag ;
{
    determine_sideS = flag ;
} /* end set_determine_side */

output_vertices( fp, cellptr )
FILE *fp ;
CELLBOXPTR cellptr ;
{
    INT xc , yc ;
    TILEBOXPTR tileptr ;
    BOUNBOXPTR bounptr ;         /* bounding box pointer */
    INT *xvert ;                 /* the xvertices of cell */
    INT *yvert ;                 /* the yvertices of cell */
    VERTBOXPTR vert ;            /* the cells vertices */
    INT p ;
    INT x, y ;
    INT l, b ;
    INT this_pt ;
    INT inverse ;
    INT old_orient ;
    INT lowestp ;
    INT limit ;
    INT nextpos, nextneg ;
    INT *temp_x ;
    INT *temp_y ;
    INT numpts ;

    xc = cellptr->xcenter ;
    yc = cellptr->ycenter ;
    vert = cellptr->vertices ;
    xvert = vert->x ;
    yvert = vert->y ;
    numpts = cellptr->numsides ;

    /* setup translation of output points */
    bounptr = cellptr->bounBox[0] ;
    /* now init the translation routines using bounding box */
    Ytrans_init( bounptr->l,bounptr->b,bounptr->r,bounptr->t,
		cellptr->orient ) ;

    /* allocate the space for reoordering the points of the macro */
    temp_x = (INT *) Yvector_alloc( 1, numpts,sizeof(INT) ) ;
    temp_y = (INT *) Yvector_alloc( 1, numpts,sizeof(INT) ) ;
    for( p = 1; p <= cellptr->numsides; p++ ){
	/* rel position is a macro which calculates */
	/* absolute pin loc - defined in relpos.h */
	REL_POST( cellptr->orient, 
	    temp_x[p], temp_y[p],                 /* result */
	    xvert[p], yvert[p], xc, yc ) ;
    }
    /* now we need to reorder the vertices */
    /* all the points have been rotated into their proper view and */
    /* stored in the vertices array.  Now find lowest of the left pts. */
    l = INT_MAX ;
    b = INT_MAX ;
    for( p = 1; p <= numpts; p++ ){
	if( temp_x[p] <= l ){
	    if( temp_y[p] <= b ){
		l = temp_x[p] ;
		b = temp_y[p] ;
		lowestp = p ;
	    }
	}
    }
    /* output the first points */
    fprintf( fp, "%d %d ", l, b ) ;

    /* now determine CW direction */
    nextpos = lowestp + 1 ;
    if( nextpos > numpts ){
	nextpos = 1 ;
    }
    if( lowestp == 1 ){
	nextneg = numpts ;
    } else {
	nextneg = lowestp - 1 ;
    }
    if( temp_x[nextpos] == l && temp_y[nextpos] > b ){
	/* clockwise is positive */
	limit = lowestp + numpts; 
	for( p = lowestp+1; p < limit; p++ ) {
	    if( p > numpts ){
		this_pt = p - numpts ;
	    } else {
		this_pt = p ;
	    }
	    fprintf( fp, "%d %d ", temp_x[this_pt], temp_y[this_pt] ) ; 
	}

    } else if( temp_x[nextneg] == l && temp_y[nextneg] > b ){
	/* clockwise is negative */
	limit = lowestp - numpts ;
	for( p = lowestp-1; p > limit; p-- ) {
	    if( p < 1 ){
		this_pt = numpts + p ;
	    } else {
		this_pt = p ;
	    }
	    fprintf( fp, "%d %d ", temp_x[this_pt], temp_y[this_pt] ) ; 
	}
    } else {
	M( ERRMSG, "output_vertices",
	"couldn't find clockwise direction for boundary\n" ) ;
    }
    fprintf( fp, "\n" ) ;
    Yvector_free( temp_x, 1, sizeof(INT) ) ;
    Yvector_free( temp_y, 1, sizeof(INT) ) ;

} /* end output_vertices */

create_sc_output()
{
    create_pl1() ;
    create_pin() ;
} /* end create_sc_output */

/** by kroy July 1991 **/
/* modified by WPS Aug 6, 1991 */
static create_pl1()
{
    FILE *fpp1 ;
    CELLBOXPTR cellptr ;
    PADBOXPTR pad ;
    INT cell, length, row ;
    char instance_name[LRECL], tmp_name[LRECL], *tmp_string;
    char filename[LRECL];

    sprintf( filename , "%s.pl1" , cktNameG ) ;
    fpp1 = TWOPEN( filename , "w", ABORT ) ;

    for( cell = 1 ; cell <= totalcellsG ; cell++ ) {
	cellptr = cellarrayG[cell] ;
	switch( cellptr->celltype ){
	case CUSTOMCELLTYPE :
	case SOFTCELLTYPE :
#ifdef NSC
	    strcpy( tmp_name , cellptr->cname ) ;
	    length = strcspn( tmp_name , ":" ) ;
	    if( length < strlen( tmp_name ) ) {
		tmp_string = strtok( tmp_name , ":" ) ;
		tmp_string = strtok( NULL , ":" ) ;
		sprintf( instance_name, "%s" , tmp_string ) ;
	    } else {
		sprintf( instance_name , "%s" , tmp_name ) ;
	    }
	    fprintf(fpp1,"%s ", instance_name);
#else
	    fprintf(fpp1,"%s ", cellptr->cname);
#endif
	    print_four_corners (fpp1, cellptr ) ;

	    fprintf(fpp1," %d 0\n",  cellptr->orient ) ;
	    break ;
	case PADCELLTYPE :
#ifdef NSC
	    strcpy( tmp_name , cellptr->cname ) ;
	    length = strcspn( tmp_name , ":" ) ;
	    if( length < strlen( tmp_name ) ) {
		tmp_string = strtok( tmp_name , ":" ) ;
		tmp_string = strtok( NULL , ":" ) ;
		sprintf( instance_name, "%s" , tmp_string ) ;
	    } else {
		sprintf( instance_name , "%s" , tmp_name ) ;
	    }
	    fprintf(fpp1,"%s ", instance_name);
#else
	    fprintf(fpp1,"%s ", cellptr->cname);
#endif
	    print_four_corners (fpp1, cellptr ) ;
	    pad = cellptr->padptr ;

	    switch( pad->padside ){
	    case L:
		row = - 1 ;
		break ;
	    case T:
		row = -4 ;
		break ;
	    case R:
		row = - 2 ;
		break ;
	    case B:
		row = - 3 ;
		break ;
	    } /* end switch */
	    fprintf(fpp1," %d %d\n",  cellptr->orient , row ) ;
	    break ;
	case PADGROUPTYPE :
	case SUPERCELLTYPE :
	case MERGEDCELLTYPE :
	case GROUPCELLTYPE :
	case PADMACROTYPE :
	case STDCELLTYPE :
	    break ;
	} /* end switch( cellptr->celltype... */
    } /* end for( cell = 1... */
    fflush( fpp1 );
    TWCLOSE( fpp1 ) ;

} /* create_pl1 */

static print_four_corners( fp, cellptr )
FILE *fp ;
CELLBOXPTR cellptr ;
{
    INT xc , yc ;
    BOUNBOXPTR bounptr ;         /* bounding box pointer */

    xc = cellptr->xcenter ;
    yc = cellptr->ycenter ;

    /* setup translation of output points */
    bounptr = cellptr->bounBox[0] ;
    /* now init the translation routines using bounding box */
    Ytrans_init( bounptr->l,bounptr->b,bounptr->r,bounptr->t,
                cellptr->orient ) ;

    YtranslateC( &(bounptr->l), &(bounptr->b), 
	&(bounptr->r),&(bounptr->t), cellptr->orient ) ;

    fprintf( fp, "%d %d ", bounptr->l + xc, bounptr->b + yc ) ;
    fprintf( fp, "%d %d ", bounptr->r + xc, bounptr->t + yc ) ;
    fflush ( fp );
} /* end print_four_corners */


/* by WPS Aug 6, 1991 */
static create_pin()
{
    FILE *fpp1 ;
    CELLBOXPTR cellptr ;
    PADBOXPTR pad ;
    char filename[LRECL];
    INT net, row ;
    NETBOXPTR netptr ;
    PINBOXPTR pinptr ;

    sprintf( filename , "%s.pin" , cktNameG ) ;
    fpp1 = TWOPEN( filename , "w", ABORT ) ;

    for( net = 1 ; net <= numnetsG ; net++ ) {

	netptr =  netarrayG[net] ;
	if(!(netptr)) {
	    continue ;
	}
	if( netptr->numpins <= 1 ) {
	    continue ;
	}
	/* output the pins on the net */
	for( pinptr = netptr->pins;pinptr; pinptr = pinptr->next ) {
	    cellptr = cellarrayG[pinptr->cell] ;
	    switch( cellptr->celltype ){
	    case CUSTOMCELLTYPE :
	    case SOFTCELLTYPE :
		fprintf( fpp1, "%s %d %s %s %d %d 0 0 %d\n",
		    netptr->nname, net, cellptr->cname, pinptr->pinname,
		    pinptr->xpos, pinptr->ypos, pinptr->layer ) ;
		break ;
	    case PADCELLTYPE :
		pad = cellptr->padptr ;

		switch( pad->padside ){
		case L:
		    row = - 1 ;
		    break ;
		case T:
		    row = -4 ;
		    break ;
		case R:
		    row = - 2 ;
		    break ;
		case B:
		    row = - 3 ;
		    break ;
		} /* end switch */
		fprintf( fpp1, "%s %d %s %s %d %d %d 0 %d\n",
		    netptr->nname, net, cellptr->cname, pinptr->pinname,
		    pinptr->xpos, pinptr->ypos, row, pinptr->layer ) ;
		fprintf( fpp1, "%s %d PSEUDO_CELL PSEUDO_PIN %d %d %d 0 %d\n",
		    netptr->nname, net,
		    pinptr->xpos, pinptr->ypos, row, pinptr->layer ) ;
		fprintf( fpp1, "%s %d PSEUDO_CELL PSEUDO_PIN %d %d 0 0 %d\n",
		    netptr->nname, net,
		    pinptr->xpos, pinptr->ypos, pinptr->layer ) ;
		break ;
	    case PADGROUPTYPE :
	    case SUPERCELLTYPE :
	    case MERGEDCELLTYPE :
	    case GROUPCELLTYPE :
	    case PADMACROTYPE :
	    case STDCELLTYPE :
		break ;
	    } /* end switch( cellptr->celltype... */
	}
    } /* end for( cell = 1... */
    fflush( fpp1 );
    TWCLOSE( fpp1 ) ;

} /* create_pin */
