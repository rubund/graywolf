/*
 *   Copyright (C) 1991-1992 Yale University
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

/*-------------------------------------------------------------------
  FILE:	    graph.c                                       
  DESCRIPTION:generic graph algorithms.  See Cormen, Leiserson, and Rivest
  Intorduction to Algorithms
  Chapters 23,24
  CONTENTS:   
  DATE:	    Jun 03,1991
  REVISIONS:Sun Dec 15 02:41:01 EST 1991 - rewrote for new dset routines.
  NOTES: In case the need for speed arises:
  1) instead of rebuilding priority heaps all the time,
  keep them around as part of graph structures.
  -----------------------------------------------------------------*/
#ifndef lint
static char SccsId[] = "@(#) graph.c version 1.35 4/2/92" ;
#endif

#include <yalecad/base.h>
#include <yalecad/colors.h>
#include <yalecad/debug.h>
#include <yalecad/deck.h>
#include <yalecad/draw.h>
#include <yalecad/dset.h>
#include <yalecad/heap.h>
#include <yalecad/message.h>
#include <yalecad/rbtree.h>

/* numbers are TWcolors to ease debugging */
#define BLACK        TWBLACK
#define GRAY         TWLIGHTBLUE
#define WHITE        TWLIGHTBROWN

/* defines for VECTOR array */
#define LO        -1
#define MAXSIZE   -1
#define SIZE   0
#define START  1

/* step size for realloc */
#define STEPSIZE 5

typedef struct graph {
  YTREEPTR nodeTree;          /* all nodes in the graph */
  YTREEPTR edgeTree;          /* all nodes in the graph */
  YDECKPTR topSortDeck;       /* results of topological sort */
  YDECKPTR strongConnectDeck; /* strongly connected nodes */
  YDECKPTR mstDeck;           /* minimum spanning tree over enitre graph */
  YDECKPTR cycleDecks;        /* edges which if add to steiner tree=cycle */ 
  YDECKPTR cyclePrimeDecks;   /* edges used to prime multiple steiner trees */
  YTREEPTR steinerTrees;      /* Tree of graphs which spans some nodes */
  YDSETPTR sourceSet;         /* Nodes which must be part of steiner tree */
  YDECKPTR primeDeck;         /* Edges with which to prime steiner tree */
  int flags;
  INT size;                   /* any metric by which the graph is measured */
  INT (*userEdgeWeight)();    /* user routine to get an edge's weight */
  VOID (*userDrawEdge)();     /* user routine to draw edges */
  VOID (*userDrawNode)();     /* user routine to draw edges */
} YGRAPH, *YGRAPHPTR ;

typedef struct graph_node {
  VOIDPTR data ;           /* optional user data associated with node */
  struct graph_edge **adjEdge;        /* adjacent edge array */
  struct graph_edge **backEdge;       /* back edge array */
  struct graph_node *predecessor; /* predecesor */
  int color;
  int start;
  int finish;
  INT distance;
} YNODE, *YNODEPTR;

typedef struct graph_edge {
  YNODEPTR node1;
  YNODEPTR node2;
  VOIDPTR data;          /* user data associated with edge */
  int weight;
  short color;
  short type;
} YEDGE, *YEDGEPTR ;

#define GRAPH_DEFINED
#include <yalecad/graph.h>

/*---------------------------------------------------------
  Macros
  ---------------------------------------------------------*/

/* if a dynamic edge weight function exists, use it */
/* otherwise, use the edge's static weight */

#define EDGEWEIGHT(graph,edge)   ( (graph)->userEdgeWeight ? \
       (edge)->weight = (*(graph)->userEdgeWeight)(edge) : (edge)->weight )


/*---------------------------------------------------------
  static variables 
  ---------------------------------------------------------*/
static INT dfs_timeS;  /* time variable used by depth first search */
static YGRAPHPTR graphS;
static INT bestSpanDistanceS;  /* span of last required path found */
static VOID (*userNodeFreeS)();
static VOID (*userEdgeFreeS)();

/*---------------------------------------------------------
  static prototypes
  ---------------------------------------------------------*/
static VOID graph_adjedge_free(P2(YEDGEPTR *a_p,YEDGEPTR edge));
static YEDGEPTR *graph_adjedge_insert(P2(YEDGEPTR *a_p,YEDGEPTR edge));
static VOID graph_edge_free(P1(YEDGEPTR edge));

/*--------------------------------------------------
  set_compare_node_ptr: compare node pointers
  --------------------------------------------------*/
static INT set_compare_node_ptr(p1,p2)
    YNODEPTR p1;
    YNODEPTR p2;
{
  return ( (INT)(p1) - (INT)(p2) );
}

/*--------------------------------------------------
  compare_node_distance: compare node distance fields
  --------------------------------------------------*/
static INT compare_node_distance(p1,p2)
    YNODEPTR p1;
    YNODEPTR p2;
{
  return ( p1->distance - p2->distance );
}

/*--------------------------------------------------
  compare_edges: compare edges based on their weight
  --------------------------------------------------*/
static INT compare_edge_weight(p1,p2)
     YEDGEPTR p1;
     YEDGEPTR p2;
{
  return ( p1->weight - p2->weight );
}

/*--------------------------------------------------
  compare_edges: compare edges based on their weight
  --------------------------------------------------*/
static INT compare_edge(p1,p2)
     YEDGEPTR p1;
     YEDGEPTR p2;
{
  return ( p1 - p2 );
}

/*-------------------------------------------------------
  compare_graph_size: compare graphs based on size field
  -------------------------------------------------------*/
static INT compare_graph_size(p1,p2)
     YGRAPHPTR p1;
     YGRAPHPTR p2;
{
  return ( p1->size - p2->size );
}

/*----------------------------------------------------------------
  graph_adjedge_insert: insert edges into an array.
  ----------------------------------------------------------------*/
static YEDGEPTR *graph_adjedge_insert(a_p,edge)
     YEDGEPTR *a_p;    /* the array pointer */
     YEDGEPTR edge;
{
  INT newSize;
  INT max;
  
  newSize = (INT) a_p[SIZE] + 1;
  max = (INT) a_p[MAXSIZE];
  
   /* do we need to allocate another chunk of memory */
   if ( newSize > max ) {

      /* rellocate the array to a new size */
      max += STEPSIZE;
      a_p = YVECTOR_REALLOC ( a_p, LO, max, YEDGEPTR );
   
      /* update the maximum size */
      a_p[MAXSIZE] = (YEDGEPTR) max;   /* this is really an INT */
   }

   /* add the new edge to the array */
   a_p[newSize] = edge;
 
   /* update the size of the array */
   a_p[SIZE] = (YEDGEPTR) newSize;  /* This is an INT */

   return(a_p);
}


/*-------------------------------------------------------------
  graph_adjedge_free: free edge from adj edge data structure
  ------------------------------------------------------------*/
static VOID graph_adjedge_free(a_p,edge)
     YEDGEPTR *a_p;
     YEDGEPTR edge;
{
     YEDGEPTR *s_p;   /* size pointer */
     YEDGEPTR *c_p;   /* current edge pointer */
     YEDGEPTR *l_p;   /* last edge pointer */
     INT size;
     INT newSize;

   s_p = a_p + SIZE;
   size = (INT) *s_p;

   c_p = a_p + START;
   l_p = a_p + size;

   /* search list for edge until edge found or end of list */
   for ( ; *c_p != edge && c_p <= l_p; c_p++ );

   if ( *c_p == edge && c_p <= l_p ) {

     /* set new size */
     newSize = size -1;
     *s_p = (YEDGEPTR) newSize;  /* this is really an int */
 
     /* copy last edge in list to current position */
     *c_p = *l_p;     
   }

}

/*--------------------------------------------------
  edge_free: free edges from data structure
  --------------------------------------------------*/
static VOID graph_edge_free(edgePtr)
     YEDGEPTR edgePtr;
{
  /* now delete users edge data */
  if (userEdgeFreeS) {
    if (edgePtr->data) {
      (*userEdgeFreeS)(edgePtr->data);
    } else {
      M( ERRMSG, "edge_free","no user edge data to free\n" ) ;
    }
  }
  
  /* finally free the edge */
  YFREE(edgePtr);
}

/*--------------------------------------------------
  free_node: free node from data structure
  --------------------------------------------------*/
static VOID graph_node_free(nodePtr)
    YNODEPTR nodePtr;
{
  /* free node adjacency edge tree */
  YVECTOR_FREE(nodePtr->adjEdge,LO);
  YVECTOR_FREE(nodePtr->backEdge,LO);
  
  /* now delete users node data */
  if (userNodeFreeS) {
    if (nodePtr->data) {
      (*userNodeFreeS)(nodePtr->data);
    } else {
      M( ERRMSG, "free_node","no user node data to free\n" ) ;
    }
  }
  
  /* and free the node */
  YFREE(nodePtr);
}


/*-------------------------------------------------------------
  Ygraph_init: Initialize a graph.
  User must make either make the compare_edge function unique
  or set compare_edge equal to NULL
  The user can set an edge's weight statically.
  Or, the use can set an edge's weight dynamicallly.
  Thus, the user edge weight function is optional and only
  should be used if the user need dynamic edge weights.
  -------------------------------------------------------------*/
YGRAPHPTR Ygraph_init( user_compare_node, user_compare_edge,
                       user_edge_weight, flags )
     INT (*user_compare_node)();
     INT (*user_compare_edge)();
     INT (*user_edge_weight)();
     INT flags ;             /* Directed edges, redundant edges, etc. */
{
  YGRAPHPTR graph ;      /* the current tree being built */
  
  graph = YMALLOC( 1, YGRAPH ) ;
  graph->nodeTree = Yrbtree_init( user_compare_node );

  if ( user_compare_edge ) {
    graph->edgeTree = Yrbtree_init( user_compare_edge );
  } else {
    graph->edgeTree = Yrbtree_init( compare_edge );
  }

  graph->userEdgeWeight = user_edge_weight;
  graph->flags = flags;
  graph->sourceSet = Ydset_init( set_compare_node_ptr );
  graph->primeDeck = Ydeck_init();
  graph->cyclePrimeDecks = Ydeck_init();
  graph->topSortDeck = NULL;
  graph->strongConnectDeck = NULL;
  graph->mstDeck = NULL;
  graph->cycleDecks = NULL;
  graph->steinerTrees = NULL;
  graph->userDrawEdge = NULL;  
  graph->userDrawNode = NULL;  

  D( "Ygraph_init",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_init","bogus");
   );

  return (graph);
} /* end Ygraph_init */

/*--------------------------------------------------
  empty all nodes and edge from the graph;
  --------------------------------------------------*/
VOID Ygraph_empty(graph,userNodeDelete,userEdgeDelete)
     YGRAPHPTR graph;
     VOID (*userNodeDelete)();  /* user function to free node data */
     VOID (*userEdgeDelete)();  /* user function to free edge data */
{

  userEdgeFreeS = userEdgeDelete;
  userNodeFreeS = userNodeDelete;

  Yrbtree_empty( graph->edgeTree, graph_edge_free );
  Yrbtree_empty( graph->nodeTree, graph_node_free );
}

/*--------------------------------------------------
  free and empty the graph
  --------------------------------------------------*/
VOID Ygraph_free(graph,userNodeDelete,userEdgeDelete)
     YGRAPHPTR graph;
     VOID (*userNodeDelete)();  /* user function to free node data */
     VOID (*userEdgeDelete)();  /* user function to free edge data */
{
  
  Ydset_free(graph->sourceSet,NULL);
  Ydeck_free(graph->primeDeck,NULL);
  Ydeck_free(graph->cyclePrimeDecks,NULL);

  if( graph->topSortDeck ){
    Ydeck_free( graph->topSortDeck, NULL ) ;
  }
  if( graph->strongConnectDeck ){
    Ydeck_free( graph->strongConnectDeck, NULL ) ;
  }
  if( graph->mstDeck ){
    Ydeck_free( graph->mstDeck, NULL ) ;
  }
  if( graph->cycleDecks ){
    Ydeck_free( graph->cycleDecks, NULL ) ;
  }
  
  Ygraph_empty(graph,userNodeDelete,userEdgeDelete);
  Yrbtree_free( graph->nodeTree, NULL ) ;
  Yrbtree_free( graph->edgeTree, NULL ) ;
  YFREE(graph);
}

/*-------------------------------------------------
  Ygraph_nodeDegree
  -------------------------------------------------*/
INT Ygraph_nodeDegree( node )
     YNODEPTR node;
{
  return ( (INT) node->adjEdge[SIZE] + 
           (INT) node->backEdge[SIZE] );
}

/*-------------------------------------------------
  Ygraph_nodeCount
  -------------------------------------------------*/
INT Ygraph_nodeCount( graph )
     YGRAPHPTR graph;
{
  return (Yrbtree_size(graph->nodeTree));
}

/*-------------------------------------------------
  Ygraph_edgeCount
  -------------------------------------------------*/
INT Ygraph_edgeCount( graph )
     YGRAPHPTR graph;
{
  return (Yrbtree_size(graph->edgeTree));
}

/*-------------------------------------------------
  Ygraph_nodeData
  -------------------------------------------------*/
VOIDPTR Ygraph_nodeData( node )
     YNODEPTR node;
{
  return(node->data);
}

/*-------------------------------------------------
  Ygraph_nodeFind
  -------------------------------------------------*/
YNODEPTR Ygraph_nodeFind( graph, nodeData )
     YGRAPHPTR graph;
     VOIDPTR nodeData;
{
  YNODE nodeDummy;
  
  nodeDummy.data = nodeData;
  return ((YNODEPTR) Yrbtree_search(graph->nodeTree,&nodeDummy) );
}

/*-------------------------------------------------
  Ygraph_nodeFindClosest
  -------------------------------------------------*/
YNODEPTR Ygraph_nodeFindClosest( graph, nodeData, func )
YGRAPHPTR graph;
VOIDPTR nodeData;
INT func ;
{
  YNODE nodeDummy;
  
  nodeDummy.data = nodeData;
  return ((YNODEPTR) Yrbtree_search_closest(graph->nodeTree,&nodeDummy,func) );
}

/*-------------------------------------------------
  Ygraph_nodeSuc - find successor to one previously searched.
  -------------------------------------------------*/
