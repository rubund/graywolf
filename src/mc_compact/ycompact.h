#ifndef YCOMPACT_H
#define YCOMPACT_H
int longestyPath( BOOL find_path );
void formyEdge( int fromNode, int toNode ) ;
void inityPicket( );
void update_ypicket( int i, PICKETPTR lowerLimit, PICKETPTR upperLimit );
#endif
