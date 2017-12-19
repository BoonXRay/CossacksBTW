#ifndef NEWAI_H
#define NEWAI_H

#include <cstdint>


//01-wood(trees>2)
//02-stone(stones>5)
//04-full empty
//08-empty from locking
//16-plane zone
//32-high plate
extern uint8_t * InfoMap;
//01-sklad
//02-farm
//04-tower
//08-building
//16-melnica
#define CB_Sklad	1
#define CB_Farm		2
#define CB_Tower	4
#define CB_Building 8
#define CB_Melnica  16
#define CB_Port     32
extern uint8_t * CantBuild;

extern uint8_t NPORTS;
extern short PORTSX[32];
extern short PORTSY[32];

extern int* MineList;
extern uint16_t NMines;
extern uint16_t MaxMine;
void CreateInfoMap();
void RenewInfoMap(int x,int y);
typedef bool SearchFunction(int xx,int yy);
bool CheckMelnica(int x,int y);
bool CheckStoneSklad(int x,int y);
bool CheckWoodSklad(int x,int y);
bool CheckStoneWoodSklad(int x,int y);
bool CheckBuilding(int x,int y);
bool CheckPort(int x,int y);
bool SearchPlace(int* xx,int* yy,SearchFunction* SFN,int r);
bool FindCenter(int* xx,int *yy, uint8_t NI);
void SetUnusable(int x,int y, uint8_t Mask);

bool SearchTowerPlace( int* xx1, int* yy1, SearchFunction* SFN, int r, int xc, int yc, int xe, int ye );
bool CheckTower( int x, int y );

#endif // NEWAI_H
