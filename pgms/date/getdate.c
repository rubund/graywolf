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
