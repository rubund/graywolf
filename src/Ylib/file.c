/*
 *   Copyright (C) 1988-1991 Yale University
 *
 *   This work is distributed in the hope that it will be useful; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation;
 *   either version 2 of the License,
 *   or any later version, on the following conditions:
 *
 *   (a) YALE MAKES NO, AND EXPRESSLY DISCLAIMS
 *   ALL, REPRESENTATIONS OR WARRANTIES THAT THE MANUFACTURE, USE, PRACTICE,
 *   SALE OR
 *   OTHER DISPOSAL OF THE SOFTWARE DOES NOT OR WILL NOT INFRINGE UPON ANY
 *   PATENT OR
 *   OTHER RIGHTS NOT VESTED IN YALE.
 *
 *   (b) YALE MAKES NO, AND EXPRESSLY DISCLAIMS ALL, REPRESENTATIONS AND
 *   WARRANTIES
 *   WHATSOEVER WITH RESPECT TO THE SOFTWARE, EITHER EXPRESS OR IMPLIED,
 *   INCLUDING,
 *   BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *   PARTICULAR
 *   PURPOSE.
 *
 *   (c) LICENSEE SHALL MAKE NO STATEMENTS, REPRESENTATION OR WARRANTIES
 *   WHATSOEVER TO
 *   ANY THIRD PARTIES THAT ARE INCONSISTENT WITH THE DISCLAIMERS BY YALE IN
 *   ARTICLE
 *   (a) AND (b) above.
 *
 *   (d) IN NO EVENT SHALL YALE, OR ITS TRUSTEES, DIRECTORS, OFFICERS,
 *   EMPLOYEES AND
 *   AFFILIATES BE LIABLE FOR DAMAGES OF ANY KIND, INCLUDING ECONOMIC DAMAGE OR
 *   INJURY TO PROPERTY AND LOST PROFITS, REGARDLESS OF WHETHER YALE SHALL BE
 *   ADVISED, SHALL HAVE OTHER REASON TO KNOW, OR IN FACT SHALL KNOW OF THE
 *   POSSIBILITY OF THE FOREGOING.
 *
 */

/* ----------------------------------------------------------------- 
FILE:	    file.c                                       
DESCRIPTION:miscellaneous file utility functions 
CONTENTS:   FILE *YopenFile(char *,char *,int)
	    BOOL YfileExists(char *)
	    BOOL YdirectoryExists(pathname)
		char *pathname ;
DATE:	    Jan 29, 1988 
REVISIONS:  May 04, 1988 - updated initProgram to include 
			introTextFunction. Added remove_lblanks,
			directoryExists functions.
	    Sep 26, 1988 - split utils.c into three pieces for archive.
	    Jan 26, 1989 - changed directoryExists from macro to routine.
	    Apr 27, 1989 - changed to Y prefix.
	    Aug 28, 1989 - changed YfileExists to use stat functions
		instead of kludge fopen call.
	    Fri Feb 22 23:38:59 EST 1991 - added locking functions.
	    Tue Mar 12 16:57:30 CST 1991 - modified lock function
		to work on apollo - now a level 2 function.
	    Thu Apr 18 00:37:52 EDT 1991 - fixed file descriptor.
	    Sun Apr 21 21:21:21 EDT 1991 - added Yfile_slink.
	    Sat May 11 22:53:09 EDT 1991 - added a conditional compile
		for HPUX.
	    Oct 07, 1991 - fix #include sys/dir.h for SYS5 A/UX (RAWeier)
            Oct 18, 1991 - change INT to BOOL in YopenFile (RAWeier)
----------------------------------------------------------------- */
#define SERROR  0

#include <globals.h>

/* check if a file exists */
BOOL YfileExists(char *pathname)
{
	int ret = 1;
	if( access( pathname, F_OK ) == -1 ) {
		ret = 0;
	}
	return ret;
}

char *Yfile_slink( char *pathname )
{
	static char buf[BUFSIZ] ;

	int len = readlink( pathname, buf, BUFSIZ ) ;
	if( len <= SERROR ){
		sprintf( YmsgG, "ERROR[Yfile_slink]:%s", pathname ) ;
		perror( YmsgG ) ;
		Ymessage_error_count() ;
		return(NIL(char *)) ;
	} else {
		/* null terminate string */
		buf[len] = EOS ;
		return( buf ) ;
	}

} /* end Yfile_slink */


/* check if a directory exists */
BOOL YdirectoryExists(char *pathname)
{
	DIR *dp ;

	if( pathname ){
		if( dp = opendir(pathname) ){
		closedir(dp) ;
		return(TRUE) ;
		}
	}
	return(FALSE) ;
}

FILE *Yfile_create_lock( char *filename, BOOL readNotWrite ) 
{
	int fd ;             /* file descriptor */
	int status ;         /* return status */
	FILE *fp ;           /* file stream descriptor */

	if( readNotWrite ){
		fd = creat( filename, O_RDONLY ) ;
	} else {
		fd = creat( filename, O_WRONLY ) ;
	}
	if( fd <= 0 ){
		perror( "Yfile_create_lock" ) ;
		sprintf( YmsgG,"could not open file %s\n",filename ) ;
		M(ERRMSG,"Yfile_create_lock",YmsgG ) ;
		YexitPgm(PGMFAIL) ;
	}
	status = flock( fd, LOCK_EX | LOCK_NB ) ;
	if( status != 0 ){
		sprintf( YmsgG,"could not lock file %s\n",filename ) ;
		M(ERRMSG,"Yfile_create_lock",YmsgG ) ;
		YexitPgm(PGMFAIL) ;
	}
	/* now if we get this far find file descriptor */
	if( readNotWrite ){
		fp = fdopen( fd, "r" ) ;
	} else {
		fp = fdopen( fd, "w" ) ;
	}
	if(!(fp)){
		perror( "Yfile_create_lock" ) ;
		sprintf( YmsgG,"could not get file descriptor %s\n",filename ) ;
		M(ERRMSG,"Yfile_create_lock",YmsgG ) ;
		YexitPgm(PGMFAIL) ;
	}
	return( fp ) ;
} /* end Yfile_create_lock */

/* see a file is locked */
BOOL Yfile_test_lock( char *filename ) 
{
	int fd ;             /* file descriptor */
	int status ;         /* return status */

	if(!(YfileExists(filename))){ 
		/* file does not exist */
		return( FALSE ) ;
	}

	fd = open( filename, O_RDONLY, 0 ) ;
	if( fd <= 0 ){
		sprintf( YmsgG,"could not open file %s\n",filename ) ;
		M(ERRMSG,"Yfile_test_lock",YmsgG ) ;
		YexitPgm(PGMFAIL) ;
	}
	status = flock( fd, LOCK_EX | LOCK_NB ) ;
	if( status != 0 ){
		return( TRUE ) ;
	} else {
		return( FALSE ) ;
	}
} /* end Yfile_test_lock */

/* --------------end file routines ------------------------------- */
