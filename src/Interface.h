#ifndef INTERFACE_H
#define INTERFACE_H

#include <cstdint>

extern uint16_t NTextures;
extern uint16_t TexList[128];

extern int TM_Height0;
extern int TM_Height1;

extern bool TexMapMod;

extern int TM_Tan0;
extern int TM_Tan1;

extern int RES[8][8];

extern int RM_LandType;
extern int RM_Resstart;
extern int RM_Restot;

__declspec( dllexport ) extern int LastKey;
__declspec( dllexport ) extern bool KeyPressed;

__declspec( dllexport ) extern int ItemChoose;

extern char PlName[64];
extern bool use_gsc_network_protocol;

extern bool InGame;
extern bool InEditor;

extern int CurrentNation;
extern int PlayMode;

extern int WarSound;
extern int OrderSound;
extern int MidiSound;

extern int menu_x_off;
extern int menu_y_off;

extern bool InMainMenuLoop;

extern int time1, time2, time8;

extern char CurrentMap[64];

extern int LastCTRLPressTime;

extern char IPADDR[128];
extern bool DoNewInet;

extern char MapScenaryDLL[200];
extern char DEFPLNAMES[8][64];

extern uint8_t MPL_NatRefTBL[8];

extern char RECFILE[128];

extern bool GameExit;
extern int MenuType;
extern bool MakeMenu;

extern bool ChangeNation;
extern bool MultiTvar;

extern int Zone1X;
extern int Zone1Y;
extern int Zone1LX;
extern int Zone1LY;
extern int PanelY;
extern int BrigPnX;
extern int BrigPnY;
extern int BrigNx;
extern int BrigNy;

extern int InteriorType;

extern int CurrentCampagin;
extern int CurrentMission;

extern uint8_t MI_Mode;

extern bool OptHidden;
extern bool ToolsHidden;

extern bool DRAWLOCK;

extern int CUR_TOOL_MODE;
extern uint8_t NeedToPopUp;
extern bool TP_Made;

extern char HEIGHTSTR[12];

int GetGSC_Profile();
void ShowLoading();
void CenterScreen();

void ProcessChatKeys();
bool ProcessMessagesEx();

__declspec( dllexport ) void DarkScreen();
void SetGameDisplayModeAnyway( int SizeX, int SizeY );

void SFLB_InitDialogs();

class SimpleDialog;
bool MMItemChoose( SimpleDialog* SD );

uint16_t GetTexture();
int EnterHi( int * val, int Type );

void PrepareGameMedia( uint8_t myid, bool SaveNR );

bool ProcessMessages();

bool EnterStr(const char *str, const char* Message );

__declspec( dllexport )void StdKeys();

void ProcessMissionText( char* Bmp, char* Text );
bool AskMissionQuestion( char* Bmp, char* Text );

int ProcessComplexQuestion( int Nx, char* Bmp1, uint8_t or1, char* Text1, char* Bmp2, uint8_t or2, char* Text2, const char* Quest );

int ShowHistryItem( char* Bmp, char* Text );
void ShowHistory();

void GSYSDRAW();

int ProcessMultilineQuestion( int Nx, char* Bmp1, uint8_t or1, char* Text1, char* Quest );

void RedrawGameBackground();

__declspec( dllexport ) bool EnumPlr();
const char* GetPName( int i );

void ShowCentralMessage( const char* Message, int GPIDX );

class RLCFont;
__declspec( dllexport ) void ShowClanString(int x, int y, char* s, uint8_t State, RLCFont* Fn, RLCFont* Fn1, int DY );
void LimitString(char* str, RLCFont *FONT, int L );
void AddChatString( const char* Nick, char* str, int MaxLx, RLCFont * FONT, char** &ChatMess, char** &ChatSender, int &NCHATS, int &MAXCHATS );

void DRAWBOX( int x, int y, int Lx, int Ly, int /*Idx*/, uint8_t /*Active*/, int param );

int DetermineNationAI( uint8_t Nat );

void DrawZones();

bool ShowStatistics();

void ShowAbout();

bool IngameYesNoDialog( const char* dialog_text );

void PlayGame();

void AllGame();

int GetTreeItem();

bool CheckFNSend( int idx );
void ShowChat();

void LoadMapInterface();
void ProcessMapInterface();

bool CheckFlagsNeed();

void CreateNEWMAP();

void ReadClanData();

__declspec( dllexport ) bool CheckUsingAI();

#endif // INTERFACE_H
