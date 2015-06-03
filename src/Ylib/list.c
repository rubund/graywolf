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
FILE:	    list.c                                       
DESCRIPTION:Provides a generalized method for handling linked lists.
	    Currently the only access routines make the list act like a
	    generalized queue.  Push and pop functions make it act like a
	    LIFO queue or stack.  Enqueue and pop make it act like a FIFO.
	    A dequeue function is added for completeness.
CONTENTS:   
DATE:	    Dec  9, 1989 
REVISIONS:  Oct  9, 1990 - rename routines for easier use.
	    Thu Apr 19 22:29:18 1990 - revisions from Ted.
	    15 OCT 1990 -- Ted Stanion -- LIST_DELETE() function added.

	    31 JAN 1991 -- Ted Stanion -- Added capability to have sorted
	    lists.  New functions:
		    Ylist_create_with_parms();
		    Ylist_insert_before();
		    Ylist_insert_after();
		    Ylist_insert_in_order();
		    Ylist_sort();
	    New macros;
		    Ylist_for_all_safe() {
		    } Ylist_for_all_end;

	    18 MAR 1991 -- Ted Stanion -- Added function LIST_APPEND().

	    28 OCT 1991 -- Ted Stanion -- Added function LIST_FIND_AND_DELETE().
	    Made LIST_SORT into a quicksort for greater than 20 elements.
	    LIST_SORT returns a new list. (Old undiscovered bug.)
	    Sun Nov  3 12:54:08 EST 1991 - added to library.
            12/09/91 - cleanup for non-ANSI compilers  -R.A.Weier
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) list.c Yale Version 1.9 12/9/91" ;
#endif

#include <yalecad/list.h>

/************************************************************************
 *  									*
 *  Internal Functions							*
 *  									*
 ************************************************************************/
static YLIST insort(P1(YLIST list));
static YLIST quicksort(P1(YLIST list));
static YLIST allocate_list();
static YLIST_EL allocate_list_el();
static VOID free_list(P1(YLIST));
static VOID free_list_el(P1(YLIST_EL));
static INT def_comp(P2(VOIDPTR, VOIDPTR));


/************************************************************************
 *  									*
 *  Internal Variables							*
 *  									*
 ************************************************************************/
static YLIST free_listS = NIL(YLIST);
static YLIST_EL free_list_elS = NIL(YLIST_EL);
static long lists_allocatedS = 0L;
static long list_els_allocatedS = 0L;

/************************************************************************
 *  									*
 *  Internal Macros							*
 *  									*
 ************************************************************************/

#define COMP(l, d, e)				\
  ((l)->comp == def_comp ? (d) < (e) : 		\
   (l)->comp((d), (e)) > 0L)

#define EQ(l, d, e) ((COMP((l),(d),(e)) == 0) ? TRUE : FALSE)
		    

/****************************************************************************

	Function : Ylist_create
	Author   : Ted Stanion
	Date     : Fri Apr 20 10:46:00 1990

	Abstract : Creates a list.  All other routines work on this list.

*****************************************************************************/

YLIST Ylist_create()
{
  YLIST list = allocate_list();

  list->comp = def_comp;
  return list;
}   /*  Ylist_create  */


/*************************************************************************

	Function : Ylist_create_with_parms
	Author   : Ted Stanion
	Date     : Thu Jan 31 11:26:06 1991

	Abstract : Creates a parameterized list.  Currently the only
	parameter is a compare function.  This function returns a positive
	value if the first argument should precede the second in the list,
	a negative value if the second should precede the first and zero
	if their order is irrelevant.

**************************************************************************/

YLIST Ylist_create_with_parms(comp)
INT (*comp)();
{
  YLIST list = allocate_list();

  list->comp = comp;
  return list;
}   /*  Ylist_create_with_parms  */


/****************************************************************************

	Function : Ylist_enqueue
	Author   : Ted Stanion
	Date     : Fri Apr 20 10:49:11 1990

	Abstract : Puts a data item at the end of a list.

*****************************************************************************/

VOID Ylist_enqueue(list, data)
     YLIST list;
     VOIDPTR data;
{
  YLIST_EL el;

  el = allocate_list_el();
  el->data = data;
  if (list->last == NIL(YLIST_EL))
    list->first = list->last = el;
  else {
    list->last->next = el;
    el->prev = list->last;
    list->last = el;
  }  /* if (list ... */
  list->size++;
}   /*  Ylist_enqueue  */


