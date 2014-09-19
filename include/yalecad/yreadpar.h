/* ----------------------------------------------------------------- 
FILE:	    readpar.h                                       
DESCRIPTION:The include file for reading TimberWolf parameter files.
CONTENTS:   
DATE:	    Oct 25, 1990 
REVISIONS:  Sat Jan 26 12:36:04 PST 1991 - added genrows GENR to 
		list of programs.
	    Sat Feb 23 00:46:29 EST 1991 - added TOMUS program and
		added wildcarding argument to Yreadpar_next.
	    Thu Apr 18 00:53:01 EDT 1991 - added design rule
		parameter extraction routines.
----------------------------------------------------------------- */
#ifndef YREADPAR_H
#define YREADPAR_H

#ifndef lint
static char YreadParId[] = "@(#) yreadpar.h version 1.4 4/18/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/rbtree.h>

/* The following are valid programs */
#define ALLP	-1
#define UNKN	0
#define CMPT	1
#define GENR	2
#define MICK	3
#define MINC	4
#define PART	5
#define SGGR	6
#define TWAR	7
#define TWMC	8
#define TWSC	9
#define USER	10
#define MAXID   USER    /* set this to the last program */

typedef YTREEPTR YPARPTR ;
typedef YTREEBOX YPARBOX ;

extern VOID Yreadpar_init( P4(char *design_name, INT  program,
				INT  filter, BOOL abortFlag ) ) ;
/* ******************* READPAR ROUTINES ****************** */
/* 
Function:
    Initializes the readpar mechanism.  This must be call before any
    other readpar functions.  Restrictions: only one parameter file
    may be read at a time.  Design_name is the name of the design. 
    Program is one of the listed program id's at the top of this 
    file.  The filter is used to screen out other programs. There
    are two ways of initializing the parameter files.  In the
    first case program = USER, and filter is one of the programs
    such as SGGR.  The second way is to specify program =
    filter = program such as SGGR.  If abort flag is set true, the
    routine will exit program if file is not found.  If abort flag is
    set false, it will ignore any subsequent readpar commands 
    relating to this file.
*/

extern char **Yreadpar_next( P5(char **lineptr,INT *line,
    INT *numtokens,BOOL *onNotOff,BOOL *wildcard)) ;
/* 
Function:
    Reads a parameter one line at a time ignoring comments. Returns
    only parameters which pertain to the given program.  Nothing
    is stored in memory.  Lineptr is the contents of the current
    line, line is the current line number. Numtokens is the 
    number of tokens returned and onNotOff tells the state of
    the variable.   Wildcard returns whether this line is
    a wildcard.  Wildcard should not generate error messages 
    in the programs readpar file.
*/

extern YPARPTR Yreadpar_file();
/* 
Function:
    Reads the an entire parameter file and stores the data in memory 
    to be accessed randomly using Yreadpar_lookup.  Returns a
    par_object to be used with Yreadpar_lookup.
*/

extern char **Yreadpar_lookup( P4(YPARPTR par_object, char *param,
			    INT program, INT *numtokens )) ;
/* 
Function:
    Given a par object, and a program id, look up the given parameter.
    If the parameter is found it will return a token buffer similar
    to argv[]. In addition, numtokens will be set to the number
    of tokens in the argument vertor.  If no match is found for
    the parameter, a NULL ptr is returned.
*/


/* The routines below get spacing, width, pitch, etc. for the design */
extern DOUBLE Yreadpar_spacing( P2(char *object1, char *object2 ) ) ;

extern DOUBLE Yreadpar_width( P1(char *object) )  ;

extern DOUBLE Yreadpar_pitch( P1(char *object) )  ;

extern DOUBLE Yreadpar_layer_res( P1(char *object) ) ;

extern DOUBLE Yreadpar_layer_cap( P1(char *object) ) ;

extern BOOL Yreadpar_layer_HnotV( P1(char *object) ) ;

extern INT Yreadpar_layer2id( P1(char *object) ) ;

extern char *Yreadpar_id2layer( P1(INT layerid) ) ;

extern INT Yreadpar_numlayers() ;

extern char *Yreadpar_vianame( P2(char *object1, char *object2) ) ;

extern char *Yreadpar_viaId2name( P1(INT viaid) ) ;

#endif /* YREADPAR_H */
