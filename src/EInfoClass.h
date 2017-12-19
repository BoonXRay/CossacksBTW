#ifndef EINFOCLASS_H
#define EINFOCLASS_H

#include <cstdint>

#include "LoadSave.h"
#include "Megapolis.h"
#include "StrategyResearch.h"

constexpr int MAXTOW = 24;

#pragma pack( push, 1 )

#define TSSHIFT (4+8)
struct HumanShip{
    uint16_t ID;
    uint16_t SN;
	int x,y;
	int Force;
	int Life;
	int R;
};
struct HShipsGroup{
	int xs,ys;
	int xL,yL;
	int xR,yR;
	int Ns, Force;
	int CTop;
	int R;
};

constexpr uint8_t SafeCellSH = 2;
constexpr uint8_t SafeCLX = 1 << SafeCellSH;
constexpr uint8_t SafeMask = SafeCLX - 1;
constexpr uint8_t SafeCN = SafeCLX * SafeCLX;
#define SafeLX (TopLx >> SafeCellSH)
#define SafeSX (TopSH - SafeCellSH)
#define SafeSize (SafeLX * SafeLX)

struct SafeCellInfo{
    uint16_t Index,SN;
    uint16_t Prio,BusyIndex,BusySN;
};

class EnemyInfo{
public:
	GlobalArmyInfo GAINF;
    uint8_t Mask;
    uint8_t BasicNation;

    uint8_t * ProtectionMap;
    uint16_t * SupMortBestID;
    uint16_t * SupMortLastTime;
    uint16_t EnmBuildList[128];
    uint16_t EnmBuildSN[128];
	int  NEnmBuild;
	int  NEnWalls;
    uint16_t WallsX[200];
    uint16_t WallsY[200];
    uint16_t TowsID[MAXTOW];
    uint16_t TowsSN[MAXTOW];
    uint16_t TowsMaxR[MAXTOW];
    uint8_t TowDanger[MAXTOW];
    uint16_t CurTow;
    uint16_t CurR;
    uint16_t CurXip;
    uint16_t MaxTowR;
    uint32_t * InflMap;
	DangerInfo DINF[40];
	int NDINF;
	int LastBuildSafetyTime;
    uint8_t * SafeMAP;
	int CurRStage;
	SafeCellInfo** SCINF;
    uint16_t WTopNear[64];
	int NWTopNear;
    uint16_t * NUN;
    uint8_t * TMAP;
	HumanShip SHIPS[128];
	HShipsGroup SGRP[64];
	int NSGRP;
	int NHSHIPS;
	int ShipsForce;
    uint16_t TopAreasDanger[1024];
	
	void InitInflMap();
	void ClearTow(OneObject* OB);
	void AddTow(OneObject* OB);
	void ProcessTow();
	void InitSuperMortiraCells();
    uint16_t ResearchCellForSupermortira(int cell,int MinDistance,int MaxDistance);
    uint16_t SearchDangerousPlaceForEnemy(int* utx,int* uty,int MINR,int MAXR);
	bool FindSafePlace(int* utx,int* uty);
    uint16_t SearchBestEnemyAndPlaceForSupermortira(OneObject* OB,int* BestX,int* BestY);
	void CreateEnmBuildList();
	void CreateProtectionMap();
	void CreateWallsList();
    void CreateListOfDangerObjects();
	void ResearchArmyDistribution();
	void InitBuildSafety();
	void CreateBuildSafetyMap();
	void RefreshSafeMap();
	byte GetSafeVal(int x,int y);
    void AddSafePoint(int x,int y,uint16_t Index,uint16_t SN,uint16_t Prio);
    void InitSafeInfo();
	void CloseSafeInfo();
	void CheckSafeInfo();
	SafeCellInfo* GetCellInfo(int x,int y);
	void RegisterSafePoint(OneObject* OB,int x,int y);
    void ResearchSafeObject(OneObject* OB,int MinR,int MaxR,int pstart);
	void ResearchSafeCells(int MinR,int MaxR);
	void Clear();
	void ResearchHumanAttackPlaces();
	void ClearPlaces();
	void RegisterHumanShips();
	int  GetMaxForceOnTheWay(int TopStart,int FinalTop);
	void ResearchShipsGroups();
	int  GetShipsForce(int x,int y,int r);
	void ClearIslands();

	void ALLOCATE();
	void FREE();


};
class GlobalEnemyInfo{
public:
	EnemyInfo* EINF[8];
	void Clear();
	void Setup();
	void Process();
	void AddTow(OneObject* OB);
	void ClearTow(OneObject* OB);
	~GlobalEnemyInfo();
	GlobalEnemyInfo();
	bool FindNearestEnemy(byte NNUM,int* x,int* y,bool TowerFear,int Min,bool Defence);
};

#pragma pack(pop)

extern GlobalEnemyInfo GNFO;

int GetShipForce(uint8_t Usage);

#endif // EINFOCLASS_H