YNODEPTR Ygraph_nodeSuc( graph )
     YGRAPHPTR graph;
{
  return ((YNODEPTR) Yrbtree_search_suc(graph->nodeTree));
}

/*-------------------------------------------------
  Ygraph_nodePred - find predecessor to one previously searched.
  -------------------------------------------------*/
YNODEPTR Ygraph_nodePred( graph )
     YGRAPHPTR graph;
{
  return ((YNODEPTR) Yrbtree_search_pred(graph->nodeTree));
}

/*-------------------------------------------------
  Ygraph_enumerateNodes
  -------------------------------------------------*/
YNODEPTR Ygraph_nodeEnumerate( graph, startFlag )
     YGRAPHPTR graph;
     BOOL startFlag;
{
  return ((YNODEPTR) Yrbtree_enumerate(graph->nodeTree, startFlag) );
}

/*-------------------------------------------------
  Ygraph_nodeEnumeratePush
  -------------------------------------------------*/
VOID Ygraph_nodeEnumeratePush(graph)
     YGRAPHPTR graph;
{
  Yrbtree_enumeratePush(graph->nodeTree);
}

/*-------------------------------------------------
  Ygraph_nodeEnumeratePop
  -------------------------------------------------*/
VOID Ygraph_nodeEnumeratePop(graph)
     YGRAPHPTR graph;
{
  Yrbtree_enumeratePop(graph->nodeTree);
}

/*-------------------------------------------------
  Ygraph_nodeMin
  -------------------------------------------------*/
YNODEPTR Ygraph_nodeMin(graph)
     YGRAPHPTR graph;
{
  return ( (YNODEPTR) Yrbtree_min(graph->nodeTree) );
}

/*-------------------------------------------------
  Ygraph_nodeMax
  -------------------------------------------------*/
YNODEPTR Ygraph_nodeMax(graph)
     YGRAPHPTR graph;
{
  return ( (YNODEPTR) Yrbtree_max(graph->nodeTree) );
}

/*-------------------------------------------------
  Ygraph_listAdjNodes
  -------------------------------------------------*/
YNODEPTR Ygraph_listAdjNodes( node, listNum )
     YNODEPTR node;
     INT listNum;
{
  YEDGEPTR edge;
  YNODEPTR adjNode;

  adjNode = NIL(YNODEPTR);

  if ( listNum < 1 ) {
    M( ERRMSG, "Ygraph_listAdjNode","adj list numbered 1-n\n" ) ;
  }
  
  if ( listNum <= (INT) node->adjEdge[SIZE] ) {
    edge = node->adjEdge[listNum];
    if (edge->node1 == node) {
      adjNode = edge->node2;
    } else {
      adjNode = edge->node1;
    }
  }

  return ( adjNode );
}

/*-------------------------------------------------
  Ygraph_listAdjNodes
  -------------------------------------------------*/
YNODEPTR Ygraph_listBackNodes( node, listNum )
     YNODEPTR node;
     INT listNum;
{
  YEDGEPTR edge;
  YNODEPTR backNode;

  backNode = NIL(YNODEPTR);

  if ( listNum < 1 ) {
    M( ERRMSG, "Ygraph_listBackNode","back list numbered 1-n\n" ) ;
  }
  
  if ( listNum <= (INT) node->backEdge[SIZE] ) {
    edge = node->backEdge[listNum];
    if (edge->node1 == node) {
      backNode = edge->node2;
    } else {
      backNode = edge->node1;
    }
  }

  return ( backNode );
}

/*-------------------------------------------------
  Ygraph_nodeInterval
  -------------------------------------------------*/
YNODEPTR Ygraph_nodeInterval( graph, loData, hiData, startFlag )
     YGRAPHPTR graph;
     VOIDPTR loData;
     VOIDPTR hiData;
     BOOL startFlag;
{
  YNODE lo;
  YNODE hi;

  lo.data = loData;
  hi.data = hiData;
  return ((YNODEPTR) Yrbtree_interval(graph->nodeTree, &lo, &hi, startFlag) );
}

/*-------------------------------------------------
  Ygraph_nodeIntervalPush
  -------------------------------------------------*/
VOID Ygraph_nodeIntervalPush(graph)
     YGRAPHPTR graph;
{
  Yrbtree_intervalPush(graph->nodeTree);
}

/*-------------------------------------------------
  Ygraph_nodeIntervalPop
  -------------------------------------------------*/
VOID Ygraph_nodeIntervalPop(graph)
     YGRAPHPTR graph;
{
  Yrbtree_intervalPop(graph->nodeTree);
}

/*-------------------------------------------------
  Ygraph_nodeInsert: insert a node into the graph  
  -------------------------------------------------*/
YNODEPTR  Ygraph_nodeInsert( graph, nodeData)
     YGRAPHPTR graph;
     VOIDPTR nodeData;   /* users optional node Data */
{
  
  YNODE    nodeDummy;
  YNODEPTR nodePtr;
  
  /* does node already exist? */
  nodeDummy.data = nodeData;
  if ( nodePtr = (YNODEPTR)Yrbtree_search (graph->nodeTree,&nodeDummy)  ) {
    return(nodePtr);  /* node already exists */
  }  
  
  /* create the new node */
  nodePtr = YMALLOC( 1, YNODE );
  nodePtr->adjEdge = YVECTOR_MALLOC ( LO, STEPSIZE, YEDGEPTR );
  nodePtr->backEdge = YVECTOR_MALLOC ( LO, STEPSIZE, YEDGEPTR );
  nodePtr->adjEdge[MAXSIZE] = nodePtr->backEdge[MAXSIZE] = (YEDGEPTR) STEPSIZE;
  nodePtr->adjEdge[SIZE] = nodePtr->backEdge[SIZE] = (YEDGEPTR) 0;
  nodePtr->data = nodeData;
  nodePtr->color = BLACK;
  
  /* add node to the graph data structure */
  Yrbtree_insert(graph->nodeTree,nodePtr);
 
  D("Ygraph_nodeInsert",
    ASSERTNFAULT(Ygraph_nodeVerify(nodePtr),"Ygraph_nodeInsert","bogus node");
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_nodeInsert","bogus graph");
    );

  return(nodePtr);
}

/*--------------------------------------------------
  free_node: free node from data structure
  --------------------------------------------------*/
VOID Ygraph_nodeDelete(graph,node,userNodeFree,userEdgeFree)
     YGRAPHPTR graph;
     YNODEPTR node;
     VOID (*userNodeFree)();
     VOID (*userEdgeFree)();
{
  YEDGEPTR edge;
  YNODEPTR node2;
  YEDGEPTR *a_p;   /* array pointer */
  YEDGEPTR *c_p;   /* current edge pointer */
  YEDGEPTR *l_p;   /* last edge pointer */
  
  
  userNodeFreeS = userNodeFree;
  userEdgeFreeS = userEdgeFree;
  
  /* we need to remove all edges which contain this node */
  /* do this by searching the adjEdge and backEdge lists */  

  for ( a_p = node->adjEdge; a_p;
       a_p = ( a_p == node->adjEdge ? node->backEdge:0 ) ) {
    
    c_p = a_p + START;
    l_p =  c_p + (INT) a_p[SIZE];
    
    /* node adj list for edges */
    for ( ; c_p < l_p; c_p++ ) {
      
      edge = *c_p;
      
      /* any edges found must be removed from other nodes adj list */
      if ( a_p == node->adjEdge ) {
        /* for adjEdge tree get other node */      
        if ( node == edge->node1 ) {
	  node2 = edge->node2;
	} else {
	  node2 = edge->node1;
	}
      } else {
        /* for backEdge tree get node1 */
        node2 = edge->node1;
      }
      
      /* delete the edge from node2 adj edge lists */
      graph_adjedge_free(node2->adjEdge,edge);
      graph_adjedge_free(node2->backEdge,edge);
      
      /* delete the edge from the global edge tree */
      Yrbtree_delete(graph->edgeTree,edge,NULL); 
      
      /* free the edge */
      graph_edge_free(edge);
      
    }
  }  

  /* remove node from global graph node tree */
  Yrbtree_delete(graph->nodeTree,node,NULL);
  
  /* free the node and user data */
  graph_node_free(node);
}


/*--------------------------------------------------
  Ygraph_edgeDelete: free edges from data structure
  --------------------------------------------------*/
VOID Ygraph_edgeDelete(graph,edge,userEdgeFree)
     YGRAPHPTR graph;
     YEDGEPTR  edge;
     VOID (*userEdgeFree)();
{
  YEDGEPTR edge2;

  D( "Ygraph_edgeDelete",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_edgeDelete","bogus");
  ) ;

  userEdgeFreeS = userEdgeFree;

  if ( ! edge ){
    M( ERRMSG, "Ygraph_edgeDelete","Null edge\n" ) ;
  }

  /* delete the edge from node adj edge lists */
  graph_adjedge_free(edge->node1->adjEdge,edge);
  graph_adjedge_free(edge->node2->adjEdge,edge);
  graph_adjedge_free(edge->node2->backEdge,edge);

  /* Delete the edge from the global edge tree */
  /* User's comparison function may not be unique, */
  /* so do an interval to get the right edge */
  for ( edge2 = (YEDGEPTR) Yrbtree_interval(graph->edgeTree,edge,edge,TRUE);
        edge2;
        edge2 = (YEDGEPTR) Yrbtree_interval(graph->edgeTree,edge,edge,FALSE) ){
     if ( edge2 == edge ) {
       /* null for user edge free because graph_edge_free() does it below */
       Yrbtree_deleteCurrentInterval(graph->edgeTree,NULL);
       break;
     }
  }

  if ( !edge2 ) {
    M( ERRMSG, "Ygraph_edgeDelete","Edge not found in global edge tree\n" ) ;
  }

  /* free the edge and user data */
  graph_edge_free(edge);

  D( "Ygraph_edgeDelete",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_edgeDelete","bogus");
  ) ;

}

/*-------------------------------------------------
  Ygraph_edgeFindByNodes
  -------------------------------------------------*/
YEDGEPTR Ygraph_edgeFindByNodes( graph, node1, node2 )
     YGRAPHPTR graph;
     YNODEPTR node1;
     YNODEPTR node2;
{
  YEDGEPTR edge;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  YEDGEPTR adjEdge;

  /* check node1 adj edge tree to */
  /* see if the edge already exists  */
  edge = NIL(YEDGEPTR) ;
  c_p = node1->adjEdge + START;
  l_p = c_p + (INT) node1->adjEdge[SIZE];
  for ( ; c_p < l_p; c_p ++ ) {
    adjEdge = *c_p;
    if (adjEdge->node1 == node1 && adjEdge->node2 == node2 ) {
      edge = adjEdge;
      break;
    } else if ( ! (graph->flags & YGRAPH_DIRECTED) ) {
      if (  adjEdge->node2 == node1 && adjEdge->node1 == node2 ) {
	edge = adjEdge;
	break;
      }
    }
  }
  return(edge);
} /* end Ygraph_edgeFindByNodes() */

/*-------------------------------------------------
  Ygraph_edgeFindByNodeData
  -------------------------------------------------*/
YEDGEPTR Ygraph_edgeFindByNodeData( graph, node1Data, node2Data )
     YGRAPHPTR graph;
     VOIDPTR node1Data;
     VOIDPTR node2Data;
{
  YNODEPTR node1;
  YNODEPTR node2;
  YEDGEPTR edge;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  YEDGEPTR adjEdge;

  if(!(node1 = Ygraph_nodeFind( graph, node1Data ))){
    return( NIL(YEDGEPTR) ) ;
  }
  if(!(node2 = Ygraph_nodeFind( graph, node2Data ))){
    return( NIL(YEDGEPTR) ) ;
  }

  /* check node1 adj edge tree to */
  /* see if the edge already exists  */
  edge = NIL(YEDGEPTR) ;
  c_p = node1->adjEdge + START;
  l_p = c_p + (INT) node1->adjEdge[SIZE];
  for ( ; c_p < l_p; c_p ++ ) {
    adjEdge = *c_p;
    if (adjEdge->node1 == node1 && adjEdge->node2 == node2 ) {
      edge = adjEdge;
      break;
    } else if ( ! (graph->flags & YGRAPH_DIRECTED) ) {
      if (  adjEdge->node2 == node1 && adjEdge->node1 == node2 ) {
	edge = adjEdge;
	break;
      }
    }
  }
  return(edge);
} /* end Ygraph_edgeFindByNodes() */

/*-------------------------------------------------
  Ygraph_edgeFind
  -------------------------------------------------*/
YEDGEPTR Ygraph_edgeFind( graph, edgeData, node1Data, node2Data )
     YGRAPHPTR graph;
     VOIDPTR edgeData;
     VOIDPTR node1Data;
     VOIDPTR node2Data;
{
  YEDGEPTR edge;
  YEDGE edgeDummy;
  YNODE node1Dummy;
  YNODE node2Dummy;
  
  edgeDummy.data = edgeData;
  /* user may have edge sorted by node data */
  node1Dummy.data = node1Data;
  node2Dummy.data = node2Data;

  edgeDummy.node1 = &node1Dummy;
  edgeDummy.node2 = &node2Dummy;
  fprintf( stderr, "Obsolete routine. Use edgeFindByNode() or edgeFindByNodeData().\n" ) ;

  edge = (YEDGEPTR) Yrbtree_search(graph->edgeTree,&edgeDummy);

  /* if the edge was not found, reverse the nodes and look again */
  /* if the graph does not have directed edges */
  if (!edge && !(graph->flags & YGRAPH_DIRECTED)) {
    edgeDummy.node1 = &node2Dummy;
    edgeDummy.node2 = &node1Dummy;
    edge = (YEDGEPTR) Yrbtree_search(graph->edgeTree,&edgeDummy);
  }

  return(edge);
}

/*-------------------------------------------------
  Ygraph_edgeEnumeratePush
  -------------------------------------------------*/
VOID Ygraph_edgeEnumeratePush( graph)
     YGRAPHPTR graph;
{
  Yrbtree_enumeratePush( graph->edgeTree);
}

/*-------------------------------------------------
  Ygraph_edgeEnumeratePop
  -------------------------------------------------*/
