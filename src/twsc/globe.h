#ifndef H_GLOBE
#define H_GLOBE
extern int implicit_pins_usedG ;
extern BOOL connectFlagG ;

void unequiv_pin_pre_processing();
void rebuild_nextpin();
int globe();
void globe_free_up();
void preFeedAssgn();
void FeedAssgn( int row );
void free_static_in_globe();
void elim_unused_feedsSC();
void rebuild_cell_paths();
int improve_place_sequential( int row , int index );
int cell_rotate( int row , int index );
void relax_padPins_pinloc();
void relax_unequiv_pinloc();
int check_unequiv_connectivity();
void row_seg_intersect( PINBOXPTR ptr1 , PINBOXPTR ptr2 , SEGBOXPTR segptr );
void copy_workerS_field( FEED_SEG_PTR aptr, FEED_SEG_PTR bptr );
void assgn_impin( IPBOXPTR imptr , FEED_SEG_PTR fsptr , int row );
#endif