/****************************************************************************

	Function : Ylist_push
	Author   : Ted Stanion
	Date     : Fri Apr 20 10:54:10 1990

	Abstract : Puts a data item at the beginning of a list.

*****************************************************************************/

VOID Ylist_push(list, data)
     YLIST list;
     VOIDPTR data;
{
  YLIST_EL el;

  el = allocate_list_el();
  el->data = data;
  if (list->first == NIL(YLIST_EL))
    list->first = list->last = el;
  else {
    list->first->prev = el;
    el->next = list->first;
    list->first = el;
  }  /* if (list ... */  
  list->size++;
}   /*  Ylist_push  */


/*************************************************************************

	Function : Ylist_insert_after
	Author   : Ted Stanion
	Date     : Thu Jan 31 11:13:35 1991

	Abstract : Insert DATA after ITEM in LIST.  WARNING: No check
	is made to make sure ITEM really is in LIST.

**************************************************************************/

VOID Ylist_insert_after(list, item, data)
     YLIST list;
     YLIST_EL item;
     VOIDPTR data;
{
  YLIST_EL el, tmp;

  el = allocate_list_el();
  el->data = data;

  if (list->last == item) {
    list->last =
      item->next = el;
    el->prev = item;
  } else {
    tmp = Ylist_next(item);
    item->next = el;
    el->prev = item;
    el->next = tmp;
    tmp->prev = el;
  }  /*  if (list ... */
  list->size++;
}   /*  Ylist_insert_after  */


/*************************************************************************

	Function : Ylist_insert_before
	Author   : Ted Stanion
	Date     : Thu Jan 31 11:23:29 1991

	Abstract : Insert DATA before ITEM in LIST.  WARNING: No check
	is made to make sure ITEM really is in LIST.

**************************************************************************/

VOID Ylist_insert_before(list, item, data)
     YLIST list;
     YLIST_EL item;
     VOIDPTR data;
{
  YLIST_EL el, tmp;

  el = allocate_list_el();
  el->data = data;

  if (list->first == item) {
    list->first =
      item->prev = el;
    el->next = item;
  } else {
    tmp = Ylist_prev(item);
    item->prev = el;
    el->next = item;
    el->prev = tmp;
    tmp->next = el;
  }  /*  if (list ... */
  list->size++;
}   /*  Ylist_insert_before  */


/*************************************************************************

	Function : Ylist_insert_in_order
	Author   : Ted Stanion
	Date     : Thu Jan 31 11:42:10 1991

	Abstract : Insert DATA into LIST in order using the compare
	function of LIST.

**************************************************************************/

VOID Ylist_insert_in_order(list, data)
     YLIST list;
     VOIDPTR data;
{
  YLIST_EL el;
  INT tmp;

  for (el = Ylist_first(list);
       el && ((tmp = COMP(list, Ylist_data(el), data)) > 0);
       el = Ylist_next(el));

  if (el)
    Ylist_insert_before(list, el, data);
  else
    Ylist_enqueue(list, data);
}   /*  list_insert_in_order  */


/****************************************************************************

	Function : Ylist_dequeue
	Author   : Ted Stanion
	Date     : Fri Apr 20 11:03:37 1990

	Abstract : Returns the last data item of a list and deletes it
	from the list.

*****************************************************************************/

VOIDPTR Ylist_dequeue(list)
     YLIST list;
{
  YLIST_EL el;
  VOIDPTR data;

  if (Ylist_empty(list)) {
    fprintf(stderr, "LIST: Dequeue of empty list.");
    abort();
  }

  el = list->last;
  if (list->first == list->last)
    list->first = list->last = NIL(YLIST_EL);
  else {
    list->last = list->last->prev;
    list->last->next = NIL(YLIST_EL);
  }  /* if (list ... */
  data = el->data;
  free_list_el(el);
  list->size--;

  return data;
}   /*  list_dequeue  */


/****************************************************************************

	Function : Ylist_pop
	Author   : Ted Stanion
	Date     : Fri Apr 20 12:19:52 1990

	Abstract : Returns the first item in a list and deletes it from
	the list.

*****************************************************************************/

