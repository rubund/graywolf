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
FILE:	    okmalloc.c                                       
DESCRIPTION:This file contains memory management routines. 
	    User should always call safe_malloc, safe_free, etc.
	    A conditional compile allows the choice between this
	    memory manager and system.  Using the memory manager
	    helps make the environment similar on all machines.
CONTENTS:   
	    ++++++++++++ HEAP MANAGEMENT ROUTINES ++++++++++++++
	    static status_t  heapDispose (ptr)
		char_p ptr;
	    static status_t  allocateRegion (min_size)
		INT min_size;
	    static status_t  heapInit (initial_size)
		INT initial_size;
	    static status_t  heapNew (ptr, req_size)
		char_p  *ptr;
		INT   req_size;
	    +++++++++ END HEAP MANAGEMENT ROUTINES ++++++++++++ 
	      USER  CALLS FOR C RUN TIME LIBRARY 
	    VOID Ysafe_free(ptr)
		char *ptr;
	    VOID Ysafe_cfree(ptr)
		char *ptr;
	    char *Ysafe_malloc(bytes)
		long bytes;
	    char *Ysafe_calloc(num_entries, bytes)
		long num_entries;
		long bytes;
	    char *Ysafe_realloc(ptr, bytes)
		char *ptr;
		long bytes;
	    INT YgetCurMemUse()
	    INT YcheckMemObj(ptr)
		char *ptr ;
	    INT Yinit_memsize(memsize)
		INT memsize ;
	    INT YgetListSize(ptr, offsetPtr)
		char *ptr ;     
		char *offsetPtr ;
	    YdebugMemory( flag )
		BOOL flag ;
	    Ypmemerror( s )
		char *s ;
	    YcheckDebug( where )
		INT where ; 
	    char *Yvector_alloc( lo, hi, size )
		INT size, lo, hi ;
	    char *Yvector_calloc( lo, hi, size )
		INT size, lo, hi ;
	    char *Yvector_realloc( array_orig, lo, hi, size )
	    VOIDPTR array_orig ;
		INT size, lo, hi ;
	    VOID Yvector_free( array, lo, size )
		VOIDPTR array ;
		INT lo, size ;
DATE:	    Feb  2, 1988 
REVISIONS:  Sep 25, 1988 - converted to common utility.
	    Feb 22, 1989 - added new memory check for debugger.
		    made local function calls static - hide from linker.
	    Apr 18, 1989 - added Yinit_memsize for init memory size.
	    May  3, 1989 - added Y prefixes.
	    Sep 16, 1989 - all debug directed to stderr.
	    Dec 21, 1989 - now initialize freed memory to -1 to
		catch more bugs.
	    Jan 31, 1990 - corrected arg reversal in calloc call.
	    Feb  4, 1990 - fixed bug in initializing freed memory.
		Also renamed static variables.
	    Tue Oct 23 03:33:19 EDT 1990 - fixed for prototypes in
		base.h.
	    Tue Jan 15 02:06:35 EST 1991 - added vector allocation
		routines.
	    Thu Jan 24 20:19:46 PST 1991 - added more vector routines.
	    Thu Jan 31 15:42:52 EST 1991 - added char * cast to 
		voidptr use in vector routines.
	    Fri Feb 15 15:37:16 EST 1991 - added call to message
		system so that silent mode will work.
	    Sat May 11 22:53:49 EDT 1991 - added a conditional compile
		for HPUX.
	    Sat Dec 14 14:33:10 EST 1991 - added YgetMaxMemUse()
	    Sun Dec 15 02:30:59 EST 1991 - added the MEM_DEBUG conditional
		compile.  We added extra arguments to the alloc routines
		so that we could pass the line that the memory was
		created.  We now can dump all the allocated memory by
		calling Ydump_mem().
	    Tue Jan  7 18:03:31 EST 1992 - fixed memory manager
		on the MAC.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) okmalloc.c (Yale) version 3.24 3/6/92" ;
#endif

#include	<stdio.h>
#include	<unistd.h>
#include	<signal.h>
#include	<errno.h>
#include 	<yalecad/base.h>
#include 	<yalecad/message.h>

#ifdef HPUX
#undef		NBPG
#include        <machine/parm.h>
#endif


/* memory errors */
#define  heap_error_base   0x40060000
#define  heap_bad_block    (heap_error_base + 1)
#define  heap_no_mem       (heap_error_base + 2)
#define  heap_ok          0

