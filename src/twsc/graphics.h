#ifndef H_GRAPHIC
#define H_GRAPHIC
void expand_heat_index();
void erase_a_cell( int cell, int x, int y );
void graphics_cell_update( int cell );
void graphics_cell_attempt( int cell );
void reset_heat_index();
void graphics_dump();
void closegraphics();
void init_heat_index();
void process_graphics();
void check_graphics( BOOL drawFlag );
void set_update( BOOL flag );
#endif
