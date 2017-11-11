/*
 *   Copyright (C) 1989-1992 Yale University
 *   Copyright (C) 2013 Tim Edwards <tim@opencircuitdesign.com>
 *   Copyright (C) 2014 Ruben Undheim <ruben.undheim@gmail.com>
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
FILE:	    partition.c                                       
DESCRIPTION:code to support standard cell partitioner.
CONTENTS:   
DATE:	    Apr  9, 1989 
REVISIONS:  May 24, 1989 - added stdcell configuration code.
	    Jun 21, 1989 - modified final_conf and output partition
		to handle equiv pins correctly.
	    Jun 23, 1989 - added reset cell function and added
		reconfig capability.
	    Aug 20, 1989 - moved standard cell calculation to 
		child program genrows.
	    Sep 16, 1989 - fixed problem with unbust.  Multiple tile
		cells should use tile count as argument since tiles
		need to be seamed together.
	    Nov  4, 1989 - now read block file created by genrows in
		order to update the size of the core.
	    Apr 23, 1990 - modified instance record.
	    May 15, 1990 - changed .mstat parser.
	    Sun Dec 16 00:32:16 EST 1990 - use the library version
		of buster.
	    Fri Jan 18 18:29:54 PST 1991 - now use conservative
		approach to building .scel.
	    Wed Jan 30 14:13:36 EST 1991 - now left justify orientation
		rotations.
	    Sat Feb 23 00:28:28 EST 1991 - now handle wildcarding for
		reading and updated for TOMUS.
	    Thu Apr 18 01:39:27 EDT 1991 - now allow genrows to
		orient macros.
	    Wed May  1 19:22:03 EDT 1991 - MC now always call genrows
		during partitioning.
	    Wed Jun  5 16:30:30 CDT 1991 - eliminated unbust.
	    Mon Aug  5 18:06:17 CDT 1991 - eliminated special
		partitioning case.
	    Fri Oct 18 00:09:45 EDT 1991 - eliminated read_blk_file.
		Instead, to find core we read it using read_gen_file.
----------------------------------------------------------------- */
#include "allheaders.h"

#include "config-build.h"

#define ROWSEP    "rowSep"
#define GENROWPROG      "genrows"
#define  NUMROWKEYWORD  "rows"
#define  ROWKEYWORD     "row"

static int tlengthS ;                 /* total length of stdcells */
static int cheightS ;                 /* height of stdcell */
static int num_classeS ;	      /* number of cell classes */
static int *classS ;	      	      /* cell class name */
static int *lbS ;	              /* lower bound of cell class length */
static int *ubS ;	              /* upper bound of cell class length */

static float rowSepS ;               /* row separation, relative */
static int rowSepAbsS ;            /* row separation, absolute */

static int num_macroS ;               /* number of macros output */

/* Forward declaration */
int read_gen_file();
void read_stat_file();
void build_mver_file( int left, int right, int bottom, int top );
double read_par_file();

void config_rows()
{
	int left, right, bottom, top;/* core area */
	BOOL stateSaved = FALSE ; /* whether need to restore state */

	read_stat_file() ;
	(void) read_par_file() ;
	grid_cells() ;

	/* place the pads to get standard cell core area */
	find_core_boundary( &left, &right, &bottom, &top ) ;

	build_mver_file( left, right, bottom, top ) ;

	/* now call genrows program */
	/* find the path of genrows relative to main program */
	/* Ysystem will kill program if catastrophe occurred */
	int status;
	int Genrows( BOOL d, BOOL f, BOOL n, char *cktName);
	if( doGraphicsG ){
		// setup the variables
		status = Genrows( 0, 0, 0, cktNameG);
	} else if( get_batch_mode() ){
		status = Genrows( 0, 0, 0, cktNameG);
	} else {
		status = Genrows( 0, 0, 1, cktNameG);
	}
	if(doGraphicsG) {
		G( TWrestoreState() ) ;
	}

	/* ############# end of genrows execution ############# */

	if( stateSaved ){
		/* if we save the graphics state we need to restore it */
		G( TWrestoreState() ) ;
	}

	/* read result to update new core */
	read_gen_file() ;

	/* add spacing between pads and core area. */
#ifdef LATER
	cellarrayG[endpadgrpsG+L]->tiles->rborder = numpadsG*track_spacingXG ;
	cellarrayG[endpadgrpsG+R]->tiles->lborder = numpadsG*track_spacingXG ;
	cellarrayG[endpadgrpsG+B]->tiles->tborder = numpadsG*track_spacingYG ;
	cellarrayG[endpadgrpsG+T]->tiles->bborder = numpadsG*track_spacingYG ;
#endif

	/* read back result */
	setVirtualCore( TRUE ) ;
	placepads() ;
} /* end config_rows */

