#ifndef MEGAPOLIS_H
#define MEGAPOLIS_H

#include <cstdint>

#include "Brigade.h"

#define N_KINDS 5
#define OrdDist 320

#pragma pack( push, 1 )

class CityProject
{
public:

};
class ProposedProject
{
public:
    uint8_t PKind;//=0-Monster,==1-Upgrade
    uint16_t NIndex;//номер типа монстра/Upgrade index
    uint16_t ProducerIndex;//индекс свободного производителя
    uint16_t Percent;//процент денег, употребляемый на производство
	//int Cost;//цена 
	//int Useful[NBRANCH];
};
struct BuildProject
{
	bool Used : 1;
	bool PlaceFound : 1;
	bool Founded : 1;
	bool PeasantsCalled : 1;
    uint8_t Usage;
	short NearX;
	short NearY;
	int  x, y;
	int Options;
    uint16_t AttemptsToStand;
    uint16_t AttemptsToFindApprPlace;
    uint16_t Index;
    uint16_t NIndex;
    uint8_t MaxPeasants;
    uint8_t MinPeasants;
    uint8_t NPeasantsCalled;
};
class CityCell
{
public:
	int x, y;
    uint16_t WorkTimes;//==0xFFFF if inside the city
	CityCell* NextCell;
};
class CityCluster
{
public:
	CityCell* FirstCell;
	void AddCell( int x, int y );
};
typedef int Functional( int* x, int* y, int Lx, int Ly, uint8_t NI );

class City;

int GetBMIndex( OneObject* OB );

class Idea;
typedef void IdeaBrain( Idea* IDEA );
typedef void FreeIdea( Idea* ID );
class Idea
{
public:
	City* CT;
	Idea* PrevIdea;
	Idea* NextIdea;
	IdeaBrain* Brain;
	void* IdeaData;
	FreeIdea* FI;
	int   DataSize;
	void ClearIdea();
};
class Inform;
//typedef void I_Save(SaveBuf* SB,Inform* Inf);
//typedef void I_Load(SaveBuf* SB,Inform** Inf);
typedef void I_Clear( Inform* Inf );
class Inform
{
public:
    uint16_t ID;
    uint16_t Essence;
	Inform* Next;
	Inform* Previous;
	City*   CT;
	I_Clear* IClr;
    uint16_t Size;
};
class MineBase :public Inform
{
public:
    uint16_t   M_ID;
    uint16_t   M_SN;
	short  topx;
	short  topy;
    uint8_t   ResKind;
    uint8_t   UpgLevel;
    uint8_t   PSend;
    uint16_t   DefendersID;
    uint16_t   DefendersSN;
    uint16_t   MinersID;
    uint16_t   MinersSN;
	/*
    uint16_t   TowID;
    uint16_t   TowSN;
    uint16_t   NWalls;
	short* Walls;
    uint16_t   NGateProj;
	short* GateProj;
    uint16_t   NGates;
	short* Gates;
	*/
};
class PRM_Info :public Inform
{
public:
	int NBrigs;
    uint16_t BrigsID[5];
    uint16_t BrigsSN[5];
};

//#define MBR4    16
//#define MaxBrig (MBR4*32)
constexpr int MBR4 = 16;
constexpr int MaxBrig = MBR4*32;

#define MaxArm 128
class DefendInfo
{
public:
    uint8_t  x;
    uint8_t  y;
    uint8_t  Importance;
    uint16_t  NDefenders;
    uint16_t  MaxDefs;
    uint16_t * Def;
    uint16_t * DefSN;
	void AddUnit( OneObject* OB );
};
class AI_Army;
//----------------------ARMY--------------------//

#define PolkSize   36
#define NBrigTypes 6
class ExtendedBrigade
{
public:
    uint8_t BrigadeType;
	//0-Short range infantry
	//1-Long range infantry
	//2-Fast cavalry
	//3-Hard cavalry
	//4-Long range cavalry
	//5-Artillery
	//6-Sapers
	//7-Builders
	int Force;
	int NeedMembers;
    uint16_t NextBrigade;
	Brigade* Brig;
};
typedef void ArmyLink( AI_Army* ARM );
class ArmyOrder;
class ArmyOrder
{
public:
    uint8_t OrdType;
    uint8_t Prio;
    uint16_t Size;
    const char* Message;
	ArmyOrder* Next;
	ArmyLink*  ALink;
};
struct FormInfo
{
    uint16_t ComID;
    uint16_t ComSN;
    uint16_t BarID;
    uint16_t BarSN;
};

