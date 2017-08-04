#include <globals.h>

typedef struct {
    INT tile ;    /* tile that point is attached */
    INT x ;       /* x position of point */
    INT y ;       /* y position of point */
    INT Vnum ;    /* position in VPts array */
    INT Hnum ;    /* position in HPts array */
    INT order ;   /* order that points should be output - negative means invalid */
    BOOL marked ; /* point has been used */
} POINTBOX, *POINTPTR ;

void add_vpts();
void chek_vpt();
void add_hpts();
void chek_hpt();
void dump_pts(  POINTPTR *pt );
void addPt( int tile, int x, int y );
void addPts( int cell, int l, int r, int b, int t );
void initPts( BOOL addpoint_flag );
void add_vpts( int numpts );
void chek_vpt(POINTPTR tile1, POINTPTR tile2, POINTPTR tile3, POINTPTR tile4);
void add_hpts( int numpts );
void chek_hpt(POINTPTR tile1, POINTPTR tile2, POINTPTR tile3, POINTPTR tile4);
void dump_pts(  POINTPTR *pt );
