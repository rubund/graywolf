typedef struct graph_edge_cost {
    SHORT node1 ;
    SHORT node2 ;
    int cost ;
    int channel ;
}
*EDGE_COST ,
EDGE_COST_BOX ;

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