VOID Ygraph_edgeEnumeratePop(graph)
     YGRAPHPTR graph;
{
  Yrbtree_enumeratePop( graph->edgeTree);
}

/*-------------------------------------------------
  Ygraph_edgeEnumerate
  -------------------------------------------------*/
YEDGEPTR Ygraph_edgeEnumerate( graph, startFlag )
     YGRAPHPTR graph;
     BOOL startFlag;
{
  return ( (YEDGEPTR)Yrbtree_enumerate(graph->edgeTree, startFlag) );
}

/*-------------------------------------------------
  Ygraph_edgeMin
  -------------------------------------------------*/
YEDGEPTR Ygraph_edgeMin(graph)
     YGRAPHPTR graph;
{
  return ( (YEDGEPTR) Yrbtree_min(graph->edgeTree) );
}

/*-------------------------------------------------
  Ygraph_edgeMax
  -------------------------------------------------*/
YEDGEPTR Ygraph_edgeMax(graph)
     YGRAPHPTR graph;
{
  return ( (YEDGEPTR) Yrbtree_max(graph->edgeTree) );
}

/*-------------------------------------------------
  Ygraph_listAdjEdges
  -------------------------------------------------*/
YEDGEPTR Ygraph_listAdjEdges( node, listNum )
     YNODEPTR node;
     INT listNum;
{
  YEDGEPTR edge;

  edge = NIL(YEDGEPTR);

  if ( listNum < 1 ) {
    M( ERRMSG, "Ygraph_listAdjEdge","adj list numbered 1-n\n" ) ;
  }
  
  if ( listNum <= (INT) node->adjEdge[SIZE] ) {
    edge = node->adjEdge[listNum];
  }

  return(edge);
}

/*-------------------------------------------------
  Ygraph_listBackEdges
  -------------------------------------------------*/
YEDGEPTR Ygraph_listBackEdges( node, listNum )
     YNODEPTR node;
     INT listNum;
{
  YEDGEPTR edge;

  edge = NIL(YEDGEPTR);

  if ( listNum < 1 ) {
    M( ERRMSG, "Ygraph_listBackEdges","adj list numbered 1-n\n" ) ;
  }
  
  if ( listNum <= (INT) node->backEdge[SIZE] ) {
    edge = node->backEdge[listNum];
  }

  return(edge);
}

/*-------------------------------------------------
  Ygraph_edgeInterval
  -------------------------------------------------*/
YEDGEPTR Ygraph_edgeInterval( graph, loData, hiData, startFlag )
     YGRAPHPTR graph;
     VOIDPTR loData;
     VOIDPTR hiData;
     BOOL startFlag;
{
  YEDGE lo;
  YEDGE hi;

  lo.data = loData;
  hi.data = hiData;
  return ((YEDGEPTR) Yrbtree_interval(graph->edgeTree, &lo, &hi, startFlag) );
}

/*-------------------------------------------------
  Ygraph_edgeIntervalPush
  -------------------------------------------------*/
VOID Ygraph_edgeIntervalPush(graph)
     YGRAPHPTR graph;
{
  Yrbtree_intervalPush(graph->edgeTree);
}

/*-------------------------------------------------
  Ygraph_edgeIntervalPop
  -------------------------------------------------*/
VOID Ygraph_edgeIntervalPop(graph)
     YGRAPHPTR graph;
{
  Yrbtree_intervalPop(graph->edgeTree);
}

/*-------------------------------------------------
  Ygraph_edgeNodes
  -------------------------------------------------*/
VOID Ygraph_edgeNodes( edge, p1, p2 )
     YEDGEPTR edge;
     YNODEPTR *p1;
     YNODEPTR *p2;
{
  *p1 = edge->node1;
  *p2 = edge->node2;
}

/*-------------------------------------------------
  Ygraph_edgeData
  -------------------------------------------------*/
VOIDPTR Ygraph_edgeData( edge )
     YEDGEPTR edge;
{
  return(edge->data);
}

/*-------------------------------------------------
  Ygraph_edgeNode1
  -------------------------------------------------*/
YNODEPTR Ygraph_edgeNode1( edge )
     YEDGEPTR edge;
{
  return(edge->node1);
}

/*-------------------------------------------------
  Ygraph_edgeNode2
  -------------------------------------------------*/
YNODEPTR Ygraph_edgeNode2( edge )
     YEDGEPTR edge;
{
  return(edge->node2);
}

/*-------------------------------------------------
  Ygraph_edgeNode1Data
  -------------------------------------------------*/
VOIDPTR Ygraph_edgeNode1Data( edge )
     YEDGEPTR edge;
{
  return(edge->node1->data);
}

/*-------------------------------------------------
  Ygraph_edgeNode2Data
  -------------------------------------------------*/
VOIDPTR Ygraph_edgeNode2Data( edge )
     YEDGEPTR edge;
{
  return(edge->node2->data);
}

/*-------------------------------------------------
  Ygraph_edgeWeight
  -------------------------------------------------*/
INT Ygraph_edgeWeight( edge )
     YEDGEPTR edge;
{
  return( edge->weight );
}

/*-------------------------------------------------
  Ygraph_edgeWeightSet
  -------------------------------------------------*/
INT Ygraph_edgeWeightSet( edge, weight )
     YEDGEPTR edge;
     INT weight;
{
  edge->weight = weight;
  return( edge->weight );
}

/*-------------------------------------------------
  Ygraph_edgeWeights2Size
  Sum all of the edge weights, and put into the
  graph size field.
  -------------------------------------------------*/
INT Ygraph_edgeWeights2Size( graph )
     YGRAPHPTR graph;
{
  YEDGEPTR edge; 
  INT size;
  
  size = 0; 
  
  Ygraph_edgeEnumeratePush(graph);
  for ( edge = (YEDGEPTR) Ygraph_edgeEnumerate(graph,TRUE);
       edge;
       edge = (YEDGEPTR) Ygraph_edgeEnumerate(graph,FALSE) ){
    
    size += edge->weight;
  }
  Ygraph_edgeEnumeratePop(graph);
  
  return (graph->size = size);
}

/*-------------------------------------------------
  Ygraph_edgeNode1Data
  -------------------------------------------------*/
int Ygraph_edgeType( edge )
     YEDGEPTR edge;
{
  return(edge->type);
}

/*-------------------------------------------------
  Ygraph_edgeInsert: insert an edge into the graph  
  The new YEDGEPTR is returned.
  If the edge already existed, NULL is returned.
  -------------------------------------------------*/
YEDGEPTR Ygraph_edgeInsert( graph, edgeData, edgeWeight, nodeData, node2Data)
     YGRAPHPTR graph;
     VOIDPTR edgeData;   /* users optional edge Data */
     INT edgeWeight;
     VOIDPTR nodeData;
     VOIDPTR node2Data;
{
  YNODEPTR node1;
  YNODEPTR node2;
  YEDGEPTR adjEdge;
  YEDGEPTR edge;
  YEDGEPTR edge2;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  
  node1 = Ygraph_nodeInsert ( graph, nodeData );
  node2 = Ygraph_nodeInsert ( graph, node2Data );

  edge  = NIL(YEDGEPTR);
  edge2 = NIL(YEDGEPTR);

  /* check node1 adj edge tree to */
  /* see if the edge already exists  */
  c_p = node1->adjEdge + START;
  l_p = c_p + (INT) node1->adjEdge[SIZE];
  for ( ; c_p < l_p; c_p ++ ) {
    adjEdge = *c_p;
    if (adjEdge->node1 == node1 && adjEdge->node2 == node2 ) {
      edge = adjEdge;
      break;
    } else if ( ! (graph->flags & YGRAPH_DIRECTED) ) {
      if (  adjEdge->node2 == node1 && adjEdge->node1 == node2 ) {
	edge = adjEdge;
	break;
      }
    }
  }
    
  /* the edge does not exist, so we need to create it */
  if ( !edge ) {
    edge2 = YMALLOC( 1, YEDGE );
    edge2->node1 = node1;
    edge2->node2 = node2;
    edge2->weight = edgeWeight; 
    edge2->data = edgeData;
    edge2->color = WHITE;
    edge2->type = ( graph->flags & YGRAPH_DIRECTED );
    
    /* edge is always add to node1 adj edge tree */
    node1->adjEdge = graph_adjedge_insert(node1->adjEdge,edge2);
    
    /* edge is added to node2 adj edge tree if graph is not directed */
    if ( graph->flags & YGRAPH_DIRECTED ) {
      node2->backEdge = graph_adjedge_insert(node2->backEdge,edge2);
    } else {
      node2->adjEdge = graph_adjedge_insert(node2->adjEdge,edge2);
    }
    
    /* Add edge to global edge tree */
    Yrbtree_insert(graph->edgeTree,edge2);

  }  /* end create a new edge */

  D("Ygraph_edgeInsert",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_edgeInsert","bogus");
    ASSERTNFAULT(Ygraph_nodeVerify(node1),"Ygraph_edgeInsert","bogus node1");
    ASSERTNFAULT(Ygraph_nodeVerify(node2),"Ygraph_edgeInsert","bogus node2");
    if ( !edge ) {
      ASSERTNFAULT(Ygraph_edgeVerify(edge2),"Ygraph_edgeInsert","bogus edge");
    } else {
      ASSERTNFAULT(Ygraph_edgeVerify(edge),"Ygraph_edgeInsert","bogus edge");
    }
    );

  return(edge2);
}

/*-------------------------------------------------
  Ygraph_flags
  -------------------------------------------------*/
INT Ygraph_flags( graph )
     YGRAPHPTR graph;
{
  return(graph->flags);
}

/*-------------------------------------------------
  Ygraph_flagsSet
  -------------------------------------------------*/
INT Ygraph_flagsSet(graph, flag )
     YGRAPHPTR graph;
     INT flag;
{
  return(graph->flags = flag);
}

/*-------------------------------------------------
  Ygraph_size()
  Returns the graph size field.
  The size of the graph is set if the graph was
  generated and returned by Ygraph_steiner().
  Also, Ygraph_edgeWeights2Size() can be called to
  add up all of the edge weights into a single size.
  -------------------------------------------------*/
INT Ygraph_size( graph )
     YGRAPHPTR graph;
{
  return(graph->size);
}

/*-------------------------------------------------
  Ygraph_copy: returns a copy of a graph  
  -------------------------------------------------*/
YGRAPHPTR Ygraph_copy( graph )
     YGRAPHPTR graph;
{
  YGRAPHPTR    newGraph;
  YEDGEPTR edge;
  YNODEPTR node;
  INT (*nodeComp)();
  INT (*edgeComp)();
  
  edgeComp = Yrbtree_get_compare(graph->edgeTree);
  nodeComp = Yrbtree_get_compare(graph->nodeTree);
  
  /* initialize a new graph */
  newGraph = Ygraph_init( nodeComp, edgeComp,
                          graph->userEdgeWeight, graph->flags );

 
  /* copy the draw functions */
  Ygraph_drawFunctions(newGraph,graph->userDrawNode,graph->userDrawEdge);
  
  D("Ygraph_copy",
    ASSERTNFAULT(Ygraph_verify(graph),
		 "Ygraph_copy","old graph verification fails");
    ASSERTNFAULT(Ygraph_verify(newGraph),
		 "Ygraph_copy","new graph verification fails");
    );

  
  /*  duplicate the nodes */
  for ( node = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,TRUE);
       node;
       node = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,FALSE)){
    Ygraph_nodeInsert ( newGraph, node->data);
    /* DEBUG */
    ASSERTNFAULT(Ygraph_nodeFind(graph,node->data),"Ygraph_copy","bad");
    ASSERTNFAULT(Ygraph_nodeFind(newGraph,node->data),"Ygraph_copy","bad");
  }
  
  /* duplicate the edges */
  for ( edge = (YEDGEPTR) Yrbtree_enumerate(graph->edgeTree,TRUE);
       edge;
       edge = (YEDGEPTR) Yrbtree_enumerate(graph->edgeTree,FALSE)){

    Ygraph_edgeInsert ( newGraph, edge->data, edge->weight,
		       edge->node1->data,edge->node2->data);
    /* DEBUG */
    ASSERTNFAULT( Ygraph_edgeFind(graph,edge->data,edge->node1->data,edge->node2->data),
	"Ygraph_copy","bad");
    ASSERTNFAULT( Ygraph_edgeFind(newGraph,edge->data,edge->node1->data,edge->node2->data), "Ygraph_copy","bad");
  }
  
   ASSERTNFAULT(Ygraph_nodeCount(newGraph)==Ygraph_nodeCount(graph),
                "Ygraph_copy","node count mismatch");

   ASSERTNFAULT(Ygraph_edgeCount(newGraph)==Ygraph_edgeCount(graph),
                "Ygraph_copy","edge count mismatch");

  D("Ygraph_copy",
    ASSERTNFAULT(Ygraph_verify(graph),
		 "Ygraph_copy","old graph verification fails");
    ASSERTNFAULT(Ygraph_verify(newGraph),
		 "Ygraph_copy","new graph verification fails");
    );

  /* return the copy */
  return(newGraph);
}

/*--------------------------------------------------
  breadth first search
  Perform a beadth first seach rooted at sourceNode.
  Optionally, the user can provide a targetNode beyond
  which the search is terminated.
  --------------------------------------------------*/
