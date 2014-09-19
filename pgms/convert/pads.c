/* ----------------------------------------------------------------- 
FILE:	    pads.c
DESCRIPTION:This file contains the pad processing routines.
CONTENTS:   
DATE:	    Apr 27, 1988 - original coding.
REVISIONS:  Jun 19, 1989 - added macro processing.  Modified statics
		to have S suffix.
	    Mon Jan 21 21:35:47 PST 1991 - now handle case where one
		or more sides are missing.
	    Wed May  1 18:59:26 EDT 1991 - reversed corners.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) pads.c version 1.8 5/1/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include "globals.h"

#define E                   0
#define PADKEYWORD          "pad"
#define RELATIVE_TO_CURPOS  1
#define SIGNAL              "signal"
#define LAYER               "layer"
#define EQUIV               "equiv"

static int transTableS[5][8] = {
    { E, E, E, E, E, E, E, E   },   /* error state */
    { 0, 1, 2, 3, 4, 5, 7, 6   },   /* PADB state */
    { 6, 5, 4, 7, 1, 2, 3, 0   },   /* PADL state */
    { 7, 4, 5, 6, 2, 1, 0, 3   },   /* PADR state */
    { 1, 0, 3, 2, 6, 7, 4, 5   }    /* PADT state */
} ;

static char *padnameS[1000] ;
static int  start_sideS[5] = {
    0, 0, 0, 0, 0
} ;
static int  end_sideS[5] ;
static int  padCounterS = 0 ;
static int  padnumS ;

static DOUBLE side_spaceS ;
static DOUBLE space_counterS ;
static int pad_sideS = -1 ;
static int orientS ;

addpad( padnum, padname, orient ) 
int padnum, orient ;
char *padname ;
{
    padCounterS++ ;
    /* save name for output */
    padnameS[padCounterS] = padname ;
    padnumS = padnum ;
    side_spaceS = -1.0 ;
    orientS = orient ;
}

padside( side ) 
int side ;
{
    switch( side ){
	case PADB:
	case PADL:
	case PADR:
	case PADT:
	    fprintf( fpoG, "\npad %d name %s\n", padnumS, 
		padnameS[padCounterS] ) ;
	    if( side != pad_sideS ){
		space_counterS = 0.0 ;
		start_sideS[side] = padCounterS ;
	    }
	    pad_sideS = side ;
	    end_sideS[side] = padCounterS ;
	break ;
	case PMUL:
	case PMUR:
	case PMLL:
	case PMLR:
	case PML:
	case PMR:
	case PMTT:
	case PMBB:
	    fprintf( fpoG, "\nhardcell %d name %s\n", padnumS, 
		padnameS[padCounterS] ) ;
	    pad_sideS = side ;
	break ;
	default:
	M(ERRMSG, NULL, "Invalid side for macro.\n" ) ;
    }
}

process_side_space() 
{
}


add_bbox( left, right, bottom, top )
int left, right, bottom, top ;
{
    int orient ;

    fprintf( fpoG, "corners 4 " ) ;
    /* calculate effective orientation to pad B */
    orient = transTableS[pad_sideS][orientS] ;
    translate( &left, &bottom, &right, &top, orient ) ;
    fprintf( fpoG, "%d %d ", left, bottom ) ;
    fprintf( fpoG, "%d %d ", left, top ) ;
    fprintf( fpoG, "%d %d ", right, top ) ;
    fprintf( fpoG, "%d %d\n", right, bottom ) ;
    switch( pad_sideS ){
	case PADB:fprintf( fpoG, "restrict side B\n" ) ;
	break ;
	case PADL:fprintf( fpoG, "restrict side L\n" ) ;
	break ;
	case PADR:fprintf( fpoG, "restrict side R\n" ) ;
	break ;
	case PADT:fprintf( fpoG, "restrict side T\n" ) ;
	break ;
	case PMUL:
	case PMUR:
	case PMLL:
	case PMLR:
	case PML:
	case PMR:
	case PMTT:
	case PMBB:
	    fprintf( fpoG, "class 0 8 orientations 0 1 2 3 4 5 6 7\n" );
    }
}

add_space( space ) 
DOUBLE space ;
{
    side_spaceS = space ;
    if( side_spaceS >= 0.0 ){
	fprintf( fpoG, "sidespace %f\n", space ) ;
    }
}

process_pad_groups()
{
    int i, j ;

    for( i = 1; i<= 4; i++ ){
	if( start_sideS[i] == 0 ){
	    /* no pads on this side */
	    continue ;
	}
	fprintf( fpoG, "\npadgroup padgroup%d nopermute\n", i ) ;
	for( j = start_sideS[i]; j<= end_sideS[i]; j++ ){
	    fprintf( fpoG, "%s fixed\n", padnameS[j] ) ;
	}
	switch( i ){
	    case PADB:fprintf( fpoG, "restrict side B\n" ) ;
	    break ;
	    case PADL:fprintf( fpoG, "restrict side L\n" ) ;
	    break ;
	    case PADR:fprintf( fpoG, "restrict side R\n" ) ;
	    break ;
	    case PADT:fprintf( fpoG, "restrict side T\n" ) ;
	    break ;
	}
    }
} /* end process_pad_groups */

calc_orientation( x, y )
int *x, *y ;
{
    int orient ;

    orient = transTableS[pad_sideS][orientS] ;
    rel_pos( orient, x, y, *x, *y, 0, 0 ) ;
}

