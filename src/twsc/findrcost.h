#ifndef H_FIND_R_COST
#define H_FIND_R_COST

extern DENSITYPTR **DboxHeadG ;

extern int *maxTrackG;
extern int *nmaxTrackG;
extern int **cedgebinS;
extern int cedge_binwidthS;
extern int num_edgebinS;
extern int max_tdensityG;

void findrcost();
void initial_tracks( SEGBOXPTR segptr );
void set_cedgebin();
void reset_track();
int facing_cellheight( int pin , int row , int pinloc , int status );

#endif
