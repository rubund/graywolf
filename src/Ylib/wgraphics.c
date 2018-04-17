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
FILE:	    wgraphics.c                                       
DESCRIPTION:graphic drawing routines which only write files.
    This is a derivative of draw.c except that it does not need to 
    link with the X11 library.  If you are just writing binary files
    for later use, please use this set of routines.  Make sure to 
    use <yalecad/wgraphics.h> instead of <yalecad/draw.h>
    ********************************************************
CONTENTS:   TWinitWGraphics(desiredColors,numC,dataDir)
		char **desiredColors, *datDir ;
		INT  numC ;
	    TWcloseWGraphics()
	    TWstartWFrame()
	    TWflushWFrame()
	    TWsetWFrame( number )
		INT number ;
	    drawWPin( ref_num, x1,y1,x2,y2,color,label)
	    drawWLine( ref_num,x1,y1,x2,y2,color,label)
	    drawWRect( ref_num, x1,y1,x2,y2,color,label)
		INT     ref_num ;
		INT	x1,y1,x2,y2, color;
		char	*label;
DATE:	    Mar 21, 1989 - began as derivative of draw.c
REVISIONS:  Thu Jan 24 20:20:32 PST 1991 - added missing return.
	    Mon May  6 20:35:28 EDT 1991 - no longer support pin file.
	    Mon Aug 12 15:57:17 CDT 1991 - fixed problem with routine
		names.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) wgraphics.c version 3.8 8/12/91" ;
#endif

#include <stdio.h>
#include <string.h>

#define  TWDRAWCODE
#include <yalecad/base.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/debug.h>
#include <yalecad/dbinary.h>
#include <yalecad/wgraphics.h>
#include <yalecad/string.h>

#define NODIRECTORY     1
#define NOINIT          2

/********** STATIC DEFINITIONS FOR WRITE FUNCTIONS *************/
static  char *dirNameS ;      /* pathname including DATA directory */
static  BOOL initS = FALSE ;  /* tells whether initialization performed */
static  INT  frameCountS ;    /* current number of display frames */
static  BOOL frameOpenS ;     /* tells whether frame files are open */
static  FILE *cellFileS = NULL ; /* cellfile pointer */   
static  FILE *netFileS = NULL ;  /* net file pointer */
static  FILE *symbFileS = NULL ; /* symbfile pointer */
static  INT  numCellS = 0 ; /* cell counter */
static  INT  numNetS = 0 ;  /* net counter */
static  INT  numCharS = 0 ; /* symbol table counter */

BOOL TWinitWGraphics( numC, desiredColors)
INT  numC ;
char **desiredColors ;
{

    char *Yfixpath() ;
    char *Ygetenv() ;

    if(!(dirNameS = Ygetenv( "DATADIR" ))){
	/* find fullpathname of data directory */
	dirNameS = Ystrclone( Yfixpath( "./DATA", FALSE )) ;
    }

    /* first look for data directory */
    if( !(YdirectoryExists(dirNameS)) ){
	sprintf( YmsgG,"Can't find data directory:%s\n", dirNameS );
	M(ERRMSG,"TWinitGraphics",YmsgG ) ;
	YexitPgm( NODIRECTORY ) ;
    }

    /* set count to zero */
    frameCountS = 0 ;

    frameOpenS = FALSE ;

    /* initialization has been done sucessfully */
    initS = TRUE ;

    return( initS ) ;

} /* end function TWinitWGraphics */


TWcloseWGraphics()
{

    if(!(initS )){
	fprintf(stderr,"ERROR[closeGraphics]:initialization was not" ) ;
	fprintf(stderr,"performed\n  before calling closeGraphics\n" ) ;
	YexitPgm( NOINIT ) ;
    }

    /* check to see if other files are open */
    if( frameOpenS ){
	TWflushWFrame() ;
    }

} /* end TWcloseGraphics */

TWstartWFrame()
{
    char filename[LRECL] ;
    char dummy[5] ;
    UNSIGNED_INT nitems ;

    if(!(initS )){
	fprintf(stderr,"ERROR[startNewFrame]:initialization was not" ) ;
	fprintf(stderr,"performed\n  before calling startNewFrame\n" ) ;
	YexitPgm( NOINIT ) ;
    }

    if( frameOpenS ){
	TWflushWFrame() ;
    }
    /* signal that frame is now open */
    frameOpenS = TRUE ;

    /* now open next frame files */
    /* increment frame count */
    frameCountS++ ;

    /* first cell file */
    sprintf( filename, "%s/cell.bin.%d", dirNameS, frameCountS ) ;
    cellFileS = TWOPEN( filename, "w", ABORT ) ; 
    numCellS = 0 ; /* reset cell counter */

    /* next net file */
    sprintf( filename, "%s/net.bin.%d", dirNameS, frameCountS ) ;
    netFileS = TWOPEN( filename, "w", ABORT ) ; 
    numNetS = 0 ; /* reset net counter */

    /* next symb file */
    sprintf( filename, "%s/symb.bin.%d", dirNameS, frameCountS ) ;
    symbFileS = TWOPEN( filename, "w", ABORT ) ; 
    /* write a dummy character at start file makes test for label */
    /* index easier since symbtable[0] is meaningless now. */
    /* All indexes into symbol table which are valid must be positive */
    dummy[0] = EOS ;
    nitems = (UNSIGNED_INT) 1 ;
    fwrite( dummy, sizeof(char), nitems, symbFileS ) ;
    numCharS = 1 ; /* reset symbol table counter */

} /* end startWFrame */

