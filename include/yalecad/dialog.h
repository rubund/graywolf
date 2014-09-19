/* ----------------------------------------------------------------- 
FILE:	    dialog.h                                       
DESCRIPTION:user insert file for dialog box routines.
CONTENTS:   typedefs and definitions
DATE:	    Sep 16, 1989 - original coding.
REVISIONS:  Dec  7, 1990 - added update function to dialog box.
----------------------------------------------------------------- */
#ifndef DIALOG_H

#define DIALOG_H

#ifndef lint
static char dialogHId[] = "@(#) dialog.h version 1.4 12/7/90" ;
#endif

#include <yalecad/base.h>

#define LABELTYPE   0
#define INPUTTYPE   1
#define BUTTONTYPE  2
#define CASETYPE    3

typedef struct {
    INT   row,  column,  len ;
    char *string ;
    INT   type ;
    INT   color ;
    INT   group ; /* used for CASETYPE - initial switch */
	  	  /* used for BUTTONTYPE - member of case group */
} TWDIALOGBOX, *TWDIALOGPTR ;

typedef struct {
    INT   type ;
    char  *string ;
    BOOL  bool ;
} TWDRETURNBOX, *TWDRETURNPTR ;

/* build a dialog box and get info */
extern TWDRETURNPTR TWdialog( P3(TWDIALOGPTR fieldp, char *name,
    INT (*user_function)() ) ) ;
/* 
Arguments:
    TWDIALOGPTR fieldp ;
    char *name ;
    INT (*user_function)() ;
Function:
    Creates a dialog box for user to enter data.  Return a pointer
    to an array of type TWDRETURNPTR of the same size as input argument.
    Name is the name given to the dialogbox for .Xdefault
    control over the size of the dialog box upon creation.
    The easier way to use this routine is to use the TWread_dialog
    routine below to create a dialog.h file which has the necessary
    input to create the correct input field array.  See a sample
    dialog file in graphics/example/example.dialog for more details.
    User function allows an incremental calculation update.
*/

#ifdef DEBUG
extern TWDIALOGPTR TWread_dialog( P1(char *filename) ) ;
/* 
Arguments:
    char *filename ;
Function:
    Reads file and builds correct data structure, output a dialog.h
    file for inclusion in user program and then calls TWdialog.
    This allows fast development of dialog boxes for the codes yet
    the final result is stored in the program itself.
*/
#endif

#endif /* DIALOG_H */
