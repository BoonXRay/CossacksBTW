#ifndef LOADSAVE_H
#define LOADSAVE_H

#include <cstdint>

#include "MapDescr.h"
#include "ResFile.h"

extern int ADDSH;

class SaveBuf{
public:
    uint8_t * Buf;
	int Pos;
	int Size;
	int RealSize;
	SaveBuf();
	void Init();
	~SaveBuf();
	void Clear();
	void SaveToFile(ResFile f1);
	void LoadFromFile(ResFile f1);
};

void SFLB_PreLoadGame(SaveBuf* SB,bool LoadNation);
void SFLB_LoadGame(char* fnm,bool LoadNation);
void SaveGame( const char* Name, const char* Messtr,int ID);
void xBlockRead(SaveBuf* SB,void* Data,int Size);
void xBlockWrite(SaveBuf* SB, const void* Data,int Size);

void InitDestn();
void AddDestn( int x, int y );
void ShowDestn();
void ShowMiniDestn();
void AddPulse( uint16_t ObjID, uint8_t c );
void AddXYPulse( int x, int y );
void ShowPulse();

void SetupArrays();
void FreeArrays();
void UnLoading();

void SetFractalTexture();

extern OneObject OBJECTS[ULIMIT];

extern int TopLx;
extern int TopLy;
extern int MaxTop;
extern int TopSH;
extern int StratLx;
extern int StratLy;
extern int StratSH;

extern int SafeMLx;
extern int SafeMSH;
extern int TSH;

extern int LastTimeStage;
extern int StartTmtmt;

extern int CURTMTMT;

int GetOrderKind( ReportFn * RF );

extern int WLX;

extern char GameName[128];

extern int sfVersion;

extern char PL_Names[8][32];
extern uint8_t PL_Colors[8];
extern int PL_NPlayers;
extern uint8_t PL_NatRefTBL[8];

extern int LX_fmap;

extern int VAL_SHFCX;
extern int VAL_MAXCX;
extern int VAL_MAXCIOFS;
extern int VAL_SPRNX;
extern int VAL_SPRSIZE;
extern int VAL_MAPSX;

extern int WMPSIZE;
extern int MAPSX;
extern int MAPSY;
extern int BMSX;

extern int MAXCIOFS;
extern int TSX;

extern int MaxLI;
extern int MaxLIX;
extern int MaxLIY;

//Maximum size of cells map
extern int MAXCX;
#define MAXCY MAXCX

extern int RivNX;
extern int RivSH;

extern SelGroup SelSet[80];

int GetMapSUMM( char* Name );

void CreateMaskForSaveFile( char* );

uint16_t GetV_fmap( int x, int y );

inline int GetLI( int x, int y )
{
    return x + ( y << ( VAL_SHFCX + 1 ) );
}

uint16_t GetNMSL( int i );
void SetNMSL(int i, uint16_t W );
void CleanNMSL();

#endif // LOADSAVE_H
