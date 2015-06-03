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

/****************************************************************************
 ****************************************************************************
 
 File   : heap.c
 Author : Ted Stanion
 Date   : Tue May  1 00:06:55 1990
 
 Abstract : Implements a heap structure or priority queue.  Each
 heap may have its own compare function.  It is recommended that
 you use the predefined functions heap_cmp_ptr, heap_cmp_num
 and strcmp, if your keys are pointers, numbers or strings.
 If you create a heap with no function specified, heap_cmp_num
 is assumed.
 
 The following operations are defined:
 * create : creates an empty heap.
 * insert : insert an arbitrary key and data into the heap.
 * delete_min : delete and return the minimum heap element.
 * meld : returns the heap formed by combining two disjoint heaps.
 
 This implementation is based on Chapter 3.3 of Tarjan's SIAM book,
 Data_Structures_and_Network_Algorithms.  We use a leftist heap
 in order to guarantee O(log n) time complexity.
 
 Revisions :
 2 April 1991 -- Ted Stanion -- Hid HEAP_EL from user.
 June 1991    - Port to TimberWolf library.  R.A.Weier
 October 1991 - Make changes for Gnu gcc, a strict compiler.  R.A.Weier 
 Futures : ;
 
 ****************************************************************************
 ****************************************************************************/

#include <yalecad/heap.h>

#ifndef lint
static char Yheap_Id[] = "@(#) heap.c version 1.8 12/15/91";
#endif

/****************************************************************************
  
  Structure : heap_el
  Author    : Ted Stanion
  Date      : Mon Apr 30 22:59:47 1990
  
  Abstract : Structure for holding a heap element.  Points to the
  left and rite children of this element, and this elements key
  and data items.
  
  *****************************************************************************/

typedef struct heap_el {
  VOIDPTR data;
  INT rank;
  struct heap_el *left, *rite;
} HEAP_EL_STRUCT, *HEAP_EL;

/************************************************************************
 *  									*
 *  Access Macros							*
 *  									*
 ************************************************************************/
#define get_data(e) ((e)->data)
#define set_data(e,d) (e)->data = (d)
#define get_rank(e) ((e) ? (e)->rank : 0)
#define set_rank(e,r) (e)->rank = (r)
#define get_left(e) ((e)->left)
#define set_left(e,l) (e)->left = (l)
#define get_rite(e) ((e)->rite)
#define set_rite(e,r) (e)->rite = (r)


/************************************************************************
 *  									*
 *  Local Functions							*
 *  									*
 ************************************************************************/
static VOID free_all_els(P1(HEAP_EL));
static HEAP_EL meld(P3(HEAP_EL, HEAP_EL, INT (*cmp)()));
static HEAP_EL mesh(P3(HEAP_EL, HEAP_EL, INT (*cmp)()));
static YHEAPPTR allocate_heap();
static HEAP_EL allocate_heap_el();
static VOID free_heap(P1(YHEAPPTR));
static VOID free_heap_el(P1(HEAP_EL));
     
     
/************************************************************************
*  									*
*  Local Variables							*
*  									*
 ************************************************************************/
static long heaps_allocated = 0L;
static long heap_els_allocated = 0L;
     
     
/************************************************************************
*  									*
*  Local Defines							*
*  									*
************************************************************************/
#define GTR(fn,e1,e2)					\
((fn) == Yheap_cmp_num ? (get_data(e1) > get_data(e2)) : \
 (fn) == Yheap_cmp_ptr ? (get_data(e1) > get_data(e2)) : \
 (fn)(get_data(e1), get_data(e2)) > 0L)
     
     
/***************************************************************************

 Function : heap_create
 Author   : Ted Stanion
 Date     : Tue May  1 00:19:11 1990

 Abstract : Returns a new heap structure.

*****************************************************************************/
     
     extern YHEAPPTR Yheap_init()
{
  YHEAPPTR tmp;
  
  tmp = allocate_heap();
  tmp->heap_cmp = Yheap_cmp_num;
  return tmp;
}   /*  heap_create  */


/****************************************************************************
  
  Function : heap_create_with_parms
  Author   : Ted Stanion
  Date     : Fri Sep 21 11:27:59 1990
  
  Abstract : Creates a new heap with a user specified compare
  function.
  
  *****************************************************************************/

extern YHEAPPTR Yheap_init_with_parms(fn)
     INT (*fn)();
{
  YHEAPPTR tmp;
  
  tmp = allocate_heap();
  tmp->heap_cmp = fn;
  return tmp;
}   /*  heap_create_with_parms  */


