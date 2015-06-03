/*
 *   Copyright (C) 1990-1991 Yale University
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
FILE:	    findflow.c
DESCRIPTION:Determine the correct flow file for the TimberWolf system.
DATE:	    Aug 1, 1990 
REVISIONS:  Sun Jan 20 21:31:36 PST 1991 - ported to AIX.
	    Sat Feb 23 04:39:32 EST 1991 - added TOMUS case.
	    Thu Apr 18 01:18:32 EDT 1991 - now changed where to 
		look for the flow files.  User can override.
---------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) findflow.c version 1.4 4/18/91" ;
#endif

#include <stdio.h>

#include <globals.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>

#ifndef NOGRAPHICS
#include <yalecad/draw.h>
#include <yalecad/colors.h>
#endif

FILE *find_flow_file( general_mode, debug, filename )
BOOL general_mode, debug ;
char *filename ;
{

    INT type ;                     /* design type */
    INT find_design_type() ;
    char prefix[LRECL] ;
    char suffix[LRECL] ;
    FILE *fp ;
    
    /* **************************************************************
    *  Now look for twflow file.  First determine prefix based on
    *  whether it is TimberWolf mode or general mode.  If general
    *  mode the prefix is easy "user".  If the mode is the TimberWolf
    *  mode determine the design style using the syntax program.
    *  The syntax program will tell us the number of standard cells,
    *  macro cells etc.   Use file dependancies to check if it needs
    *  to be run.  The TimberWolf prefixes are:
    *     1. mixed
    *     2. standard
    *     3. macro
    *     4. tomus
    *  The suffix may be one of the following:
    *     1.  Debug.
    *     2.  No graphics.
    *     3.  Graphics and no debug.
    *  Next look for the flowfile in :
    *     1. specified flow directory. Exit if can't open.
    *     2. current directory.  Don't exit if doesn't exist.
    *     3. TimberWolf directory.
    * ************************************************************* */
    if( general_mode ){
	sprintf( prefix, "user" ) ;
    } else {
	type = find_design_type() ;
	switch( type ){
	    case MIXED:
		sprintf( prefix, "mixed" ) ;
		break ;
	    case STANDARD:
		sprintf( prefix, "standard" ) ;
		break ;
	    case MACRO:
		sprintf( prefix, "macro" ) ;
		break ;
	    case TOMUS:
		sprintf( prefix, "tomus" ) ;
		break ;
	    case UNKNOWN:
		M( ERRMSG, "find_flow_file", 
		"FATAL - Couldn't determine design type\n" ) ;
		YexitPgm( PGMFAIL ) ;
	}
    }
    if( debug ){
	sprintf( suffix, "fdbg" ) ;
    } else if ( !(graphicsG) ){ 
	sprintf( suffix, "fnog" ) ;
    } else {
	sprintf( suffix, "flow" ) ;
    }

    if( flow_dirG ){
	/* first try absolute path */
	sprintf( filename, "%s/%s.%s", flow_dirG, prefix, suffix ) ;
	if( fp = TWOPEN( filename, "r", NOABORT ) ){
	    return( fp ) ;
	}
	/* next try relative to TimberWolf root directory. */
	sprintf( filename, "%s/bin/flow/%s/%s.%s", twdirG, flow_dirG, prefix, suffix ) ;
	if( fp = TWOPEN( filename, "r", NOABORT ) ){
	    return( fp ) ;
	}
	/* if we get here we have trouble we must abort.  Write msg */
	M( ERRMSG, "find_flow_file", "Could not open either\n" ) ;
	sprintf( YmsgG, "\n\t%s\n", filename ) ;
	M( ERRMSG, NULL, YmsgG ) ;
	M( ERRMSG, NULL, "  or\n" ) ;
	sprintf( YmsgG, "\t%s/%s.%s\n", flow_dirG, prefix,suffix);
	M( ERRMSG, NULL, YmsgG ) ;
	show_flows() ;
	if( graphicsG ){
	    G( TWcloseGraphics() ) ;
	}
	YexitPgm(PGMFAIL) ;

    }
    sprintf( filename, "%s.%s", prefix, suffix ) ;
    fp = TWOPEN( filename, "r", NOABORT ) ;
    if( !(fp) ){
	sprintf( filename, "%s/bin/flow/flow/%s.%s",twdirG,prefix,suffix ) ;
	fp = TWOPEN( filename, "r", ABORT ) ;
    }
    return( fp ) ;
} /* end find_flow_file */

