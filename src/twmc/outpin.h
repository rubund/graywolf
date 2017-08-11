#ifndef H_OUTPIN
#define H_OUTPIN
void outpin();
void setPadMacroNum( int side, int cellnum );
void output_matches();
int get_circuit_type();
PINBOXPTR findTerminal( char *pinName, int cell );
#endif
