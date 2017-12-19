#ifndef NEWMON_H
#define NEWMON_H

#include <cmath>
#include <cstdint>

#include "GP_Draw.h"

class NewMonster;

extern uint16_t NNewMon;
extern NewMonster NewMon[512];
extern int CurrentCursorGP;
extern uint16_t RLCNSpr[MaxGPIdx];

extern uint8_t KeyCodes[512][2];
#define NKEYS 68
extern const char* KeyNames[NKEYS];
extern const uint8_t ScanKeys[NKEYS];

extern int AnmCursorGP;

extern uint8_t * MCount;//amount of monsters in 4x4 cell; 16384 elements
extern uint8_t * TmpMC; //amount of monsters in 4x4 cell,

extern int PortBuiX, PortBuiY;

extern uint16_t * BLDList;

extern bool GroundBox;

extern int ExplMedia;

extern uint8_t NatRefTBL[8];

class NewAnimation;
NewAnimation* GetNewAnimationByName(const char* Name);

class OneObject;
void MakeOrderSound( OneObject* OB, uint8_t SMask );

int GetRAngle(int dx, int dy, int Angle);
int GetLAngle( int dx, int dy, int Angle );

void IncPar( const char* name, int line, const char* Sect );

class GeneralObject;
bool CreateGOByName(GeneralObject* GO, char* name, char* newName);

int CheckPt(int x, int y);
bool CheckBar(int x, int y, int Lx, int Ly);

bool CheckBuildingsForWalls( int x, int y );
bool CheckCostHint( uint8_t NI, uint16_t NIndex );

int GetTotalUnits();

//returns (2|x| + |y|) / 2 if x > y
//returns (2|y| + |x|) / 2 if y > x
inline int Norma(int x, int y)
{
	// BoonXRay 13.08.2017
	//__asm
	//{
	//	mov		eax, x
	//	cmp		eax, 0
	//	jge		ggg1
	//	neg		eax
	//	ggg1 : mov		ebx, y
	//		   cmp		ebx, 0
	//		   jge		ggg2
	//		   neg		ebx
	//		   ggg2 : mov		ecx, eax
	//				  cmp		eax, ebx
	//				  ja		ggg3
	//				  mov		ecx, ebx
	//				  ggg3 : add		ecx, eax
	//						 add		ecx, ebx
	//						 shr		ecx, 1
	//						 mov		eax, ecx
	//}
	x = abs(x);
	y = abs(y);
	if (x > y) return ( 2 * x + y) / 2;
	return (2 * y + x) / 2;
}

void TryToStand(OneObject* OB, bool rest);
void BSetPt(int x, int y);
void BClrPt(int x, int y);
void BSetBar(int x, int y, int Lx);
void BClrBar(int x, int y, int Lx);
//FindPoint flags:
#define FP_NEAREST_POINT  1
#define FP_UNLOCKED_POINT 2
#define FP_FIND_WORKPOINT 4
#define FP_FIND_DAMPOINT  8
#define FP_CONCENTRATION  16
void MakeRoundDamage(int x, int y, int r, uint16_t Damage, OneObject* Sender, uint16_t Attr);
//Adding to visual options:
#define AV_NORMAL      0x00000000
#define AV_PULSING     0x00000001
#define AV_TRANSPARENT 0x00000002
#define AV_PALETTE     0x00000003
#define AV_GRADIENT    0x00000004
//---
#define AV_RED         0x00000010
#define AV_WHITE       0x00000020
#define AV_DARK        0x00000030
//---
#define AV_SHADOWONLY    ((0x20<<9)+256)
#define AV_WITHOUTSHADOW (((0xFFFF-0x20)<<9)+256)
//bool CheckCostNM(byte NI,NewMonster* NM);
//bool ApplyCostNM(byte NI,NewMonster* NM);
//bool CheckCost(byte NI,GeneralObject* GO);
//bool ApplyCost(byte NI,GeneralObject* GO);
bool CheckCost(uint8_t NI, uint16_t NIndex);
bool ApplyCost(uint8_t NI, uint16_t NIndex);

