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
FILE:	    assign.c                                       
DESCRIPTION:solve the linear assignment problem.
DATE:	    Jan 26, 1990 - modified mighty code.
REVISIONS:  Sun Nov  3 12:49:49 EST 1991 - made assign more
		memory efficient by using YVECTOR routines.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) assign.c (Yale) version 1.4 4/16/92" ;
#endif

#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/program.h>
#include <yalecad/assign.h>
#include <yalecad/debug.h>

static INT **costS = (INT **) NULL ;
static INT *capS = (INT *) NULL ;
static INT *rowS = (INT *) NULL ;
static INT *colS = (INT *) NULL ;
static INT *nextS = (INT *) NULL ;
static INT *h1S = (INT *) NULL ;
static INT *h2S = (INT *) NULL ;
static INT *h3S = (INT *) NULL ;
static INT *h4S = (INT *) NULL ;
static INT *h5S = (INT *) NULL ;
static INT *h6S = (INT *) NULL ;

static void initassign();
static void shortestpath();
static void augmentation();
static void transformation();
static INT *allocatevector() ;
static INT **allocatematrix() ;

INT *Yassign( cost_matrix, m, n )
INT  **cost_matrix ;
INT m, n;
/*
 *  For bipatite graph with m source nodes ( i : rows )
 *  and n sink nodes ( j : cols ) connected by edges with costS[i][j] >=0
 *  an assignment of rows i to columns j is calculated which mininimizes
 *  the costS
 *
 *        z = sum costS[i][j] * x[i][j]
	      i,j
 *  subject to
 *
 *   sum x[i][j] <= capS[j];  sum x[i][j] = 1;  x[i][j] >= 0
 *    i                       j
 *
 *  INPUT :  The capacity vector capS[1..n]
 *	     costS matrix         costS[1..m][1..n]
 *     	     problem size m, n.  m <= sum capS[j]
 *				       i
 *  OUTPUT :  Two vectors rowS[1..n], colS[1..m] describing rows and
 *	     columns of the selected matrix entries.
 * 	     rowS[j] points to one of the rows assigned to col j.
 *	     Others may be accessed by next[rowS[j]] ...
 *  scratch : h1S[1..m], h2S[1..n], h3S[1..m], h4S[1..n], h5S[1..n], h6S[1..n]
 *
 *  method :  First an optimal assignment of a subset of rows and
 *	     columns is constructed heuristically.  Then shortest
 *	     augmenting paths are generated starting from unassigned
 *	     source nodes to sink nodes with free capacity, to determine
 *	     the solution of the complete problem.
 *	     REF : Lauther's class note; Burkhard & Derigs, "assignment
 *	     and matching problems, springer 1980
 *
 *	     Problems with m < n are handled as an n by n problem with
 *	     c[i][j] = INFINITY for i > m.
 *	     Sink nodes j with capS[j] > 1 are handled like a bunch of
 *	     capS[j] nodes with capacity 1 each.
 */
{
    INT i, j, k;
/*
    static void initassign();
    static void shortestpath();
    static void augmentation();
    static void transformation();
*/
    costS = cost_matrix ;

    /*
     *  Initialize candidate list for shortest path
     */
    initassign(h1S, h2S, m, n);

    for( j = 2; j <= n; j++ ){
	h6S[j] = j - 1;
    }

    h6S[1] = n;

    for( i = 1; i <= m; i++ ){
	if( colS[i] == 0 ) {
	    shortestpath( h1S, h2S, h3S, h4S, h5S, h6S, i, &j, &k, m, n );
	    augmentation( h5S, i, j );
	    transformation( h1S, h2S, h3S, h4S, k, m, n );
	}
    }
    D( "Yassign/print",
	Yassign_print( costS, m, n ) ;
    ) ;
#if 1
    for( i = 1; i <= m; i++ ) {
	colS[0] += cost_matrix[i][colS[i]] ;
    }
#endif
    return( colS );
} /* end assign */


