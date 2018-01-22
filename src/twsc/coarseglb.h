#ifndef H_COARSEGLB
#define H_COARSEGLB
void assign_row_to_pin();
void set_up_grid( );
void feed_config( );
void compute_feed_diff( int iteration );
void initialize_feed_need();
int space_for_feed();
void update_feed_config( int iteration );
int no_of_feedthru_cells();
int set_node( int x );
void addin_feedcell();
void final_feed_config( );
void free_cglb_data();
void assign_row_to_pin();
void coarseglb();

extern int longest_row_lengthG ;
extern int *right_most_in_classG ;
extern int *row_rite_classG ;
extern int **fdcel_needG;
extern int *fdcel_addedG;
extern int hznode_sepG ;
extern int **pairArrayG ;
extern int blk_most_leftG ;
extern int fdthrusG ; 
extern int chan_node_noG ;
extern int blk_most_riteG ;
extern int *total_feed_in_the_rowG ;
extern int actual_feed_thru_cells_addedG ;

extern FEED_DATA **feedpptrG ;

#endif
