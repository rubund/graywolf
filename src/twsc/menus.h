
/* TWmenu definitions */  
#define TWNUMMENUS		23
#define CANCEL			0
#define AUTO_REDRAW_ON		1
#define AUTO_REDRAW_OFF		2
#define CLOSE_GRAPHICS		3
#define COLORS			4
#define CONTINUE_PGM		5
#define DUMP_GRAPHICS		6
#define FULLVIEW		7
#define GRAPHICS_UPDATE_ON	8
#define GRAPHICS_UPDATE_OFF	9
#define REDRAW			10
#define TELL_POINT		11
#define TRANSLATE		12
#define ZOOM			13
#define DRAW_BLOCKS		20
#define IGNORE_BLOCKS		21
#define DRAW_STDCELLS		22
#define IGNORE_STDCELLS		23
#define DRAW_LABELS		24
#define IGNORE_LABELS		25
#define DRAW_NETS		26
#define IGNORE_NETS		27
#define DRAW_ORIENT		33
#define IGNORE_ORIENT		34
#define DRAW_PINS		28
#define IGNORE_PINS		29
#define DRAW_SINGLE_NET		30
#define DRAW_SINGLE_CELL_MOVES	31
#define IGNORE_SINGLE_MOVES	32

#ifndef DEVELOPMENU

static TWMENUBOX menuS[24] = {
    "CONTROL",0,0,1,0,0,
    "Auto Redraw On","Auto Redraw Off",1,0,1,2,
    "Close Graphics",0,0,0,3,0,
    "Colors",0,0,0,4,0,
    "Continue Pgm",0,0,0,5,0,
    "Dump Graphics",0,0,0,6,0,
    "FullView",0,0,0,7,0,
    "Graphics Update On","Graphics Update Off",1,0,8,9,
    "Redraw",0,0,0,10,0,
    "Tell Point",0,0,0,11,0,
    "Translate",0,0,0,12,0,
    "Zoom",0,0,0,13,0,
    "Cancel",0,0,0,0,0,
    "DRAW ",0,0,1,0,0,
    "Draw Blocks","Ignore Blocks",1,0,20,21,
    "Draw Stdcells","Ignore Stdcells",1,0,22,23,
    "Draw Labels","Ignore Labels",0,0,24,25,
    "Draw Nets","Ignore Nets",0,0,26,27,
    "Draw Orient","Ignore Orient",0,0,33,34,
    "Draw Pins","Ignore Pins",0,0,28,29,
    "Draw Single Net",0,0,0,30,0,
    "Draw Single Cell Moves","Ignore Single Moves",0,0,31,32,
    "Cancel",0,0,0,0,0,
    0,0,0,0,0,0
} ;

#else  /* DEVELOPMENU */

static TWMENUBOX *menuS ;

#endif  /* DEVELOPMENU */