VOIDPTR Ylist_pop(list)
YLIST list;
{
  YLIST_EL el;
  VOIDPTR data;

  if (Ylist_empty(list)) {
    return NIL(VOIDPTR);
  }

  el = list->first;
  if (list->first == list->last)
    list->first = list->last = NIL(YLIST_EL);
  else {
    list->first = list->first->next;
    list->first->prev = NIL(YLIST_EL);
  }  /* if (list ... */
  data = el->data;
  free_list_el(el);
  list->size--;

  return data;
}   /*  list_pop  */


/*************************************************************************

	Function : Ylist_delete
	Author   : Ted Stanion
	Date     : Mon Oct 15 07:56:09 1990

	Abstract : Deletes EL from LIST.  Automatically disposes of
	EL.  Do NOT do this inside of a list_for_all loop!!!

**************************************************************************/

VOID Ylist_delete(list, el, user_delete)
     YLIST list;
     YLIST_EL el;
     INT (*user_delete)();
{
  if (el->prev)
    el->prev->next = el->next;
  else list->first = el->next;
  if (el->next)
    el->next->prev = el->prev;
  else list->last = el->prev;

  if( user_delete ){
    (*user_delete)( Ylist_data(el) ) ;
  }
    
  free_list_el(el);
  list->size--;
}   /* Ylist_delete  */


/*************************************************************************

	Function : Ylist_find_and_delete
	Author   : Ted Stanion
	Date     : Mon Oct 28 15:18:24 1991

	Abstract : Searches LIST for element whit data element equal
	to DATA. If the LIST has a comparison function, it is used for
	equality checking, otherwise, the pointer must match.  If the
	element is found, it is deleted from the list and the function
	returns TRUE. Otherwise, it returns FALSE.

**************************************************************************/

BOOL Ylist_find_and_delete(list, data, user_delete )
     YLIST list;
     VOIDPTR data;
     INT (*user_delete)();
{
  YLIST_EL el;
  BOOL found_it = FALSE;

  Ylist_for_all(list, el) {
    if (EQ(list, Ylist_data(el), data)) {
      found_it = TRUE;
      break;
    }  /* if (EQ( ... */
  } Ylist_for_all_end;

  if (found_it) {
    Ylist_delete(list, el, user_delete ) ;
  }  /* if (found_it) ... */

  return found_it;
}   /*  Ylist_find_and_delete  */


/*************************************************************************

	Function : Ylist_sort
	Author   : Ted Stanion
	Date     : Thu Jan 31 11:55:27 1991

	Abstract : Sorts LIST using its compare function.  Uses simple
	insertion sort. Should be modified to use a quicksort for large
	lists.

**************************************************************************/

YLIST Ylist_sort(list)
     YLIST list;
{
  if (list->size > 20) {
    list = quicksort(list);
  } else {
    list = insort(list);
  }  /* if (list->size ... */

  return list;
}   /*  Ylist_sort  */


/*************************************************************************

	Function : insort
	Author   : Ted Stanion
	Date     : Mon Oct 28 15:39:39 1991

	Abstract : Sorts LIST using insertion sort.

**************************************************************************/

static YLIST insort(list)
     YLIST list;
{
  YLIST nu_list = Ylist_create_with_parms(list->comp);
  VOIDPTR data;

  while (!(Ylist_empty(list))) {
    data = Ylist_pop(list);
    Ylist_insert_in_order(nu_list, data);
  }  /* while (not Ylist_empty( ... */
  Ylist_free(list);

  return nu_list;
}   /*  insort  */


/*************************************************************************

	Function : quicksort
	Author   : Ted Stanion
	Date     : Mon Oct 28 15:42:20 1991

	Abstract : Sorts LIST using QUICKSORT algorithm.

**************************************************************************/

