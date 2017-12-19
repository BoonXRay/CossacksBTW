#ifndef _3DGRAPH_H
#define _3DGRAPH_H

#include <cstdint>

#include "Fonts.h"

//----------------------NEW MAP WITH CASHING--------------------
//
//
//    /|\ /|\ /|\ /|\|
//   |/|\|/|\|/|\|/|\|
//   |/|\|/|\|/|\|/|\|
//
//
//
//
//

#define TEX_ALWAYS_LAND_LOCK		1
#define TEX_ALWAYS_LAND_UNLOCK		2
#define TEX_ALWAYS_WATER_UNLOCK		4
#define TEX_PLAIN                   8
#define TEX_HARD                    16
#define TEX_HARDLIGHT               32
#define TEX_NOLIGHT                 64
#define TEX_NORMALPUT               128


#define TriUnit		16 //32 //24
#define TriUnit34	12 //24 //18
//void ShowSector(int i,bool Mode3D);
int GetLighting( int i );
void SetHi( int i, int h );
int GetHi( int i );
int GetTriX( int i );
int GetTriY( int i );

extern int MaxTex;
extern bool Mode3D;
extern uint8_t * tex1;
extern uint8_t TexColors[256];

extern short* THMap;// Map of heights in vertices
extern uint8_t * TexMap;//Map of textures in vertices
extern uint8_t * SectMap;//Map of sections on lines

extern RLCTable SimpleMask;
extern RLCTable SimpleMaskA;
extern RLCTable SimpleMaskB;
extern RLCTable SimpleMaskC;
extern RLCTable SimpleMaskD;

extern uint8_t ExtTex[ 256 ][ 4 ];
extern uint16_t TexFlags[ 256 ];
extern uint8_t TexMedia[ 256 ];
extern uint8_t TileMap[ 256 ];
extern int TEXARR[ 8 ];

extern short randoma[ 8192 ];

extern int VertLx;
extern int MaxPointIndex;
extern int MaxLineIndex;
extern int MaxTH;
extern int MaxSector;
extern int VertInLine;
extern int SectInLine;


inline int AddTHMap( int i )
{
    return ( TexFlags[ TexMap[ i ] ] & 8 ? 0 : uint16_t( randoma[ uint16_t( i % 8133 ) ] ) & 7 );
}

inline uint8_t SECTMAP( uint16_t i )
{
    return SectMap ? SectMap[i] : ( uint16_t( randoma[ i & 8191 ] ) % 3 );
}

int srando();

typedef int HiCallback( int x, int y, int Hig );
void CreateEffect( int x, int y, int r, HiCallback * HCB );
void DirectRenderTriangle64( int xs1, int ys1,
                             int xs2, int ys2,
                             int xs3, int ys3,
                             int xb1, int yb1,
                             int xb2, int yb2,
                             int xb3, int yb3,
                             int f1, int f2, int f3,
                             uint8_t * Dest, uint8_t * Bitm,
                             int StartLine, int EndLine, int ScanSize );
inline int GetVTex(int i)
{
    return TexMap[i];
}

void Init3DMapSystem();
void ClearRender();
void Reset3D();
void Loadtextures();
void TestTriangle();

struct VertOver
{
    short xs;
    short ys;
    short xz;
    int   v;
    bool Visible;
    uint8_t * Data;
};

class OverTriangle
{
public:
    VertOver** TRIANG;
    uint16_t *      NTRIANG;
    uint8_t ** Buffer;
    int MaxElm;
    int CurElm;

    OverTriangle();
    ~OverTriangle();

    void Clear();
    void AddTriangle(int i);
    void CreateFullMap();
    void Show();
    void ShowElement(int Sq, int idx);
};

extern OverTriangle OTRI;

//-----------------------------------------------------------------------------------//
void ShowTrianglesOwerUnits();
void DrawTriangleElement( int sq, int idx );
void ClearTrianglesSystem();
void CreateTrianglesSystem();
void SetTrianglesState(int xc, int yc, short* xi, short* yi, int NP, bool State);

inline int prp34(int i)
{
    return i >> 1;
}

inline void SetTHMap( int v, int Val )
{
    if( v >= 0 && v < MaxPointIndex )
    {
        THMap[v] = Val;
    }
}
inline int GetTHMap( int v )
{
    if( v >= 0 && v < MaxPointIndex )
    {
        return THMap[v];
    }
    else return 0;
}
inline void SetTexMap( int v, uint8_t t )
{
    if( v >= 0 && v < MaxPointIndex )
    {
        TexMap[v] = t;
    }
}
inline int GetTexMap( int v )
{
    if( v >= 0 && v < MaxPointIndex )
    {
        return TexMap[v];
    }
    else return 0;
}
inline int GetSectMap( int v )
{
    if( v >= 0 && v < MaxLineIndex )
    {
        return SECTMAP( v );
    }
    else return 0;
}
inline void SetSectMap( int v, uint8_t s )
{
    if( SectMap && v >= 0 && v < MaxLineIndex )
    {
        SectMap[v] = s;
    }
}

extern int LightDX;
extern int LightDY;
extern int LightDZ;

void SetLight(int Ldx,int Ldy,int Ldz);
void InitRenderMap();

#endif // _3DGRAPH_H