static void initassign(ys, yt, m, n)
INT ys[], yt[];
INT m, n;
/*
 *  Initializes, checks feasibility and constructs an optimal assignment
 *  for a subset of rowSs and colSumns :
 *  First we calculate ys[i] = min costS[i][j];  yt[j] = min (costS[i][j] - ys[i])
 *				j			 i
 *
 *  Then admissible assignments (i,j) with c[i][j] = ys[i] + yt[j]
 *  are made.
 */
{
    INT i,
	j,
	j0,
	ui,
	cij;
    INT neff = 0;  /* total capacity */

    /*  initial assignment */
    for( j = 1; j <= n; j++ ) {
	rowS[j] = 0;
	neff += capS[j];
    }
    if( neff < m ) {
	M( ERRMSG, "initassign", "total capacity too small\n");
	fprintf(stderr, "failed\n");
	YexitPgm( 03 );
    }

    /*
     *  calculate ys and make trvial assignments
     */
    for( i = 1; i <= m; i++ ) {
	colS[i] = 0;
	ui = ASSIGN_INF ;
	for( j = 1; j <= n; j++ ) {
	    if( costS[i][j] < ui ) {
		ui = costS[i][j];
		j0 = j;
	    }
	}
	ys[i] = ui;
	if( capS[j0] > 0 ) {
	    capS[j0]--;
	    nextS[i] = rowS[j0];
	    rowS[j0] = i;
	    colS[i] = j0;
	    yt[j0] = 0;
	}
    }
    /*
     *  calculate yt
     */
    if( m < neff ) {
	for( j = 1; j <= n; j++ ){
	    yt[j] = 0;
	}
    } else {
	for( j = 1; j <= n; j++ ) {
	    if( rowS[j] == 0 ) {
		ui = ASSIGN_INF ;
		for( i = 1; i <= m; i++ ) {
		    if( (cij = costS[i][j] - ys[i]) < ui ){
			ui = cij;
		    }
		}
		yt[j] = ui;
	    }
	}
    }
    /*
     *  make admissible assignments
     */
    for( i = 1; i <=m; i++ ) {
	if( colS[i] == 0 ) {
	    ui = ys[i];
	    for( j = 1; j <=n; j++ ) {
		if( capS[j] > 0 ) {
		    if( costS[i][j] == ui + yt[j] ) {
			capS[j]--;
			nextS[i] = rowS[j];
			rowS[j] = i;
			colS[i] = j;
			goto DONE;
		    }
		}
	    }
	}
DONE :
	continue;
    }
} /* end initassign */

     
static void shortestpath( ys, yt, dplus, dminus, back, next, i, indexp, dp, m, n )
INT ys[],
    yt[],
    dplus[],
    dminus[],
    back[],
    next[];
INT *indexp;
INT *dp;
INT i,
    m,
    n;
/*
 *  Finds a shortest path from starting node i to sink node index
 *  using Dijkstra's algorithm and modify costs.
 *  next is a cyclic list of all sink nodes.
 *  next[headcand..lastcand] are the active candidate nodes.
 *  back points back in the path.
 */
{
INT lastcand,
    headcand,
    v,
    vgl,
    j,
    dalt,
    ysi;

    for( j = 1; j <= n; j++ )
	dminus[j] = ASSIGN_INF ;
    for( j = 1; j <= m; j++ )
	dplus[j] = ASSIGN_INF ;

    lastcand = headcand = n;
    *dp = 0;

    /* till sink node with free capacity is reached */
    for( ;; ) {
	dplus[i] = dalt = *dp;
	*dp = ASSIGN_INF ;
	v = headcand;
	ysi = ys[i];

	/* for all active and feasible sink nodes */
	for( ;; ) {
	    j = next[v];
	    if( capS[j] > 0 || rowS[j] > 0 ) {
		/*  update distance  */
		vgl = dalt + costS[i][j] - ysi - yt[j];
		if( vgl < dminus[j] ) {
		    dminus[j] = vgl;
		    back[j] = i;
		}
		if( dminus[j] < *dp ) {
		    *dp = dminus[j];
		    *indexp = v;
		}
	    }
	    v = j;
	    if( v == lastcand ) break;
	}

	/*
	 *  remove next[v] from candidate set
	 */
	v = *indexp;
	*indexp = next[v];
	if( *indexp == lastcand ){
	    lastcand = v;
	} else {
	    next[v] = next[*indexp];
	    next[*indexp] = next[headcand];
	    next[headcand] = *indexp;
	    headcand = *indexp;
	}
	i = rowS[*indexp];
        if( capS[*indexp] > 0 ){
	    break;
	} 
    }
} /* end shortest path */


static void augmentation( back, u, ind )
INT back[];
INT u, ind;
/*
 *  tracing back the augmenting path from index back to u,
 *  assignments are updated accordingly.
 */
{
    INT oldind;
    INT w = (INT)NULL;

    (capS[ind])--;

    for( ; w != u; ) {
	oldind = ind;

	w = back[ind];
	if( w != u ) {
	    /* remove w, colS[w] from list associated with colS[w]  */
	    ind = colS[w];
	    rowS[ind] = nextS[w];
	}

	/*  add ede w, index to list associated with index  */
	nextS[w] = rowS[oldind];
	rowS[oldind] = w;
	colS[w] = oldind;
    }
} /* end augmentation */


