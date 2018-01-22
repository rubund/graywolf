void check_validbound();
void wire_boundary1();
void wire_boundary2( int c , NETBOXPTR netptr );
void init_unet();
void check_validbound(NETBOXPTR netptr, PINBOXPTR termptr, PINBOXPTR nextptr);
void wire_boundary2( int c , NETBOXPTR netptr );
void wire_boundary1( NETBOXPTR netptr );
int unet( PINBOXPTR antrmptr );
int unet2( PINBOXPTR antrmptr, PINBOXPTR bntrmptr );
