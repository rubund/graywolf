#ifndef H_STEINER
#define H_STEINER
#include "groute.h"

extern int Max_numPinsG ;
extern int *add_st_flagG ;
extern int enough_built_in_feedG ;

extern PINBOXPTR *steinerHeadG;
extern SEGBOXPTR *netsegHeadG;

void steiner();
void redo_steiner();
void make_net_Tree( PINBOXPTR startptr );
#endif
