#ifndef H_MERGE
#define H_MERGE
void check_max_length();
void merge_adjacent_tiles();
void merge_tiles();
void merge_upward( TILE_BOX *begin_tile );
void merge_left( TILE_BOX *begin_tile );
void merge_right( TILE_BOX *begin_tile );
void merge_downward( TILE_BOX *begin_tile );
void merge_tiles();
void renumber_tiles();
#endif