/****************************************************************************
  
  Function : heap_empty
  Author   : Ted Stanion
  Date     : Tue May  1 00:25:16 1990
  
  Abstract : Removes all elements from a heap.  Note: this
  function does not free the data or key items. 
  
  *****************************************************************************/

extern VOID Yheap_empty(heap)
     YHEAPPTR heap;
{
  free_all_els(heap->top);
  heap->top = NIL(HEAP_EL);
}   /*  heap_clear  */


/****************************************************************************
  
  Function : heap_free
  Author   : Ted Stanion
  Date     : Tue May  1 16:56:02 1990
  
  Abstract : Frees up a heap and all of its elements.
  
  *****************************************************************************/

extern VOID Yheap_free(heap)
     YHEAPPTR heap;
{
  Yheap_empty(heap);
  free_heap(heap);
}   /*  heap_free  */


/****************************************************************************
  
  Function : heap_insert
  Author   : Ted Stanion
  Date     : Tue May  1 00:29:08 1990
  
  Abstract : Inserts a data item associated with a key into the
  heap.
  
 *****************************************************************************/

extern VOID Yheap_insert(heap, data)
     YHEAPPTR heap;
     VOIDPTR data;
{
  HEAP_EL el;
  
  /**********************************************************************
   *  									*
   *  Create a new heap element.					*
   *  									*
   **********************************************************************/
  el = allocate_heap_el();
  el->data = data;
  el->rank = 1;
  
  /**********************************************************************
   *  									*
   *  This new element is technically a heap by itself.  This allows	*
   *  us to call MELD to get the new heap.				*
   *  									*
   **********************************************************************/
  heap->top = meld(el, heap->top, heap->heap_cmp);
}   /*  heap_insert  */


/****************************************************************************
  
  Function : heap_delete_min
  Author   : Ted Stanion
  Date     : Tue May  1 17:06:59 1990
  
  Abstract : Returns data associated with the heap element with
  the lowest valued key and deletes it from the heap.
  
 *****************************************************************************/

extern VOIDPTR Yheap_delete_min(heap)
     YHEAPPTR heap;
{
  HEAP_EL el;
  VOIDPTR rtn;
  
  if (el = heap->top) {
    heap->top = meld(get_left(el), get_rite(el), heap->heap_cmp);
    rtn = get_data(el);
    free_heap_el(el);
  } else {  /* if (el = ... */
    rtn = (VOIDPTR) NULL;
  }
  return rtn;
}   /*  heap_delete_min  */


/****************************************************************************
  
  Function : heap_meld
  Author   : Ted Stanion
  Date     : Tue May  1 17:38:14 1990
  
  Abstract : Returns the heap formed by joining two disjoint heaps.
  This operation destroys h1 and h2 in the process.  This procedure
  ASSUMES that h1 and h2 have the same compare function.  The new
  heap is pointed to by h1.
  
 *****************************************************************************/

extern YHEAPPTR Yheap_meld(h1,h2)
     YHEAPPTR h1;
     YHEAPPTR h2;
{
  if ((h1->heap_cmp) != (h2->heap_cmp)) {
    fprintf(stderr,
	    "HEAP: melding heaps with different compare functions.\n");
    abort();
  } else {
    h1->top = meld(h1->top, h2->top, h1->heap_cmp);
  }  /* if (h1->heap_cmp ... */
  
  return h1;
}   /*  heap_meld  */


/****************************************************************************
  
  Function : free_all_els
  Author   : Ted Stanion
  Date     : Wed May  2 09:42:11 1990
  
  Abstract : Recursively frees all elements rooted at el.
  
 *****************************************************************************/

static VOID free_all_els(el)
     HEAP_EL el;
{
  if (el != NIL(HEAP_EL)) {
    free_all_els(get_left(el));
    free_all_els(get_rite(el));
    free_heap_el(el);
  }  /* if (el ... */
}   /*  free_all_els  */


/****************************************************************************
  
  Function : meld
  Author   : Ted Stanion
  Date     : Tue May  1 18:27:38 1990
  
  Abstract : Performs meld operation on heaps rooted at E1 and E2
  using compare function FN.
  
 *****************************************************************************/

static HEAP_EL meld(e1,e2,fn)
     HEAP_EL e1;
     HEAP_EL e2;
     INT (*fn)();
{
  if ( ! e1)
    return e2;
  else if ( ! e2)
    return e1;
  else
    return mesh(e1, e2, fn);
}   /*  meld  */


/****************************************************************************
  
  Function : mesh
  Author   : Ted Stanion
  Date     : Tue May  1 18:30:46 1990
  
  Abstract : Performs actual melding process on two non-empty heaps.
  
 *****************************************************************************/

