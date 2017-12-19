#ifndef MULTI_H
#define MULTI_H

#include <cstdint>

#include "Brigade.h"
#include "MapDescr.h"

class Nation;

extern uint8_t  MYNATION;
#define SetMyNation(x) {MYNATION=x^133;}
#define MyNation (MYNATION^133)

extern Nation NATIONS[8];
extern uint8_t ExBuf[8192];
extern int EBPos;

extern uint16_t * Selm[8];
extern uint16_t * SerN[8];
extern uint16_t * ImSelm[8];
extern uint16_t * ImSerN[8];
extern uint16_t NSL[8];
extern uint16_t ImNSL[8];
extern bool CmdDone[ULIMIT];
extern bool OneDirection;

extern int WaitState;

extern int ShowGameScreen;

extern char LASTSAVEFILE[64];

extern uint8_t * NPresence;
extern bool PreNoPause;
extern bool VotingMode;

void InitEBuf();

int CreateNewTerrMons2( uint8_t NI, int x, int y, uint16_t Type );

void CmdSendToXY( uint8_t NI, int x, int y, short Dir );
void CmdAttackObj(uint8_t NI, uint16_t ObjID, short DIR );
void CmdCreateBuilding(uint8_t NI, int x, int y, uint16_t Type );
void CmdProduceObj( uint8_t NI, uint16_t Type );
void CmdMemSelection( uint8_t NI, uint8_t Index );
void CmdRememSelection( uint8_t NI, uint8_t Index );
void CmdBuildObj( uint8_t NI, uint16_t ObjID );
void CmdRepairWall( uint8_t NI, short xx, short yy );
void CmdTakeRes( uint8_t NI, int x, int y, uint8_t ResID );
void CmdPerformUpgrade( uint8_t NI, uint16_t UI );
void CmdCreateGoodSelection( uint8_t NI, uint16_t x, uint16_t y, uint16_t x1, uint16_t y1 );
void CmdCreateGoodKindSelection( uint8_t NI, uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint8_t Kind );
void CmdCreateGoodTypeSelection( uint8_t NI, uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint16_t Type );
void CmdSetDst( uint8_t NI, int x, int y );
void CmdStop( uint8_t NI );
void CmdStandGround( uint8_t NI );
void CmdPatrol( uint8_t NI, int x, int y );
void CmdDie( uint8_t NI );
void CmdNucAtt( uint8_t NI, uint8_t x, uint8_t y );
void CmdUnProduceObj( uint8_t NI, uint16_t Type );
void CmdSetRprState( uint8_t NI, uint8_t State );
void CmdChooseSelType( uint8_t NI, uint16_t ID );
void CmdChooseUnSelType( uint8_t NI, uint16_t ID );
void CmdGoToMine( uint8_t NI, uint16_t ID );
void CmdLeaveMine( uint8_t NI, uint16_t Type );
void CmdCreateWall( uint8_t NI );
void CmdChooseSelBrig( uint8_t NI, uint16_t ID );
void CmdChooseUnSelBrig( uint8_t NI, uint16_t ID );
void CmdErseBrigs( uint8_t NI );
void CmdMakeStandGround( uint8_t NI );
void CmdCancelStandGround( uint8_t NI );
void CmdCrBig( uint8_t NI, int i );
void CmdSelBrig( uint8_t NI, uint8_t Type, uint16_t ID );
void CmdFieldBar( uint8_t NI, uint16_t n );
void CmdSetSrVictim( uint8_t NI, uint8_t val );
void CmdMakeReformation( uint8_t NI, uint16_t BrigadeID, uint8_t FormType );
void CmdFillFormation( uint8_t NI, uint16_t BrigadeID );
void CmdDoGroup( uint8_t NI );
void CmdUnGroup( uint8_t NI );

// Unused
void CmdRepair( uint8_t NI, uint8_t x, uint8_t y );
void CmdUnload( uint8_t NI, uint8_t x, uint8_t y );
// End of unused

void CmdGiveMoney( uint8_t SrcNI, uint8_t DstNI, uint8_t Res, int Amount );
void CmdOfferVoting();
void CmdMoney( uint8_t NI );

void DieSelected( uint8_t NI );
void EraseSelected( uint8_t NI );

