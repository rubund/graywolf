/* ----------------------------------------------------------------- 
FILE:	    cif.c                                       
DESCRIPTION:Routines to build cif files.
CONTENTS:   YcifInit( ) 
	    YcifLine( x0, y0, x1, y1, color )
		int x0, y0, x1, y1 ;
		int color ;
	    YcifBox( x0, y0, x1, y1, color ) 
		int x0, y0, x1, y1 ;
		int color ;
	    YcifClose( ) 
DATE:	    Sep 21, 1989 - moved original to library.
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) cif.c (Yale) version 1.1 5/17/90" ;
#endif

#include <yalecad/base.h>
#include <yalecad/colors.h>
#include <yalecad/file.h>
#include <yalecad/message.h>

static FILE *cifFileS ;
static int oldcolorS ;
static BOOL cifS ;


static check_color();

YcifInit( design ) 
char *design ;
{
    char filename[LRECL] ;

    cifS = TRUE ;

    sprintf( filename, "%s.cif", design ) ;
    cifFileS = TWOPEN( filename,"w", ABORT ) ;

    oldcolorS = 0 ;

    /* write out design name */
    fprintf( cifFileS, "DS 1 1 1;\n" ) ;
    fprintf( cifFileS, "9 %s;\n", design ) ;
} /* end YcifInit */

YcifLine( x0, y0, x1, y1, color )
int x0, y0, x1, y1 ;
int color ;
{
    if( !cifS ){
	return ;
    }

    if( check_color( color ) ){
    
	/* now draw wire zero width */
	fprintf( cifFileS, "W 0 %d %d %d %d;\n", x0, y0, x1, y1 ) ;
    }
} /* end YcifLine */

YcifBox( x0, y0, x1, y1, color ) 
int x0, y0, x1, y1 ;
int color ;
{
    int length, width, xcenter, ycenter ;

    if( !cifS ){
	return ;
    }
    if( check_color( color ) ){

	length = ABS( x1 - x0 ) ;
	width =  ABS( y1 - y0 ) ;
	xcenter = (x0 + x1) / 2 ;
	ycenter = (y0 + y1) / 2 ;

	/* now draw box */
	fprintf( cifFileS, "B %d %d %d %d;\n",
	    length, width, xcenter, ycenter );
    }
} /* end YcifBox */

YcifClose( ) 
{
    if( !cifS ){
	return ;
    }
    /* close definition */
    fprintf( cifFileS, "DF;\n" ) ;

    /* call the definition */
    fprintf( cifFileS, "C 1;\n" ) ;

    /* the end */
    fprintf( cifFileS, "End\n" ) ;

    /* close the file */
    fclose( cifFileS );

    cifS = FALSE ;
	
} /* YcifClose */

static check_color( color )
int color ;
{
    if( color != oldcolorS ){/* minimize pen changes*/
	/* change color first */
	switch( color ){
	    case TWWHITE:
	    case TWBLACK:
		fprintf( cifFileS, "L NC;\n" ) ;
		break ;
	    case TWRED :
		fprintf( cifFileS, "L NP;\n" ) ;
		break ;
	    case TWBLUE:
		fprintf( cifFileS, "L NM;\n" ) ;
		break ;
	    case TWYELLOW:
	    case TWGREEN:
	    case TWVIOLET:
	    case TWORANGE:
	    case TWMEDAQUA:
		fprintf( cifFileS, "L ND;\n" ) ;
		break ;
	    case TWCYAN:
		fprintf( cifFileS, "L NM;\n" ) ;
		break ;

	    default:
		sprintf( YmsgG, "Unknown color %s\n",color ) ;
		M( ERRMSG, "cif_check_color", YmsgG ) ;
		return( FALSE ) ;
	}
	oldcolorS = color ;
    }
    return( TRUE ) ;
} /* end check_color */