#ifdef MEM_DEBUG
#include        	<yalecad/file.h>    
/* -----------------------------------------------------------------
   These two unusual definitions MEM_DEBUG1 and MEM_DEBUG2 add 
   arguments to memory user routines so that we can pass the file
   and line number from the preprocessor.
----------------------------------------------------------------- */
#define MEM_DEBUG1	,file,line
#define MEM_DEBUG2      char *file ; int line ;
#define ALLOC_NAME_SIZE 40
#else
#define MEM_DEBUG1
#define MEM_DEBUG2
#endif /* MEM_DEBUG */

/* conditional compile for the memory manager */
/* if MEMORYMAN is not defined use system malloc calloc etc. */

#ifdef MEMORYMAN

#define systype "bsd4.2"
/*
  *--------------------------------------------------------------
  * C MEMORY MANAGEMENT SYSTEM
  *          Description : These Routines replace malloc/calloc
  *                        & free calls.  AVOID deficiencies in
  *                        APOLLO heap management routines.
  *
  * CODED BY : Bill Swartz
  *--------------------------------------------------------------
*/

#define  mp_block_cnt      256
#define  region_mask       0xFFFF8000
#define  region_overhead   8
#define  DFL_REGION_SIZE   (4095 * 1024) 
#define  expected_size     (256  * 1024) 
#define  waste_allowed     28
#define  FOREVER           1

#define  block_overhead    (sizeof(header_t) + sizeof(trailer_t))
#define  double_word_mask  0xFFFFFFFC

#define  char_p            char *

typedef  struct {
            INT              size;
         }  header_t;

typedef  struct {
            INT              all;
         }  status_t;

typedef  struct block_type {
            header_t         head;
            union {
               struct {
                  struct block_type   *link;
                  struct block_type   *back;
               } zero_case;
               INT              data;
            } case_of_two;
         }  block_t, *block_p;

typedef  INT              region_head_t, *region_head_p;

typedef  struct {
            INT             size;
         }  trailer_t, *trailer_p;

typedef  struct {
            block_t          avail;
            block_p          alloc_rover;
            region_head_p    highest_region;
            INT              cnt_blks;
            INT              max_blks;
         }  zone_t;

static  zone_t       zoneS ;
static status_t      statuS;
static BOOL heapNotStartedS = TRUE;
static INT totalAllocationS = 0 ;
static BOOL debugFlagS = FALSE ;
static INT inUseS = 0 ;
static INT regionSizeS = DFL_REGION_SIZE ;
static INT max_usedS = 0 ;

#ifdef MEM_DEBUG

/* keep an linked list of allocated memory */
typedef struct mem_obj {
    char name[ALLOC_NAME_SIZE] ;
    char *ptr_p ;
    struct mem_obj *next ;
    struct mem_obj *prev ;
} MEMOBJ, *MEMOBJPTR ;

static MEMOBJ name_sentinelS ;
static MEMOBJPTR name_listS = &name_sentinelS ;

#endif /* MEM_DEBUG */