class Nation;
class AI_Army
{
public:
	int NExBrigs;
	int MaxExBrigs;
	bool Enabled : 1;
	bool SpecialOrder : 1;
	int TopPos;
	int x, y;
	int Force;
    uint8_t Spec;
    uint16_t ArmyID;
	ExtendedBrigade* ExBrigs;
	//Additional information
	City* CT;
	Nation* NT;
    uint8_t NI;
	int LastBitvaTime;
    uint16_t FirstBrig;
	//Order information
	ArmyOrder* AOrder;
	//Formation info;
    uint16_t ComID[8];
    uint16_t ComSN[8];
    uint16_t NCom;
    uint16_t BarID[8];
    uint16_t BarSN[8];
    uint16_t NBar;
    uint16_t NFreeUnits;
	//Functions
	void InitArmy( City* CT );
	void ClearArmy();
	void CreateMinimalArmyFromBrigade( Brigade* BR, int Type );
	void AddBrigade( Brigade* BR );
	void AddUnit( OneObject* OB );
	void CloseArmy();
	void MoveArmyTo( int x, int y );
	void CreateBrigLink();
	void AttackArea( int x, int y );
    ArmyOrder* CreateOrder( uint8_t OrdType, int Size );
	void DeleteAOrder();
	void ClearAOrders();
    void LocalSendTo( int x, int y, uint8_t Prio, uint8_t OrdType );
	int  GetArmyDanger( int x, int y );
	void SendToMostSafePosition();
    void WideLocalSendTo( int x, int y, uint8_t Prio, uint8_t OrdType );
	void Parad();
    void ConnectToArmy( int ID, uint8_t Prio, uint8_t OrdType );
	void MakeBattle();
	void Bitva();
	void Diversia();
	void SetZombi();
	void ClearZombi();
	bool CheckArtillery();
    int  GetAmountOfBrigs( uint8_t Type );
	void SendGrenaders();
};

class GeneralObject;

class City
{
public:
	Nation* Nat;
    uint8_t NI;
	//int InX,InY,InX1,InY1,OutX,OutY,OutX1,OutY1;
	//int TryTimes;
	//int ExpandSize;
	int Amount;
	//int GoldPeasPart;
	//int WoodPeasPart;
	//int BuildPeasPart;
    //uint16_t KindMax[N_KINDS];//макс. кол-во тварей типа Kind
    //uint16_t *MKList[N_KINDS];//массив указателей на массивы с индексами монстров данного типа
	//Functional* FNC[N_KINDS];//массив функционалов для выбора места, если место вообще не подходит, то возвр. -1
    //uint16_t BranchPart[NBRANCH];
	int AutoEraseTime;
    uint16_t BestProj[128];
    uint16_t NBestProjects;
    uint8_t NWoodSklads;
    uint8_t NStoneSklads;
    uint16_t WoodSkladID[8];
    uint16_t WoodSkladSN[8];
    uint16_t StoneSkladID[8];
    uint16_t StoneSkladSN[8];
	char AIID[48];
    uint8_t StartRes;
    uint8_t ResOnMap;
    uint8_t Difficulty;
    uint8_t LandType;
    uint16_t TransportID;
    uint16_t TransportSN;

    uint8_t NMeln;
    uint16_t MelnID[4];
    uint16_t MelnSN[4];

	int  MelnCoorX[4];
	int  MelnCoorY[4];
	int  NMelnCoor;

	int  FieldsID[512];
    uint16_t FieldsSN[512];
    uint16_t NFields;

    uint16_t Construction[64];
    uint16_t ConstructionSN[64];
    uint16_t ConstructionTime[64];
    uint16_t NConstructions;

    uint16_t FarmsUnderConstruction;
	int Account;

	bool FieldReady;
	bool AllowWood;
	bool AllowStone;
	bool AllowMine;
	bool DefenceStage;

