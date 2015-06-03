/*
 *   Copyright (C) 1990-1992 Yale University
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
   FILE:	    deck.c                                       
   DESCRIPTION:This file contains the routines to handle decks
   CONTENTS:
   YDECK *Ydeck_init( ) 
   Ydeck_push( deck, node ) 
   YDECK *deck ;
   char *node ;
   Ydeck_enqueue( deck, node ) 
   YDECK *deck ;
   char *node ;
   char *Ydeck_pop( deck )
   char *Ydeck_dequeue( deck )
   YDECKPTR Ydeck_notEmpty( deck ) 
   YDECK *deck ;
   Ydeck_free ( deck , userfree )
   YDECK *deck;
   userfree is memory deallocation routine
   Ydeck_empty ( deck , userfree )
   YDECK *deck;
   userfree is memory deallocation routine                
   Ydeck_dump( deck ) 
   YDECK *deck ;
   DATE:	    Mar 16, 1990 
		    Apr 30, 1990 - fixed something.
		    Fri Feb 22 23:34:38 EST 1991 - fixed Ydeck_pop 
			Ydeck_dequeue when queue is empty.
   ----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) deck.c (Yale) version 1.17 1/22/92" ;
#endif

#include <yalecad/deck.h>

static YCARD deckEndS;

/*-----------------*/
/* initialize deck */
/*-----------------*/
YDECK *Ydeck_init( ) 
{   
  YDECKPTR deckPtr ;
  
  deckPtr = (YDECKPTR) YMALLOC( 1, YDECK );
  deckPtr->top= &deckEndS;              /* init to point to sentinal */
  deckPtr->bottom= &deckEndS;           /* init to point to sentinal */
  deckPtr->current= &deckEndS;          /* init to point to sentinal */
  deckPtr->size=0;
  return(deckPtr);  
} /* end Ydeck_init */


/*-------------------------------------*/
/* add a vertex to the top of the deck */
/*-------------------------------------*/
VOID Ydeck_push( deckPtr, data ) 
     YDECK *deckPtr ;
     VOIDPTR data ;
{
  YCARDPTR cardPtr;
  
  cardPtr= YMALLOC( 1, YCARD );  /* allocate a new card */
  cardPtr->data=data;                   /* attach users data to new card */
  
  if (deckPtr->size) {       /* deck is not empty */
    cardPtr->down=deckPtr->top;
    cardPtr->up= &deckEndS;
    deckPtr->top->up=cardPtr;
    deckPtr->top=cardPtr;
  } else {                   /* deck was empty */
    deckPtr->top=cardPtr;    /* the new card is the top and bottom */  
    deckPtr->bottom=cardPtr;
    cardPtr->up= &deckEndS;
    cardPtr->down= &deckEndS;
  }
  
  deckPtr->size++;
} /* end Ydeck_push */

/*----------------------------------------*/
/* add a vertex to the bottom of the deck */
/*----------------------------------------*/
VOID Ydeck_enqueue( deckPtr, data ) 
     YDECK *deckPtr ;
     VOIDPTR data ;
{
  YCARDPTR cardPtr;
  
  cardPtr= YMALLOC( 1, YCARD );  /* allocate a new card */
  cardPtr->data=data;                   /* attach users data to new card */
  
  if (deckPtr->size) {       /* deck is not empty */
    cardPtr->up=deckPtr->bottom;
    cardPtr->down= &deckEndS;
    deckPtr->bottom->down=cardPtr;
    deckPtr->bottom=cardPtr;
  } else {                   /* deck was empty */
    deckPtr->top=cardPtr;    /* the new card is the top and bottom */  
    deckPtr->bottom=cardPtr;
    cardPtr->up= &deckEndS;
    cardPtr->down= &deckEndS;
  }
  
  deckPtr->size++;
} /* end Yadd2Deck */

/*------------------------------*/
/* remove card from top of deck */
/*------------------------------*/
VOIDPTR Ydeck_pop(deckPtr)
     YDECK *deckPtr;
{
  YCARDPTR cardPtr;
  VOIDPTR data;

  if( deckPtr->size <= 0 ){
      return( NIL(VOIDPTR) ) ;
  }
  
  cardPtr=deckPtr->top;
  data=cardPtr->data;
  
  deckPtr->top=cardPtr->down;
  cardPtr->down->up= &deckEndS;
  
  if ( (deckPtr->current = deckPtr->top) == &deckEndS) {
   deckPtr->bottom= &deckEndS;
  }

  YFREE(cardPtr);
  deckPtr->size--;
  return(data);
}  /* end Ydeck_pop */

/*---------------------------------*/
/* remove card from bottom of deck */
/*---------------------------------*/
VOIDPTR Ydeck_dequeue(deckPtr)
     YDECK *deckPtr;
{
  YCARDPTR cardPtr;
  VOIDPTR data;
  
  if( deckPtr->size <= 0 ){
      return( NIL(VOIDPTR) ) ;
  }
  
  cardPtr=deckPtr->bottom;
  data=cardPtr->data;
  
  deckPtr->bottom=cardPtr->up;
  cardPtr->up->down= &deckEndS;
  
  if ( (deckPtr->current = deckPtr->bottom) == &deckEndS) {
   deckPtr->top= &deckEndS;
  }

  YFREE(cardPtr);
  deckPtr->size--;
  return(data);
}  /* end Ydeck_dequeue */


/*--------------------------*/
/* check status of the deck */
/*--------------------------*/
INT Ydeck_notEmpty( deckPtr ) 
     YDECK *deckPtr ;
{  
  return( deckPtr->size ) ;
}

