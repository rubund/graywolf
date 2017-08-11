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
FILE:	    output.c
DESCRIPTION:This file accumulates data for TimberWolf system.
CONTENTS:   
DATE:	    Apr 24, 1989 
REVISIONS:  May 24, 1989 - updated argument to YcurTime.
	    May 15, 1990 - now add up cell area so that standard cell
		macros size will be the average macro size.
	    Aug 3,  1990 - added port counting to output.
	    Thu Apr 18 01:12:20 EDT 1991 - added more error checking.
	    Wed Jun 12 13:40:11 CDT 1991 - added check for pins
		outside cell boundary.
	    Wed Jul 24 21:06:02 CDT 1991 - added a more
		meaningful output error message.
----------------------------------------------------------------- */
#include <globals.h>
#include "mincut.h"
#include "output.h"

#define LSHAPE
#define L2SHAPE
#define TSHAPE
#define USHAPE

int objectS = 0 ;       /* number of objects read */
int celltypeS ;         /* current cell type */
double total_cell_lenS = 0.0 ;
double total_cell_heightS = 0.0;
double total_areaS = 0.0 ;
double core_areaS = 0.0 ;
double average_cell_heightS ;
double row_sepS = 0;
int row_sep_absS = 0;
int total_std_cellS = 0 ;
char current_cellS[LRECL] ; /* the current cell name */
char cur_pinnameS[LRECL] ;  /* current pinname */

YHASHPTR netTableS ;    /* hash table for cross referencing nets */
char *cktNameG ;

void init()
{
	/* get ready for parsing */
	/* make hash table for nets */
	netTableS = Yhash_table_create( EXPECTEDNUMNETS ) ;
} /* end init */

void addCell(int celltype, char *cellname)
{
	strcpy( current_cellS, cellname ) ;
	/* passify the user */
	if( (++objectS % 50) == 0 ){
		sprintf( YmsgG, "Read %4d objects so far...\n", objectS ) ;
		M( MSG, NULL, YmsgG ) ;
	}
	celltypeS = celltype ;   /* save for determining instances etc. */

	switch( celltypeS ){
		case HARDCELLTYPE:
			break ;
		case SOFTCELLTYPE:
			break ;
		case PADCELLTYPE:
			break ;
		case STDCELLTYPE:
			total_std_cellS++ ;
			break ;
	}
} /* end addCell */

void addNet(char *signal )
{
	NETPTR data ;

	if( strcmp( signal, "TW_PASS_THRU" ) == STRINGEQ ){
		return ; /* not a net so return */
	}
	if((data = (NETPTR) Yhash_search( netTableS, signal, NULL, FIND))) {
		/* this net now makes an io connection mark it as such */
		switch( celltypeS ){
			case HARDCELLTYPE:
			case SOFTCELLTYPE:
			case PADCELLTYPE:
				data->io_signal = TRUE ;
				break ;
			case STDCELLTYPE:
				break ;
		}
	} else {
		/* else a new net - load data holder */
		data = (NETPTR) Ysafe_malloc( sizeof(NETBOX) ) ;
		data->net = signal ;
		switch( celltypeS ){
			case HARDCELLTYPE:
			case SOFTCELLTYPE:
			case PADCELLTYPE:
				data->io_signal = TRUE ;
				break ;
			case STDCELLTYPE:
				data->io_signal = FALSE ;
				break ;
		}
		if( Yhash_search( netTableS, signal, (VOIDPTR)data, ENTER )) {
			sprintf( YmsgG, "Trouble adding signal:%s to hash table\n", signal ) ;
			M(ERRMSG,"addNet",YmsgG ) ;
		}
	}
} /* end addNet */

void set_bbox( int left, int right, int bottom, int top )
{
	int width, height ;
	width = right - left;
	height = top - bottom;
	total_cell_heightS += height;
	total_cell_lenS += width;
	total_areaS += width * height ;
	core_areaS += (width + (double)row_sep_absS) * (height + (double)row_sep_absS) ;
} /* end set_bbox */