/* ++++++++++++++++++ HEAP MANAGEMENT ROUTINES ++++++++++++++ */
/* ---------heap_dispose--------- */
static status_t  heapDispose (ptr)
char_p        ptr;
{
   status_t     st;
   trailer_p     tail;
   block_p       headPtr;
   block_p       check;
   block_p       lower;
   block_p       higher;
   block_p       temp;
   long          l ;
   long          *i ;

/*  Begin procedure heap dispose.  */

   headPtr = (block_p) ((INT) ptr - sizeof(header_t));
   inUseS += headPtr->head.size ;
   tail = (trailer_p) ( ((INT)headPtr) - headPtr->head.size - sizeof(trailer_t));
   if ((headPtr->head.size > 0) || (headPtr->head.size != tail->size)) {
     st.all = heap_bad_block;
     return (st);
   } else if( debugFlagS ) {
	/* set the old memory to -1 */	
	headPtr = (block_p) ((INT) ptr - sizeof(header_t));
	/* find number of words; - 2 accounts for header and trailer */
	l = ABS( headPtr->head.size ) / sizeof(long) - 2 ;
	for(i=(long *) ptr;l>0;i++, l--){
	    *i=(long)-1;
	}
   }

   headPtr->head.size *= -1;
   tail = (trailer_p) ( ((INT)headPtr) - sizeof(trailer_t));
   if (tail->size > 0) {
      check = (block_p) ( ((INT)headPtr) - tail->size);
      higher = check->case_of_two.zero_case.link;
      lower = check->case_of_two.zero_case.back;
      higher->case_of_two.zero_case.back = lower;
      lower->case_of_two.zero_case.link = higher;
      check->head.size += headPtr->head.size;
      headPtr = check;
   }

  check = (block_p) ( ((INT)headPtr) + headPtr->head.size);
  if (check->head.size > 0) {
      if (zoneS.alloc_rover == (block_p) &(check->head.size)) {
         zoneS.alloc_rover = (block_p) &(headPtr->head.size);
      }
      higher = check->case_of_two.zero_case.link;
      lower = check->case_of_two.zero_case.back;
      higher->case_of_two.zero_case.back = lower;
      lower->case_of_two.zero_case.link = higher;
      headPtr->head.size += check->head.size;
   }
   tail = (trailer_p) ( ((INT)headPtr) + headPtr->head.size - sizeof(trailer_t));
   tail->size = headPtr->head.size;
   headPtr->case_of_two.zero_case.link = zoneS.avail.case_of_two.zero_case.link;
   headPtr->case_of_two.zero_case.back = (block_p) &(zoneS.avail);
   temp =  (block_p) &(headPtr->head);
   zoneS.avail.case_of_two.zero_case.link->case_of_two.zero_case.back = temp ;
   zoneS.avail.case_of_two.zero_case.link = temp ;

#ifdef MEM_DEBUG
    {
	MEMOBJPTR mem_p ;
	/* this is brain dead for now */
	for( mem_p = name_listS->next; mem_p != name_listS; mem_p = mem_p->next ){
	    if( mem_p->ptr_p == ptr ){
		/* delete this from memory */
		mem_p->prev->next = mem_p->next ;
		mem_p->next->prev = mem_p->prev ;
		heapDispose( mem_p ) ;
		break ;
	    }
	}
    }
#endif /* end MEM_DEBUG */

   st.all = heap_ok;
   return (st);
}  /*  End of procedure heap dispose.  */


/* ---------- Utility Routines ---------- */

static status_t  allocateRegion (min_size)
INT            min_size;
{
typedef  INT   *tag_p;

   status_t      st;
   block_p        head;
   trailer_p      tail;
   INT            allocation;
   INT            pageSize;
   INT            i;
   INT            *memory;

/*  Begin procedure allocate region.  */
#ifndef SYS5
   pageSize = getpagesize() ;
#else /* SYS5 */
   pageSize = NBPG ;
#endif /* SYS5 */
   if( min_size < expected_size ){
	min_size = expected_size ;
   } 
   allocation = min_size + 2*sizeof(INT);
   allocation += pageSize - (allocation % pageSize) ;
   totalAllocationS += allocation ;
   head = (block_p) sbrk(0) ;
   sprintf( YmsgG, "Alternate MEMORY MANagement system invoked - allocation:%d bytes\n", allocation ) ;
   M( MSG, NULL, YmsgG ) ;
   if( debugFlagS ){ 
       fprintf( stderr, "Memory debug switch on\n") ;
       fprintf( stderr, "old starting memory address:%0x Page size = %d\n",
	    head,pageSize) ;
   } 
   head = (block_p) sbrk(allocation) ; 
   if( debugFlagS ){ 
       fprintf( stderr,
           "new starting memory address:%0x with allocation:%d bytes\n",
            head, allocation) ;
       if( !( heapNotStartedS) ){
	   fprintf( stderr,
	   "current memory request = %d bytes approx. %d pages\n",
	       min_size,min_size/pageSize) ;
	   fprintf( stderr,
	   "total user memory allocated = %d bytes approx. %d pages\n",
	        inUseS,inUseS/pageSize) ;
       }
       fprintf( stderr,
       "new memory space = %d pages of %d bytes\n",
	    totalAllocationS / pageSize, pageSize) ;
       fprintf( stderr,"total current allocation = %d\n\n",totalAllocationS ) ;
   }
   /* head = (block_p) malloc(allocation) */ ;
   if ( !(head) || head == (block_p) -1) {
       st.all = heap_no_mem ;
       return( st ) ;
   }
   if( debugFlagS ){ 
       /* initialize all bytes of memory to 1 to catch bugs */
       memory = (INT *) head ;
       for( i=0;i<allocation/4;i++){
	    memory[i] = -1 ;
       }
   } 

   inUseS += allocation - 2*sizeof(INT) ;
   tail = (trailer_p) ((INT) head + allocation - sizeof(INT));
   if (zoneS.highest_region != NULL) {
      if (zoneS.highest_region + ABS(*zoneS.highest_region) == (INT *) head) {
         *zoneS.highest_region = *zoneS.highest_region - allocation;
         head = (block_p) ((INT) head - sizeof(INT));
      } else {
         zoneS.highest_region = (INT *) head;
         *zoneS.highest_region = -allocation;
         head = (block_p) ((INT) head + sizeof(INT));
         allocation = allocation - 2*sizeof(INT);
      }
   } else {
      zoneS.highest_region = (INT *) head;
      *zoneS.highest_region = -allocation;
      head = (block_p) ((INT) head + sizeof(INT));
      allocation = allocation - 2*sizeof(INT);
   }

   tail->size = *zoneS.highest_region;
   tail = (trailer_p) ((INT) head + allocation - sizeof(INT));
   head->head.size = tail->size = -allocation;
   head = (block_p) ((INT) head + sizeof(INT));
   st = heapDispose (head);
   zoneS.alloc_rover = zoneS.avail.case_of_two.zero_case.link;
   st.all = heap_ok;
   return (st);
}  /*  End procedure allocate region.  */


