#ifndef H_ASSIGN_YALECAD
#define H_ASSIGN_YALECAD

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
int **Yassign_init(int m, int n) ;
/* 
 * Arguments:
 *   int m, n ; 
 * Function:
 *   Initializes the linear assignment solver.
 * Bug:  Internally alllcate MAX(m,n) by MAX(m,n) square matrix 
 */

int *Yassign(int **cost_matrix, int m, int n) ;
/* 
 * Arguments:
 *   int  **cost_matrix ;
 *   int m, n;
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

void Yassign_reset(int **cost_matrix, int m, int n);
/* 
 * Arguments:
 *   int m, n ;
 *   int **cost_matrix ;
 * Function:
 *   Do nothing, backward compatiblity.
 */

void Yassign_print(int **cost_matrix, int m, int n);
/* 
 * Arguments:
 *   int m, n ;
 *   int **cost_matrix ;
 * Function:
 *   Print the state of the linear assignment solver.
 */

void Yassign_free(int **cost_matrix, int m, int n) ;
/* 
 * Arguments:
 *  int **cost_matrix ;
 *  int m, n ;
 * Function:
 *   Free the memory associated with the solver.
 */

#endif
