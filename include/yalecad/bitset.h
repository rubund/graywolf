#ifndef YBITSET_H
#define YBITSET_H

#define BITSET_GET( array, index )    (array[index>>5] & (1 <<(index & 0x1F)))
#define BITSET_SET( array, index )    (array[index>>5] |= (1 << (index & 0x1F)) )
#define BITSET_RESET( array, i )      (array[i>>5] &= (~(1 << (i & 0x1F))) )
#define BITSET_ALLOC( num )           ( YCALLOC( (num>>5) + 1, UNSIGNED_INT) )
#define BITSET_ARRYSIZE( num )        ( (num>>5) + 1 )

#ifdef TEST

main()
{

    unsigned int *array ;
    int i ;
    int max ;

    max = 32 ;

    array = BITSET_ALLOC( max ) ;

    for( i = 0 ; i <= max ; i++ ){
	if( i == 1 || i == 2 || i == 5 || i == 11 || i == 13 || i == 69 ){
	    BITSET_SET( array, i ) ;
	} else {
	    BITSET_RESET( array, i ) ;
	}
    }

    for( i = 0 ; i <= max ; i++ ){
	if( BITSET_GET( array, i ) ){
	    fprintf( stderr, "i:%d T\n", i ) ;
	} else {
	    fprintf( stderr, "i:%d F\n", i ) ;
	}
    }
}

#endif /* TEST */

#endif /* YBITSET_H */
