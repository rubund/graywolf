/*
 *   Copyright (C) 1988-1990 Yale University
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
FILE:	    debug.c                                       
DESCRIPTION:debug routines
CONTENTS:   dbins( )
DATE:	    Feb 13, 1988 
REVISIONS:  Jan 29, 1989 - changed msg to YmsgG.
	    Feb 01, 1989 - move all graphic routines to graphics.c
	    Mar 30, 1989 - changed tile datastructure.
	    Apr 23, 1990 - moved graph routines to library.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) debug.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
/* #include <stdarg.h> */
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>

#define CELLEST     0
#define CELLBORDER  1
#define TRUECELL    2

/* ***************************************************************** 
   DUMP CELL BIN LISTS
*/
dcellList( cell )
INT cell ;
{
    INT binX, binY, limit ;
    INT *cellList, i ;
    BINBOXPTR bptr ;

    printf("Dumping bin cell list...\n") ;

    binX = SETBINX( cellarrayG[cell]->xcenter ) ;
    binY = SETBINY( cellarrayG[cell]->ycenter ) ;
    bptr = binptrG[binX][binY] ;
    cellList = bptr->cells ;

    printf("bin:(%2d,%2d) ",binX,binY );
    printf("cellList: %0x ", cellList );
    printf("numcells: %d ", cellList[0] );
    printf("space: %d\n", bptr->space );
    printf("Cells in bin: \n ");
    limit = cellList[0] ;
    for( i=1;i<=limit;i++ ){
	printf("%d ", cellList[i] ) ; 
	if( ( i % 10 ) == 0 ){
	    printf("\n") ;
	}
    }
    printf("\n") ;

}  /* end dbins */

/* ***************************************************************** 
   CHECK CELL BIN LIST FOR CORRECT NUMBER
*/
BOOL checkbinList()
{
    INT x, y ;
    INT *cellList ;
    BINBOXPTR bptr ;
    CELLBOXPTR ptr ;
    INT numcells_in_bin = 0 ;
    INT i ;
    INT cell ;
    INT celltype ; 
    BOOL found ;

    for( y=0;y<=maxBinYG;y++ ){
	for( x=0;x<=maxBinXG;x++ ){
	    bptr = binptrG[x][y] ;
	    cellList = bptr->cells ;
	    numcells_in_bin += ABS( cellList[0] ) ;
	}
    }
    if( numcells_in_bin != numcellsG ){
	return(FALSE) ;
    }
    /* now test to see if all the cells are in their correct bins */
    for( cell = 1; cell<= endsuperG; cell++ ){
	ptr = cellarrayG[cell] ;
	celltype = ptr->celltype ;
	if( celltype != CUSTOMCELLTYPE && celltype != SOFTCELLTYPE ){
	    continue ;
	}
	x = SETBINX( ptr->xcenter ) ;
	y = SETBINY( ptr->ycenter ) ;
	bptr = binptrG[x][y] ;
	cellList = bptr->cells ;
	found = FALSE ;
	for( i = 1; i <= cellList[0] ; i++ ){
	    if( cellList[i] == ptr->cellnum ){
		found = TRUE ;
		break ;
	    }
	}
	if( !found ){
	    sprintf( YmsgG, "Problem with cell:%d bins\n" ) ;
	    M( ERRMSG, "checkbinList", YmsgG ) ;
	    return(FALSE) ;
	}
    }
    return( TRUE ) ;

}  /* end checkbinList */

/* ***************************************************************** 
   DUMP CELL BIN LISTS
*/
dbinList( flag )
INT flag ;
{
    INT x, y, limit ;
    INT *cellList, i ;
    BINBOXPTR bptr ;

    printf("Dumping bin cell list...\n") ;

    for( y=0;y<=maxBinYG;y++ ){
	for( x=0;x<=maxBinXG;x++ ){
	    bptr = binptrG[x][y] ;
	    cellList = bptr->cells ;

	    if( flag ){ /* prINT all bins */

		printf("bin:(%2d,%2d) ",x,y );
		printf("cellList: %0x ", cellList );
		printf("numcells: %d ", cellList[0] );
		printf("space: %d ", bptr->space );
		printf("cells: ");
		limit = cellList[0] ;
		for( i=1;i<=limit;i++ ){
		    printf("%2d ", cellList[i] ) ; 
		    if( ( i % 10 ) == 0 ){
			printf("\n") ;
		    }
		}
		printf("\n") ;
	    } else { /* print non empty bins */
		if( cellList[0] ){
		    printf("bin:(%2d,%2d) ",x,y );
		    printf("cellList: %0x ", cellList );
		    printf("numcells: %d ", cellList[0] );
		    printf("space: %d ", bptr->space );
		    printf("cells: ");
		    limit = cellList[0] ;
		    for( i=1;i<=limit;i++ ){
			printf("%2d ", cellList[i] ) ; 
			if( ( i % 10 ) == 0 ){
			    printf("\n") ;
			}
		    }
		    printf("\n") ;
		}
	    }
	}
    }

}  /* end dbins */

