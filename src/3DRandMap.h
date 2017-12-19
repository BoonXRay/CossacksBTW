#ifndef _3DRANDMAP_H
#define _3DRANDMAP_H

#include <cstdint>

#include "ResFile.h"

class OneObject;

struct MineRec
{
    char * Name;
    int Ng, Ni, Nc;
};
struct ResRec
{
    char * Name;
    int RES[8];
};
struct PlRec
{
    int NPlayers;
    char * name;
};
struct StyleRec
{
    char * Name;
    char * Style;
    int NPl;
    int AI_Style;
    PlRec * Players;
};
class RandomMapDesc
{
    public:
        int NMINES;
        int MINES_DEF;
        MineRec * MINES;
        int NRES;
        int RES_DEF;
        ResRec * RES;
        int NRelief;
        int Relief_DEF;
        char ** Relief;
        int NSTY;
        int STY_DEF;
        StyleRec * STY;
        RandomMapDesc();
        ~RandomMapDesc();
        void Close();
        void Load( const char * Name );
};
class GlobalProgress
{
    public:
        int NWeights;
        int StageID[64];
        int StageWeight[64];
        int StagePositions[64];
        int CurStage;
        int CurPosition;
        int MaxPosition;
        void Setup();
        void AddPosition( int ID, int Weight, int Max );
        void SetCurrentStage( int ID );
        void SetCurrentPosition( int Pos );
        int GetCurProgress();
        GlobalProgress();
};
extern GlobalProgress GPROG;

int mrand();

void CheckFirstLine();

void GenerateMapForMission( const char * Relief, const char * Ground, const char * Mount, const char * Soft );

extern uint8_t * NatDeals;

void GenShow();

void CreateNationalMaskForRandomMap( const char * Name );
void CheckMapName( char * Name );
void CheckMapNameForStart( char * Name );

extern bool ImmVis;

void RM_Load( char * Name, int x, int y );
void ProcessRM_Load();
void ProcessRM_LoadEx( char * Name );
void ProcessRM_Save( int x0, int y0, int x1, int y1 );

extern bool TexPieceMode;
void ProcessTexPiece( int x, int y, bool press );

void DeleteInArea( int x0, int y0, int x1, int y1 );

void ProcessSaveInSquares();

extern int PeaceTimeLeft;
extern int MaxPeaceTime;
extern int PeaceTimeStage;

extern uint8_t BalloonState;
extern uint8_t CannonState;
extern uint8_t NoArtilleryState;
extern uint8_t XVIIIState;
extern uint8_t CaptState;
extern uint8_t SaveState;
extern uint8_t DipCentreState;
extern uint8_t ShipyardState;
extern uint8_t MarketState;

extern int SqMaxX;
extern int SqMaxY;
extern int SqDX;
extern int SqDY;

void ShowProgressBar( const char * Message, int N, int NMax );

extern uint8_t CordonIDX[8];

int GetUnitActivity( OneObject * OB );
bool CheckBuildPossibility( uint8_t NI, int x, int y );

void HandleSMSChat( const char * Mess );
void HandlwSMSMouse();
void AutoSMSSet();
void DrawSMS();
void SaveSMSInMap( ResFile F );
void ClearSMS();
void LoadSMSInMap( ResFile F );



#endif // _3DRANDMAP_H
