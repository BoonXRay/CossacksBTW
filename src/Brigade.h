#ifndef BRIGADE_H
#define BRIGADE_H

#include <cstdint>

#pragma pack( push, 1 )

struct BrigMemb
{
    uint16_t Peons;
    uint16_t Infantry;
    uint16_t Strelkov;
    uint16_t Mortir;
    uint16_t Pushek;
    uint16_t Grenaderov;
    uint16_t Other;
    uint16_t reserv;
};

struct Brigade;
typedef void BrigadeLink( Brigade* Brig );
struct BrigadeOrder
{
    uint8_t OrdType;
    uint8_t Prio;
    uint16_t Size;
    const char* Message;
    BrigadeOrder* Next;
    BrigadeLink* BLink;
};

class OneObject;
class City;
typedef int CheckWorkFn( OneObject* OB, City* CT );

struct Brigade
{
    City* CT;
    uint16_t * Memb;
    uint16_t * MembSN;
    uint16_t  SN;
    int* posX;
    int* posY;
    uint16_t NMemb;
    uint16_t MaxMemb;
    uint16_t MembID;
    char AddDamage;
    char AddShield;
    uint8_t WarType;
    uint8_t Direction;
    uint16_t ID;
    uint16_t BrigDelay;
    uint16_t LastTopology;
    int LastOrderTime;
    uint16_t ArmyID;
    uint16_t ErasureTime;
    bool Enabled : 1;
    bool PosCreated : 1;
    bool Precise : 1;
    bool Strelki : 1;
    bool AttEnm : 1;
    BrigMemb BM;
    CheckWorkFn* CFN;
    BrigadeOrder* BOrder;
    uint16_t LastEnemyID;
    uint16_t LastEnemySN;


    void SetIndex();
    void Init( City* CT, uint16_t ID );
    void AddObject( OneObject* OB );
    void CheckMembers( City* CT );
    void RemoveObjects( int NObj, Brigade* Dest );
    bool RemoveOne( int Index, Brigade* Dest );
    void RemovePeasants( Brigade* Dest );
    void FreeMember( int Idx );
    void DeleteAll();
    void CreateSquare();
    void CreateConvoy( uint8_t Type );
    int AddInRadius( int x, int y, int r, BrigMemb* BMem, Brigade* Dest );
    int AddInRadius( int x, int y, int r, BrigMemb* BMemb );
    BrigadeOrder* CreateOrder( uint8_t OrdType, int Size );
    int SelectPeasants( uint8_t NI );
    void Rospusk();
    bool GetCenter( int* x, int* y );
    //----------------------ORDERS----------------------//
    bool LocalSendTo( int x, int y, uint8_t prio, uint8_t OrdType );
    bool LinearLocalSendTo( int x, int y, uint8_t prio, uint8_t OrdType );
    bool WideLocalSendTo( int x, int y, uint8_t prio, uint8_t OrdType );
    bool CaptureMine( int SID, uint8_t prio, uint8_t OrdType );
    bool AttackEnemy( int x, int y, uint8_t prio, uint8_t OrdType );
    void MakeBattle();
    //-----------------Human functions------------------//
    bool CreateNearOfficer( OneObject* OB, uint16_t Type, int ODIndex );
    void CreateOrderedPositions( int x, int y, char dir );
    void CreateSimpleOrderedPositions( int x, int y, char dir );
    void HumanLocalSendTo( int x, int y, short Dir, uint8_t Prio, uint8_t OrdType );
    void HumanGlobalSendTo( int x, int y, short Dir, uint8_t Prio, uint8_t OrdType );
    void KeepPositions( uint8_t OrdType, uint8_t Prio );
    void Bitva();
    //--------------------------------------------------//
    void ClearBOrders();
    void DeleteBOrder();
};

void EraseBrigade(Brigade* BR);

void ResearchCurrentIsland( uint8_t Nat );
void DrawPlaneLine( int x0, int y0, int x1, int y1, uint8_t c );
void DrawBorder( Brigade* BR );

void CorrectBrigadesSelection(uint8_t NT );
void ImCorrectBrigadesSelection( uint8_t NT );

//------------------------------Local send to---------------------------//
int GetTopology( int x, int y );
int GetTopology( int* x, int* y );

int FindUnits( int x, int y, int r, int Type, uint8_t Nation, int Need, uint16_t * Dest );
void SearchArmy( OneObject* OB );

void OptimiseBrigadePosition( Brigade* BR );

void B_LocalSendToLink( Brigade* BR );
void B_WideLocalSendToLink( Brigade* BR );
void B_GlobalSendToLink( Brigade* BR );
void B_CaptureMineLink( Brigade* BR );
void B_MakeBattleLink( Brigade* BR );
void B_KeepPositionsLink( Brigade* BR );
void B_LeaveAttackLink( Brigade* BR );
void B_BitvaLink( Brigade* BR );
void B_HumanGlobalSendToLink( Brigade* BR );

void HumanLocalSendToLink( Brigade* BR );
void HumanEscapeLink( Brigade* BR );

void LocalSendShipsLink( Brigade* BR );

void RotUnitLink( OneObject* OB );
void RotUnit( OneObject* OB, char Dir, uint8_t OrdType );

extern int NIslands;
extern uint16_t * TopIslands;

void ResearchIslands();
void ResearchBestPortToFish( uint8_t Nat );
void ProduceByAI( uint8_t nat, uint16_t Producer, uint16_t ID );

void MakeShipBattle( Brigade* BR );
void MakeShipBattleLink( Brigade* BR );

struct CostPlace
{
    int xw, yw;
    int xc, yc;
    uint8_t Island;
    uint8_t R;
    uint8_t Access;
    uint16_t Transport;
};

#pragma pack(pop)

extern CostPlace* COSTPL;
extern int NCost;
extern int MaxCost;

void CreateCostPlaces();
void ProcessCostPoints();

void MakeDiversionLink( Brigade* BR );
void MakeDiversion( Brigade* BR );

class AI_Army;
void CalculateFreeUnits( AI_Army* AIR );

void SearchArmyLink( OneObject* OBJ );

#endif // BRIGADE_H
