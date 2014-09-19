#ifndef YRADIXSORT_H
#define YRADIXSORT_H

#include <yalecad/base.h>

extern int Yradixsort7( P7(VOIDPTR *l1,INT n,UNSIGNED_INT endchar,
    VOIDPTR *tab, INT indexstart,char *(*ralloc)(), VOID (*rfree)() ) ) ;

extern int Yradixsort5( P5(VOIDPTR *l1,INT n,UNSIGNED_INT endchar,VOIDPTR *tab,
    INT indexstart) ) ;

extern int Yradixsort_pref( P2(VOIDPTR *l1,INT n) ) ;

extern int Yradixsort4( P4(VOIDPTR *l1,INT n,UNSIGNED_INT endchar,VOIDPTR *tab));

extern int Yradixsort( P4(VOIDPTR *l1,INT n,VOIDPTR *tab,UNSIGNED_INT endchar)) ;

extern int Yradixsort3( P3(VOIDPTR *l1,INT n, UNSIGNED_INT endchar) ) ;

extern char *Yradix_prefix( P2(char *buffer, INT num ) ) ;

extern int Yradix_number( P1(char *buffer ) ) ;

extern char *Yradix_suffix( P1(char *buffer) ) ;

extern char *Yradix_pref_clone( P1(char *buffer) ) ;

#endif

