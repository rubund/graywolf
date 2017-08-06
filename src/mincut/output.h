void addCell(int celltype, char *cellname);
void set_bbox( int left, int right, int bottom, int top );
void addNet(char *signal );
void read_par();
void update_stats( FILE *fp );
void output( FILE *fp );
void init();
void write_softpins( FILE *fp );

typedef struct {
	BOOL io_signal ;
	char *net ;
} NETBOX, *NETPTR ;

extern YHASHPTR netTableS ;    /* hash table for cross referencing nets */
