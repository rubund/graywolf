
/****************************************************************************
 ****************************************************************************

	File   : heap.h
	Author : Ted Stanion
	Date   : Mon Apr 30 22:36:17 1990

	Abstract : Include file for heap.c

	Revisions :

	Futures : 

 ****************************************************************************
 ****************************************************************************/

#ifndef HEAP_H
#define HEAP_H

#ifndef lint
static char YHeap_SccsId[] = "@(#) heap.h version 1.3 7/11/91";
#endif

#include <yalecad/base.h>

/****************************************************************************

	Structure : heap
	Author    : Ted Stanion
	Date      : Mon Apr 30 22:57:04 1990

	Abstract : Top level data structure for heaps.

*****************************************************************************/

typedef struct heap {
  INT (*heap_cmp)();
  struct heap_el *top;
} YHEAP, *YHEAPPTR;


/****************************************************************************

	Macro  : heap_empty
	Author : Ted Stanion
	Date   : Tue May  1 16:40:02 1990

	Abstract : Returns TRUE if the heap is empty.

*****************************************************************************/

#define heap_empty(h) (((h)->top) ? FALSE : TRUE)


/************************************************************************
 *  									*
 *  Global Functions							*
 *  									*
 ************************************************************************/

extern YHEAPPTR Yheap_init();
extern YHEAPPTR Yheap_init_with_parms(P1(INT (*fn)()));
extern VOID Yheap_empty(P1(YHEAPPTR));
extern VOID Yheap_free(P1(YHEAPPTR));
extern VOID Yheap_insert(P2(YHEAPPTR, VOIDPTR));
extern VOIDPTR Yheap_delete_min(P1(YHEAPPTR));
extern VOIDPTR Yheap_top(P1(YHEAPPTR));
extern YHEAPPTR Yheap_meld(P2(YHEAPPTR, YHEAPPTR));
extern INT Yheap_cmp_num(P2(INT, INT));
extern INT Yheap_cmp_ptr(P2(VOIDPTR, VOIDPTR));
extern VOID Yheap_check_mem();
extern INT Yheap_verify(P1(YHEAPPTR));

#endif
