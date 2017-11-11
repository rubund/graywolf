#ifndef H_DRAW
#define H_DRAW
void process_graphics();
void draw_tile();
void draw_macro();
void draw_fs();
void last_chance();
void no_move_message();
void save_for_do();
void update_macro();
void graphics_dump();
int pick_macro();
TILE_BOX *pick_tile();
ROW_BOX *pick_row();
BOOL edit_tiles();
void edit_macro();
void update_vertices();
void rotate_vertices();
void find_nearest_corner(int macro, int x, int y, int* x_ret, int* y_ret);
void highlight_corner();
int outm(int errtype, char *routine, char *string );
void edit_row(ROW_BOX *rowptr);
void get_global_pos( int macro, int *l, int *b, int *r, int *t );
void draw_the_data();
#endif