/* -------- heap_init -------- */
static status_t  heapInit (initial_size)
INT           initial_size;
{
   status_t     st;
   block_p       block;
   INT           allocation;

/*  Begin procedure heap init.  */

   /* initialize zone record */
   zoneS.avail.head.size = 0 ;
   zoneS.avail.case_of_two.zero_case.link = &(zoneS.avail) ;
   zoneS.avail.case_of_two.zero_case.back = &(zoneS.avail) ;
   zoneS.alloc_rover = &(zoneS.avail) ;
   zoneS.highest_region = NULL ;
   zoneS.cnt_blks = 0 ;
   zoneS.max_blks = 0 ;
#ifdef MEM_DEBUG
   name_listS->next = name_listS ; 
   name_listS->prev = name_listS ; 
#endif /* MEM_DEBUG */

   if (initial_size > 0) {
      st = allocateRegion (initial_size);
      if (st.all != heap_ok){
         return (st);
      } else {
         heapNotStartedS = FALSE;
      }
   }
   st.all = heap_ok;
   return (st);
}  /*  End of procedure heap init.  */


/* -------- heap_new --------- */
static status_t  heapNew (ptr, req_size MEM_DEBUG1 )
char_p        *ptr;
INT           req_size;
MEM_DEBUG2
{
   status_t       st;
   INT            excess;
   INT            allocation;
   INT            block_size;
   block_p        new_block;
   block_p        check;
   trailer_p      trailer;
   region_head_p  region;

/*  Begin procedure heap new.  */
   if (req_size <= 0) {
      *ptr = NULL;
      st.all = heap_ok;
      return (st);
   } else if (heapNotStartedS){
      heapInit(regionSizeS);
   }
   block_size = (req_size + 3 + block_overhead) & double_word_mask;
   check = zoneS.alloc_rover;
   do {
      if (check->head.size < block_size) {
         check = check->case_of_two.zero_case.link;
      } else {
         excess = check->head.size - block_size;
         zoneS.alloc_rover = check->case_of_two.zero_case.link;
         if (excess < waste_allowed) {
            check->case_of_two.zero_case.back->case_of_two.zero_case.link =
	        check->case_of_two.zero_case.link;
            check->case_of_two.zero_case.link->case_of_two.zero_case.back =
		check->case_of_two.zero_case.back;
            block_size = check->head.size;
         } else {
            check->head.size = excess;
            trailer = (trailer_p) ((INT) check + excess - sizeof(trailer_t));
            trailer->size = excess;
            check = (block_p) ((INT) check + excess);
         }
         check->head.size = -block_size;
         trailer = (trailer_p) ((INT) check + block_size - sizeof(trailer_t));
         trailer->size = -block_size;
         *ptr = (char_p) &(check->case_of_two.data);
#ifdef MEM_DEBUG
	 {  
	    INT len ;
	    char alloc_name[BUFSIZ] ;
	    MEMOBJPTR name_data ;
	    if( file ){
		sprintf( alloc_name, "%s:%d", file, line ) ;
		len = strlen( alloc_name ) ;
		if( len < ALLOC_NAME_SIZE ){
		    /* allocate space for record info but don't put in tree */
		    statuS = heapNew (&name_data, sizeof(MEMOBJ), NULL, 0 ) ;
		    if (statuS.all!=heap_ok){
		       errno = statuS.all ;	
		       kill(getpid(),SIGUSR1);
		    }
		    strcpy( name_data->name, alloc_name ) ;
		    name_data->ptr_p = *ptr ;
		    name_data->next = name_listS->next ;
		    name_listS->next->prev = name_data ;
		    name_listS->next = name_data ;
		    name_data->prev = name_listS ;
		} else {
		    fprintf( stderr,
			"Alloc name:%s too long to store\n",alloc_name ) ;
		}
	    }
	 }
#endif /* MEM_DEBUG */
	 inUseS += block_size ;
	 if( inUseS > max_usedS ) max_usedS = inUseS ;
         st.all = heap_ok;
         return (st);
      }
      if (check == zoneS.alloc_rover) {
         st = allocateRegion (block_size);
	 if( inUseS > max_usedS ) max_usedS = inUseS ;
         if (st.all != heap_ok)
            return (st);
         check = zoneS.alloc_rover;
      }
   } while (FOREVER);
}  /*  End procedure heap new.  */

