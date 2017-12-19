#ifndef MAPA_H
#define MAPA_H

#include <cstdint>

constexpr uint8_t clrRed = 0xCD;
constexpr uint8_t clrGreen = 0x9B;
constexpr uint8_t clrBlue = 0xCE;
constexpr uint8_t clrYello = 0xFB;
constexpr uint8_t clrBlack = 0;
constexpr uint8_t clrWhite = 0xFF;


extern int LastActionX;
extern int LastActionY;

extern uint16_t BuildingID;
class Nation;
extern Nation * BNat;

#define ULIMIT 65535
class OneObject;
extern OneObject* Group[ULIMIT];

//Some additional offsets
//Proportional to display resolution
//Observed values: 25, 32, 40, 60
extern int smaplx;
extern int smaply;

//Coordinates of the view window of the player
//Range 1 to 421 for normal maps
//Range 1 to 901 for large (2x) maps
//Range 1 to 1861 for huge (4x) maps
extern int	mapx;
extern int	mapy;

extern int msx;
extern int msy;
extern int minix;
extern int miniy;

extern int smapx;
extern int smapy;

extern int MiniLx;
extern int MiniLy;
extern int MiniX, MiniY;

#define maxmap 256//(128<<1)//ADDSH)  //Don't change it!
extern uint8_t minimap[ maxmap ][ maxmap ];

extern bool MiniMade;

extern int SumAccount[8];

extern int PreviewBrig;

constexpr inline int mul3( int x ) noexcept
{
    return x + x;//x+x+x;
}

//divide by 16
constexpr inline int div24( int y ) noexcept
{
    return y / 16;
}

//multiply by 2
constexpr inline int Prop43( int y ) noexcept
{
    return y * 2;
}

constexpr int GridX = 4;
constexpr int GridY = 4;
extern int GridLx;
extern int GridLy;
extern int GridNx;
extern int GridNy;
extern int GridMode;

extern bool ShowStat;
extern bool Tutorial;
extern bool NoPress;
extern bool TutOver;
extern bool MiniActive;

extern int ScrollSpeed;

extern int CBB_GPFILE;

extern uint8_t SpecCmd;

extern int InfAddY;
extern int InfDY;
extern int InfDX;
extern int InfY1;
extern int InfY2;

extern bool NoWinner;

extern int Inform_Var;

extern bool BuildMode;

extern int GameTime;

extern int FirstMissTime;
extern int PrevMissTime;

extern bool RESMODE;
extern bool HaveExComm;

extern bool Recreate;
extern bool NoText;
extern bool unpress;

extern bool GUARDMODE;
extern bool PATROLMODE;

void ProcessScreen();

void ClearMINIMENU();

void HandleMouse( int x, int y );
void GlobalHandleMouse(bool process_scrolling);

class OneObject;
void GetRect( OneObject* ZZ, int* x, int* y, int* Lx, int* Ly );

void MFix();

void NewMap( int szX, int szY );

void CreateINFORM();

void GFieldShow();
void GMiniShow();

void REPLAY();

int ShowUnitsList( int x, int y, int Lx, uint16_t * Value );

int SortPlayers( uint8_t * Res, int* par );

void DrawMarker( OneObject* OB );
void DrawColorMarker( OneObject* OB, uint8_t cl );
void DrawHealth( OneObject* OB );

void ClearUINF();

void SelectNextGridMode();

void ShowCentralText0( const char* sss );

void LoadLock();

__declspec( dllexport ) void SetExComm( char* Data, int size );

#endif // MAPA_H
