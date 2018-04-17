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
FILE:	    readblck.c                                       
DESCRIPTION:read block file.
CONTENTS:   readblck( fp )
		FILE *fp ;
DATE:	    Mar 27, 1989 
REVISIONS:  Wed Jan  2 10:55:01 CST 1991 - moved DEC code to parser.c
	    Thu Mar  7 02:42:59 EST 1991 - now there can be comments
		in the .blk file.
	    Wed Sep 11 11:25:16 CDT 1991 - updated for
		new global router algorithm (feeds).
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) readblck.c (Yale) version 4.10 11/8/91" ;
#endif
#endif

#define READBLCK_VARS

#include "standard.h"
#include "main.h"
#include "parser.h"
#include "readpar.h"
#include "config.h"
#include "readblck.h"

/* global variable references */
extern INT spacer_widthG ;
extern INT vertical_pitchG ;
extern INT total_row_lengthG ;

#if SIZEOF_VOID_P == 64
#define INTSCANSTR "%ld"
#else
#define INTSCANSTR "%d"
#endif

/* global variables */


readblck( fp )
FILE *fp ;

{

INT test , block , class , height , row , width , row_sep_abs ;
INT lowerL_x , upperR_x , lowerL_y , upperR_y ;
INT shift_amount , deviation , left_edge , reference_point ;
DOUBLE relLen , row_sep , avg_row_sep , avg_row_height ;
BOOL comment ;
char input[1024] ;

block = 0 ;
rowsG = 0 ;
num_exceptsG = 0 ;
uniform_rowsG = 1 ;
individual_rowSepsG = 0 ;
total_except_widthG = 0 ;

comment = FALSE ;
while( fscanf( fp , " %s " , input ) == 1 ) {

    if( strncmp( input , "/*", 2 ) == STRINGEQ ){
	comment = TRUE ;
	continue ;
    } else if( strncmp( input , "*/", 2 ) == STRINGEQ ){
	comment = FALSE ;
	continue ;
    }
    if( comment ){
	continue ;
    }
    if( strcmp( input , "block") == 0 ) {
	block++ ;
    } else if( strcmp( input , "row_sep") == 0 ) {
	test = fscanf( fp , " %lf " INTSCANSTR " " , &row_sep, &row_sep_abs ) ;
	if( test != 2 ) {
	    test = fscanf( fp , " %lf " , &row_sep ) ;
	    if( test != 1 ) {
	        fprintf( fpoG,"Failed to input row_sep of a block\n");
	        fprintf( fpoG,"at position row_sep\n");
	        YexitPgm(PGMFAIL);
	    } else {
	        row_sep_abs = 0;
	    }
	}
	if( ++individual_rowSepsG != block ) {
	    fprintf( fpoG,"Failed to input a row_sep for each block\n");
	    YexitPgm(PGMFAIL);
	}
    } else if( strcmp( input , "height") == 0 ) {
	test = fscanf( fp , " " INTSCANSTR " " , &height ) ;
	if( test != 1 ) {
	    fprintf( fpoG,"Failed to input height of a block\n");
	    fprintf( fpoG,"at position height\n");
	    YexitPgm(PGMFAIL);
	}
    } else if( strcmp( input , "class") == 0 ) {
	test = fscanf( fp , " " INTSCANSTR " " , &class ) ;
	if( test != 1 ) {
	    fprintf( fpoG, "Failed to input class of a block\n");
	    fprintf( fpoG, "current block: %d\n", block );
	    YexitPgm(PGMFAIL);
	}
	if( class <= 0 || class > 256 ) {
	    fprintf( fpoG, "block class is less than one ");
	    fprintf( fpoG, "or greater than the limit (256)\n");
	    fprintf( fpoG, "current block: %d\n", block );
	    YexitPgm(PGMFAIL);
	}
    } else if( strcmp( input , "rows") == 0 ) {
	(void) fscanf( fp , " " INTSCANSTR " " , &rowsG ) ;
    } else if( strcmp( input , "except") == 0 ) {
	num_exceptsG++ ;
	test = fscanf(fp, INTSCANSTR " " INTSCANSTR, &lowerL_x, &upperR_x ) ;
	if( test != 2 ) {
	    fprintf( fpoG, "error in new .blk format: except\n");
	    YexitPgm(PGMFAIL);
	}
    } else if( strcmp( input , "row") == 0 ) {
	test = fscanf(fp, INTSCANSTR " " INTSCANSTR " " INTSCANSTR " " INTSCANSTR,
			&lowerL_x, &lowerL_y, &upperR_x, &upperR_y ) ;
	if( test != 4 ) {
	    fprintf( fpoG, "error in new .blk format\n");
	    YexitPgm(PGMFAIL);
	}
    } else if( strcmp( input , "mirror") == 0 ) {
    } else if( strcmp( input , "relative_length") == 0 ) {
	(void) fscanf( fp , " %lf " , &relLen ) ;
    } else {
	fprintf( fpoG, "unexpected keyword in the .blk file\n");
	fprintf( fpoG, "current block: %d\n", block );
	YexitPgm(PGMFAIL);
    }
}


if( rowsG == 0 ) {
    numRowsG = block ;
} else {
    numRowsG = rowsG ;
    total_row_lengthG = 0 ;
}

rewind( fp ) ;

barrayG = (BBOXPTR *) Ysafe_malloc( (numRowsG + 1) * sizeof( BBOXPTR ) ) ;
rowSepsG = (DOUBLE *) Ysafe_malloc( (numRowsG + 1) * sizeof( DOUBLE ) ) ;
rowSepsAbsG = (INT *) Ysafe_malloc( (numRowsG + 1) * sizeof( INT ) ) ;

relativeLenG = (DOUBLE *) Ysafe_malloc( (numRowsG + 1) * sizeof(DOUBLE));
for( block = 1 ; block <= numRowsG ; block++ ) {
    relativeLenG[block] = 1.0 ;
}

exceptionsG = (EXCEPTBOX *) Ysafe_malloc( (1 + num_exceptsG) * 
				    sizeof( EXCEPTBOX ) ) ;

for( block = 1 ; block <= numRowsG ; block++ ) {
    
    barrayG[ block ] = (BBOXPTR) Ysafe_malloc( sizeof( BBOX ));
    barrayG[ block ]->bxcenter = 0 ;
    barrayG[ block ]->bycenter = 0 ;
    barrayG[ block ]->bleft    = 0 ;
    barrayG[ block ]->bright   = 0 ;
    barrayG[ block ]->bbottom  = 0 ;
    barrayG[ block ]->btop     = 0 ;
    barrayG[ block ]->bheight  = 0 ;
    barrayG[ block ]->blength  = 0 ;
    barrayG[ block ]->bclass   = 0 ;
    barrayG[ block ]->borient  = 0 ;
    barrayG[ block ]->desire   = 0 ;
    barrayG[ block ]->oldsize  = 0 ;
    barrayG[ block ]->newsize  = 0 ;
}

block = 0 ;
num_exceptsG = 0 ;
celllenG = 0 ;

comment = FALSE ;
while( fscanf( fp , " %s " , input ) == 1 ) {

    if( strncmp( input , "/*", 2 ) == STRINGEQ ){
	comment = TRUE ;
	continue ;
    } else if( strncmp( input , "*/", 2 ) == STRINGEQ ){
	comment = FALSE ;
	continue ;
    }
    if( comment ){
	continue ;
    }

    if( strcmp( input , "block" ) == 0 ) {
	block++ ;
    } else if( strcmp( input , "rows" ) == 0 ) {
	fscanf( fp , " " INTSCANSTR " " , &rowsG ) ;
    } else if( strcmp( input , "except") == 0 ) {
	num_exceptsG++ ;
	fscanf(fp, INTSCANSTR " " INTSCANSTR, &lowerL_x, &upperR_x ) ;
	celllenG += upperR_x - lowerL_x ;
	total_except_widthG += upperR_x - lowerL_x ;
	exceptionsG[num_exceptsG].row  = block ;
	exceptionsG[num_exceptsG].ll_x = lowerL_x ;
	exceptionsG[num_exceptsG].ll_y = lowerL_y ;
	exceptionsG[num_exceptsG].ur_x = upperR_x ;
	exceptionsG[num_exceptsG].ur_y = upperR_y ;
    } else if( strcmp( input , "row" ) == 0 ) {
	block++ ;
	fscanf(fp, INTSCANSTR " " INTSCANSTR " " INTSCANSTR " " INTSCANSTR,
		&lowerL_x, &lowerL_y, &upperR_x, &upperR_y ) ;
	height = upperR_y - lowerL_y ;
	width  = upperR_x - lowerL_x ;
	total_row_lengthG += width ;

	barrayG[block]->bxcenter = (lowerL_x + upperR_x) / 2 ;
	barrayG[block]->bycenter = (lowerL_y + upperR_y) / 2 ;
	barrayG[block]->btop     = height - height / 2 ;
	barrayG[block]->bbottom  = - height / 2 ;
	barrayG[block]->bheight  = height ;
	barrayG[block]->bright   = width - width / 2 ;
	barrayG[block]->bleft    = - width / 2 ;
	barrayG[block]->blength  = width ;
	barrayG[block]->desire   = width ;
	barrayG[block]->orig_desire = width ;
	barrayG[block]->bclass   = 1 ;
	barrayG[block]->borient  = 1 ;
    } else if( strcmp( input , "row_sep" ) == 0 ) {
	if (fscanf( fp , " %lf " INTSCANSTR " " , &row_sep, &row_sep_abs ) != 2) {
	    fscanf( fp , " %lf " , &row_sep );
	    rowSepsAbsG[block] = 0;
	} else {
	    rowSepsAbsG[block] = row_sep_abs ;
	}
	rowSepsG[block] = row_sep ;
    } else if( strcmp( input , "height" ) == 0 ) {
	fscanf( fp , " " INTSCANSTR " " , &height ) ;
	barrayG[block]->btop    = height - height / 2 ;
	barrayG[block]->bbottom = - height / 2 ;
	barrayG[block]->bheight = height ;
    } else if( strcmp( input , "class" ) == 0 ) {
	fscanf( fp , " " INTSCANSTR " " , &class ) ;
	barrayG[block]->bclass = class ;
	barrayG[block]->borient = 1 ;
    } else if( strcmp( input , "mirror" ) == 0 ) {
	barrayG[block]->borient = 2 ;
    } else if( strcmp( input , "relative_length" ) == 0 ) {
	fscanf( fp , " %f " , &relLen ) ;
	relativeLenG[block] = relLen ;
	uniform_rowsG = 0 ;
    }
}


if( rowsG > 0 ) {  /*  then compute rowSepG */
    if( rowsG > 1 ) {
	avg_row_sep = 0.0 ;
	for( row = 2 ; row <= rowsG ; row++ ) {
	    avg_row_sep += (DOUBLE) barrayG[row]->bycenter -
			    (DOUBLE) barrayG[row-1]->bycenter ;
	}
	avg_row_sep /= ((DOUBLE) rowsG - 1.0) ;

	avg_row_height = 0.0 ;
	for( row = 1 ; row <= rowsG ; row++ ) {
	    avg_row_height += (DOUBLE) barrayG[row]->bheight ;
	}
	avg_row_height /= (DOUBLE) rowsG ;
    } else {
	avg_row_height = (DOUBLE) barrayG[1]->bheight ;
	avg_row_sep = avg_row_height ;
    }

    rowSepG = (avg_row_sep - avg_row_height) / avg_row_height ;
}

if( rowsG == 0 && rowSepG < 0.0 ) {
    fprintf( fpoG, "rowSep was not entered in the .par file\n");
    YexitPgm(PGMFAIL);
} else {
    fprintf( fpoG, "rowSep: %f\n" , rowSepG ) ;
}

if( rowsG > 0 ) {
    top_of_top_rowG = barrayG[rowsG]->bycenter + barrayG[rowsG]->btop ;
    bot_of_bot_rowG = barrayG[1]->bycenter + barrayG[rowsG]->bbottom ;

    /*  make sure left edges of the rows latch to a grid
	relative to left edge of the first row   */
    
    if( vertical_pitchG > 0 ) {
	reference_point = barrayG[1]->bxcenter + barrayG[1]->bleft ;
	for( row = 2 ; row <= numRowsG ; row++ ) {
	    shift_amount = 0 ;
	    left_edge = barrayG[row]->bxcenter + barrayG[row]->bleft ;
	    deviation = left_edge - reference_point ;
	    if( deviation > 0 ) {
		deviation = deviation % vertical_pitchG ;
		if( deviation != 0 ) {
		    if( deviation > vertical_pitchG / 2 ) {
			shift_amount = vertical_pitchG - deviation ;
		    } else {
			shift_amount = - deviation ;
		    }
		}
	    } else if( deviation < 0 ) {
		deviation = (-deviation) % vertical_pitchG ;
		if( deviation != 0 ) {
		    if( deviation > vertical_pitchG / 2 ) {
			shift_amount = -(vertical_pitchG - deviation) ;
		    } else {
			shift_amount = deviation ;
		    }
		}
	    }
	    barrayG[row]->bxcenter += shift_amount ;
	}
    }
}

return ;
}