readcells( fp ) 
FILE *fp ;
{
    char buffer[LRECL], *bufferptr ;
    char **tokens ;     /* for parsing menu file */
    char copyBuf[LRECL] ;
    int  numtokens ;
    int  delta ;
    int  error ;

    lineG = 0 ;
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	lineG ++ ; /* increment line number */

	/* make copy because scanner adds EOS characters */
	strcpy( copyBuf, bufferptr ) ;
	tokens = Ystrparser( bufferptr, " \t\n", &numtokens );

	if( numtokens == 0 ){
	    /* skip over blank lines */
	    fprintf( fpoG, "\n" ) ;
	    continue ;
	}


	/* echo input until we reach a pad */
	if( strcmp( tokens[0], PADKEYWORD ) == STRINGEQ){
	    break ;
	}

	/* now we try to add layer information if not given */
	/* look for signal */
	if( check_for_layer( tokens, numtokens ) ){
	    continue ;
	}

	fprintf( fpoG, "%s", copyBuf ) ;

    } /* end while loop */

    /* now we need to unread the last line */
    delta = - strlen(copyBuf) ;
    error = fseek( fp, delta, RELATIVE_TO_CURPOS ) ;

    if( error == -1 ){
	M( ERRMSG, "readcells", "can't jump backwards in file\n" ) ;
    }
    convert( fp ) ;
} /* end readcells */

BOOL check_for_layer( tokens, numtokens )
char **tokens ;
INT numtokens ;
{
    int  i ;
    int  j ;
    int  lay_tok ;

    for( i = 0; i < numtokens; i++ ){
	if( strcmp( tokens[i], SIGNAL ) == STRINGEQ ){
	    lay_tok = i + 2 ;
	} else if( strcmp( tokens[i], EQUIV ) == STRINGEQ ) {
	    lay_tok = i + 3 ;
	} else {
	    lay_tok = 0 ;
	}
	if( lay_tok ) {
	    if( lay_tok < numtokens ){
		if( strcmp( tokens[lay_tok], LAYER ) != STRINGEQ ){
		    /* we have valid syntax lets fix it up */
		    for( j = 0 ; j < lay_tok ; j++ ){
			/* output up to and include signal name */
			fprintf( fpoG, "%s ", tokens[j] ) ;
		    }
		    /* now add the layer */
		    fprintf( fpoG, "layer 1 " ) ;
		    /* now finish up */
		    for( j = lay_tok ; j < numtokens ; j++ ){
			/* output to end */
			fprintf( fpoG, "%s ", tokens[j] ) ;
		    }
		    /* output and end of line */
		    fprintf( fpoG, "\n" ) ;
		    /* no need to echo below */
		    return( TRUE ) ;
		}
	    }
	}
    }
    return( FALSE ) ;
} /* end check_for_layer */

/* ***************************************************************** 
translate rotation of cells
----------------------------------------------------------------- */
translate(l,b,r,t, orient) 
int *l, *b, *r, *t, orient ;
{

int temp ;

switch( orient ){
    case 0 :
	    break ;
    case 1 :
	    /* mirror around x - axis */
	    temp   = *t ;
	    *t = - *b ;
	    *b = - temp   ;
	    break ;
    case 2 :
	    /* mirror around y - axis */
	    temp   = *r ;
	    *r = - *l ;
	    *l = - temp   ;
	    break ;
    case 3 :
	    /* rotate 180 */
	    temp   = *t ;
	    *t = - *b ;
	    *b = - temp   ;
	    temp   = *r ;
	    *r = - *l ;
	    *l = - temp   ;
	    break ;
    case 4 :
	    /* mirror x and then rot 90 */
	    temp   = *l ;
	    *l = - *t ;
	    *t = - temp   ;
	    temp   = *r ;
	    *r = - *b ;
	    *b = - temp   ;
	    break ;
    case 5 :
	    /* mirror x and then rot -90 */
	    temp   = *r ;
	    *r = *t ;
	    *t = temp   ;
	    temp   = *l ;
	    *l = *b ;
	    *b = temp   ;
	    break ;
    case 6 :
	    /* rotate 90 degrees */
	    temp   = *l ;
	    *l = - *t ;
	    *t = *r ;
	    *r = - *b ;
	    *b = temp   ;
	    break ;
    case 7 :
	    /* rotate - 90 degrees */
	    temp   = *t ;
	    *t = - *l ;
	    *l = *b ;
	    *b = - *r ;
	    *r = temp   ;
	    break ;
} /* end switch */
} /* end function translate */


/* ----------------------------------------------------------------- 
    rel_pos - takes a cell orientation, cell pin position, and
    cell center and returns global postion.  
----------------------------------------------------------------- */
rel_pos( orient_R, globalX_R, globalY_R, relativeX_R, relativeY_R,         
    cellX_R, cellY_R )               
int orient_R, *globalX_R, *globalY_R, relativeX_R, relativeY_R,         
    cellX_R, cellY_R ;               
{
    switch(orient_R){
	case 0: *globalX_R = cellX_R + relativeX_R ;
		*globalY_R = cellY_R + relativeY_R ;
		break ;
	case 1: *globalX_R = cellX_R + relativeX_R ;
		*globalY_R = cellY_R - relativeY_R ;
		break ;
	case 2: *globalX_R = cellX_R - relativeX_R ;
		*globalY_R = cellY_R + relativeY_R ;
		break ;
	case 3: *globalX_R = cellX_R - relativeX_R ;
		*globalY_R = cellY_R - relativeY_R ;
		break ;
	case 4: *globalX_R = cellX_R - relativeY_R ;
		*globalY_R = cellY_R - relativeX_R ;
		break ;
	case 5: *globalX_R = cellX_R + relativeY_R ;
		*globalY_R = cellY_R + relativeX_R ;
		break ;
	case 6: *globalX_R = cellX_R - relativeY_R ;
		*globalY_R = cellY_R + relativeX_R ;
		break ;
	case 7: *globalX_R = cellX_R + relativeY_R ;
		*globalY_R = cellY_R - relativeX_R ;
		break ;
    }
} /* end rel_pos */
