#include "pads.h"
void find_optimum_locations( void ) ;
void place_pad( PADBOXPTR pad, int bestside ) ;
void place_children( PADBOXPTR pad, int side, double lb, double ub, BOOL sr);
int find_cost_for_a_side(PADBOXPTR pad, int side, double lb, double ub, BOOL spacing_restricted) ;
void find_core(void) ;
void setVirtualCore( BOOL flag );
void placepads();
void placepads_retain_side( BOOL flag );
