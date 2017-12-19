#ifndef EXPLORE_H
#define EXPLORE_H

#include "IntExplorer.h"

extern sicExplorer SXP[8];

extern int IBOR0;
extern int IBOR2;

void InitSXP();
void RunSXP( int Index, const char * home, int x, int y, int x1, int y1 );

class DialogsSystem;
void ProcessSXP( int Index, DialogsSystem* DSS );

void Replace( char** str, const char* src, char* dst, int& MaxL );
DWORD GetTableHash( OneSXPTable* TB, int Line );

void SXP_StepBack( int Index );
void SXP_StepForw( int Index );
void SXP_Refresh( int Index );

void SXP_SetVar( int Index, const char* Name, const char* value );
char* SXP_GetVar( int Index, const char* Name );
void OpenRef( int Index, const char* home );
void ResizeSXP( int Index, int x, int y, int x1, int y1 );

char* GetAccessKey( int Index );
void SetAccessKey( int Index, const char* Accesskey );

int ReadNumber( char* s, int* L, int vmax );

#endif // EXPLORE_H
