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

/* ----------------------------------------------------------------- 
FILE:	    system.c                                       
DESCRIPTION:system routines
DATE:	    Apr 26, 1990 
REVISIONS:  May 12, 1990 - added move file and getenv.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) system.c version 3.4 8/28/90" ;
#endif

#include <yalecad/file.h>
#include <yalecad/message.h>
#include <dirent.h>
#include <unistd.h>

void YcopyFile( char *sourcefile, char *destfile )
{
	if( access( sourcefile, F_OK ) == -1 ) {
		printf("%s doesn't exist\n",sourcefile);
		return;
	}

	if( access( destfile, F_OK ) == -1 ) {
		remove(destfile);
	}

	FILE* source = fopen(sourcefile, "rb");
	FILE* dest = fopen(destfile, "wb");
	for (int i = getc(source); i != EOF; i = getc(source)) {
		putc(i, dest);
	}
	fclose(dest);
	fclose(source);
} /* end Ycopyfile */

int YmoveFile( char *sourcefile, char *destfile )
{
	return rename(sourcefile, destfile);
} /* end Ycopyfile */

int Yrm_files( char *files )
{
	if(remove(files)) {
		printf("Could not remove %s \n", files);
		if(rmdir(files)) {
			printf("Could not remove %s \n", files);
			return 1;
		} else {
			printf("Removed %s \n", files);
			return 0;
		}
	} else {
		printf("Removed %s \n", files);
		return 0;
	}
} /* end Ycopyfile */

char *Ygetenv( char *env_var )
{
	char *getenv() ;
	return( (char *) getenv( env_var ) ) ;

} /* end Ygetenv */
