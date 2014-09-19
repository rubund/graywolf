/* ----------------------------------------------------------------- 
FILE:	    ascii.c                                       
DESCRIPTION:Convert graphics file to ascii.
CONTENTS:   convert_to_ascii( cIn, cOut, nIn, nOut, pIn, pOut, symb ) ;
DATE:	    Jan 26, 1989 - 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) mif.c version 1.4 5/8/92" ;
#endif

#include <stdio.h>
#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/draw.h>
#include <yalecad/colors.h>
#include <yalecad/dbinary.h>
#include <yalecad/message.h>
#include <yalecad/file.h>
#include <yalecad/string.h>


#define FAILBINARY   1
#define FROMSTART    (int) 0
#define FROMCUR      (int) 1
#define FROMEND      (int) 2
#define FSEEKERROR   (int) -1
#define PAGE_WIDTH   7.0
#define PAGE_HEIGHT  9.0
#define PAGE_OFFSET  1.0
#define GROUPINIT    10

/* ***************** STATIC FUNCTION DEFINITIONS ******************* */
static INT TW2mif_color( P2(INT color, INT *fill_pat ) ) ;
static output_font( P1(FILE *fp) ) ;

static INT num_lineS ;
static char **font_infoS ;

convert_to_mif( cIn, nIn, symb, mif, fill )
FILE *cIn, *nIn, *symb, *mif ;
BOOL fill ;
{
    char    **colorArray ;
    char    *symb_table ;
    int     numcolors ; /* number of colors in stdcolor array */
    int     offset ;
    int     color ;
    int     i ;
    int     fill_pat ;
    int     num_char, num_cel_rec, num_net_rec, num_pin_rec ;
    int     minx, miny, maxx, maxy ;
    int     oldcolor ;
    int     groupCount ;
    DOUBLE  x1, x2, y1, y2, scale, width, height ;
    DATAPTR cell_file_ptr, net_file_ptr, pin_file_ptr ;
    DATABOX ccell, pcell, ncell ;
    unsigned nitems ;
    char *label, *bufferptr ;
    char *twdir, *Ygetenv() ;
    char buffer[LRECL] ;
    BOOL first ;
    FILE *fin ;


    /* -----------------------------------------------------------------
	First load preample.
    ----------------------------------------------------------------- */
    if( twdir = Ygetenv( "TWDIR" ) ){
	sprintf( buffer, "%s/bin/condraw.info/preample.mif", twdir ) ;
    } else {
	M( ERRMSG, "convert_to_mif", 
	"Couldn't get TWDIR environment variable.  Must abort.\n" ) ;
	YexitPgm( PGMFAIL ) ;
    }

    fin = TWOPEN( buffer, "r", ABORT ) ;
    while( bufferptr = fgets( buffer, LRECL, fin ) ){
	fprintf( mif, "%s", bufferptr ) ;
    }
    TWCLOSE( fin ) ;

    /* -----------------------------------------------------------------
	Read string font.
    ----------------------------------------------------------------- */
    sprintf( buffer, "%s/bin/condraw.info/font.mif", twdir ) ;

    fin = TWOPEN( buffer, "r", ABORT ) ;
    num_lineS = 0 ;
    while( bufferptr = fgets( buffer, LRECL, fin ) ){
	num_lineS++ ;
    }
    /* allocate space */
    font_infoS = YVECTOR_MALLOC( 1, num_lineS, char * ) ;

    rewind( fin ) ;
    num_lineS = 0 ;
    while( bufferptr = fgets( buffer, LRECL, fin ) ){
	font_infoS[++num_lineS] = Ystrclone( bufferptr ) ;
    }
    TWCLOSE( fin ) ;

    /* -----------------------------------------------------------------
	Read files for scaling factors.
    ----------------------------------------------------------------- */
    minx = INT_MAX ;
    miny = INT_MAX ;
    maxx = INT_MIN ;
    maxy = INT_MIN ;

    /* first get colorArray */
    colorArray = TWstdcolors() ;
    numcolors =  TWnumcolors() ;

    /* READ STRING TABLE FIRST */
    /* find size of file stored at end of file */
    offset = - 4 ;  /* four bytes from end */
    if( fseek(symb, offset, FROMEND) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to end of binary file\n");
	YexitPgm(FAILBINARY) ;
    }
    nitems = (unsigned) 1 ;
    fread( &num_char, sizeof(unsigned int), nitems, symb ) ;
    ASSERT( num_char, "convert_to_ascii","number of records zero.");
    symb_table = (char *) Ysafe_malloc((num_char+1) * sizeof (char) ) ;

    /* now read from beginning */
    offset = 0 ; 
    if( fseek(symb, offset, FROMSTART) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to beginning of binary file\n");
	YexitPgm(FAILBINARY) ;
    }
    fread( symb_table, sizeof(char), num_char, symb ) ;
    /* END READ CELL FILE */

    /* READ CELL FILE FIRST */
    /* find size of file stored at end of file */
    offset = - 4 ;  /* four bytes from end */
    if( fseek(cIn, offset, FROMEND) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to end of binary file\n");
	YexitPgm(FAILBINARY) ;
    }
    nitems = (unsigned) 1 ;
    fread( &num_cel_rec, sizeof(unsigned int), nitems, cIn ) ;
    ASSERT( num_cel_rec, "convert_to_ascii","number of records zero.");
    cell_file_ptr = (DATAPTR) Ysafe_malloc((num_cel_rec)*sizeof(DATABOX)) ;

    /* now read from beginning */
    offset = 0 ; 
    if( fseek(cIn, offset, FROMSTART) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to beginning of binary file\n");
	YexitPgm(FAILBINARY) ;
    }
    fread( cell_file_ptr, sizeof(DATABOX), num_cel_rec, cIn ) ;
    for( i=0;i<num_cel_rec ; i++ ){
	/* for speed */
	ccell = cell_file_ptr[i] ;
	maxx = MAX( maxx, ccell.x1 ) ;
	maxx = MAX( maxx, ccell.x2 ) ;
	minx = MIN( minx, ccell.x1 ) ;
	minx = MIN( minx, ccell.x2 ) ;

	maxy = MAX( maxy, ccell.y1 ) ;
	maxy = MAX( maxy, ccell.y2 ) ;
	miny = MIN( miny, ccell.y1 ) ;
	miny = MIN( miny, ccell.y2 ) ;
    }

    /* READ NET FILE */
    /* find size of file stored at end of file */
    offset = - 4 ;  /* four bytes from end */
    if( fseek(nIn, offset, FROMEND) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to end of binary file\n");
	YexitPgm(FAILBINARY) ;
    }
    nitems = (unsigned) 1 ;
    fread( &num_net_rec, sizeof(unsigned int), nitems, nIn ) ;
    ASSERT( num_cel_rec, "convert_to_ascii","number of records zero.");
    net_file_ptr = (DATAPTR) 
	Ysafe_malloc( (num_net_rec)* sizeof(DATABOX) ) ;

    /* now read from beginning */
    offset = 0 ; 
    if( fseek(nIn, offset, FROMSTART) == FSEEKERROR){
	fprintf( stderr, "Couldn't move to beginning of binary file\n");
	YexitPgm(FAILBINARY) ;
    }
    fread( net_file_ptr, sizeof(DATABOX), num_net_rec, nIn ) ;
    for( i=0;i<num_net_rec ; i++ ){
	/* for speed */
	ncell = net_file_ptr[i] ;
	maxx = MAX( maxx, ncell.x1 ) ;
	maxx = MAX( maxx, ncell.x2 ) ;
	minx = MIN( minx, ncell.x1 ) ;
	minx = MIN( minx, ncell.x2 ) ;

	maxy = MAX( maxy, ncell.y1 ) ;
	maxy = MAX( maxy, ncell.y2 ) ;
	miny = MIN( miny, ncell.y1 ) ;
	miny = MIN( miny, ncell.y2 ) ;
    }
    /* -----------------------------------------------------------------
	Calculate scaling factors.
    ----------------------------------------------------------------- */
    scale = (DOUBLE) PAGE_WIDTH / (DOUBLE) (maxx - minx) ;


    /* -----------------------------------------------------------------
	Now output to the file.  First the rectangles filled.
    ----------------------------------------------------------------- */
    first = TRUE ;
    oldcolor = -1 ;
    groupCount = GROUPINIT ;
    for( i=0;i<num_cel_rec ; i++ ){
	/* for speed */
	ccell = cell_file_ptr[i] ;
	/* get correct color */
	color = ccell.color ;
	if( color > numcolors || color <= 0 ){
	    color = TWBLACK ;  /* default to black */
	}
	color = TW2mif_color( color, &fill_pat ) ;
	/* first add data offset to move relative to 0,0 */
	x1 = (DOUBLE) ccell.x1 - minx ;
	x2 = (DOUBLE) ccell.x2 - minx ;
	y1 = (DOUBLE) ccell.y1 - miny ;
	y2 = (DOUBLE) ccell.y2 - miny ;
	/* next scale coordinates to window */
	x1 = (DOUBLE) x1 * scale ;
	x2 = (DOUBLE) x2 * scale ;
	y1 = (DOUBLE) y1 * scale ;
	y2 = (DOUBLE) y2 * scale ;
	width = fabs(x2 - x1) ;
	height = fabs(y2 - y1) ;
	/* account for inversion of y axis */
	y1 = PAGE_HEIGHT - y1 ;
	y2 = PAGE_HEIGHT - y2 ;

	/* get label if present */
	if( ccell.label != 0 ){
	    /* label is offset into symbol table */
	    label = &( symb_table[ccell.label] ) ;
	} else {
	    label = NIL( char * ) ;
	} 
	/* output data */
	fprintf( mif, "  <Rectangle\n" ) ;
	if( first ){
	    fprintf( mif, "   <Pen 0>\n" ) ;
	    fprintf( mif, "   <Fill %d>\n", fill_pat ) ;
	    first = FALSE ;
	}
	if( fill ){
	    fprintf( mif, "   <Fill %d>\n", fill_pat ) ;
	    fprintf( mif, "   <GroupID %d>\n", ++groupCount ) ;
	}
	/* output color */
	if( color != oldcolor ){
	    fprintf( mif, "   <Separation %d>\n", color ) ;
	    oldcolor = color ;
	}
	fprintf( mif, "   <BRect %4.4f \" %4.4f \" %4.4f \" %4.4f \">\n",
		x1, y2, width, height ) ;
	fprintf( mif, "  > # end of Rectangle\n" ) ;

	if( fill ){
	    fprintf( mif, "  <Rectangle\n" ) ;
	    fprintf( mif, "   <Fill 15>\n" ) ;
	    fprintf( mif, "   <GroupID %d>\n", groupCount ) ;
	    /* output color */
	    color = TW2mif_color( TWBLACK, &fill_pat ) ;
	    if( color != oldcolor ){
		fprintf( mif, "   <Separation %d>\n", color ) ;
		oldcolor = color ;
	    }
	    fprintf( mif, "   <BRect %4.4f \" %4.4f \" %4.4f \" %4.4f \">\n",
		    x1, y2, width, height ) ;
	    fprintf( mif, "  > # end of Rectangle\n" ) ;
	    fprintf( mif, "  <Group\n" ) ;
	    fprintf( mif, "   <ID %d>\n", groupCount ) ;
	    fprintf( mif, "  > # end of Group\n" ) ;
	}

	if( label ){
	    if( x1 < minx || x1 > maxx || y1 < miny || y1 > maxy ){
		continue ;
	    }
	    fprintf( mif, "  <TextLine\n" ) ;
	    if( fill ){
		fprintf( mif, "   <Separation %d>\n", color ) ;
		oldcolor = color ;
	    }
	    fprintf( mif, "   <TLOrigin %4.4f \" %4.4f \">\n", 
		x1 + width / 2, y1 - height / 2 ) ;
	    fprintf( mif, "   <TLAlignment Left>\n" ) ;
	    fprintf( mif, "   <Angle 0>\n" ) ;
	    output_font( mif ) ;
	    fprintf( mif, "   <String %s>\n", label ) ;
	    fprintf( mif, "  > # end of TextLine\n" ) ;
	}
    }
    for( i=0;i<num_net_rec ; i++ ){
	/* for speed */
	ncell = net_file_ptr[i] ;
	/* get correct color */
	color = ncell.color ;
	if( color > numcolors || color <= 0 ){
	    color = TWBLACK ;  /* default to black */
	}
	color = TW2mif_color( color, &fill_pat ) ;
	/* first add data offset to move relative to 0,0 */
	x1 = (DOUBLE) ncell.x1 - minx ;
	x2 = (DOUBLE) ncell.x2 - minx ;
	y1 = (DOUBLE) ncell.y1 - miny ;
	y2 = (DOUBLE) ncell.y2 - miny ;
	/* next scale coordinates to window */
	x1 = (DOUBLE) x1 * scale ;
	x2 = (DOUBLE) x2 * scale ;
	y1 = (DOUBLE) y1 * scale ;
	y2 = (DOUBLE) y2 * scale ;
	/* account for inversion of y axis */
	y1 = PAGE_HEIGHT - y1 ;
	y2 = PAGE_HEIGHT - y2 ;
	/* get label if present */
	if( ncell.label != 0 ){
	    /* label is offset into symbol table */
	    label = &( symb_table[ncell.label] ) ;
	} else {
	    label = NIL( char * ) ;
	}
	/* output data */
	fprintf( mif, "  <PolyLine\n" ) ;
	if( first ){
	    fprintf( mif, "   <HeadCap Square>\n" ) ;
	    fprintf( mif, "   <TailCap Square>\n" ) ;
	    first = FALSE ;
	}
	/* output color */
	if( color != oldcolor ){
	    fprintf( mif, "   <Separation %d>\n", color ) ;
	    oldcolor = color ;
	}
	fprintf( mif, "   <NumPoints 2>\n" ) ;
	fprintf( mif, "   <Point %4.4f \" %4.4f \">\n", x1, y1 ) ;
	fprintf( mif, "   <Point %4.4f \" %4.4f \">\n", x2, y2 ) ;
	fprintf( mif, "  > # end of PolyLine\n" ) ;
    } /* end for( i=0;i<num_net_rec... */


    /* -----------------------------------------------------------------
	Finally load afterword.
    ----------------------------------------------------------------- */
    sprintf( buffer, "%s/bin/condraw.info/afterword.mif", twdir ) ;

    fin = TWOPEN( buffer, "r", ABORT ) ;
    while( bufferptr = fgets( buffer, LRECL, fin ) ){
	fprintf( mif, "%s", bufferptr ) ;
    }
    TWCLOSE( fin ) ;

} /* end function convert_to_mif */