static void transformation( ys, yt, dplus, dminus, d, m, n )
INT ys[], yt[], dplus[], dminus[] ;
INT d ;
/*
 *  update ys and yt
 */
{
    INT i;
    
    for( i = 1; i <= m; i++ ) {
	if( dplus[i] != ASSIGN_INF  ){
	    ys[i] = ys[i] + d - dplus[i];
	}
    }
    for( i = 1; i <= n; i++ ) {
	if( dminus[i] < d ){
	    yt[i] = yt[i] - d + dminus[i];
	}
    }
} /* end transformation */

INT **Yassign_init(m, n)
INT m, n ; 
{
    INT j ;
    INT **cost_matrix ;
/*
    static INT *allocatevector() ;
    static INT **allocatematrix() ;
*/

    capS = allocatevector( n ) ;
    for( j = 1; j <= n; j++ ){
	capS[j] = 1 ;
    }
    cost_matrix = allocatematrix( m, n );
    rowS = allocatevector( n );
#if 1
    colS = YVECTOR_CALLOC( 0, m, INT ) ;
#else 
    colS = allocatevector( m );
#endif
    nextS = allocatevector( m );
    h1S = allocatevector( m );
    h2S = allocatevector( n );
    h3S = allocatevector( m );
    h4S = allocatevector( n );
    h5S = allocatevector( n );
    h6S = allocatevector( n );
    return( cost_matrix );
} /* end structure */

void Yassign_reset(cost_matrix, m, n )
INT m, n ;
INT **cost_matrix ;
/*
 *  reset to 0 all the entries, except capacities which are 1
 */
{
INT i,
    j;
    for( j = 1; j <= n; j++ )
	capS[j] = 1;
    for( i = 1; i <= m; i++ )
        for( j = 1; j <= n; j++ )
	    cost_matrix[i][j] = 0;
    for( j = 1; j <= n; j++ )
	rowS[j] = 0;
    for( i = 1; i <= m; i++ )
	colS[i] = 0;
#if 1
colS[0] = 0;
#endif
    for( i = 1; i <= m; i++ )
	nextS[i] = 0;
    for( i = 1; i <= m; i++ )
	h1S[i] = 0;
    for( j = 1; j <= n; j++ )
	h2S[j] = 0;
    for( i = 1; i <= m; i++ )
	h3S[i] = 0;
    for( j = 1; j <= n; j++ )
	h4S[j] = 0;
    for( j = 1; j <= n; j++ )
	h5S[j] = 0;
    for( j = 1; j <= n; j++ )
	h6S[j] = 0;
} /* end Yassign_reset */

static INT **allocatematrix ( rows, cols )
INT rows,		/* number of rows */
    cols;		/* number of columns */
{
    INT i ;			/* loop counters */
    INT ** matrix;		/* the allocated matrix */

    /*
     *  Allocate space for the matrix
     */
    matrix = YVECTOR_MALLOC( 1, rows, INT * ) ;
    for (i = 1; i <= rows; i++){
	matrix[i] = YVECTOR_CALLOC( 1, cols, INT ) ;
    }
    return( matrix );
} /* end allocatematrix */


static INT *allocatevector( cols )
INT cols;		/* number of columns */
{
    INT * vector;		/* used to allocate the vector */

    /*
     *  Allocate space for the vector
     */
    vector = YVECTOR_CALLOC( 1, cols, INT ) ;
    return( vector ) ;
} /* end allocatevector */

void Yassign_print( cost_matrix, m, n )
INT m, n ;
INT **cost_matrix ;
{
    INT i,
	j,
	sum = 0,
	max = 0,
	cij;

    for( i = 1; i <= m; i++ ) {
	if( (cij = cost_matrix[i][colS[i]]) > max ){
	    max = cij;
	}
	sum += cij;
    }

    /* if too high, exit */
    if( max >= ASSIGN_PREASSIGN ){
        fprintf(stderr,"ERROR in assignment\n");
    }
    fprintf(stderr,"sum = %d, max = %d\n", sum, max );
    for( j = 1; j <= n; j++ ){
	fprintf(stderr,"%d  %d\n", j, capS[j]);
    }
    for( i = 1; i<= m; i++ ) {
	for( j = 1; j <=n; j++ ) {
	    if( colS[i] == j ){
		fprintf( stderr, "[%d]", cost_matrix[i][j]);
	    } else {
		fprintf( stderr, " %d ", cost_matrix[i][j]);
	    }
	}
	fprintf( stderr, "\n");
    }
} /* end Yassign_print */

