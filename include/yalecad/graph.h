/*----------------------------------------------------------------- 
FILE:	    Graph.h                                       
DESCRIPTION:Graph include file for set of generic graph routines
CONTENTS:   
DATE:	    Jun 01, 1991
REVISIONS:  
----------------------------------------------------------------- */
#ifndef YGRAPH_H
#define YGRAPH_H

#ifndef lint
static char YgraphId[] = "@(#) Graph.h version 1.8 8/19/91";
#endif

#include <yalecad/base.h>
#include <yalecad/deck.h>
#include <yalecad/dset.h>
#include <yalecad/rbtree.h>

#ifndef GRAPH_DEFINED
typedef struct graph *YGRAPHPTR;
typedef struct graph_node *YNODEPTR;
typedef struct graph_edge *YEDGEPTR;
#endif

#define YGRAPH_DIRECTED    1
#define YGRAPH_NONDIRECTED 0

/*-------------------------------------------------------------
  Ygraph_init: Initialize a graph.
  User must make either make the compare_edge function unique
  or set compare_edge equal to NULL
  The user can set an edge's weight statically.
  Or, the use can set an edge's weight dynamicallly.
  Thus, the user edge weight function is optional and only
  should be used if the user need dynamic edge weights.
  -------------------------------------------------------------*/
extern YGRAPHPTR Ygraph_init(P4(INT(*compare_node)(),
                                INT(*compare_edge)(),
                                INT(*user_edge_weight)(),
                                INT flags));
/*
Arguments:
INT (*compare_node)() - users node data comparison function
INT (*compare_edge)() - users edge data comparicson function
INT (*user_edge_weight)() - users edge weight function
INT flags - the bits in the field allow for things like directed edges
            and redundant edges.

Function:  
This function initializes the data structures used by the various graphics
routines.  In order to use all of the graph routines, the user must create
two functions. The compare_node comparison function should be unique as some of
the graph routines make use of disjoint sets.  The compare_edge function
can be anything the user desires, however, the use may want to define
the compare_edge in terms of compare_node so that an edge may be retreived
on the basis of nodeData only.

The bits in the flags variable can be used to turn on directed graph behavior.
The default setting is non directed edges.

Example:

1) myGraph = Ygraph_init( myNodeCompare, myEdgeCompare, 0, 0);
   This initializes a graph with non directed edges.

2)  myGraph = Ygraph_init( myNodeCompare, myEdgeCompare, 0, YGRAPH_DIRECTED );
   This initializes a graph with directed edges.
*/

/*-------------------------------------------------
  Ygraph_flagsSet
  Set flags for graph operation.
  Use YGRAPH_DIRECTED, YGRAPH_NONDIRECTED
  -------------------------------------------------*/
INT Ygraph_flagsSet( P2(YGRAPHPTR graph, INT flag) );

/*-------------------------------------------------
  Ygraph_nodeInsert: insert a node into the graph  
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodeInsert( P2(YGRAPHPTR graph,
			     VOIDPTR nodeData
			     ));
     
/*
Arguments:
  YGRAPHPTR graph - the graph created with Ygraph_init();
  INT node - identification of node
  VOIDPTR nodeData - users optional node Data
    
Function:
  This routine inserts a node into the graph data structure.
  A new node is not created if the node already exists.
    
Example:
  1) Ygraph_nodeInsert( myGraph, 3, myNodeData);
  This creates a node with id 3.
*/
     


/*--------------------------------------------------
  Ygraph_edgeDelete: free edges from data structure
  --------------------------------------------------*/
extern VOID Ygraph_edgeDelete(P3(YGRAPHPTR graph,YEDGEPTR  edge,VOID (*userEdgeFree)()));

