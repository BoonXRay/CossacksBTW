#ifndef _3DBARS_H
#define _3DBARS_H

#include <cstdint>

#pragma pack( push, 1 )

struct OneBar
{
    int x0, y0, x1, y1;
    short MinBX, MinBY, MaxBX, MaxBY;
    short H;
    uint16_t Owner;
    uint16_t ID;
};

#pragma pack(pop)

void DrawBar3D( OneBar * ONB );
extern OneBar * OBARS[8192];
extern int NBars;
#define B3SHIFT 8
extern int B3SX;
extern int B3SY;
extern int B3SZ;

extern uint16_t ** Obj3Map;
extern uint16_t  * NObj3;
extern uint16_t OWNER;

void InitObjs3();
void ClearObjs3();
uint16_t Add3DBar( int X0, int Y0, int X1, int Y1, short H, int ID, uint16_t Owner );
void Delete3DBar( uint16_t ID );
int GetBar3DHeight( int x, int y );
int GetBar3DOwner( int x, int y );

void ShowAllBars();

#endif // _3DBARS_H