void output( FILE *fp )
{
	int g ;

	if( total_std_cellS > 0 ){
		average_cell_heightS = total_cell_heightS / total_std_cellS;
	} else {
		average_cell_heightS = 0.0 ;
	}

	core_areaS *= row_sepS*2;

	printf( "\n----------------------------\n" ) ;
	printf( "Total stdcells     :%d\n", total_std_cellS ) ;
	printf( "Total cell width   :%4.2le\n", total_cell_lenS ) ;
	printf( "Total cell height  :%4.2le\n", total_cell_heightS ) ;
	printf( "Total cell area    :%4.2le\n", total_areaS ) ;
	printf( "Total core area    :%4.2le\n", core_areaS ) ;
	printf( "Average cell height:%4.2le\n\n", average_cell_heightS ) ;

	/* the first instance take as a rectangle - initially a square */
	g = (int) sqrt( core_areaS ) ;
	fprintf( fp, "cluster 1 name core\n" ) ;
	fprintf( fp, "corners 4 0 0   0 %d  %d %d   %d 0\n", g, g, g, g ) ;
	write_softpins( fp ) ;

#ifdef LSHAPE
	/* for the second instance use an L shape */
	g = (int) sqrt( core_areaS / 3.0 ) ;
	if( g > 2 ){
		fprintf( fp, "instance core_L\n" ) ;
		fprintf( fp, "corners 6 " ) ;
		fprintf( fp, "0 0 " ) ;
		fprintf( fp, "0 %d ", 2*g ) ;
		fprintf( fp, "%d %d ", g, 2*g ) ;
		fprintf( fp, "%d %d ", g, g ) ;
		fprintf( fp, "%d %d ", 2*g, g ) ;
		fprintf( fp, "%d 0\n", 2*g ) ;
		write_softpins( fp ) ;
	}
#endif

#ifdef TSHAPE
	/* for the third instance use a T shape */
	g = (int) sqrt( core_areaS / 4.0 ) ;
	if( g > 2 ){
		fprintf( fp, "instance core_T\n" ) ;
		fprintf( fp, "corners 8 " ) ;
		fprintf( fp, "%d 0 ", g ) ;
		fprintf( fp, "%d %d ", g, g ) ;
		fprintf( fp, "0 %d ", g ) ;
		fprintf( fp, "0 %d ", 2*g ) ;
		fprintf( fp, "%d %d ", 3*g, 2*g ) ;
		fprintf( fp, "%d %d ", 3*g, g ) ;
		fprintf( fp, "%d %d ", 2*g, g ) ;
		fprintf( fp, "%d 0\n", 2*g ) ;
		write_softpins( fp ) ;
	}
#endif

#ifdef USHAPE
	/* for the third instance use a U shape */
	g = (int) sqrt( core_areaS / 5.0 ) ;
	if( g > 2 ){
		fprintf( fp, "instance core_U\n" ) ;
		fprintf( fp, "corners 8 " ) ;
		fprintf( fp, "0 0 " ) ;
		fprintf( fp, "0 %d ", 2*g ) ;
		fprintf( fp, "%d %d ", g, 2*g ) ;
		fprintf( fp, "%d %d ", g, g ) ;
		fprintf( fp, "%d %d ", 2*g, g ) ;
		fprintf( fp, "%d %d ", 2*g, 2*g ) ;
		fprintf( fp, "%d %d ", 3*g, 2*g ) ;
		fprintf( fp, "%d 0\n", 3*g ) ;
		write_softpins( fp ) ;
	}
#endif

#ifdef L2SHAPE
	/* for the fourth instance use a modified L shape */
	g = (int) sqrt( core_areaS / 5.0 ) ;
	if( g > 2 ){
		fprintf( fp, "instance core_L2\n" ) ;
		fprintf( fp, "corners 6 " ) ;
		fprintf( fp, "0 0 " ) ;
		fprintf( fp, "0 %d ", 2*g ) ;
		fprintf( fp, "%d %d ", 2*g, 2*g ) ;
		fprintf( fp, "%d %d ", 2*g, g ) ;
		fprintf( fp, "%d %d ", 3*g, g ) ;
		fprintf( fp, "%d 0\n", 3*g ) ;
		write_softpins( fp ) ;
	}
#endif

} /* end output */

void write_softpins( FILE *fp )
{
	YTABLEPTR thread ;
	NETPTR net ;
	int pin_count ;

	fprintf( fp, "asplb 0.5 aspub 2.0\n");
	fprintf( fp, "class 0 orientations 0 1 2 3 4 5 6 7\n") ;

	pin_count = 0 ;
	for( thread=netTableS->thread;thread;thread=thread->threadNext){
		net = (NETPTR) thread->data ;
		if(net) if(net->io_signal) {
			fprintf( fp, "softpin name pin%d signal %s\n", ++pin_count, net->net ) ;
		}
	}
	fprintf( fp, "\n" ) ;
} /* end write_softpins */

void read_par()
{
	char *bufferptr ;
	char **tokens ;
	int numtokens ;
	int line ;
	BOOL onNotOff ;
	BOOL wildcard ;
	BOOL found ;

	found = FALSE ;
	Yreadpar_init( cktNameG, USER, TWSC, TRUE ) ;
	char *tmpStr;
	while((tokens = Yreadpar_next( &bufferptr, &line, &numtokens, &onNotOff, &wildcard))) {
		if( numtokens ) {
			if((tmpStr = strstr(tokens[0], "TWSC*"))) {
				tmpStr+=strlen("TWSC*");
				tokens[0] = Ystrclone(tmpStr);
			} else if ((tmpStr = strstr(tokens[0], "TWMC*"))) {
				tmpStr+=strlen("TWMC*");
				tokens[0] = Ystrclone(tmpStr);
			} else if ((tmpStr = strstr(tokens[0], "GENR*"))) {
				tmpStr+=strlen("GENR*");
				tokens[0] = Ystrclone(tmpStr);
			} else if ((tmpStr = strstr(tokens[0], "*"))) {
				tmpStr++;
				tokens[0] = Ystrclone(tmpStr);
			}
		}
		if( numtokens == 0 ){
		/* skip over empty lines */
			continue ;
		}
		if( strcmp( tokens[0], "rowSep" ) == STRINGEQ ){
			row_sepS = atof( tokens[1] ) ;
			if(!(row_sepS>0.0))
				row_sepS = 1.0;
			row_sep_absS = (numtokens == 3) ? atoi( tokens[2] ) : 0;
			found = TRUE ;
			printf("%s: rowSepG %f rowSepAbsG %d\n",__FUNCTION__,row_sepS,row_sep_absS);
		}
	}
	if(!(found)){
		M(WARNMSG, "read_par", "Couldn't find rowsep in parameter file\n" ) ;
		M(WARNMSG, NULL,"Using default of 1.0\n" ) ; 
		row_sepS = 1.0 ;
		row_sep_absS = 0;
	}
} /* end readpar */

void update_stats( FILE *fp )
{
	fprintf( fp, "tot_length:%d\n", (int)total_cell_lenS);
	fprintf( fp, "num_soft:1\n" ) ;
	fprintf( fp, "cell_height:%d\n", (int)average_cell_heightS);
} /* end update_stats */
