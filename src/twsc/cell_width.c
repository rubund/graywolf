/*
 *   Copyright (C) 1991 Yale University
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
FILE:	    calc_cells_width.c                                       
DESCRIPTION:calculates the width that encompasses most of the cells 
            in the stdcell.cel file
CONTENTS:   calc_cell_width()
	    
	    
DATE:	    July 26, 1991 
----------------------------------------------------------------- */

#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) cell_width.c (Yale) version 1.1 9/27/91" ;
#endif
#endif

#define PADKEYWORD          "pad"
#define RELATIVE_TO_CURPOS  1

/* #define MITLL */

#include <string.h>
#include "standard.h"
#include "main.h"
#include "config.h"
#include "readpar.h"
#include "parser.h"
#include <yalecad/debug.h>
#include <yalecad/message.h>
#include <yalecad/rbtree.h>
#include <yalecad/string.h>

#define GREATER 1
#define LESS -1 
#define EQUAL 0
#define MAX_CELL_LEN 10


extern INT extra_cellsG ;

static INT compare_cell_length();
static read_pads(); 



/*--------------------------------------------------------------------------*/
/*------------ User defined print routine to print out the tree ------------*/
/*--------------------------------------------------------------------------*/

INT print_cell_name(c1)
CBOXPTR c1;
{
 return;
}


INT print_cell_length(c1)
CBOXPTR c1;
{
  return;
}


INT print_cell_pins(c1)
CBOXPTR c1;
{
  return ;
}


/*--------------------------------------------------------------------------*/