static YLIST quicksort(list)
     YLIST list;
{
  YLIST before, after;
  VOIDPTR pivot, tmp;

  if (Ylist_size(list) > 1) {
    before = Ylist_create_with_parms(list->comp);
    after = Ylist_create_with_parms(list->comp);
    pivot = Ylist_pop(list);

    while (!(Ylist_empty(list))) {
      tmp = Ylist_pop(list);
      if (COMP(list, pivot, tmp) > 0) {
	Ylist_enqueue(after, tmp);
      } else {
	Ylist_enqueue(before, tmp);
      }  /* if (COMP( ... */
    }  /* while (not Ylist_empty( ... */

    before = Ylist_sort(before);
    after = Ylist_sort(after);
    Ylist_enqueue(before, pivot);
    if (Ylist_size(after) > 0) {
      Ylist_append(before, after);
    }  /* if (Ylist_size( ... */

    Ylist_free(after);
    Ylist_free(list);
    return before;
  } else {
    return list;
  }  /* if (Ylist_size( ... */
}   /*  quicksort  */


/*************************************************************************

	Function : Ylist_append
	Author   : Ted Stanion
	Date     : Mon Mar 18 17:44:23 1991

	Abstract : Appends list L2 to the end of list L1.  This is a
	destructive operation which destroys L2.  (L2 is set to be an
	empty list.)  L2 must still be freed by the user.

**************************************************************************/

VOID Ylist_append( l1, l2)
     YLIST l1;
     YLIST l2;
{
  l1->size = l1->size + l2->size;
  l1->last->next = l2->first;
  l2->first->prev = l1->last;
  l1->last = l2->last;

  l2->size = 0;
  l2->first = l2->last = NIL(YLIST_EL);
}   /*  Ylist_append  */


/****************************************************************************

	Function : Ylist_clear
	Author   : Ted Stanion
	Date     : Fri Apr 20 13:35:10 1990

	Abstract : Deletes all elements of a list.

*****************************************************************************/

VOID Ylist_clear(list)
     YLIST list;
{
  YLIST_EL el, el1;

  for (el = list->first; el; el = el1) {
    el1 = el->next;
    free_list_el(el);
  }  /* for (el ... */

  list->first = list->last = NIL(YLIST_EL);
  list->size = 0L;
}   /*  list_clear  */


/****************************************************************************

	Function : Ylist_free
	Author   : Ted Stanion
	Date     : Fri Apr 20 13:37:50 1990

	Abstract : Clears a list then frees it.

*****************************************************************************/

VOID Ylist_free(list)
     YLIST list;
{
  Ylist_clear(list);
  free_list(list);
}   /*  Ylist_free  */


/****************************************************************************

	Function : allocate_list
	Author   : Ted Stanion
	Date     : Fri Apr 20 13:38:51 1990

	Abstract : Returns a LIST structure from the free list.  If the
	free list is empty, then more are allocated from memory.

*****************************************************************************/

#define NUM_LISTS 10L
static YLIST allocate_list()
{
  YLIST tmp;
  long idx;
  static long last_id;

  if (!(free_listS)) {
    tmp = YMALLOC(NUM_LISTS, YLIST_STRUCT) ;
    for (idx = 0; idx < NUM_LISTS; idx++, tmp++) {
      tmp->next = free_listS;
      free_listS = tmp;
    }  /* for (idx = 0; ... */
  }  /* if (not free_lists ... */

  tmp = free_listS;
  free_listS = tmp->next;
  lists_allocatedS++;

  tmp->size = 0L;
  tmp->first = tmp->last = NIL(YLIST_EL);
  tmp->next = NIL(YLIST);

  return tmp;
}   /*  allocate_list  */


/****************************************************************************

	Function : allocate_list_el
	Author   : Ted Stanion
	Date     : Fri Apr 20 13:43:45 1990

	Abstract : Returns a LIST structure from the free list.  If the
	free list is empty, then more are allocated from memory.

*****************************************************************************/

#define NUM_LIST_ELS 50L
static YLIST_EL allocate_list_el()
{
  YLIST_EL tmp;
  long idx;
  static long last_id;

  if (!(free_list_elS)) {
    tmp = YMALLOC(NUM_LIST_ELS, YLIST_EL_STRUCT);
    for (idx = 0; idx < NUM_LIST_ELS; idx++, tmp++) {
      tmp->next = free_list_elS;
      free_list_elS = tmp;
    }  /* for (idx = 0; ... */
  }  /* if (not free_list_els ... */

  tmp = free_list_elS;
  free_list_elS = tmp->next;
  list_els_allocatedS++;

  tmp->data = NIL(VOIDPTR);
  tmp->next = tmp->prev = NIL(YLIST_EL);

  return tmp;
}   /*  allocate_list_el  */


