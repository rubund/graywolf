/* ----------------------------------------------------------------- 
FILE:	    23tree.h                                       
CONTENTS:   definitions for two three tree data structure.
DATE:	    Feb 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) 23tree.h version 1.1 3/3/90" ;
***************************************************************** */
#ifndef TREE23_H
#define TREE23_H

#include <stdio.h>

typedef struct plist {
    int property ;
    struct plist *pnext  ;
} 
PLIST, *PLISTPTR ;

typedef struct tnode {
    int value ;
    int nsons ;
    struct tnode *son1  ;
    struct tnode *son2  ;
    struct tnode *son3  ;
    struct tnode *son4  ;
    struct tnode *father;
    int Lval  ;
    int Mval  ;
    PLISTPTR plist ;
} 
TNODE, *TNODEPTR ;

extern TNODEPTR tsearch() ;
extern TNODEPTR tdsearch() ;
extern TNODEPTR tfind() ;
extern PLISTPTR pfind() ;
extern PLISTPTR tplist() ;

#endif /* TREE23_H */