VOID Ydump_mem()
{
#ifdef MEM_DEBUG
    FILE *fp ;
    MEMOBJPTR	mem_p ;
    static INT dump_noL = 0 ;
    char filename[LRECL] ;
    INT sum_allocated ;
    INT size ;

    sum_allocated = 0 ;
    sprintf( filename, "mem.data.%d", ++dump_noL ) ;
    fp = TWOPEN( filename, "w", ABORT ) ;
    fprintf( fp, "Address:amount_allocated file:line_number\n" ) ;
    for( mem_p = name_listS->next; mem_p != name_listS; mem_p = mem_p->next ){
	size = YcheckMemObj(mem_p->ptr_p) ;
	fprintf( fp, "%x:%d %s\n", mem_p->ptr_p, size, mem_p->name ) ;
	sum_allocated += size ;
    }
    fprintf( fp, "-----------------------------------------------------------\n");
    fprintf( fp, "Total allocation:%d\n", sum_allocated ) ;

    TWCLOSE( fp ) ;
#else /* MEM_DEBUG */
    fprintf( stderr, "Ydump_mem() is not available. Use libycad.mem-d.a\n" ) ;
#endif /* MEM_DEBUG */
} /* end Ydump_mem() */


/* ++++++++++++++++++ END HEAP MANAGEMENT ROUTINES ++++++++++++++ */

/*  SUBSTITION CALLS FOR C RUN TIME LIBRARY */

VOID Ysafe_free(ptr MEM_DEBUG1 )
VOIDPTR ptr;
MEM_DEBUG2
{
    if( !( ptr ) ){
	fprintf( stderr, 
	    "WARNING[safe_free]:pointer = nil.  Ignoring safe_free.\n") ;
	return ;
    } 

    statuS = heapDispose (ptr);
    if (statuS.all != heap_ok){
	errno = statuS.all ;	
	kill(getpid(),SIGUSR1);
    }
}

VOID Ysafe_cfree(ptr MEM_DEBUG1 )
VOIDPTR ptr;
MEM_DEBUG2
{
    if( !( ptr ) ){
	fprintf(stderr,
	"WARNING[safe_cfree]:pointer = nil.  Ignoring safe_cfree.\n") ;
	return ;
    } 

    statuS = heapDispose (ptr);
    if (statuS.all!=heap_ok){
	errno = statuS.all ;	
	kill(getpid(),SIGUSR1);
    }
}


char *Ysafe_malloc(bytes MEM_DEBUG1 )
INT bytes;
MEM_DEBUG2
{
   char *ptr;
   char *i;
   if (bytes<5) {
      bytes = 8;
   }
   statuS = heapNew (&ptr, bytes MEM_DEBUG1 );
   if (statuS.all!=heap_ok){
       errno = statuS.all ;	
       kill(getpid(),SIGUSR1);
   }
   return(ptr);
}

