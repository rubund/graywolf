/* ----------------------------------------------------------------- 
FILE:	    gentwf.c                                       
DESCRIPTION:This file contains the routine which translates the pin
	    file into the twf file for the global router.
CONTENTS:   gentwf( )
DATE:	    Apr 17, 1988 
REVISIONS:  Apr 15, 1989 - pass on x,y location of pin for detail routing.
	    Jun 22, 1990 - rewrote parser due to problems on SUN.
	    Sun Dec 16 01:26:46 EST 1990 - added analog output.
	    Thu Dec 20 00:20:16 EST 1990 - now handle current and
		matches.
	    Dec 21, 1990 - added layer information to output.
	    Thu Mar  7 01:39:42 EST 1991 - now pins are always moved
		from a vertex.
	    Mon May  6 22:38:06 EDT 1991 - pins should not be moved
		only the offset in channel graph should change.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) gentwf.c version 1.10 5/6/91" ;
#endif

#include <string.h>
#include <geo.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <yalecad/file.h>
#include <yalecad/string.h>

#define TI




static serror();

gentwf( crossref )
int *crossref ;
{

FILE *fp2 , *fp1  ;
char filename[LRECL] ;
char input[LRECL] ;
char copy_buf[LRECL] ;
char *bufferptr ;
char **tokens ;
INT  numtokens ;
int  line ;
int x , y , cell , PorE , offset , channel , hit ;
int node1 , node2 , min , edge , loc , ecount , max ;
int layer ;
FLAREPTR ifptr , ofptr , savefptr ;
BOOL firstLine = TRUE ;


/***********************************************************
* Read from circuitName.mpin file.
***********************************************************/
sprintf( filename, "%s.mtwf", cktNameG ) ;
fp2 = TWOPEN( filename , "w", ABORT ) ;

sprintf( filename, "%s.mpin", cktNameG ) ;
fp1 = TWOPEN( filename , "r", ABORT ) ;

line = 0 ;
while( bufferptr = fgets( input, LRECL, fp1 ) ){
    /* save a copy since Ystrparse is destructive */
    strcpy( copy_buf, bufferptr ) ; 
    tokens = Ystrparser( bufferptr, " \t\n", &numtokens ) ;
    ++line ;
    if( numtokens <= 0 ){
	continue ;
    }
    if( strcmp( tokens[0], "net" ) == STRINGEQ ){
	if( numtokens != 2 ){
	    serror( line, filename ) ;
	    continue ;
	}

	if( firstLine ){
	    fprintf(fp2,"net %s\n", tokens[1] );
	    firstLine = FALSE ;
	} else {
	    fprintf(fp2,"\nnet %s\n", tokens[1] );
	}
    } else if( strcmp( tokens[0], "pin" ) == STRINGEQ ){
	if( numtokens != 12 && numtokens != 14 ){
	    serror( line, filename ) ;
	    continue ;
	}

	x = atoi( tokens[3] ) ;
	y = atoi( tokens[5] ) ;
	cell = atoi( tokens[7] ) ;
	layer = atoi( tokens[9] ) ;
	PorE = atoi( tokens[11] ) ;

	min = INT_MAX ;
	ifptr = xNodulesG[cell].inList ;
	for( ; ifptr != (FLAREPTR) NULL ; ifptr = ifptr->next ) {
	    edge = ifptr->fixEdge ;
	    loc = edgeListG[edge].loc ;
	    if( ABS(loc - x) < min ) {
		if( y <= rectArrayG[ 
			    eArrayG[ ifptr->eindex[1] ].index2 ].yc &&
			    y >= rectArrayG[ 
			    eArrayG[ ifptr->eindex[1] ].index1 ].yc ) {
		    min = ABS(loc - x) ;
		    savefptr = ifptr ;
		    hit = 1 ;
		}
	    }
	}
	ofptr = xNodulesG[cell].outList ;
	for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
	    edge = ofptr->fixEdge ;
	    loc = edgeListG[edge].loc ;
	    if( ABS(loc - x) < min ) {
		if( y <= rectArrayG[ 
			    eArrayG[ ofptr->eindex[1] ].index2 ].yc &&
			    y >= rectArrayG[ 
			    eArrayG[ ofptr->eindex[1] ].index1 ].yc ) {
		    min = ABS(loc - x) ;
		    savefptr = ofptr ;
		    hit = 1 ;
		}
	    }
	}
	ifptr = yNodulesG[cell].inList ;
	for( ; ifptr != (FLAREPTR) NULL ; ifptr = ifptr->next ) {
	    edge = ifptr->fixEdge ;
	    loc = edgeListG[edge].loc ;
	    if( ABS(loc - y) < min ) {
		if( x <= rectArrayG[ 
			    eArrayG[ ifptr->eindex[1] ].index2 ].xc &&
			    x >= rectArrayG[ 
			    eArrayG[ ifptr->eindex[1] ].index1 ].xc ) {
		    min = ABS(loc - y) ;
		    savefptr = ifptr ;
		    hit = 2 ;
		}
	    }
	}
	ofptr = yNodulesG[cell].outList ;
	for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
	    edge = ofptr->fixEdge ;
	    loc = edgeListG[edge].loc ;
	    if( ABS(loc - y) < min ) {
		if( x <= rectArrayG[ 
			    eArrayG[ ofptr->eindex[1] ].index2 ].xc &&
			    x >= rectArrayG[ 
			    eArrayG[ ofptr->eindex[1] ].index1 ].xc ) {
		    min = ABS(loc - y) ;
		    savefptr = ofptr ;
		    hit = 2 ;
		}
	    }
	}
	ecount = 0 ;
	do {
	    channel = savefptr->eindex[++ecount] ;
	} while( eArrayG[channel].active == FALSE ) ;

	node1 = eArrayG[channel].index1 ;
	node2 = eArrayG[channel].index2 ;
	/* now check to make sure nodes are valid in graph */
	if( crossref[node1] <= 0 || crossref[node2] <= 0 ){
	    /* we have a problem here.  Find a valid side */
	    /* find direction */
	    sprintf( YmsgG, "Pin:%s outside channel graph - ignored\n",
	        tokens[1] ) ;
	    M( ERRMSG, "gentwf", YmsgG ) ;
	    continue ;
	}

