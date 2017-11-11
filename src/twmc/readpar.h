#ifndef H_READPAR
#define H_READPAR
extern BOOL restartG ;
extern BOOL doPartitionG ;

extern int track_spacingXG;
extern int track_spacingYG;

void init_read_par();
void readparam();
void process_readpar();
void err_msg(char* keyword);
void readpar();
void init_read_par();
void readparam( int parfile );
void process_readpar();
void err_msg(char *keyword);
void init_read_par();
void readparam();
void process_readpar();
void err_msg(char* keyword);
BOOL sc_output();
#endif
