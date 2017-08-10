#ifndef H_NETGRAPH
#define H_NETGRAPH

typedef struct graph_edge_cost {
    short node1 ;
    short node2 ;
    int cost ;
    int channel ;
}
*EDGE_COST ,
EDGE_COST_BOX ;

extern int *count_G;
extern int *father_G;
extern int *root_G;

extern PINBOXPTR *vertex_G;

void do_set_union( int i , int j );
void postFeedAssgn();
void postFeedAssgn_carl();
void switchable_or_not();
void remove_unnecessary_feed( int net , int flag );
void free_z_memory();
void netgraph_free_up();
void rebuild_netgraph( int net );
int find_set_name( int v );
void rebuild_netgraph_carl( int net );
#endif
