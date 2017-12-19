#ifndef ZBUFFER_H
#define ZBUFFER_H

#include <cstdint>

class OneObject;

void AddPoint( short XL, short YL, short x, short y, OneObject * OB, uint16_t FileID, uint16_t SpriteID, int Param1, int Param2 );
void AddLine( short X1, short Y1, short X2, short Y2, short x, short y, OneObject * OB, uint16_t FileID, uint16_t SpriteID, int Param1, int Param2 );
void AddHiPoint( short x, short y, OneObject * OB, uint16_t FileID, uint16_t SpriteID, int Param1, int Param2 );
void AddLoPoint( short x, short y, OneObject * OB, uint16_t FileID, uint16_t SpriteID, int Param1, int Param2 );
void AddSuperLoPoint( short x, short y, OneObject * OB, uint16_t FileID, uint16_t SpriteID, int Param1, int Param2 );
void AddOptPoint( uint8_t Method, short XL, short YL, short x, short y, OneObject * OB, uint16_t FileID, uint16_t Sprite, int Options );
void AddOptLine( short X1, short Y1, short X2, short Y2, short x, short y, OneObject * OB, uint16_t FileID, uint16_t SpriteID, int Options );
void ShowZBuffer();
void ClearZBuffer();
#define ZBF_NORMAL	0
#define ZBF_LO		1
#define ZBF_HI		2
extern uint8_t CurDrawNation;

#endif // ZBUFFER_H
