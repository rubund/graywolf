void add_adj( SEGBOXPTR segptr, PINBOXPTR node );
void assgn_channel_to_seg();
void remove_overlap_segment( int net );
void free_chan_seg();
void check_overlap_at_pin( PINBOXPTR ptr );
void rm_segm_overlap( SEGBOXPTR *checkseg , int m );
int check_connectivity( int net );
void replace_seg( PINBOXPTR netptr, PINBOXPTR oldnode, PINBOXPTR newnode );
void depth_first_check( PINBOXPTR ptr , SEGBOXPTR oldedge );