/****************************************************************************

	Function : free_list
	Author   : Ted Stanion
	Date     : Fri Apr 20 13:46:23 1990

	Abstract : Puts a LIST structure back onto the free list.

*****************************************************************************/

static VOID free_list(list)
     YLIST list;
{
  list->next = free_listS;
  free_listS = list;
  lists_allocatedS--;
}   /*  free_list  */


/****************************************************************************

	Function : free_list_el
	Author   : Ted Stanion
	Date     : Fri Apr 20 13:49:05 1990

	Abstract : Puts a LIST_EL structure back onto the free list.

*****************************************************************************/

static VOID free_list_el(el)
     YLIST_EL el;
{
  el->next = free_list_elS;
  free_list_elS = el;
  list_els_allocatedS--;
}   /*  free_list_el  */


/*************************************************************************

	Function : def_comp
	Author   : Ted Stanion
	Date     : Thu Jan 31 11:53:01 1991

	Abstract : Default compare function for lists.

**************************************************************************/

static INT def_comp(d1, d2)
     VOIDPTR d1;
     VOIDPTR d2;
{
  return ( (INT) d1 - (INT) d2 ) ;
}   /*  def_comp  */


/****************************************************************************

	Function : list_check_mem
	Author   : Ted Stanion
	Date     : Wed May  2 17:11:01 1990

	Abstract : Prints out status of list memory usage.

*****************************************************************************/

VOID Ylist_check_mem()
{
  printf("\tlists_allocated = %d\n", lists_allocatedS);
  printf("\tlist_els_allocated = %d\n", list_els_allocatedS);
}   /*  Ylist_check_mem  */




#ifdef TEST
/********************************************************************/
/*  								    */
/*  Local Functions						    */
/*  								    */
/********************************************************************/
static VOID Yprint_list( P1(YLIST list) ) ;
static VOID ph1();


/****************************************************************************

	Function : main
	Author   : Ted Stanion
	Date     : Wed May  2 09:09:16 1990

	Abstract : Main driver.

*****************************************************************************/

INT main(argc, argv)
     INT argc;
     char *argv[];
{
  char c;
  INT data;
  YLIST list;
  
  fprintf( stderr, "List testing program.  Enter ? to start\n" ) ;
  list = Ylist_create();
  while ((c = getchar()) != EOF) {
    switch (c) {
    case 'e' :
      (VOID) scanf("%d", &data);
      Ylist_enqueue(list, (VOIDPTR) data);
      break;
    case 'u' :
      (VOID) scanf("%d", &data);
      Ylist_push(list, (VOIDPTR) data);
      break;
    case 'd' :
      data = (int) Ylist_dequeue(list);
      printf("\t%d\n", data);
      break ;
    case 'o' :
      data = (int) Ylist_pop(list);
      printf("\t%d\n", data);
      break ;
    case 'c' :
      printf("Clearing list ...\n");
      Ylist_clear(list);
      break;
    case 'p':
      Yprint_list(list);
      break;
    case 's':
      list = Ylist_sort(list);
      break;
    case 'q' :
      return;
    case 'h' :
    case '?' :
      printf("e <num> : enqueue <num> onto list.\n");
      printf("u <num> : push <num> onto list.\n");
      printf("d : dequeue element from list.\n");
      printf("o : pop element from list.\n");
      printf("p : print list.\n");
      printf("s : sort list.\n");
      printf("c : clear all elements from list.\n");
      printf("q : quit.\n");
    }  /* switch(c ... */
  }  /* while ((c ... */
}   /*  main  */


/****************************************************************************

	Function : print_list
	Author   : Ted Stanion
	Date     : Wed May  2 09:53:11 1990

	Abstract : Prints out a list.

*****************************************************************************/

static VOID Yprint_list(list)
     YLIST list;
{
  YLIST_EL el;

  printf("\t");
  Ylist_for_all(list, el) {
    printf("%d  ", (INT) Ylist_data(el));
  } Ylist_for_all_end;
  printf("\n");
}   /*  print_list  */

#endif /* TEST */
