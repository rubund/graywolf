typedef struct vertex_box {
    INT x ;
    INT y ;
    INT class ;
    struct vertex_box *next ;
} *VERTEXPTR, VERTEX_BOX ;

#define START 1
#define END 2
#define NOTHING 3

void multi_tiles();
void preprocess_multi();
BOOL fix_tiles();
void add_to_multi_list();
BOOL find_tile();
void init_vertex_list();
void add_extra_points();
void swap();
void add_pt();
void free_vertex_list();
void add_mtiles_to_xgraph();
void add_mtiles_to_ygraph();