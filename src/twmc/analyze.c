/*
 *   Copyright (C) 1988-1991 Yale University
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
FILE:	    analyze.c                                       
DESCRIPTION:This file contains routines for analyzing random to optimal
	    wire ratio.
CONTENTS:   analyze( )
	    compare( int, int )
DATE:	    Apr  4, 1988 
REVISIONS:  
----------------------------------------------------------------- */
#include "allheaders.h"

#define CURWT      1
#define PRIORWT    1
#define KEYSTRING "TimberWolfMC reduction"
#define EXPECTEDWIRERATIO   2.0

int *howmanyS ;

double analyze()
{

int **number , i , net , net1 , net2 , num , cell ;
int *count , different , cnum , c2num , *arraynet ;
int num_nets , tot_cels ;
double C , C1 , C2 , C3 , wireRatio ;
PINBOXPTR pinptr ;
int comparex() ;
double weight_past_runs( /* wireRatio */ ) ;

count  = (int *) Ysafe_malloc( (1 + numcellsG) * sizeof( int ) ) ;
number = (int **) Ysafe_malloc( (1 + numnetsG) * sizeof( int *) ) ;
howmanyS = (int *) Ysafe_malloc( (1 + numnetsG) * sizeof( int ) ) ;
arraynet = (int *) Ysafe_malloc( (1 + numnetsG) * sizeof( int ) ) ;
for( net = 0 ; net <= numnetsG ; net++ ) {
    number[net] = (int *) Ysafe_malloc( (1 + numcellsG) * sizeof(int) ) ;
}

for( net = 1 ; net <= numnetsG ; net++ ) {
    for( cell = 0 ; cell <= numcellsG ; cell++ ) {
	count[cell] = 0 ;
	number[net][cell] = 0 ;
    }
    for( pinptr=netarrayG[net]->pins;pinptr; pinptr = pinptr->next ){
	if( pinptr->cell <= numcellsG ) {
	    count[pinptr->cell] = 1 ;
	}
    }
    /*
     *  I would like to find the number of distinct nets
     */
    for( cell = 1 ; cell <= numcellsG ; cell++ ) { 
	if( count[cell] == 1 ) {
	    number[net][ ++number[net][0] ] = cell ;
	}
    }
}
/* ********************************************************** */
num_nets = 0 ;
tot_cels = 0 ;
for( net1 = 1 ; net1 <= numnetsG ; net1++ ) {
    if( number[net1][0] <= 1 ) {
	continue ;
    }
    num_nets++ ;
    tot_cels += number[net1][0] ;
}


printf("\n\n*************************************\n");
printf("AVERAGE NUMBER OF CELLS PER NET: %f\n",
		( (double) tot_cels / (double) num_nets	)  ) ;
printf("*************************************\n\n\n");
/* ********************************************************** */
for( net1 = 1 ; net1 <= numnetsG ; net1++ ) {
    if( number[net1][0] == 0 ) {
	howmanyS[net1] = 0 ;
	continue ;
    }
    if( number[net1][0] == 1 ) {
	number[net1][0] = 0 ;
	howmanyS[net1] = 0 ;
	continue ;
    }
    howmanyS[net1] = 1 ;
    for( net2 = net1 + 1 ; net2 <= numnetsG ; net2++ ) {
	if( number[net2][0] != number[net1][0] ) {
	    continue ;
	}
	different = 0 ;
	for( i = 1 ; i <= numcellsG ; i++ ) {
	    if( number[net2][i] != number[net1][i] ) {
		different = 1 ;
		break ;
	    }
	}
	if( ! different ) {
	    number[net2][0] = 0 ;
	    howmanyS[net1]++ ;
	}
    }
}

arraynet[0] = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    if( howmanyS[net] <= 0 ) {
	continue ;
    }
    arraynet[ ++arraynet[0] ] = net ;
}
num = arraynet[0] ;
arraynet[0] = arraynet[ arraynet[0] ] ;
Yquicksort( (char *) arraynet ,  num , sizeof( int ), comparex  ) ;
/*  sorted: most occurrences first  */

num = 0 ;
cnum = 0 ;
c2num = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    if( number[net][0] > 0 ) {
	cnum += number[net][0] - 1 ;
	c2num += number[net][0] ;
	num++ ;
    }
}

C = (double) num / (double) numcellsG ;
C1 = (double) cnum / (double) num ;
C2 = (double) c2num / (double) num ;
C3 = (double) cnum / (double)(numcellsG - 1) ;
printf("\n\n\n**********************************************\n\n");
printf("The average number of distinct nets per cell is\n");
printf("given by: %6.2f\n\n", C );
printf("The average number of cells per net is\n");
printf("given by: %6.2f\n\n", C2 );
printf("The average number of other cells per net is\n");
printf("given by: %6.2f\n\n", C1 );
printf("The ratio of total cells specified per net to\n");
printf("numcells is given by: %6.2f\n\n", C3 );
printf("The average number of cells connected to a cell is\n");
printf("given by: %6.2f\n\n", C * C1 );
printf("**********************************************\n\n\n");
wireRatio = EXPECTEDWIRERATIO ;

printf("Expected Wire Reduction Relative to Random:%6.2f\n\n",wireRatio);

wireRatio = weight_past_runs( wireRatio ) ;
sprintf( YmsgG,"\n\nWire ratio updated to:%4.2f\n\n", wireRatio ) ;
M( MSG, "analyze", YmsgG ) ;

return( wireRatio );
}

int comparex( a , b )
int *a , *b ;

{
    return( howmanyS[*b] - howmanyS[*a] ) ;
}

double weight_past_runs( wireRatio )
double wireRatio ;
{
    FILE *fp ;          /* log file */
    char buffer[LRECL], *bufferptr ;
    char **tokens ;      /* for parsing file */
    int  numtokens ;
    double reduction ;   /* previous value of reduction */

    sprintf( buffer, "%s.log", cktNameG ) ;
    fp = TWOPEN( buffer, "r", NOABORT ) ;

    if(!(fp) ){
	/* doesn't exist return calculated value */
	return( wireRatio ) ;
    }
    /* parse log file */
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	tokens = Ystrparser( bufferptr, ":\t\n", &numtokens );

	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( numtokens == 5 &&
	    strcmp( tokens[3], KEYSTRING ) == STRINGEQ){

	    reduction = atof( tokens[4] ) ;
	    /* weight the average towards latest run */
	    wireRatio = (CURWT * reduction + PRIORWT * wireRatio)/
		( CURWT + PRIORWT ) ;	
	}
    }
    return( wireRatio ) ;
	    
} /* end weight_past_runs */
