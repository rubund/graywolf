BEGIN { state = 1; pstate = 0 }
$1 == "%%" { pstate = 1; next }
$1 == "{" { state = 0 }
$1 == "}" { state = 1; next }
$1 == "#include" && pstate == 1 { exit }
state == 1 && pstate == 1 { 
    if( $2 == ":" ){
	printf( "\n%s\t: ", $1 ) ;
	i = 3 ;
	while( i <= NF ){
	    printf( "%s ", $i ) ;
	    i++ ;
	}
	printf( "\n" ) ;

    } else if( $1 == "|" ){
	if( $NF == "error" || $(NF-1) == "error" ){
	    next ;
	}
	printf( "\t| " ) ;
	i = 2 ;
	while( i <= NF ){
	    printf( "%s ", $i ) ;
	    i++ ;
	}
	printf( "\n" ) ;
    } else if( $NF > 0 ){
	i = 1 ;
	printf( "\t\t" ) ;
	while( i <= NF ){
	    printf( "%s ", $i ) ;
	    i++ ;
	}
	printf( "\n" ) ;
    }
}
END { }
