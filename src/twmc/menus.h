
/* TWmenu definitions */  
#define TWNUMMENUS		38
#define AUTO_REDRAW_ON		1
#define AUTO_REDRAW_OFF		2
#define CLOSE_GRAPHICS		3
#define COLORS		4
#define CONTINUE_PROGRAM		5
#define DUMP_GRAPHICS		6
#define FULLVIEW		8
#define GRAPHICS_UPDATE_ON		9
#define GRAPHICS_UPDATE_OFF		10
#define REDRAW		11
#define TELL_POINT		12
#define TRANSLATE		13
#define ZOOM		14
#define CANCEL		0
#define CELL_NEIGHBORHOOD		15
#define EDIT_CELL		16
#define FIX_CELL		17
#define FIX_CELL_BUT_ROT		18
#define GROUP_CELLS		19
#define MOVE_CELL		20
#define CANCEL		0
#define DRAW_BINS		21
#define IGNORE_BINS		22
#define DRAW_BORDER		23
#define DRAW_TILES		24
#define DRAW_GLOBE_AREAS		37
#define IGNORE_GLOBE_AREAS		38
#define DRAW_LABELS		25
#define IGNORE_LABELS		26
#define DRAW_NEIGHBORHD		27
#define IGNORE_NEIGHBORHOOD		28
#define DRAW_NETS		29
#define IGNORE_NETS		30
#define DRAW_ORIENT		44
#define IGNORE_ORIENT		45
#define DRAW_PINS		31
#define IGNORE_PINS		32
#define DRAW_SINGLE_CELL_MOVES		42
#define IGNORE_SINGLE_CELL		43
#define DRAW_SINGLE_NET		33
#define DRAW_WIRING_EST		34
#define IGNORE_WIRING_EST		35
#define CANCEL		0
#define CHANGE_ASPECT_RATIO		36
#define GRAPHICS_WAIT		40
#define NO_GRAPHICS_WAIT		41
#define CANCEL		0


static TWMENUBOX menuS[39] = {
    "CONTROL",0,0,1,0,0,
    "Auto Redraw On","Auto Redraw Off",1,0,1,2,
    "Close Graphics",0,0,0,3,0,
    "Colors",0,0,0,4,0,
    "Continue Program",0,0,0,5,0,
    "Dump Graphics",0,0,0,6,0,
    "FullView",0,0,0,8,0,
    "Graphics Update On","Graphics Update Off",1,0,9,10,
    "Redraw",0,0,0,11,0,
    "Tell Point",0,0,0,12,0,
    "Translate",0,0,0,13,0,
    "Zoom",0,0,0,14,0,
    "Cancel",0,0,0,0,0,
    "EDIT",0,0,1,0,0,
    "Cell Neighborhood",0,0,0,15,0,
    "Edit Cell",0,0,0,16,0,
    "Fix Cell",0,0,0,17,0,
    "Fix Cell but Rot",0,0,0,18,0,
    "Group Cells",0,0,0,19,0,
    "Move Cell",0,0,0,20,0,
    "Cancel",0,0,0,0,0,
    "DRAW ",0,0,1,0,0,
    "Draw Bins","Ignore Bins",0,0,21,22,
    "Draw Border","Draw Tiles",1,0,23,24,
    "Draw Globe Areas","Ignore Globe Areas",1,0,37,38,
    "Draw Labels","Ignore Labels",0,0,25,26,
    "Draw Neighborhd","Ignore Neighborhood",1,0,27,28,
    "Draw Nets","Ignore Nets",0,0,29,30,
    "Draw Orient","Ignore Orient",0,0,44,45,
    "Draw Pins","Ignore Pins",0,0,31,32,
    "Draw Single Cell Moves","Ignore Single Cell",0,0,42,43,
    "Draw Single Net",0,0,0,33,0,
    "Draw Wiring Est","Ignore Wiring Est",0,0,34,35,
    "Cancel",0,0,0,0,0,
    "PARAMETERS ",0,0,1,0,0,
    "Change Aspect Ratio",0,0,0,36,0,
    "Graphics Wait","No Graphics Wait",0,0,40,41,
    "Cancel",0,0,0,0,0,
    0,0,0,0,0,0
} ;

