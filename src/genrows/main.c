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
FILE:	    main.c                                       
DESCRIPTION:This file contains the main control structure for
	    generating standard cell rows.
CONTENTS:   main()
DATE:	    Aug  9, 1989 - original coding. 
REVISIONS:  Feb  7, 1990 - took total_row_length out of procedure calls.
	    Mon Jan  7 18:27:31 CST 1991 - remove .undo and
		.redo files after execution. Automatically,
		save a restart file at end.
	    Sat Jan 26 16:28:34 PST 1991 - moved force rows to
		main routine.
	    Sat Feb 23 00:35:50 EST 1991 - added no macro move flag.
	    Thu Mar  7 03:12:20 EST 1991 - now allow user to set
		number of rows in standard cell case.
	    Wed Apr 17 22:40:01 EDT 1991 - added conditional compile
		for debugging X.
	    Sat Sep 21 15:48:12 EDT 1991 - added memory functionality.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) main.c (Yale) version 3.10 9/21/91" ;
#endif

#define GLOBAL_DEFS
#define EXPECTEDMEMORY ( 512 * 1024 )
#define VERSION        "v1.0"

#include <stdio.h>
#include <yalecad/cleanup.h>
#include <yalecad/message.h>
#include <yalecad/file.h>
#include <yalecad/string.h>
#include <yalecad/debug.h>
#include <globals.h>

/* NOTE ****** macros cannot overlap !!! ****** */


/* NOTE: time efficiencies have been completely ignored during
	 the development of these algorithms. At this point,
	 I am only interested in EFFECTIVE and ROBUST 
	 functionality. ---Carl Sechen  */




