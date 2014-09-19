/* ----------------------------------------------------------------
FILE:	     list.h                                       
DESCRIPTION: Declarations for list management package.
DATE:	    Dec  9, 1989 
REVISIONS:  Sun Nov  3 12:56:37 EST 1991 - modified from Ted.
            12/09/91 - cleanup for non-ANSI compilers  -R.A.Weier
----------------------------------------------------------------- */
#ifndef YLIST_H
#define YLIST_H

#ifndef lint
static char Ylist_h_SID[] = "@(#) list.h Yale Version 1.7 12/9/91" ;
#endif

#include <yalecad/base.h>

/****************************************************************************

	Structure : Ylist
	Author    : Ted Stanion
	Date      : Thu Apr 19 22:16:29 1990

	Abstract  : Top level structure for holding lists.

*****************************************************************************/

typedef struct ylist {
  INT (*comp)();
  INT size;
  struct ylist_el *first, *last;
  struct ylist *next;
} YLIST_STRUCT, *YLIST;


/****************************************************************************

	Structure : Ylist_el
	Author    : Ted Stanion
	Date      : Thu Apr 19 22:19:07 1990

	Abstract : Structure for list element.

*****************************************************************************/

typedef struct ylist_el {
  VOIDPTR data;
  struct ylist_el *next, *prev;
} YLIST_EL_STRUCT, *YLIST_EL;


/****************************************************************************

	Macro  : ylist_first
	Author : Ted Stanion
	Date   : Fri Apr 20 13:22:58 1990

	Abstract : Returns the first element in a list.

*****************************************************************************/

#define Ylist_first(l) (l)->first


/****************************************************************************

	Macro  : Ylist_last
	Author : Ted Stanion
	Date   : Fri Apr 20 13:23:38 1990

	Abstract : Returns the last element of a list.

*****************************************************************************/

#define Ylist_last(l) (l)->last


/****************************************************************************

	Macro  : Ylist_next
	Author : Ted Stanion
	Date   : Tue Apr 24 13:52:00 1990

	Abstract : Returns the next element of a list.

*****************************************************************************/

#define Ylist_next(e) (e)->next


/****************************************************************************

	Macro  : Ylist_prev
	Author : Ted Stanion
	Date   : Tue Apr 24 13:52:42 1990

	Abstract : Returns the previous element of a list.

*****************************************************************************/

#define Ylist_prev(e) (e)->prev


/****************************************************************************

	Macro  : Ylist_data
	Author : Ted Stanion
	Date   : Tue Apr 24 13:53:27 1990

	Abstract : Returns the data item contained by a list element.

*****************************************************************************/

#define Ylist_data(e) (e)->data


/*************************************************************************

	Macro  : Ylist_data_replace
	Author : Ted Stanion
	Date   : Wed Jan 16 15:16:39 1991

	Abstract : Replaces data item contained by a list elelement.

**************************************************************************/

#define Ylist_data_replace(e, i) (e)->data = (i)


/****************************************************************************

	Macro  : Ylist_empty
	Author : Ted Stanion
	Date   : Fri Apr 20 13:25:14 1990

	Abstract : Returns TRUE if a list has no elements, FALSE otw.

*****************************************************************************/

#define Ylist_empty(l) ((l)->size ? FALSE : TRUE)


/*************************************************************************

	Macro  : Ylist_size
	Author : Ted Stanion
	Date   : Sat Feb 16 13:48:58 1991

	Abstract : Retruns th number of elements in the list.

**************************************************************************/

#define Ylist_size(l) ((l)->size)


/****************************************************************************

	Macro  : Ylist_for_all
	Author : Ted Stanion
	Date   : Thu Apr 19 22:30:59 1990

	Abstract : Starts the walking of a list.  Elements may NOT
	be deleted while walking the list.  L is the list to be walked.
	E is a pointer to a YLIST_EL.  Use YLIST_DATA to get to the
	actual data item.  If you want to delete a list item while
	walking through the list, use YLIST_FOR_ALL_SAFE.

	Ylist_for_all(my_list, my_data) {
	  do something to my_data ...
	} Ylist_for_all_end;

*****************************************************************************/

#define Ylist_for_all(l, e) {			\
  for (e = (l)->first; e; e = e->next) {	\


/****************************************************************************

	Macro  : Ylist_for_all_end
	Author : Ted Stanion
	Date   : Thu Apr 19 23:17:45 1990

	Abstract : Closes list-walking loop.

*****************************************************************************/

#define Ylist_for_all_end }}


/*************************************************************************

	Macro  : Ylist_for_all_safe
	Author : Ted Stanion
	Date   : Tue Feb  5 10:57:18 1991

	Abstract : Starts the walking of a list.  Elements may 
	be deleted while walking the list.  L is the list to be walked.
	E is a pointer to a YLIST_EL.  Use YLIST_DATA to get to the
	actual data item.  Close the loop with YLIST_FOR_ALL_SAFE_END.

**************************************************************************/

#define Ylist_for_all_safe(l, e) 			\
{							\
  YLIST_EL LIST_xx_tmp;					\
  for (e = (l)->first; e; e = LIST_xx_tmp){		\
    LIST_xx_tmp = (e)->next;				\
  

/*************************************************************************

	Macro  : Ylist_for_all_safe_end
	Author : Ted Stanion
	Date   : Tue Feb  5 11:01:35 1991

	Abstract : Ends safe list walking loop.

**************************************************************************/

#define Ylist_for_all_safe_end }}


/************************************************************************
 *  									*
 *  Global Functions							*
 *  									*
 ************************************************************************/
extern YLIST Ylist_create();
extern YLIST Ylist_create_with_parms(P1(INT (*comp)()));
extern VOID Ylist_enqueue(P2(YLIST list, VOIDPTR data));
extern VOID Ylist_push(P2(YLIST list, VOIDPTR data));
extern VOID Ylist_insert_after(P3(YLIST list, YLIST_EL item, VOIDPTR data));
extern VOID Ylist_insert_before(P3(YLIST list, YLIST_EL item, VOIDPTR data));
extern VOID Ylist_insert_in_order(P2(YLIST list, VOIDPTR data));
extern VOIDPTR  Ylist_dequeue(P1(YLIST));
extern VOIDPTR  Ylist_pop(P1(YLIST));
extern VOID Ylist_delete(P3(YLIST, YLIST_EL, INT (*user_delete)() ));
extern BOOL Ylist_find_and_delete(P3(YLIST list,VOIDPTR data,INT (*user_delete)()));
extern YLIST Ylist_sort(P1(YLIST list));
extern VOID Ylist_append(P2(YLIST l1, YLIST l2));
extern VOID Ylist_clear(P1(YLIST list));
extern VOID Ylist_free(P1(YLIST list));
extern VOID Ylist_check_mem();


#endif /* YLIST_H */
