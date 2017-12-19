#ifndef MAIN_H
#define MAIN_H

#include <cstdint>

#include <windows.h>

#include "Megapolis.h"


extern HWND hwnd;

extern int MaxSizeX;
extern int MaxSizeY;

extern bool NOPAUSE;
extern bool LockPause;
extern bool FullMini;
extern bool HealthMode;
extern bool InfoMode;
extern bool TransMode;
extern bool fixed;

extern bool MUSTDRAW;

extern int CostThickness;

extern int HISPEED;
extern int AutoTime;
extern int CoalID;
extern int Flips;
extern int FoodID;
extern int FrmDec;
extern int GoldID;
extern int IronID;
extern int LastAttackDelay;
extern int REALTIME;

extern int Shifter;
extern int SpeedSh;
extern int StoneID;
extern int TreeID;

extern uint8_t PlayGameMode;

extern uint8_t LastAsciiKey;
extern int NKeys;

extern City CITY[8];

extern int NInGold[ 8 ];
extern int NInIron[ 8 ];
extern int NInCoal[ 8 ];
extern int WasInGold[ 8 ];
extern int WasInIron[ 8 ];
extern int WasInCoal[ 8 ];

extern int tmtmt;  //Main internal counter for intervals

extern bool PalDone;

extern bool EditMapMode;

extern int exFMode;
__declspec( dllexport ) extern uint16_t dwVersion;

extern int PitchTicks;
extern int MaxPingTime;

extern int exRealLx;
extern int exRealLy;
extern const char* FormationStr;
extern uint8_t LockGrid;
extern uint16_t Creator;


extern short WheelDelta;

extern bool ChoosePosition;

extern bool SHOWSLIDE;

extern bool EnterChatMode;

extern int GLOBALTIME;
extern int PGLOBALTIME;

extern int screen_width;
extern int screen_height;
extern double screen_ratio;

__declspec( dllexport ) extern const char LobbyVersion[32];
__declspec( dllexport ) extern const char BuildVersion[32];

extern bool RetryVideo;

extern bool PeaceMode;

extern uint8_t EditMedia;
extern uint8_t LockMode;
extern bool MEditMode;
extern int HeightEditMode;

extern int BlobMode;

extern uint8_t LastAscii;

extern int WaterEditMode;

extern int HiStyle;

extern int  ReliefBrush;

struct xRLCTable;
extern xRLCTable * RCross;

extern int RealPause;
extern int RealStTime;
extern int RealGameLength;
extern int CurrentStepTime;

extern int SUBTIME;
extern int NeedCurrentTime;

extern int SHIFT_VAL;

unsigned long GetRealTime();

void UnPress();
void ClearMStack();
void ClearKeyStack();
int ReadKey();

void ClearModes();

bool InitScreen();

void ResizeAndCenterWindow();

void PreDrawGameProcess();
void PostDrawGameProcess();

void EraseRND();

struct MouseStack;
MouseStack* ReadMEvent();

void GameKeyCheck();

void PrepareToEdit();
void PrepareToGame();

#endif // MAIN_H