#ifdef TI /* make it work at TI for Xmas - otherwise give Dahe the latest format */
	if( PorE == 1 ) {
	    fprintf(fp2,"    pin   %s cell %d nodes   ", tokens[1], cell);
	} else {
	    fprintf(fp2,"    equiv %s cell %d nodes   ", tokens[1], cell);
	}
#else
	if( PorE == 1 ) {
	    fprintf(fp2,"    pin   %s cell %d layer %d nodes ",
				    tokens[1],cell,layer ) ;
	} else {
	    fprintf(fp2,"    equiv %s cell %d layer %d nodes ",
				    tokens[1], cell,layer ) ;
	}
#endif
	if( hit == 1 ) {
	    offset = y - rectArrayG[node1].yc ;
	    max = rectArrayG[node2].yc - rectArrayG[node1].yc ;
	} else {
	    offset = x - rectArrayG[node1].xc ;
	    max = rectArrayG[node2].xc - rectArrayG[node1].xc ;
	}
	if( offset < 0 ) {
	    printf("\n\n");
	    printf("Pin: %s has location outside the range\n",
							    tokens[1] );
	    printf("of the edge: %d %d to which it was assigned\n",
				crossref[node1], crossref[node2] ) ;
	    printf("by an amount: %d\n", offset ) ;
	    printf("\n\n");
	    fflush(stdout);
	    offset = 0 ;
	} else if( offset > max ) {
	    printf("\n\n\n\n\n\n");
	    printf("Pin: %s has location outside the range\n",
							    tokens[1] );
	    printf("of the edge: %d %d to which it was assigned\n",
				crossref[node1], crossref[node2] ) ;
	    printf("by an amount: %d\n", offset - max ) ;
	    printf("\n\n");
	    fflush(stdout);
	    offset = max ;
	} else if( offset == 0 ){
	    offset = 1 ;
	    D( "gengraph/gentwf",
		printf("\n\n");
		printf("Pin: %s on edge:%d %d is located at a vertex\n", tokens[1],
			crossref[node1], crossref[node2] ) ;
		printf("\tMoved to: %d (%d,%d)\n\n", offset, x, y ) ;
		fflush(stdout);
	    ) ;
	} else if( offset == max ){
	    offset = max - 1 ;
	    D( "gengraph/gentwf",
		printf("\n\n");
		printf("Pin: %s on edge:%d %d is located at a vertex\n", tokens[1],
			crossref[node1], crossref[node2] ) ;
		printf("\tMoved to: %d (%d,%d)\n\n", offset, x, y ) ;
		fflush(stdout);
	    ) ;
	}
	fprintf(fp2,"%d  %d  at  %d (%d,%d)",
	    crossref[node1] , crossref[node2] , offset, x, y ) ;
	if( numtokens == 14 ){
	    fprintf( fp2, " %s %s", tokens[12], tokens[13] ) ;
	}
	fprintf( fp2, "\n" ) ;
	/* end pin */

    } else if( strcmp( tokens[0], "analog_net" ) == STRINGEQ ){
	if(!(firstLine)){
	    fprintf( fp2,"\n" ) ;
	} else {
	    firstLine = FALSE ;
	}
	fprintf( fp2, "%s", copy_buf ) ;
    } else if( strcmp( tokens[0], "common_point" ) == STRINGEQ ){
	fprintf( fp2, "%s", copy_buf ) ;
    } else if( strcmp( tokens[0], "cap_match" ) == STRINGEQ ){
	fprintf( fp2, "%s", copy_buf ) ;
    } else if( strcmp( tokens[0], "res_match" ) == STRINGEQ ){
	fprintf( fp2, "%s", copy_buf ) ;
    } else if( strcmp( tokens[0], "net_res_match" ) == STRINGEQ ){
	fprintf( fp2, "%s", copy_buf ) ;
    } else if( strcmp( tokens[0], "net_cap_match" ) == STRINGEQ ){
	fprintf( fp2, "%s", copy_buf ) ;
    } else {
	sprintf( YmsgG, "Syntax error at line:%d\n", line ) ;
	M(ERRMSG, "gentwf", YmsgG ) ;
    }
}
TWCLOSE(fp1) ;
TWCLOSE(fp2) ;

return ;
} /* end gentwf */

static serror( line, file )
int line ;
char *file ;
{
    sprintf( YmsgG, "Syntax error at line %d in file %s\n",
	line, file ) ;
    M( ERRMSG, "gentwf", YmsgG ) ;
} /* end serror */
