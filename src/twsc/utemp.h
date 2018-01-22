#ifndef H_UTEMP
#define H_UTEMP
extern BOOL pairtestG;

int comparex() ;
int compute_attprcel() ;
void from_middle();
void from_beginning() ;
void elim_nets(int) ;
void init_control(int);
void init_uloop();
void utemp();
int rm_overlapping_feeds();
int refine_fixed_placement();
#endif
