void updateFixedCells(BOOL initializeFlag);
void init_fixcell(int left, int bottom, int right, int top);
void update_fixed_record( CELLBOXPTR ptr, FIXEDBOXPTR fptr, BOOL initFlag );
void build_active_array();
void build_soft_array();
void determine_origin( int *x, int *y, char *left_not_right, char *bottom_not_top );
void delete_fix_constraint( int cell );
