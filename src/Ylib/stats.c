/*
 *   Copyright (C) 1990-1992 Yale University
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
FILE:	    stats.c                                       
DESCRIPTION:Get runtime statistics in UNIX.
CONTENTS:   Yprint_stats( fdat )
		FILE fdat ;
	    double Ycpu_time()
DATE:	    May  5, 1990 
REVISIONS:  Oct  4, 1990 - added elaspse time for the mips machine.
	    Fri Jan 25 15:44:46 PST 1991 - added AVOID conditional
		compile for HPUX.
	    Wed Feb 26 03:56:25 EST 1992 - added date for reference.
----------------------------------------------------------------- */
#include	<globals.h>

#ifdef VMS
#define AVOID
#endif

#ifdef HPUX
#define AVOID
#endif

#ifdef SYS5
#define AVOID
#endif

/* print out the statistics of the program to the given file */
void Yprint_stats( FILE *fout )
{
	char	hostname[256]	;
	char	*timestring ;
	int text		;
	int data		;
	int vm_used		;
	int vm_limit	;
	int vm_soft_limit	;
	int elapsed_time    ;
	double user		;
	double systemTime	;
	double scale		;
	double run_time	;

	char *YcurTime();	/* Forward declaration */

	struct rusage	rusage	;
	struct rlimit 	rlp	;
	caddr_t p	;
	//caddr_t sbrk()	;

	/***********************************************************
	* Get the hostname
	***********************************************************/
	gethostname(hostname, 256) ;
	hostname[255] = EOS ;		/* just in case */

	/***********************************************************
	* Get the brk() value
	***********************************************************/
	p = sbrk(0) ;
	vm_used = ((int) p) / 1024.0 + 0.5 ;

	/***********************************************************
	* Get virtual memory limits
	***********************************************************/
	getrlimit(RLIMIT_DATA, &rlp) ;
	vm_limit = rlp.rlim_max / 1024.0 + 0.5 ;
	vm_soft_limit = rlp.rlim_cur / 1024.0 + 0.5 ;

	/***********************************************************
	* get usage stats
	***********************************************************/
	getrusage(RUSAGE_SELF, &rusage) ;
	user = rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec / 1.0e6 ;
	systemTime = rusage.ru_stime.tv_sec + rusage.ru_stime.tv_usec/1.0e6 ;
	scale = (user + systemTime) * 100.0 ;
	if (scale == 0.0)
	{
		scale = 0.001 ;
	}
	/* now get elapsed time */
	Ytimer_elapsed( &elapsed_time ) ;
	/* time is in milliseconds */
	run_time = ( (double) elapsed_time) / 1000.0 ;

	fprintf(fout, "\n\n") ;
	fprintf(fout, "Runtime Statistics\n") ;
	fprintf(fout, "-------------------------\n") ;
	fprintf(fout, "Machine name: %s\n", hostname) ;

	// This tends to segfault.  Don't know why (Tim, 10/14/08)
	// fprintf(fout, "Date        : %s\n\n", YcurTime(0)) ;
	timestring = YcurTime(0); 
	fprintf(fout, "Date        : %s\n\n", timestring) ;

	fprintf(fout, "User    time:%6.1f seconds\n", user) ;
	fprintf(fout, "System  time:%6.1f seconds\n", systemTime) ;
	fprintf(fout, "Elapsed time:%6.1f seconds\n\n", run_time) ;

	text = rusage.ru_ixrss / scale + 0.5 ;
	data = (rusage.ru_idrss + rusage.ru_isrss) / scale + 0.5 ;
	fprintf(fout, "Average resident text size       = %5dK\n", text) ;
	fprintf(fout, "Average resident data+stack size = %5dK\n", data) ;
	fprintf(fout, "Maximum resident size = %5dK\n", rusage.ru_maxrss/2) ;
	fprintf(fout, "Virtual memory size = %5dK\n", vm_used) ;
	fprintf(fout, "Virtual memory limit = %5dK (%dK)\n", vm_soft_limit, vm_limit) ;

	if( YgetMaxMemUse() ){
		fprintf(fout, "Maximum heap size                = %5d\n", 
		YgetMaxMemUse() ) ;
		fprintf(fout, "Current heap size                = %5d\n", 
		YgetCurMemUse() ) ;
	} 

	fprintf(fout, "\nMajor page faults = %d\n", rusage.ru_majflt) ;
	fprintf(fout, "Minor page faults = %d\n", rusage.ru_minflt) ;
	fprintf(fout, "Swaps = %d\n\n", rusage.ru_nswap) ;

	fprintf(fout, "Input blocks = %d\n", rusage.ru_inblock) ;
	fprintf(fout, "Output blocks = %d\n\n", rusage.ru_oublock) ;

	fprintf(fout, "Context switch (voluntary) = %d\n", rusage.ru_nvcsw) ;
	fprintf(fout, "Context switch (involuntary) = %d\n", rusage.ru_nivcsw) ;
} /* end Yprint_stats */
/* ==================================================================== */

double Ycpu_time()
{
    struct rusage rusage;
    double user, systemTime;

    getrusage(RUSAGE_SELF, &rusage);
    user = rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec/1.0e6;
    systemTime = rusage.ru_stime.tv_sec + rusage.ru_stime.tv_usec/1.0e6;
    return( user+systemTime ) ;
} /* end Ycpu_time */
/* ==================================================================== */

/* #define TEST */
#ifdef TEST

/* test whether code works correctly. */

main()
{
    Yprint_stats( stdout ) ;
    exit(0) ;
}

#endif
    