    uint16_t NeedOnField;
    uint16_t NeedOnStone;
    uint16_t NeedOnWood;

    uint16_t ** GroupsSet;
    uint16_t *  NGroupsInSet;
	int    NGroups;

//	Brigade Brigs[MaxBrig - 11];
//	Brigade Settlers;
//	Brigade Defenders;
//	Brigade Guards;
//	Brigade Agressors;
//	Brigade OnField;
//	Brigade OnStone;
//	Brigade OnWood;
//	Brigade Builders;
//	Brigade Free;
//	Brigade InMines;
//	Brigade FreeArmy;

    Brigade Brigs[ MaxBrig ];
    inline Brigade & Settlers()     { return Brigs[ MaxBrig - 11 ]; }
    inline Brigade & Defenders()    { return Brigs[ MaxBrig - 10 ]; }
    inline Brigade & Guards()       { return Brigs[ MaxBrig - 9 ]; }
    inline Brigade & Agressors()    { return Brigs[ MaxBrig - 8 ]; }
    inline Brigade & OnField()      { return Brigs[ MaxBrig - 7 ]; }
    inline Brigade & OnStone()      { return Brigs[ MaxBrig - 6 ]; }
    inline Brigade & OnWood()       { return Brigs[ MaxBrig - 5 ]; }
    inline Brigade & Builders()     { return Brigs[ MaxBrig - 4 ]; }
    inline Brigade & Free()         { return Brigs[ MaxBrig - 3 ]; }
    inline Brigade & InMines()      { return Brigs[ MaxBrig - 2 ]; }
    inline Brigade & FreeArmy()     { return Brigs[ MaxBrig - 1 ]; }

    uint8_t N_mineUp[10];
    uint16_t MaxFields;
    uint16_t BestOnField;
    uint16_t WaterBrigs[128];
	//------------------ARMIES-------------------//
	AI_Army ARMS[MaxArm];
    uint16_t * DefArms;
	int NDefArms;
	int MaxDefArms;
	int NDivr;
    uint8_t LockUpgrade[8];
    uint8_t MyIsland;
	//-------------Peasants balance--------------//
	short NPeas;
	short NeedPF;
	short NeedPW;
	short NeedPS;
	short FreePS;
	int   WoodAmount;
	int   StoneAmount;
	int   FoodAmount;
	int   LastTime;
	int  WoodSpeed;
	int  StoneSpeed;
	int  FoodSpeed;
	//-------------------------------------------//
	int   LFarmIndex;
	int   LFarmRadius;
	short FarmLX;
	short FarmLY;
	//-------------------------------------------//
    uint16_t  MAX_WORKERS;
	//-------------------------------------------//
	bool AI_DEVELOPMENT : 1;
	bool AI_WATER_BATTLE : 1;
	bool AI_LAND_BATTLE : 1;
	bool AI_MINES_CPTURE : 1;
	bool AI_TOWN_DEFENCE : 1;
	bool AI_MINES_UPGRADE : 1;
	bool AI_FAST_DIVERSION : 1;
	//-----------DEFENDING THE BASE--------------//
	DefendInfo* DefInf;
	int NDefn;
	int MaxDefn;
	int MaxGuards;
	int AbsMaxGuards;
	//-------------------------------------------//
    uint16_t NP;
    uint16_t NAgressors;
    uint16_t MinType;
    uint16_t MaxType;
    uint16_t UnitAmount[2048];//Amount of units of some kind
    uint16_t ReadyAmount[2048];
    uint16_t PRPIndex[2048];//index of project to produce this type of monster,if not proposed,0xFFFF
    uint16_t UPGIndex[2048];
    uint16_t UnBusyAmount[2048];//сколько модулей простаивает
    uint16_t Producer[2048];
	bool PresentProject : 1;
	bool IntellectEnabled : 1;
	bool CenterFound : 1;
	short CenterX;
	short CenterY;
	BuildProject BPR;
    uint16_t NProp;
	ProposedProject Prop[100];
	Idea* IDEA;
	Inform* INFORM;
	Idea* AddIdea( IdeaBrain* IBR, bool Dup );
	void AddInform( Inform* Inf, I_Clear* ICL );
	void DelInform();
	void DelInform( Inform* Inf );
    Inform* SearchInform( uint16_t ID, uint16_t Essence, Inform* inf );
    Inform* SearchInform( uint16_t ID, Inform* inf );
	void HandleIdeas();
	void DelIdeas();
    uint16_t EnemyList[32];
    void CreateCity( uint8_t NI );
    void EnumUnits();
    void AddProp( uint16_t NIN, GeneralObject* GO, uint16_t prop, uint16_t per );
    void AddUpgr( uint16_t NIN, uint16_t prod, uint16_t per );
	void EnumProp();
	void ProcessCreation();
    void HelpMe( uint16_t ID );
    uint16_t FindNeedProject();
	void RefreshAbility();
	bool FindApproximateBuildingPlace( GeneralObject* GO );
	bool FindPreciseBuildingPlace( GeneralObject* GO );
	void AddConstruction( OneObject* OB );
	void HandleConstructions();
	void MarkUnusablePlace();
	void CloseCity();
	void HandleFields();
	void MakeZasev();
	void MakeSlowZasev();
	void HandleBrigades( int NP );
	void RegisterNewUnit( OneObject* OB );
	int  GetFreeBrigade();
	void ExecuteBrigades();
	void CalculateBalance();
	void GetNextPlaceForFarm( int* x, int* y );
	void UnRegisterNewUnit( OneObject* OB );
	void RegisterWaterBrigade( int ID );
	void UnRegisterWaterBrigade( int ID );
	//---------------Armies--------------
	int GetFreeArmy();

