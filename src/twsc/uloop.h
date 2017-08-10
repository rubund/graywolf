#ifndef H_ULOOP
#define H_ULOOP

extern BOOL fences_existG ;
extern double avg_timeG ; /* average random time penalty */
extern double avg_funcG ; /* average random wirelength penalty */
extern double start_timeG ;/* initial target value for the time penalty */
extern double end_timeG ;  /* final target value for the time penalty - obviously zero */
extern double ratioG;
extern double finalRowControlG ;
extern double initialRowControlG ;
extern double TG ;

void save_control(FILE *fp);
void read_control( FILE *fp );
void uloop();
void pick_position(int *x, int *y, int ox, int oy,double scale);
void pick_fence_position(int *x, int *y, FENCEBOX *fence);
void update_window_size( double iternum );
int tw_frozen( int cost );
void rowcon();

#endif