calc_cells_width()
{

FILE *fp;     /*--- file pointer to stdcell.comp ---*/
INT cell, std_length, cell_length = 0 ;
INT Ratio, sect, xpost,xminus, ypost, yminus, corient;
INT alength, lower, upper, counter, longest_cell, cell_count, pp;
DOUBLE avg_cell_length, deviation, percent, ratio, part_pin;

YTREEPTR CellTree; 
CBOXPTR acellptr, dummy_box, Low_Key, Hi_Key, TestTree;
PINBOXPTR termptr, netptr;
EQ_NBOXPTR eptr;
DBOXPTR dptr;
char *aptr, *bptr, *cptr, *nptr;
char *add_ptr = "-XTRA", *add_fptr = "-1";
INT xx , yy, zzz, r_term, last_term, pin_diff ;
char layer;

system("clear");
fprintf(stderr, "\n\n ");

/*------ Memory allocation ------*/
dummy_box   = (CBOXPTR) Ysafe_malloc( sizeof(CBOX) );
Low_Key     = (CBOXPTR) Ysafe_malloc( sizeof(CBOX) );
Hi_Key      = (CBOXPTR) Ysafe_malloc( sizeof(CBOX) );
TestTree    = (CBOXPTR) Ysafe_malloc( sizeof(CBOX) );


/*------ Initialize and build the tree ------*/
CellTree = Yrbtree_init(compare_cell_length);
for(cell = 1; cell <= numcellsG - extra_cellsG ; cell++){

   acellptr = carrayG[cell];                /*----Get the pointer----*/
   cell_length += carrayG[cell]->clength;   /*---Total cell length---*/

   Yrbtree_insert(CellTree, acellptr);      /*----insert the cell-----*/
   ASSERTNFAULT(Yrbtree_verify(CellTree),"calc_cells_name","bad tree");
  }  

  /* fprintf(stderr,"\n\n The RB-Tree of All The Cells Follows: \n\n  ");

   Yrbtree_dump(CellTree,print_cell_name);     printout the whole tree--*/


/*--------- Calculate the avarage cell length and deviation ---------*/ 
 avg_cell_length = cell_length / (numcellsG - extra_cellsG) ;

 for(cell = 1; cell <= numcellsG - extra_cellsG ; cell++){
   deviation += (carrayG[cell]->clength - avg_cell_length)
              * (carrayG[cell]->clength - avg_cell_length) ;
 }
 deviation = sqrt (deviation / (numcellsG - extra_cellsG) ) ;
/*--------------------------------------------------------------------*/


fprintf(stderr, "\n------------------------------------\n");
fprintf(stderr, "The total number of cells = %d \n", --cell );
fprintf(stderr, "The avarage cell length = %f \n", avg_cell_length );
fprintf(stderr, "The cell deviation = %f", deviation );
fprintf(stderr,"\n------------------------------------");



/*-------------- Set the limits from "0" to "avg_cell_length" -------------*/
/* ----------- Get the count of all the cells within that range ------------ */
Low_Key->clength = 0;
Hi_Key->clength = avg_cell_length;
upper = Hi_Key->clength ;

fprintf(stderr, "\n");
counter = 0;

for(TestTree = (CBOXPTR) Yrbtree_interval(CellTree,Low_Key,Hi_Key,TRUE);
    TestTree ;
    TestTree = (CBOXPTR) Yrbtree_interval(CellTree,Low_Key,Hi_Key,FALSE)) {
counter ++;
}

percent =  (counter / (float) (numcellsG - extra_cellsG)) ;


/*--------------------------------------------------------------------------*/
/*----------- For cells within the limit, clength = avg_cell_length --------*/
/*--------------------------------------------------------------------------*/

if (percent >= 0.6){

fprintf(stderr,"\n\nThe Total Number of Cells b/w the Lower Limit = %d",lower);
fprintf(stderr,"\nand the Upper Limit = %d is ----->> %d \n", upper, counter);
fprintf(stderr,"\nThe Percentage of Cells within the Limits = %f \n", 100*percent);
fprintf(stderr,"\nPlease wait, conversion is being done ... \n");

Hi_Key->clength = avg_cell_length ;

} else {
   
  while(percent < 0.6){

    Hi_Key->clength += (INT) deviation / 20 ; 
    upper = Hi_Key->clength ; 

    counter = 0;
    for(TestTree = (CBOXPTR)Yrbtree_interval(CellTree,Low_Key,Hi_Key,TRUE);
	TestTree ;
	TestTree = (CBOXPTR)Yrbtree_interval(CellTree,Low_Key,Hi_Key,FALSE)) {
      counter ++;
    }

    percent =  (counter / (float) (numcellsG - extra_cellsG)) ;

  }/*--- end while ---*/


  for(TestTree = (CBOXPTR)Yrbtree_interval(CellTree,Low_Key,Hi_Key,TRUE);
    TestTree ;
    TestTree = (CBOXPTR)Yrbtree_interval(CellTree,Low_Key,Hi_Key,FALSE)) {

    TestTree->clength = Hi_Key->clength;



  }

fprintf(stderr,"\n\nThe Total Number of Cell b/w the Lower Limit = %d",lower);
fprintf(stderr,"\nand the  Upper Limit = %d is ----->> %d \n", upper, counter);
fprintf(stderr,"\nThe Percentage of Cells within the Limits = %f \n", 100*percent);
fprintf(stderr,"\nPlease wait, conversion is being done ... \n");

}/*---- end else ----*/


std_length = Hi_Key->clength ;

/*---------------------------------------------------------------*/
/*------------ Now copy the adjusted cells to a file ------------*/
/*---------------------------------------------------------------*/


fp = TWOPEN ("stdcell.comp", "w", ABORT);
cell_count = 0;

for(TestTree = (CBOXPTR)Yrbtree_interval(CellTree,Low_Key,Hi_Key,TRUE);
    TestTree ;
    TestTree = (CBOXPTR)Yrbtree_interval(CellTree,Low_Key,Hi_Key,FALSE)) {


/*------ setting up the parameters to be passed on to the (fp) --------*/
  TestTree->clength = Hi_Key->clength ;
  TestTree->tileptr->left  = - Hi_Key->clength / 2 ;
  TestTree->tileptr->right =   Hi_Key->clength / 2 ;


  cell_count ++;
  print_cell_length(TestTree);   /*--- printout the cell's length ---*/


  

  fprintf(fp,"cell %d  %s  \n", cell_count, TestTree->cname);
  fprintf(fp,"left  %d  right  %d", TestTree->tileptr->left,TestTree->tileptr->right);
  fprintf(fp,"  bottom  %d  top  %d \n", TestTree->tileptr->bottom,TestTree->tileptr->top);



  xx = 100 ;

 for(termptr = TestTree->pins; termptr; termptr = termptr->nextpin) {

   /*--- putting the y-pins on both sides ---*/
   /*if ((termptr->typos[0] != 0) && (termptr->typos[1] == 0)) { 
     termptr->typos[1] =  - termptr->typos[0] ;
   } else if ((termptr->typos[0] == 0) && (termptr->typos[1] != 0)) { 
            termptr->typos[0] =  - termptr->typos[1] ;
	  } else */

   if ((termptr->typos[0] == 0) && (termptr->typos[1] == 0)){
	         termptr->typos[0] = TestTree->tileptr->top ;
		 termptr->typos[1] = TestTree->tileptr->bottom ;
   }	       
   corient = TestTree->corient ;
   xpost = termptr->txpos[corient / 2] ;
   ypost = termptr->typos[corient % 2] ;

   /*
   xminus = termptr->txpos[1] ;
   yminus = termptr->typos[1] ;   
   */

   aptr = termptr->pinname ;
   aptr++ ;
   layer = *(aptr) ;
   aptr++ ;
 
   dptr = netarrayG[termptr->net] ;
   bptr = dptr->name ;

   eptr = termptr->eqptr ;
   if (eptr){
    /*-- if the equivalent pin exists, print it out too --*/
    cptr = eptr->pinname ;
    yminus = eptr->typos ;
    cptr++ ;
    layer = *(cptr) ;
    cptr++ ;

   fprintf(fp,"pin name %s  signal %s  layer %c    %d  %d \n", aptr , bptr, layer,xpost, -yminus);

    fprintf(fp,"    equiv name %s  layer %c    %d  %d \n", cptr, layer, xpost, yminus);
   } else  {
   fprintf(fp,"pin name %s  signal %s  layer %c    %d  %d \n", aptr , bptr, layer,xpost,ypost);
     }   

      fprintf(fp,"pin name %d signal TW_PASS_THRU  layer %c   %d  %d \n",xx,layer, xpost + 3 , ypost);
   
   fprintf(fp,"    equiv name %d  layer %c  %d  %d \n",++xx,layer, xpost + 3 , - ypost); 

   xx++ ;

 }/*--end for-loop--*/

fprintf(fp, "\n\n");
}


longest_cell = MAX_CELL_LEN;
/*----------- Get the maximum cell length now -------------*/
 for(cell = 1; cell <= numcellsG - extra_cellsG ; cell++){

   if (carrayG[cell]->clength > longest_cell){
     longest_cell = carrayG[cell]->clength ;
     }
 }
/*---------------------------------------------------------*/



/*---------------------------------------------------------------------------
 -----------  Now adjust the length of the oversized cells ------------------
 ---------------------------------------------------------------------------*/
if (percent != 1.0 ){
Low_Key->clength = ++Hi_Key->clength ;
Hi_Key->clength  = longest_cell;

cell_count = 100;
counter = 0;

for(TestTree = (CBOXPTR)Yrbtree_interval(CellTree,Low_Key,Hi_Key,TRUE);
    TestTree ;
    TestTree = (CBOXPTR)Yrbtree_interval(CellTree,Low_Key,Hi_Key,FALSE)) {
    counter++;
    cell_count++ ;
    
    lower = TestTree->numterms ;
    upper = TestTree->clength ;
    cell =  (numcellsG - extra_cellsG) + 1 ;



/*--------------- find out how many times we have to break it ------------*/
/*  TestTree->clength = lower + 1 ;  left out for testing  ----  */ 

if ( upper > std_length){
   ratio = (float) TestTree->clength / std_length ;
   Ratio = (INT) ratio ;
   if((ratio - Ratio) > 0){
   Ratio++ ;
   }

 pin_diff = TestTree->clength - std_length ;
 termptr = TestTree->pins ;    
 nptr = TestTree->cname ;   /*--- concatenate the name ---*/
 part_pin = lower / (float) Ratio ; /*--calculate the pins per partition--*/   


 pp   = (INT) part_pin ;
 if ((part_pin - pp) > 0){
   pp++ ;
 } 

 r_term = (pp * Ratio) - lower ;

for(sect = 0; sect < Ratio ; sect++){ 

  nptr = strcat(nptr,add_ptr);

  dummy_box = carrayG[cell++] ;         /*-- get a dummy structure --*/

  /*---- distributing the pins, the last section will have only one pin ---*/
  if (sect < Ratio - 1) {
  dummy_box->numterms = pp  ;     /*-- get # of pins on the sub_cells-*/ 
  } else {  -- pp ;
            if ((part_pin - pp) > 0) {
	    last_term = ++pp - r_term ;  
            dummy_box->numterms = last_term ;
	  } else dummy_box->numterms = pp ;
    }/*-end else-*/
  /*-----------------------------------------------------------------------*/  


  dummy_box->clength = std_length;      /*-- force the length to std_cell ---*/
  dummy_box->cname   = nptr ;           /*-- concatenate the name ---*/
  dummy_box->tileptr->left  = - std_length / 2; /*-- adjust left --*/
  dummy_box->tileptr->right =   std_length / 2; /*--adjust right --*/

  /*---------------- output the partition on stdcell.comp ----------------*/
  
  if ( dummy_box->numterms == 0) {
       continue ;
  }

       fprintf(fp,"cell %d  %s  \n", cell_count, dummy_box->cname);
       fprintf(fp,"left  %d  right  %d", dummy_box->tileptr->left,dummy_box->tileptr->right);
       fprintf(fp,"  bottom  %d  top  %d \n", dummy_box->tileptr->bottom,dummy_box->tileptr->top);
     

  xx = 100 ;
  
  /*-------------- trying to print the pins in the dummy_box ---------------*/
    for(counter = 0; counter < dummy_box->numterms; counter++){


      if ((termptr->typos[0] == 0) && (termptr->typos[1] == 0)){
	         termptr->typos[0] = TestTree->tileptr->top ;
		 termptr->typos[1] = TestTree->tileptr->bottom ;
      } 
	       
      corient = TestTree->corient ;
      xpost = termptr->txpos[corient / 2] ;

      /*-- fixing for out of space pins --*/
      for ( zzz = 0; zzz < Ratio ; zzz++) {
            if (abs(xpost) >= ((zzz + 1) * (std_length / 2 ) ) ){
                if (abs(xpost) == ((zzz + 1) * (std_length / 2 ) ) ){

	            if (xpost > 0){
		        xpost = xpost - (( zzz + 1 ) * ( std_length / 2 )) ;
	             } else { 
		        xpost = xpost + (( zzz + 1 ) * ( std_length / 2 )) ;
	            }
	         } else {
	            if (xpost > 0){
		        xpost = xpost - (( zzz + 2 ) * ( std_length / 2 )) ;
	            } else { 
		        xpost = xpost + (( zzz + 2 ) * ( std_length / 2 )) ;
		    }
		 }

	      }
       }

      ypost = termptr->typos[corient % 2] ;

      aptr = termptr->pinname ;
      aptr++ ;
      layer = *(aptr) ;
      aptr++ ;

      dptr = netarrayG[termptr->net] ;
      bptr = dptr->name ;


      eptr = termptr->eqptr ; /*-- get the equivalent pin --*/
      if (eptr){ /*-- if the equivalent pin exists, print it out --*/
       cptr = eptr->pinname ;
       yminus = eptr->typos ;


      if (abs(yminus) >= std_length){
	if (yminus > 0){
	  yminus = yminus - std_length ;
	} else { 
	  yminus = yminus + std_length ;
	}
      }



       cptr++ ;
       layer = *(cptr) ;      /*-- get the equivalent pin number --*/
       cptr++ ;
    
     fprintf(fp,"pin name %s  signal %s  layer %c    %d  %d\n", aptr , bptr, layer, xpost, - yminus);
     fprintf(fp,"    equiv name %s  layer %c    %d  %d\n", cptr, layer, xpost, yminus);
      } else {
	      fprintf(fp,"pin name %s  signal %s  layer %c    %d  %d\n", aptr , bptr, layer, xpost, yminus);
	    }

      termptr = termptr->nextpin ;
   
   fprintf(fp,"pin name %d signal TW_PASS_THRU  layer %c   %d  %d \n",xx,layer, xpost + 3 , ypost);
   fprintf(fp,"    equiv name %d  layer %c  %d  %d \n",++xx,layer, xpost + 3 , - ypost); 

      xx++ ;

   }/*---end the inner for-statement---*/

      xx++ ;
      fprintf(fp,"pin name %d signal TW_PASS_THRU  layer %c   %d  %d \n",xx,layer, xpost + 1 , ypost);

      fprintf(fp,"    equiv name %d  layer %c  %d  %d \n",++xx,layer, xpost + 1 , - ypost); 



   fprintf(fp,"\n\n");
  }/*----end for-statement---*/
 
} 
  
} /*----- for the outer for-loop -----*/

}/*---end if percent ---*/


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            Transferring the Pads info as it is to the stdcell.comp
 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



   read_pads( fp );
   TWCLOSE( fp );

return;

}