/*-------------------------------------------------
  Ygraph_edgeInsert: insert an edge into the graph  
 
  The new YEDGEPTR is returned.
  If the edge already existed, NULL is returned.
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_edgeInsert( P5(YGRAPHPTR graph,
			     VOIDPTR   edgeData,
			     INT       edgeWeight,
			     VOIDPTR   nodeData1,
			     VOIDPTR   nodeData2
			     ));
/*
Arguments
YGRAPHPTR graph - the graph created with Ygraph_init();
INT node1 - identification of first node in edge;
INT node2 - identification of second node in edge;
INT edgeWeight - weight of the edge
VOIDPTR edgeData - users optional edge Data

Function:
This routine inserts an edge into the graph data structure.
Redundant edges are not added to the data structure.

If the graph flag YGRAPH_DIRECTED is set, then
only node2 will be in node1's adjacency list.
Otherwise, node1 will also be put into node2's adjacency list.

The new YEDGEPTR is returned.  If the edge already exists,
then NULL returned.

Example:
1) Ygraph_insert( myGraph, 3, 5, 20, myEdgeData);
   This creates an edge between node 3 and node 5 with a weight of 20
*/

/*---------------------------------------------
  empty all nodes and edge from the graph
  ---------------------------------------------*/
VOID Ygraph_empty(P3(YGRAPHPTR graph,VOID (*nodeDelete)(),VOID (*edgeDelete)()));

/*-------------------------------------------------
  Ygraph_nodeCount
  -------------------------------------------------*/
extern INT Ygraph_nodeCount( P1(  YGRAPHPTR graph ));

/*-------------------------------------------------
  Ygraph_edgeCount
  -------------------------------------------------*/
extern INT Ygraph_edgeCount( P1(  YGRAPHPTR graph ));

/*-------------------------------------------------
  Ygraph_edgeWeight
   Returns the weigh of an edge
  -------------------------------------------------*/
INT Ygraph_edgeWeight( P1 (YEDGEPTR edge) );

/*-------------------------------------------------
  Ygraph_edgeWeightSet
  Sets the edge weight to weight
  -------------------------------------------------*/
INT Ygraph_edgeWeightSet( P2 (YEDGEPTR edge, INT weight) );

/*-------------------------------------------------
  Ygraph_edgeNode1Data
  -------------------------------------------------*/
int Ygraph_edgeType( P1 (YEDGEPTR edge) );

/*-------------------------------------------------
  Ygraph_nodeDegree
  -------------------------------------------------*/
extern INT Ygraph_nodeDegree(P1(YNODEPTR node));

/*-------------------------------------------------
  Ygraph_nodeFind
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodeFind(P2(YGRAPHPTR graph,VOIDPTR nodeData));

/*-------------------------------------------------
  Ygraph_nodeFindClosest - find closest node in graph
  Use same functions as rbtree_search_closest.
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodeFindClosest(P3(YGRAPHPTR graph,
    VOIDPTR nodeData,INT function));

/*-------------------------------------------------
  Ygraph_nodeSuc
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodeSuc(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_nodePred
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodePred(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_edgeFind
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_edgeFind(P4(YGRAPHPTR graph,VOIDPTR edgeData, 
                          VOIDPTR node1Data,VOIDPTR node2Data));

/*-------------------------------------------------
  Ygraph_edgeFindByNodes
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_edgeFindByNodes( P3(YGRAPHPTR graph,
			   YNODEPTR node1, YNODEPTR node2 ) ) ;

/*-------------------------------------------------
  Ygraph_edgeFindByNodeData
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_edgeFindByNodeData( P3(YGRAPHPTR graph,
		       VOIDPTR node1Data, VOIDPTR node2Data ) ) ;

/*-------------------------------------------------
  Ygraph_edgeData
  -------------------------------------------------*/
extern VOIDPTR Ygraph_edgeData( P1( YEDGEPTR edge));

/*-------------------------------------------------
  Ygraph_edgeNode1
  -------------------------------------------------*/
extern YNODEPTR Ygraph_edgeNode1( P1(YEDGEPTR edge));

/*-------------------------------------------------
  Ygraph_edgeNode2
  -------------------------------------------------*/