void read_stat_file()
{
	char filename[LRECL] ;
	char input[LRECL] ;
	char *bufferptr ;
	char **tokens ;
	int class ;
	int numtokens ;
	int class_count ;
	FILE *fin ;

	/***********************************************************
	* Read from circuitName.stat file.
	***********************************************************/
	sprintf( filename, "%s.stat", cktNameG ) ;
	fin = fopen(filename,"r") ;

	class_count = 0 ;
	num_classeS = 0 ;
	while((bufferptr = fgets( input, LRECL, fin ))){
		tokens = Ystrparser( bufferptr, " :\t\n", &numtokens ) ;
		if( strcmp( tokens[0], "tot_length" ) == STRINGEQ ){
			tlengthS = atoi( tokens[1] ) ;
		} else if( strcmp( tokens[0], "cell_height" ) == STRINGEQ ){
			cheightS = atoi( tokens[1] ) ;
		} else if( strcmp( tokens[0], "num_classes" ) == STRINGEQ ){
			num_classeS = atoi( tokens[1] ) ;
			classS = YVECTOR_MALLOC( 1, num_classeS, int ) ;
			lbS = YVECTOR_MALLOC( 1, num_classeS, int ) ;
			ubS = YVECTOR_MALLOC( 1, num_classeS, int ) ;
		} else if( strcmp( tokens[0], "class" ) == STRINGEQ ){
			class = atoi( tokens[1] ) ;
			classS[++class_count] = class ;
			lbS[class_count] = atoi( tokens[3] ) ;
			ubS[class_count] = atoi( tokens[5] ) ;
		}
	}

	fclose( fin ) ;
} /* end read_stat_file */

double read_par_file()
{
    char *bufferptr ;
    char **tokens ;      /* for parsing menu file */
    int  numtokens ;
    int  line ; /* line number of TWmenu file */
    BOOL onNotOff ;
    BOOL wildcard ;

    Yreadpar_init( cktNameG, USER, TWSC, TRUE ) ;
    while((tokens = Yreadpar_next( &bufferptr, &line, &numtokens,  &onNotOff, &wildcard ))){
	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	}
	if( numtokens != 2 && numtokens != 3 ){
	    continue ;
	}
	if( strcmp( tokens[0], ROWSEP ) == STRINGEQ){
	    
	    /* there better be only two or three tokens on this line */
	    /* 3rd token for absolute row spacing is not handled */
	    if( numtokens == 2 ){
		rowSepS = atof( tokens[1] ) ;
		rowSepAbsS = 0;
	    } else if (numtokens == 3 ){
		rowSepS = atof( tokens[1] ) ;
		rowSepAbsS = atoi( tokens[2] ) ;
	    } else {
		sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
		M(ERRMSG, "read_par_file", YmsgG ) ;
	    }
	    printf("%s: rowSepG %f rowSepAbsG %d\n",__FUNCTION__,rowSepS,rowSepAbsS);
	}
    }
    if( rowSepS < 0.0 ) {
	M(ERRMSG,"read_par_file","Improper row separation.\n" ) ;
	M(MSG,NULL,"Row separaion should be specified in .par file\n" ) ;
	M(MSG,NULL,"Defaulting to 1.0\n" ) ;
	rowSepS = 1.0 ;
	rowSepAbsS = 0.0 ;
    }
    return( rowSepS ) ;
} /* end read_par_file */

#define  RELATIVE_TO_CURPOS  1
void output_partition()
{
	int line = 0;               /* current line number */
	FILE *fp ;
	char filename[LRECL] ;
	char buffer[LRECL] ;
	char *bufferptr ;
	char *matchptr ;

	/* ****************************************************************
		Output the ports into sc cell file.
	***************************************************************** */

	/* other definitions for the apollo version */
	char filename_out[LRECL] ;
	FILE *fout ;

	sprintf( filename, "%s.scel", cktNameG ) ;
	if(!YfileExists(filename))
		return;

	fp = fopen( filename, "r" ) ;
	sprintf( filename_out, "%s.temp", cktNameG ) ;
	fout = fopen( filename_out, "w") ;

	/* start at beginning of file and read till read mc entity or pad */
	while( bufferptr = fgets( buffer, LRECL, fp )) {
		/* remove leading blanks or tabs */
		if(!(line%1000))
			printf("Did read %d lines\n",line);

		matchptr = Yremove_lblanks( bufferptr ) ;
		if( strncmp( matchptr, "hardcell", 8 ) == STRINGEQ ){
			break ;
		} else if( strncmp( matchptr, "softcell", 8 ) == STRINGEQ ){
			break ;
		} else if( strncmp( matchptr, "pad", 3 ) == STRINGEQ ){
			break ;
		} else {
			/* do an echo */
			line++ ;
			fprintf( fout, "%s", bufferptr ) ;
		}
	}
	D( "output_partition", printf( "broke on line:%d\n", line ) ; );
	/* send the rest of the macro output of .mdat to fout */
	output( fout ) ;
	fclose( fout ) ;
	fclose( fp ) ;
	YmoveFile( filename_out, filename );

} /* end of prnt_data */

