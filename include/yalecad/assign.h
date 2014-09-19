/* ----------------------------------------------------------------- 
FILE:	    assign.h                                       
DESCRIPTION:include file for the linear assignment solver.
DATE:	    May 15, 1990 - modified mighty code.
REVISIONS:  
            Wed Apr 15 16:40:01 EDT 1992
	       - Replaced by new algorithms
----------------------------------------------------------------- */
#ifndef YASSIGN_H
#define YASSIGN_H

#ifndef lint
static char YassignId[] = "@(#) assign.h (Yale) version 1.3 10/9/90" ;
#endif

#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>

/* LOW MED HIGH and INFINITY are used for the cost matrix */
#define ASSIGN_INF         1000000
#define ASSIGN_PREASSIGN    500000

/* a large number, bigger than any cost */
#define		LAP_HUGE			200000000

typedef struct {
  INT	*xmate;    	/* match array for x's	*/
  INT	*ymate;		/* match array for y's	*/
  INT	*udual;   	/* dual array for u's	*/
  INT	*vdual;		/* dual array for v's	*/
  INT	*pred;	        /* predecessor array for shortest path	*/
  INT	*unassigned;	/* unassigned rows			*/
  INT	*shortlen;	/* shortest path lengths	*/
  INT	*column;	/* array of columns		*/
} LAPJV_block;
	

/******** ASSIGNMENT FUNCTIONS *************/
extern INT **Yassign_init( P2( INT m, INT n ) ) ;
/* 
 * Arguments:
 *   INT m, n ; 
 * Function:
 *   Initializes the linear assignment solver.
 * Bug:  Internally alllcate MAX(m,n) by MAX(m,n) square matrix 
 */

extern INT *Yassign( P3( INT **cost_matrix, INT m, INT n ) ) ;
/* 
 * Arguments:
 *   INT  **cost_matrix ;
 *   INT m, n;
 * Function:
 *   Solves the linear assignment problem.
 * Return:
 *   A 0..m ans array,  ans[0] contain total cost,
 *                      ans[i], 1 <= i <= m,
 *                             means row i going to map column ans[i]
 * Bug: 
 *   m must smaller or equal to n.
 *   Actually, when m < n, cost_matrix will be padded to a 
 *   square matrix, the padding value is ASSIGN_INF.
 *   Therefore, it might overflow if cost become too large. 
 */

extern void Yassign_reset( P3(INT **cost_matrix, INT m, INT n ) ) ;
/* 
 * Arguments:
 *   INT m, n ;
 *   INT **cost_matrix ;
 * Function:
 *   Do nothing, backward compatiblity.
 */

extern void Yassign_print( P3(INT **cost_matrix, INT m, INT n ) ) ;
/* 
 * Arguments:
 *   INT m, n ;
 *   INT **cost_matrix ;
 * Function:
 *   Print the state of the linear assignment solver.
 */

extern void Yassign_free( P3(INT **cost_matrix, INT m, INT n ) ) ;
/* 
 * Arguments:
 *  INT **cost_matrix ;
 *  INT m, n ;
 * Function:
 *   Free the memory associated with the solver.
 */

#endif
