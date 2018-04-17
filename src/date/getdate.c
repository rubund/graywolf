/*
 *   Copyright (C) 1990 Yale University
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


#include <stdio.h>

#define LRECL 256
#define EOS   '\0'

#ifndef lint
static char SccsId[] = "@(#) getdate.c version 1.2 5/12/90" ;
#endif

/* this small program creates a file called date.h */
/* which contains a define statement with the current time */

main( argc , argv )
int argc ;
char *argv ;
{

FILE        *fp ;
char        *date ,
	    buffer[LRECL] ;
int 	    len ;

system("date > date.h") ;

if( !(fp = fopen("date.h","r" ))){ 
    printf("Could not reopen date.h");
    exit(0) ;
}

rewind(fp) ;

/* read line from system call */
date = fgets(buffer,LRECL,fp) ;
fclose(fp) ;

if( !(fp = fopen("date.h","w" ))){ 
    printf("Could not open date.h");
    exit(0) ;
}

if( date ){
    /* get rid of newline character */
    len = strlen( date ) ;
    date[len-1] = EOS ;
    fprintf(fp,"#define DATE \"@\(#\) Yale compilation date:%s\"\n",date);
    fclose(fp) ;
    exit(0) ;
} 

/* errors - give unknown date */
fprintf(fp,"#define DATE \"@\(#\) Yale compilation date:unknown\"\n") ;
fclose(fp) ;

} /* end main */
