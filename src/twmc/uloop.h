void set_dump_ratio( int count );
void make_movebox();
void save_uloop( FILE *fp );
int read_uloop(FILE *fp);
void uloop( int limit );
void output_move_table(int *flip, int *att, double *move_size);
