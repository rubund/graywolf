#ifndef H_OVERLAP_H
#define H_OVERLAP_H
void new_old(int c);
void old_assgnto_new1( int alobin , int ahibin , int anewlobin , int anewhibin );
void sub_penal( int startx , int endx , int block , int LoBin , int HiBin );
void add_penal( int startx , int endx , int block , int LoBin , int HiBin );
void term_newpos( PINBOXPTR antrmptr  , int xcenter , int ycenter , int newaor );
void new_assgnto_old1( int alobin , int ahibin , int anewlobin , int anewhibin );
void old_assgnto_new2( int a1lobin , int a1hibin , int a2lobin , int a2hibin , int b1lobin , int b1hibin , int b2lobin , int b2hibin );
void new_assgnto_old2( int a1lobin , int a1hibin , int a2lobin , int a2hibin , int b1lobin , int b1hibin , int b2lobin , int b2hibin );
#endif