static HEAP_EL mesh(e1,e2,fn)
     HEAP_EL e1;
     HEAP_EL e2;
     INT (*fn)();
{
  HEAP_EL tmp;
  
  if (GTR(fn, e1, e2)) {
    tmp = e1;
    e1 = e2;
    e2 = tmp;
  }
  
  e1->rite = (get_rite(e1) == NIL(HEAP_EL)) ? e2 :
    mesh(get_rite(e1), e2, fn);
  if (get_rank(get_left(e1)) < get_rank(get_rite(e1))) {
    tmp = get_rite(e1);
    e1->rite = get_left(e1);
    e1->left = tmp;
  }  /* if (get_rank ... */
  e1->rank = get_rank(get_rite(e1)) + 1;
  
  return e1;
}   /*  mesh  */


/****************************************************************************
  
  Function : heap_cmp_num
  Author   : Ted Stanion
  Date     : Fri Sep 21 12:56:44 1990
  
  Abstract : Compares two numbers.
  
 *****************************************************************************/

INT extern Yheap_cmp_num(x, y)
     INT x;
     INT y;
{
  return x - y;
}   /*  heap_cmp_num  */


/****************************************************************************
  
  Function : heap_cmp_ptr
  Author   : Ted Stanion
  Date     : Fri Sep 21 12:56:44 1990
  
  Abstract : Compares two numbers.
  
 *****************************************************************************/

INT extern Yheap_cmp_ptr( x, y)
     VOIDPTR x;
     VOIDPTR y;
{
  return (INT) ((long) x - (long) y);
}   /*  heap_cmp_ptr  */


/****************************************************************************
  
  Function : allocate_heap
  Author   : Ted Stanion
  Date     : Fri Apr 20 13:38:51 1990
  
  Abstract : Returns a HEAP structure from the free list.  If the
  free list is empty, then more are allocated from memory.
  
 *****************************************************************************/

static YHEAPPTR allocate_heap()
{
  YHEAPPTR tmp;
  
  tmp = YCALLOC(1,YHEAP);
  heaps_allocated++;
  
  return tmp;
}   /*  allocate_heap  */


/****************************************************************************
  
  Function : allocate_heap_el
  Author   : Ted Stanion
  Date     : Fri Apr 20 13:38:51 1990
  
  Abstract : Returns a HEAP_EL structure from the free list.  If the
  free list is empty, then more are allocated from memory.
  
 *****************************************************************************/

static HEAP_EL allocate_heap_el()
{
  HEAP_EL tmp;
  
  tmp = YCALLOC(1,HEAP_EL_STRUCT);
  tmp->data = (VOIDPTR)0;
  tmp->rank = -1;
  tmp->left = tmp->rite = NIL(HEAP_EL);
  
  heap_els_allocated++;
  
  return tmp;
}   /*  allocate_heap_el  */


/****************************************************************************
  
  Function : free_heap
  Author   : Ted Stanion
  Date     : Tue May  1 19:16:16 1990
  
  Abstract : Puts a HEAP structure back onto the free list.
  
 *****************************************************************************/

static VOID free_heap(heap)
     YHEAPPTR heap;
{
  YFREE(heap);
  heaps_allocated--;
}   /*  free_heap  */


/****************************************************************************
  
  Function : free_heap_el
  Author   : Ted Stanion
  Date     : Tue May  1 19:17:44 1990
  
  Abstract : Puts a HEAP_EL structure back onto the free list.
  
 *****************************************************************************/

static VOID free_heap_el(el)
     HEAP_EL el;
{
  YFREE(el);
  heap_els_allocated--;
}   /*  free_heap_el  */


/****************************************************************************
  
  Function : heap_check_mem
  Author   : Ted Stanion
  Date     : Wed May  2 17:09:22 1990
  
  Abstract : Prints out status of heap memory.
  
 *****************************************************************************/

extern VOID Yheap_check_mem()
{
  fprintf(stderr,"\theaps_allocated = %d\n", heaps_allocated);
  fprintf(stderr,"\theap_els_allocated = %d\n", heap_els_allocated);
}   /*  heap_check_mem  */

/*--------------------------------
  Yheap_verify:
  Verify heap data structures
  --------------------------------*/
extern INT Yheap_verify(heap)
  YHEAPPTR heap;
{
  INT rc = TRUE;

  if ( YcheckDebug(heap) < sizeof(YHEAP) ) {
    fprintf(stderr,"tile memory corrupt\n");
    rc = FALSE;
  }
  
  /* Future enhancements */
  /* we need a heap_suc here to implement a clean search of heap tree */
  /* OR create a recursive heal_el_verify */

  return( rc );
}
