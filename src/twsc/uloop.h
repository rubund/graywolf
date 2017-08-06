void save_control(FILE *fp);
void read_control( FILE *fp );
void uloop();
void pick_position(int *x, int *y, int ox, int oy,double scale);
void pick_fence_position(int *x, int *y, FENCEBOX *fence);
void update_window_size( double iternum );
int tw_frozen( int cost );
void rowcon();