VOID Ygraph_bfs(graph,sourceNode,targetNode)
     YGRAPHPTR graph;
     YNODEPTR sourceNode;
     YNODEPTR targetNode;
{
  YNODEPTR nextNode;
  YNODEPTR adjNode;
  YEDGEPTR adjEdge;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  YHEAPPTR heap;
  
  if ( !sourceNode )  {
    M( ERRMSG, "Ygraph_bfs","BFS cannot start from a null node\n" ) ;
    return;
  }

  /* create a heap */
  heap = Yheap_init_with_parms(compare_node_distance);
  
  /* initialize elements of priority heap with source adj edge */
  Yheap_insert(heap,sourceNode);
  
  /* initialize all nodes in the graph */  
  for (nextNode = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,TRUE);
       nextNode;
       nextNode = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,FALSE) ) {
    nextNode->color = WHITE;
    nextNode->distance = INT_MAX;
    nextNode->predecessor = nextNode;
  }
  
  /* initialize the source */
  sourceNode->color = GRAY;
  sourceNode->distance = 0;
  
  /* initially on the source is in the fifo */
  Yheap_insert(heap,sourceNode);
  
  while ( nextNode = (YNODEPTR) Yheap_delete_min(heap) ) {
    
    /* enumerate all of the adjacent nodes */
    c_p = nextNode->adjEdge + START;
    l_p = c_p + (INT) nextNode->adjEdge[SIZE];
    
    /* search list for edges */
    for ( ; c_p < l_p; c_p++ ) {
      
      adjEdge = *c_p;
      
      /* get the adjacent node */
      if (adjEdge->node1 == nextNode) {
	adjNode = adjEdge->node2;
      } else {
	ASSERT(adjEdge->node2 == nextNode,"Ygraph_bfs","corrupt graph");
	adjNode = adjEdge->node1;
      }
      
      if ( adjNode->color != BLACK ) {
	if ( adjNode->distance > nextNode->distance + 
             EDGEWEIGHT(graph,adjEdge) ) {
	  adjNode->color = GRAY;
	  adjNode->distance = nextNode->distance + adjEdge->weight;
	  adjNode->predecessor = nextNode;
	  Yheap_insert(heap,adjNode);
	}
      }
    }    /* END enumerate all of the adjacent nodes */
    
    nextNode->color = BLACK;

    /* if we found the target node, we can quit now */
    if ( nextNode == targetNode ) {
      break;
    }

  }  /* end pop items off of the queue */
  
  if ( targetNode && targetNode != nextNode ) {
    M(WARNMSG,"Ygraph_bfs","target node specified, but not found\n");
  }

  /* free up the heap */
  Yheap_free(heap);

}   /* end Ygraph_bfs */

/*--------------------------------------------------------------------
  Ygraph_path: returns a deck containing a path
  between the target and the source of the last
  execute graph search algorithm.
  To use, first run Ygraph_bfs or Ygraph_dijkstra.
  Returns a deck of nodes.  Target is on top.  Source is on bottom.
  User is responsible for freeing the Deck.
  ---------------------------------------------------------------------*/
YDECKPTR Ygraph_path(graph,targetNode)
     YGRAPHPTR graph;
     YNODEPTR targetNode;
{
  YNODEPTR nextNode;
  YDECKPTR pathDeck ;

  pathDeck = Ydeck_init();
  
  for (nextNode = targetNode; nextNode != nextNode->predecessor;
       nextNode = nextNode->predecessor ) {
    Ydeck_push(pathDeck,nextNode);
  }

  Ydeck_push(pathDeck,nextNode);
 
  D("Ygraph_path",
    fprintf(stderr,"path found with %d nodes\n",Ydeck_size(pathDeck));
    ASSERTNFAULT(Ydeck_verify(pathDeck),"Ygraph_path","bad deck");
  );

  return( pathDeck );
}

/*---------------------------------------------------------------------
  graph_dfs_visit: depth first search subroutine
  This is not the quickest dfs visit available,
  but is is quite general.  It it will correctly
  identify edge types for the directed, undirected, and mixed case.
  -----------------------------------------------------------------------*/
static VOID graph_dfs_visit(node)
     YNODEPTR node;
{
  YNODEPTR adjNode;
  YNODEPTR node1;
  YNODEPTR node2;
  YNODEPTR ancestorNode;
  YEDGEPTR adjEdge;
  YEDGEPTR *c_p;   /* current list pointer */
  YEDGEPTR *c2_p;   /* current list pointer */
  YEDGEPTR *l_p;   /* last item in list pointer */
  YEDGEPTR *l2_p;   /* last item in list pointer */
  YDECKPTR swapDeck;
  
  node->color = GRAY;
  dfs_timeS ++;
  node->start = dfs_timeS;
  
 D("Ygraph_dfs",
   fprintf(stderr,"dfs visit start\n");
   Ygraph_draw(graphS);
   );
  
  /* enumerate all of the adjacent nodes */
  c_p = node->adjEdge + START;
  l_p = c_p + (INT) node->adjEdge[SIZE];
  
  /* search list for edges */
  for ( ; c_p < l_p; c_p++ ) {
    
    adjEdge = *c_p;
    
    /* get the adjacent node */
    if (adjEdge->node1 == node) {
      adjNode = adjEdge->node2;
    } else {
      ASSERT(adjEdge->node2 == node,"graph_dfs","corrupt graph");
      adjNode = adjEdge->node1;
    }
 
    /* This code handles mixed, directed, and undirected graphs */
    /* through the following */
    if (adjEdge->type == YGRAPH_NONDIRECTED ){
      if (adjNode == node->predecessor ) {
	/* Do go not go back to parent on non directed edge */
        continue;
      }
    }

    /* classify edge. white=tree edge, gray=back edge, */
    /* black=forward/cross edge.                       */
    adjEdge->color = adjNode->color;  
    
    D("Ygraph_dfs",
      if ( adjEdge->color == GRAY ) {
        fprintf(stderr,"edge classified as back edge (GRAY)\n");
      } else if ( adjEdge->color == WHITE ) {
        fprintf(stderr,"edge classified as tree edge (WHITE)\n");
      } else if ( adjEdge->color == BLACK ) {
        fprintf(stderr,"edge classified as forward/cross edge (BLACK)\n");
      } else {
        fprintf(stderr,"edge classified as unknow edge (UNKNOWN)\n");
      }
      Ygraph_draw(graphS);
      );
    
    /* This code handles mixed, directed, and undirected graphs */
    /* through the following */
    /* this is necessary to correctly find back edges in the mixed case */
    if (adjEdge->color == BLACK) {
      /* A back edge in the mixed case, may actually be a cycle */
      /* This is needed so that an undirected edge can be used in both */
      /* direction in a mixed type graph.  It should be ok because */
      /* forward and cross edges should not occur in a purly undirected */
      /* graph, so this code will not be executed.  In the purely directed  */
      /* case, this code break will not break out until the first */
      /* directed edge is traced back */
      node1 = adjNode;
      node2 = adjNode->predecessor;
      ancestorNode = node;
      
      swapDeck = Ydeck_init();
      
      while ( node2 != node1 && ancestorNode !=node1) {
	
	/* enumerate all of the adjacent nodes */
	c2_p = node2->adjEdge + START;
	l2_p = c2_p + (INT) node2->adjEdge[SIZE];
	
	/* search list for edges */
	for ( ; c2_p < l2_p; c2_p++ ) {
	  
	  adjEdge = *c2_p;
	  
	  /* get the adjacent node */
	  if (adjEdge->node1 == node1 ||
	      adjEdge->node2 == node1 ) {
            break;
	  }   /* end get adj with adj node */
	} /* end for each adjacent edge */

	
	if (adjEdge->type == YGRAPH_NONDIRECTED) {
	  
	  /* need to switch the direction traveled on this undirected edge */
	  Ydeck_push(swapDeck,adjEdge);
	  
          node1 = node2;
          node2 = node2->predecessor;
	  
	  /* trace back forward/cross edge towards a common ancestor */
	  while (ancestorNode->start > node1->start &&
		 ancestorNode != ancestorNode->predecessor ) {
	    ancestorNode = ancestorNode->predecessor;
	  }    /* end trace back to potential ancestor */
	  
	} else {
	  break;   /* an undirected edge was found. */
	}      /* end if type is undirected */
	
      }   /* end for each node1/node2 pair */
      
      if (ancestorNode == node1) {
        adjNode->predecessor = node;
        adjEdge->color = GRAY;
        
	D("Ygraph_dfs",
	  fprintf(stderr,"mixed graph edge re-classified\n");
	  Ygraph_draw(graphS);
	  );
	
        Ydeck_pop(swapDeck);
        while (adjEdge = (YEDGEPTR) Ydeck_pop(swapDeck)) {
          adjEdge->node1->predecessor = adjEdge->node2;
	  D("Ygraph_dfs",
	    fprintf(stderr,"%d predecessor = %d\n",
		    adjEdge->node1->start,adjEdge->node2->start);
	    );
	  
	}

      }
      Ydeck_free(swapDeck,NULL);      
    } /* end if this is a back Edge, handle special mixed case */
    
    if (adjNode->color == WHITE) {
      adjNode->predecessor = node;
      graph_dfs_visit(adjNode);
    }
  }
 
 Ydeck_push(graphS->topSortDeck,node);
 
 node->color = BLACK;
 dfs_timeS ++;
 node->finish = dfs_timeS;
 
 D("Ygraph_dfs",
   fprintf(stderr,"dfs visit complete\n");
   Ygraph_draw(graphS);
   );
}

/*--------------------------------------------------
  Ygraph_dfs:  Perform a depth first search
  Returns a Deck to the nodes topologically sorted.
  User is responsible for freeing deck.
  --------------------------------------------------*/
YDECKPTR Ygraph_dfs(graph)
     YGRAPHPTR graph;
{
 YNODEPTR nextNode;

  graphS = graph;
  graph->topSortDeck = Ydeck_init();
  
  /* initialize all nodes in the graph */  
  for (nextNode = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,TRUE);
       nextNode;
       nextNode = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,FALSE) ) {
    nextNode->color = WHITE;
    nextNode->distance = INT_MAX;
    nextNode->predecessor = nextNode;
  }
  
 D("Ygraph_dfs",
   fprintf(stderr,"Ygraph_dfsstart\n");
   Ygraph_draw(graph);
   );

  dfs_timeS = 0;
  
  /* do the depth first search */  
  for (nextNode = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,TRUE);
       nextNode;
       nextNode = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,FALSE) ) {
    if (nextNode->color == WHITE) {
      graph_dfs_visit(nextNode);
    }
  }
   
  return(graph->topSortDeck);
}


/*----------------------------------------------------------
  Ygraph_mst_Kruskal:  Find a mst using Kruskal's algorithm
  ----------------------------------------------------------*/
YDECKPTR Ygraph_mst_kruskal(graph)
     YGRAPHPTR graph;
{
  YEDGEPTR nextEdge;
  YDSETPTR     dset;
  YHEAPPTR     heap;

  /* initialize mst deck */
  Ydeck_free(graph->mstDeck,NULL);
  graph->mstDeck = Ydeck_init();
  
  /* initialize disjoint set and heap */
  dset = Ydset_init(set_compare_node_ptr);
  heap = Yheap_init_with_parms(compare_edge_weight);
  
  /* initialize elements of priority heap and disjoint sets */  
  for (nextEdge = (YEDGEPTR) Yrbtree_enumerate(graph->edgeTree,TRUE);
       nextEdge;
       nextEdge = (YEDGEPTR) Yrbtree_enumerate(graph->edgeTree,FALSE) ) {
    Ydset_find(dset,nextEdge->node1);
    Ydset_find(dset,nextEdge->node2);
    Yheap_insert(heap,nextEdge);
  }
  
  while ( nextEdge = (YEDGEPTR)Yheap_delete_min(heap)) {
    if ( Ydset_find_set(dset,nextEdge->node1) !=
  	 Ydset_find_set(dset,nextEdge->node2)     ) {
      Ydset_union(dset,nextEdge->node1,nextEdge->node1);
      Ydeck_push(graph->mstDeck,nextEdge->data);
    }
  }
  
  Yheap_free(heap);
  Ydset_free(dset,NULL);
  
  return(graph->mstDeck);
}

/*----------------------------------------------------------
  Ygraph_mst_prim:  Find a mst using prim's algorithm
  ----------------------------------------------------------*/
YDECKPTR Ygraph_mst_prim(graph,source)
     YGRAPHPTR graph;
     YNODEPTR source;
{
  YNODEPTR nextNode;
  YEDGEPTR adjEdge;
  YEDGEPTR nextEdge;
  YHEAPPTR heap;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  
  /* initialize edge colors */  
  for (nextEdge = (YEDGEPTR) Yrbtree_enumerate(graph->edgeTree,TRUE);
       nextEdge;
       nextEdge = (YEDGEPTR) Yrbtree_enumerate(graph->edgeTree,FALSE) ) {
    nextEdge->color = WHITE;         /* mark edge not found */
    nextEdge->node1->color = WHITE;  /* mark node not found */
    nextEdge->node2->color = WHITE;  /* mark node not found */
  }
  
  /* initialize mst deck */
  Ydeck_free(graph->mstDeck,NULL);
  graph->mstDeck = Ydeck_init();
  
  /* initialize disjoint set and heap */
  heap = Yheap_init_with_parms(compare_edge_weight);
  
  /* initialize elements of priority heap with source adj edge */

  /* enumerate all of the adjacent nodes */
  c_p = source->adjEdge + START;
  l_p = c_p + (INT) source->adjEdge[SIZE];
  
  /* search list for edges */
  for ( ; c_p < l_p; c_p++ ) {
    nextEdge = *c_p;
    nextEdge->color=GRAY;        /* mark edge as queued */
    Yheap_insert(heap,nextEdge);
  }
  source->color = BLACK;  /* make node as edges queued */
  
  while ( nextEdge = (YEDGEPTR)Yheap_delete_min(heap)) {
    nextEdge->color = BLACK;  /* make edge as part of mst */
    Ydeck_push(graph->mstDeck,nextEdge->data);
    
    for ( nextNode = nextEdge->node1; nextNode; nextNode = 
	 nextNode == nextEdge->node1 ? nextEdge->node2:NIL(YNODEPTR) ) {
      if ( nextNode->color != BLACK ) {
	
	/* enumerate all of the adjacent nodes */
	c_p = nextNode->adjEdge + START;
	l_p = c_p + (INT) nextNode->adjEdge[SIZE];
	
	/* search list for edges */
	for ( ; c_p < l_p; c_p++ ) {
	  adjEdge = *c_p;
	  if (adjEdge->color == WHITE) {
	    adjEdge->color=GRAY;        /* mark edge as queued */
	    Yheap_insert(heap,nextEdge);
	  }
	}
	nextNode->color = BLACK;  /* make node as edges queued */
      }
    }
  }
  
  Yheap_free(heap);
  
  return(graph->mstDeck);
}

/*----------------------------------------------------------
  Ygraph_dijkstra:  Find a shortest paths from source
  This could be speeded up for some uses by sending a target
    and terminating when that target is found.
  ----------------------------------------------------------*/