/* write size of data at end of files and close them if frames are open */
TWflushWFrame()
{
    char dummy[5] ;
    UNSIGNED_INT nitems ;
    INT numw ;
    INT excess ;

    /* check to see if other files are open */
    if( frameOpenS ){
	nitems = (UNSIGNED_INT) 1 ;
	/* terminate the file with number of records in each file */
	numw = fwrite( &numCellS,sizeof(UNSIGNED_INT),nitems,cellFileS ) ;
	ASSERT( numw == 1, "startNewFrame", "Number written zero" ) ;
	numw = fwrite( &numNetS, sizeof(UNSIGNED_INT),nitems,netFileS ) ;
	ASSERT( numw == 1, "startNewFrame", "Number written zero" ) ;
	/* need to put on integer boundary */
	if( excess = numCharS % 4 ){
	    /* pad the remainder with dummy */
	    nitems = (UNSIGNED_INT) (4 - excess ) ;
	    numw = fwrite( dummy, sizeof(char), nitems, symbFileS ) ;
	    ASSERT( numw == nitems,"startNewFrame","Number written zero");
	}
	nitems = (UNSIGNED_INT) 1 ;
	numw = fwrite( &numCharS,sizeof(UNSIGNED_INT),nitems,symbFileS ) ;
	ASSERT( numw == 1, "startNewFrame", "Number written zero" ) ;

	/* files are open close them */
	ASSERT( cellFileS, "startNewFrame", "cell file should be open" ) ;
	TWCLOSE( cellFileS ) ;
	ASSERT( netFileS, "startNewFrame", "net file should be open" ) ;
	TWCLOSE( netFileS ) ;
	ASSERT( symbFileS, "startNewFrame", "symb file should be open" ) ;
	TWCLOSE( symbFileS ) ;
	/* signal that frame has been closed */
	frameOpenS = FALSE ;
    }

} /* TWflushWFrame */

TWsetWFrame( number )
INT number ;
{
    char fileName[LRECL] ;

    if( number <= 0 ){
	/* search for the first valid file */
	/* find max number of frames of data */ 
	for( frameCountS=1;;frameCountS++ ){

	    sprintf( fileName,"%s/cell.bin.%d",dirNameS,frameCountS ) ;
	    if(! (YfileExists(fileName) )){
		/* last file successfully read is one less */
		frameCountS-- ; 
		break ;
	    }
	}
    } else {
	frameCountS = number ;
    }
} /* end TWsetWFrame */

/* *********  GENERIC WRITE ROUTINES **************  */
/* draw a rectangle whose diagonals are (x1,y1) and (x2,y2) */
/* 	if the specified color is default or invalid, use default color */
TWdrawWPin( ref_num, x1,y1,x2,y2,color,label)
INT     ref_num ; /* reference number */
INT	x1,y1,x2,y2, color;
char	*label;
{
    TWdrawWRect( ref_num, x1,y1,x2,y2,color,label) ;
} /* end drawWPin */

/* draw a one pixel tall line segment from x1,y1 to x2,y2 */
TWdrawWLine( ref_num,x1,y1,x2,y2,color,label)
INT     ref_num ; /* reference number */
INT	x1,y1,x2,y2,color ;
char	*label;
{	
    DATABOX record ;
    UNSIGNED_INT nitems ;
    INT numw ; /* number written */

    /* fill up data record  file destination net file */
    record.ref = (UNSIGNED_INT) ref_num ;
    record.x1 = x1 ;
    record.x2 = x2 ;
    record.y1 = y1 ;
    record.y2 = y2 ;
    /* now take care of color */
    record.color = color ;
    /* now store string in symbol table if given */
    if( label ){
	/* write string to symbol table file */
	nitems = (UNSIGNED_INT) ( strlen( label ) + 1 ) ;
	numw = fwrite( label, sizeof(char), nitems, symbFileS ) ;
	ASSERT( numw == nitems, "drawLine", 
	    "Couldnt write to string table" );
	/* now store in net file offset in table */
	record.label = numCharS ;
	/* now update offset to include this string */
	numCharS += (INT) nitems ;
	
    } else {
	record.label = 0 ;
    }
    /* now write record */
    nitems = (UNSIGNED_INT) 1 ;
    numw = fwrite( &record, sizeof(DATABOX),nitems,netFileS ) ;
    ASSERT( numw == 1, "drawLine", "Record not written..." ) ;
    numNetS++ ;

} /* end drawWLine */

/* draw a rectangle whose diagonals are (x1,y1) and (x2,y2) */
/* 	if the specified color is default or invalid, use default color */
TWdrawWRect( ref_num, x1,y1,x2,y2,color,label)
INT     ref_num ; /* reference number */
INT	x1,y1,x2,y2, color;
char	*label;
{
    DATABOX record ;
    UNSIGNED_INT nitems ;
    INT numw ; /* number of records written */

    /* fill up data record  file destination net file */
    record.ref = (UNSIGNED_INT) ref_num ;
    record.x1 = x1 ;
    record.x2 = x2 ;
    record.y1 = y1 ;
    record.y2 = y2 ;
    record.color = color ;
    /* now store string in symbol table if given */
    if( label ){
	/* write string to symbol table file */
	nitems = (UNSIGNED_INT) ( strlen( label ) + 1 ) ;
	numw = fwrite( label, sizeof(char), nitems, symbFileS ) ;
	ASSERT( numw == nitems, "drawWRect", 
	    "Couldn't write to string table" );
	/* now store in net file offset in table */
	record.label = numCharS ;
	/* now update offset to include this string */
	numCharS += (INT) nitems ;
	
    } else {
	record.label = 0 ;
    }
    /* now write record */
    nitems = (UNSIGNED_INT) 1 ;
    numw = fwrite( &record, sizeof(DATABOX),nitems,cellFileS ) ;
    numCellS++ ;
    ASSERT( numw == 1, "drawWRect", "Record not written..." ) ;

} /* end drawWRect */
/* ************************************************************** */
