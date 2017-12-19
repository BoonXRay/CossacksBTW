#ifndef TOPOGRAF_H
#define TOPOGRAF_H

#include <cstdint>

#include "LoadSave.h"

#pragma pack( push, 1 )

struct Radio{
	char* xi;
	char* yi;
    uint16_t N;
};
#define RRad 90
extern Radio Rarr[RRad];

bool GetTCStatus( int x, int y );
void CreateAreas();
void EraseAreas();
extern uint16_t * TopRef;

#define SI_Center	1
#define SI_Meln		2
#define SI_Sklad	4
#define SI_Shahta	8
#define SI_Tower	16
#define SI_ArtDepo	32
#define SI_House	64
#define SI_Barrack  128
#define SI_Farm		256
#define SI_Other	512
#define SI_Near     1024
#define SI_FarLand  8192
#define SI_Okrug    16384
#define SI_Central  32768

struct StrategyInfo
{
public:
    uint16_t BuildInfo;
    uint8_t NPeasants;
    uint8_t NShortRange;
    uint8_t NLongRange;
    uint8_t NMortir;
    uint8_t NTowers;
    uint8_t NPushek;
};

struct Area
{
	short x;
	short y;
    uint8_t  Importance;
    uint8_t NTrees;
    uint8_t NStones;
	//01-gold
	//02-iron
	//04-coal
    uint16_t   NMines;
    uint16_t * MinesIdx;
    uint16_t  NLinks;
    uint16_t * Link;
    uint16_t MaxLink;
	StrategyInfo SINF[8];
};

struct MediaTop
{
    uint16_t * MotionLinks;
    uint16_t * LinksDist;
    uint16_t * TopRef;
	int NAreas;
	Area* TopMap;
};

#pragma pack( pop )

extern MediaTop GTOP[2];
extern Area* TopMap;
extern int   NAreas;
extern int MaxArea;
extern uint16_t * MotionLinks;
extern uint16_t * LinksDist;
void CreateStrategyInfo();
//---------
#define WTopLx (TopLx)
#define WTopSH (TopSH)
#define WCLSH 2
#define WCLLX 4
extern uint16_t * WTopRef;
extern Area * WTopMap;
extern int   WNAreas;
extern int WMaxArea;
extern uint16_t * WMotionLinks;
extern uint16_t * WLinksDist;
extern int NeedITR;

extern bool NeedProcessTop;
extern bool WasOnlyOpen;

extern uint16_t NChAreas;
extern uint16_t MaxChAreas;
extern uint16_t  * ChAreas;

//procesing variables
extern int CurIStart;
extern int TmpChanges;
extern uint16_t * TmpMLinks;
extern uint16_t * TmpMDist;
extern int MinChX;
extern int MaxChX;
extern int MinChY;
extern int MaxChY;

//---------
void InitTopChange();
void ClearTopChange();
void StopDynamicalTopology();
void DynamicalLockTopCell(int x,int y);
void DynamicalUnLockTopCell(int x,int y);
void ProcessDynamicalTopology();

__forceinline word SafeTopRef( int x, int y )
{
    if ( x >= 0 && y >= 0 && x < TopLx&&y < TopLx )
    {
        return TopRef[x + ( y << TopSH )];
    }
    else
    {
        return 0xFFFF;
    }
}

__forceinline void SafeSetTopRef( int x, int y, word Val )
{
    if ( x >= 0 && y >= 0 && x < TopLx&&y < TopLx )
    {
        TopRef[x + ( y << TopSH )] = Val;
    }
}

void ClearLinkInfo();

void CreateRadio();
void CreateWTopMap();

#endif // TOPOGRAF_H