VOID Ygraph_dijkstra(graph,sourceNode)
     YGRAPHPTR graph;
     YNODEPTR sourceNode;
{
  YNODEPTR adjNode;
  YEDGEPTR adjEdge;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  YNODEPTR nextNode;
  YHEAPPTR heap;
  
    D("Ygraph_dijkstra",
      fprintf(stderr,"start dijkstra \n");
      ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_dijkstra","bogus graph");
      Ygraph_draw(graph);
      );
    
  ASSERTNFAULT(Ygraph_nodeFind(graph,sourceNode->data),
    "Ygraph_dijkstra","no source");

  /* initialize all nodes in the graph */  
  for (nextNode=(YNODEPTR)Yrbtree_enumerate(graph->nodeTree,TRUE);
       nextNode;
       nextNode=(YNODEPTR)Yrbtree_enumerate(graph->nodeTree,FALSE)){
    nextNode->distance = INT_MAX;
    nextNode->predecessor = NIL(YNODEPTR);
    nextNode->color = WHITE;  /* mark node as not found */
  }

  sourceNode->distance = 0;
  sourceNode->predecessor = sourceNode;
  sourceNode->color = GRAY; /* mark node as on heap */
  
  /* initialize disjoint set and heap */
  heap = Yheap_init_with_parms(compare_node_distance);
  
  /* initialize elements of priority heap with source adj edge */
  Yheap_insert(heap,sourceNode);
  
  D("Ygraph_dijkstra",
    fprintf(stderr,"dijkstra initialized\n");
    Ygraph_draw(graph);
    );
  
  while ( nextNode = (YNODEPTR)Yheap_delete_min(heap)) {
    
    /* the priority queue code does not allow an arbitrary item */
    /* in the queue to change its key.  If a node is relaxed, */
    /* it is added to the queue again.  Thus, a node may be in the queue */
    /* more than once.  The colors are used to detect this situation */
    
    if (nextNode->color == BLACK) {
      continue;  /* node was relaxed and is already done */
    }
    
    /* enumerate all of the adjacent edges */
    c_p = nextNode->adjEdge + START;
    l_p = c_p + (INT) nextNode->adjEdge[SIZE];
    
    /* search list for edges */
    for ( ; c_p < l_p; c_p++ ) {
      adjEdge = *c_p;
      
      /* get the adjacent node */
      if (adjEdge->node1 == nextNode) {
	adjNode = adjEdge->node2;
      } else {
	ASSERT(adjEdge->node2 == nextNode,"graph_dfs","corrupt graph");
	adjNode = adjEdge->node1;
      }
      
      /* relax */
      if ( adjNode->distance > nextNode->distance +
           EDGEWEIGHT(graph,adjEdge) ) {
	adjNode->distance = nextNode->distance + adjEdge->weight;
	adjNode->predecessor = nextNode;
        adjNode->color = GRAY;   /* mark edge as in heap */
        Yheap_insert(heap,adjNode);
	
	D("Ygraph_dijkstra",
	  fprintf(stderr,"dijkstra adj node relaxed\n");
	  ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_dijkstra","bogus graph");
	  /* Ygraph_draw(graph);   /* debug */
	  );
      }
      
    }
    nextNode->color = BLACK;  /* make node as done */
  }
  
  D("Ygraph_dijkstra",
    fprintf(stderr,"dijkstra complete\n");
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_dijkstra","bogus graph");
    Ygraph_draw(graph);
    );
  
  
  Yheap_free(heap);
}


/*----------------------------------------------------------
  Ygraph_bellman_ford: single source shortest path for 
  directed edge graph.
  Returns TRUE if shortest path found.
  Returns FALSE if negative weight cycle exists
  ----------------------------------------------------------*/
BOOL Ygraph_bellman_ford(graph,sourceNode)
     YGRAPHPTR graph;
     YNODEPTR sourceNode;
{
  YNODEPTR nextNode;
  YNODEPTR node1;
  YNODEPTR node2;
  YEDGEPTR nextEdge;
  YTREEPTR tree;
  INT count;
  INT numberOfNodes;
  
  /* make sure graph is directed edge graph */
  if ( ! ( graph->flags & YGRAPH_DIRECTED ) ) {
    M( ERRMSG, "Ygraph_bellman_ford","not a directed graph\n" ) ;
  }
  
  /* initialize all nodes in the graph */  
  for (nextNode=(YNODEPTR)Yrbtree_enumerate(graph->nodeTree,TRUE);
       nextNode;
       nextNode=(YNODEPTR)Yrbtree_enumerate(graph->nodeTree,FALSE)){
    nextNode->distance = INT_MAX;
    nextNode->predecessor = NIL(YNODEPTR);
  }
  sourceNode->distance = 0;
  
  numberOfNodes = Yrbtree_size( graph->nodeTree );
  
  for ( count = 1; count < numberOfNodes; count ++) {
    tree = graph->edgeTree;
    for (nextEdge = (YEDGEPTR) Yrbtree_enumerate(tree,TRUE);
	 nextEdge;
	 nextEdge = (YEDGEPTR) Yrbtree_enumerate(tree,FALSE) ) {
      
      node1 = nextEdge->node1;
      node2 = nextEdge->node2;
      
      /* relax */
      if ( node2->distance > node1->distance + EDGEWEIGHT(graph,nextEdge) ) {
	node2->distance = node1->distance + nextEdge->weight;
	node2->predecessor = node1;
      }
    }
  }
  
  for (nextEdge = (YEDGEPTR) Yrbtree_enumerate(tree,TRUE);
       nextEdge;
       nextEdge = (YEDGEPTR) Yrbtree_enumerate(tree,FALSE) ) {
    node1 = nextEdge->node1;
    node2 = nextEdge->node2;
    
    /* check for negative weight cycle*/
    if ( node2->distance > node1->distance + nextEdge->weight ) {
      return (FALSE);
    }
  }
  
  /* no negative weight cycle, so shortest path is valid */
  return(TRUE);
}

/*----------------------------------------------------------------
  Ygraph_cycles: uses depth first search to find cycles
  in a graph.  The returned deck contains all cycles.
  Each cycle is a deck. 
  User is responsible for freeing decks
  ------------------------------------------------------------------*/
YDECKPTR Ygraph_cycles(graph)
     YGRAPHPTR graph;
     
{
  YNODEPTR node1;
  YNODEPTR node2;
  YNODEPTR finalNode;
  YEDGEPTR edge;
  YDECKPTR cycle;
  YDECKPTR subtractCycle;
  YDECKPTR cycle1;
  YDECKPTR cycle2;
  
  graph->cycleDecks = Ydeck_init();
  
  Ygraph_dfs(graph);
  
  D("Ygraph_cycles",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_cycles","bad graph");
    Ygraph_draw(graph);
    );
  
  /* walk all edges and find back edges (GRAY) */
  for ( edge = Ygraph_edgeEnumerate(graph,TRUE); edge;
       edge = Ygraph_edgeEnumerate(graph,FALSE) ) {
    
    /* For a directed graph, only gray edges are part of cycles */
    /* If a graph contains undirected edges, use black edges also */
    if ( edge->color == GRAY ) {
      
      D("Ygraph_cycles",
	ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_cycles","bad graph");
	fprintf(stderr,"back edge found\n");
	);
      
      node1 = Ygraph_edgeNode1(edge);
      node2 = Ygraph_edgeNode2(edge);
      
      cycle1 = Ygraph_path(graph,node1);
      
      /* give Ygraph_path a fresh deck for next call */
      /* Actuallly, Ygraph_path does this.   */
      
      cycle2 = Ygraph_path(graph,node2);
      
      /* give Ygraph_path a fresh deck for next call */
      /* Actuallly, Ygraph_path does this.   */
      
      /* the cycle is max deck - min deck*/
      if ( Ydeck_size(cycle1) > Ydeck_size(cycle2) ) {
	cycle = cycle1;
	subtractCycle = cycle2;
	finalNode = node2;
      } else {
	cycle = cycle2;
	subtractCycle = cycle1;
	finalNode = node1;
      }
      
      /* subtract out nodes which are not part of the cycle */
      while( Ydeck_pop(subtractCycle) ) {
	Ydeck_pop(cycle);
      }
      
      Ydeck_free(subtractCycle,NULL);
      
      /* complete the loop */
      Ydeck_push(cycle,finalNode);
      Ydeck_enqueue(cycle,finalNode);
      
      Ydeck_push(graph->cycleDecks,cycle);
    }
  }
  
  D("Ygraph_cycles",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_cycles","bad graph");
    fprintf(stderr,"%d cycles found\n",Ydeck_size(graph->cycleDecks));
    );
  
  return(graph->cycleDecks);
}


/*-------------------------------------------------
  Ygraph_nodeRequired: insert a node which must be
  part of any steiner tree  
  -------------------------------------------------*/
YNODEPTR Ygraph_nodeRequired( graph, node, equivNode)
     YGRAPHPTR graph;
     YNODEPTR node;   /* users  node Data */
     YNODEPTR equivNode;   /* equiv node is equivalent to node */
{
  static char routineNameS[32] = "Ygraph_nodeRequired";
  YNODEPTR returnNode;

  D( routineNameS,
    ASSERTNFAULT(Ydset_verify(graph->sourceSet),routineNameS,"bad set");
    /* make sure nodes are valid */
    ASSERTNFAULT(Ygraph_nodeVerify(node),routineNameS,"bogus node");
    );
  
  if ( equivNode ) {
    
    D(routineNameS,    
      ASSERTNFAULT(Ygraph_nodeVerify(equivNode),routineNameS,"bogus node");
      );

    returnNode = (YNODEPTR) Ydset_union(graph->sourceSet,node,equivNode);
  } else {
    returnNode = (YNODEPTR) Ydset_find(graph->sourceSet,node);
  }

  D( routineNameS,
    ASSERTNFAULT(Ygraph_verify(graph),routineNameS,"badGraph");
    ASSERTNFAULT(Ydset_verify(graph->sourceSet),routineNameS,"bad set");
    );

  return( returnNode );
}

/*-------------------------------------------------
  Ygraph_nodeRequiredCount:
  Returns the number of nodes in the required node
  set.
  -------------------------------------------------*/
INT Ygraph_nodeRequiredCount( graph )
     YGRAPHPTR graph;
{
  return( Ydset_superset_size( graph->sourceSet )  );
}

/*-------------------------------------------------
  Ygraph_clearRequired: clear all nodes which must
                       part of any steiner tree  
  -------------------------------------------------*/
VOID Ygraph_clearRequired(graph)
     YGRAPHPTR graph;
{
  Ydset_empty(graph->sourceSet,NULL);
}

/*-------------------------------------------------
  Ygraph_enumerateRequired: enumerates all nodes which
  must be part of any steiner tree or required path.
  -------------------------------------------------*/
YNODEPTR Ygraph_enumerateRequired(graph,startFlag)
     YGRAPHPTR graph;
     BOOL startFlag;
{
  YNODEPTR node;
  
  if ( graph->sourceSet ) {    
    return( (YNODEPTR) Ydset_enumerate(graph->sourceSet,startFlag) );
  } else {
    return( NIL(YNODEPTR) );
  }
}

/*-------------------------------------------------
  Ygraph_edgePrime: Prime a steiner tree search
  with this edge
  -------------------------------------------------*/
VOID Ygraph_edgePrime( graph, edge)
     YGRAPHPTR graph;
     YEDGEPTR edge;   /* users edge */
{
  static char routineNameS[32] = "Ygraph_edgePrime";

  D( routineNameS,
  /* make sure edge is valid */
  ASSERTNFAULT(Ygraph_edgeVerify(edge),routineNameS,"bogus node");
  );
  
  Ydeck_push( graph->primeDeck, edge );
}

/*-------------------------------------------------
  Ygraph_clearPrime: clear all nodes which must
                       part of any steiner tree  
  -------------------------------------------------*/
VOID Ygraph_clearPrime(graph)
     YGRAPHPTR graph;
{
  Ydeck_empty( graph->primeDeck, NULL );
}

/*----------------------------------------------------------
  stiener_trace_back:  trace back and add edges to a deck
  user is responsible for freeing deck.
  ----------------------------------------------------------*/
static YDECKPTR steiner_trace_back(bridgeEdge)
     YEDGEPTR bridgeEdge;
{
  static char routineNameS[32] = "steiner_trace_back";
  YNODEPTR sourceNode;
  YNODEPTR targetNode;
  YNODEPTR traceBackNode;
  YNODEPTR currentNode;
  YEDGEPTR traceBackEdge;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  YDECKPTR traceBackDeck;   
   

  /* cycle has been found.  save the edges to help generate n */
  /* best steiner trees */
  traceBackDeck = Ydeck_init();

  /* the edge between source and target is part of the cycle */
  bridgeEdge->color = GRAY;              /* mark edge as traced */

  Ydeck_push(traceBackDeck,bridgeEdge);

  sourceNode = bridgeEdge->node1;
  targetNode = bridgeEdge->node2;

  /* predecessor edges of source and target are part of the cycle */
  for (traceBackNode = sourceNode; traceBackNode; traceBackNode =
       traceBackNode == sourceNode ? targetNode : NIL(YNODEPTR) ) {

    currentNode = traceBackNode;

    /* trace back predecessors to originator */
    while (currentNode->predecessor != currentNode) {
      /* go back to the predessor in case directed edge */

    /* enumerate all of the adjacent edges */
    c_p = currentNode->predecessor->adjEdge + START;
    l_p = c_p + (INT) currentNode->predecessor->adjEdge[SIZE];
    
    /* search list for edges */
    for ( ; c_p < l_p; c_p++ ) {
      traceBackEdge = *c_p;
      
      if ( traceBackEdge->node1 == currentNode ||
             traceBackEdge->node2 == currentNode ) {
	  currentNode = currentNode->predecessor;
	  break;
	}
      }  /* end look for trace back edge */

      traceBackEdge->color = GRAY;              /* mark edge as traced */
      
      if ( traceBackNode == sourceNode ) {
        Ydeck_push(traceBackDeck,traceBackEdge);
      } else {
        Ydeck_enqueue(traceBackDeck,traceBackEdge);
      }

    }  /* end while traceBackNode->predecessor != traceBackNode) */
  }  /* end for each source and target node trace back */

  D( routineNameS,
  ASSERTNFAULT(Ydeck_verify(traceBackDeck),routineNameS,"bad deck");
  );

  return(traceBackDeck);  

}