void Yassign_free( cost_matrix, m, n )
INT **cost_matrix ;
INT m, n ;
{
    INT i ;

    for (i = 1; i <= m; i++){
	YVECTOR_FREE( cost_matrix[i], 1 ) ;
    }
    YVECTOR_FREE( cost_matrix, 1 ) ;
    if( capS ){
	YVECTOR_FREE( capS, 1 ) ;
	capS = (INT *) NULL ; 
    }
    if( rowS ){
	YVECTOR_FREE( rowS, 1 ) ;
	rowS = (INT *) NULL ; 
    }
    if( colS ){
#if 1
	YVECTOR_FREE( colS, 0 ) ;
#else
	YVECTOR_FREE( colS, 1 ) ;
#endif
	colS = (INT *) NULL ; 
    }
    if( nextS ){
	YVECTOR_FREE( nextS, 1 ) ;
	nextS = (INT *) NULL ; 
    }
    if( h1S ){
	YVECTOR_FREE( h1S, 1 ) ;
	h1S = (INT *) NULL ; 
    }
    if( h2S ){
	YVECTOR_FREE( h2S, 1 ) ;
	h2S = (INT *) NULL ; 
    }
    if( h3S ){
	YVECTOR_FREE( h3S, 1 ) ;
	h3S = (INT *) NULL ; 
    }
    if( h4S ){
	YVECTOR_FREE( h4S, 1 ) ;
	h4S = (INT *) NULL ; 
    }
    if( h5S ){
	YVECTOR_FREE( h5S, 1 ) ;
	h5S = (INT *) NULL ; 
    }
    if( h6S ){
	YVECTOR_FREE( h6S, 1 ) ;
	h6S = (INT *) NULL ; 
    }
} /* end Yassign_free */

#ifdef TEST

#include <yalecad/cleanup.h>

main( argc , argv )
INT argc ;
char *argv[] ;
{

    INT i ;
    INT **c ;
    INT *answer ;
    char *intro ;

    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

    Yinit_memsize( 1024 * 1024 ) ;

    YdebugMemory( FALSE ) ;

    intro = YinitProgram( "Test assignment", "v0.0", NULL );
    fprintf( stderr, "\n%s\n\n", intro ) ;

    /* Yassign_init initializes a cost matrix of size x by y */
    c = Yassign_init( 5, 5 ) ;
    c[1][1] = 7; c[1][2] = 2; c[1][3] = 1; c[1][4] = 9; c[1][5] = 4;
    c[2][1] = 9; c[2][2] = 6; c[2][3] = 9; c[2][4] = 5; c[2][5] = 5;
    c[3][1] = 3; c[3][2] = 8; c[3][3] = 3; c[3][4] = 1; c[3][5] = 8;
    c[4][1] = 7; c[4][2] = 9; c[4][3] = 4; c[4][4] = 2; c[4][5] = 2;
    c[5][1] = 8; c[5][2] = 4; c[5][3] = 7; c[5][4] = 4; c[5][5] = 8;

    answer = Yassign( c, 5, 5 ) ;

    /* returns a vector with the answers */
    fprintf( stderr, "The answer is:" ) ;
    for( i = 1; i <= 5; i++ ){
	fprintf( stderr, "%d ", answer[i] ) ;
    }
    fprintf( stderr, "\n" ) ;
	
    Yassign_print( c, 5, 5 ) ;

    /* now if you know that the memory size is ok and just want to */
    /* change the cost matrix use Yassign_reset */
    Yassign_reset(c, 5, 5 ) ;
    c[1][1] = 9; c[1][2] = 4; c[1][3] = 3; c[1][4] =11; c[1][5] = 6;
    c[2][1] =11; c[2][2] = 8; c[2][3] =11; c[2][4] = 7; c[2][5] = 7;
    c[3][1] = 5; c[3][2] =10; c[3][3] = 5; c[3][4] = 3; c[3][5] =10;
    c[4][1] = 9; c[4][2] =11; c[4][3] = 6; c[4][4] = 4; c[4][5] = 4;
    c[5][1] =10; c[5][2] = 6; c[5][3] = 9; c[5][4] = 6; c[5][5] =10;
    answer = Yassign( c, 5, 5 ) ;

    /* returns a vector with the answers */
    fprintf( stderr, "The answer is:" ) ;
    for( i = 1; i <= 5; i++ ){
	fprintf( stderr, "%d ", answer[i] ) ;
    }
    fprintf( stderr, "\n" ) ;
	
    Yassign_print( c, 5, 5 ) ;

    /* when you are done use Yassign_free */
    Yassign_free( c, 5, 5 ) ;

} /* end main */

#endif


