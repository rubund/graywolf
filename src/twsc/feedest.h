#ifndef H_FEED_TEST
#define H_FEED_TEST
extern int add_Lcorner_feedG ;
extern int *rowfeed_penaltyG ;

void feedest();
void update_feedest( int net );
void re_estimate_feed_penalty();
void free_up_feedest_malloc();
void update_segment_data( SEGBOXPTR segptr );
SEGBOXPTR makeseg( PINBOXPTR lowptr, PINBOXPTR highptr );
#endif