void ClearMaps();

uint16_t GetDir( int dx, int dy );

void AttackObjLink( OneObject* OBJ );

void DosToWin( char * );


void NewMonsterSmartSendToLink( OneObject * OBJ );
void NewMonsterSendToLink( OneObject * OB );
void AI_AttackPointLink( OneObject * OBJ );
void BuildObjLink( OneObject* OBJ );
void WaterAttackLink( OneObject* OBJ );

int CheckCreationAbility( uint8_t NI, NewMonster* NM, int* x2i, int* y2i, uint16_t * BLD, int NBLD );

void MoveAway( int x, int y );

void NewMonsterPreciseSendToLink( OneObject* OB );

int FindSuperSmartBestPosition( OneObject* OB, int* cx, int* cy, int dx, int dy, uint16_t Top, uint8_t LTP );

void GetUnitCost( uint8_t NI, uint16_t NIndex, int* Cost );
void GetUnitCost( uint8_t NI, uint16_t NIndex, int* Cost, uint16_t Power );

void FindUnitPosition( int* x, int *y, NewMonster* NM );
bool PInside( int x, int y, int x1, int y1, int xp, int yp );

int GetIconByName( char* Name );

int CheckPointForDamageAbility( OneObject* OBJ, int x, int y, int z );

int GetWTopology( int x, int y, uint8_t LTP );

void __stdcall CDGINIT_INIT1();

void DoNormalTBL();

void DeleteBlockLink( OneObject* OBJ );
void SetUnlimitedLink( OneObject* OB );
void ClearUnlimitedLink( OneObject* OB );
void RotateShipAndDieLink( OneObject* OBJ );
void NewAttackPointLink( OneObject* OBJ );
void MakeOneShotLink( OneObject* OBJ );

void CreateGround( OneObject* G );

void SetMonstersInCells();

void LoadNewAimations();
void LoadAllNewMonsters();
void InitNewMonstersSystem();
void ProcessNewMonsters();
void HandleShar( Nation* NT );
void ProcessGuard();

void LShow();
void ShowNewMonsters();

void GetDiscreta( int* x, int* y, int val );
void ProcessSelectedTower();
int CheckSmartCreationAbility( uint8_t NI, NewMonster* NM, int* x2i, int* y2i );
void DrawShar( Nation* NT );
uint16_t GetNewEnemy( int xr, int yr, uint8_t NI );

uint16_t AdvancedGetNewEnemy( int rex, int rey, int xr, int yr, uint8_t NI );
uint16_t AdvancedGetNewFriend( int rex, int rey, int xr, int yr, uint8_t NI );

int TestCapture( OneObject* OBJ );

int SmartCreationUnit( uint8_t NI, int NIndex, int x, int y );

int GetAmountOfProtectors( OneObject* OBJ );
uint16_t GetNearestDefender( OneObject* OB );
uint16_t SearchVictim( OneObject* OBJ, int R0, int R1 );

int EnumUnitsInRound( int x, int y, int r, uint16_t Type, uint8_t Nation );

void RunLeftVeslo( OneObject* OB, bool State );
void RunRightVeslo( OneObject* OB, bool State );

typedef  bool CHOBJ( OneObject* OB, int N );
uint16_t GoodSelectNewMonsters( uint8_t NI, int xr, int yr, int xr1, int yr1, uint16_t *Collect, uint16_t* Ser, bool WRITE, CHOBJ* FN, int NN, int MAX );

void CreateFields( uint8_t NI, int x, int y, int n );

int CheckShipDirection( char Dir );
void RotateShipAndDie( OneObject* OBJ );

int GetResByName( char* gy );

void InviteAI_Peasants( OneObject* Mine );

void PlayAnimation( NewAnimation* NA, int Frame, int x, int y );

#endif // NEWMON_H