extern YNODEPTR Ygraph_edgeNode2( P1( YEDGEPTR edge));

/*-------------------------------------------------
  Ygraph_edgeNode1Data
  -------------------------------------------------*/
extern VOIDPTR Ygraph_edgeNode1Data( P1(YEDGEPTR edge));

/*-------------------------------------------------
  Ygraph_edgeNode2Data
  -------------------------------------------------*/
extern VOIDPTR Ygraph_edgeNode2Data( P1( YEDGEPTR edge));

/*-------------------------------------------------
  Ygraph_nodeData
  -------------------------------------------------*/
extern VOIDPTR Ygraph_nodeData( P1( YNODEPTR node));

/*--------------------------------------------------
  free and empty the graph
  --------------------------------------------------*/
extern VOID Ygraph_free(P3(YGRAPHPTR graph,VOID (*nodeDelete)(),VOID (*edgeDelete)()));

/*-------------------------------------------------
  Ygraph_edgeEnumerate
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_edgeEnumerate(P2( YGRAPHPTR graph, BOOL startFlag));

/*-------------------------------------------------
  Ygraph_edgeEnumeratePush
  -------------------------------------------------*/
extern VOID Ygraph_edgeEnumeratePush(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_edgeEnumeratePop
  -------------------------------------------------*/
extern VOID Ygraph_edgeEnumeratePop(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_edgeMin
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_edgeMin(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_edgeMax
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_edgeMax(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_listAdjEdges
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_listAdjEdges(P2(YNODEPTR node, INT listNum));
     
/*-------------------------------------------------
  Ygraph_listBackEdges
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_listBackEdges(P2(YNODEPTR node, INT listNum));
     
/*-------------------------------------------------
  Ygraph_nodeEnumerate
  -------------------------------------------------*/
extern  YNODEPTR Ygraph_nodeEnumerate(P2( YGRAPHPTR graph, BOOL startFlag));

/*-------------------------------------------------
  Ygraph_nodeEnumeratePop
  -------------------------------------------------*/
extern VOID Ygraph_nodeEnumeratePop( P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_nodeEnumeratePush
  -------------------------------------------------*/
extern VOID Ygraph_nodeEnumeratePush( P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_nodeMin
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodeMin(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_nodeMax
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodeMax(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_listAdjNodes
  -------------------------------------------------*/
extern YNODEPTR Ygraph_listAdjNodes(P2(YNODEPTR node,INT listNum));

/*-------------------------------------------------
  Ygraph_listBackNodes
  -------------------------------------------------*/
extern YNODEPTR Ygraph_listBackNodes(P2(YNODEPTR node,INT listNum));

/*-------------------------------------------------
  Ygraph_nodeInterval
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodeInterval( P4(YGRAPHPTR graph, VOIDPTR loData, VOIDPTR hiData, BOOL startFlag) );

/*-------------------------------------------------
  Ygraph_nodeIntervalPush
  -------------------------------------------------*/
VOID Ygraph_nodeIntervalPush(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_nodeIntervalPop
  -------------------------------------------------*/
VOID Ygraph_nodeIntervalPop(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_edgeInterval
  -------------------------------------------------*/
extern YEDGEPTR Ygraph_edgeInterval( P4(YGRAPHPTR graph, VOIDPTR loData, VOIDPTR hiData, BOOL startFlag) );

/*-------------------------------------------------
  Ygraph_edgeIntervalPush
  -------------------------------------------------*/
VOID Ygraph_edgeIntervalPush(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_edgeIntervalPop
  -------------------------------------------------*/
VOID Ygraph_edgeIntervalPop(P1(YGRAPHPTR graph));

/*--------------------------------------------------
  free_node: free node from data structure
  --------------------------------------------------*/
extern VOID Ygraph_nodeDelete(P4(YGRAPHPTR graph,YNODEPTR node,
                         VOID (*nodeFree)(),VOID (*edgeFree)()));

/*-------------------------------------------------
  Ygraph_nodeVeriy: exercise node data structure
  -------------------------------------------------*/
extern int Ygraph_nodeVerify(P1(YNODEPTR node));

/*-------------------------------------------------
  Ygraph_edgeVeriy: exercise edge data structure
  -------------------------------------------------*/
extern int Ygraph_edgeVerify(P1(YEDGEPTR edge));

/*-------------------------------------------------
  Ygraph_copy: returns a copy of a graph  
  -------------------------------------------------*/
extern YGRAPHPTR Ygraph_copy(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_size()
  Returns the graph size field.
  The size of the graph is set if the graph is
  returned by Ygraph_steiner().
  Also, Ygraph_edgeWeights2Size() can be called to
  add up all of the edge weights into a single size.
  -------------------------------------------------*/
extern INT Ygraph_size( P1 (YGRAPHPTR graph) );

/*-------------------------------------------------
  Ygraph_edgeWeights2Size
  Sum all of the edge weights, and put into the
  graph size field.
  -------------------------------------------------*/
extern INT Ygraph_edgeWeights2Size( P1 (YGRAPHPTR graph) );

/*------------------------------------------------------------
  Ygraph_bfs()
  Perform a beadth first seach rooted at sourceNode.
  Optionally, the user can provide a targetNode beyond
  which the search is terminated.
  ------------------------------------------------------------*/
extern VOID Ygraph_bfs(P3(YGRAPHPTR graph,YNODEPTR source,YNODEPTR target));

/*-----------------------------------------------------------
  Ygraph_path: returns a deck containing a path
  between a source and target.
  To use, first run Ygraph_bfs or Ygraph_dijkstra on a graph.
  Then, run Ygraph_path.  Node target will be traced back to
  its source, and the path will be returned in a deck.
  NOTE: The path deck is passed to the user and the user
        is responsible for freeing it.
  -----------------------------------------------------------*/
extern YDECKPTR Ygraph_path(P2(YGRAPHPTR graph,YNODEPTR target));

/*--------------------------------------------------
  Ygraph_dfs:  Perform a depth first search
  --------------------------------------------------*/
extern YDECKPTR Ygraph_dfs(P1(YGRAPHPTR graph));

/*----------------------------------------------------------
  Ygraph_mst_Kruskal:  Find a mst using Kruskal's algorithm
  ----------------------------------------------------------*/
extern YDECKPTR Ygraph_mst_kruskal(P1(YGRAPHPTR graph));

/*----------------------------------------------------------
  Ygraph_mst_prim:  Find a mst using prim's algorithm
  ----------------------------------------------------------*/
extern YDECKPTR Ygraph_mst_prim(P2(YGRAPHPTR graph,YNODEPTR source));

/*----------------------------------------------------------
  Ygraph_mst_dijkstra:  Find a shortest paths from source
  ----------------------------------------------------------*/
extern VOID Ygraph_dijkstra(P2(YGRAPHPTR graph,YNODEPTR source));

/*----------------------------------------------------------
  Ygraph_bellman_ford: single source shortest path for 
  directed edge graph.
  Returns TRUE if shortest path found.
  Returns FALSE if negative weight cycle exists
  ----------------------------------------------------------*/
extern BOOL Ygraph_bellman_ford(P2(YGRAPHPTR graph,YNODEPTR source));

/*-------------------------------------------------------
  Ygraph_cycles: uses depth first search to find cycles
  in a graph.  The returned deck contains all cycles.
  Each cycle is a deck.  Thus, the returned deck contains
  decks.  The user is responsible for freeing memory.
  ---------------------------------------------------------*/
extern YDECKPTR Ygraph_cycles(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_clearRequired: clear all nodes which must
                       part of any steiner tree  
  -------------------------------------------------*/
extern VOID Ygraph_clearRequired(P1(YGRAPHPTR graph));

/*-------------------------------------------------
  Ygraph_nodeRequired: insert a node which must be
  part of any steiner tree  
  -------------------------------------------------*/
extern YNODEPTR Ygraph_nodeRequired(P3(YGRAPHPTR graph,YNODEPTR node,YNODEPTR equiv));

/*-------------------------------------------------
  Ygraph_nodeRequiredCount:
  Returns the number of nodes in the required node
  set.
  -------------------------------------------------*/
INT Ygraph_nodeRequiredCount( P1(YGRAPHPTR graph) );

/*-------------------------------------------------
  Ygraph_enumerateRequired: enumerates all nodes which
  must be part of any steiner tree or required path.
  -------------------------------------------------*/
extern YNODEPTR Ygraph_enumerateRequired(P2(YGRAPHPTR graph,BOOL startFlag));

/*------------------------------------------------------------
  Ygraph_requiredPath()
   Perform a beadth first seach to
   find a single path between nodes of different sets.
   Use Ygraph_nodeRequired() to set up initial sets.
   Use Ygraph_clearRequired() to clear initial sets.
  ------------------------------------------------------------*/
extern YDECKPTR Ygraph_requiredPath(P1(YGRAPHPTR graph));

/*--------------------------------------------------
  Return size of last required path
  --------------------------------------------------*/
extern INT Ygraph_requiredPathSize(P1(YGRAPHPTR graph));

/*----------------------------------------------------------
  Ygraph_steiner:  Find the mst for a set of nodes.
  Returns a graph which spans the required nodes.
  The returned graph is not guaranteed to be optimal.
  A random interchange is performed maxImproves times in
  an attempt to reduce the weight of the spanning graph.
  The user can use Ygraph_size() to find the total cost
  of the returned steiner tree.
  ----------------------------------------------------------*/
extern YGRAPHPTR Ygraph_steiner(P2(YGRAPHPTR graph,int maxImproves));

/*----------------------------------------
  Improves the steiner tree.
  The original graph must have its required
  nodes properly set.
  ----------------------------------------*/
VOID Ygraph_steinerImprove(P3(YGRAPHPTR graph,YGRAPHPTR steinerGraph,
     int maxIterations));

/*---------------------------------------
  Ygraph_copy: copy a graph.
  ---------------------------------------*/
extern YGRAPHPTR Ygraph_copy( P1(YGRAPHPTR graph) );

/*----------------------------------------------------------
  Ygraph_nodeVerify: Verify a nodes data structure
  ----------------------------------------------------------*/
extern int Ygraph_nodeVerify(P1( YNODEPTR node ));

/*----------------------------------------------------------
  Ygraph_verify: exercise graph data structure
  ----------------------------------------------------------*/
extern int Ygraph_verify(P1(YGRAPHPTR graph));

/*------------------------------------------------------------
  Ygraph_dump: print graph stats and call user print functions
  ------------------------------------------------------------*/
extern VOID Ygraph_dump(P3(YGRAPHPTR graph,VOID (*printNode)(),VOID (*printEdge)()));

/*---------------------------------------------------------
  Sets up the users draw functions for Ygraph_draw.
  ---------------------------------------------------------*/
extern VOID Ygraph_drawFunctions(P3( YGRAPHPTR graph, VOID (*userNodeDraw)(), VOID (*userEdgeDraw)()));

/*---------------------------------------------------------
  Get the current edge weight function
  ---------------------------------------------------------*/
extern INT (*Ygraph_getEdgeWeightFunction(P1(YGRAPHPTR graph)))();

/*---------------------------------------------------------
  Set the current edge weight function
  ---------------------------------------------------------*/
extern VOID Ygraph_setEdgeWeightFunction(P2(YGRAPHPTR graph,
					    INT (*userEdgeWeight)()));

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
extern VOID Ygraph_drawRequired(P1(YGRAPHPTR graph));

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
extern VOID Ygraph_drawPrime(P1(YGRAPHPTR graph));

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
extern VOID Ygraph_draw(P1(YGRAPHPTR graph));

#endif /* YGRAPH_H */
