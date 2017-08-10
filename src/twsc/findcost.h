#ifndef H_FIND_COST
#define H_FIND_COST
int get_max_pin();
void spread_equal_cells();
void spread_cells();
int findcost();
void set_print_pin( int pins );
void find_net_sizes();
void create_cell( );

extern int average_pin_sepG;
extern int average_feed_sepG;
extern int minxspanG;
extern int *total_feed_in_the_rowG;
extern int fdWidthG;
extern int *feeds_in_rowG;

extern double mean_widthG ;
#endif