char *Ysafe_calloc(num_entries, bytes MEM_DEBUG1 )
INT num_entries;
INT bytes;
MEM_DEBUG2 
{
   char *ptr;
   long *i;
   long k,l;

   k = bytes*num_entries;
   if (k<5) {
      k = 8;
   }
   statuS = heapNew (&ptr, k MEM_DEBUG1 );
   if (statuS.all!=heap_ok){
       errno = statuS.all ;	
       kill(getpid(),SIGUSR1);
   }
   l = (k+3)>>2;
   for(i=(long *)ptr;l>0;i++, l--){
      *i=(long)0;
   }
   return(ptr);
}

char *Ysafe_realloc(ptr, bytes MEM_DEBUG1 )
VOIDPTR ptr;
INT bytes;
MEM_DEBUG2
{

   char               *ptr2;
   INT                oldSize;
   INT                i;
   block_p            headPtr;
   trailer_p          tail;

   /* allocate new memory */
   ptr2 = Ysafe_malloc(bytes MEM_DEBUG1 );

   /* get current size of ptr */
   headPtr = (block_p) ((INT) ptr - sizeof(header_t));
   tail = (trailer_p) ( ((INT)headPtr) - headPtr->head.size
	   - sizeof(trailer_t));
   if ((headPtr->head.size > 0) || (headPtr->head.size != tail->size)) {
       errno = heap_bad_block ;
       kill(getpid(),SIGUSR1);
   }
   /* copy only the smaller amount of two blocks */
   oldSize = - (headPtr->head.size) ;
   if( oldSize < bytes ){
      bytes = oldSize ;
   }	

   for (i=0; i<bytes; i++){
       ptr2[i] = ((char *) ptr)[i];
   }


   Ysafe_free(ptr MEM_DEBUG1 );

   return(ptr2);
}

VOID Yinit_memsize( memsize )
INT memsize ;
{
    regionSizeS = memsize ;
} /* end Yinit_memsize */

/* getCurMemUse - returns the current allocated memory by user */
INT YgetCurMemUse()
{
    return(inUseS) ;
}

/* getMaxMemUse - returns the maximum allocated memory by user */
INT YgetMaxMemUse()
{
    return(max_usedS) ;
}

/* checkMemObj(ptr) - returns the size of the object pointed to */
/* returns size in bytes.  returns -1 if invalid object */
INT YcheckMemObj(ptr)
char *ptr ;
{
   block_p            headPtr;
   trailer_p          tail;

   /* get current size of ptr */
   headPtr = (block_p) ((INT) ptr - sizeof(header_t));
   tail = (trailer_p) ( ((INT)headPtr) - headPtr->head.size
	   - sizeof(trailer_t));
   if ((headPtr->head.size > 0) || (headPtr->head.size != tail->size)) {
      return(-1) ;
   }
   return(-headPtr->head.size - block_overhead) ;
} /* end checkMemObj */

/* checkDebug call checkMemObj so it can be used in debugger */
INT YcheckDebug( where )
VOIDPTR where ; /* must be integer to work in dbx */
{
    INT size ;

    if( (size = YcheckMemObj( (char *) where )) == -1 ){
	fprintf( stderr, "Memory has been damaged\n" ) ;
	return( 0 ) ;
    } else {
	return( size ) ;
    }
} /* end checkDebug */

