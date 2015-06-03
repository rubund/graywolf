/*
 *   Copyright (C) 1989-1990 Yale University
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
FILE:	    path.c                                       
DESCRIPTION:routine for correctly resolving pathnames on system
		especially ~ and ../ in names.
CONTENTS:   char *Yfixpath(pathname, fileNotDir )
		char *pathname;
		BOOL fileNotDir;
DATE:	    Jan 25, 1989 - major rewrite of version 1.0
REVISIONS:  Sep 15, 1989 - replaced which with my own version.
	    Oct 20, 1990 - pathname should be bufsize since
		users path may be very long.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) path.c version 3.8 10/23/90" ;
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <yalecad/base.h>
#include <yalecad/string.h>

#undef  LRECL
#define LRECL	BUFSIZ

static char *getpath ();

/* it is up to user to free memory */
char *Yfixpath(given_path, fileNotDir ) /* fix a path to get rid of .. */
char *given_path;
BOOL fileNotDir; /* tells whether path is file or directory */
{

    INT  i ;                  /* token counter */
    INT  numtokens ;          /* number of tokens on line */
    char pathname[LRECL] ;    /* copy of given_path */
    char cur_dir[LRECL] ;     /* current working directory */
    char temp[LRECL] ;        /* temporary buffer */
    char *usrpath ;           /* the users environment path */
    char *getenv() ;          /* get the users environment path */
    char *result ;            /* the final result */
    char *rest ;              /* the rest of the string */
    char *user ;              /* the user */
    char *Yrelpath() ;        /* get relative path */
    char **tokens ;           /* tokenizer */

    /* get the current working directory */
    getcwd( cur_dir, LRECL ) ;
    /* make a copy for strtok is destructive */
    strcpy( pathname, given_path ) ;

    switch( pathname[0] ){
    case '~':
	/* look for ~/ construct */
	if( pathname[1] == '/' ){
	    /* use login as user */
	    user = NULL ;
	    /* skip over ~/ */
	    rest = pathname + 2 ;
	} else {
	    /* get user from string */
	    user = strtok( pathname," ~//,\n" ) ;
	    /* skip over ~ */
	    rest = strtok( NULL," ~\n" ) ;
	}
	/* call get path to get user path */
	strcpy( temp, getpath(user) );
	/* copy the rest of the string to result */
	strcat( temp, "/" ) ;
	strcat( temp, rest ) ;
	result = Ystrclone( temp ) ;
	break ;
    case '.':
	result =  Yrelpath( cur_dir, pathname ) ;
	break ;
    default:
	/* no work to be done */
	result = pathname ;
	break ;
    } /* end switch */

    if( fileNotDir ){
	if( YfileExists( result ) ){
	    /* we are done */
	    return(result);
	} /* otherwise continue below */
    } else { /* a directory */
	if( YdirectoryExists( result ) ){
	    /* we are done */
	    return(result);
	} /* otherwise continue below */
    }

    /* now that that has failed try looking in user's path */
    /* take last part of file name to search */
    tokens = Ystrparser( pathname, " //,\t\n", &numtokens );

    /* temp is the name to search for in user path */
    strcpy( temp, tokens[numtokens-1] ) ;

    usrpath = getenv( "PATH" ) ;
    /* make copy for destructive Ystrparser */
    strcpy( pathname, usrpath ) ; 

    tokens = Ystrparser( pathname, ":\t\n", &numtokens );
    for( i = 0; i < numtokens; i++ ){
	/* use directory to look for file */
	strcpy( cur_dir, tokens[i] ) ;
	strcat( cur_dir, "/" ) ;
	strcat( cur_dir, temp ) ;

	/* check to see if file exist with this pathname */
	if( YfileExists( cur_dir ) ){
	    result = Ystrclone( cur_dir ) ;
	    return( result ) ;
	}
    }

    /* at this point we didn't find anything in user's path */
    /* return what we are given */
    return( given_path ) ;
}

#include <pwd.h>
struct passwd *getpwnam();
char *getlogin();

static char *getpath(user)     /* get path of home directory */
register char *user;
{
        register char *who;
        struct passwd *pass;

        if( !(user) || *(user) == '/'){
	    who = getlogin();
        } else {
	    who = user;
	}
        if (pass = getpwnam(who)){
	    return(pass->pw_dir);
        } else {
	    return(user);
	}
}

#ifdef TEST 

/* include date.o object for link */
main( argc , argv )
INT argc ;
char *argv[] ;
{
    char *Yfixpath() ; /* fix a path to get rid of .. */
    if( argc == 2 ){
	fprintf( stderr, "given file:%s\n", argv[1] ) ;
	fprintf( stderr, "resolved pathname:%s\n", 
	    Yfixpath(argv[1],TRUE) ) ;
    } else {
	fprintf( stderr, "Error[syntax]: a.out pathName\n" ) ;
	exit(1) ;
    }
    exit(0) ;
}
#endif /* TEST */
