/*
 *   Copyright (C) 1988-1992 Yale University
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
FILE:	    string.c                                       
DESCRIPTION:miscellaneous string utility functions 
CONTENTS:   char **Ystrparser(char *,char *,int)
	    char *Ystrclone(char *)
	    char *str_tok(char *,char *)
	    char *Yremove_lblanks( char * )
DATE:	    Jan 29, 1988 
REVISIONS:  May 04, 1988 - updated initProgram to include 
			introTextFunction. Added remove_lblanks,
			directoryExists functions.
	    Sep 26, 1988 - broke utils.c into three for archive.
	    May  3, 1989 - added Yprefix and change name of scanner
		for Dahe.
	    Aug 13, 1990 - modified Yremove_lblanks to understand
		tab characters too.
	    Tue Oct 23 01:32:34 EDT 1990 - removed definition of
		Ysafe_free.
	    Wed Apr 17 23:37:19 EDT 1991 - fixed logic problem with
		Yremove_lblanks.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) string.c version 3.8 2/29/92" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/string.h>

extern char *str_tok( P2( char *str, char *delimiters ) ) ; 
 
/* ----------------------------------------------------------------- 
                String routines                                    */ 
	  
/* Ystrclone - clone a string - user must free memory when done */
char *Ystrclone(str)
char *str ;
{
    if( str ){  /* first check for null pointer */
	if( *str ){  /* first check allow dereference */
	    return( (char *) strcpy( YMALLOC(strlen(str)+1, char), str ) ) ;
	}
    }
    return(NULL) ;

} /* end strclone */

char **Ystrparser(str,dels,numtokens)
char *str;
char *dels ; /* delimiters */
INT  *numtokens ; /* pass back number of tokens */
{

    static char *tokenBuf[LRECL] ; /* impossible to have more tokens */
				  /* than length of input line length */
    INT  index = 0 ;

    /* use str_tok to strip away delimiters */
    /* first call to str_tok requires the string reference */
    tokenBuf[index] = str_tok(str,dels) ;
    while( tokenBuf[index] ){
	/* subsequent calls to str_tok doesn't requires the string ref */
	/* str_tok remembers place in string */
	tokenBuf[++index] = str_tok(NULL,dels) ;
    }
    /* return number of tokens and token buffer */
    *numtokens = index ;
    return(tokenBuf) ;
} /* end Ystrparser */

#ifndef apollo
/* SYS5 string command str_tok - a string tokenizer */
char *str_tok(str,dels)
char *str ;  /* input string */
char *dels ; /* delimiters of string */
{

    static char *p,        /* current place in string */
		*token ;   /* substring */

    if( str ){		/* check if new string */
	p = str ;
    }
    /* remove leading delimiters */
    while( *p && strchr(dels,*p) ){
	++p ;
    }

    /* test for EOS */
    if( *p ){
	token = p++ ;    /* mark beginning of token */
	/* find end of token */
	while( *p ){
	    if( strchr(dels,*p) ){
		break ;
	    } else {
		++p ;
	    }
	}
	/* mark end of string */
	if( *p ){
	    *p++ = EOS ;
	}
	return(token) ;
    }
    /* no valid delimiters found */
    return(NULL) ;
}
/* end str_tok */
#endif

/* Yremove_lblanks - remove leading blanks from a string */
/* including tabs */
char *Yremove_lblanks( bufferptr )
char *bufferptr ;
{
    
    for( ;bufferptr && *bufferptr;bufferptr++){
	if( *bufferptr != ' ' && *bufferptr != '\t' ){
	    break ;
	}
    }
    return(bufferptr);
}
/* ---------------end string ------------------------------------- */
