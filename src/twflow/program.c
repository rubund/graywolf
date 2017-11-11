/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    program.c 
DESCRIPTION:This file contains routines for executing and logging the
	    execution times of the program.
CONTENTS:   BOOL executePgm( adjptr )
		ADJPTR adjptr ;
	    set_path0( pathname )
		char *pathname ;
DATE:	    May  7, 1989 - original coding.
REVISIONS:  Jun 19, 1989 - shortened designName to $ for substitution.
	    Thu Mar  7 01:35:22 EST 1991 - now windowID is a global
		and twflow can inherit a window itself.
	    Sun Apr 21 22:37:20 EDT 1991 - changed windowId to @WINDOWID
		and added @FLOWDIR.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) program.c version 2.3 4/21/91" ;
#endif

#include <globals.h>
#include "twflow.h"

#define  DESIGNNAME   "$"
#define  DSNLEN       1
#define  FLOWDIR      "@FLOWDIR"

BOOL executePgm( ADJPTR adjptr, int debug )
{
	char *Yfixpath() ;               /* get full pathname */
	char command[LRECL] ;
	OBJECTPTR obj ;                  /* current object */
	int i ;                          /* arg counter */
	int status ;                     /* program return status */
	BOOL stateSaved ;                /* whether graphics state was saved*/

	obj = proGraphG[adjptr->node] ;

	/* build command to be executed */
	command[0] = EOS ; /* clear character string */

	stateSaved = FALSE ;  /* for remember whether we save graphics */

	D( "twflow/executePgm", sprintf( YmsgG, "%s\n", command ) ) ;
	D( "twflow/executePgm", M( MSG, NULL, YmsgG ) ) ;

	/* now log the beginning time */
	//Ylog_msg( YmsgG ) ;
	status = 0;

	if(!strcmp("edit_twfiles",obj->name)) {
		printf("It's edit_twfiles!\n");
		status = 0;
	}

	if(!strcmp("edit_mcfiles",obj->name)) {
		printf("It's edit_mcfiles!\n");
		status = 0;
	}

	int Mincut( int, char*);

	if(!strcmp("Mincut",obj->name)) {
		status = Mincut(debug,Ystrclone(cktNameG));
	}

	int TimberWolfMC(int b, int d, int n, int scale_dataP, int p, int q, int v, char *dName);

	if(!strcmp("TimberWolfMC",obj->name)) {
		status = TimberWolfMC(0, debug, !graphicsG, 0, 0, 0, debug, cktNameG);
	}

	int TimberWolfSC(int n, int v, char *cktName);

	if(!strcmp("TimberWolfSC",obj->name)) {
		status=TimberWolfSC (!graphicsG, debug, cktNameG);
	}

	sprintf( YmsgG, "%s completed...", obj->name ) ;
	Ylog_msg( YmsgG ) ;

	return( status ) ;

} /* end execute Pgm */

