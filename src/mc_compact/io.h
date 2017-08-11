#ifndef IO_H
#define IO_H
void init( int numtiles, int numcells );
void initCell( int celltype, int cellnum, int x, int y, int xoffset, int yoffset );
void endCell();
int addtile( int l, int r, int b, int t );
void output();
void final_tiles();
void init_extra_tile( int cell, int type );
#endif
