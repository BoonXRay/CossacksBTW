#ifndef GP_DRAW_H
#define GP_DRAW_H

#include <cstdint>

#include "Fonts.h"

#ifndef GP_USER
#define GP_API __declspec(dllexport)
#else
#define GP_API __declspec(dllimport)
#endif

#pragma pack( push, 1 )

typedef unsigned short word;
typedef char* LPCHAR; 
#define MaxGPIdx 3000

extern int LOADED;

class GP_API GP_Header
{
public:
	int NextPict;
	short dx;
	short dy;
	short Lx;
	short Ly;
    uint8_t * Pack;
	char  Options;
    uint32_t CData;
	short NLines;
	int GetLx();
	int GetLy();
	int GetDx();
	int GetDy();
};

struct GP_API GP_GlobalHeader
{
	int		 Sign;
	short	 NPictures;
	short    Reserved;
	int		 VocOffset;
	short    VocLength;
	//GP_Header* LGPH[256];
    uint32_t    LGPH[256];
};
struct OneUnicodeSet{
	int DY;
	int DX;
    uint8_t UseColor;
	int Start;
	int NSymbols;
	int GP_Start;
	int GPID;
};
struct UNICODETABLE{
	int NTables;
	OneUnicodeSet USET[4];
};
struct OneUniFont{
	char FontName[48];
	UNICODETABLE UTBL;
};
class GP_API UNIFONTS{
public:
	OneUniFont* UFONTS;
	int NFonts;
	UNIFONTS();
	~UNIFONTS();
	void LoadFonts();
    UNICODETABLE* FindFont( const char* Name );
};
extern GP_API UNIFONTS UFONTS;
#define NO_PACK ((byte*)0xFFFFFFFF)
typedef GP_GlobalHeader* lpGP_GlobalHeader;

class GP_API GP_System
{
public:
    uint8_t* PackCash;
	int CashSize;
	int CashPos;

	//Current amount of loaded images in this instance
	int NGP;

	int NGPReady;

	//Array of pointers to buffers with loaded resource files
	GP_GlobalHeader** GPH;

    uint32_t** CASHREF;
    uint8_t* Mapping;
	char** GPNames;
	word* GPNFrames;
	RLCTable* RLCImage;
	RLCTable* RLCShadow;
	int* GPSize;
	int* GPLastTime;

	//Type of image resource file
	//0 = filed does not exist
	//1 = GP
	//2 = RLC
	//3 = RLC + Shadow 
    uint8_t* ImageType;

	short** ImLx;
	short** ImLy;
	char** ItDX;
	char** ItLX;
	UNICODETABLE** UNITBL;
	GP_System();
	~GP_System();
    uint8_t* GetCash(int Size);
    int PreLoadGPImage(const char* Name);
    int PreLoadGPImage(const char* Name,bool Shadow);
	bool LoadGP(int i);
	void  UnLoadGP(int i);
	int  GetGPWidth(int i,int n);
    int  GetGPHeight(int i,int n);
    void ShowGP(int x,int y,int FileIndex,int SprIndex,uint8_t Nation);
    void ShowGPLayers(int x,int y,int FileIndex,int SprIndex,uint8_t Nation,int mask);
    void ShowGPTransparent(int x,int y,int FileIndex,int SprIndex,uint8_t Nation);
    void ShowGPTransparentLayers(int x,int y,int FileIndex,int SprIndex,uint8_t Nation,int mask);
    void ShowGPPal(int x,int y,int FileIndex,int SprIndex,uint8_t Nation,uint8_t* Table);
    void ShowGPPalLayers(int x,int y,int FileIndex,int SprIndex,uint8_t Nation,uint8_t* Table,int mask);
    void ShowGPRedN(int x,int y,int FileIndex,int SprIndex,uint8_t Nation,int N);
    void ShowGPDarkN(int x,int y,int FileIndex,int SprIndex,uint8_t Nation,int N);
    void ShowGPDark(int x,int y,int FileIndex,int SprIndex,uint8_t Nation);
    void ShowGPFired(int x,int y,int FileIndex,int SprIndex,uint8_t Nation);
    void ShowGPMutno(int x,int y,int FileIndex,int SprIndex,uint8_t Nation);
    void ShowGPGrad(int x,int y,int FileIndex,int SprIndex,uint8_t Antion,uint8_t* Table);
    void FreeRefs(int i);
};

extern GP_API GP_System GPS;

class GP_API LocalGP
{
public:
	int GPID;
	bool Uniq;
//-----------------//
	LocalGP();
    LocalGP( const char* Name );
	~LocalGP();
    void Load(const char *Name);
	void UnLoad();
};

//-----------colored bar---------------------//
__declspec( dllexport ) void CBar( int x0, int y0, int Lx0, int Ly0, uint8_t c );

void xLine( int x, int y, int x1, int y1, uint8_t c );

extern int UNI_LINEDLY1;
extern int UNI_LINEDY1;
extern int UNI_LINEDLY2;
extern int UNI_LINEDY2;
extern int UNI_HINTDY1;
extern int UNI_HINTDLY1;
extern int UNI_HINTDLY2;

void InitXShift();

__declspec( dllexport ) bool CheckGP_Inside( int FileIndex, int SprIndex, int dx, int dy );

void Init_GP_IMG();
void Clean_GP_IMG();
void RegisterVisibleGP( word Index, int FileIndex, int SprIndex, int x, int y );
uint16_t CheckCoorInGP( int x, int y );
void GetRealGPSize( int File, int spr, int* dx, int* dy );

#pragma pack( pop )

#endif // GP_DRAW_H