void build_mver_file( int left, int right, int bottom, int top )
{
	CELLBOXPTR cellptr ;
	FILE *fp ;
	char filename[LRECL] ;
	int  type ;
	int  i, k, cell ;
	int  xc, yc ;
	int  x, y ;
	int  separation ;
	BOUNBOXPTR bounptr ;         /* bounding box pointer */

	/* ######### Create genrows file and exec genrows ######### */
	/* open vertex file for writing */
	sprintf(filename, "%s.mver" , cktNameG ) ;
	fp = fopen( filename , "w" ) ;

	fprintf( fp, "total_row_length %d\n", tlengthS ) ;
	if( num_classeS ){
		fprintf( fp, "num_classes %d\n", num_classeS ) ;
	}
	for( i = 1 ; i <= num_classeS ; i++ ){
		fprintf( fp, "class %d lb %d ub %d\n", classS[i], lbS[i], ubS[i] ) ;
	}
	fprintf( fp, "actual_row_height %d\n", cheightS ) ;
	separation = (int) ( (double) cheightS * rowSepS + rowSepAbsS) ;
	fprintf( fp, "channel_separation %d\n", separation ) ;
	fprintf( fp, "min_length %d\n", (right-left) / 5 ) ;
	fprintf( fp, "core %d %d %d %d\n", left, bottom, right, top ) ;
	fprintf( fp, "grid %d %d\n", track_spacingXG, track_spacingYG ) ;
	num_macroS = 0 ;
	for( cell = 1 ; cell <= numcellsG ; cell++ ) {
		cellptr = cellarrayG[cell] ;
		type = cellptr->celltype ;
		if( type != CUSTOMCELLTYPE && type != SOFTCELLTYPE ){
			continue ;
		}
		num_macroS++ ;
	}
	fprintf( fp, "num_macro %d\n", num_macroS ) ;


	for( cell = 1 ; cell <= numcellsG ; cell++ ) {
		cellptr = cellarrayG[cell] ;
		type = cellptr->celltype ;
		if( type != CUSTOMCELLTYPE && type != SOFTCELLTYPE ){
			continue ;
		}
		fprintf(fp,"macro orient %d %d vertices ", cellptr->orient,
		cellptr->numsides ) ;
		output_vertices( fp, cellptr ) ;
	}

	fclose( fp ) ;
} /* end build_mver_file */

int read_gen_file()
{
	char filename[LRECL] ;
	char buffer[LRECL], *bufferptr ;
	char **tokens ;     /* for parsing file */
	int  numtokens, line ;
	int  cell ;          /* current cell number */
	int  type ;          /* current cell type */
	CELLBOXPTR cellptr ; /* current cell */
	BOOL abort ; /* whether to abort program */
	FILE *fp ;

	if( num_macroS == 0 ){
		return 0;
	}
	/* **************** READ RESULTS of genrows ************/
	sprintf(filename, "%s.gen" , cktNameG ) ;
	fp = TWOPEN( filename , "r", ABORT ) ;

	/* parse file */
	line = 0 ;
	abort = FALSE ;
	for( cell = 1 ; cell <= numcellsG ; cell++ ) {

		cellptr = cellarrayG[cell] ;
		type = cellptr->celltype ;
		if( type != CUSTOMCELLTYPE && type != SOFTCELLTYPE ){
			continue ;
		}
		while( bufferptr=fgets(buffer,LRECL,fp )){
			/* parse file */
			line++ ; /* increment line number */
			tokens = Ystrparser( bufferptr, " \t\n", &numtokens );
			if( numtokens == 0 ){
				/* skip over empty lines */
				continue ;
			} else if( numtokens == 3 ){
				cellptr->xcenter = atoi( tokens[0] ) ;
				cellptr->ycenter = atoi( tokens[1] ) ;
				cellptr->orient = atoi( tokens[2] ) ;
				break ; /* go on to the next cell */
			} else if( strcmp(tokens[0], "core" ) == STRINGEQ &&
				numtokens == 5 ){
				blocklG = MIN( blocklG, atoi( tokens[1] ) ) ;
				blockbG = MIN( blockbG, atoi( tokens[2] ) ) ;
				blockrG = MAX( blockrG, atoi( tokens[3] ) ) ;
				blocktG = MAX( blocktG, atoi( tokens[4] ) ) ;
			} else {
				sprintf( YmsgG, "Problem reading .gen file on line:%d\n",line ) ;
				M( ERRMSG, "read_gen_file", YmsgG ) ;
				abort = TRUE ;
			}
		}
	}

	TWCLOSE( fp ) ;

	if( abort ){
		M(ERRMSG, "read_gen_file", "Problem with genrows. Must abort\n" ) ;
	return 1;
	}
	/* ************ END READ RESULTS of genrows ************/

} /* end read_gen_file() */
