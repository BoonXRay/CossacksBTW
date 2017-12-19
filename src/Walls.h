#ifndef WALLS_H
#define WALLS_H

#include <cstdint>

#pragma pack( push, 1 )

class WallCharacter
{
public:
	char* Name;
    uint16_t RIndex;
    uint16_t NBuild;
    uint16_t Ndamage;
	short dx, dy;
    uint16_t GateFile;
	short GateDx;
	short GateDy;
	short UpgradeGateIcon;
	short OpenGateIcon;
	short CloseGateIcon;
	short UpgradeTower1;
	short UpgradeTower2;
    uint16_t  GateCost[8];
	short TexRadius;
	short NTex;
	short TexPrec;
	short* Tex;
	short OpenGateSoundID;
	short CloseGateSoundID;
    uint8_t  OpenKeyFrame;
    uint8_t  CloseKeyFrame;
};

class WallCluster;

class WallCell
{
public:
	short x;
	short y;
    uint8_t DirMask;
    uint8_t Type;
    uint8_t NI;
    uint8_t Stage;
    uint8_t MaxStage;
    uint16_t Health;
    uint16_t MaxHealth;
    uint8_t Sprite;
    uint8_t SprBase;
    uint16_t ClusterIndex;
    uint16_t OIndex;
    uint16_t GateIndex;
	int Locks;
	bool Visible;
	bool CheckPosition();
	int GetLockStatus();
	void SetLockStatus();
	bool StandOnLand( WallCluster* WC );
	void CreateLocking( WallCluster* WC );
	void Landing( WallCluster* WC );
	void ClearLocking();
};

class WallSystem;
class NewMonster;

class WallCluster
{
public:
    uint8_t Type;
	int NCornPt;
    uint16_t * CornPt;
	int NCells;
	WallCell* Cells;
	WallSystem* WSys;
	short LastX;
	short LastY;
	short FinalX;
	short FinalY;
    NewMonster * NM;
    uint16_t  NIndex;
    uint8_t  NI;
	//------------------//
	WallCluster();
	~WallCluster();
	void SetSize( int N );
	void ConnectToPoint( short x, short y );
	void ConnectToPoint( short x, short y, bool Vis );
	void UndoSegment();
    void KeepSegment();
	void CreateSprites();
	void AddPoint( short x, short y, bool Vis );
	void Preview();
	void View();
    int  CreateData( uint16_t * Data, uint16_t Health );
    void CreateByData( uint16_t * Data );
    void SendSelectedToWork( uint8_t NI, uint8_t OrdType );
};

class WallSystem
{
public:
	int NClusters;
	WallCluster** WCL;
	//-------------------//
	WallSystem();
	~WallSystem();
	void AddCluster( WallCluster* );
    void Show();
};

void WallHandleDraw();
void WallHandleMouse();
void SetWallBuildMode( uint8_t NI, uint16_t NIndex );
void LoadAllWalls();

extern WallCharacter WChar[32];

//extern WallCell* WRefs[MAXCX*MAXCY*4];

extern WallCell** WRefs;
extern WallSystem WSys;

void DetermineWallClick( int x, int y );
void SetLife( WallCell* WC, int Health );

//gates
#define NGOpen 9

class Gate
{
public:
	short x;
	short y;
    uint8_t NI;
    uint8_t NMask;
    uint8_t State;
    uint8_t delay;
    uint8_t Locked;
    uint8_t CharID;
};

#pragma pack(pop)

extern Gate* Gates;
extern int NGates;
extern int MaxGates;

extern bool BuildWall;
extern WallCluster TMPCluster;

void SetupGates();
void InitGates();
int AddGate( short x, short y, uint8_t NI );
void DelGate( int ID );

bool CheckVisibility( int x1, int y1, int x2, int y2, uint16_t MyID );
bool CheckGateUpgrade( OneObject* OB );
bool CreateGates( OneObject* OB );

class OneObject;
void BuildWallLink( OneObject* OB );
void CreateGatesLink( OneObject* OBJ );

bool CheckOpenGate( OneObject* OB );
bool CheckCloseGate( OneObject* OB );

void ControlGates();
void CloseGates( OneObject* OB );
void OpenGates( OneObject* OB );

bool CheckWallClick( int x, int y );

WallCharacter* GetWChar( OneObject* OB );

void TempUnLock( OneObject* OBJ );

int GetWallType( char* Name );

#endif // WALLS_H
