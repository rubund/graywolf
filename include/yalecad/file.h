/* ----------------------------------------------------------------- 
"@(#) file.h version 1.7 4/21/91"
FILE:	    file.h - file utility routine insert file 
DESCRIPTION:The file utilities open and close files, check to 
	    see if files and directories exist.
NOTE: The define statements are designed so that external users
may customize their I/O routines.
CONTENTS:   Definitions for file routines.
DATE:	    Apr 10, 1989 
REVISIONS:  Thu Apr 18 00:51:06 EDT 1991 - added lock routines.
	    Sun Apr 21 21:22:40 EDT 1991 - added Yfile_slink.
----------------------------------------------------------------- */
#ifndef FILE_H
#define FILE_H

#include <yalecad/base.h>

extern FILE *YopenFile( P3(char *filename,char *readwrite, BOOL abort ) ) ;
/*
Function:
    Open the given file name for reading and/or writing.  Readwrite is
    the standard C lib control string. It may be "w", "r", "a", "w+",
    etc.  If abort is set to TRUE, the program will exit if the file
    cannot be found and an appropriate error message will be output.
    If abort is set to FALSE, the routine will return a NULL pointer
    if the file could not be opened.
*/

extern BOOL YfileExists( P1(char *pathname ) ) ;
/*
Function:
    Returns TRUE if the given file exists.  It returns FALSE otherwise.
*/

extern BOOL YdirectoryExists( P1(char *pathname) ) ;
/*
Function:
    Returns TRUE if the given pathname is a directory and it exists.
    It returns FALSE otherwise.
*/

extern FILE *Yfile_create_lock( P2(char *filename, BOOL readNotWrite ) ) ;
/*
Function:
    Creates a lock file ready for read or write 
*/

extern BOOL Yfile_test_lock( P1( char *filename ) ) ; 
/*
Function:
    See if a file is locked.
*/

extern char *Yfile_slink( P1( char *pathname ) ) ; 
/*
Function:
    Return symbolic link of a file.
*/

#define TWOPEN( a_z, b_z, c_z )  YopenFile( a_z, b_z, c_z )
#define TWCLOSE( a_z )           fclose( a_z )

/* I/O MACRO DEFINITIONS */
#define ABORT   1
#define NOABORT 0 

#endif /* FILE_H */