void ComOpenGates( uint8_t NI );
void ComCloseGates( uint8_t NI );

void ProduceObject( uint8_t NI, uint16_t Type );
void SendSelectedToXY( uint8_t NI, int xx, int yy, short Dir, uint8_t Prio, uint8_t Type );
void PatrolGroup( uint8_t NI, int x1, int y1, uint8_t Dir );

void MakeStandGround( Brigade* BR );
void MakeStandGround( uint8_t NI );
void CancelStandGround( uint8_t NI );
void SetSearchVictim( uint8_t NI, uint8_t Val );

void CmdEndGame( uint8_t NI, uint8_t state, uint8_t cause );
void CmdTorg( uint8_t NI, uint8_t SellRes, uint8_t BuyRes, int SellAmount );
void CmdChangeNatRefTBL( uint8_t * TBL );

void BuildWithSelected( uint8_t NI, uint16_t ObjID, uint8_t OrdType );

void GoToMineWithSelected( uint8_t NI, uint16_t ID );

void CancelStandGroundAnyway( Brigade* BR );

void SelBrigade( uint8_t NI, uint8_t Type, int ID );
void ImSelBrigade( uint8_t NI, uint8_t Type, int ID );

//Cancels queued production of selected unit in selected building
//OneObject *OB: Building with production order to be canceled
//word Type: Object index of the unit type to be canceled
//Iterates through all queued productions in the building
//Unlinks the order with provided unit type from the chain
//Refunds resources spent on unit construction
void UnProduce( OneObject* OB, uint16_t Type );

void ProcessVotingKeys();

//[28][ni][ID:32][Length:8][Name...]
void CmdSaveNetworkGame( uint8_t NI, int ID, const char* Name );
//[29][ni][ID:32][Length:8][Name...]
void CmdLoadNetworkGame( uint8_t NI, int ID, char* Name );

void WaitWithError( const char* ID, int GPID );

int GetLogRank();

void ShowCentralMessage4( const char* Message, int GPIDX, int DX );

void LOOSEIGAME(char* Nick);
void LOOSEANDEXITFAST();
void WINIGAME(char* Nick);

void DontMakeRaiting();
void StopRaiting();
void ProcessUpdate();
bool CheckGameTime();

bool IsGameActive();
void UpdateCurGame();

int GetMyProfile();
void SetBrokenState();

void SetAttState( Brigade* BR, bool Val );

//Process orders from execution buffer
void ExecuteBuffer();

int GetCurGamePtr( uint8_t ** Ptr );

void CmdSetSpeed( uint8_t );
void CmdChangePeaceTimeStage( int Stage );
void CmdGoToTransport( uint8_t NI, uint16_t ID );
void CmdEndPT();
void CmdCreateNewTerr( uint8_t NI, int x, int y, uint16_t ID );
void CmdSelectBuildings( uint8_t NI );
void CmdSelectIdlePeasants( uint8_t NI );
void CmdSelectIdleMines( uint8_t NI );
void CmdPause( uint8_t NI );
void CmdSelAllUnits( uint8_t NI );
void CmdSelAllShips( uint8_t NI );
void CmdVote( uint8_t result );
void CmdSetGuardState( uint8_t NI, uint16_t State );
void CmdAttackGround( uint8_t NI, int x, int y );
void CmdAddMoney( uint8_t NI, uint32_t Value );
void CmdUnSelUnitsSet( uint8_t NI, uint16_t * BUF, int NU );
void CmdCreateGates( uint8_t NI );
void CmdOpenGates( uint8_t NI );
void CmdCloseGates( uint8_t NI );
void CmdStopUpgrade( uint8_t NI );
void CmdUnloadAll( uint8_t NI );
void CmdSetStartTime( int * MASK );
void CmdSetMaxPingTime( int );
void CmdDoItSlow( uint16_t DT );

void ProcessVotingView();

//Stops running upgrade in any building in game
//Made to solve 'captured / destroyed while upgrading' bugs
//OneObject *OB = pointer to instance of the building
void StopUpgradeInBuilding( OneObject *OB );

void PlayStart();
void SetGameID( const char* ID );
void SetChatWState( int ID, int State );

#endif // MULTI_H