/*--------------------------------*/
/* remove all cards from the deck */
/*--------------------------------*/
VOID Ydeck_empty(deckPtr,userDelete)
     VOID (*userDelete)();
     YDECK *deckPtr;
{
  VOIDPTR deckData;

  if (userDelete) {   
    for (Ydeck_top(deckPtr);Ydeck_notEnd(deckPtr);) {
      deckData=Ydeck_pop(deckPtr);
      userDelete(deckData);
    }
  } else {
    for (Ydeck_top(deckPtr);Ydeck_notEnd(deckPtr);) {
      Ydeck_pop(deckPtr);
    }
  }
}

/*-----------------------------------------*/
/* free the deck and all cards it contains */
/*-----------------------------------------*/
VOID Ydeck_free(deckPtr,userDelete)
     YDECK *deckPtr;
     VOID (*userDelete)();
{
  Ydeck_empty(deckPtr,userDelete);
  YFREE(deckPtr);
}

/*-------------------------------------------------*/
/* debug function to dump the contents of the deck */
/*-------------------------------------------------*/
VOID Ydeck_dump( deckPtr, userPrint ) 
     YDECK *deckPtr ;
     VOID (*userPrint)();
{
  
  fprintf( stderr,"%d cards\n",deckPtr->size);
  fprintf( stderr,"Deck top to bottom:" ) ;
  for ( Ydeck_top(deckPtr); Ydeck_notEnd(deckPtr); Ydeck_down(deckPtr) ) {
    if (userPrint) {
      (*userPrint)(Ydeck_getData(deckPtr));
    } else {
      fprintf( stderr,"%d ", (INT)Ydeck_getData(deckPtr) ) ;
    }
  }
  fprintf( stderr,"\n" ) ;  
  fprintf( stderr,"Deck bottom to top:" ) ;
  for ( Ydeck_bottom(deckPtr); Ydeck_notEnd(deckPtr); Ydeck_up(deckPtr)) {
    if (userPrint) {
      (*userPrint)(Ydeck_getData(deckPtr));
    } else {
      fprintf( stderr,"%d ", (INT) Ydeck_getData(deckPtr) ) ;
    }
  }
  fprintf( stderr,"\n" ) ;  
  
} /* end Ydeck_dump */

/*-----------------------------------------------------------------------------*/
/* returns address of sentinel. This makes sentinal address available to users */
/*-----------------------------------------------------------------------------*/
YCARD *Ydeck_sentinel()
{
  return(&deckEndS);
}

/*-----------------------
  exercise deck pointer 
 ------------------------*/
  int Ydeck_verify(deck)
  YDECK *deck;
{
  static char routineNameS[LRECL] = "Ydeck_verify";
  YCARDPTR saveCurrent;
  INT count1 = 0;
  INT count2 = 0;
  int rc = TRUE;

  /* save the decks current position */
  saveCurrent = deck->current;

  /* verify memory allocation of deck */  
  if ( YcheckDebug(deck) < sizeof(YDECK) ) {
    M( ERRMSG, routineNameS, "deck memory is corrupt\n");
    rc = FALSE;
  }

  /* walk deck top to bottom. verify memory allocation and count cards */
  for ( Ydeck_top(deck); Ydeck_notEnd(deck); Ydeck_down(deck) ) {
    count1 ++;
    if ( YcheckDebug(deck->current) < sizeof(YCARD) ) {
      M( ERRMSG, routineNameS, "deck has card with corrupt memory\n");
      rc = FALSE;
    }
    /* check pointer above current card */
    if ( deck->current->up != &deckEndS ) {
      if ( YcheckDebug(deck->current->up) < sizeof(YCARD) ) {
	M( ERRMSG, routineNameS, "card up is currupt\n");
	rc = FALSE;
      }
    }
    /* check pointer above below card */
    if ( deck->current->down != &deckEndS ) {
      if ( YcheckDebug(deck->current->down) < sizeof(YCARD) ) {
	M( ERRMSG, routineNameS, "card down is currupt\n");
	rc = FALSE;
      }
    }
  }
  
  /* walk deck bottom to top. verify memory allocation and count cards */
  for ( Ydeck_bottom(deck); Ydeck_notEnd(deck); Ydeck_up(deck) ) {
    count2 ++;
    if ( YcheckDebug(deck->current) < sizeof(YCARD) ) {
      M( ERRMSG, routineNameS, "deck has card with corrupt memory\n");
      rc = FALSE;
    }
    /* check pointer above current card */
    if ( deck->current->up != &deckEndS ) {
      if ( YcheckDebug(deck->current->up) < sizeof(YCARD) ) {
	M( ERRMSG, routineNameS, "card up is currupt\n");
	rc = FALSE;
      }
    }
    /* check pointer above below card */
    if ( deck->current->down != &deckEndS ) {
      if ( YcheckDebug(deck->current->down) < sizeof(YCARD) ) {
	M( ERRMSG, routineNameS, "card down is currupt\n");
	rc = FALSE;
      }
    }
  }
  
  /* check card counts */
  if ( count1 != count2 ||  count1 != Ydeck_size(deck) ) {
    M( ERRMSG, routineNameS, "deck size is inconsistent with card count\n");
    rc = FALSE;
  }

  /* make sure top and bottom cards point to correct sentinel */
  if ( deck->size >= 1 ) {
    /* a top card exists, card above should be sentinel */
    if ( deck->top->up != &deckEndS ) {
      M( ERRMSG, routineNameS, "card above top card is not sentinel\n");
      rc = FALSE;
    }

    /* a bottom card exists, card above should be sentinel */
    if ( deck->bottom->down != &deckEndS ) {
      M( ERRMSG, routineNameS, "card below bottom card is not sentinel\n");
      rc = FALSE;
    }
  }

  /* restore the decks current position */
  deck->current = saveCurrent;

  return(rc);
}