/*--------------------------------------------------
  Perform a beadth first seach to 
  find a single path between nodes of different sets.
  Use Ygraph_nodeRequired() to set up initial sets.
  Use Ygraph_clearRequired() to clear initial sets.
  --------------------------------------------------*/
YDECKPTR Ygraph_requiredPath(graph)
     YGRAPHPTR graph;
{
  static char routineNameS[32] = "Ygraph_requiredPath";
  YNODEPTR nextNode;
  YNODEPTR node;
  YNODEPTR node1;
  YNODEPTR node2;
  YNODEPTR adjNode;
  YEDGEPTR bestSpanEdge;
  YEDGEPTR adjEdge;
  YEDGEPTR edge;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  YHEAPPTR heap;
  YDSETPTR dset;
  YDECKPTR spanDeck;
  INT distance;
  int sourceSetCount;

  bestSpanEdge = NIL(YEDGEPTR);
  bestSpanDistanceS = INT_MAX;
  
  /* create a heap */
  heap = Yheap_init_with_parms(compare_node_distance);
  
  /* The BFS starts simultaneously from all pins.  Use disjoint sets */
  /* to handle equivalent pins */
  dset = Ydset_init(set_compare_node_ptr);

  /* return nil if no path can be found */
  spanDeck = NIL(YDECKPTR);

  /* initialize all nodes in the graph */  
  for (node = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,TRUE);
       node;
       node = (YNODEPTR) Yrbtree_enumerate(graph->nodeTree,FALSE) ) {
    node->color = WHITE;
    node->distance = INT_MAX;
    node->predecessor = node;
  }
  
  sourceSetCount = 0;

  /* initialize the priority queue with nodes to be a part of the tree */
  for( node=(YNODEPTR)Ydset_enumerate(graph->sourceSet,TRUE); node;
      node=(YNODEPTR)Ydset_enumerate(graph->sourceSet,FALSE) ) {
    node->distance = 0;
    node->predecessor = node;
    node->color = GRAY;  /* mark node as on heap*/
    Yheap_insert(heap,node);
    /* handle equivalent pins by always union with parent */
    adjNode = (YNODEPTR) Ydset_find_set(graph->sourceSet,node);
    Ydset_union(dset,node,adjNode);
    sourceSetCount++;
  }

  /* initialize with edges required to be a part of the tree */
  for( Ydeck_top(graph->primeDeck); Ydeck_notEnd(graph->primeDeck);
      Ydeck_down(graph->primeDeck) ) {
    edge = (YEDGEPTR) Ydeck_getData(graph->primeDeck);
    node1 = edge->node1;
    node2 = edge->node2;
    node1->distance = 0;
    node1->predecessor = node1;
    node1->color = GRAY;  /* mark node as on heap*/
    Yheap_insert(heap,node1);
    node2->distance = 0;
    node2->predecessor = node2;
    node2->color = GRAY;  /* mark node as on heap*/
    Yheap_insert(heap,node2);
    edge->color = BLACK;  /* make edge as part of tree */
    Ydset_union(dset,node1,node2);
    sourceSetCount++;
   }

  if ( sourceSetCount < 2 ) {
     M( ERRMSG, routineNameS, "not enough required nodes or primed edges\n" );
  }

  node = (YNODEPTR) Ydset_enumerate(graph->sourceSet,TRUE);

 /* make sure there is some work to do */
 if ( Ydset_superset_size(dset) == Ydset_subset_size(dset,node) ) {
   /* force fall through to free heap and free set by depleting the heap */
   Yheap_empty(heap);
 }

  D(routineNameS,
    fprintf(stderr,"%s: initialize\n",routineNameS);
    Ygraph_draw(graph); /* DEBUG */
    );
  
  while ( nextNode = (YNODEPTR) Yheap_delete_min(heap) ) {
    
    if (nextNode->color == BLACK) {
      continue;  /* node was relaxed and is already done */
    }
    
    nextNode->color = BLACK;
    Ydset_union(dset,nextNode,nextNode->predecessor);

    /* enumerate all of the adjacent nodes */
    c_p = nextNode->adjEdge + START;
    l_p = c_p + (INT) nextNode->adjEdge[SIZE];
    
    /* search list for edges */
    for ( ; c_p < l_p; c_p++ ) {
      
      adjEdge = *c_p;
      
      /* get the adjacent node */
      if (adjEdge->node1 == nextNode) {
	adjNode = adjEdge->node2;
      } else {
	ASSERT(adjEdge->node2 == nextNode,"Ygraph_bfs","corrupt graph");
	adjNode = adjEdge->node1;
      }
      
      if ( adjNode->color != BLACK ) {
	if ( adjNode->distance > nextNode->distance + 
	    EDGEWEIGHT(graph,adjEdge) ) {
	  adjNode->color = GRAY;
	  adjNode->distance = nextNode->distance + EDGEWEIGHT(graph,adjEdge);
	  adjNode->predecessor = nextNode;
	  Yheap_insert(heap,adjNode);
	}
      } else if ( nextNode->predecessor != adjNode ) {

        if (Ydset_find_set(dset,adjNode) != Ydset_find_set(dset,nextNode) ) {
	  
          /* we have found a connection, but it is not necessarily the */
          /* best connection.  We cannot be sure until the cost of nodes */
          /* on the heap, exceedS the cost of the current best connection */

	  if ( nextNode->distance > bestSpanDistanceS ) {
	    /* we cannot find a better connection force stop */
	    Yheap_empty(heap);
	    break;

	  } else {

	    distance = nextNode->distance + adjNode->distance + 
	      EDGEWEIGHT(graph,adjEdge);

	    if ( distance < bestSpanDistanceS ) {
	      bestSpanDistanceS = distance;
	      bestSpanEdge = adjEdge;
	    }
	    
	  }
	}
      }
      
    }    /* END enumerate all of the adjacent nodes */
    
    D(routineNameS,
      fprintf(stderr,"%s: ready to pop heap\n",routineNameS);
      Ygraph_draw(graph); /* DEBUG */
      );
    
  }  /* end pop items off of the queue */
  
  /* if a bridge edge was found, trace it back in both directions */
  if ( bestSpanEdge ) {
    spanDeck = steiner_trace_back(bestSpanEdge);
  }

  D(routineNameS,
    fprintf(stderr,"%s single path finished:\n",routineNameS);
    Ygraph_draw(graph); /* DEBUG */
    );

  /* free up the heap */
  Yheap_free(heap);

  /* free the disjoint set */
  Ydset_free(dset,NULL);

  return (spanDeck);
}   /* end Ygraph_bfs */

/*--------------------------------------------------
  Return size of last required path
  --------------------------------------------------*/
INT Ygraph_requiredPathSize(graph)
     YGRAPHPTR graph;
{
  return(bestSpanDistanceS);
}

/*----------------------------------------------------------
  Ygraph_steiner:  Find the mst for a set of nodes.
  Returns a graph which spans the required nodes.
  The returned graph is not guaranteed to be optimal.
  A random interchange is performed maxImproves times in
  an attempt to reduce the weight of the spanning graph.
  The user can use Ygraph_size() to find the total cost
  of the returned steiner tree.
  ----------------------------------------------------------*/
YGRAPHPTR Ygraph_steiner(graph,maxImproves)
     YGRAPHPTR graph;
     int maxImproves;
{
  static char routineNameS[16] = "Ygraph_steiner";
  YNODEPTR node;
  YNODEPTR adjNode;
  YNODEPTR node1;
  YNODEPTR node2;
  YEDGEPTR edge;
  YHEAPPTR heap;
  YDECKPTR deck;
  YDECKPTR pathDeck;
  YDECKPTR savePrimeDeck;
  YDSETPTR dset;
  YDSETPTR saveSourceSet;
  YGRAPHPTR steinerGraph;
  INT (*compareEdge)();
  INT (*compareNode)();
  int done = FALSE;
  
  /* first run a sanity on the graph */
  D( routineNameS,
    ASSERTNFAULT(Ygraph_verify(graph),"generateTrees","bogus graph");
    );
  
  /* initialize internal data structures */

  /* use priority queue keyed on node distance for BFS type tree building */
  heap = Yheap_init_with_parms(compare_node_distance);

  /* Use disjoint sets to determine which nodes are part of a tree */
  dset = Ydset_init(set_compare_node_ptr);

  /* dummy deck */
  deck = Ydeck_init();

  /* save the users required nodes and prime edges */
  /* this routine will change them */
  savePrimeDeck = graph->primeDeck;
  saveSourceSet = graph->sourceSet;

  compareEdge = Yrbtree_get_compare(graph->edgeTree);

  compareNode = Yrbtree_get_compare(graph->nodeTree);

  steinerGraph = Ygraph_init(compareNode,compareEdge,
                             graph->userEdgeWeight,YGRAPH_NONDIRECTED);

  /* initialize the dset with nodes to be a part of the tree */
  for( node=(YNODEPTR)Ydset_enumerate(saveSourceSet,TRUE); node;
      node=(YNODEPTR)Ydset_enumerate(saveSourceSet,FALSE) ) {
    /* handle equivalent pins by always union with parent */
    adjNode = (YNODEPTR) Ydset_find_set(saveSourceSet,node);
    Ydset_union(dset,node,adjNode);
  }

   D(routineNameS,
     fprintf(stderr,"%s edge initialized\n",routineNameS);
     Ygraph_draw(graph); /* DEBUG */
     );

  /* initialize with dset with edge nodes required to be a part of the tree */
  for( Ydeck_top(savePrimeDeck); Ydeck_notEnd(savePrimeDeck);
       Ydeck_down(savePrimeDeck) ) {
     edge = (YEDGEPTR) Ydeck_getData(savePrimeDeck);
     node1 = edge->node1;
     node2 = edge->node2;
     
     edge->color = BLACK;  /* make edge as part of tree */
     Ygraph_edgeInsert(steinerGraph,edge->data,edge->weight,
            node1->data,node2->data);
      Ydset_union(dset,node1,node2);
   }

  D(routineNameS,
    fprintf(stderr,"%s steiner tree primed\n",routineNameS);
    Ygraph_draw(graph); /* DEBUG */
    );
  
  /* replace graph prime deck and source set with our own */
  /* this allows us to change the source set as the tree is built up */
  graph->sourceSet = dset;
  graph->primeDeck = deck;

  for ( pathDeck = Ygraph_requiredPath(graph); pathDeck;
        pathDeck = Ygraph_requiredPath(graph) ) {
    
     while ( edge = (YEDGEPTR) Ydeck_pop( pathDeck ) )  {
        node1 = edge->node1;
        node2 = edge->node2;
        edge->color = BLACK;
        Ygraph_edgeInsert(steinerGraph,edge->data,edge->weight,
              node1->data,node2->data);
        Ydset_union(graph->sourceSet,node1,node2);
     }

     Ydeck_free(pathDeck,NULL);
  }
  
  graph->primeDeck = savePrimeDeck;
  graph->sourceSet = saveSourceSet;

  Ygraph_steinerImprove(graph,steinerGraph,maxImproves);

  Yheap_free(heap);

  Ydeck_free(deck,NULL);

  Ydset_free(dset,NULL);

  D(routineNameS,
    fprintf(stderr,"spanning tree found with %d edges and %d nodes \n",
            Ygraph_edgeCount(steinerGraph), Ygraph_nodeCount(steinerGraph));
    Ygraph_draw(graph); /* DEBUG */
    );
  
  return(steinerGraph);
}

/*----------------------------------------
  Improves the steiner tree.
  The original graph must have its required
  nodes properly set.
  ----------------------------------------*/
