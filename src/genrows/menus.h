
/* TWmenu definitions */  
#define TWNUMMENUS		52
#define AUTO_REDRAW_ON		1
#define AUTO_REDRAW_OFF		2
#define CLOSE_GRAPHICS		3
#define COLORS		4
#define CONTINUE_PGM		5
#define DUMP_GRAPHICS		6
#define FULLVIEW		7
#define REDRAW		8
#define TELL_POINT		9
#define TRANSLATE		10
#define ZOOM		11
#define CANCEL		0
#define ALIGN_MACRO_IN_X		12
#define ALIGN_MACRO_IN_Y		13
#define ALIGN_ROWS		14
#define EDIT_MACRO		16
#define EDIT_ROW		15
#define EDIT_TILE		17
#define KEEP_SHORT_ROW		18
#define DISCARD_SHORT_ROW		19
#define MEMORY_ON		51
#define MEMORY_OFF		52
#define MODIFY_CORE_AREA		20
#define MOVE_MACRO		21
#define NUMROWS		22
#define REDO		23
#define RESTORE_STATE		24
#define SAVE_STATE		25
#define UNDO		27
#define CANCEL		0
#define DIVIDE_TILE_LEFT_RIGHT		28
#define DIVIDE_TILE_UP_DOWN		29
#define LIMIT_MERGES		30
#define UNLIMIT_MERGES		31
#define MERGE_DOWNWARD		32
#define MERGE_LEFT		33
#define MERGE_RIGHT		34
#define MERGE_UPWARD		35
#define RESET_TILES		36
#define CANCEL		0
#define DRAW_LABELS		37
#define IGNORE_LABELS		38
#define DRAW_MACROS		39
#define IGNORE_MACROS		40
#define DRAW_ORIENT		49
#define IGNORE_ORIENT		50
#define DRAW_ROWS		41
#define IGNORE_ROWS		42
#define DRAW_TILES		43
#define IGNORE_TILES		44
#define CANCEL		0
#define FEED_PERCENTAGE		45
#define MIN_ROW_LENGTH		46
#define ROW_SEPARATION		47
#define SET_SPACING		48
#define CANCEL		0


static TWMENUBOX menuS[53] = {
    "CONTROL",0,0,1,0,0,
    "Auto Redraw On","Auto Redraw Off",1,0,1,2,
    "Close Graphics",0,0,0,3,0,
    "Colors",0,0,0,4,0,
    "Continue Pgm",0,0,0,5,0,
    "Dump Graphics",0,0,0,6,0,
    "FullView",0,0,0,7,0,
    "Redraw",0,0,0,8,0,
    "Tell Point",0,0,0,9,0,
    "Translate",0,0,0,10,0,
    "Zoom",0,0,0,11,0,
    "Cancel",0,0,0,0,0,
    "EDIT",0,0,1,0,0,
    "Align Macro in X",0,0,0,12,0,
    "Align Macro in Y",0,0,0,13,0,
    "Align Rows",0,0,0,14,0,
    "Edit Macro",0,0,0,16,0,
    "Edit Row",0,0,0,15,0,
    "Edit Tile",0,0,0,17,0,
    "Keep Short Row","Discard Short Row",0,0,18,19,
    "Memory On","Memory Off",1,0,51,52,
    "Modify Core Area",0,0,0,20,0,
    "Move Macro",0,0,0,21,0,
    "Numrows",0,0,0,22,0,
    "Redo",0,0,0,23,0,
    "Restore State",0,0,0,24,0,
    "Save State",0,0,0,25,0,
    "Undo",0,0,0,27,0,
    "Cancel",0,0,0,0,0,
    "MERGE",0,0,1,0,0,
    "Divide Tile Left_Right",0,0,0,28,0,
    "Divide Tile Up_Down",0,0,0,29,0,
    "Limit Merges","Unlimit Merges",0,0,30,31,
    "Merge Downward",0,0,0,32,0,
    "Merge Left",0,0,0,33,0,
    "Merge Right",0,0,0,34,0,
    "Merge Upward",0,0,0,35,0,
    "Reset Tiles",0,0,0,36,0,
    "Cancel",0,0,0,0,0,
    "DRAW ",0,0,1,0,0,
    "Draw Labels","Ignore Labels",0,0,37,38,
    "Draw Macros","Ignore Macros",1,0,39,40,
    "Draw Orient","Ignore Orient",0,0,49,50,
    "Draw Rows","Ignore Rows",1,0,41,42,
    "Draw Tiles","Ignore Tiles",1,0,43,44,
    "Cancel",0,0,0,0,0,
    "PARAMETERS ",0,0,1,0,0,
    "Feed Percentage",0,0,0,45,0,
    "Min. Row Length",0,0,0,46,0,
    "Row Separation",0,0,0,47,0,
    "Set Spacing",0,0,0,48,0,
    "Cancel",0,0,0,0,0,
    0,0,0,0,0,0
} ;

