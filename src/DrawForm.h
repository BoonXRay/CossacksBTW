#ifndef DRAWFORM_H
#define DRAWFORM_H

struct TempWindow
{
	int WindX;
	int WindY;
	int WindLx;
	int WindLy;
	int WindX1;
	int WindY1;
	int ScrWidth;
};
void PushWindow( TempWindow* W );
void PopWindow( TempWindow* W );
void IntersectWindows( int x0, int y0, int x1, int y1 );

struct Corners
{
	int CLU, CRU, CLD, CRD;
	int LL, LR, LU, LD;
};

void DrawRect( int x0, int y0, int x1, int y1, Corners* CR, int GP_File );
void DrawHeader( int xc, int yc, int Lx, int GP_File, int Frame );
void DrawVScroller( int x, int y, int Ly, int GP_File, int Up, int Down, int Center, int Ncenter );

void DrawStdRect( int x0, int y0, int x1, int y1 );
void DrawStdBar( int x0, int y0, int x1, int y1 );
void DrawStdBar1( int x0, int y0, int x1, int y1 );
void DrawLeftRect( int x0, int y0, int x1, int y1 );
void DrawRightRect( int x0, int y0, int x1, int y1 );
void DrawDoubleTable2( int x0, int y0, int x1, int y1, int ym );
void DrawScrollDoubleTable( int x0, int y0, int x1, int y1, int ym );
void DrawScrollDoubleTable1( int x0, int y0, int x1, int y1, int ym );
void DrawScrollDoubleTableF( int x0, int y0, int x1, int y1, int ym );
void DrawPaperPanel( int x0, int y0, int x1, int y1 );
void DrawPaperPanelShadow( int x0, int y0, int x1, int y1 );
void DrawHintPattern( int x0, int y0, int x1, int y1 );
void DrawStatTable( int x0, int y0, int Lx, int GraphLy, int LineLy, int NLines );
void DrawStHeader( int x, int y, int Lx );
void DrawStatTable1( int x0, int y0, int x1, int y1, int xm );

void DrawHdrTable( int x0, int y0, int x1, int y1 );

void DrawStdRect2( int x0, int y0, int x1, int y1, int GP );
__declspec( dllexport ) void DrawStdBar2( int x0, int y0, int x1, int y1, int GP );

extern int BordGP;
extern int BordGP2;
void DrawFilledRect1( int x0, int y0, int x1, int y1, Corners* CR, int GP_File, int StartFill, int NFill );
__declspec( dllexport ) void DrawFilledRect3( int x0, int y0, int x1, int y1, Corners* CR, int GP_File, int StartFill, int NFill );
__declspec( dllexport ) void DrawRect3( int x0, int y0, int x1, int y1, Corners* CR, int GP_File );

void LoadBorders();

#endif // DRAWFORM_H