/* call syntax if necessary and then read result */
INT find_design_type()
{
    ADJBOX syntax_info ;
    FBOX infile ;
    FBOX outfile ;
    INT status ;
    INT numtokens ;
    INT num_stdcell ;
    INT num_macro ;
    INT mode ;
    FILE *fin ;
    char in_name[8] ;
    char out_name[8] ;
    char buffer[LRECL] ;
    char *bufferptr ;
    char **tokens ;

    /* this is an easy case */
    if( tomusG ){
	return( TOMUS ) ;
    }

    /* set up the information to call check_dependencies */
    sprintf( in_name, "$.cel" ) ;
    sprintf( out_name, "$.stat" ) ;
    infile.fname = in_name ;
    infile.next = NULL ;
    outfile.fname = out_name ;
    outfile.next = NULL ;
    syntax_info.ifiles = &infile ;
    syntax_info.ofiles = &outfile ;
    G( TWmessage( "Determining design type" ) ) ;
    if( !graphicsG ){
	M( MSG, "find_flow", "Determining design type\n" ) ;
    }
    if( check_dependencies( &syntax_info ) ){
	/* we need to execute the syntax program */
	sprintf( buffer, "%s/bin/syntax %s", twdirG, cktNameG ) ;

	/* now log the beginning time */
	Ylog_msg( "Syntax started..." ) ;
	G( TWstartFrame() ) ;
	G( TWmessage( "Calling syntax to determine design type" ) ) ;
	G( TWsetwindow( -165, -55, 165, 55 ) ) ;
	G( TWdrawCell( 0, -150, -50, 150, 50, TWRED, "syntax" ) ) ;
        G( TWflushFrame() ) ;

	/* now execute the command */
	status = system( buffer ) ;
	if( status ){
	    M( ERRMSG, "find_design_type", "Trouble executing syntax\n" ) ;
	    M( MSG, NULL, "FATAL - must exit\n\n" ) ;
	    YexitPgm( PGMFAIL ) ;
	}
	Ylog_msg( "Syntax completed..." ) ;
	G( TWmessage( "Syntax completed successfully..." ) ) ;
    }

    /***********************************************************
    * Read from circuitName.stat file to determine design style.
    ***********************************************************/
    sprintf( buffer, "%s.stat", cktNameG ) ;
    fin = TWOPEN( buffer,"r", ABORT ) ;

    while( bufferptr = fgets( buffer, LRECL, fin ) ){
	tokens = Ystrparser( bufferptr, ":\t\n", &numtokens ) ;
	if( numtokens != 2 ){
	    continue ;
	}
	if( strcmp( tokens[0], "num_stdcells" ) == STRINGEQ ){
	    num_stdcell = atoi( tokens[1] ) ;
	} else if( strcmp( tokens[0], "num_macros" ) == STRINGEQ ){
	    num_macro = atoi( tokens[1] ) ;
	}
    }
    TWCLOSE( fin ) ;

    /* now make decision based on the numbers */
    if( num_stdcell > 0 && num_macro > 0 ){
	mode = MIXED ;
    } else if( num_stdcell > 0 && num_macro == 0 ){
	mode = STANDARD ;
    } else if( num_stdcell == 0 && num_macro > 0 ){
	mode = MACRO ;
    } else {
	mode = UNKNOWN ;
    }
    D( "find_design_type", 
	fprintf( stderr, "Found design type %d\n", mode ) ;
    ) ;

    return( mode ) ;

} /* end find_design_type */