main( argc, argv )
int  argc ;
char *argv[] ;
{

    FILE *fp ;
    int yaleIntro() ;
    char filename[LRECL] ; /* used for input filename */
    char *ptr ;     /* used to parse command line */
    int  windowId ; /* window id */
    BOOL parasite ; /* true if window inheritance is on */
    BOOL debug ;    /* true if debug is requested */
    BOOL force_tiles() ; /* returns true if last tile is forced */

    /* ********************** start initialization *********************** */
#ifdef DEBUGX
    extern int _Xdebug ;
    _Xdebug = TRUE ;
#endif

    /* start up cleanup handler */
    YINITCLEANUP( "config", NULL, MAYBEDUMP ) ;

    Yinit_memsize( EXPECTEDMEMORY ) ;

    if( argc < 2 || argc > 4 ){
	syntax() ;
    } else {
	debug       = FALSE ;
	parasite    = FALSE ;
	windowId    = 0 ;
	noMacroMoveG = FALSE ;
#ifndef NOGRAPHICS
	graphicsG = TRUE ;
#else
	graphicsG = FALSE ;
#endif
	if( *argv[1] == '-' ){
	    for( ptr = ++argv[1]; *ptr; ptr++ ){
		switch( *ptr ){
		case 'd':
		    debug = TRUE ;
		    YsetDebug( TRUE ) ;
		    break ;
		case 'f':
		    noMacroMoveG = TRUE ;
		    break ;
		case 'n':
		    graphicsG = FALSE ;
		    break ;
		case 'w':
		    parasite = TRUE ;
		    break ;
		default:
		    sprintf( YmsgG,"Unknown option:%c\n", *ptr ) ;
		    M(ERRMSG,"main",YmsgG);
		    syntax() ;
		}
	    }
	    YdebugMemory( debug ) ;

	    /* handle I/O requests */
	    cktNameG = Ystrclone( argv[2] );

	    YinitProgram( "config", VERSION, yaleIntro );

	    /* now tell the user what he picked */
	    M(MSG,NULL,"\n\nconfig switches:\n" ) ;
	    if( debug ){
		YsetDebug( TRUE ) ;
		M(MSG,NULL,"\tdebug on\n" ) ;
	    } 
	    if( graphicsG ){
		M(MSG,NULL,"\tGraphics mode on\n" ) ;
	    } else {
		M(MSG,NULL,"\tGraphics mode off\n" ) ;
	    }
	    if( parasite ){
		M(MSG,NULL,"\tconfig will inherit window\n" ) ;
		/* look for windowid */
		if( argc != 4 ){
		    M(ERRMSG,"main","Need to specify windowID\n" ) ;
		    syntax() ;
		} else {
		    windowId = atoi( argv[3] ) ;
		} 
	    }
	    if( noMacroMoveG ){
		M(MSG,NULL,"\tconfig will not alloc macro moves\n" ) ;
	    }
	    M(MSG,NULL,"\n" ) ;
	} else if( argc == 2 ){
	    /* order is important here */
	    YdebugMemory( FALSE ) ;
	    cktNameG = Ystrclone( argv[1] );

	    YinitProgram( "config", VERSION, yaleIntro );

	} else {
	    syntax() ;
	}
    }
    init_data_structures() ;
    G( initgraphics( argc, argv, windowId ) ) ;
    /* ********************** end initialization ************************* */

    sprintf( filename, "%s.mver", cktNameG ) ;
    fp = TWOPEN( filename, "r", ABORT ) ;

    /* actual_row_heightS is the default used for rows placed across the 
       top of the core--these are NOT generated from a tile */
    read_vertices(fp,TRUE) ;
    TWCLOSE(fp);

    /* now safe to read parameter file */
    readpar() ;

    build_macros() ;

    process_vertices() ;
    D( "genrows/main", print_vertices() ) ;

    process_tiles() ;

    D( "genrows/main", print_tiles() ) ;

    merge_tiles() ;

    D( "genrows/main", print_tiles() ) ;
    
    check_tiles() ;

    /* *************************************************************** */
    /* THE USER SHOULD BE ABLE TO DO:
	(1) Be able to divide a tile into two parts, separated by a
	    horizontal line -- when the rows above should have a 
	    higher height, for example.
	    To divide a tile, simply call the function: 
		    divide_tile( tile , horiz_line ) 
		    TILE_BOX *tile ;
		    int horiz_line ;

	(2) Be able to expand a tile in any direction.
	(3) Be able to edit the actual_row_height.
	(4) Be able to edit the channel_separation.
		*BUT,THE check_tiles MUST BE CONDUCTED IF THE USER 
		CHANGES ANYTHING IN (3) OR (4)
	(5) Be able to edit the min_length

    /* *************************************************************** */
    /* make the rows once to see where the tiles are */
    /* force the last tile with rows to line up */
    remakerows() ;

    if( num_rowsG && num_macrosG == 0 ){
	memoryG = FALSE ;
	calculate_numrows() ;
	memoryG = TRUE ;
    }

    if( force_tiles() ){
	remakerows() ;
    }

    if( graphicsG ){

	G( process_graphics() ) ;
	    
    } /* end graphics switch */

    if( graphicsG ){
	G( TWcloseGraphics() ) ;
    }
    print_blk_file()  ;

    /* now save a restart file */
    sprintf( filename, "%s.gsav", cktNameG ) ;
    fp = TWOPEN( filename, "w", ABORT ) ;
    save_state(fp) ;
    TWCLOSE( fp ) ;

    /* now blow away do and undo files if they exist */
    sprintf( filename, "%s.undo", cktNameG ) ;
    if( YfileExists( filename) ){
	Yrm_files( filename ) ;
    }
    sprintf( filename, "%s.redo", cktNameG ) ;
    if( YfileExists( filename) ){
	Yrm_files( filename ) ;
    }

    YexitPgm( PGMOK ) ;

} /* end main */


yaleIntro() 
{

    fprintf(stdout,"\n%s\n",YmsgG) ;
    fprintf(stdout,"Row configuration program\n");
    fprintf(stdout,"    Yale University\n");

} /* end yaleIntro */

/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, 
       "\nconfig [-dnw] designName [windowId] \n" ) ;
   M(MSG,NULL,YmsgG ) ; 
   M(MSG,NULL,"\twhose options are zero or more of the following:\n");
   M(MSG,NULL,"\t\td - prints debug info and performs extensive\n");
   M(MSG,NULL,"\t\t    error checking\n");
   M(MSG,NULL,"\t\tn - no graphics - the default is to open the\n");
   M(MSG,NULL,"\t\t    display and output graphics to an Xwindow\n");
   M(MSG,NULL,"\t\tw - parasite mode - user must specify windowId\n");
   YexitPgm(PGMFAIL);
} /* end syntax */
