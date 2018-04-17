/* ----------------------------------------------------------------- 
"@(#) string.h version 1.7 3/6/92"
FILE:	    string.h                                       
DESCRIPTION:Insert file for string library routines.
DATE:	    Mar 16, 1989 
REVISIONS:  Tue Oct 23 01:29:24 EDT 1990 - added string prototypes.
----------------------------------------------------------------- */
#ifndef YSTRING_H
#define YSTRING_H

#include <yalecad/base.h>

extern char *Ystrclone( P1(char *str) ) ;
/* 
Function:
    Clone a string by allocating memory.  User must free memory when done.
*/

extern char **Ystrparser( P3(char *str, char *delimiters,INT *numtokens ) ) ;
/*
Function:
    This string parsing function uses strtok to break up the string
    into tokens delimited by the set of characters given in the delimiter
    string.  Numtokens is set to the number of parsed tokens.  The function
    returns a pointer to an array of all the tokens each of which has
    been terminated with EOS.  This function destroys the given string
    so remember to copy the string if you need the original string for
    future use.
*/

extern char *Yremove_lblanks( P1(char *bufferptr) ) ;
/* 
Function:
    Remove leading blanks and tabs from a string.
*/

/* 
Function:
    Very often used string functions that are defined by system.
    See man pages for details.
*/
#ifndef __GNUC__
extern char *strcat( P2( char *str1, char *str2 ) ) ; 
extern char *strncat( P3( char *str1, char *str2, INT n ) ) ; 
extern char *strcpy( P2( char *str1, char *str2 ) ) ; 
extern char *strncpy( P3( char *str1, char *str2, INT n) ) ; 
extern char *strtok( P2( char *str, char *delimiters ) ) ; 
extern char *strchr(  P2( char *str, char c ) ) ; 
extern char *strrchr( P2( char *str, char c ) ) ; 
extern char *index( P2( char *str, char c ) ) ; 
extern char *rindex( P2( char *str, char c ) ) ; 
extern unsigned int  strlen( P1( char *str) ) ; 
extern INT  strcmp( P2( char *str1, char *str2 ) ) ; 
extern INT  strncmp( P3( char *str1, char *str2, INT n ) ) ; 
#endif  /* __GNUCC__ */

#endif  /* YSTRING_H */