/* ***************************************************************** 
   DUMP OVERLAP BIN STRUCTURE
*/
dbins( flag )
BOOL flag ;
{

    INT x, y ;
    INT cost ;
    BINBOXPTR bptr;

    if( flag ){
	    
	printf("Dumping bins...\n") ;
	printf(" x  y   left          right       bottom       top     pen    nupen \n") ;
	for( y=0;y<=maxBinYG;y++ ){
	    for( x=0;x<=maxBinXG;x++ ){
		bptr = binptrG[x][y] ;
		printf("%2d %2d %11d %10d %11d %10d:%6d %6d\n",
		x, y, 
		bptr->left,
		bptr->right,
		bptr->bottom,
		bptr->top,
		bptr->penalty,
		bptr->nupenalty ) ;
	    }
	}

    }
    /* now calculate the penalties */
    cost = 0 ;
    for( x= 0 ; x <= maxBinXG; x++ ) {
	for( y = 0 ; y <= maxBinYG; y++ ) {
	    bptr = binptrG[x][y] ;
	    cost += ABS( bptr->penalty ) ;
	}
    }
    printf("cost = %d\n", cost ) ;

    cost = 0 ;
    for( x= 0 ; x <= maxBinXG; x++ ) {
	for( y = 0 ; y <= maxBinYG; y++ ) {
	    bptr = binptrG[x][y] ;
	    cost += ABS( bptr->nupenalty ) ;
	}
    }
    printf("nucost = %d\n", cost ) ;
}  /* end dbins */

/* ***************************************************************** 
   DUMP MOVEBOX STRUCTURE
*/
dmove()
{

INT i;

printf("Dumping movebox records...") ;
for( i=0 ; i<= 1; i++ ){
    printf("\ntileptr %d:\n",i ) ;
    printf("          old_apos0        new_apos0         old_bpos0            new_bpos0\n") ;
    printf("xcenter    %6d           %6d            %6d         %6d\n",
    old_aposG[i]->xcenter,new_aposG[i]->xcenter,old_bposG[i]->xcenter,new_bposG[i]->xcenter) ;
    printf("ycenter    %6d           %6d            %6d         %6d\n",
    old_aposG[i]->ycenter,new_aposG[i]->ycenter,old_bposG[i]->ycenter,new_bposG[i]->ycenter) ;
    printf("cell       %6d           %6d            %6d         %6d\n",
    old_aposG[i]->cell,new_aposG[i]->cell,old_bposG[i]->cell,new_bposG[i]->cell) ;
    printf("orient     %6d           %6d            %6d         %6d\n",
    old_aposG[i]->orient,new_aposG[i]->orient,old_bposG[i]->orient,new_bposG[i]->orient) ;
    printf("aspChange  %6d           %6d            %6d         %6d\n",
    old_aposG[i]->loaded_previously,new_aposG[i]->loaded_previously,
    old_bposG[i]->loaded_previously,new_bposG[i]->loaded_previously) ;
    printf("l          %6d           %6d            %6d         %6d\n",
    old_aposG[i]->l,new_aposG[i]->l,old_bposG[i]->l,new_bposG[i]->l) ;
    printf("r          %6d           %6d            %6d         %6d\n",
    old_aposG[i]->r,new_aposG[i]->r,old_bposG[i]->r,new_bposG[i]->r) ;
    printf("b          %6d           %6d            %6d         %6d\n",
    old_aposG[i]->b,new_aposG[i]->b,old_bposG[i]->b,new_bposG[i]->b) ;
    printf("t          %6d           %6d            %6d         %6d\n",
    old_aposG[i]->t,new_aposG[i]->t,old_bposG[i]->t,new_bposG[i]->t) ;
    printf("binL       %6d           %6d            %6d         %6d\n",
    old_aposG[i]->binL,new_aposG[i]->binL,old_bposG[i]->binL,new_bposG[i]->binL) ;
    printf("binR       %6d           %6d            %6d         %6d\n",
    old_aposG[i]->binR,new_aposG[i]->binR,old_bposG[i]->binR,new_bposG[i]->binR) ;
    printf("binB       %6d           %6d            %6d         %6d\n",
    old_aposG[i]->binB,new_aposG[i]->binB,old_bposG[i]->binB,new_bposG[i]->binB) ;
    printf("binT       %6d           %6d            %6d         %6d\n",
    old_aposG[i]->binT,new_aposG[i]->binT,old_bposG[i]->binT,new_bposG[i]->binT) ;
} /* end for loop */
} /* end dmove */

/* ***************************************************************** 
   DUMP TILEPTR STRUCTURE
*/
dtile(cell)
INT cell ;
{

CELLBOXPTR ptr ;
TILEBOXPTR t ;

printf("Dumping tileptr records for cell %d...", cell ) ;
ptr = cellarrayG[cell] ;
for( t=ptr->tiles;t;t=t->next ){
    printf("   left        right             bottom           top\n") ;
    printf("%6d       %6d            %6d         %6d\n",
	t->left,t->right,t->bottom,t->top) ;
} /* end for loop */
} /* end dtile */

