#ifndef H_OUTPUT
#define H_OUTPUT
void addNet( char *signal );
void addCell( int celltype, char *cellname );
void set_pinname( char *pinname );
void set_bbox( int left, int right, int bottom, int top );
void add_instance();
void start_pt( int x, int y );
void processCorners();
void add_pt( int x, int y );
void addEquiv();
void addUnEquiv();
void output();
void init();

extern char *cktNameG ;
#endif