/* getListSize(ptr,offset) - returns the size of a linked-list */
/* returns size in bytes.  returns -1 if invalid object */
/* example.  getListSize(netPtr,&(netPtr->next) ) ;  */
INT YgetListSize(ptr, offsetPtr)
char *ptr ;     /* pointer to beginning of list structure */
char *offsetPtr ;  /* pointer to "next" field within structure */
{
   block_p            headPtr;
   trailer_p          tail;
   INT		      recordCount = 0 ;
   INT		      memInUse = 0 ;
   INT		      offset ;
   INT		      limit ;    /* max number of records */
   INT                *intPtr ;
   INT                temp ;

   /* first calculate offset of next field */
   offset = offsetPtr - ptr ;

   /* calculate limit to detect circular link list */
   /* the max number of records = MaxMemory/sizeof(record)
   /* need current size of ptr for calculation */
   headPtr = (block_p) ((INT) ptr - sizeof(header_t));
   tail = (trailer_p) ( ((INT)headPtr) - headPtr->head.size
      - sizeof(trailer_t));
   if ((headPtr->head.size > 0) || (headPtr->head.size != tail->size)) {
       return(-1) ;
   }
   if( headPtr->head.size ){
       limit =  - totalAllocationS / headPtr->head.size ;
   } else {
      return(-1) ;	
   }

   while( ptr ) {   /* perform check while pointer is not null */
       	
       headPtr = (block_p) ((INT) ptr - sizeof(header_t));
       tail = (trailer_p) ( ((INT)headPtr) - headPtr->head.size
             - sizeof(trailer_t));
       if ((headPtr->head.size > 0) || (headPtr->head.size != tail->size)) {
	    if( debugFlagS ){
	       fprintf( stderr, "ERROR[getListSize]:record has corrupted data\n") ;
            }
            return(-1) ;
       }
       memInUse += - headPtr->head.size ;
       /* check for circular linked list */
       if( recordCount > limit) {
	  fprintf( stderr, "Detected a circular linked list\n") ;
          return(-1) ;
       }	

       /* update pointer */
       /* calculate addresss of next field */
       intPtr = (INT *) (ptr + offset) ;
       /* next line does indirect addressing - contents of */
       /* field is put in temp.  Note we use char point but */
       /* we need all four bytes of the next field for new pointer */
       temp = *intPtr ;
       ptr = (char *) temp ;
       recordCount++ ;
    }
    return(memInUse) ;
}

/* debugMemory - turns on memory check debug messages and */
/* memory to all 1's to detect access violations. */
VOID YdebugMemory( flag )
BOOL flag ;
{
    debugFlagS = flag ;
}

/*
  *--------------------------------------------------------------
  * END OF C MEMORY MANAGEMENT SYSTEM
  *--------------------------------------------------------------
*/

#else 

/*
  *--------------------------------------------------------------
  * BEGIN NORMAL C MEMORY MANAGEMENT SYSTEM
  *--------------------------------------------------------------
*/

/* use standard calls to malloc, calloc, etc */

char *Ysafe_malloc(size)
INT size;
{
    char *p;

    /*extern char *malloc() ;*/

    if ((p = malloc(size)) == (char *) 0) {
        errno = heap_no_mem ;
        kill(getpid(),SIGUSR1);
    }
    return p;
}


char *Ysafe_realloc(obj, size)
VOIDPTR obj;
INT size;
{
    char *p;

    /* extern char *realloc() ;*/

    if ((p = realloc(obj, size)) == (char *) 0) {
        errno = heap_no_mem ;
        kill(getpid(),SIGUSR1);
    }
    return p;
}


char *Ysafe_calloc(num, size)
INT size, num;
{
    char *p;

    /*extern char *calloc() ;*/

    if ((p = calloc(num,size)) == (char *) 0) {
        errno = heap_no_mem ;
        kill(getpid(),SIGUSR1);
    }
    return p;
}
/* when not testing memory just call system free */
VOID Ysafe_free(ptr)
VOIDPTR ptr;
{
    free(ptr);
    return;
}

VOID Ysafe_cfree(ptr)
VOIDPTR ptr;
{
    cfree(ptr);
    return;
}

/* ***********DUMMY ROUTINES TO RESOLVE GLOBALS **************** */
/* see above for normal use */
INT YgetCurMemUse()
{
    return(0) ;
}

INT YgetMaxMemUse()
{
    return(0) ;
}

INT YcheckMemObj(ptr)
char *ptr ;
{
   return(0) ;
}

INT YgetListSize(ptr, offsetPtr)
char *ptr ;     /* pointer to beginning of list structure */
char *offsetPtr ;  /* pointer to "next" field within structure */
{
    return(0) ;
}

VOID YdebugMemory( flag )
INT flag ;
{
    return ;
}

INT YcheckDebug( where )
VOIDPTR where ; 
{
    return ( INT_MAX ) ;
} /* end checkDebug */

VOID Yinit_memsize( memsize )
INT memsize ;
{
    return ;
} /* end Yinit_memsize */

VOID Ydump_mem()
{
} /* end Ydump_mem() */
/*
  *--------------------------------------------------------------
  * END NORMAL C MEMORY MANAGEMENT SYSTEM
  *--------------------------------------------------------------
*/
#endif