	//--------------Ideas----------------
	void TakeNewMine();
    void UpgradeMines();
	void SendAgressors();
    //----------------defending--------------//
	void HandleDefending();
    int  CheckDefending( uint8_t x, uint8_t y );
    void AddDefending( uint8_t x, uint8_t y, uint8_t Importance );
	void AddUnitDefender( OneObject* OB );
};

#define SendPTime 10000
class SendPInform :public Inform
{
public:
	int time;
};
struct DangerInfo
{
    uint16_t ID;
    uint16_t SN;
    uint16_t Kind;
    uint16_t UpgradeLevel;
    uint16_t Life;
    uint16_t MaxLife;
	int x, y, z;
	int MinR;
	int MaxR;
};

#pragma pack(pop)

void OutCInf( uint8_t x, uint8_t y );
void InitSuperMortiraCells();
void CreateEnmBuildList();
void CreateProtectionMap();

int GetTopDistance( int xa, int ya, int xb, int yb );

extern int * ResTBL;
extern int  NInResTBL;
extern uint16_t FIELDID;
extern int DefaultResTBL[12];

extern bool CINFMOD;
extern bool CanProduce;
extern int CURRENTAINATION;

void DeleteFromGroups( uint8_t NI, uint16_t ID );

OneObject* DetermineMineBySprite( int Spr );

uint16_t GetTopFast( int x, int y );

void ArmyMakeBattleLink( AI_Army* ARM );
void GrenaderSupermanLink( OneObject* OBJ );

void ArmyLocalSendToLink( AI_Army* ARM );
void ArmyConnectToArmyLink( AI_Army* ARM );
void A_BitvaLink( AI_Army* ARM );
void A_DiversiaLink( AI_Army* ARM );
void ArmyMakeDiversiaLink( AI_Army* ARM );

int CheckBuilders( OneObject* OB, City* CT );
int CheckFree( OneObject* OB, City* CT );
int CheckFieldWorker( OneObject* OB, City* CT );
int CheckFreeArmy( OneObject* OB, City* CT );
int CheckStoneWorker( OneObject* OB, City* CT );
int CheckWoodWorker( OneObject* OB, City* CT );

void TakeNewMineBrain( Idea* ID );
void UpgradeMineBrain( Idea* ID );
void BuildWallBrain( Idea* ID );

void HandleGeology();

class AdvCharacter;
void UpdateAttackR( AdvCharacter* ADC );

void UnGroupSelectedUnits( uint8_t NI );
void GroupSelectedFormations( uint8_t NI );
bool CheckGroupPossibility( uint8_t NI );
bool CheckUnGroupPossibility( uint8_t NI );

#endif // MEGAPOLIS_H
