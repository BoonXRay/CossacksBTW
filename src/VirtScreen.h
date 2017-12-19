#ifndef VIRTSCREEN_H
#define VIRTSCREEN_H

#include <cstdint>

#pragma pack( push, 1 )

class VirtualScreen
{
public:
	int RealVLx;
	int RealVLy;

	int CellSX;
	int CellSY;
	int CellNX;
	int CellNY;
	int NCells;
	int ShiftsPerCellX;
	int ShiftsPerCellY;
    uint8_t * CellQuotX;
    uint8_t * CellQuotY;
    uint8_t * CellFlags;
	int* TriangMap;
	int* LoTriMap;
	int MaxTMX;
	int MaxTMY;
	int Lx;
	int Ly;
    uint8_t * MarkedX;
	bool Grids;
    uint8_t * VirtualScreenPointer;
	VirtualScreen();
	~VirtualScreen();
	void CopyVSPart( int vx, int vy, int sx, int sy, int SizeX, int SizeY );
	void CopyVSToScreen();
    void CreateVerticalTrianglesMapping( int VertSet );
	void RenderVerticalSet( int QuotX, int QuotY, int cx, int cy, int cly );
	void RenderVSPart( int QuotX, int QuotY, int cx, int cy, int clx, int cly );
	void RefreshSurface();
	int  ShowLimitedSector( int i, bool Mode3D, int HiLine, int LoLine, int QuotX, int QuotY );
	void Execute();
    void RefreshTriangle( int i );
	void RefreshScreen();
    void SetSize( int Lx, int Ly );
	void CheckVLINE( int Vert );
};

#pragma pack(pop)

extern VirtualScreen SVSC;

#endif // VIRTSCREEN_H
