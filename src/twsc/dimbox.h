#ifndef H_DIMBOX
#define H_DIMBOX
#include "standard.h"
void new_dbox( PINBOXPTR antrmptr , int *costptr );
void dbox_pos( PINBOXPTR antrmptr  ) ;
void new_dbox2( PINBOXPTR antrmptr , PINBOXPTR bntrmptr , int *costptr  );
void wire_boundary1( DBOXPTR dimptr );
void check_validbound( DBOXPTR dimptr , PINBOXPTR termptr , PINBOXPTR nextptr );
void wire_boundary2( int c , DBOXPTR dimptr );
void init_dbox_pos_swap( PINBOXPTR antrmptr );
void dbox_pos_swap( PINBOXPTR antrmptr  ) ;
int init_dimbox();
#endif