/***************************************************************************/
 
static INT compare_cell_length(c1,c2)

CBOXPTR c1;
CBOXPTR c2;

{

if (c1->clength > c2->clength){
  return(GREATER);
} else if (c1->clength < c2->clength){
        return(LESS);
      }
return(EQUAL);
}

/****************************************************************************/


static read_pads( fp ) 
FILE *fp ;
{
    char buffer[LRECL], *bufferptr ;
    char **tokens ;     /* for parsing menu file */
    char copyBuf[LRECL] ;
    INT  numtokens ;
    int  delta ;
    int  error ;
    int lineG ;
    FILE *fp_cell ;


    fp_cell = TWOPEN ("stdcell.cel","r" , ABORT) ;

    while( bufferptr = fgets(buffer,LRECL,fp_cell )){
	/* make copy because scanner adds EOS characters */
	strcpy( copyBuf, bufferptr ) ;
	tokens = Ystrparser( bufferptr, " \t\n", &numtokens );

	if( numtokens == 0 ){
	  /*-- skip over the blank lines --*/
	    continue ;
	}


  /*-------------------------------------------------------------------*/
  /*----- Once the Pad is detected, throw in the rest of the file -----*/
  /*-------------------------------------------------------------------*/ 
	if( strcmp( tokens[0], PADKEYWORD ) == STRINGEQ){
	  fprintf( fp, "%s", copyBuf ) ; 
          while( bufferptr = fgets(buffer,LRECL,fp_cell )){
        	strcpy( copyBuf, bufferptr ) ;
	        fprintf( fp, "%s", copyBuf ) ;
	      } 
	  break ;
	}

    } /* end the big while loop */

    /* now we need to unread the last line */
    delta = - strlen(copyBuf) ;
    error = fseek( fp, delta, RELATIVE_TO_CURPOS ) ;

    if( error == -1 ){
	M( ERRMSG, "readcells", "can't jump backwards in file\n" ) ;
    }
} /* end read_pads */