VOID Ygraph_steinerImprove(graph,steinerGraph,maxIterations)
     YGRAPHPTR graph;
     YGRAPHPTR steinerGraph;
     int maxIterations;
{
  static char routineNameS[32]="Ygraph_steinerImprove";
  YTREEPTR pathTree;
  YDECKPTR primeDeck;
  YDECKPTR savePrimeDeck;
  YDECKPTR connectDeck;
  YEDGEPTR nextEdge;
  YEDGEPTR edge;
  YEDGEPTR edge2;
  YNODEPTR node;
  YNODEPTR nextNode;
  YNODEPTR lastNode;
  YNODEPTR node1;
  YNODEPTR node2;
  YNODEPTR origNode;
  INT oldWeight;
  INT newWeight;
  int count;
  int numEdges;
  int pass;
  int r;
  
  if ( ! Ygraph_nodeCount(steinerGraph) ) {
    M(ERRMSG,routineNameS,"aborting: steiner graph has no nodes\n");
    return;
  }

  if ( ! Ygraph_edgeCount(steinerGraph) ) {
    M(ERRMSG,routineNameS,"aborting: steiner graph has no edges\n");
    return;
  }

  if ( Ygraph_nodeCount(steinerGraph) < 3 ||
       Ygraph_edgeCount(steinerGraph) < 2  ) {
    Ygraph_edgeWeights2Size(steinerGraph);
    return;
  }

  pathTree = Yrbtree_init(compare_edge);
  primeDeck = Ydeck_init();
  
  while ( maxIterations-- ) {
    
    /* get a random edge */
    numEdges = Yrbtree_size(steinerGraph->edgeTree);
    r = Yacm_random() % numEdges;
    
    edge = (YEDGEPTR) Yrbtree_enumerate(steinerGraph->edgeTree,TRUE);
    while ( r-- ) {
      edge = (YEDGEPTR) Yrbtree_enumerate(steinerGraph->edgeTree,FALSE);
    }
    
    Yrbtree_insert(pathTree,edge);

    oldWeight = EDGEWEIGHT(steinerGraph,edge);
    
    node1 = edge->node1;
    node2 = edge->node2;
    
    /* build a path starting with this edge */
    /* the path should extend until the end points are */
    /* nodes with degree > 2  OR  a required node */
    
    for ( pass = 0; pass < 2; pass ++ ) {
      
      if ( pass ) {
	node = node1;
	lastNode = node2;
      } else {
	node = node2;
	lastNode = node1;
      }
      
      origNode = Ygraph_nodeFind(graph,node->data);
      
      while (  Ygraph_nodeDegree(node) <= 2 &&
	     ! Ydset_search(graph->sourceSet,origNode)  ) { 
	
	for ( count = 1; nextEdge = (YEDGEPTR) Ygraph_listAdjEdges(node,count);
	     count++ ) {
	  
	  /* get adjacent nodes */
	  if ( nextEdge->node1 == node ) {
	    nextNode = nextEdge->node2;
	  } else {
	    nextNode = nextEdge->node1;
	  }
	  
	  /* avoid backtrace */
	  if ( nextNode == lastNode ) {
	    continue;
	  }
	  
	  lastNode = node;
          node = nextNode;
          origNode = Ygraph_nodeFind(graph,node->data);

	  oldWeight += EDGEWEIGHT(steinerGraph,nextEdge);
	  Yrbtree_insert(pathTree,nextEdge);
	  break;
	}
	
        ASSERTNFAULT(node,"Ygraph_steinerImprove","dangling node found");
        ASSERTNFAULT(origNode, "Ygraph_steinerImprove",
                               "node in steiner, but not original graph\n");
	
      } /* end while node is unrequired or degree < 2 */
    }  /* end for each pass */
    
    /* save the users required nodes and prime edges */
    /* this routine will change them */
    savePrimeDeck = graph->primeDeck;
    graph->primeDeck = primeDeck;
    
    /* build a new prime deck.  This deck cointains all edges in */
    /* the existing steiner tree except the segment determined above */
    for ( edge = (YEDGEPTR) Yrbtree_enumerate(steinerGraph->edgeTree,TRUE);
	 edge;
	 edge = (YEDGEPTR) Yrbtree_enumerate(steinerGraph->edgeTree,FALSE) ) {
      
      if ( !Yrbtree_search(pathTree,edge) ) {
        node1 = edge->node1;	
        node2 = edge->node2;	
	edge2 = Ygraph_edgeFindByNodeData( graph, node1->data, node2->data );
	Ydeck_push(primeDeck,edge2);
      }
      
    }
    
    connectDeck = Ygraph_requiredPath(graph);
    
    /* restore the original prime deck */
    graph->primeDeck = savePrimeDeck;
    
    newWeight = 0;
    for ( Ydeck_top(connectDeck); Ydeck_notEnd(connectDeck);
	 Ydeck_down(connectDeck) ) {
      edge = (YEDGEPTR) Ydeck_getData(connectDeck);
      newWeight += edge->weight;
    }
    
    if ( newWeight < oldWeight ) {
      
      D("Ygraph_steinerImprove",
        fprintf(stderr,"steiner graph improved %d -> %d\n",
		oldWeight,newWeight);
	);

      /* remove old edges from the graph */
      for ( edge2 = (YEDGEPTR) Yrbtree_enumerate(pathTree,TRUE);
	   edge2;
	   edge2 = (YEDGEPTR) Yrbtree_enumerate(pathTree,FALSE) ) {
	Ygraph_edgeDelete(steinerGraph,edge2,NULL);
      }
      
      /* put new edges into the graph */
      while ( edge2 = (YEDGEPTR) Ydeck_pop(connectDeck) ) {
	node1 = edge2->node1;
	node2 = edge2->node2;
	Ygraph_edgeInsert(steinerGraph, edge2->data, edge2->weight,
			  node1->data, node2->data);
      }  /* end put new edges into graph */
      
    }  /* end if new weight is better than old weight */

      Yrbtree_empty(pathTree,NULL);
      Ydeck_empty(primeDeck,NULL);
      Ydeck_free(connectDeck,NULL);
    
  }    /* end for each iteration count */

  Ygraph_edgeWeights2Size(steinerGraph);

  Yrbtree_free(pathTree,NULL);      
  Ydeck_free(primeDeck,NULL);
}
  
/*----------------------------------------------------------
  Ygraph_nodeVerify:  exercise graph data structures
  ----------------------------------------------------------*/
int Ygraph_nodeVerify(node)
     YNODEPTR node;
{ 
  int rc = TRUE;

  if (YcheckDebug(node) < sizeof(YNODE)) {
    M(ERRMSG,"Ygraph_nodeVerify","node memory corrupt\n");
    rc = FALSE;
  }

  if ( YcheckDebug( &node->adjEdge[LO] )  < 
      ( sizeof(YEDGEPTR) * ( (INT) node->adjEdge[MAXSIZE] - LO + 1 ) ) ) {
    M(ERRMSG,"Ygraph_nodeVerify","bogus node adj edge list\n");
    rc = FALSE;
  }
  
  if ( YcheckDebug( &node->backEdge[LO] )  < 
      ( sizeof(YEDGEPTR) * ( (INT) node->backEdge[MAXSIZE] - LO + 1 ) ) ) {
    M(ERRMSG,"Ygraph_nodeVerify","bogus node adj edge list\n");
    rc = FALSE;
  }

   return(rc);
}

/*----------------------------------------------------------
  Ygraph_edgeVerify:  exercise graph data structures
  ----------------------------------------------------------*/
int Ygraph_edgeVerify(edge)
     YEDGEPTR edge;
{  
  int rc = TRUE;

  if (YcheckDebug(edge) < sizeof(YEDGE)) {
    M(ERRMSG,"Ygraph_edgeVerify","edge memory corrupt\n");
    rc = FALSE;
  }

  if (YcheckDebug(edge->node1) < sizeof(YNODE)) {
    M(ERRMSG,"Ygraph_edgeVerify","edge node1 memory corrupt\n");
    rc = FALSE;
  }

  if (YcheckDebug(edge->node2) < sizeof(YNODE)) {
    M(ERRMSG,"Ygraph_edgeVerify","edge node2 memory corrupt\n");
    rc = FALSE;
  }

  if ( ! (edge->type == YGRAPH_DIRECTED || edge->type == YGRAPH_NONDIRECTED)){
    M(ERRMSG,"Ygraph_edgeVerify","edge type is bogus\n");
    rc = FALSE;
  }

   return(rc);

}

/*----------------------------------------------------------
  Ygraph_verify:  exercise graph data structures
  ----------------------------------------------------------*/
int Ygraph_verify(graph)
     YGRAPHPTR graph;
{  
  YNODEPTR node;
  YNODEPTR node1;
  YNODEPTR node2;
  YEDGEPTR edge;
  YEDGEPTR edge2;
  YEDGEPTR edge3;
  YEDGEPTR edge4;
  YEDGEPTR edge5;
  YEDGEPTR adjEdge;
  YEDGEPTR *c_p;
  YEDGEPTR *l_p;
  
  int rc = TRUE;
  
  ASSERTNFAULT(YcheckDebug(graph) >= sizeof(YGRAPH),
	       "Ygraph_verify","graph memory corrupt");
  
  if ( ! Yrbtree_verify(graph->nodeTree) ) {
    M(ERRMSG,"Ygraph_verify","bogus graph node tree\n");
    rc = FALSE;
  }
  
  if ( ! Yrbtree_verify(graph->edgeTree) ) {
    M(ERRMSG,"Ygraph_verify","bogus graph edge tree\n");
    rc = FALSE;
  }
  
  if ( ! Ydeck_verify(graph->primeDeck) ) {
    M(ERRMSG,"Ygraph_verify","bogus prime deck\n");
    rc = FALSE;
  }
  
  if ( ! Ydeck_verify(graph->cyclePrimeDecks) ) {
    M(ERRMSG,"Ygraph_verify","cyclePrimeDecks\n");
    rc = FALSE;
  }
  
  if ( ! Ydset_verify(graph->sourceSet) ) {
    M(ERRMSG,"Ygraph_verify","bogus graph source tree\n");
    rc = FALSE;
  }
  
  if ( graph->flags < 0  || graph->flags > YGRAPH_DIRECTED ) {
    M(ERRMSG,"Ygraph_verify","bogus graph flags\n");
    rc = FALSE;
  }
  
  Ygraph_nodeEnumeratePush(graph);
  for ( node = (YNODEPTR) Ygraph_nodeEnumerate(graph,TRUE);
       node;
       node = (YNODEPTR) Ygraph_nodeEnumerate(graph,FALSE) ){

    if ( ! Ygraph_nodeVerify(node) ) {
      M(ERRMSG,"Ygraph_verify","graph has bogus node\n");
      rc = FALSE;
    }
    
    node2 = Ygraph_nodeFind(graph,node->data);
    if ( !node2 ) {
      M(ERRMSG,"Ygraph_verify","could not find node\n");
      rc = FALSE;
    }
    
    if ( node2 != node ) {
      M(WARNMSG,"Ygraph_verify","found duplicate node\n");
      rc = FALSE;
    }

    /* check node's adjacent edge list */
  
    c_p = node->adjEdge + START;
    l_p = c_p + (INT) node->adjEdge[SIZE];

    /* search list for edges */

    for ( ; c_p < l_p; c_p++ ) {
      adjEdge = *c_p;
      if ( ! Ygraph_edgeVerify(adjEdge) ) {
	M(ERRMSG,"Ygraph_verify","node has bogus adj edge\n");
	rc = FALSE;
      }
    }

    /* check node's back edge list */
  
    c_p = node->backEdge + START;
    l_p = c_p + (INT) node->backEdge[SIZE];

    /* search list for edges */

    for ( ; c_p < l_p; c_p++ ) {
      adjEdge = *c_p;
      if ( ! Ygraph_edgeVerify(adjEdge) ) {
	M(ERRMSG,"Ygraph_verify","node has bogus back edge\n");
	rc = FALSE;
      }
    }


  }
  Ygraph_nodeEnumeratePop(graph);
  
  /* Verify all the edges in the graph */
  Ygraph_edgeEnumeratePush(graph);
  for ( edge = (YEDGEPTR) Ygraph_edgeEnumerate(graph,TRUE);
       edge;
       edge = (YEDGEPTR) Ygraph_edgeEnumerate(graph,FALSE) ){
    
    if ( ! Ygraph_edgeVerify(edge) ) {
      M(ERRMSG,"Ygraph_verify","graph has bogus edge\n");
      rc = FALSE;
    }
    
    if ( ! Ygraph_nodeVerify(edge->node1) ) {
      M(ERRMSG,"Ygraph_verify","graph has bogus node\n");
      rc = FALSE;
    }
    
    if ( ! Ygraph_nodeVerify(edge->node2) ) {
      M(ERRMSG,"Ygraph_verify","graph has bogus node\n");
      rc = FALSE;
    }
    
    /* see if the edge's exists in its node adj trees  */

    node1 = edge->node1;
    node2 = edge->node2;

   if ( edge->node1 == edge->node2 ) {
      M(ERRMSG,"Ygraph_verify","edge has the same node on both ends\n");
      rc = FALSE;
   }

    /* enumerate all of the adjacent edges */
    c_p = node1->adjEdge + START;
    l_p = c_p + (INT) node1->adjEdge[SIZE];
    
    /* search list for edges */
    for ( edge2=NIL(YEDGEPTR) ; c_p < l_p; c_p++ ) {
      adjEdge = *c_p;
      if (adjEdge->node1 == node1 && adjEdge->node2 == node2 ) {
	edge2 = adjEdge;
	break;
      }
    }
    
    if ( !edge2 ) {
      M(ERRMSG,"Ygraph_verify","could not find edge in edge's node1 list\n");
      rc = FALSE;
    } else if ( edge2 != edge ) {
      M(WARNMSG,"Ygraph_verify","found duplicate edge in edge's node1 list\n");
      rc = FALSE;
    }

    c_p = node1->backEdge + START;
    l_p = c_p + (INT) node1->backEdge[SIZE];
    
    /* search list for edges */
    for ( edge3=NIL(YEDGEPTR) ; c_p < l_p; c_p++ ) {
      adjEdge = *c_p;
      if (adjEdge->node1 == node1 && adjEdge->node2 == node2 ) {
	edge3 = adjEdge;
	break;
      }
    }

    if ( edge3 ) {
      M(ERRMSG,"Ygraph_verify","found edge in edge's node1 back list\n");
      rc = FALSE;
    }

    /* enumerate all of the adjacent edges */
    c_p = node2->adjEdge + START;
    l_p = c_p + (INT) node2->adjEdge[SIZE];
    
    /* search list for edges */
    for ( edge4=NIL(YEDGEPTR) ; c_p < l_p; c_p++ ) {
      adjEdge = *c_p;
      if (adjEdge->node1 == node1 && adjEdge->node2 == node2 ) {
	edge4 = adjEdge;
	break;
      }
    }
    
    if ( edge->type == YGRAPH_DIRECTED ) {
      if ( edge4 ) {
        M(ERRMSG,"Ygraph_verify","directed edge in edge's node2 adj list\n");
        rc = FALSE;
      }
    } else if ( !edge4 ) {
      M(ERRMSG,"Ygraph_verify","could not find edge in edge's node2  list\n");
      rc = FALSE;
    } else if ( edge4 != edge ) {
      M(WARNMSG,"Ygraph_verify","found duplicate edge in edge's node2 list\n");
      rc = FALSE;
    }

    c_p = node2->backEdge + START;
    l_p = c_p + (INT) node2->backEdge[SIZE];
    
    /* search list for edges */
    for ( edge5=NIL(YEDGEPTR) ; c_p < l_p; c_p++ ) {
      adjEdge = *c_p;
      if (adjEdge->node1 == node1 && adjEdge->node2 == node2 ) {
	edge5 = adjEdge;
	break;
      }
    }
    
    if ( edge->type == YGRAPH_NONDIRECTED ) {
      if ( edge5 ) {
       M(ERRMSG,"Ygraph_verify","undirected edge in edge's node2 back list\n");
        rc = FALSE;
      }
    } else if ( !edge5 ) {
     M(ERRMSG,"Ygraph_verify","directed edge not in edge's node2 back list\n");
      rc = FALSE;
    } else if ( edge5 != edge ) {
      M(WARNMSG,"Ygraph_verify","duplicate edge in edge's node2 back list\n");
      rc = FALSE;
    }

    if ( !edge->node1 || !edge->node2) {
      M(ERRMSG,"Ygraph_verify","edge has Null node\n");
      rc = FALSE;
    }
    
  }    /* end for each edge */
  Ygraph_edgeEnumeratePop(graph);
  
  return(rc);
}

