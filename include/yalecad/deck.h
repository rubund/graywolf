/* ----------------------------------------------------------------- 
DESCRIPTION:Insert file for deck library routines.  Add & subtract 
members from a top/bottom of doubly-linked list. Multiple decks may 
occur simultaneously.  The complexity of the routines are as follows.
    Ydeck_top         : O(1) 
    Ydeck_bottom      : O(1)
    Ydeck_up          : O(1)
    Ydeck_down        : O(1)
    Ydeck_pop         : O(1)
    Ydeck_push        : O(1)
    Ydeck_enqueue     : O(1)
    Ydeck_dequeue     : O(1)
    Ydeck_getTop      : O(1)
    Ydeck_getBottom   : O(1)
    Ydeck_getData     : O(1)
    Ydeck_size        : O(1)
    Ydeck_init        : O(1)
    Ydeck_empty       : O(n)
    Ydeck_free        : O(n)
----------------------------------------------------------------- */
#ifndef DECK_H
#define DECK_H

#ifndef lint
static char YdeckId[] = "@(#) deck.h (Yale) version 1.16 1/22/92" ;
#endif

#include <yalecad/base.h>
#include <yalecad/message.h>

/* *********** data structures *************** */
typedef struct Ydeck_info {
    VOIDPTR  data;
    struct Ydeck_info *up ;
    struct Ydeck_info *down ;
} YCARD, *YCARDPTR  ;

typedef struct {
    INT       size ;            /* size of list */
    YCARDPTR  top ;
    YCARDPTR  bottom ;
    YCARDPTR  current ;
} YDECK, *YDECKPTR ;

/* *********** list macros ***************** */

#define Ydeck_up( deck_xyz )    (deck_xyz->current = deck_xyz->current->up)
/* 
Arguments:
    YDECK *deck_xyz ;
Function:
    Returns element above the current element in the deck.
*/

#define Ydeck_down( deck_xyz )    (deck_xyz->current = deck_xyz->current->down)
/* 
Arguments:
    YDECK *deck_xyz ;
Function:
    Returns the element below the current element in the deck.
*/

#define Ydeck_top( deck_xyz )    (deck_xyz->current = deck_xyz->top)
/* 
Arguments:
    YDECK *deck_xyz ;
Function:
    Returns the top of the deck and resets position to head.
*/

#define Ydeck_bottom( deck_xyz )    ( deck_xyz->current = deck_xyz->bottom )
/* 
Arguments:
    YDECK *deck_xyz ;
Function:
    Returns the bottom of the deck and resets position to bottom.
*/

#define Ydeck_size( deck_xyz )    (deck_xyz->size)
/* 
Arguments:
    YDECK *deck_xyz ;
Function:
    Returns the number of elements in the deck
*/

#define Ydeck_getData( deck_xyz )    (deck_xyz->current->data)
/* 
Arguments:
    YDECK *deck_xyz ;
Function:
    Returns a pointer to data stored on current card 
*/

#define Ydeck_notEnd( deck_xyz )    (deck_xyz->current != Ydeck_sentinel())
/*
Arguments:
    YDECK *deck_xyz ;
Function:
    returns true if current card is the end of the deck.  This is
    useful if the user wishes to make his own loop.
    eg)  for(Ydeck_top(); Ydeck_notEnd() ; Ydeck_down() ) {
             operations on Ydeck_getData();
	     }
*/

#define Ydeck_getTop( deck_xyz )    (deck_xyz->top->data)
/* 
Arguments:
    YDECK *deck_xyz ;
Function:
    Returns a pointer to data stored on the top card 
*/

#define Ydeck_getBottom( deck_xyz )    (deck_xyz->bottom->data)
/* 
Arguments:
    YDECK *deck_xyz ;
Function:
    Returns a pointer to data stored on the bottom card 
*/


/* *********** deck routines *************** */
extern YDECK *Ydeck_init( /* */ ) ;
/* 
Arguments:
    none.
Function:
    Initialize deck structure and return pointer to deck memory.
*/

extern VOID Ydeck_push( P2( YDECK *deck, VOIDPTR data ) ) ;
/* 
Arguments:
    YDECK *deck ;
    VOIDPTR data ;
Function:
    Add a new element to top of the deck.  User is required to maintain
    memory of the data element.
*/

extern VOID Ydeck_enqueue( P2( YDECK *deck, VOIDPTR data ) ) ;
/* 
Arguments:
    YDECK *deck ;
    VOIDPTR data ;
Function:
    Add a new element to bottom of the deck.  User is required to maintain
    memory of the data element.
*/

extern VOIDPTR Ydeck_pop( P1(YDECK *deck) ) ;
/* 
Arguments:
    YDECK *deck ;
Function:
    Delete top item from deck.  If item does not exist does nothing unless
    debug switch has been turned on.  User is required to maintain
    memory of the data element.
    Returns data (interger or pointer) stored on card.  
*/

extern VOIDPTR Ydeck_dequeue( P1(YDECK *deck) ) ;
/* 
Arguments:
    YDECK *deck ;
Function:
    Delete bottom item from deck.  If item does not exist does nothing unless
    debug switch has been turned on.  User is required to maintain
    memory of the data element.
    Returns data (interger or pointer) stored on card.  
*/

extern VOID Ydeck_dump ( P2( YDECK *deckPtr, VOID (*userPrint)() ) );
/*
Arguments:
    YDECK *deckPt ;
    INT (*userPrint)();
Function:
    For each data item in the deck, call the user print function.
*/

extern VOID Ydeck_empty( P2( YDECK *deck, VOID (*userDelete)() ) ) ;
/* 
Arguments:
    YDECK *deck ;
    (*userDelete)();
Function:
    Frees deck memory. User userdelete function is used to free data
    if desired.  Deck structure remains but links are removed.
    Should be used if deck is to be used again.
*/

extern VOID Ydeck_free( P2( YDECK *deck, VOID (*userDelete)() ) ) ;
/* 
Arguments:
    YDECK *deck ;
    (*userdelete)();
Function:
    Frees deck memory. User userdelete function is used to free data
*/

extern YCARD *Ydeck_sentinel();
/* Arguments:
    none
Function:
   returns address of deck sentinel.  normally this is not used directly
   by the user.  Use macro YNOTDECKEND
*/

extern  int Ydeck_verify(P1(YDECK *deck));

#endif /* DECK_H */