/* print memory error in the same style as perror and psignal */
VOID Ypmemerror( s )
char *s ;
{
    /* first print user message if available */
    if( s ){
	fprintf( stderr, "%s:", s ) ;
    }
    switch(errno){
	case heap_ok:
	   fprintf(stderr,
	   "Memory ok - Problem in memory management logic.\n" ) ;
	   break; 
	case heap_bad_block:
	   fprintf(stderr,
	   "Memory block was found to be corrupted.\n" ) ;
	   break; 
	case heap_no_mem:
	   fprintf(stderr,
	   "No memory available to allocate.\n" ) ;
	   break; 
	default:
	   fprintf(stderr,
	   "Error = %0x Unrecognized error code.\n",errno ) ;
    }
} /* end Ypmemerror */

/* *******  memory convenience functions  ******* */
/* ALLOCATE an array [lo..hi] of the given size not initialized */
char *Yvector_alloc( lo, hi, size MEM_DEBUG1 )
INT size, lo, hi ;
MEM_DEBUG2
{
    char *array_return ;

    array_return = (char *) Ysafe_malloc((unsigned) (hi-lo+1)*size MEM_DEBUG1 ) ;
    if( array_return ){
	return( array_return - size * lo ) ;
    }
    return( NIL(char *) ) ;

} /* end Yvector_alloc */

/* ALLOCATE an array [lo..hi] of the given size initialized to zero */
char *Yvector_calloc( lo, hi, size MEM_DEBUG1 )
INT size, lo, hi ;
MEM_DEBUG2
{
    char *array_return ;

    array_return = (char *) Ysafe_calloc((unsigned) (hi-lo+1),size MEM_DEBUG1 ) ;
    if( array_return ){
	return( array_return - size * lo ) ;
    }
    return( NIL(char *) ) ;

} /* end Yvector_calloc */

/* REALLOCATE an array [lo..hi] of the given size no initialization */
char *Yvector_realloc( array_orig, lo, hi, size MEM_DEBUG1 )
VOIDPTR array_orig ;
INT size, lo, hi ;
MEM_DEBUG2
{
    char *adj_array ;          /* put back the offset */
    char *array_return ;       /* the new offset */

    adj_array = ((char *) array_orig) + lo * size ;
    array_return = (char *) 
	Ysafe_realloc( adj_array, (unsigned) (hi-lo+1)*size MEM_DEBUG1 ) ;
    if( array_return ){
	return( array_return - size * lo ) ;
    }
    return( NIL(char *) ) ;

} /* end Yvector_realloc */

VOID Yvector_free( array, lo, size MEM_DEBUG1 )
VOIDPTR array ;
INT lo, size ;
MEM_DEBUG2
{
    Ysafe_free( ((char *)array) + lo * size MEM_DEBUG1 ) ;
} /* end Yvector_free */

/* ************************* TEST ROUTINES ******************************** */
#ifdef TEST 

typedef struct {
    INT bogus_dude ;
    DOUBLE narly ;
    char *awesome ;
} MEMDATA, *MEMDATAPTR ;

main()
{
    MEMDATAPTR array ;
    MEMDATAPTR vector ;
    char *string, *Ystrclone() ;

    YdebugMemory( TRUE ) ;
    Yinit_memsize( 1024 ) ;

    /* allocate an array 0..9 */
    array = YMALLOC( 10, MEMDATA ) ;
    fprintf( stderr, "Memory size :%d\n", YgetCurMemUse() ) ;
    /* allocate an array 1..10 */
    vector = YVECTOR_MALLOC( 1, 10, MEMDATA ) ;
    fprintf( stderr, "Memory size :%d\n", YgetCurMemUse() ) ;
    /* clone this string */
    string = Ystrclone( "Droog" ) ;
    fprintf( stderr, "Memory size :%d\n", YgetCurMemUse() ) ;
    /* look at mem.data at this point */
    Ydump_mem() ;
    /* free all the memory */
    YFREE( array ) ;
    YFREE( string ) ;
    YVECTOR_FREE( vector, 1 ) ;
    fprintf( stderr, "Memory size :%d\n", YgetCurMemUse() ) ;
    fprintf( stderr, "Memory max size :%d\n", YgetMaxMemUse() ) ;
    Ydump_mem() ;

    exit(0) ;
    
}/* end main() */

#endif /* TEST */

