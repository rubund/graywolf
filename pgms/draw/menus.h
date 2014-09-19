
/* TWmenu definitions */  
#define TWNUMMENUS		24
#define AUTO_REDRAW_ON		6
#define AUTO_REDRAW_OFF		7
#define COLORS		5
#define FULLSCREEN		4
#define QUIT		8
#define REDRAW		1
#define TELL_POINT		9
#define TRANSLATE		3
#define ZOOM		2
#define CANCEL		0
#define BACKWARD_FRAME		21
#define DELETE_FRAME		23
#define FORWARD_FRAME		20
#define JUMP_TO_FRAME		22
#define RENUMBER_FRAMES		26
#define REREAD_INPUT		24
#define WRITE_CIF_FILE		25
#define CANCEL		0
#define DRAW_LABELS		45
#define IGNORE_LABELS		46
#define DRAW_MST		47
#define IGNORE_MST		48
#define DRAW_NETS		43
#define IGNORE_NETS		44
#define CANCEL		0


static TWMENUBOX menuS[25] = {
    "SCREEN",0,0,1,0,0,
    "Auto Redraw On","Auto Redraw Off",1,0,6,7,
    "Colors",0,0,0,5,0,
    "FullScreen",0,0,0,4,0,
    "Quit",0,0,0,8,0,
    "Redraw",0,0,0,1,0,
    "Tell Point",0,0,0,9,0,
    "Translate",0,0,0,3,0,
    "Zoom",0,0,0,2,0,
    "Cancel",0,0,0,0,0,
    "FRAME",0,0,1,0,0,
    "Backward Frame",0,0,0,21,0,
    "Delete Frame",0,0,0,23,0,
    "Forward Frame",0,0,0,20,0,
    "Jump to Frame",0,0,0,22,0,
    "Renumber Frames",0,0,0,26,0,
    "Reread input",0,0,0,24,0,
    "Write CIF File",0,0,0,25,0,
    "Cancel",0,0,0,0,0,
    "DATA",0,0,1,0,0,
    "Draw Labels","Ignore Labels",0,0,45,46,
    "Draw MST","Ignore MST",0,0,47,48,
    "Draw Nets","Ignore Nets",1,0,43,44,
    "Cancel",0,0,0,0,0,
    0,0,0,0,0,0
} ;