/*---------------------------------------------------------
  Dump graph statistics and call users print functions for
  nodes and edges
  ---------------------------------------------------------*/
VOID Ygraph_dump(graph,printNode,printEdge)
  YGRAPHPTR graph;
  VOID (*printNode)();
  VOID (*printEdge)();
{
  YEDGEPTR edge;
  YNODEPTR node;
  
  D("Ygraph_dump",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_dump","bogus graph");
    );
  
  fprintf(stderr,"graph has %d edges \n",Ygraph_edgeCount(graph));

  if ( printEdge ) {

    Ygraph_edgeEnumeratePush(graph);
    /* now draw the edges */
    for (edge=Ygraph_edgeEnumerate(graph,TRUE); edge;
	 edge=Ygraph_edgeEnumerate(graph,FALSE) ) {
      (*printEdge)(edge);    
    }     /* end for each edge */
    Ygraph_edgeEnumeratePop(graph);

  }
  

  fprintf(stderr,"graph has %d nodes \n",Ygraph_nodeCount(graph));

  if ( printNode ) {    

    Ygraph_nodeEnumeratePush(graph);
    /* now draw the nodes which have not yet been drawn */
    for (node = Ygraph_nodeEnumerate(graph,TRUE); node;
	 node = Ygraph_nodeEnumerate(graph,FALSE) ) {
      (*printNode)(node);    
    }
    Ygraph_nodeEnumeratePop(graph);
    
  }     /* end for each node */
  
}

/*---------------------------------------------------------
  Sets up the users draw functions for Ygraph_draw.
  ---------------------------------------------------------*/
VOID Ygraph_drawFunctions(graph,userNodeDraw, userEdgeDraw)
     YGRAPHPTR graph;
     VOID (*userNodeDraw)();
     VOID (*userEdgeDraw)();
{
  graph->userDrawNode = userNodeDraw;
  graph->userDrawEdge = userEdgeDraw;
}

/*---------------------------------------------------------
  Get the current edge weight function
  ---------------------------------------------------------*/
INT (*Ygraph_getEdgeWeightFunction(graph))()
     YGRAPHPTR graph;
{
  return( graph->userEdgeWeight );
}

/*---------------------------------------------------------
  Set the current edge weight function
  ---------------------------------------------------------*/
VOID Ygraph_setEdgeWeightFunction(graph,userEdgeWeight)
     YGRAPHPTR graph;
     INT (*userEdgeWeight)();
{
  graph->userEdgeWeight = userEdgeWeight;
}

/*---------------------------------------------------------
  Call the users drawing functions for all required nodes 
  To use this function first call Ygraph_drawFunctions().
  The user's node draw function will be passed a node and a color.
  The user's edge draw function will be passed an edge and a color.

  Some sample edge and node draw function are below:

  VOID drawNode(node,color)
    YNODEPTR node;
    int color;
  {
    myNodeType n;
    n = (myNodeType) Ygraph_nodeData(node);
    TW3DdrawLine(0,n->x,n->y,n->z,n->x,n->y,n->z,color,0);
  }

  VOID drawEdge(edge,color)
    YEDGEPTR edge;
    int color;
  {
    myEdgeType e;
    e = (myEdgeType) Ygraph_edgeData(edge);
    TW3DdrawLine(0,e->x1,e->y1,e->z1,e->x2,e->y2,e->z2,color,0);
  }
  ---------------------------------------------------------*/
VOID Ygraph_drawRequired(graph)
  YGRAPHPTR graph;
{
  YNODEPTR node;
  
  D("Ygraph_drawRequired",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_drawRequired","bogus graph");
    );
  
  if ( graph->userDrawNode ) {    

    /* now draw the nodes which have not yet been drawn */
    for (node = (YNODEPTR) Ydset_enumerate(graph->sourceSet,TRUE); node;
	 node = (YNODEPTR) Ydset_enumerate(graph->sourceSet,FALSE) ) {
      (*graph->userDrawNode)(node,node->color);    
    }

    
  }     /* end for each node */
  
  TWflushFrame();       /* draw any pending events */
  
  D("Ygraph_drawRequired",
    TWmessage("Waiting for keypress:");
    getchar();
    TWmessage("keypress acknowledged...");
    );
}

/*---------------------------------------------------------
  Call the users drawing functions for all primed edges
  User must write drawing routines since library has no
  way of knowing what node and edge data is.
  To use this function first call Ygraph_drawFunctions().
  The user's node draw function will be passed a node and a color.
  The user's edge draw function will be passed an edge and a color.

  Some sample edge and node draw function are below:

  VOID drawNode(node,color)
    YNODEPTR node;
    int color;
  {
    myNodeType n;
    n = (myNodeType) Ygraph_nodeData(node);
    TW3DdrawLine(0,n->x,n->y,n->z,n->x,n->y,n->z,color,0);
  }

  VOID drawEdge(edge,color)
    YEDGEPTR edge;
    int color;
  {
    myEdgeType e;
    e = (myEdgeType) Ygraph_edgeData(edge);
    TW3DdrawLine(0,e->x1,e->y1,e->z1,e->x2,e->y2,e->z2,color,0);
  }
  ---------------------------------------------------------*/
VOID Ygraph_drawPrime(graph)
  YGRAPHPTR graph;
{
  YEDGEPTR edge;
  
  D("Ygraph_drawPrime",
    ASSERTNFAULT(Ygraph_verify(graph),"Ygraph_drawPrime","bogus graph");
    );
  
  if ( graph->userDrawEdge ) {    
    
    /* now draw the nodes which have not yet been drawn */
    for ( Ydeck_top(graph->primeDeck); Ydeck_notEnd(graph->primeDeck);
	 Ydeck_down(graph->primeDeck ) ) {
      edge = (YEDGEPTR) Ydeck_getData(graph->primeDeck);
      (*graph->userDrawEdge)(edge,edge->color);    
    }
    
    
  }     /* end for each node */
  
  TWflushFrame();       /* draw any pending events */

  D("Ygraph_drawPrime",
    TWmessage("Waiting for keypress:");
    getchar();
    TWmessage("keypress acknowledged...");
    );
}

/*---------------------------------------------------------
  Call the users drawing functions for all nodes and edges
  User must write drawing routines since library has no
  way of knowing what node and edge data is.
  To use this function first call Ygraph_drawFunctions().
  The user's node draw function will be passed a node and a color.
  The user's edge draw function will be passed an edge and a color.

  Some sample edge and node draw function are below:

  VOID drawNode(node,color)
    YNODEPTR node;
    int color;
  {
    myNodeType n;
    n = (myNodeType) Ygraph_nodeData(node);
    TW3DdrawLine(0,n->x,n->y,n->z,n->x,n->y,n->z,color,0);
  }

  VOID drawEdge(edge,color)
    YEDGEPTR edge;
    int color;
  {
    myEdgeType e;
    e = (myEdgeType) Ygraph_edgeData(edge);
    TW3DdrawLine(0,e->x1,e->y1,e->z1,e->x2,e->y2,e->z2,color,0);
  }
  ---------------------------------------------------------*/

VOID Ygraph_draw(graph)
  YGRAPHPTR graph;
{
  YEDGEPTR edge;
  YNODEPTR node;
  
  D("Ygraph_draw",
    ASSERTNFAULT(Ygraph_verify(graph),"generateTrees","bogus graph");
    );
  
  if ( graph->userDrawEdge ) {

    Ygraph_edgeEnumeratePush(graph);
    /* now draw the edges */
    for (edge=Ygraph_edgeEnumerate(graph,TRUE); edge;
	 edge=Ygraph_edgeEnumerate(graph,FALSE) ) {
      (*graph->userDrawEdge)(edge,edge->color);    
    }     /* end for each edge */
    Ygraph_edgeEnumeratePop(graph);

  }
  

  if ( graph->userDrawNode ) {    

    Ygraph_nodeEnumeratePush(graph);
    /* now draw the nodes which have not yet been drawn */
    for (node = Ygraph_nodeEnumerate(graph,TRUE); node;
	 node = Ygraph_nodeEnumerate(graph,FALSE) ) {
      (*graph->userDrawNode)(node,node->color);    
    }
    Ygraph_nodeEnumeratePop(graph);
    
  }     /* end for each node */
  
  TWflushFrame();       /* draw any pending events */
  
  D("Ygraph_draw",
    TWmessage("Waiting for keypress:");
    getchar();
    TWmessage("keypress acknowledged...");
    );
  
}

#ifdef TEST
/* ##################################################################
                       TEST OF GRAPH ROUTINES
   ################################################################## */
typedef struct {
  INT  len;
  char *name ;
} DATA, *DATAPTR ;

static int compare_node( node1, node2 )
     YNODEPTR node1;
     YNODEPTR node2;
{
  DATAPTR n1;
  DATAPTR n2;
  
  n1 = Ygraph_nodeData(node1);
  n2 = Ygraph_nodeData(node2);
  
  return( strcmp(n1->name,n2->name) );
} /* end node */

static VOID print_node_data( node )
     YNODEPTR node;
{
  DATAPTR data ;
  data = (DATAPTR) Ygraph_nodeData(node);
  fprintf( stderr, "%s:%d ", data->name, data->len ) ;
} /* end print_data() */

static VOID print_node_info( node )
     YNODEPTR node;
{
  DATAPTR data ;
  data = (DATAPTR) Ygraph_nodeData(node);
  fprintf( stderr, "node %s has %d neighbors\n",
                    data->name, Ygraph_nodeDegree(node) ) ;
} /* end print_data() */

static VOID print_edge_info( edge )
     YEDGEPTR edge;
{
  DATAPTR n1;
  DATAPTR n2;

  n1 = Ygraph_edgeNode1Data(edge);
  n2 = Ygraph_edgeNode2Data(edge);

  fprintf( stderr, "edge from ");
  fprintf( stderr, "%s\t ", n1->name ) ;
  fprintf( stderr, "to ");
  fprintf( stderr, "%s\t ", n2->name ) ;
  fprintf( stderr, "has weight = %d ", Ygraph_edgeWeight(edge) ) ;
  fprintf( stderr, "\n") ;
} /* end print_data() */

static VOID free_data( data )
     DATAPTR data;
{
  YFREE( data->name ) ;
  YFREE( data ) ;
} /* end print_data() */

static char *make_data( string )
     char *string ;

{
  DATAPTR data ;
  
  data = YMALLOC( 1, DATA ) ;
  data->len = strlen(string) + 1 ;
  data->name = YMALLOC( data->len, char ) ;
  strcpy( data->name, string ) ;
  return( (char *) data ) ;
} /* end make_data */

main()
{
  YGRAPHPTR graph;
  DATAPTR the;
  DATAPTR one;
  DATAPTR red;
  DATAPTR fox;
  DATAPTR jumped;
  DATAPTR silverish;
  DATAPTR n2;
  YNODEPTR theNode;
  YNODEPTR jumpedNode;
  YNODEPTR node;
  YEDGEPTR edge;
  YDECKPTR pathDeck;

  YdebugMemory( TRUE ) ;
    
  fprintf(stderr,"start\n");

  /* initialize a graph with no compare edge function AND no graph flags */
  graph = Ygraph_init(compare_node,NULL,NULL,NULL);

  /* make some data */
  the = (DATAPTR) make_data("the");  
  one = (DATAPTR) make_data("one");  
  red = (DATAPTR) make_data("red");  
  fox = (DATAPTR) make_data("fox");  
  jumped = (DATAPTR) make_data("jumped");
  silverish = (DATAPTR) make_data("silverish");  

  /* build the graph with edge weights = 1 but with no data */
  Ygraph_edgeInsert(graph,NULL,1,the,one);
  Ygraph_edgeInsert(graph,NULL,1,one,red);
  Ygraph_edgeInsert(graph,NULL,1,red,fox);
  Ygraph_edgeInsert(graph,NULL,1,fox,jumped);
  Ygraph_edgeInsert(graph,NULL,1,the,silverish);
  Ygraph_edgeInsert(graph,NULL,1,silverish,fox);

  /* get the nodes which contain the data */
  theNode = Ygraph_nodeFind(graph,the);
  jumpedNode = Ygraph_nodeFind(graph,jumped);

  /* verify integrity of graph */
  Ygraph_verify(graph);

  /* perform a breath first search from starting with node "the" */
  Ygraph_bfs(graph,theNode,NULL);

  /* find the shortest path from "the" to "jumped" */
  pathDeck = Ygraph_path(graph,jumpedNode);

  /* print out the shortest path */
  fprintf(stderr,"\nshortest path from `the` to `jumped`\n");
  fprintf(stderr,"edge weights = 1\n");
  while ( node = (YNODEPTR) Ydeck_pop(pathDeck) ) {
    print_node_data(node);    
  }
  fprintf(stderr,"\n");

  /* free the path deck */
  Ydeck_free(pathDeck,NULL);

  /* now set all edge weights equal to number of chars in node2 data */
  for ( edge = Ygraph_edgeEnumerate(graph,TRUE); edge;
        edge = Ygraph_edgeEnumerate(graph,FALSE) ) {
     n2 = Ygraph_edgeNode2Data(edge);
     Ygraph_edgeWeightSet(edge,n2->len);
  }
  /* perform a breath first search from starting with node "the" */
  Ygraph_bfs(graph,theNode,NULL);

  /* find the shortest path from "the" to "jumped" */
  pathDeck = Ygraph_path(graph,jumpedNode);

  /* print out the shortest path */
  fprintf(stderr,"\nshortest path from `the` to `jumped`\n");
  fprintf(stderr,"edge weights = number of chars word\n");
  while ( node = (YNODEPTR) Ydeck_pop(pathDeck) ) {
    print_node_data(node);    
  }
  fprintf(stderr,"\n");

  /* print whole tree */
  fprintf(stderr,"FINAL GRAPH\n");
  fprintf(stderr,"===========\n");
  Ygraph_dump(graph,print_node_info,print_edge_info);
  fprintf(stderr,"\n");

  /* free the path deck */
  Ydeck_free(pathDeck,NULL);


  /* free the graph */
  Ygraph_free(graph, free_data, NULL);  
  fprintf( stderr, "Final memory:%d\n", YgetCurMemUse() ) ;
  Yprint_stats( stderr ) ;
  Ydump_mem() ;

  exit(0) ;
}

#endif /* TEST */
