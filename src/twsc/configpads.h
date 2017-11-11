#ifndef H_CONFIG_PADS
#define H_CONFIG_PADS
BOOL check_overflow( P1(BOOL retain_pad_groups) ) ;
void move_clockwise( P4(INT pad, INT side, INT cw_side, INT moveable_cw) ) ;
void move_counterclockwise( P4(INT pad,INT side,INT ccw_side,INT m_ccw ) ) ;
void update_pad_position( P3(PADBOXPTR pad,INT current_side, INT move_side) );
void expand_core( P1(INT side) ) ;
int compare_overflow( P2(INT *side1, INT *side2) ) ;
void update_pad_groups( P1(void) ) ;
void resort_place_array( P1(void) ) ;
void child_constraints(P5(PADBOXPTR pad,INT side,DOUBLE lb,DOUBLE ub,BOOL s));
void place_variable( P3(INT first,INT numpads,INT side) ) ;
void calc_constraints( PADBOXPTR pad, int side, double *lb, double *ub, BOOL *spacing_restricted, int *lowpos, int *uppos );
#endif
