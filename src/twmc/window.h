void save_window( FILE *fp );
void init_control(BOOL first);
int read_window(FILE *fp);
double eval_ratio( int iteration );
void pick_neighborhood(int *x, int *y, int ox, int oy, FIXEDBOXPTR fixptr);
void update_window_size(double iteration);
