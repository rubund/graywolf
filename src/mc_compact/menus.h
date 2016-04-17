
/* TWmenu definitions */  
#define TWNUMMENUS		26
#define AUTO_REDRAW_ON		1
#define AUTO_REDRAW_OFF		2
#define CLOSE_GRAPHICS		3
#define COLORS		4
#define CONTINUE_PROG		5
#define DUMP_GRAPHICS		6
#define FULLVIEW		7
#define REDRAW		8
#define TRANSLATE		9
#define ZOOM		10
#define CANCEL		0
#define DRAW_EDGES		11
#define IGNORE_EDGES		12
#define DRAW_EDGE_LABELS		13
#define IGNORE_EDGE_LABELS		14
#define DRAW_FORWARD		15
#define DRAW_BACKWARD		16
#define DRAW_LABELS		17
#define IGNORE_LABELS		18
#define SET__3D_CAMERA		24
#define NORMAL_VIEW		25
#define CANCEL		0
#define DUMP_STATE		19
#define HIGHLITE_CELL		20
#define TELL_POINT		21
#define DRAW_MOVED_TILES		22
#define IGNORE_MOVED_TILES		23
#define FIND_EDGE		26
#define CANCEL		0


static TWMENUBOX menuS[27] = {
    "CONTROL",0,0,1,0,0,
    "Auto Redraw On","Auto Redraw Off",1,0,1,2,
    "Close Graphics",0,0,0,3,0,
    "Colors",0,0,0,4,0,
    "Continue Prog.",0,0,0,5,0,
    "Dump Graphics",0,0,0,6,0,
    "FullView",0,0,0,7,0,
    "Redraw",0,0,0,8,0,
    "Translate",0,0,0,9,0,
    "Zoom",0,0,0,10,0,
    "Cancel",0,0,0,0,0,
    "DRAW ",0,0,1,0,0,
    "Draw Edges","Ignore Edges",0,0,11,12,
    "Draw Edge Labels","Ignore Edge Labels",0,0,13,14,
    "Draw Forward","Draw Backward",1,0,15,16,
    "Draw Labels","Ignore Labels",0,0,17,18,
    "Set  3D Camera",0,0,0,24,0,
    "Normal View",0,0,0,25,0,
    "Cancel",0,0,0,0,0,
    "DEBUG ",0,0,1,0,0,
    "Dump State",0,0,0,19,0,
    "HighLite Cell",0,0,0,20,0,
    "Tell Point",0,0,0,21,0,
    "Draw Moved Tiles","Ignore Moved Tiles",1,0,22,23,
    "Find Edge",0,0,0,26,0,
    "Cancel",0,0,0,0,0,
    0,0,0,0,0,0
} ;

