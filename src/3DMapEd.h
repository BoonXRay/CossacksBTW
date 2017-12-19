#ifndef _3DMAPED_H
#define _3DMAPED_H

#include <cstdint>

struct BlockCell
{
    uint16_t x;
    uint16_t y;
};

class BlockBars
{
    public:
        int NBars = 0;
        int MaxBars = 0;
        BlockCell * BC  = nullptr;

        ~BlockBars();
        void Clear();
        bool Add( uint16_t x, uint16_t y );
        bool FastAdd( uint16_t x, uint16_t y );
        bool Delete( uint16_t x, uint16_t y );
};
void AddLockBar( uint16_t x, uint16_t y );
void FastAddLockBar( uint16_t x, uint16_t y );
void AddUnLockbar( uint16_t x, uint16_t y );
void FastAddUnLockBar( uint16_t x, uint16_t y );

void AddDefaultBar( uint16_t x, uint16_t y );
void SetTexture( int Vert, int nm );
void SetTexturedRound( int x, int y, int r, uint8_t Tex );
int ConvScrY( int x, int y );
extern int NCurves;
void AddPointToCurve( int x, int y, bool Final, uint8_t Type );
void MarkPointToDraw( int i );

extern int MaxWX;
extern int MaxWY;

void CreateMiniMapPart( int x0, int y0, int x1, int y1 );
void CreateMapPreview( uint8_t * Data, int Lx, int Ly );
void CreateMiniMap();

void ShowPen( int x, int y, int r, uint8_t c );
void AddHi( int x, int y1, int r, int h );
void AddHiPlanar( int x, int y1, int r, int h );
void CreatePlane( int x, int y1, int r );
void PlanarHi( int x, int y1, int r );
void AverageHi( int x, int y1, int r );
void RandomHi( int x, int y1, int r, int ms, int fnd );


extern BlockBars LockBars;
extern BlockBars UnLockBars;
void ShowLockBars();

void CreateLandLocking( int TAlp, bool ForVision );
void CreateTotalLocking();

extern int LastMx;
extern int LastMy;
extern int PlatoHi;
extern int SurfType;

extern int ADDRES[8];


inline bool CheckCurve() noexcept
{
    return NCurves != 0;
}

inline void ClearCurve() noexcept
{
    NCurves = 0;
}

void SoftRegion();

bool CheckCurveLinked( int x, int y );
void DrawCurves();

class PaintHills
{
    public:
        uint16_t NLayers;
        uint16_t TexAmount[32];
        uint16_t Texs[32][8];
        uint16_t TexStartHi[32];
        uint16_t TexEndHi[32];
        uint16_t TexStartAng[32];
        uint16_t TexEndAng[32];
        uint8_t CondType[32];
        uint8_t Param1[32];
        uint8_t Param2[32];
        int NHiSections;
        int HiSect[8];
};

void PaintAllMap( uint8_t * VertHi, uint8_t * VertType, PaintHills * PHL, int NTypes );

void GenerateRandomRoad( int idx );
bool GenerateStartUnits( char* NationID, uint8_t NI, int x, int y, int GenIndex );


#endif // _3DMAPED_H
