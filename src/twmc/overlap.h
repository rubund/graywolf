void turn_wireest_on( int turn_on );
void setup_Bins( CELLBOXPTR s_cellptr, int s_xc, int s_yc, int s_orient );
void add2bin( MOVEBOXPTR *cellpos );
void sub_penal( MOVEBOXPTR *cellpos );
void add_penal( MOVEBOXPTR *cellpos );
int overlap();
int update_overlap();
int overlap2();
int update_overlap2();
