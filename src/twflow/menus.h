
/* TWmenu definitions */  
#define TWNUMMENUS		19
#define AUTO_REDRAW_ON		1
#define AUTO_REDRAW_OFF		2
#define CLOSE_GRAPHICS		3
#define COLORS		4
#define CONTINUE_PGM		5
#define DUMP_GRAPHICS		6
#define EXIT_PROGRAM		7
#define FULLVIEW		8
#define REDRAW		9
#define TELL_POINT		10
#define TRANSLATE		11
#define ZOOM		12
#define CANCEL		0
#define AUTOFLOW		13
#define EXECUTE_PGM		14
#define PICK_PGM		15
#define PROMPT_ON		16
#define PROMPT_OFF		17
#define CANCEL		0


static TWMENUBOX menuS[20] = {
    "CONTROL",0,0,1,0,0,
    "Auto Redraw On","Auto Redraw Off",1,0,1,2,
    "Close Graphics",0,0,0,3,0,
    "Colors",0,0,0,4,0,
    "Continue Pgm",0,0,0,5,0,
    "Dump Graphics",0,0,0,6,0,
    "Exit Program",0,0,0,7,0,
    "FullView",0,0,0,8,0,
    "Redraw",0,0,0,9,0,
    "Tell Point",0,0,0,10,0,
    "Translate",0,0,0,11,0,
    "Zoom",0,0,0,12,0,
    "Cancel",0,0,0,0,0,
    "FLOW ",0,0,1,0,0,
    "AutoFlow",0,0,0,13,0,
    "Execute Pgm",0,0,0,14,0,
    "Pick Pgm",0,0,0,15,0,
    "Prompt On","Prompt Off",0,0,16,17,
    "Cancel",0,0,0,0,0,
    0,0,0,0,0,0
} ;

