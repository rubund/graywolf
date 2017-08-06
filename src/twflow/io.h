typedef struct ebox {
	int from ;                           /* from node for edge */
	int to ;                             /* to node for edge */
	char **argv ;                        /* argument vector */
	int  argc ;                          /* argument count */
	FPTR ifiles ;                        /* list of files */
	FPTR ofiles ;                        /* list of files */
	DPTR dlist ;                         /* list of drawables */
	struct ebox *next ;                  /* list of drawables */
} EDGEBOX, *EDGEPTR ;

void add_pdependency( int fromNode );
void unmark_edges();
void init( int numobj );
void add_object( char *pname, int node );
void add_path( char * pathname );
void start_edge( int fromNode );
void add_line( int x1, int y1, int x2, int y2 );
void set_file_type( BOOL type );
void add_fdependency( char *file ) ;
void add_box( int l, int b, int r, int t );
void set_window();
void process_arcs();
