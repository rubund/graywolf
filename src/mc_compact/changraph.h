#include "compact.h"
void init_graph( int numnodes, int numedges );
void addNode( int node, int xc, int yc );
void addEdge( int node1 , int node2, BOOL HnotV, int cell_lb, int cell_rt );
void stretch_graph( INFOPTR stretch_edge, int x, int y );
void build_trees();
