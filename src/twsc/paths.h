#ifndef H_PATHS
#define H_PATHS
int dcalc_min_path_len() ;
int dcalc_max_path_len() ;
int dcalc_path_len(int, int);
void add2path_set( int path );
void init_net_set();
void clear_path_set();
void clear_net_set();
int dcalc_full_penalty( int newtimepenal );
int calc_incr_time( int cell );
void update_time( int cell );
int calc_incr_time2( int cella, int cellb );
void update_time2() ;
void add2net_set( int net );
void calc_init_timeFactor();
void print_paths();
BOOL member_net_set( int net );
int dprint_error();
int dpath_len( int net_num, BOOL old_not_new );
#endif
