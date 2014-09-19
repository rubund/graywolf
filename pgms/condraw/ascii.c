/* ----------------------------------------------------------------- 
FILE:	    ascii.c                                       
DESCRIPTION:Convert graphics file to ascii.
CONTENTS:   convert_to_ascii( cIn, cOut, nIn, nOut, pIn, pOut, symb ) ;
DATE:	    Jan 26, 1989 - 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) ascii.c version 1.4 10/14/90" ;
#endif

#include <stdio.h>
#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/draw.h>
#include <yalecad/colors.h>
#include <yalecad/dbinary.h>


#define FAILBINARY   1
#define FROMSTART    (int) 0
#define FROMCUR      (int) 1
#define FROMEND      (int) 2
#define FSEEKERROR   (int) -1

convert_to_ascii( cIn, cOut, nIn, nOut, symb )
FILE *cIn, *cOut, *nIn, *nOut, *symb ;
{
    char    **colorArray ;
    char    *symb_table ;
    int     numcolors ; /* number of colors in stdcolor array */
    int     offset ;
    int     color ;
    int     i ;
    int     num_char, num_cel_rec, num_net_rec, num_pin_rec ;
    DATAPTR cell_file_ptr, net_file_ptr, pin_file_ptr ;
    DATABOX ccell, pcell, ncell ;
    unsigned nitems ;
    char *label ;

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
	/* get correct color */
	color = ccell.color ;
	if( color > numcolors || color <= 0 ){
	    color = TWBLACK ;  /* default to black */
	}
	/* get label if present */
	if( ccell.label != 0 ){
	    /* label is offset into symbol table */
	    label = &( symb_table[ccell.label] ) ;
	    fprintf( cOut, "%d, %d, %d, %d, color: %s, label: %s\n",
		ccell.x1, ccell.y1, ccell.x2, ccell.y2, 
		colorArray[color], label ) ;
	} else {
	    fprintf( cOut, "%d, %d, %d, %d, color: %s\n",
		ccell.x1, ccell.y1, ccell.x2, ccell.y2, 
		colorArray[color] ) ;
	}
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
	/* get correct color */
	color = ncell.color ;
	if( color > numcolors || color <= 0 ){
	    color = TWBLACK ;  /* default to black */
	}
	/* get label if present */
	if( ncell.label != 0 ){
	    /* label is offset into symbol table */
	    label = &( symb_table[ncell.label] ) ;
	    fprintf( nOut, "%d, %d, %d, %d, %d, color: %s, label: %s\n",
		ncell.ref, ncell.x1, ncell.y1, ncell.x2, ncell.y2, 
		colorArray[color], label ) ;
	} else {
	    fprintf( nOut, "%d, %d, %d, %d, %d, color: %s\n",
		ncell.ref, ncell.x1, ncell.y1, ncell.x2, ncell.y2, 
		colorArray[color] ) ;
	}
    }

} /* end function convert_to_ascii */