static output_font( fp )
FILE *fp ;
{
    INT i ; 	/* counter */
    for( i = 1 ; i <= num_lineS; i++ ){
	fprintf( fp, "%s", font_infoS[i] ) ;
    }
} /* end output_font */

static INT TW2mif_color( color, fill_pat )
INT color ;
INT *fill_pat ;
{
    switch( color ){
    case TWWHITE :
	color = 1 ;
	*fill_pat = 15 ;
	break ;
    case TWBLACK :
	color = 0 ;
	*fill_pat = 0 ;
	break ;
    case TWRED :
	color = 2 ;
	*fill_pat = 1 ;
	break ;
    case TWBLUE :
	color = 4 ;
	*fill_pat = 2 ;
	break ;
    case TWYELLOW :
	color = 7 ;
	*fill_pat = 3 ;
	break ;
    case TWGREEN :
	color = 3 ;
	*fill_pat = 4 ;
	break ;
    case TWVIOLET :
	color = 6 ;
	*fill_pat = 5 ;
	break ;
    case TWORANGE :
	color = 7 ;
	*fill_pat = 6 ;
	break ;
    case TWMEDAQUA :
	color = 3 ;
	*fill_pat = 7 ;
	break ;
    case TWCYAN :
	color = 5 ;
	*fill_pat = 8 ;
	break ;
    case TWLIGHTGREEN :
	color = 3 ;
	*fill_pat = 9 ;
	break ;
    case TWLIGHTBLUE  :
	color = 4 ;
	color = 10 ;
	break ;
    case TWBROWN :
	color = 0 ;
	color = 11 ;
	break ;
    case TWLIGHTBROWN :
	color = 0 ;
	color = 12 ;
	break ;
    case TWNAVY:
	color = 4 ;
	color = 13 ;
	break ;
    case TWLIGHTVIOLET:
	color = 6 ;
	color = 14 ;
	break ;
    case TWKHAKI:
	color = 0 ;
	color = 14 ;
	break ;
    } /* end switch */
    return( color ) ;
} /* end TW2mif_color() */
