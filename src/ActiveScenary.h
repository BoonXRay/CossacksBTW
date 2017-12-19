#ifndef ACTIVESCENARY_H
#define ACTIVESCENARY_H

#include <cstdint>

#include <windows.h>

#pragma pack( push, 1 )

struct GAMEOBJ{
    uint16_t  Index;
    uint16_t  Serial;
	int  Type;
};
struct UnitsGroup{
    uint16_t * IDS;
    uint16_t * SNS;
	int N;
};
struct UnitsPosition{
    uint16_t * Type;
	int*  coor;
	int N;
};
struct ZonesGroup{
    uint16_t * ZoneID;
	int N;
};
typedef void StdVoid();
struct GTimer{
	int Time;
	bool Used;
	bool First;
};
struct LightSpot{
	int x,y,Type;
};
class ScenaryInterface{
public:
	void** SaveZone;
	int *  SaveSize;
	int    NSaves;
	int    MaxSaves;
	HINSTANCE hLib;
	char*  DLLName;

	UnitsGroup* UGRP;
	int    NUGRP;
	int    MaxUGRP;
	
	UnitsPosition* UPOS;
	int    NUPOS;
	int    MaxUPOS;

	ZonesGroup* ZGRP;
	int    NZGRP;
	int    MaxZGRP;

    const char** Messages;
	int    NMess;
	int    MaxMess;

	char** Sounds;
	int    NSnd;
	int    MaxSnds;

	int NErrors;

	//char*  MissText;
	//int    TextSize;

	int NPages;
	int MaxPages;
	char** Page;
	int*   PageSize;
	char** PageID;
	char** PageBMP;

	bool   TextDisable[52];
	
	bool   StandartVictory;
	bool   Victory;
	char*  VictoryText;

	bool   LooseGame;
	char*  LooseText;

    const char*  CentralText;
	int CTextTime;

	GTimer TIME[32];
    uint16_t   TRIGGER[512];
	LightSpot LSpot[64];
	StdVoid* ScenaryHandler;
	ScenaryInterface();
	~ScenaryInterface();
	void Load(char* Name,char* Text);
	void UnLoading();
};
extern ScenaryInterface SCENINF;
class SingleMission{
public:
	char* ID;
	char* DLLPath;
	char* MapName;
	char* Name;
	char* Description;
	int NIntro;
	char** Intro;
};
class MissPack{
public:
	SingleMission* MISS;
	int NMiss;
	int MaxMiss;
	int CurrentMission;
	int* SingleMS;
	int MSMiss;
	void LoadMissions();
	MissPack();
	~MissPack();
};
struct SingleCampagin{
	char* CampMessage;
	char* CampText;
	char* CampBmp;
	int NMiss;
	int* Miss;
    uint32_t * OpenIndex;
};
class CampaginPack{
public:
	int NCamp;
	SingleCampagin* SCamp;
	CampaginPack();
	~CampaginPack();
};
class OneBattle{
public:
	char* ID;
	char* Map;
	char* Text;
	char* Brief;
	char* BigMap;
	char* MiniMap;
	char* BigHeader;
	char* SmallHeader;
	char* Date;
	char* RedUnits;
	char* BlueUnits;
	int NHints;
	char** Hints;
	int* Coor;
};
class OneWar{
public:
	char* Name;
	char* Date;
	char* Text;
	int NBatles;
	int* BattleList;
};
class WarPack{
public:
	int NWars;
	OneWar* Wars;
	int NBattles;
	OneBattle* Battles;
	WarPack();
	~WarPack();
};

#pragma pack(pop)

extern WarPack WARS;
extern MissPack MISSLIST;
extern CampaginPack CAMPAGINS;

class City;
class Nation;

extern uint8_t CurAINation;
extern City* CCIT;
extern Nation* CNAT;
extern bool AiIsRunNow;

extern int _pr_Nx;

extern int AI_Registers[8][32];

extern bool RUNMAPEDITOR;
extern bool RUNUSERMISSION;
extern char USERMISSPATH[ 128 ];

extern uint8_t INVECO[6];

void SFLB_LoadPlayerData();
void SavePlayerData();

class GFILE;
bool ReadWinString( GFILE* F, char* STR, int Max );

void ScenaryLights();
void CreateMissText();

void ImClearSelection( uint8_t Nat );
void ReClearSelection( uint8_t Nat );

extern "C" __declspec( dllexport ) void ShowCentralText( const char* ID, int time );

void RetryCQuest();

void LoadAIFromDLL(uint8_t Nat, const char *Name );

void StartAIEx( uint8_t Nat, char* Name, int Land, int Money, int ResOnMap, int Difficulty );

extern "C" __declspec( dllexport ) void ShowVictory();
extern "C" __declspec( dllexport ) void SelChangeNation( uint8_t SrcNat, uint8_t DstNat );

void HandleMission();

#endif // ACTIVESCENARY_H