/* ***************************************************************** 
   RELOAD BINS same as loadbins make call because of dbx bug. 
   Also reinitializes nupenalty field 
*/
dloadbins( flag )
BOOL flag ;
{
    INT x, y ;

    dbins( TRUE ) ;
    loadbins( flag ) ;
    for( x = 0; x<= maxBinXG; x++){
	for( y = 0; y<= maxBinYG; y++){
	    binptrG[x][y]->nupenalty = 0 ;
	}
    }
    dbins( TRUE ) ;
}

/* ***************************************************************** 
   DUMP CURRENT CELL AREAS
*/
dcellareas()
{

printf("Area without routing area:%d\n",calc_cellareas(FALSE) ) ;
printf("Area with estimated routing area:%d\n",calc_cellareas(TRUE) ) ;

} /* end dcellareas */

#ifdef NEEDED
DORIENT( cell )
INT cell ;
{
    FILE *fp ;
    CELLBOXPTR cptr ;
    TILEBOXPTR tileptr ;
    TERMBOXPTR  termptr ;
    INT  i ;
    INT  x ;
    INT  y ;
    INT  orient ;
    INT  newx, newy ;
    INT  x0, x1, y0, y1 ;
    char filename[LRECL] ;
    char label[LRECL] ;

    system("rm -f DATA/*" ) ;
    for( i= 0; i<=7; i++ ){
	cptr = cellarrayG[cell] ;
	cptr->orient = i ;

	/* dump force results serially from processors [0][k] */
	/* first dump the objects */
	sprintf(filename,"%s/cell.file.%d",dataDir,k) ;
	if( fp = TWOPEN(filename,"w", ABORT )){ 
	    x =  cptr->xcenter ;
	    y =  cptr->ycenter ;
	    orient = cptr->orient ;
	    /* name the cell and change colors for pretty picture */
	    sprintf(label,"C%d orient:%d",i,orient) ; 
	    for( tileptr=cptr->tiles;tileptr;tileptr=tileptr->nexttile ) {

		/* first get orientation from  orientation */
		/* add cell offset */
		x0 = x + tileptr->left  ;
		x1 = x + tileptr->right ;
		y0 = y + tileptr->bottom;
		y1 = y + tileptr->top   ;

		fprintf(fp,"%d, %d, %d, %d, color: %s, label: %s\n",
		    x0,y0,x1,y1, ROWCOLOR, label ) ;


	    } /* end for loop */

	} else {
	    printf("WARNING[dumpMC]:could not open %s - dump aborted.\n",
		filename) ;
	    return ;
	} 
	fclose(fp) ;

	/* now build net file */
	/* nets are the interconnections between the cells */
	sprintf(filename,"%s/net.file.%d",dataDir,k ) ;
	if( fp = fopen(filename,"w" )){ 
	    fclose(fp) ;
	}


	/* now build pin file */
	/* pins are the terminal points on a net */
	sprintf(filename,"%s/pin.file.%d",dataDir,k ) ;
	if( fp = fopen(filename,"w" )){ 

	    /* get target position from Rotational record */
	    x = cptr->xcenter ;
	    y = cptr->ycenter ;
	    termptr = cptr->termptr ;
	    for( ; termptr != TERMNULL ; termptr = termptr->nextterm ) {
		switch( orient ){
		case 0:
		    newx = x + termptr->xpos ;
		    newy = y + termptr->ypos ;
		    break ;
		case 1:
		    newx = x + termptr->xpos ;
		    newy = y - termptr->ypos ;
		    break ;
		case 2:
		    newx = x - termptr->xpos ;
		    newy = y + termptr->ypos ;
		    break ;
		case 3:
		    newx = x - termptr->xpos ;
		    newy = y - termptr->ypos ;
		    break ;
		case 4:
		    newx = x + termptr->ypos ;
		    newy = y + termptr->xpos ;
		    break ;
		case 5:
		    newx = x - termptr->ypos ;
		    newy = y - termptr->xpos ;
		    break ;
		case 6:
		    newx = x - termptr->ypos ;
		    newy = y + termptr->xpos ;
		    break ;
		case 7:
		    newx = x + termptr->ypos ;
		    newy = y - termptr->xpos ;
		    break ;
		}

		fprintf(fp,"%d, %d, %d, %d, %d, color: %s\n",
		    i,newx-1,newy-1,newx+1,newy+1, PINCOLOR ) ;
	    }

	} else {
	    printf("ERROR[dumpMC]: could not open %s\n",filename) ;
	    return ;
	} 
	fclose(fp) ;

	/* increment  k afterwards */
	k++ ;
    }

} /* end dumpForce */

dsoftpins( cell )
INT cell ;
{

    
    INT pin ;
    PINBOXPTR  *sarray    ;  /* array of just the soft pins of a cell */

    sarray = cellarrayG[cell]->softpins ;
    for( pin = 1; pin <= cellarrayG[cell]->numsoftpins;pin++ ) {
	fprintf( stderr, " %d ", sarray[pin]->softinfo->site );
	if( pin % 15 == 0 ){
	    fprintf( stderr, "\n" ) ;
	}
    }
    fprintf( stderr, "\n" ) ;
}
#endif
