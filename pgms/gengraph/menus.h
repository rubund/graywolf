
/* TWmenu definitions */  
#define TWNUMMENUS		24
#define AUTO_REDRAW_ON		1
#define AUTO_REDRAW_OFF		2
#define CLOSE_GRAPHICS		3
#define COLORS		4
#define CONTINUE_PROGRAM		5
#define DUMP_GRAPHICS		6
#define FULLVIEW		8
#define REDRAW		9
#define TELL_POINT		10
#define TRANSLATE		11
#define ZOOM		12
#define CANCEL		0
#define DRAW_CELL_EDGES		20
#define IGNORE_CELL_EDGES		21
#define DRAW_CHANGRAPH		22
#define IGNORE_CHANGRAPH		23
#define DRAW_LABELS		24
#define IGNORE_LABELS		25
#define DRAW_EDGE_LABELS		36
#define IGNORE_EDGE_LABELS		37
#define DRAW_RTILE_LABELS		38
#define IGNORE_RTILE_LABELS		39
#define DRAW_WHITE_SPACE		26
#define IGNORE_WSPACE		27
#define DRAW_X_POLAR_GRAPH		28
#define IGNORE_X_POLAR		29
#define DRAW_Y_POLAR_GRAPH		30
#define IGNORE_Y_POLAR		31
#define DRAW_X_GRAPH		32
#define IGNORE_X_GRAPH		33
#define DRAW_Y_GRAPH		34
#define IGNORE_Y_GRAPH		35
#define CANCEL		0


static TWMENUBOX menuS[25] = {
    "CONTROL",0,0,1,0,0,
    "Auto Redraw On","Auto Redraw Off",1,0,1,2,
    "Close Graphics",0,0,0,3,0,
    "Colors",0,0,0,4,0,
    "Continue Program",0,0,0,5,0,
    "Dump Graphics",0,0,0,6,0,
    "FullView",0,0,0,8,0,
    "Redraw",0,0,0,9,0,
    "Tell Point",0,0,0,10,0,
    "Translate",0,0,0,11,0,
    "Zoom",0,0,0,12,0,
    "Cancel",0,0,0,0,0,
    "DRAW ",0,0,1,0,0,
    "Draw cell edges","Ignore cell edges",0,0,20,21,
    "Draw changraph","Ignore changraph",1,0,22,23,
    "Draw Labels","Ignore Labels",0,0,24,25,
    "Draw Edge Labels","Ignore Edge Labels",0,0,36,37,
    "Draw Rtile Labels","Ignore Rtile Labels",0,0,38,39,
    "Draw white space","Ignore wspace",0,0,26,27,
    "Draw X Polar Graph","Ignore X Polar",0,0,28,29,
    "Draw Y Polar Graph","Ignore Y Polar",0,0,30,31,
    "Draw X Graph","Ignore X Graph",0,0,32,33,
    "Draw Y Graph","Ignore Y Graph",0,0,34,35,
    "Cancel",0,0,0,0,0,
    0,0,0,0,0,0
} ;

