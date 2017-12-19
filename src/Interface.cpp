#define INITGUID

//If you don't include UdpHolePuncher.h first,
//it's winsock includes will mess up the project >_<
#include "NewCode/UdpHolePuncher.h"

#include <cassert>
#include <cmath>

#include <dplobby.h>

#include "Arc/GSCTypes.h"
#include "IntExplorer/Explore.h"

#include "3DGraph.h"
#include "3DMapEd.h"
#include "3DRandMap.h"
#include "ActiveScenary.h"
#include "ActiveZone.h"
#include "BmpTool.h"
#include "BoonUtils.hpp"
#include "ConstStr.h"
#include "Ddini.h"
#include "DeviceCD.h"
#include "Dpchat.h"
#include "DrawForm.h"
#include "Economy.h"
#include "EInfoClass.h"
#include "ExConst.h"
#include "FastDraw.h"
#include "Fog.h"
#include "GameSound.h"
#include "GP_Draw.h"
#include "Hint.h"
#include "IntExplorer/DataXchange.h"
#include "LoadSave.h"
#include "main.h"
#include "Mapa.h"
#include "MapSprites.h"
#include "Mouse_X.h"
#include "MPlayer.h"
#include "Multi.h"
#include "Nation.h"
#include "Nature.h"
#include "NewCode/MapOptionsEncoder.h"
#include "NewAI.h"
#include "NewMon.h"
#include "NewUpgrade.h"
#include "RealWater.h"
#include "Recorder.h"
#include "SaveNewMap.h"
#include "SelProp.h"
#include "ShipTrace.h"
#include "stRecog.h"
#include "TopoGraf.h"
#include "UnSyncro.h"
#include "Weapon.h"

#include "Interface.h"


static UdpHolePuncher udp_hole_puncher;

//Was used to distinguish between GameSpy and GSC-Game.Net
//Probably obsolete, but could also be some sort of "game active" indicator
bool use_gsc_network_protocol = false;

static RoomInfo GlobalRIF;

int GetGSC_Profile()
{
	return GlobalRIF.Profile;
}

int CurrentNation = -1;
int time1, time2, time8;
char CurrentMap[64];
int RES[8][8];

int LastKey;
bool KeyPressed;

int WarSound;
int OrderSound;
int MidiSound;

bool InGame;
bool InEditor;

int LastCTRLPressTime = 0;

static constexpr int MinPing0 = 50;

//Selected network protocol for multiplayer
//0 = IPX Network
//1 = TCP/IP LAN Connection
//2 = Direct TCP/IP
//3 = GSC-Game.Net Internet Game
//Set in MPL_ChooseConnection()
//When (3): use_gsc_network_protocol = true
static int selected_network_protocol = 0;

static void SFLB_CreateGamesList( ListBox* LB );
static bool WaitingHostGame( int );
static bool WaitingJoinGame( int );
static void ChooseInterior( int Type );
static void EditMiniMap();
static void processMLoadGame();
static void PrepareEditMedia(uint8_t myid );

static bool IgnoreSlow = false;
static bool ContinueGame;
static int nsmaplx;
static int nsmaply;
static int GameMode;

static RLCFont FPassive;
static RLCFont FActive;
static RLCFont FDisable;

static bool GameNeedToDraw;

static int PrevProgStage = 0;
static const char* PROGSTR = nullptr;


static void ShowProgStr( char* ss, int prog )
{
	if ( CurPalette )
	{
		LocalGP LOAD( "Interface\\_Loading" );
		GPS.ShowGP( ( RealLx - GPS.GetGPWidth( LOAD.GPID, 0 ) ) / 2, ( RealLy - GPS.GetGPHeight( LOAD.GPID, 0 ) ) / 2, LOAD.GPID, 0, 0 );
	}
	else
	{
		DrawStdBar( ( RealLx - 330 ) / 2 + 20, ( RealLy - 70 ) / 2 + 18, ( RealLx - 330 ) / 2 + 318, ( RealLy - 70 ) / 2 + 55 );
    }
	int L = strlen( ss );
	L = ( L*prog ) / 100;
	int x0 = ( RealLx - GetRLCStrWidth( ss, &YellowFont ) ) / 2 + 1 + 5;
	int y0 = ( RealLy - GetRLCHeight( YellowFont.RLC, 'W' ) ) / 2 + 1;
	int spos = 0;
	do
	{
		if ( spos <= L )ShowCharUNICODE( x0, y0, (byte*) ( ss + spos ), &WhiteFont );
		else ShowCharUNICODE( x0, y0, (byte*) ( ss + spos ), &YellowFont );
		int LL;
		x0 += GetRLCWidthUNICODE( YellowFont.RLC, (byte*) ( ss + spos ), &LL );
		spos += LL;
	} while ( ss[spos] );
	CopyToScreen( 0, 0, RealLx, RSCRSizeY );
}

static int PrevT = 0;

static void ShowOptMessage( const char* ID, int v )
{
    const char* str = GetTextByID( ID );
	if ( str[0] != '#' )
	{
		char buf[512];
		sprintf( buf, str, v );
		CreateTimedHintEx( buf, kSystemMessageDisplayTime, 32 );//System messages through ShowOptMessage()
	}
}

static bool NeedToPerformGSC_Report = 0;
static bool NeedToReportInGameStats = 0;
static int PrevReportTime = 0;
static int LastTimeReport_tmtmt = 0;

static void EndGSC_Reporting()
{
	NeedToReportInGameStats = 0;
	NeedToPerformGSC_Report = 0;
	LeaveGSCRoom();
}

static bool PDIF_Inside = 0;
static int PrevProcTime = 0;
//--------------------inserted from DIPLOMACY.H---------------//

//-----------------------------------------------------------//

bool ProcessMessages()
{
	if ( PDIF_Inside )
	{
		return false;
	}

	int TT = GetTickCount();

	if ( !PrevProcTime )
	{
		PrevProcTime = TT;
	}

	if ( TT - PrevProcTime > 3000 )
	{
		PDIF_Inside = 1;
        ProcessDownloadInternetFiles();
        ProcessSXP( 16, nullptr );
		PDIF_Inside = 0;
		PrevProcTime = TT;
	}

	if ( NeedToPerformGSC_Report )
	{
		int T = GetTickCount();
		if ( !PrevReportTime )
		{
			PrevReportTime = T;
		}
		if ( T - PrevReportTime > 120000 )
		{
			int NZeros = 0;
			int PROFS[8];
			for ( int i = 0; i < NPlayers; i++ )
			{
				if ( !PINFO[i].ProfileID )
				{
					NZeros++;
				}
				PROFS[i] = PINFO[i].ProfileID;
			}
			if ( !NZeros )
			{
				ReportAliveState( NPlayers, PROFS );
				PrevReportTime = T;
			}
		}
	}

	if ( NeedToReportInGameStats && use_gsc_network_protocol )
	{
		if ( tmtmt - LastTimeReport_tmtmt >= 256 )
		{
            int NMN[8] = { 0,0,0,0,0,0,0 };
			for ( int i = 0; i < MAXOBJECT; i++ )
			{
				OneObject* OB = Group[i];
				if ( OB && ( OB->Hidden || !OB->Sdoxlo ) && ( !OB->NewBuilding ) )
				{
					NMN[OB->NNUM]++;
				}
			}

			OnePlayerReport OPR[8];
			for ( int i = 0; i < NPlayers; i++ )
			{
				if ( PINFO[i].PlayerID == MyDPID )
				{
					int NI = PINFO[i].ColorID;
					int NP = 0;
					int NW = 0;
					int NN = NATIONS[NI].NMon;
					for ( int j = 0; j < NN; j++ )
					{
						NewMonster* NM = NATIONS[NI].Mon[j]->newMons;
						if ( NM->Peasant )
						{
							NP += NATIONS[NI].NProduced[j];
						}
						else
						{
							if ( !NM->Building )
							{
								NW += NATIONS[NI].NProduced[j];
							}
						}
					}

					OPR[0].NBornP = NP;
					OPR[0].NBornUnits = NW;
					OPR[0].Population = NMN[PINFO[i].ColorID];
					OPR[0].Profile = PINFO[i].ProfileID;
					for ( int p = 0; p < 6; p++ )
					{
						OPR[i].ReachRes[p] = NATIONS[NI].ResTotal[p];
					}
					OPR[0].Score = CITY[NI].Account / 100;
					OPR[0].State = NATIONS[NI].VictState;
					for ( int q = 0; q < 6; q++ )
					{
						OPR[0].ReachRes[q] = NATIONS[NI].ResTotal[q];
					}
				}
			}
			ReportGSCGame( tmtmt, NPlayers, OPR );
			LastTimeReport_tmtmt = tmtmt;
		}
	}

	try
	{
		int T1 = GetTickCount();
		if ( ( !PrevT ) || T1 - PrevT > 10 )
		{
			try
			{
                ProcessNewInternet();
			}
			catch ( ... )
			{
			}

            //int TT0 = GetTickCount() - T1;

			if ( GetKeyState( VK_CONTROL ) & 0x8000 )
			{
				LastCTRLPressTime = GetTickCount();
			}

            int TT0 = GetTickCount();

			try
			{
				ProcessReceive();
			}
			catch ( ... )
			{
			}

			TT0 = GetTickCount() - TT0;

			try
			{
				ProcessNetCash();
			}
			catch ( ... )
			{
			}

			PrevT = T1;

			/*
			if (bActive)
			{
				LastBActiveTime = T1;
			}
			else
			{
				if (GameInProgress && NPlayers > 1 && T1 - LastBActiveTime > 10000)
				{
					if (NATIONS[MyNation].VictState != 2)
					{
						IAmLeft();
						SpecCmd = 199;
						ShowOptMessage("#ALTTABDEFEAT", 0);//IMPORTANT: Alt tab defeat procedure
					}
				}
			}
			*/
		}

		BOOL ret = FALSE;
		MSG msg;
		while ( FALSE != ( ret = PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) ) )
		{
			if ( -1 == ret )
			{
				continue;
			}
			if ( msg.message == WM_QUIT )
			{
				return true;
			}
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		if ( GPROG.NWeights )
		{
			if ( ( !PrevProgStage ) || GetTickCount() - PrevProgStage > 300 )
			{
				PrevProgStage = GetTickCount();
				if ( !PROGSTR )
				{
					PROGSTR = GetTextByID( "PROGSTR%s" );
				}
				char ccx[128];

				char tmp[32];
				DWORD CID = GPROG.StageID[GPROG.CurStage];

				tmp[0] = CID >> 24;
				tmp[1] = ( CID >> 16 ) & 255;
				tmp[2] = ( CID >> 8 ) & 255;
				tmp[3] = ( CID ) & 255;
				tmp[4] = ' ';
				tmp[5] = 0;

				int cpr = GPROG.GetCurProgress();
				sprintf( tmp + 5, "%d%%", cpr );
				sprintf( ccx, PROGSTR, tmp );
				ShowProgStr( ccx, cpr );
			}
		}
	}
	catch ( ... )
	{
	}

	return false;
}

bool ProcessMessagesEx()
{
	if ( GetKeyState( VK_CONTROL ) & 0x8000 )
	{
		LastCTRLPressTime = GetTickCount();
	}

	ProcessVotingKeys();

	if ( EnterChatMode )
	{
		ProcessChatKeys();
	}

	MSG msg;

	bool MSR = 0;
	while ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
	{
		if ( msg.message == WM_QUIT )
		{
			return true;
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		MSR = 1;
	}
	return MSR;
}

char PlName[64];
static void ProcessHelp();
static void ProcessCampagins( int Options );

static void SFLB_ReadFirstPlayerName( char* ccc );
static void SFLB_LoadMEDButtons();

//Read players.txt, load dialog resources
void SFLB_InitDialogs()
{
	SFLB_ReadFirstPlayerName( PlName );
	unsigned long dws = 20;
	if ( !PlName[0] )
	{
		GetComputerName( PlName, &dws );
		normstr( PlName );
	}
	FPassive = YellowFont;
	FActive = WhiteFont;
	FDisable = YellowFont;
	SFLB_LoadMEDButtons();
}


int ItemChoose;

bool MMItemChoose( SimpleDialog* SD )
{
	ItemChoose = SD->UserParam;
	Lpressed = false;
	return true;
}

void StdKeys()
{
	if ( KeyPressed && ( LastKey == 27 || LastKey == 13 ) )
	{
		if ( LastKey == 13 )
		{
			ItemChoose = mcmOk;
		}

		if ( LastKey == 27 )
		{
			ItemChoose = mcmCancel;
		}

		KeyPressed = 0;
		LastKey = 0;
	}
}

static void InstallMap( ListBox* LB, char* Name )
{
	LB_SORT_ITEM = 1;
	LB->AddItem( Name, 0 );
	LB_SORT_ITEM = 0;
}

static void CreateMapsList( ListBox* LB )
{
	TGSCFindInfo* FD = GSFILES.gFindFirst( "*.m3d" );
	if ( FD )
	{
		InstallMap( LB, FD->m_FileName );
		while ( GSFILES.gFindNext( FD ) )InstallMap( LB, FD->m_FileName );
    }
}

//----------------Graphics menu----------------//

static void GSSetup800();
static void DrawAllScreen();

void DarkScreen()
{
	byte* sptr = (byte*) ScreenPtr;
	int sz = ScrWidth * RealLy;
	for ( int i = 0; i < sz; i++ )
	{
		sptr[i] = fog[4096 + 1024 + sptr[i]];
	}

	int ofs = 0;
	int ry = RealLy >> 1;
	for ( int iy = 0; iy < ry; iy++ )
	{
		ofs = ScrWidth*iy * 2;
		for ( int ix = 0; ix < RealLx; ix += 2 )
		{
			sptr[ofs] = 0;
			ofs += 2;
        }
		ofs = ScrWidth*( iy * 2 + 1 ) + 1;
		for ( int ix = 0; ix < RealLx; ix += 2 )
		{
			sptr[ofs] = 0;
			ofs += 2;
		}
	}
}

void ShowLoading()
{
	DarkScreen();

	if ( CurPalette )
	{
		LocalGP LOAD( "Interface\\_Loading" );
		GPS.ShowGP( ( RealLx - GPS.GetGPWidth( LOAD.GPID, 0 ) ) / 2,
			( RealLy - GPS.GetGPHeight( LOAD.GPID, 0 ) ) / 2, LOAD.GPID, 0, 0 );
	}
	else
	{
		DrawStdBar( ( RealLx - 330 ) / 2 + 20, ( RealLy - 70 ) / 2 + 18,
			( RealLx - 330 ) / 2 + 318, ( RealLy - 70 ) / 2 + 55 );
	}

    const char* ss = GetTextByID( "LOADING" );
	ShowString( ( RealLx - GetRLCStrWidth( ss, &YellowFont ) ) / 2 + 1 + 5,
		( RealLy - GetRLCHeight( YellowFont.RLC, 'W' ) ) / 2 + 1, ss, &BlackFont );
	ShowString( ( RealLx - GetRLCStrWidth( ss, &YellowFont ) ) / 2 + 5,
		( RealLy - GetRLCHeight( YellowFont.RLC, 'W' ) ) / 2, ss, &YellowFont );

	CopyToScreen( 0, 0, RealLx, RSCRSizeY );
}

//Changes resolution
void SetGameDisplayModeAnyway( int SizeX, int SizeY )
{
	ClearScreen();
	RealLx = SizeX;
	RealLy = SizeY;

	InitScreen();
	GSSetup800();
	DrawAllScreen();

	if ( window_mode )
	{
		ResizeAndCenterWindow();
	}
}

//Changes resolution
static bool SetGameDisplayMode( int SizeX, int SizeY )
{
	ClearScreen();

	RealLx = SizeX;
	RealLy = SizeY;
	exRealLx = RealLx;
	exRealLy = RealLy;

	InitScreen();
	GSSetup800();
	DrawAllScreen();

	if ( window_mode )
	{
		ResizeAndCenterWindow();
	}

	return true;
}

//-----------------SINGLE PLAYER---------------//

static char ROOMNAMETOCONNECT[128];

static bool ProcessOneBattle( int BtlID );

//Necessary for menu positioning in fullscreen. Export for IChat.dll
int menu_x_off = 0;
int menu_y_off = 0;
static int menu_hint_x = 18;
static int menu_hint_y = 701;

static bool MPL_JoinGame( int ID )
{
	LocalGP BTNS( "Interface\\Game_Select" );
	LocalGP FONT1( "romw" );
	LocalGP FONT2( "romw1" );
	RLCFont FontA( FONT2.GPID );
	RLCFont FontP( FONT1.GPID );
	LoadFog( 2 );
	SQPicture Back( "Interface\\Game_Select_Background.bmp" );
	DialogsSystem MENU( menu_x_off, menu_y_off );
	MENU.addPicture( nullptr, 0, 0, &Back, &Back, &Back );
	GP_Button* Join = MENU.addGP_Button( nullptr, 93, 594, BTNS.GPID, 0, 1 );
	Join->OnUserClick = &MMItemChoose;
	Join->UserParam = mcmJoin;
	GP_Button* Refresh = MENU.addGP_Button( nullptr, 238, 594, BTNS.GPID, 2, 3 );
	Refresh->OnUserClick = &MMItemChoose;
	Refresh->UserParam = mcmRefresh;
	GP_Button* Cancel = MENU.addGP_Button( nullptr, 384, 594, BTNS.GPID, 4, 5 );
	Cancel->OnUserClick = &MMItemChoose;
	Cancel->UserParam = mcmCancel;
	ListBox* LB = MENU.addGP_ListBox( nullptr, 92, 172, 15, BTNS.GPID, 6, 26, &WhiteFont, &YellowFont, nullptr );
	LBEnumerateSessions( LB, ID );
	LB->CurItem = -1;
	LB->M_Over = 9;
	if ( LB->NItems )
	{
		LB->CurItem = 0;
	}
StartJoin:;
	int pp = 1;
	do
	{
		ItemChoose = -1;
		do
		{
			ProcessMessages();
			if ( LB->CurItem == -1 )
			{
				Join->Enabled = false;
			}
			else
			{
				Join->Enabled = true;
			}
			MENU.MarkToDraw();
			MENU.ProcessDialogs();
			for ( int i = 0; i < LB->NItems; i++ )
			{
				char nps[16];
				int ppr = ( LB->GetItem( i )->Param1 ) & 65535;
				sprintf( nps, "%d/%d", ppr & 255, ppr >> 8 );
				if ( LB->CurItem == i )
				{
					ShowString( LB->x + 364, LB->y + 6 + i * 26, nps, &WhiteFont );
				}
				else
				{
					ShowString( LB->x + 364, LB->y + 6 + i * 26, nps, &YellowFont );
				}
			}

			MENU.RefreshView();

			if ( ItemChoose == mcmRefresh )
			{
				LBEnumerateSessions( LB, ID );
				ItemChoose = -1;
				if ( !LB->NItems )
				{
					LB->CurItem = -1;
				}
			}

			if ( pp )
			{
				SlowLoadPalette( "2\\agew_1.pal" );
				pp = 0;
			}
		} while ( ItemChoose == -1 );

		if ( ItemChoose == mcmJoin && LB->CurItem != -1 )
		{
			int BID = LB->GetItem( LB->CurItem )->Param1 >> 16;
			if ( BID )
			{
				SlowUnLoadPalette( "2\\agew_1.pal" );
				if ( !ProcessOneBattle( BID - 1 ) )
				{
					SlowUnLoadPalette( "2\\agew_1.pal" );
					goto StartJoin;
				}
			}
			if ( JoinNameToSession( LB->CurItem, PlName ) )
			{
				if ( WaitingJoinGame( LB->GetItem( LB->CurItem )->Param1 >> 16 ) )
				{
					return true;
				}
				else
				{
					ItemChoose = mcmCancel;
				}
			}
		}
	} while ( ItemChoose != mcmCancel );
	SlowUnLoadPalette( "2\\agew_1.pal" );
	return false;
}

//--------------------HOST THE SESSION---------------------//
char IPADDR[128] = "";
bool DoNewInet = 0;
int GMMAXPL = 7;

static int MPL_ChooseConnection();
static void WaitWithMessage( const char* Message );
static int ProcessInternetConnection( bool Active );
static bool EnterName();

static bool historical_battle_over_gsc_network = false;
static bool TOTALEXIT = 0;

//Prepares multiplayer Deathmatch lobby
//Also handles lobbies for Historical Battles over GSC network
static bool processMultiplayer()
{
	byte AddrBuf[128];
	memset( AddrBuf, 0, 128 );

	int connection_menu_result = 0;

RetryConn:
	if ( historical_battle_over_gsc_network )
	{
		//Skip network protocol selection
		goto REC3;
	}

	connection_menu_result = MPL_ChooseConnection();

	if ( TOTALEXIT )
	{
		return 0;
	}

	if ( connection_menu_result == mcmCancel )
	{
		return 0;
	}

	if ( selected_network_protocol == 3 )
	{
		//GSC-Game.Net Network Game
	REC3:
		//Show login menu
		if ( !ProcessNewInternetLogin() )
		{
			return false;
		}

	REINCONN:
		int r = ProcessInternetConnection( 1 );
		if ( !r )
		{
			return 0;
		}
		if ( r == 2 )
			connection_menu_result = 10;//Join
		if ( r == 1 )
			connection_menu_result = 11;//Host Deathmatch
		if ( r == 3 )
			connection_menu_result = 13;//Host Historical Battle
	}
	else
	{
		if ( !EnterName() )
		{
			return 0;
		}
	}

	DoNewInet = 0;
	if ( selected_network_protocol > 2 )//Not TCP/IP
	{
		DoNewInet = 1;
	}

	if ( !DoNewInet )
	{
		if ( !lpDirectPlay3A )
		{
			CreateMultiplaterInterface();
			if ( !lpDirectPlay3A )
			{
				return 0;
			}
		}

		LPDIRECTPLAYLOBBYA	lpDPlayLobbyA = nullptr;
		LPDIRECTPLAYLOBBY2A	lpDPlayLobby2A = nullptr;

		if FAILED( DirectPlayLobbyCreate( nullptr, &lpDPlayLobbyA, nullptr, nullptr, 0 ) )
		{
			return 0;
		}

		// get ANSI DirectPlayLobby2 interface
		HRESULT hr = lpDPlayLobbyA->QueryInterface( IID_IDirectPlayLobby2A, (LPVOID *) &lpDPlayLobby2A );
		if FAILED( hr )
		{
			return 0;
		}

		// don't need DirectPlayLobby interface anymore
		lpDPlayLobbyA->Release();
		lpDPlayLobbyA = nullptr;

		DPCOMPOUNDADDRESSELEMENT	addressElements[3];
		DWORD sz = 128;

		if ( selected_network_protocol == 1 )
		{
			//TCP/IP LAN Connection
			IPADDR[0] = 0;
		}

		addressElements[0].guidDataType = DPAID_ServiceProvider;
		addressElements[0].dwDataSize = sizeof( GUID );
		addressElements[0].lpData = (LPVOID) &DPSPGUID_TCPIP;
		addressElements[1].guidDataType = DPAID_INet;
		addressElements[1].dwDataSize = strlen( IPADDR ) + 1;
		addressElements[1].lpData = (LPVOID) IPADDR;

		lpDPlayLobby2A->CreateCompoundAddress( addressElements, 2, AddrBuf, &sz );
		lpDPlayLobby2A->Release();

		CloseMPL();
		CreateMultiplaterInterface();

		HRESULT HR = lpDirectPlay3A->InitializeConnection( AddrBuf, 0 );
		if ( FAILED( HR ) )
		{
			goto RetryConn;
		}
	}
	else
	{
		CloseMPL();
		CreateMultiplaterInterface();
	}

	switch ( connection_menu_result )
	{
	case mcmHost:
		if ( CreateNamedSession( PlName, 0, GMMAXPL ) )
		{
			WaitingHostGame( 0 );
		}
		break;

	case mcmJoin:
		MPL_JoinGame( 0 );
		break;

	case 11://Inet Host(Deathmatch)
		PlayerMenuMode = 1;
		if ( CreateSession( GlobalRIF.Name, GlobalRIF.Nick, 0, DoNewInet, GlobalRIF.MaxPlayers ) )
		{
			NeedToPerformGSC_Report = 1;

			//Pass all necessary data and prepare UdpHolePuncher
			udp_hole_puncher.Init( GlobalRIF.udp_server, GlobalRIF.port, GlobalRIF.udp_interval,
				GlobalRIF.player_id, ACCESS );

			WaitingHostGame( 0 );
			NeedToPerformGSC_Report = 0;
			if ( PlayerMenuMode == 1 )
			{
				//need to leave the room there
				LeaveGSCRoom();
				goto REINCONN;
			}
			else
			{
                int Profiles[8];
				char NAT[8][32];
				char* Nations[8];
				int Teams[8];
				int Colors[8];
				for ( int i = 0; i < NPlayers; i++ )
				{
					sprintf( NAT[i], "%d", PINFO[i].NationID );
					Nations[i] = NAT[i];
					Profiles[i] = PINFO[i].ProfileID;
					Teams[i] = PINFO[i].GroupID;
					Colors[i] = PINFO[i].ColorID;
				}
				StartGSCGame( "", PINFO[0].MapName, NPlayers, Profiles, Nations, Teams, Colors );
				NeedToReportInGameStats = 1;
				LastTimeReport_tmtmt = 0;
			}
		}
		else
		{
			LeaveGSCRoom();
			goto REINCONN;
		}
		break;

	case 13:
		PlayerMenuMode = 1;
		goto REINCONN;
		break;

	case 10://Inet Join(Deathmatch)
		PlayerMenuMode = 1;
		strcpy( IPADDR, GlobalRIF.RoomIP );
		if ( !FindSessionAndJoin( ROOMNAMETOCONNECT, GlobalRIF.Nick, DoNewInet, GlobalRIF.port ) )
		{
			LeaveGSCRoom();
			WaitWithMessage( GetTextByID( "ICUNJ" ) );
		}
		else
		{
			WaitingJoinGame( GMTYPE );
		}

		if ( PlayerMenuMode == 1 )
		{
			LeaveGSCRoom();
			goto REINCONN;
		}
		else
		{
            int Profiles[8];
			char NAT[8][32];
			char* Nations[8];
			int Teams[8];
			int Colors[8];
			for ( int i = 0; i < NPlayers; i++ )
			{
                sprintf( NAT[i], "%d", PINFO[i].NationID );
				Nations[i] = NAT[i];
				Profiles[i] = PINFO[i].ProfileID;
				Teams[i] = PINFO[i].GroupID;
				Colors[i] = PINFO[i].ColorID;
			}
			StartGSCGame( "", PINFO[0].MapName, NPlayers, Profiles, Nations, Teams, Colors );
			NeedToReportInGameStats = 1;
			LastTimeReport_tmtmt = 0;
		}
		break;
	}

	return 1;
}

static int ProcessWars();

//Prepares multiplayer lobby for historical battles
void processBattleMultiplayer()
{

TryConnection:

	byte AddrBuf[128];
	memset( AddrBuf, 0, 128 );

	int connection_menu_result = MPL_ChooseConnection();

	if ( TOTALEXIT )
	{
		return;
	}

	if ( connection_menu_result == mcmCancel )
	{
		return;
	}

	if ( selected_network_protocol == 3 )//GSC-Game.Net Internet Game
	{
		historical_battle_over_gsc_network = true;

		processMultiplayer();

		if ( TOTALEXIT )
		{
			return;
		}

		historical_battle_over_gsc_network = false;

		return;
	}

	if ( !EnterName() )
	{
		return;
	}

	int BTLID = 1;

	if ( connection_menu_result == mcmHost )
	{
		BTLID = ProcessWars();
	}

	if ( BTLID == -1 )
	{
		goto TryConnection;
	}

	if ( !lpDirectPlay3A )
	{
		CreateMultiplaterInterface();

		if ( !lpDirectPlay3A )
		{
			return;
		}

		LPDIRECTPLAYLOBBYA	lpDPlayLobbyA = nullptr;
		LPDIRECTPLAYLOBBY2A	lpDPlayLobby2A = nullptr;

		if FAILED( DirectPlayLobbyCreate( nullptr, &lpDPlayLobbyA, nullptr, nullptr, 0 ) )
		{
			return;
		}

		// get ANSI DirectPlayLobby2 interface
		HRESULT hr = lpDPlayLobbyA->QueryInterface( IID_IDirectPlayLobby2A, (LPVOID *) &lpDPlayLobby2A );
		if FAILED( hr )
		{
			return;
		}

		// don't need DirectPlayLobby interface anymore
		lpDPlayLobbyA->Release();
		lpDPlayLobbyA = nullptr;
		DPCOMPOUNDADDRESSELEMENT addressElements[3];
		DWORD sz = 128;

		switch ( selected_network_protocol )
		{
		case 1://TCP/IP LAN Connection
		case 2://Direct TCP/IP
			addressElements[0].guidDataType = DPAID_ServiceProvider;
			addressElements[0].dwDataSize = sizeof( GUID );
			addressElements[0].lpData = (LPVOID) &DPSPGUID_TCPIP;
			addressElements[1].guidDataType = DPAID_INet;
			addressElements[1].dwDataSize = strlen( IPADDR ) + 1;
			addressElements[1].lpData = (LPVOID) IPADDR;
			lpDPlayLobby2A->CreateCompoundAddress( addressElements, 2, AddrBuf, &sz );
			break;
		}

		lpDPlayLobby2A->Release();

		if FAILED( lpDirectPlay3A->InitializeConnection( AddrBuf, 0 ) )
		{
			return;
		}
	}

	switch ( connection_menu_result )
	{
	case mcmHost:
		if ( CreateNamedSession( PlName, BTLID + 1, 2 ) )
		{
			WaitingHostGame( BTLID + 1 );
		}
		break;

	case mcmJoin:
		MPL_JoinGame( BTLID + 1 );
		break;
	}
}

//--------------------MULTIPLAYER GAME---------------------//

//---------------------Map options-------------------------//
char MapScenaryDLL[200] = "";
char DEFPLNAMES[8][64];
//--------------------RESOURCE EDITOR----------------------//
static void MemScreenPart(int x, int y, int lx, int ly, uint8_t *Ptr );

static int ProcessResEdit()
{
	LocalGP BTNS( "Interface\\AI_Res" );
	DialogsSystem DSS( ( RealLx - 532 ) >> 1, ( RealLy - 540 ) >> 1 );
	byte* Back = (byte*) malloc( 532 * 544 );
	MemScreenPart( ( RealLx - 532 ) >> 1, ( RealLy - 540 ) >> 1, 532, 540, Back );
	DSS.addBPXView( nullptr, 0, 0, 532, 540, 1, 1, 1, Back, nullptr );
	DSS.addGPPicture( nullptr, 0, 0, BTNS.GPID, 0 );
	DSS.addGPPicture( nullptr, 0, 0, BTNS.GPID, 1 );
	ComboBox* PLAYER = DSS.addGP_ComboBoxDLX( nullptr, 100, 52, 139, CBB_GPFILE, 0, 9, 9, &WhiteFont, &YellowFont, nullptr );
    const char* cc = GetTextByID( GetTextByID( "$PLAYER%D" ) );
	for ( int i = 0; i < 8; i++ )
	{
		char ccx[128];
		sprintf( ccx, cc, i + 1 );
		PLAYER->AddLine( ccx );
    }
	PLAYER->CurLine = MyNation;
	PLAYER->AddLine( GetTextByID( "$PLAYERALL" ) );
	char ResST[22][16];
	char PLNAMES[9][64];
	for ( int i = 0; i < 8; i++ )strcpy( PLNAMES[i], DEFPLNAMES[i] );
	PLNAMES[8][0] = 0;
	byte RESRID[8];
	int RESLOC[8];
	int NRes = 0;
	for ( int i = 0; i < 8; i++ )
	{
		if ( RDS[i].Enabled )
		{
			_itoa( RES[MyNation][i], ResST[i], 10 );
			RESLOC[i] = RES[MyNation][NRes];
			RESRID[NRes] = i;
			NRes++;
        }
    }
	for ( int i = 0; i < NRes; i++ )
	{
		DSS.addInputBox( nullptr, 112 + 3, 93 + 26 * i, ResST[i], 12, 130, 24, &YellowFont, &WhiteFont );
    }
	InputBox* IBN = DSS.addInputBox( nullptr, 320 + 3, 51 + 1, PLNAMES[MyNation], 32, 160, 24, &YellowFont, &WhiteFont );
	char DLLPT[200];
	strcpy( DLLPT, MapScenaryDLL );
	DSS.addInputBox( nullptr, 161 + 3, 264, DLLPT, 190, 320, 24, &YellowFont, &WhiteFont );
	RandomMapDesc RMP;
	RMP.Load( "Terrain\\Res.dat" );
	ComboBox* CBSTYLE = DSS.addGP_ComboBoxDLX( nullptr, 263, 119, 491 - 263, CBB_GPFILE, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	for ( int p = 0; p < RMP.NSTY; p++ )
	{
		CBSTYLE->AddLine( RMP.STY[p].Name );
    }
	CBSTYLE->CurLine = RM_LandType;
	ComboBox* CBRESST = DSS.addGP_ComboBoxDLX( nullptr, 263, 171, 491 - 263, CBB_GPFILE, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	for ( int p = 0; p < RMP.NRES; p++ )
	{
		CBRESST->AddLine( RMP.RES[p].Name );
    }
	CBRESST->CurLine = RM_Resstart;
	ComboBox* CBRESTOT = DSS.addGP_ComboBoxDLX( nullptr, 263, 223, 491 - 263, CBB_GPFILE, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	for ( int p = 0; p < RMP.NMINES; p++ )
	{
		CBRESTOT->AddLine( RMP.MINES[p].Name );
    }
	CBRESTOT->CurLine = RM_Restot;
	ItemChoose = -1;
	GP_Button* OkBtn = DSS.addGP_Button( nullptr, 79, 319, BTNS.GPID, 5, 4 );
	OkBtn->OnUserClick = &MMItemChoose;
	OkBtn->UserParam = mcmOk;
	GP_Button* CancelBtn = DSS.addGP_Button( nullptr, 300, 319, BTNS.GPID, 3, 2 );
	CancelBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	int prevNat = MyNation;
	do
	{
		if ( PLAYER->CurLine != prevNat )
		{
			prevNat = PLAYER->CurLine;
			if ( prevNat < 8 )
			{
				for ( int i = 0; i < NRes; i++ )
				{
					RESLOC[RESRID[i]] = RES[prevNat][RESRID[i]];
					sprintf( ResST[i], "%d", RESLOC[RESRID[i]] );
                }
            }
            // BoonXRay 20.08.2017
            //IBN->Str = PLNAMES[prevNat];
            IBN->String = PLNAMES[prevNat];
        }
		for ( int i = 0; i < NRes; i++ )
		{
			RESLOC[RESRID[i]] = atoi( ResST[i] );
        }
		ProcessMessages();
		DSS.ProcessDialogs();
		DSS.RefreshView();
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
		if ( PLAYER->CurLine == 8 )
		{
			for ( int i = 0; i < 8; i++ )
			{
				RES[i][0] = RESLOC[0];
				RES[i][1] = RESLOC[3];
				RES[i][2] = RESLOC[2];
				RES[i][3] = RESLOC[1];
				RES[i][4] = RESLOC[4];
				RES[i][5] = RESLOC[5];
            }
		}
		else
		{
			RES[prevNat][0] = RESLOC[0];
			RES[prevNat][1] = RESLOC[3];
			RES[prevNat][2] = RESLOC[2];
			RES[prevNat][3] = RESLOC[1];
			RES[prevNat][4] = RESLOC[4];
			RES[prevNat][5] = RESLOC[5];
        }
		if ( ItemChoose == mcmOk )
		{
			RM_LandType = CBSTYLE->CurLine;
			RM_Resstart = CBRESST->CurLine;
			RM_Restot = CBRESTOT->CurLine;
        }
		strcpy( MapScenaryDLL, DLLPT );
    }
	free( Back );
	return ItemChoose;
}

static int TM_Angle0 = 0;
static int TM_Angle1 = 89;
int TM_Tan0;
int TM_Tan1;
int TM_Height0 = 0;
int TM_Height1 = 1024;
bool TexMapMod = false;

static bool SingleOptions();
static bool SelectSingleMission();

static int MM_ProcessSinglePlayer()
{
	use_gsc_network_protocol = false;
	SFLB_LoadPlayerData();
	LoadFog( 2 );
	LocalGP BTNS( "Interface\\Single_Player" );
	SQPicture MnPanel( "Interface\\Background_Single_Player.bmp" );
	DialogsSystem MMenu( menu_x_off, menu_y_off );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;
	int Dy = 27;
    /*Picture* PIC =*/ MMenu.addPicture( nullptr, 0, 0, &MnPanel, &MnPanel, &MnPanel );
	GP_Button* Compagin = MMenu.addGP_Button( nullptr, 76, 140 + 35 + Dy, BTNS.GPID, 0, 1 );
	Compagin->UserParam = 1;
	Compagin->OnUserClick = &MMItemChoose;
	Compagin->Hint = GetTextByID( "SCOMPAGIN" );
	Compagin->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );

	GP_Button* Mission = MMenu.addGP_Button( nullptr, 76, 140 + 82 + 35 + Dy, BTNS.GPID, 2, 3 );
	Mission->UserParam = 2;
	Mission->OnUserClick = &MMItemChoose;
	Mission->Hint = GetTextByID( "SMISSION" );
	Mission->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );

	GP_Button* Random = MMenu.addGP_Button( nullptr, 76, 140 + 35 + 82 * 2 + Dy, BTNS.GPID, 4, 5 );
	Random->UserParam = 3;
	Random->OnUserClick = &MMItemChoose;
	Random->Hint = GetTextByID( "SRANDOM" );
	Random->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );

	GP_Button* Back = MMenu.addGP_Button( nullptr, 76, 140 + 35 + 82 * 3 + Dy, BTNS.GPID, 6, 7 );
	Back->UserParam = 5;
	Back->OnUserClick = &MMItemChoose;
	Back->Hint = GetTextByID( "SBACK" );
	Back->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );
	ItemChoose = -1;
	int pp = 1;
	KeyPressed = false;
	LastKey = 0;
	do
	{
		ProcessMessages();
		if ( KeyPressed&&LastKey == 27 )
		{
			ItemChoose = 5;
			KeyPressed = 0;
        }
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
		if ( pp )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pp = 0;
        }
		if ( ( ItemChoose == 1 || ItemChoose == 2 ) && !CAMPAGINS.NCamp )
			ItemChoose = -1;
	} while ( ItemChoose == -1 );

	if ( ItemChoose == 3 )
	{
		SlowUnLoadPalette( "2\\agew_1.pal" );
		SingleOptions();
	}

	if ( ItemChoose == 1 )
	{
		SlowUnLoadPalette( "2\\agew_1.pal" );
		ProcessCampagins( -1 );
	}

	if ( ItemChoose == 2 )
	{
		SlowUnLoadPalette( "2\\agew_1.pal" );
		SelectSingleMission();
	}

	if ( ItemChoose == 5 )
	{
		SlowUnLoadPalette( "2\\agew_1.pal" );
	}

	return ItemChoose;
}

static int NameChoose = -1;
static bool MMChooseName( SimpleDialog* SD )
{
	NameChoose = SD->UserParam;
	Lpressed = false;
	return false;
}

static void SFLB_ReadFirstPlayerName( char* ccc )
{
	CDGINIT_INIT1();
	CDGINIT_INIT2();
	GFILE* F = Gopen( "players.txt", "r" );
    ccc[0] = 0;
	if ( F )
	{
		ccc[0] = 0;
		int cc = 0;
		int nn = 0;
		while ( !( cc == 0x0A || cc == EOF ) )
		{
			cc = Ggetch( F );
			if ( !( cc == 0x0A || cc == EOF ) )
			{
				ccc[nn] = cc;
				nn++;
			}
		}
		ccc[nn] = 0;
		Gclose( F );
	}
}

static bool ENRETVAL = 0;
static bool EnterName()
{
	ENRETVAL = 0;
	LoadFog( 2 );
	LocalGP BTNS( "Interface\\Player_Name" );
	LocalGP FONT1( "romw1" );
	LocalGP FONT2( "romw" );
	RLCFont Font1( FONT1.GPID );
	RLCFont Font2( FONT2.GPID );
	SQPicture Back( "Interface\\Background_Player_Name.bmp" );
	DialogsSystem MENU( menu_x_off, menu_y_off );
	MENU.UserClickSound = -1;

	MENU.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

	InputBox* IB = MENU.addInputBox( nullptr, 183 - 8, 561 - 15, PlName, 16, 250, 40, &WhiteFont, &YellowFont, 0 );
	IB->Active = true;
	GP_TextButton* TBS[14];
	GPPicture* GPP[14];
	int XL0 = 104 - 12;
	int YL0 = 190 - 23 + 4;

	for ( int i = 0; i < 12; i++ )
	{
		TBS[i] = nullptr;
	}

	GFILE* F = Gopen( "players.txt", "r" );
	int N = 0;
	KeyPressed = 0;
	if ( F )
	{
		char ccc[128];
		do
		{
			ccc[0] = 0;
			int cc = 0;
			int nn = 0;
			while ( !( cc == 0x0A || cc == EOF ) )
			{
				cc = Ggetch( F );
				if ( !( cc == 0x0A || cc == EOF ) )
				{
					ccc[nn] = cc;
					nn++;
				}
			}
			ccc[nn] = 0;
			if ( ccc[0] )
			{
				if ( N < 14 )
				{
					GPP[N] = MENU.addGPPicture( nullptr, XL0, YL0, BTNS.GPID, 7 + ( N % 3 ) );
					GPP[N]->Visible = 0;
					TBS[N] = MENU.addGP_TextButton( nullptr, XL0, YL0, ccc, BTNS.GPID, 4 + ( N % 3 ), &WhiteFont, &YellowFont );
					TBS[N]->FontDy++;
					TBS[N]->OnUserClick = &MMChooseName;
					TBS[N]->UserParam = N;
					TBS[N]->y1 -= 2;
					YL0 += 26;
					N++;
				}
			}
		} while ( ccc[0] && N < 12 );
		Gclose( F );
	}
	int LastName = -1;
	if ( N )
	{
		strcpy( PlName, TBS[0]->Message );
		LastName = 0;
	}

	GP_Button* Accept = MENU.addGP_Button( nullptr, 113, 594, BTNS.GPID, 0, 1 );
	Accept->OnUserClick = &MMItemChoose;
	Accept->UserParam = mcmOk;
	GP_Button* Cancel = MENU.addGP_Button( nullptr, 333, 594, BTNS.GPID, 2, 3 );
	Cancel->OnUserClick = &MMItemChoose;
	Cancel->UserParam = mcmCancel;
	ItemChoose = -1;
	NameChoose = -1;
	int pp = 1;
	KeyPressed = 0;

	do
	{
		ProcessMessages();

		StdKeys();

		MENU.ProcessDialogs();

		MENU.RefreshView();

		if ( NameChoose != -1 )
		{
			strcpy( PlName, TBS[NameChoose]->Message );
			LastName = NameChoose;
			NameChoose = -1;
		}

		Accept->Enabled = strlen( PlName ) != 0;

		if ( !Accept->Enabled&&ItemChoose == mcmOk )
		{
			ItemChoose = -1;
		}

		if ( pp )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pp = 0;
		}

		for ( int i = 0; i < N; i++ )
		{
			GPP[i]->Visible = i == LastName;
		}
	} while ( ItemChoose == -1 );

	if ( ItemChoose == mcmOk )
	{
        /*bool orig = true;

		for ( int i = 0; i < N; i++ )
		{
			if ( !strcmp( PlName, TBS[i]->Message ) )
			{
				orig = false;
			}
        }*/

		GFILE* F = Gopen( "players.txt", "w" );
		if ( F )
		{
			int NN = 0;
			Gprintf( F, "%s\n", PlName );
			NN++;

			for ( int i = 0; i < N; i++ )
			{
				if ( NN < 12 && strcmp( PlName, TBS[i]->Message ) )
				{
					Gprintf( F, "%s\n", TBS[i]->Message );
					NN++;
				}
			}
			Gclose( F );
		}
		SFLB_LoadPlayerData();
		SavePlayerData();
		SlowUnLoadPalette( "2\\agew_1.pal" );
		ENRETVAL = 1;

		return ENRETVAL;
	}
	SlowUnLoadPalette( "2\\agew_1.pal" );

	return ENRETVAL;
}

//Show network protocol menu
static int MPL_ChooseConnection()
{
	LocalGP BTNS( "Interface\\Type_connection" );
	SQPicture Back( "Interface\\Background_Type_Connection.bmp" );

	LoadFog( 2 );

	DialogsSystem MENU( menu_x_off, menu_y_off );

	MENU.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

	int dy1 = 0;
	int dy2 = 0;
	int dy3 = 0;

	dy1 -= 13;
	dy2 -= 13;
	dy3 -= 13;

	//TCP/IP LAN Connection
	GP_Button* TCP = MENU.addGP_Button( nullptr, 100 - 3, 340 - 13 - 26 + dy1, BTNS.GPID, 3, 5 );
	TCP->UserParam = 1;
	TCP->OnUserClick = &MMChooseName;

	//Direct TCP/IP
	GP_Button* TCPDIRECT = MENU.addGP_Button( nullptr, 100 - 3, 340 - 13 + dy1, BTNS.GPID, 12, 14 );
	TCPDIRECT->UserParam = 2;
	TCPDIRECT->OnUserClick = &MMChooseName;

	//GSC-Game.Net Internet Game
	GP_Button* INET = MENU.addGP_Button( nullptr, 100 - 3, 340 + 13 + dy2, BTNS.GPID, 15, 17 );
	INET->UserParam = 3;
	INET->OnUserClick = &MMChooseName;

	//IP address input form
	GPPicture* IP1 = MENU.addGPPicture( nullptr, 25 + 72, 180 + 222 + dy2 + dy3 + 26 + 13, BTNS.GPID, 19 );
	IP1->Visible = 0;
	GPPicture* IP2 = MENU.addGPPicture( nullptr, 230 + 72, 180 + 222 + dy2 + dy3 + 26 + 13, BTNS.GPID, 18 );
	IP2->Visible = 0;
	InputBox* IBIP = MENU.addInputBox( nullptr, 228 + 72, 180 + 222 + dy2 + dy3 + 26 + 13, IPADDR, 64, 260, 20, &YellowFont, &WhiteFont );

	GP_Button* Create = MENU.addGP_Button( nullptr, 93, 396 + 13 + 52 + dy2 + dy3 + 26 + 13, BTNS.GPID, 6, 7 );
	Create->UserParam = mcmHost;
	Create->OnUserClick = &MMItemChoose;

	GP_Button* Join = MENU.addGP_Button( nullptr, 238, 396 + 13 + 52 + dy2 + dy3 + 26 + 13, BTNS.GPID, 8, 9 );
	Join->UserParam = mcmJoin;
	Join->OnUserClick = &MMItemChoose;

	GP_Button* Cancel = MENU.addGP_Button( nullptr, 383, 396 + 13 + 52 + dy2 + dy3 + 26 + 13, BTNS.GPID, 10, 11 );
	Cancel->UserParam = mcmCancel;
	Cancel->OnUserClick = &MMItemChoose;

	ItemChoose = -1;
	NameChoose = 0;
	int pp = 1;

	selected_network_protocol = 1;

	//Wait for user input
	do
	{
		ProcessMessages();

		MENU.ProcessDialogs();

		MENU.RefreshView();

		IBIP->Visible = 0;
		IBIP->Enabled = 0;
		IP1->Visible = 0;
		IP2->Visible = 0;

		if ( NameChoose == 1 )
		{
			//TCP/IP LAN Connection
			TCP->PassiveFrame = 4;
			selected_network_protocol = 1;
		}
		else
		{
			TCP->PassiveFrame = 5;
		}

		if ( NameChoose == 2 )
		{
			//Direct TCP/IP
			TCPDIRECT->PassiveFrame = 13;
			selected_network_protocol = 2;
			IP1->Visible = 1;
			IP2->Visible = 1;
			IBIP->Visible = 1;
			IBIP->Enabled = 1;
		}
		else
		{
			TCPDIRECT->PassiveFrame = 14;
		}

		if ( NameChoose == 3 )
		{
			//GSC-Game.Net Network Game
			INET->PassiveFrame = 16;
			selected_network_protocol = 3;
		}
		else
		{
			INET->PassiveFrame = 17;
		}

		if ( pp )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pp = 0;
		}
	} while ( ItemChoose == -1 );

	//Sanity check
	if ( selected_network_protocol == 100 && ItemChoose != mcmCancel )
	{
		TOTALEXIT = 1;
		selected_network_protocol = 0;
		ItemChoose = mcmCancel;
	}

	SlowUnLoadPalette( "2\\agew_1.pal" );

	//No GameSpy option anymore, only GSC-Game.Net
	use_gsc_network_protocol = false;
	if ( selected_network_protocol == 3 )
	{
		use_gsc_network_protocol = true;
	}

	return ItemChoose;
}

bool EnumPlr()
{
	return PIEnumeratePlayers( PINFO, false );
}

static const char Unknm[] = "?";

const char* GetPName( int i )
{
	for ( int j = 0; j < 8; j++ )if ( PINFO[j].ColorID == i )return PINFO[j].name;
	return Unknm;
}

static bool ColorMouseOver( SimpleDialog* SD )
{
	if ( !SD->Enabled )return false;
	GPPicture* CB = (GPPicture*) SD;
	if ( Lpressed )
	{
		CB->Nation++;
		if ( CB->Nation > 6 )CB->Nation = 0;
		Lpressed = false;
    }
	if ( Rpressed )
	{
		CB->Nation--;
		if ( CB->Nation < 0 )CB->Nation = 6;
		Rpressed = false;
    }
	return true;
}

static bool ChangeAlias( SimpleDialog* SD )
{
	if ( ( !SD->Enabled ) || ( !SD->Visible ) )return false;
	GPPicture* GPP = (GPPicture*) SD;
	if ( Lpressed )
	{
		if ( GPP->Nation < 7 )GPP->Nation++;
		else GPP->Nation = 0;
		Lpressed = 0;
    }
	if ( Rpressed )
	{
		if ( GPP->Nation > 0 )GPP->Nation--;
		else GPP->Nation = 7;
		Rpressed = 0;

    }
	return true;
}

static void Decode( uint8_t * data, uint8_t * xlt, int Size )
{
	for ( int i = 0; i < Size; i++ )data[i] = xlt[data[i]];
}

static const char UNKN[] = "?UNKNOWN?";
static const char* GetPlNameByID( DWORD ID )
{
	for ( int i = 0; i < NPlayers; i++ )
	{
		if ( PINFO[i].PlayerID == ID )
		{
			return PINFO[i].name;
        }
    }
	return UNKN;
}

static void ControlLingvo( char* str )
{
	char* c1 = strstr( str, "fuck" );
	if ( c1 )
	{
		c1[0] = '?';
		c1[1] = '?';
		c1[2] = '?';
		c1[3] = '?';
    }
	c1 = strstr( str, "FUCK" );
	if ( c1 )
	{
		c1[0] = '?';
		c1[1] = '?';
		c1[2] = '?';
		c1[3] = '?';
    }
}

static void ClearCTime()
{
	memset( CurrentStartTime, 0xFF, 8 * 4 );
	memset( NextStartTime, 0xFF, 8 * 4 );
}

void ShowCentralMessage( const char* Message, int GPIDX )
{
	int L = GetRLCStrWidth( Message, &WhiteFont );
	int DX = 0;
	DrawStdBar2( ( RealLx - L - 64 ) / 2 - DX, ( RealLy - 70 ) / 2 + 18, ( RealLx + L + 64 ) / 2 - DX, ( RealLy - 70 ) / 2 + 55, GPIDX );
	ShowString( ( RealLx - L ) / 2 + 1 - DX, ( RealLy - GetRLCHeight( YellowFont.RLC, 'W' ) ) / 2 + 1, Message, &BlackFont );
	ShowString( ( RealLx - L ) / 2 - DX, ( RealLy - GetRLCHeight( YellowFont.RLC, 'W' ) ) / 2, Message, &YellowFont );
}

void LimitString( char* str, RLCFont * FONT, int L )
{
	int L0;
	do
	{
		L0 = GetRLCStrWidth( str, FONT );
		if ( L0 > L )
		{
			int LL = strlen( str );
			str[LL - 4] = '.';
			str[LL - 3] = '.';
			str[LL - 2] = '.';
			str[LL - 1] = 0;
        }
	} while ( L0 > L );
}

void DRAWBOX( int x, int y, int Lx, int Ly, int /*Idx*/, uint8_t /*Active*/, int param )
{
	DrawStdBar2( x, y, x + Lx - 1, y + Ly - 1, param );
}

static void WaitWithMessage( const char* Message )
{
	LocalGP BOR2( "Interface\\bor2" );
	DarkScreen();
	ShowCentralMessage( Message, BOR2.GPID );
	FlipPages();
	int T = GetTickCount();
	KeyPressed = 0;
	do
	{
		ProcessMessages();
	} while ( GetTickCount() - T < 3000 && !KeyPressed );
	KeyPressed = 0;
}

static void AddPrimitiveChat( const char* Nick, char* str,
	char** &ChatMess, char** &ChatSender,
	int &NCHATS, int &MAXCHATS )
{
	if ( NCHATS >= MAXCHATS )
	{
		MAXCHATS += 32;
		ChatMess = (char**) realloc( ChatMess, 4 * MAXCHATS );
		ChatSender = (char**) realloc( ChatSender, 4 * MAXCHATS );
    }
	if ( NCHATS > 512 )
	{
		free( ChatMess[0] );
		free( ChatSender[0] );
		memcpy( ChatMess, ChatMess + 1, NCHATS * 4 );
		memcpy( ChatSender, ChatSender + 1, NCHATS * 4 );
		NCHATS--;
    }
	ChatMess[NCHATS] = new char[strlen( str ) + 1];
	strcpy( ChatMess[NCHATS], str );
	ChatSender[NCHATS] = new char[strlen( Nick ) + 1];
	strcpy( ChatSender[NCHATS], Nick );
	NCHATS++;
}

void AddChatString( const char* Nick, char* str, int MaxLx, RLCFont * FONT,
	char** &ChatMess, char** &ChatSender,
	int &NCHATS, int &MAXCHATS )
{
	char ACCAMULATOR[256];
	int sp = 0;
	int LL = strlen( str );
	sprintf( ACCAMULATOR, "%s: ", Nick );
	int LName = GetRLCStrWidth( ACCAMULATOR, FONT );
	if ( LName == 32 )LName++;
	ACCAMULATOR[0] = 0;
	do
	{
		int Lx = GetRLCStrWidth( ACCAMULATOR, FONT ) + LName;
		int Lx1 = strlen( ACCAMULATOR );
		ACCAMULATOR[Lx1] = str[sp];
		ACCAMULATOR[Lx1 + 1] = 0;
		sp++;
		if ( Lx > MaxLx )
		{
			if ( LName != 32 )
			{
				AddPrimitiveChat( Nick, ACCAMULATOR, ChatMess, ChatSender, NCHATS, MAXCHATS );
				LName = 32;
				ACCAMULATOR[0] = 0;
			}
			else
			{
				AddPrimitiveChat( "", ACCAMULATOR, ChatMess, ChatSender, NCHATS, MAXCHATS );
				ACCAMULATOR[0] = 0;
            }
        }
	} while ( sp < LL );
	if ( ACCAMULATOR[0] )
	{
		if ( LName != 32 )
		{
			AddPrimitiveChat( Nick, ACCAMULATOR, ChatMess, ChatSender, NCHATS, MAXCHATS );
			LName = 32;
			ACCAMULATOR[0] = 0;
		}
		else
		{
			AddPrimitiveChat( "", ACCAMULATOR, ChatMess, ChatSender, NCHATS, MAXCHATS );
			ACCAMULATOR[0] = 0;
        }
    }
}

//Returns Process_GSC_ChatWindow()
static int ProcessInternetConnection( bool Active )
{
	return Process_GSC_ChatWindow( Active, &GlobalRIF );
}

static int PPTIME = 0;
static const char* _engNOCONN = "No direct connection established with:";
uint8_t MPL_NatRefTBL[8] = { 0,1,2,3,4,5,6,7 };

//Shows lobby interface for multiplayer deathmatch and single player random map
static bool MPL_WaitingGame( bool Host, bool SINGLE )
{
	if ( SINGLE )
	{
		use_gsc_network_protocol = false;
	}

	ClearLPACK();

	ServerDPID = 0;

	int r0 = GetTickCount();

	PPTIME = r0;

	memset( CurrentMaxPing, 0xFF, 4 * 8 );

	ReceiveAll();

	CurStatus = 0;
	PitchTicks = 8;
	MaxPingTime = 0;
	ClearCTime();
	int NCHATS = 0;
	CHATSTRING[0] = 0;
	int MAXCHATS = 0;
	char** ChatMess = nullptr;
	char** ChatSender = nullptr;
	PlayerInfo MYPINF;
	memset( &MYPINF, 0, sizeof MYPINF );

	byte* Preview = new byte[( 292 * 190 ) + 4];
	char Currand[200];
	( (word*) Preview )[0] = 292;
	( (word*) Preview )[1] = 190;
    char LastPrName[200] = "";
	byte XLAT[256];

	ResFile fx = RReset( "2\\0to1.xlt" );
	RBlockRead( fx, XLAT, 256 );
	RClose( fx );

	LoadFog( 2 );
	LocalGP BTNS( "Interface\\Multi_opt" );
	LocalGP OK1( "Interface\\anm_start_1_on" );
	LocalGP OK2( "Interface\\anm_start_1_off" );
	LocalGP CANCEL1( "Interface\\anm_cancel_on" );
	LocalGP CANCEL2( "Interface\\anm_cancel_off" );
	LocalGP DIS( "Interface\\an_start_disable" );
	LocalGP SCROL( "Interface\\_Slider" );
	LocalGP MASKA( "Interface\\preview_mask" );
	LocalGP NewFlag( "Interface\\NewFlag" );
	LocalGP INCHAT( "Interface\\IButtons" );
	LocalGP CCOM( "Interface\\CompCombo" );
	LocalGP SINGL( "Interface\\Random_map" );
	LocalGP LONGBOX( "Interface\\LongBox" );
	LocalGP ISYM3( "Interface\\isymb3" );
	LocalGP ENC1( "Interface\\anm_enc_on" );
	LocalGP ENC2( "Interface\\anm_enc_off" );

	SQPicture Back;
	if ( SINGLE )
	{
		Back.LoadPicture( "Interface\\Random_Map_Background.bmp" );
	}
	else
	{
		Back.LoadPicture( "Interface\\Multiplayer_Create_Background.bmp" );
	}

	LocalGP FONT( "romw" );
	RLCFont FontY( FONT.GPID );
	FontY.SetColorTable( 3 );
	LoadOptionalTable( 3, "2\\orange.grd" );

	RLCFont FontW( FONT.GPID );
	FontW.SetColorTable( 4 );
	LoadOptionalTable( 4, "2\\white.grd" );

	DialogsSystem MENU( menu_x_off, menu_y_off );
	MENU.OkSound = GetSound( "START" );

	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();

	MENU.HintFont = &hfnt;
	MENU.HintY = menu_hint_y;
	MENU.HintX = menu_hint_x;

	Picture* GPPB = MENU.addPicture( nullptr, 0, 0, &Back, &Back, &Back );
	TextButton* SHOWREC = MENU.addTextButton( nullptr, 1010, 4, "0018", &YellowFont, &YellowFont, &YellowFont, 0 );

	GPPicture*  NameBack[8];
	GPPicture*  NationBack[8];
	GPPicture*  ColorBack[8];
	GPPicture*  Flags[8];
	GPPicture*  CD_Back[8];
	GPPicture*  CD_Present[8];
	GPPicture*  ReadyBack[8];
	GPPicture*  AliasBack[8];
	ComboBox*   COMPS[8];
	ComboBox*   KICK[8];
	GPPicture*  BCOMP[8];

	ComboBox*   ADD_OPT;
	ComboBox*   ADD_OPT_VAL;
	bool COMPPREV[8];
	memset( COMPPREV, 0, sizeof COMPPREV );

	TextButton* AliasID[8];
	InputBox*   MNAME[8];
	ComboBox*   MNATION[8];
	TextButton* ReadyFlow[8];
	TextButton* VersTextShadow[8];
	TextButton* VersText[8];
	GPPicture*  MREADY[8];

	byte COMCOLOR[8];
	byte COMNATION[8];
	byte COMALLY[8];
	int NComp = 0;
	memset( COMCOLOR, 0, sizeof COMCOLOR );
	memset( COMNATION, 0, sizeof COMNATION );
	memset( COMALLY, 0, sizeof COMALLY );
	int HostID = -1;

	ChatViewer* CHVIEW = nullptr;
	if ( !SINGLE )
	{
		CHVIEW = MENU.addChatViewer( nullptr, 47, 502, 4, 26, 380, &ChatMess, &ChatSender, &NCHATS );
	}
	char CHATMESSAGE[256] = "";

	InputBox* CHATBOX = nullptr;
	if ( !SINGLE )
	{
		CHATBOX = MENU.addInputBox( nullptr, 126 - 8, 612, CHATMESSAGE, 250, 320, 22, &YellowFont, &WhiteFont );
		CHATBOX->Active = 1;
	}

	VScrollBar* CHSCR = MENU.addNewGP_VScrollBar( nullptr, 443, 483, 125, 1, 0, SCROL.GPID, 0 );
	CHSCR->Visible = 0;

	BPXView* PREV = MENU.addBPXView( nullptr, 496, 441, 292, 190, 1, 1, 1, Preview + 4, nullptr );
	PREV->Visible = false;

    MENU.addGPPicture( nullptr, 496, 441, MASKA.GPID, 0 );

	//Text string for nation choice 'random'
    const char* rtxt = GetTextByID( "???" );

	int RNATID = GlobalAI.NAi;
	for ( int i = 0; i < 8; i++ )
	{
		if ( SINGLE )
		{
			COMPS[i] = MENU.addGP_ComboBox( nullptr, 38, 161 + i * 26, SINGL.GPID, 24 + 8 * ( i % 3 ), 8, 0, &WhiteFont, &YellowFont, nullptr );
			KICK[i] = nullptr;
		}
		else
		{
			COMPS[i] = MENU.addGP_ComboBox( nullptr, 38, 161 + i * 26, CCOM.GPID, 8 * ( i % 3 ), 8, 0, &WhiteFont, &YellowFont, nullptr );
			if ( Host )
			{
				KICK[i] = MENU.addGP_ComboBox( nullptr, 38, 161 + i * 26, CCOM.GPID, 8 * ( i % 3 ), 8, 0, &WhiteFont, &YellowFont, nullptr );
				KICK[i]->AddLine( "                                                   " );
				KICK[i]->AddLine( GetTextByID( "KICK" ) );
				KICK[i]->Visible = 0;
			}
			else
			{
				KICK[i] = nullptr;
			}
		}
		if ( SINGLE )
		{
			BCOMP[i] = MENU.addGPPicture( nullptr, 38, 161 + i * 26, SINGL.GPID, 90 + ( i % 3 ) );
		}
		else
		{
			BCOMP[i] = MENU.addGPPicture( nullptr, 38, 161 + i * 26, CCOM.GPID, 24 + ( i % 3 ) );
		}
		BCOMP[i]->Visible = 0;
		COMPS[i]->AddLine( GetTextByID( "None" ) );
		COMPS[i]->AddLine( GetTextByID( "@RMID_EASY" ) );
		COMPS[i]->AddLine( GetTextByID( "@RMID_NORMAL" ) );
		COMPS[i]->AddLine( GetTextByID( "@RMID_HARD" ) );
		COMPS[i]->AddLine( GetTextByID( "@RMID_VERYHARD" ) );

		COMPS[i]->Visible = 0;
		COMPS[i]->Enabled = 0;
		if ( !SINGLE )
		{
			NameBack[i] = MENU.addGPPicture( nullptr, 38, 161 + i * 26, BTNS.GPID, 9 + ( i % 3 ) );
			NameBack[i]->Visible = 0;
		}

		MNAME[i] = MENU.addInputBox( nullptr, 32 + 9, 160 + i * 26, PINFO[i].name, 20, 120, 20, &WhiteFont, &YellowFont );
		MNAME[i]->Visible = 0;

		if ( !SINGLE )
		{
			NationBack[i] = MENU.addGPPicture( nullptr, 173, 161 + i * 26, BTNS.GPID, i % 3 );
			NationBack[i]->Visible = 0;
		}

		if ( !SINGLE )
		{
			CD_Back[i] = MENU.addGPPicture( nullptr, 376, 161 + i * 26, BTNS.GPID, 3 + ( i % 3 ) );
			CD_Back[i]->Visible = 0;
			CD_Present[i] = MENU.addGPPicture( nullptr, 365 + 13, 162 + i * 26, BTNS.GPID, 25 );
			CD_Present[i]->Visible = 0;
			VersTextShadow[i] = MENU.addTextButton( nullptr, 376 + 12, 161 + 6 + i * 26, "        ", &SmallBlackFont, &SmallBlackFont, &SmallBlackFont, 1 );
			VersTextShadow[i]->Visible = 0;
			VersText[i] = MENU.addTextButton( nullptr, 376 + 12 - 1, 161 + 6 - 2 + i * 26, "        ", &SmallYellowFont, &SmallYellowFont, &SmallYellowFont, 1 );
			VersText[i]->Visible = 0;
			ReadyBack[i] = MENU.addGPPicture( nullptr, 408, 161 + i * 26, BTNS.GPID, 15 + ( i % 3 ) );
			ReadyBack[i]->Visible = 0;
			ReadyFlow[i] = MENU.addTextButton( nullptr, 332 + 100 - 3, 164 + i * 26, "100%", &YellowFont, &YellowFont, &YellowFont, 1 );
			ReadyFlow[i]->Visible = 0;
			ReadyFlow[i]->Enabled = 0;
			MREADY[i] = MENU.addGPPicture( nullptr, 413 + 5, 162 + i * 26, BTNS.GPID, 26 );
			MREADY[i]->Visible = 0;

			AliasBack[i] = MENU.addGPPicture( nullptr, 332, 161 + i * 26, BTNS.GPID, 96 + ( i % 3 ) );
			AliasID[i] = MENU.addTextButton( nullptr, 332 + 18, 165 + i * 26, "X", &SmallYellowFont, &SmallYellowFont, &SmallYellowFont, 1 );
			AliasBack[i]->OnMouseOver = &ChangeAlias;
			AliasBack[i]->Visible = 0;
			AliasBack[i]->Nation = 0;
			AliasID[i]->Visible = 0;
			ColorBack[i] = MENU.addGPPicture( nullptr, 285, 161 + i * 26, BTNS.GPID, 6 + ( i % 3 ) );
			ColorBack[i]->Visible = 0;
			ColorBack[i]->OnMouseOver = &ColorMouseOver;
			ColorBack[i]->Hint = GetTextByID( "MMCHANGECOLOR" );
			Flags[i] = MENU.addGPPicture( nullptr, 295 - 5 + 19 - 14, 161 - 11 + 12 + i * 26, NewFlag.GPID, ( i * 17 ) % 45 );
			Flags[i]->Visible = 0;
			ColorBack[i]->Child = Flags[i];
		}
		else
		{
			AliasBack[i] = MENU.addGPPicture( nullptr, 359, 161 + i * 26, SINGL.GPID, 93 + ( i % 3 ) );
			AliasID[i] = MENU.addTextButton( nullptr, 358 + 21, 165 + i * 26, "X", &SmallYellowFont, &SmallYellowFont, &SmallYellowFont, 1 );
			AliasBack[i]->OnMouseOver = &ChangeAlias;
			AliasBack[i]->Visible = 0;
			AliasBack[i]->Nation = 0;
			AliasID[i]->Visible = 0;
			ColorBack[i] = MENU.addGPPicture( nullptr, 409, 161 + i * 26, SINGL.GPID, 93 + ( i % 3 ) );
			ColorBack[i]->Visible = 0;
			ColorBack[i]->OnMouseOver = &ColorMouseOver;
			ColorBack[i]->Hint = GetTextByID( "MMCHANGECOLOR" );
			Flags[i] = MENU.addGPPicture( nullptr, 418 - 5 + 19 - 14, 161 - 11 + 12 + i * 26, NewFlag.GPID, ( i * 17 ) % 45 );
			Flags[i]->Visible = 0;
			ColorBack[i]->Child = Flags[i];
		}

		MNAME[i]->Hint = GetTextByID( "MOCHANGENAME" );
		MNAME[i]->Visible = false;
		MNAME[i]->Enabled = false;

		if ( SINGLE )
		{
			MNATION[i] = MENU.addGP_ComboBox( nullptr, 203, 161 + i * 26, SINGL.GPID, 8 * ( i % 3 ), 8, 0, &WhiteFont, &YellowFont, nullptr );
		}
		else
		{
			MNATION[i] = MENU.addGP_ComboBox( nullptr, 173, 161 + i * 26, BTNS.GPID, 28 + 8 * ( i % 3 ), 8, 0, &WhiteFont, &YellowFont, nullptr );
		}

		MNATION[i]->Hint = GetTextByID( "MMCHANGENAT" );

		//Total amount of available nations
		int NG = GlobalAI.NAi;

		//Populate the dropdown list with nations
		for ( int j = 0; j < NG; j++ )
		{
			MNATION[i]->AddLine( GlobalAI.Ai[j].Message );
		}

		//Append 'random' to the list
		MNATION[i]->AddLine( rtxt );

		//Set default selected item to the last one, 'random'
		MNATION[i]->CurLine = NG;

		MNATION[i]->FontDx -= 2;
	}

	int cc1 = 0;
	byte mss1 = 1;
	byte MSS1 = 0;
	bool GMREADY = true;

	//---------------Page 1 - designed map--------------
	VScrollBar* VSB = MENU.addNewGP_VScrollBar( nullptr, 778, 152, 221, 1, 0, SCROL.GPID, 0 );
	ListBox* LBMaps = MENU.addGP_ListBox( nullptr, 494, 161, 8, BTNS.GPID, 18, 26, &WhiteFont, &YellowFont, VSB );

	//---------------Page 2 - random map----------------
	int y = 161;

	RandomMapDesc RMP;
	RMP.Load( "Terrain\\Res.dat" );

	GPPicture* RMS = MENU.addGPPicture( nullptr, 495, 149, BTNS.GPID, 21 );

	//Dropdown lists with settings
	ComboBox* CMGRP1[8];

	//Landscape type
	CMGRP1[0] = MENU.addGP_ComboBox( nullptr, 633, 161, BTNS.GPID, 60, 9, 0, &WhiteFont, &YellowFont, nullptr );
	CMGRP1[0]->CurLine = RMP.STY_DEF;
	int RSTYID = RMP.NSTY;
	for ( int p = 0; p < RMP.NSTY; p++ )
	{
		CMGRP1[0]->AddLine( RMP.STY[p].Name );
	}
	CMGRP1[0]->AddLine( rtxt );
	CMGRP1[0]->Hint = GetTextByID( "SOMAPSTYLE" );

	y += 25;

	//Nature of ground
	int RRELID = RMP.NRelief;
	CMGRP1[1] = MENU.addGP_ComboBox( nullptr, 633, 161 + 26, BTNS.GPID, 68, 9, 0, &WhiteFont, &YellowFont, nullptr );
	CMGRP1[1]->CurLine = RMP.Relief_DEF;
	for ( int p = 0; p < RMP.NRelief; p++ )
	{
		CMGRP1[1]->AddLine( RMP.Relief[p] );
	}
	CMGRP1[1]->AddLine( rtxt );
	CMGRP1[1]->Hint = GetTextByID( "SOMOUNT" );

	y += 25;

	//Initial resources
	int RRESID = RMP.NRES;
	CMGRP1[2] = MENU.addGP_ComboBox( nullptr, 633, 161 + 26 * 2, BTNS.GPID, 76, 9, 0, &WhiteFont, &YellowFont, nullptr );
	CMGRP1[2]->CurLine = RMP.RES_DEF;
	for ( int p = 0; p < RMP.NRES; p++ )
	{
		CMGRP1[2]->AddLine( RMP.RES[p].Name );
	}
	CMGRP1[2]->AddLine( rtxt );
	CMGRP1[2]->Hint = GetTextByID( "SORESTOT" );

	y += 25;

	//Minerals
	CMGRP1[3] = MENU.addGP_ComboBox( nullptr, 633, 161 + 26 * 3, BTNS.GPID, 60, 9, 0, &WhiteFont, &YellowFont, nullptr );
	CMGRP1[3]->CurLine = RMP.MINES_DEF;
	int RMINEID = RMP.NMINES;
	for ( int p = 0; p < RMP.NMINES; p++ )
	{
		CMGRP1[3]->AddLine( RMP.MINES[p].Name );
	}
	CMGRP1[3]->AddLine( rtxt );
	CMGRP1[3]->Hint = GetTextByID( "SORESONMAP" );

	//Map size dropdown list
	CMGRP1[4] = MENU.addGP_ComboBox( nullptr, 633, 161 + 26 * 4, BTNS.GPID, 60, 9, 0, &WhiteFont, &YellowFont, nullptr );
	CMGRP1[4]->AddLine( GetTextByID( "MS_NORMAL" ) );
	CMGRP1[4]->AddLine( GetTextByID( "MS_LARGE" ) );
	CMGRP1[4]->AddLine( GetTextByID( "MS_HUGE" ) );
	CMGRP1[4]->CurLine = 0;

	//Additional options
	ADD_OPT = MENU.addGP_ComboBoxDLX( nullptr,
		633 - 65 - 64, 161 + 26 * 6, 100 + 100 + 64,
		LONGBOX.GPID, 0, 0, 0, &WhiteFont, &YellowFont, nullptr );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP1" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP2" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP3" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP4" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP5" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP6" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP7" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP8" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP9" ) );
	ADD_OPT->AddLine( GetTextByID( "GMTP_TP10" ) );

	ADD_OPT_VAL = MENU.addGP_ComboBoxDLX( nullptr, 633 - 65 - 64 + 26, 161 + 26 * 7, 100 + 100 + 64 - 26, LONGBOX.GPID, 0, 0, 0, &WhiteFont, &YellowFont, nullptr );

	GPPicture* AOPPIC = MENU.addGPPicture( nullptr, 633 - 65 - 64, 161 + 26 * 7, ISYM3.GPID, 2 );

	TextButton* AOPTB = MENU.addTextButton( nullptr, 633 - 65 - 64 + 2, 161 + 26 * 5 + 3, GetTextByID( "ADDMPOPT" ), &WhiteFont, &WhiteFont, &WhiteFont, 0 );

	int PrevCOMBO = -1;

	GPPicture* Exam = MENU.addGPPicture( nullptr, 495, 413, BTNS.GPID, 95 );

	//Saved game tab
	VScrollBar* VSS = MENU.addNewGP_VScrollBar( nullptr, 778, 152, 221, 1, 0, SCROL.GPID, 0 );
	ListBox* LBSav = MENU.addGP_ListBox( nullptr, 494, 161, 8, BTNS.GPID, 18, 26, &WhiteFont, &YellowFont, VSS );

	GP_PageControl* GPP = MENU.addPageControl( nullptr, 495, 128, BTNS.GPID, 22 );
	GPP->Hint = GetTextByID( "MOPAGES" );

	//Coordinates are relative to screen, not to DialogsSystem
	//Manual offset recalculation necessary
	int page_x_off = 495 + menu_x_off;
	int page_y_off = 128 + menu_y_off;
	GPP->AddPage( page_x_off, page_y_off, page_x_off + 88, page_y_off + 22, 22 );
	GPP->AddPage( page_x_off + 88, page_y_off, page_x_off + 197, page_y_off + 22, 23 );
	GPP->AddPage( page_x_off + 197, page_y_off, page_x_off + 292, page_y_off + 22, 24 );
	GPP->CurPage = 1;

	//Speed mode
	ComboBox* speed_mode = MENU.addGP_ComboBox( nullptr,
		504, 380, BTNS.GPID, 76, 9, 0, &WhiteFont, &YellowFont, nullptr );
	speed_mode->AddLine( SLOWMOD );
	speed_mode->AddLine( FASTMOD );
	speed_mode->CurLine = exFMode;

	//Victory condition
	ComboBox* VICT = MENU.addGP_ComboBox( nullptr, 173, 418, BTNS.GPID, 52, 9, 0, &WhiteFont, &YellowFont, nullptr );

	for ( int i = 0; i <= 9; i++ )
	{
		char ccx[32];
		sprintf( ccx, "VICOND%d", i );
        const char* cc2 = GetTextByID( ccx );
		if ( cc2 != ccx )
		{
			VICT->AddLine( cc2 );
		}
		else
		{
			goto ffe1;
		}
	}

	VICT->CurLine = 0;
	VICT->Visible = 0;
	VICT->Enabled = 0;

ffe1:
	//Game time
	ComboBox* TIME = MENU.addGP_ComboBox( nullptr, 358, 418, BTNS.GPID, 84, 9, 0, &WhiteFont, &YellowFont, nullptr );
	for ( int i = 0; i <= 9; i++ )
	{
		char ccx[32];
		sprintf( ccx, "TIME%d", i );
        const char* cc2 = GetTextByID( ccx );
		if ( cc2 != ccx )
		{
			TIME->AddLine( cc2 );
		}
		else
		{
			goto ffe2;
		}
	}

ffe2:
	if ( !Host )
	{//Only host can change those
		for ( int j = 0; j < 5; j++ )
		{
			CMGRP1[j]->Enabled = false;
		}
		GPP->Enabled = false;
	}

    /*GPPicture* DISBL =*/ MENU.addGPPicture( nullptr, 862, 568, DIS.GPID, 0 );

	//Ready & start button
	VideoButton* OkBtn;
	if ( Host )
	{
		OkBtn = MENU.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
		if ( SINGLE )
		{
			OkBtn->Hint = GetTextByID( "SOSTART" );
		}
		else
		{
			OkBtn->Hint = GetTextByID( "MOSTART" );
		}
	}
	else
	{
		OkBtn = MENU.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
		OkBtn->Hint = GetTextByID( "MOREADY" );
	}
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;

	VideoButton* CancelBtn = MENU.addVideoButton( nullptr, 862, 668, CANCEL1.GPID, CANCEL2.GPID );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;

	if ( SINGLE )
	{
		CancelBtn->Hint = GetTextByID( "SOCANCEL" );
	}
	else
	{
		CancelBtn->Hint = GetTextByID( "MOCANCEL" );
	}

	if ( use_gsc_network_protocol )
	{
		GP_Button* ENC = MENU.addGP_Button( nullptr, 862, 468, INCHAT.GPID, 9, 8 );
		ENC->UserParam = 99;
		ENC->OnUserClick = &MMItemChoose;
		ENC->Hint = GetTextByID( "BACKINCHAT" );
	}

	ItemChoose = -1;

	memset( PINFO, 0, sizeof PINFO );



	for ( int i = 0; i < 8; i++ )
	{
		PINFO[i].Version = dwVersion;
	}

	if ( SINGLE )
	{
		NPlayers = 1;
		memset( &PINFO[0], 0, sizeof PINFO[0] );
		PINFO[0].PlayerID = 0x12345678;
		MyDPID = 0x12345678;
		PINFO[0].ColorID = 0;
		strcpy( PINFO[0].name, PlName );
	}
	else
	{
		PIEnumeratePlayers( PINFO, false );
	}

	if ( NPlayers > 7 )
	{
		NPlayers = 7;
	}

	byte CUSED = 0;
	for ( int i = 0; i < NPlayers; i++ )
	{
		if ( PINFO[i].PlayerID != MyDPID )
		{
			CUSED |= ( 1 << PINFO[i].ColorID );
		}
	}

	cc1 = 0;
	mss1 = 1;

	while ( CUSED & mss1 )
	{
		cc1++;
		mss1 <<= 1;
	}

	GameInProgress = 0;

	if ( !SINGLE )
	{
		PIEnumeratePlayers( PINFO, false );
	}
	if ( NPlayers > 7 )
	{
		NPlayers = 7;
	}
	for ( int i = 0; i < NPlayers; i++ )
	{
		if ( PINFO[i].PlayerID == MyDPID )
		{
			PINFO[i].ColorID = cc1;
			ColorBack[i]->Nation = cc1;
			if ( Host )
			{
				PINFO[i].Ready = 1;
			}
			else
			{
				PINFO[i].Ready = 0;
			}
			PINFO[i].Host = Host;
			PINFO[i].CD = 1;
			PINFO[i].Version = dwVersion;
            PINFO[i].Rank = GetLogRank();
			PINFO[i].ProfileID = GetMyProfile();
		}
		else
		{
            memset( PINFO + i, 0, sizeof( PlayerInfo) );
		}
	}

	if ( Host )
	{
		SFLB_CreateGamesList( LBSav );
		CreateMapsList( LBMaps );
	}

	PlayerMenuMode = 1;
	GameInProgress = 0;
	tmtmt = 0;
	REALTIME = 0;
	GLOBALTIME = 0;
	PGLOBALTIME = 0;
	int pp = 1;

	GFILE* f = Gopen( "multi.opt", "r" );
	if ( f )
	{
		int v = 0;
		Gscanf( f, "%d", &v );
		if ( v == 1234 )
		{
			for ( int i = 0; i < 5; i++ )
			{
				v = 0;
				Gscanf( f, "%d", &v );
				CMGRP1[i]->CurLine = v;
			}
			v = 0;
			Gscanf( f, "%d", &v );
			VICT->CurLine = v;
			Gscanf( f, "%d", &v );
			TIME->CurLine = v;
			Gscanf( f, "%d", &v );
			speed_mode->CurLine = v;
		}
		Gclose( f );
	}

	KeyPressed = 0;
	PSUMM.ClearPingInfo();
	PBACK.Clear();
	ReceiveAll();

	bool P1E;
	bool P2E;
	bool P3E;
    char oldname[64] = "";
	bool MyOldVers = 0;
	bool PresOldVers = 0;

	int PREVSD = GetTickCount() - 3000;

	word MaxVers = 0;
    int NOMREADY = -1;
    char* OLDV = GetTextByID( "OLDVER" );

    const char* NOCONN = GetTextByID( "$NOCONN" );
	if ( NOCONN[0] == '$' )
	{
		NOCONN = _engNOCONN;
	}

	if ( SINGLE )
	{
		VICT->CurLine = 0;
		VICT->Visible = 0;
	}

	//Total number of additional game options for random map / deathmatch
	const int kNumberOfAdditionalOptions = 10;

	//Selected values for each game option
	//[0] Start Options
	//[1] Balloon Options
	//[2] Cannons
	//[3] Peace Time
	//[4] 18th Century Options
	//[5] Capture Options
	//[6] Autosave Options
	//[7] Diplomatic Centre Options
	//[8] Shipyard Options
	//[9] Market Options
    int selected_opt_values[kNumberOfAdditionalOptions + 1] = { 0 };//+'\0'?

	GFILE* F = Gopen( "MuliExOpt.dat", "r" );
	if ( F )
	{
		Gscanf( F, "%d%d%d%d%d%d%d%d%d%d",
			selected_opt_values,
			selected_opt_values + 1,
			selected_opt_values + 2,
			selected_opt_values + 3,
			selected_opt_values + 4,
			selected_opt_values + 5,
			selected_opt_values + 6,
			selected_opt_values + 7,
			selected_opt_values + 8,
			selected_opt_values + 9,
			selected_opt_values + 10//Sort of '\0'?
		);
		Gclose( F );
	}

    //int MAXOPTVAL[8] = { 10,3,3,10,3,4,7,9 }
	int PREVCNAME = GetTickCount();


	LARGE_INTEGER V1;
	QueryPerformanceCounter( &V1 );
	LARGE_INTEGER V2;
	QueryPerformanceFrequency( &V2 );
	DWORD GMGUID = V1.LowPart + V2.LowPart;

	if ( SINGLE )
	{
		FILE* F1 = fopen( "comp.opt", "r" );
		if ( F1 )
		{
			for ( int i = 0; i < 6; i++ )
			{
				int h = 0;
				int z = fscanf( F1, "%d", &h );
				if ( z == 1 )
				{
					if ( h > 4 )h = 4;
					if ( h < 0 )h = 0;
					COMPS[i + 1]->CurLine = h;
				}
			}
			fclose( F1 );
		}
		else
		{
			COMPS[1]->CurLine = 1;
		}
	}

	bool show_options_list = false;

	do
	{
		if ( use_gsc_network_protocol )
		{
			ChatProcess();

			//Call every iteration, UdpHolePuncher keeps track of intervals by itself
			udp_hole_puncher.KeepAlive();

			if ( CheckPersonality( CHATMESSAGE ) )
			{
				CHATBOX->CursPos = strlen( CHATMESSAGE );
			}
		}

		if ( Host )
		{
			ServerDPID = MyDPID;
		}

		ADD_OPT->Visible = 1;
		ADD_OPT->Enabled = 1;

		if ( SINGLE )
		{
			ADD_OPT_VAL->Visible = 1;
			AOPPIC->Visible = 1;
			AOPTB->Visible = 1;
			ADD_OPT_VAL->Enabled = 1;
		}
		else
		{
			ADD_OPT_VAL->Visible = Host;
			AOPPIC->Visible = Host;
			AOPTB->Visible = Host;
			ADD_OPT_VAL->Enabled = Host;
		}

		//Adjust contents of additional options values dwopdown list
		if ( !Host || PrevCOMBO != ADD_OPT->CurLine || selected_opt_values[ADD_OPT->CurLine] != ADD_OPT_VAL->CurLine )
		{
			int aocl = ADD_OPT->CurLine;
			int aovcl = ADD_OPT_VAL->CurLine;
			if ( Host && PrevCOMBO == aocl )
			{
				selected_opt_values[aocl] = aovcl;
			}

			ADD_OPT->Clear();

			for ( int i = 0; i < kNumberOfAdditionalOptions; i++ )
			{
				char ccc[100];
				sprintf( ccc, "GMTP_TP%d", i + 1 );
				char cc1[128];
				char cc2[256];
				sprintf( cc1, "@GMTP_TP%d_0%d", i + 1, selected_opt_values[i] );
				sprintf( cc2, "%s: %s", GetTextByID( ccc ), GetTextByID( cc1 ) );
				LimitString( cc2, &YellowFont, 230 );
				ADD_OPT->AddLine( cc2 );
			}

			ADD_OPT->CurLine = aocl;
			ADD_OPT_VAL->Clear();

			for ( int i = 0; i < 10; i++ )
			{
				char ccc[128];
				sprintf( ccc, "@GMTP_TP%d_0%d", aocl + 1, i );
                const char* text = GetTextByID( ccc );
				if ( text[0] != '@' )ADD_OPT_VAL->AddLine( text );
			}

			if ( PrevCOMBO == aocl )
			{
				ADD_OPT_VAL->CurLine = aovcl;
			}
			else
			{
				ADD_OPT_VAL->CurLine = selected_opt_values[aocl];
			}

			PrevCOMBO = aocl;
		}

		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].PlayerID == MyDPID )
			{
				if ( Host )
				{
					PINFO[i].Ready = 1;
				}
				PINFO[i].Host = Host;
				PINFO[i].CD = 1;
				PINFO[i].Version = dwVersion;
                PINFO[i].Rank = GetLogRank();
				PINFO[i].ProfileID = GetMyProfile();

				if ( Host )
				{
					PINFO[i].Game_GUID = GMGUID;
				}
			}
		}

		int NCL = 4;
		bool AddChat = 0;
        bool settings_were_changed = false;

		ProcessMessages();

		if ( GameInProgress )
		{
			goto FinCLC;
		}

		//Handle chat window
		if ( !SINGLE )
		{
			if ( KeyPressed && LastKey == 13 )
			{
				if ( CHATBOX->Active&&CHATMESSAGE[0] )
				{
					ControlLingvo( CHATMESSAGE );
					if ( use_gsc_network_protocol && strstr( CHATMESSAGE, "-->" ) )
					{
						CheckForPersonalChat( CHATMESSAGE );
					}
					else
					{
						SendChat( CHATMESSAGE, 0 );
					}
					AddChatString(
						GetPlNameByID( MyDPID ),
						CHATMESSAGE, 375,
						&WhiteFont, ChatMess, ChatSender, NCHATS, MAXCHATS );
					AddChat = 1;
					CHATMESSAGE[0] = 0;
				}
				KeyPressed = 0;
				LastKey = 0;
			}

			if ( CHATSTRING[0] )
			{
				if ( CHATSTRING[0] == '@'&&CHATSTRING[1] == '@'&&CHATSTRING[2] == '@'&&CHATSTRING[3] == 127 )
				{
					DWORD ID;
					char ccx[64];
                    int z = sscanf( CHATSTRING, "%s%ld", ccx, &ID );
					if ( z == 2 && ID == MyDPID )
					{
						ItemChoose = mcmCancel;
						CHATSTRING[0] = 0;
						WaitWithMessage( GetTextByID( "YOUWKICK" ) );
					}
				}
				else
				{
					AddChatString(
						GetPlNameByID( CHATDPID ),
						CHATSTRING, 375,
						&WhiteFont, ChatMess, ChatSender, NCHATS, MAXCHATS );
					CHATSTRING[0] = 0;
					AddChat = 1;
				}
			}

			SendPings();

			if ( NCHATS > NCL )
			{
				CHSCR->SMaxPos = NCHATS - NCL;
				if ( AddChat )
				{
					CHSCR->SPos = CHSCR->SMaxPos;
				}
				CHSCR->Visible = 1;
				CHVIEW->ChatDY = CHSCR->SPos;
			}
			else
			{
				CHSCR->Visible = 0;
				CHSCR->SPos = 0;
				CHVIEW->ChatDY = 0;
			}
		}

		P1E = 0 == GPP->CurPage;
		P2E = 1 == GPP->CurPage;
		P3E = 2 == GPP->CurPage;

		Exam->Visible = P2E;
		VSB->Visible = P1E;
		LBMaps->Visible = P1E;

		for ( int p = 0; p < 5; p++ )
		{
			CMGRP1[p]->Visible = P2E;
			CMGRP1[p]->Enabled = P2E && Host;
		}

		VSS->Visible = P3E;
		LBSav->Visible = P3E;
		RMS->Visible = P2E;

		ADD_OPT->Visible = P2E;
		ADD_OPT->Enabled = P2E;
		ADD_OPT_VAL->Visible = P2E && Host;
		ADD_OPT_VAL->Enabled = P2E && Host;
		AOPPIC->Visible = P2E && Host;
		AOPTB->Visible = P2E && Host;

		StdKeys();

		if ( ItemChoose == mcmOk && !OkBtn->Enabled )
		{
			ItemChoose = -1;
		}

		AnalyseMessages();

		if ( GameInProgress )
		{
			goto FinCLC;
		}

		if ( MyOldVers )
		{
			GPPB->Hint = OLDV;
		}
		else
		{
			GPPB->Hint = nullptr;
		}

		MENU.MarkToDraw();
		MENU.ProcessDialogs();

		if ( GameInProgress )
		{
			goto FinCLC;
		}

		char ccc[128];

		for ( int p = NPlayers; p < 8; p++ )
		{
			CurrentMaxPing[p] = -1;
		}

		if ( PSUMM.NPL&&PSUMM.PSET[0].NPings )
		{
			int png = GetMaxRealPing();

			if ( GetRealTime() - PPTIME > 1000 )
			{
				DWORD lpp[3];
                // BoonXRay 22.08.2017
                //lpp[0] = 'PNTF';
                lpp[0] = ConvCosChar('P', 'N', 'T', 'F');
				lpp[1] = png;
				lpp[2] = lpp[0] + lpp[1];
				SendToAllPlayers( 12, lpp );
				PPTIME = GetRealTime();
				for ( int p = 0; p < NPlayers; p++ )
				{
					if ( PINFO[p].PlayerID == MyDPID )
					{
						CurrentMaxPing[p] = png;
					}
				}
			}

			for ( int p = 0; p < NPlayers; p++ )
			{
				if ( CurrentMaxPing[p] > png )
				{
					png = CurrentMaxPing[p];
				}
			}

			MaxPingTime = png;

			if ( png < MinPing0 )
			{
				MaxPingTime = 0;
			}

			if ( png >= MinPing0 && png < 300 )
			{
				MaxPingTime = 300;
			}

			if ( MaxPingTime )
			{
				MaxPingTime += 500;
			}

			char cc2[300];
			CreateDiffStr( cc2 );
			sprintf( ccc, "max ping: %d  (%s)", png, cc2 );
			strcpy( MENU.DefaultHint, ccc );
		}
		else
		{
			CreateDiffStr( MENU.DefaultHint );
		}

		//Show map configuration while waiting for the host to start
		if ( show_options_list )
		{//Player is ready and isn't the host
			//Fill background box above minimap preview
			int x = menu_x_off + 512;
			int y = menu_y_off + 438;
			GPS.ShowGP( x, y, LONGBOX.GPID, 2, 0 );//Top edge
			y += 9;
			for ( int i = 0; i < 9; i++ )
			{//Middle pieces
				GPS.ShowGP( x, y, LONGBOX.GPID, 5 + i % 2, 0 );
				y += 19;
			}
			GPS.ShowGP( x, y, LONGBOX.GPID, 7, 0 );//Bottom edge

			//List everything from additional options dropdown list
			y = menu_y_off + 447;
			x += 17;
			for ( int i = 0; i < kNumberOfAdditionalOptions; i++ )
			{
				ShowString( x, y, ADD_OPT->Lines[i], &YellowFont );
				y += 15;
			}
		}

		SHOWREC->Visible = RecordMode;
		MENU.RefreshView();

		if ( GameInProgress )
		{
			goto FinCLC;
		}

		for ( int i = NPlayers; i < NPlayers + NComp; i++ )
		{
			COMALLY[i - NPlayers] = AliasBack[i]->Nation;
			COMNATION[i - NPlayers] = MNATION[i]->CurLine;
		}

		if ( VICT->CurLine )
		{//Victory condition needs game time setting
			TIME->Visible = 1;
			TIME->Enabled = 1;
		}
		else
		{
			TIME->Visible = 0;
			TIME->Enabled = 0;
		}

		TIME->Enabled = TIME->Visible;
		NOMREADY = -1;

		if ( !SINGLE )
		{
			for ( int i = 0; i < 8; i++ )
			{
				ReadyFlow[i]->Visible = 0;
			}
		}

		if ( Host )
		{
			int HostID = -1;
			byte CMask = 0;
			for ( int i = 0; i < NPlayers; i++ )
			{
				if ( PINFO[i].Host )
				{
					HostID = i;
				}
			}
			speed_mode->Enabled = true;
			VICT->Enabled = !SINGLE;
			TIME->Enabled = TIME->Visible;
			for ( int q = 0; q < NPlayers; q++ )
			{
				CMask |= 1 << PINFO[q].ColorID;
			}
			int NN = 0;
			for ( int q = NPlayers; q < 7; q++ )
			{
				if ( PINFO[HostID].COMPINFO[q] )
				{
					word W = PINFO[HostID].COMPINFO[q];
					int Color = W & 15;
					CMask |= 1 << Color;
				}
			}
			for ( int q = 0; q < 8; q++ )
			{
				if ( CMask & ( 1 << q ) )
				{
					NN++;
				}
			}

			if ( HostID != -1 )
			{
				int PPP;
				if ( !SINGLE )
				{
					if ( false )//if (( PPP = GetReadyPercent() ) < 100) //BUGFIX: Always 100% loaded in multiplayer
					{
						sprintf( ReadyFlow[HostID]->Message, "%d%%", PPP );
						ReadyFlow[HostID]->Visible = 1;
						MREADY[HostID]->Visible = 0;
						MREADY[HostID]->Enabled = 0;
						NOMREADY = HostID;
					}
					else
					{
						ReadyFlow[HostID]->Visible = 0;
						MREADY[HostID]->Visible = 1;
						MREADY[HostID]->Enabled = 1;
						NOMREADY = -1;;
					}
				}

				PINFO[HostID].MapName[0] = 0;

				//Always get speed mode from host
				PINFO[HostID].speed_mode = speed_mode->CurLine;

				if ( GPP->CurPage == 0 )
				{//Designed map with already existing filename
					if ( LBMaps->CurItem >= 0 && LBMaps->NItems )
					{
						strcpy( PINFO[HostID].MapName, LBMaps->GetItem( LBMaps->CurItem )->Message );
					}
					strcpy( CurrentMap, PINFO[HostID].MapName );
				}
				else
				{
					if ( GPP->CurPage == 1 )
					{//Random generated map, need new filename
						PINFO[HostID].MapStyle = CMGRP1[0]->CurLine;
						PINFO[HostID].HillType = CMGRP1[1]->CurLine;
						PINFO[HostID].StartRes = CMGRP1[2]->CurLine;
						PINFO[HostID].ResOnMap = CMGRP1[3]->CurLine;
						PINFO[HostID].VictCond = VICT->CurLine;
						PINFO[HostID].GameTime = TIME->CurLine;
						for ( int i = 0; i < kNumberOfAdditionalOptions; i++ )
						{
							if ( i < 7 )
							{//Compatibility with old memory layout (see comment to PlayerInfo)
								PINFO[HostID].UserParam[i] = selected_opt_values[i];
							}
							else
							{
								PINFO[HostID].UserParam2[i - 7] = selected_opt_values[i];
							}
						}

						char Nats[9];
						char Nats1[9];
						strcpy( Nats, "00000000" );
						strcpy( Nats1, "00000000" );
                        const char* NTCHAR = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
						int NP = 0;
						for ( int q = 0; q < NPlayers; q++ )
						{
							Nats[7 - PINFO[q].ColorID] = NTCHAR[MNATION[q]->CurLine + 1];
						}

						for ( int q = NPlayers; q < 7; q++ )
						{
							if ( PINFO[HostID].COMPINFO[q] )
							{
								word W = PINFO[HostID].COMPINFO[q];
								int Nat = W >> 11;
								int Color = W & 15;
								Nats[7 - Color] = NTCHAR[Nat + 1];
							}
						}

						for ( int u = 0; u < 8; u++ )
						{
							if ( Nats[u] != '0' )
							{
								NP++;
							}
						}

						//Time generated random number
						int r = r0 % 65536;

						//Map style
						int s = CMGRP1[0]->CurLine
							+ ( ( NP - 2 ) << 4 )
							+ ( CMGRP1[1]->CurLine << 8 )
							+ ( CMGRP1[2]->CurLine << 12 )
							+ ( CMGRP1[3]->CurLine << 16 );

						//Game options
						//Encode 10 option values in a 7-digit number
						int options = EncodeOptionsInNumber( selected_opt_values );

						//Assemble the name of generated random map, the 1st?
						//For syntax details look ~70 lines below
						sprintf( PINFO[HostID].MapName, "RN%d %X %X %s %07d.m3d",
							CMGRP1[4]->CurLine,//map size
							r,//random time number
							s,//map style
							Nats,//player nations
							options//game options
						);

						//Landscape
						int sty = CMGRP1[0]->CurLine == RSTYID ? rand() % RSTYID : CMGRP1[0]->CurLine;
						//Mountains
						int rel = CMGRP1[1]->CurLine == RRELID ? rand() % RRELID : CMGRP1[1]->CurLine;
						//Initial resources
						int res = CMGRP1[2]->CurLine == RRESID ? rand() % RRESID : CMGRP1[2]->CurLine;
						//Minerals
						int min = CMGRP1[3]->CurLine == RMINEID ? rand() % RMINEID : CMGRP1[3]->CurLine;

						//Nations of all players as a 8-digit hex string
						strcpy( Nats, "00000000" );

						for ( int q = 0; q < NPlayers; q++ )
						{
							if ( MNATION[q]->CurLine == RNATID )
							{
								Nats[7 - PINFO[q].ColorID] = NTCHAR[1 + ( rand() % RNATID )];
							}
							else
							{
								Nats[7 - PINFO[q].ColorID] = NTCHAR[MNATION[q]->CurLine + 1];
							}
						}

						for ( int q = NPlayers; q < 7; q++ )
						{
							if ( PINFO[HostID].COMPINFO[q] )
							{
								word W = PINFO[HostID].COMPINFO[q];
								int Color = W & 15;
								if ( MNATION[q]->CurLine == RNATID )
								{
									Nats[7 - Color] = NTCHAR[1 + ( rand() % RNATID )];
								}
								else
								{
									Nats[7 - Color] = NTCHAR[MNATION[q]->CurLine + 1];
								}
							}
						}

						if ( NP < 2 )
						{
							NP = 2;
						}

						//Map ID with all map style options except size, 5 digits
						s = sty + ( ( NP - 2 ) << 4 ) + ( rel << 8 ) + ( res << 12 ) + ( min << 16 );

						//Assemble the name of generated random map, the 2nd?
						//RN			prefix
						//[0-2]			map size
						//[0-F]{1-4}	random time number
						//[0-9]{5}		map style
						//[0-F]{8}		player nations
						//[0-9]{7}		game options
						//.m3d			ending
						sprintf( CurrentMap, "RN%d %X %X %s %07d.m3d", CMGRP1[4]->CurLine, r, s, Nats, options );
					}
					else
					{
						//Saved game, existing filename
						if ( LBSav->CurItem >= 0 && LBSav->NItems )
						{
							strcpy( PINFO[HostID].MapName, LBSav->GetItem( LBSav->CurItem )->Message );
						}
						strcpy( CurrentMap, PINFO[HostID].MapName );
					}
				}
			}
		}
		else
		{//Player is not the host
			VICT->Enabled = 0;
			TIME->Enabled = 0;
			speed_mode->Enabled = false;
			//Search for PlayerInfo of the host
			for ( int i = 0; i < NPlayers; i++ )
			{
				if ( PINFO[i].Host )
				{
					//Get settings from host
					VICT->CurLine = PINFO[i].VictCond;
					TIME->CurLine = PINFO[i].GameTime;
					speed_mode->CurLine = PINFO[i].speed_mode;
					PINFO[0].VictCond = PINFO[i].VictCond;
					PINFO[0].GameTime = PINFO[i].GameTime;

					//Get mapsize from the name of random map file
					//"RN0 *.m3d" = normal map
					//"RN1 *.m3d" = large x2 map
					//"RN2 *.m3d" = huge x4 map
					char* MPN = PINFO[i].MapName;
					if ( MPN[0] == 'R'&&MPN[1] == 'N'&&MPN[3] == ' ' )
					{
						int sz = MPN[2] - '0';
						if ( sz > 2 )
						{
							sz = 2;
						}
						if ( sz < 0 )
						{
							sz = 0;
						}
						CMGRP1[4]->CurLine = sz;
					}
				}
				if ( !SINGLE )
				{
					ReadyFlow[i]->Visible = 0;
				}
			}
			for ( int i = 0; i < 4; i++ )
			{
				CMGRP1[i]->Enabled = 0;
			}
			NOMREADY = -1;
		}

		if ( GPP->CurPage == 0 )
		{
			if ( strcmp( CurrentMap, LastPrName ) )
			{
				strcpy( LastPrName, CurrentMap );
				if ( GetPreview( CurrentMap, Preview + 4 ) )
				{
					PREV->Visible = 1;
					Decode( Preview + 4, XLAT, 292 * 190 );
				}
				else
				{
					PREV->Visible = false;
				}
			}
		}
		else
		{
			if ( GPP->CurPage == 1 )
			{
				char Nats[9];
				char Nats1[9];
				strcpy( Nats, "00000000" );
				strcpy( Nats1, "00000000" );
                const char* NTCHAR = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
				for ( int q = 0; q < NPlayers; q++ )
				{
					Nats[7 - PINFO[q].ColorID] = NTCHAR[MNATION[q]->CurLine + 1];
				}
				for ( int q = NPlayers; q < 7; q++ )
				{
					if ( PINFO[HostID].COMPINFO[q] )
					{
						word W = PINFO[HostID].COMPINFO[q];
						int Nat = W >> 11;
						int Color = W & 15;
						Nats[7 - Color] = NTCHAR[Nat + 1];
					}
				}

                //Check number of players in the lobby
				int NP = 0;
				for ( int u = 0; u < 8; u++ )
				{
					if ( Nats[u] != '0' )
					{
						NP++;
					}
				}
				if ( NP < 2 )
				{
					NP = 2;
				}
				for ( int j = 0; j < NP; j++ )
				{
					Nats1[7 - j] = '1';
				}

				sprintf( Currand, "Preview\\%d%d%d%d",
					CMGRP1[1]->CurLine == 5,
					CMGRP1[0]->CurLine, NP,
					selected_opt_values[3] != 0
				);

				if ( strcmp( Currand, LastPrName ) )
				{
					strcpy( LastPrName, Currand );
					if ( GetPreview( Currand, Preview + 4 ) )
					{
						PREV->Visible = 1;
						Decode( Preview + 4, XLAT, 292 * 190 );
					}
					else
					{
						PREV->Visible = false;
					}
				}
			}
			else
			{
				PREV->Visible = 0;
				if ( strcmp( CurrentMap, LastPrName ) )
				{
					strcpy( LastPrName, CurrentMap );
					int S = GetMapSUMM( CurrentMap );
					for ( int v = 0; v < NPlayers; v++ )
					{
						if ( PINFO[v].PlayerID == MyDPID )
						{
							PINFO[v].CHKSUMM = S;
						}
					}
				}
			}
		}

		MaxVers = 0;

		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].Version > MaxVers )
			{
				MaxVers = PINFO[i].Version;
			}
		}

		for ( int i = 0; i < 8; i++ )
		{
			if ( i < NPlayers )
			{
				if ( PINFO[i].PlayerID == MyDPID )
				{
					PINFO[i].NationID = MNATION[i]->CurLine;
					PINFO[i].ColorID = ColorBack[i]->Nation;
					PINFO[i].GroupID = AliasBack[i]->Nation;

					if ( Host )
					{
						PINFO[i].Page = GPP->CurPage;
					}

					bool ch = memcmp( &MYPINF, PINFO + i, sizeof MYPINF ) != 0;

					if ( ch || GetTickCount() - PREVSD > 5000 )
					{
						if ( !SINGLE )
						{
							SETPLAYERDATA( MyDPID, (void*) &PINFO[i].NationID, sizeof( PlayerInfo ) - 36, ch );
						}
						memcpy( &MYPINF, PINFO + i, sizeof MYPINF );
						PREVSD = GetTickCount();
					}

					if ( ch && Host && !SINGLE )
					{
						PSUMM.ClearPingInfo();
					}

					DPNAME dpName;
					ZeroMemory( &dpName, sizeof( DPNAME ) );
					dpName.dwSize = sizeof( DPNAME );
                    // BoonXRay 20.08.2017
                    //dpName.lpszShortNameA = MNAME[i]->Str;
                    dpName.lpszShortNameA = const_cast<char *>( MNAME[i]->String.data() );
					dpName.lpszLongNameA = nullptr;

					if ( strcmp( dpName.lpszShortNameA, oldname ) || GetTickCount() - PREVCNAME > 3000 )
					{
						if ( !SINGLE )
						{
							bool change = ( strcmp( dpName.lpszShortNameA, oldname ) != 0 );
							SETPLAYERNAME( &dpName, change );
						}
						strcpy( oldname, dpName.lpszShortNameA );
						PREVCNAME = GetTickCount();
					}

					MNATION[i]->Enabled = P2E;
					MNATION[i]->Visible = P2E;
					AliasBack[i]->Visible = 1;
					AliasBack[i]->Enabled = 1;
					AliasID[i]->Visible = 1;
					AliasID[i]->Enabled = 1;
					CurrentNation = MNATION[i]->CurLine;

					if ( !SINGLE )
					{
						NameBack[i]->Visible = 1;
					}

					MNAME[i]->Visible = 1;
					MNAME[i]->Enabled = 0;

					if ( KICK[i] )
					{
						KICK[i]->Visible = 0;
						KICK[i]->Enabled = 0;
					}

					if ( !SINGLE )
					{
						NationBack[i]->Visible = 0;
						NationBack[i]->Enabled = 0;
						CD_Back[i]->Visible = 1;
					}

					ColorBack[i]->Visible = 1;
					ColorBack[i]->Enabled = 1;

					if ( !SINGLE )
					{
						ReadyBack[i]->Visible = 1;
					}

					MyOldVers = PINFO[i].Version != MaxVers;
				}
				else
				{
					if ( !SINGLE )
					{
						NationBack[i]->Visible = 1;
						NationBack[i]->Enabled = 1;
						CD_Back[i]->Visible = 0;
					}

					AliasBack[i]->Visible = 0;
					AliasBack[i]->Enabled = 0;

					if ( KICK[i] )
					{
						KICK[i]->Visible = 1;
						KICK[i]->Enabled = 1;
                        // BoonXRay 20.08.2017
                        //strcpy( KICK[i]->Lines[0], MNAME[i]->Str );
                        strcpy( KICK[i]->Lines[0], MNAME[i]->String.data() );
						while ( GetRLCStrWidth( KICK[i]->Lines[0], &YellowFont ) > 99 )
						{
							KICK[i]->Lines[0][strlen( KICK[i]->Lines[0] ) - 1] = 0;
						}
						if ( KICK[i]->CurLine == 1 )
						{
							char cc[32];
                            sprintf( cc, "@@@0002KICK %d", PINFO[i].PlayerID );
							SendChat( cc, 0 );
							KICK[i]->CurLine = 0;
							if ( DoNewInet )
							{
								DeepDeletePeer( PINFO[i].PlayerID );
							}
						}
						MNAME[i]->Visible = 0;
						MNAME[i]->Enabled = 0;
						MNAME[i]->Active = 0;
					}
					else
					{
						MNAME[i]->Visible = 1;
						MNAME[i]->Enabled = 0;
						MNAME[i]->Active = 0;
					}

					if ( !SINGLE )
					{
						NameBack[i]->Visible = 0;
					}

					MNATION[i]->Enabled = 0;
					ColorBack[i]->Visible = 0;
					ColorBack[i]->Enabled = 0;

					if ( !SINGLE )
					{
						ReadyBack[i]->Visible = 0;
					}
				}

				if ( !SINGLE )
				{
					if ( MNAME[i]->Active )
					{
						NameBack[i]->SpriteID = 12 + ( i % 3 );
					}
					else
					{
						NameBack[i]->SpriteID = 9 + ( i % 3 );
					}
				}
				Flags[i]->SpriteID = ( i * 229 + GetRealTime() / 30 ) % 45;
				Flags[i]->Nation = ColorBack[i]->Nation;

				if ( AliasBack[i]->Nation )
				{
					AliasID[i]->Message[0] = '0' + AliasBack[i]->Nation;
				}
				else
				{
					AliasID[i]->Message[0] = 'X';
				}

				MNATION[i]->Visible = true;

				if ( !SINGLE )
				{
					MREADY[i]->SpriteID = 26 + ( PINFO[i].Ready != 0 );
					if ( i != NOMREADY )
					{
						MREADY[i]->Visible = true;
						MREADY[i]->Enabled = true;
					}
				}

				Flags[i]->Visible = 1;

				if ( !SINGLE )
				{
					CD_Present[i]->Visible = 1;
				}

				MNATION[i]->Visible = 1;
				AliasID[i]->Visible = 1;
				AliasID[i]->Enabled = 1;

				int v = PINFO[i].Version;

				if ( !SINGLE )
				{
					char VERS[128];

					if ( v > 255 )
					{
						sprintf( VERS, "M%d", v >> 8 );
					}
					else
					{
						sprintf( VERS, "%d.%d%d", v / 100, ( v / 10 ) % 10, v % 10 );
					}

					strcpy( VersText[i]->Message, VERS );
					strcpy( VersTextShadow[i]->Message, VERS );

					if ( v != MaxVers && ( GetTickCount() / 300 ) & 1 )
					{
						VersText[i]->ActiveFont = &SmallRedFont;
						VersText[i]->PassiveFont = &SmallRedFont;
					}
					else
					{
						VersText[i]->ActiveFont = &SmallYellowFont;
						VersText[i]->PassiveFont = &SmallYellowFont;
					}
					VersText[i]->Visible = 1;
					VersTextShadow[i]->Visible = 1;
				}
				PresOldVers = v != MaxVers;
			}
			else
			{
				if ( i < NComp + NPlayers )
				{
                    const char* COMPTXT = "Computer";
					if ( !SINGLE )
					{
						NameBack[i]->Visible = 0;
						NationBack[i]->Visible = 1;
						CD_Back[i]->Visible = 1;
						CD_Present[i]->Visible = 1;
						ReadyBack[i]->Visible = 1;
						VersText[i]->Visible = 0;
						VersTextShadow[i]->Visible = 0;
						MREADY[i]->Visible = 0;
					}
					ColorBack[i]->Visible = 1;
					Flags[i]->Visible = 1;

					MNAME[i]->Visible = 1;
					MNAME[i]->Enabled = 0;
					if ( KICK[i] )
					{
						KICK[i]->Visible = 1;
						KICK[i]->Enabled = 1;
					}
                    // BoonXRay 20.08.2017
                    //MNAME[i]->Str = COMPTXT;
                    MNAME[i]->String = COMPTXT;
					MNAME[i]->Active = 0;
					MNATION[i]->Visible = 1;
					MNATION[i]->Enabled = 1;
					int pp = i - NPlayers;
					MNATION[i]->CurLine = COMNATION[pp];
					AliasBack[i]->Visible = 1;
					AliasBack[i]->Enabled = 1;
					AliasID[i]->Visible = 1;
					AliasID[i]->Enabled = 1;
					if ( COMALLY[pp] )
					{
						AliasID[i]->Message[0] = '0' + COMALLY[pp];
					}
					else
					{
						AliasID[i]->Message[0] = 'X';
					}
					Flags[i]->SpriteID = ( i * 229 + GetRealTime() / 30 ) % 45;
					Flags[i]->Nation = ColorBack[i]->Nation;
				}
				else
				{
					if ( !SINGLE )
					{
						NameBack[i]->Visible = 0;
						NationBack[i]->Visible = 0;
						CD_Back[i]->Visible = 0;
						CD_Present[i]->Visible = 0;
						ReadyBack[i]->Visible = 0;
						VersText[i]->Visible = 0;
						VersTextShadow[i]->Visible = 0;
						MREADY[i]->Visible = 0;
					}
					ColorBack[i]->Visible = 0;
					Flags[i]->Visible = 0;

					MNAME[i]->Visible = 0;
					MNAME[i]->Enabled = 0;
					if ( KICK[i] )
					{
						KICK[i]->Visible = 0;
						KICK[i]->Enabled = 0;
					}
					MNAME[i]->Active = 0;
					MNATION[i]->Visible = 0;
					MNATION[i]->Enabled = 0;
					AliasBack[i]->Visible = 0;
					AliasBack[i]->Enabled = 0;
					AliasID[i]->Visible = 0;
					AliasID[i]->Enabled = 0;
				}
			}
		}

		if ( !SINGLE )
		{
			if ( !PIEnumeratePlayers( PINFO, true ) )
			{
				ItemChoose = mcmCancel;
			}
		}

		if ( NPlayers > 7 )
		{
			NPlayers = 7;
		}

		//If true, ready flag will be zeroed
        // BoonXRay 18.08.2017
        //bool settings_were_changed = false;
        settings_were_changed = false;

		//Get settings from host
		if ( !Host )
		{
			int HostID = -1;
			int MyID = -1;

			//Find host ID
			for ( int i = 0; i < NPlayers; i++ )
			{
				if ( PINFO[i].Host )
				{
					HostID = i;
				}
				if ( PINFO[i].PlayerID == MyDPID )
				{
					MyID = i;
				}
			}

			//Sanity check
			if ( HostID != -1 && MyID != -1 )
			{
				//Check speed mode regardles of map page
				if ( PINFO[HostID].speed_mode != speed_mode->CurLine )
				{
					speed_mode->CurLine = PINFO[HostID].speed_mode;
					settings_were_changed = true;
				}

				//Map type (designed, random, saved game)
				switch ( PINFO[HostID].Page )
				{
				case 1:
					//Random map
					GPP->CurPage = 1;
					{
						//Reset own ready flag if seeings were changed
						if ( PINFO[HostID].MapStyle != CMGRP1[0]->CurLine )
						{
							CMGRP1[0]->CurLine = PINFO[HostID].MapStyle;
							settings_were_changed = true;
						}
						if ( PINFO[HostID].HillType != CMGRP1[1]->CurLine )
						{
							CMGRP1[1]->CurLine = PINFO[HostID].HillType;
							settings_were_changed = true;
						}
						if ( PINFO[HostID].StartRes != CMGRP1[2]->CurLine )
						{
							CMGRP1[2]->CurLine = PINFO[HostID].StartRes;
							settings_were_changed = true;
						}
						if ( PINFO[HostID].ResOnMap != CMGRP1[3]->CurLine )
						{
							CMGRP1[3]->CurLine = PINFO[HostID].ResOnMap;
							settings_were_changed = true;
						}
						if ( PINFO[HostID].VictCond != VICT->CurLine )
						{
							VICT->CurLine = PINFO[HostID].VictCond;
							settings_were_changed = true;
						}
						if ( PINFO[HostID].GameTime != TIME->CurLine )
						{
							TIME->CurLine = PINFO[HostID].GameTime;
							settings_were_changed = true;
						}
						for ( int i = 0; i < kNumberOfAdditionalOptions; i++ )
						{
							byte param = 0x00;
							if ( i < 7 )
							{//Compatibility with old memory layout (see comment to PlayerInfo)
								param = PINFO[HostID].UserParam[i];
							}
							else
							{
								param = PINFO[HostID].UserParam2[i - 7];
							}

							if ( param != selected_opt_values[i] )
							{
								selected_opt_values[i] = param;
								settings_were_changed = true;
							}
						}
					}
					break;

				case 2:
					//Saved game
					GPP->CurPage = 2;
					LBSav->ClearItems();
					LBSav->AddItem( PINFO[HostID].MapName, 0 );
					break;

				case 0:
					//Designed map
					GPP->CurPage = 0;
					LBMaps->ClearItems();
					LBMaps->AddItem( PINFO[HostID].MapName, 0 );
					break;
				}

				strcpy( PINFO[MyID].MapName, PINFO[HostID].MapName );
				PINFO[MyID].MapStyle = PINFO[HostID].MapStyle;
				PINFO[MyID].HillType = PINFO[HostID].HillType;
				PINFO[MyID].StartRes = PINFO[HostID].StartRes;
				PINFO[MyID].ResOnMap = PINFO[HostID].ResOnMap;
				PINFO[MyID].Game_GUID = PINFO[HostID].Game_GUID;
				strcpy( CurrentMap, PINFO[HostID].MapName );
			}
		}

		for ( int i = 0; i < 8; i++ )
		{
			if ( i < NPlayers )
			{
				if ( MNATION[i]->CurLine != PINFO[i].NationID )
				{
					MNATION[i]->CurLine = PINFO[i].NationID;
					settings_were_changed = true;
				}
				if ( ColorBack[i]->Nation != PINFO[i].ColorID )
				{
					ColorBack[i]->Nation = PINFO[i].ColorID;
					settings_were_changed = true;
				}
				if ( AliasBack[i]->Nation != PINFO[i].GroupID )
				{
					AliasBack[i]->Nation = PINFO[i].GroupID;
					settings_were_changed = true;
				}
			}
		}

		if ( mcmOk == ItemChoose && !Host )
		{//Start button clicked, set ready flag
			for ( int i = 0; i < NPlayers; i++ )
			{//Look for your own PlayerInfo struct
				if ( PINFO[i].PlayerID == MyDPID && !Host )
				{//Also it's time to show options list
					PINFO[i].Ready = 1;
					if ( 1 == GPP->CurPage )
					{//Random map mode, display selected options
						show_options_list = true;
					}
				}
			}
		}

		HostID = -1;
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].Host )
			{
				HostID = i;
			}
		}

		if ( HostID != -1 )
		{
			if ( Host )
			{
				for ( int i = 0; i < 7; i++ )
				{
					COMPS[i]->Visible = ( i >= NPlayers );
					COMPS[i]->Enabled = COMPS[i]->Visible;
					BCOMP[i]->Visible = 0;
					if ( COMPS[i]->Visible && COMPS[i]->CurLine )
					{
						if ( !COMPPREV[i] )
						{
							byte msk = 0;
							for ( int j = 0; j < NPlayers; j++ )
							{
								msk |= 1 << ColorBack[j]->Nation;
							}
							for ( int j = NPlayers; j < 7; j++ )
							{
								if ( COMPS[j]->Visible && COMPS[j]->CurLine )
								{
									msk |= 1 << ColorBack[j]->Nation;
								}
							}
							int defc = -1;
							for ( int j = 0; j < 7 && defc == -1; j++ )
							{
								if ( !( msk&( 1 << j ) ) )
								{
									defc = j;
								}
							}
							if ( defc == -1 )
							{
								defc = 0;
							}
							ColorBack[i]->Nation = defc;
							AliasBack[i]->Nation = 7;
						}

						if ( !SINGLE )
						{
							NameBack[i]->Visible = 0;
							NationBack[i]->Visible = 1;
							CD_Back[i]->Visible = 0;
							CD_Present[i]->Visible = 0;
							ReadyBack[i]->Visible = 0;
							MREADY[i]->Visible = 0;
							VersText[i]->Visible = 0;
							VersTextShadow[i]->Visible = 0;
						}

						ColorBack[i]->Visible = 1;
						Flags[i]->Visible = 1;
						MNAME[i]->Visible = 0;
						MNAME[i]->Enabled = 0;

						if ( KICK[i] )
						{
							KICK[i]->Visible = 0;
							KICK[i]->Enabled = 0;
						}

						MNAME[i]->Active = 0;
						MNATION[i]->Visible = 1;
						MNATION[i]->Enabled = 1;
                        AliasBack[i]->Visible = 1;
						AliasBack[i]->Enabled = 1;
						AliasID[i]->Visible = 1;
						AliasID[i]->Enabled = 1;
						if ( AliasBack[i]->Nation )
						{
							AliasID[i]->Message[0] = '0' + AliasBack[i]->Nation;
						}
						else
						{
							AliasID[i]->Message[0] = 'X';
						}
						Flags[i]->SpriteID = ( i * 229 + GetRealTime() / 30 ) % 45;
						Flags[i]->Nation = ColorBack[i]->Nation;
						word W = int( Flags[i]->Nation )
							+ ( int( AliasBack[i]->Nation ) << 4 )
							+ ( int( COMPS[i]->CurLine ) << 8 )
							+ ( int( MNATION[i]->CurLine ) << 11 );
						PINFO[HostID].COMPINFO[i] = W;
					}
					else
					{
						PINFO[HostID].COMPINFO[i] = 0;
					}
					COMPPREV[i] = COMPS[i]->Visible && COMPS[i]->CurLine;
				}
			}
			else
			{//Player is not the host
				for ( int i = 0; i < 7; i++ )
				{
					if ( i >= NPlayers )
					{
						word W = PINFO[HostID].COMPINFO[i];
						if ( W )
						{
							COMPS[i]->Visible = 1;
							word comps_val = ( W >> 8 ) & 7;
							if ( COMPS[i]->CurLine != comps_val )
							{//Changed bot difficulty
								COMPS[i]->CurLine = comps_val;
								settings_were_changed = true;
							}

							if ( !SINGLE )
							{
								NameBack[i]->Visible = 0;
								NationBack[i]->Visible = 1;
								CD_Back[i]->Visible = 0;
								CD_Present[i]->Visible = 0;
								ReadyBack[i]->Visible = 0;
								MREADY[i]->Visible = 0;
								VersText[i]->Visible = 0;
								VersTextShadow[i]->Visible = 0;
							}
							ColorBack[i]->Visible = 1;
							Flags[i]->Visible = 1;
							MNAME[i]->Visible = 0;
							MNAME[i]->Enabled = 0;
							if ( KICK[i] )
							{
								KICK[i]->Visible = 0;
								KICK[i]->Enabled = 0;
							}
							MNAME[i]->Active = 0;
							MNATION[i]->Visible = 1;
							MNATION[i]->Enabled = 1;

							if ( MNATION[i]->CurLine != W >> 11 )
							{//Changed player nation
								MNATION[i]->CurLine = W >> 11;
								settings_were_changed = true;
							}

							AliasBack[i]->Visible = 1;
							AliasBack[i]->Enabled = 1;
							AliasID[i]->Visible = 1;
							AliasID[i]->Enabled = 1;

							int nation_val = ( W & 255 ) >> 4;
							if ( AliasBack[i]->Nation != nation_val )
							{//No idea what, but it was changed
								AliasBack[i]->Nation = nation_val;
								settings_were_changed = true;
							}

							if ( AliasBack[i]->Nation )
							{
								AliasID[i]->Message[0] = '0' + AliasBack[i]->Nation;
							}
							else
							{
								AliasID[i]->Message[0] = 'X';
							}
							Flags[i]->SpriteID = ( i * 229 + GetRealTime() / 30 ) % 45;
							ColorBack[i]->Nation = W & 15;
							if ( Flags[i]->Nation != ColorBack[i]->Nation )
							{//Changed flag color
								Flags[i]->Nation = ColorBack[i]->Nation;
								settings_were_changed = true;
							}
							BCOMP[i]->Visible = 0;
						}
						else
						{
							COMPS[i]->Visible = 0;
							BCOMP[i]->Visible = 0;
						}
						COMPS[i]->Enabled = 0;
					}
					else
					{//Clear empty player slots
						if ( COMPS[i]->Visible )
						{//Player or bot was there, but isn't any longer
							settings_were_changed = true;
						}
						COMPS[i]->Visible = 0;
						COMPS[i]->Enabled = 0;
						BCOMP[i]->Visible = 0;
					}
				}
			}
		}

		if ( settings_were_changed )
		{//Cancel own ready flag
			for ( int i = 0; i < NPlayers; i++ )
			{//Look for your own PlayerInfo struct
				if ( PINFO[i].PlayerID == MyDPID && !Host )
				{
					PINFO[i].Ready = 0;
					show_options_list = false;
				}
			}
		}

		GMREADY = true;
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].PlayerID != MyDPID && !PINFO[i].Ready )
			{
				GMREADY = false;
			}
		}
		MSS1 = 0;

		for ( int i = 0; i < 7; i++ )
		{
			if ( COMPS[i]->Visible && COMPS[i]->CurLine )
			{
				byte MSS2 = 1 << ColorBack[i]->Nation;
				if ( MSS1 & MSS2 && Host )
				{
					GMREADY = false;
				}
				MSS1 |= MSS2;
			}
		}

		if ( Host )
		{
			if ( !SINGLE )
			{
				if ( NPlayers < 2 )
				{
					GMREADY = false;
				}

				/*
				//BUGFIX: start button always active for host independent of ping status
				if (!CheckPingsReady())
				{
					GMREADY = false;
				}
				*/

				byte bs = 0;
				for ( int i = 0; i < NPlayers; i++ )
				{
					bs |= 1 << AliasBack[i]->Nation;
				}
				if ( !( bs & 1 ) )
				{
					bs |= 1;
					if ( bs == 2 || bs == 4 || bs == 8 || bs == 16 || bs == 32 || bs == 64 || bs == 128 )
					{
						GMREADY = false;
					}
				}
				for ( int i = 0; i < NPlayers; i++ )
				{
					if ( CurrentMaxPing[i] == -1 )GMREADY = 0;
				}
				if ( PresOldVers )
				{
					GMREADY = 0;
				}
			}

			//teams calculating
			int ntm = 0;
			byte ams = 0;
			int cur_tm[8];
			memset( cur_tm, 0xFF, sizeof cur_tm );
			for ( int i = 0; i < 8; i++ )
			{
				if ( AliasID[i]->Visible )
				{
					int tm = AliasBack[i]->Nation;
					int cl = ColorBack[i]->Nation;
					if ( cur_tm[cl] != -1 )
					{
						if ( tm != cur_tm[cl] || !cur_tm[cl] )
						{
							GMREADY = 0;
						}
					}
					else
					{
						cur_tm[cl] = tm;
					}
					int alb = AliasBack[i]->Nation;
					if ( !alb )
					{
						ntm++;
					}
					else
					{
						if ( !( ams&( 1 << alb ) ) )
						{
							ntm++;
							ams |= 1 << alb;
						}
					}
				}
			}
			if ( ntm < 2 )
			{
				GMREADY = 0;
			}

            /*if ( GPP->CurPage == 2 )
			{
				int MINS = 0;
				for ( int i = 0; i < NPlayers; i++ )
				{
					if ( PINFO[i].PlayerID == MyDPID )
					{
						MINS = PINFO[i].CHKSUMM;
					}
				}
            }*/
			OkBtn->Enabled = GMREADY;
			OkBtn->Visible = GMREADY;
		}
		else
		{
			OkBtn->Enabled = !MyOldVers;
			OkBtn->Visible = !MyOldVers;

			/*
			//BUGFIX: Disable no direct connection check
			if (!CheckExistConn())
			{
				if (!( GetKeyState( VK_CONTROL ) & 0x8000 ))
				{
					OkBtn->Enabled = 0;
					OkBtn->Visible = 0;
				}
				if (GetTickCount() - FInTime > 2000)
				{
					strcpy( MENU.DefaultHint, NOCONN );
					PrintBadConn( MENU.DefaultHint + strlen( MENU.DefaultHint ) );
				}
			}
			*/

			if ( P1E )
			{
				OkBtn->Enabled = PREV->Visible;
				OkBtn->Visible = PREV->Visible;
				if ( !PREV->Visible )
				{
					if ( ( GetTickCount() / 200 ) & 1 )
					{
						LBMaps->AFont = &WhiteFont;
						LBMaps->PFont = &YellowFont;
					}
					else
					{
						LBMaps->AFont = &RedFont;
						LBMaps->PFont = &RedFont;
					}
				}
				else
				{
					LBMaps->AFont = &WhiteFont;
					LBMaps->PFont = &YellowFont;
				}
			}
		}

		//Ready flag handled before, ignore Start button here
		if ( mcmOk == ItemChoose && !Host )
		{
			ItemChoose = -1;
		}

		if ( pp )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pp = 0;
			ReceiveAll();
		}

		if ( ItemChoose == 99 )
		{
			ProcessInternetConnection( 0 );

			ItemChoose = -1;
			if ( PlayerMenuMode != 1 )
			{
				ItemChoose = mcmSingle;
			}
		}

	FinCLC:
        ;
	} while ( ItemChoose == -1 && PlayerMenuMode == 1 );

	if ( Host )
	{
		GFILE* F = Gopen( "MuliExOpt.dat", "wt" );
        if ( F )
		{
			Gprintf( F, "%d %d %d %d %d %d %d %d %d %d",
				selected_opt_values[0],
				selected_opt_values[1],
				selected_opt_values[2],
				selected_opt_values[3],
				selected_opt_values[4],
				selected_opt_values[5],
				selected_opt_values[6],
				selected_opt_values[7],
				selected_opt_values[8],
				selected_opt_values[9],
				selected_opt_values[10] //Sort of '\0'?
			);
			Gclose( F );
		}
	}

	if ( SINGLE )
	{
		FILE* Fx = fopen( "comp.opt", "wt" );
		if ( Fx )
		{
			for ( int i = 0; i < 6; i++ )
			{
				fprintf( Fx, "%d ", COMPS[i + 1]->CurLine );
			}
			fclose( Fx );
		}
	}

    // BoonXRay 22.08.2017
    //if ( Host && PlayerMenuMode != -1 )
    if ( Host && (PlayerMenuMode != 0xFFFF) )
	{
		for ( int i = 0; i < 8; i++ )
		{
			MPL_NatRefTBL[i] = 0xFF;
		}

        int NN[8] = { 0,0,0,0,0,0,0,0 };
		for ( int i = NPlayers; i < 7; i++ )
		{
			if ( PINFO[HostID].COMPINFO[i] )
			{
				word W = PINFO[HostID].COMPINFO[i];
                int Color = W & 15;
				MPL_NatRefTBL[Color] = Color;
				NN[Color] = 1;
			}
		}
		for ( int i = 0; i < NPlayers; i++ )
		{
			int c = PINFO[i].ColorID;
			if ( MPL_NatRefTBL[c] == 0xFF )
			{
				MPL_NatRefTBL[c] = c;
			}
		}
		for ( int i = 0; i < NPlayers; i++ )
		{
			int c = PINFO[i].ColorID;
			if ( NN[c] )
			{
				for ( int j = 0; j < 8; j++ )
				{
					if ( MPL_NatRefTBL[j] == 0xFF )
					{
						MPL_NatRefTBL[j] = c;
						PINFO[i].ColorID = j;
						j = 8;
					}
				}
			}
			else
			{
				NN[c] = 1;
			}
		}
		for ( int i = 0; i < 8; i++ )
		{
			if ( MPL_NatRefTBL[i] == 0xFF )
			{
				MPL_NatRefTBL[i] = i;
			}
		}
		if ( SINGLE )
		{
			memcpy( NatRefTBL, MPL_NatRefTBL, 8 );
		}
	}

	if ( NCHATS )
	{
		for ( int i = 0; i < NCHATS; i++ )
		{
			free( ChatMess[i] );
			free( ChatSender[i] );
		}
		free( ChatMess );
		free( ChatSender );
	}

	if ( PINFO[0].VictCond )
	{
		int t = 60;
		char cc3[32];
		sprintf( cc3, "-TIME%d", PINFO[0].GameTime );
		sscanf( GetTextByID( cc3 ), "%d", &t );
		PINFO[0].GameTime = t * 60;
	}

	if ( Host )
	{
		//Save game settings for next session
		f = Gopen( "multi.opt", "w" );
		if ( f )
		{
			Gprintf( f, "1234 %d %d %d %d %d %d %d %d",
				CMGRP1[0]->CurLine,//Landscape
				CMGRP1[1]->CurLine,//Mountains
				CMGRP1[2]->CurLine,//Initial resources
				CMGRP1[3]->CurLine,//Minerals
				CMGRP1[4]->CurLine,//Map size (0=normal,1=large,2=huge)
				VICT->CurLine,
				TIME->CurLine,
				speed_mode->CurLine
			);
			Gclose( f );
		}
	}

	free( Preview );

	if ( mcmCancel == ItemChoose )
	{
		CloseMPL();
	}

	exFMode = speed_mode->CurLine;
	SpeedSh = exFMode;
	FrmDec = exFMode + 1;

	return ( ItemChoose == mcmOk ) || PlayerMenuMode != 1;
}

static int SetCurrentBattle( int BattleID,
	SQPicture* Prev,
	ComboBox** MNATION,
	TextViewer* BTTXT )
{
	if ( BattleID < 0 )
	{
		return 0;
	}

	if ( BattleID >= WARS.NBattles )
	{
		return WARS.NBattles - 1;
	}
	Prev->LoadPicture( WARS.Battles[BattleID].MiniMap );
	for ( int i = 0; i < 2; i++ )
	{
		int cl = MNATION[i]->CurLine;
		MNATION[i]->Clear();
		MNATION[i]->AddLine( WARS.Battles[BattleID].RedUnits );
		MNATION[i]->AddLine( WARS.Battles[BattleID].BlueUnits );
		MNATION[i]->CurLine = cl;
	}
	ResFile FF1 = RReset( WARS.Battles[BattleID].Brief );
	if ( FF1 != INVALID_HANDLE_VALUE )
	{
		ResFile FF2 = RRewrite( "miss.txt" );
		if ( FF2 != INVALID_HANDLE_VALUE )
		{
			RBlockWrite( FF2, WARS.Battles[BattleID].BigHeader, strlen( WARS.Battles[BattleID].BigHeader ) );
			char c = 0x0D;
			RBlockWrite( FF2, &c, 1 );
			c = 0x0A;
			RBlockWrite( FF2, &c, 1 );
			c = 0x0D;
			RBlockWrite( FF2, &c, 1 );
			c = 0x0A;
			RBlockWrite( FF2, &c, 1 );
			byte* Data = new byte[RFileSize( FF1 )];
			RBlockRead( FF1, Data, RFileSize( FF1 ) );
			RBlockWrite( FF2, Data, RFileSize( FF1 ) );
			RClose( FF2 );
		}
		RClose( FF1 );
	}

	BTTXT->LoadFile( "miss.txt" );

	return BattleID;
}

//Shows and processes Historic Battle lobby
static bool MPL_WaitingBattleGame( bool Host, int BattleID )
{
	ClearLPACK();
	ReceiveAll();
	CurStatus = 0;
	PitchTicks = 8;
	MaxPingTime = 0;
	ClearCTime();
	int NCHATS = 0;
	CHATSTRING[0] = 0;
	int MAXCHATS = 0;
	char** ChatMess = nullptr;
	char** ChatSender = nullptr;
	LoadFog( 2 );
	LocalGP BTNS( "Interface\\Multi_opt" );
	LocalGP OK1( "Interface\\anm_start_1_on" );
	LocalGP OK2( "Interface\\anm_start_1_off" );
	LocalGP CANCEL1( "Interface\\anm_cancel_on" );
	LocalGP CANCEL2( "Interface\\anm_cancel_off" );
	LocalGP DIS( "Interface\\an_start_disable" );
	LocalGP SCROL( "Interface\\_Slider" );
	LocalGP MASKA( "Interface\\History_mask1" );
	LocalGP NewFlag( "Interface\\NewFlag" );
	LocalGP INCHAT( "Interface\\IButtons" );
	PSUMM.ClearPingInfo();
	PlayerInfo MYPINF;
	memset( &MYPINF, 0, sizeof MYPINF );

	SQPicture Back( "Interface\\Background_Historical_Create.bmp" );
	SQPicture Prev( WARS.Battles[BattleID].MiniMap );
	LocalGP FONT( "romw" );
	RLCFont FontY( FONT.GPID );
	FontY.SetColorTable( 3 );
	LoadOptionalTable( 3, "2\\orange.grd" );
	RLCFont FontW( FONT.GPID );
	FontW.SetColorTable( 4 );
	LoadOptionalTable( 4, "2\\white.grd" );

	DialogsSystem MENU( menu_x_off, menu_y_off );
	MENU.OkSound = GetSound( "START" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	MENU.HintFont = &hfnt;
	MENU.HintY = menu_hint_y;
	MENU.HintX = menu_hint_x;
	Picture* GPPB = MENU.addPicture( nullptr, 0, 0, &Back, &Back, &Back );
	GPPicture*  NameBack[2];
	GPPicture*  NationBack[2];
	GPPicture*  ColorBack[2];
	GPPicture*  Flags[2];
	GPPicture*  CD_Back[2];
	GPPicture*  CD_Present[2];
	GPPicture*  ReadyBack[2];
	InputBox*   MNAME[2];
	ComboBox*   MNATION[2];
	TextButton* ReadyFlow[2];
	//ColoredBar* MCOLOR [8];
	TextButton* VersTextShadow[8];
	TextButton* VersText[8];
	GPPicture*  MREADY[2];
	//MENU.addInputBox(nullptr,0,0,CurrentMap,64,400,25,&FontW,&FontW);
	ChatViewer* CHVIEW = MENU.addChatViewer( nullptr, 506, 424, 7, 26, 250, &ChatMess, &ChatSender, &NCHATS );
	char CHATMESSAGE[256] = "";
	InputBox* CHATBOX = MENU.addInputBox( nullptr, 588 - 14, 612, CHATMESSAGE, 250, 190, 22, &YellowFont, &WhiteFont );
	CHATBOX->Active = 1;
	VScrollBar* CHSCR = MENU.addNewGP_VScrollBar( nullptr, 778, 409, 198, 1, 0, SCROL.GPID, 0 );
	CHSCR->Visible = 0;
    /*Picture* PREV =*/ MENU.addPicture( nullptr, 496, 156, &Prev, &Prev, &Prev );
    /*GPPicture* PMASK =*/ MENU.addGPPicture( nullptr, 496, 156, MASKA.GPID, 0 );
	VScrollBar* TXSB = MENU.addNewGP_VScrollBar( nullptr, 443, 283, 351, 1, 0, SCROL.GPID, 0 );

	ResFile FF1 = RReset( WARS.Battles[BattleID].Brief );
	if ( FF1 != INVALID_HANDLE_VALUE )
	{
		ResFile FF2 = RRewrite( "miss.txt" );
		if ( FF2 != INVALID_HANDLE_VALUE )
		{
			RBlockWrite( FF2, WARS.Battles[BattleID].BigHeader, strlen( WARS.Battles[BattleID].BigHeader ) );
			char cx = 0x0D;
			RBlockWrite( FF2, &cx, 1 );
			cx = 0x0A;
			RBlockWrite( FF2, &cx, 1 );
			cx = 0x0D;
			RBlockWrite( FF2, &cx, 1 );
			cx = 0x0A;
			RBlockWrite( FF2, &cx, 1 );
			byte* Data = new byte[RFileSize( FF1 )];
			RBlockRead( FF1, Data, RFileSize( FF1 ) );
			RBlockWrite( FF2, Data, RFileSize( FF1 ) );
			RClose( FF2 );
        }
		RClose( FF1 );
    }

	TextViewer* BTTXT = MENU.addTextViewer( nullptr, 47, 294, 385, 330, "miss.txt", &YellowFont );
	BTTXT->SymSize = 20;
	BTTXT->PageSize = ( BTTXT->y1 - BTTXT->y ) / BTTXT->SymSize;

	BTTXT->AssignScroll( TXSB );
	for ( int i = 0; i < 2; i++ )
	{
		NameBack[i] = MENU.addGPPicture( nullptr, 38 + 5, 161 + i * 26, BTNS.GPID, 9 + ( i % 3 ) );
		NameBack[i]->Visible = 0;
		MNAME[i] = MENU.addInputBox( nullptr, 32 + 5 + 5, 160 + i * 26, PINFO[i].name, 20, 128, 20, &WhiteFont, &YellowFont );
		MNAME[i]->Visible = 0;
		NationBack[i] = MENU.addGPPicture( nullptr, 183, 161 + i * 26, BTNS.GPID, i % 3 );
		NationBack[i]->Visible = 0;
		ColorBack[i] = MENU.addGPPicture( nullptr, 295 + 8, 161 + i * 26, BTNS.GPID, 6 + ( i % 3 ) );
		ColorBack[i]->Visible = 0;
		Flags[i] = MENU.addGPPicture( nullptr, 295 - 5 + 19 + 1, 161 - 11 + 12 + i * 26, NewFlag.GPID, ( i * 17 ) % 45 );
		Flags[i]->Visible = 0;
		CD_Back[i] = MENU.addGPPicture( nullptr, 358 + 5, 161 + i * 26, BTNS.GPID, 3 + ( i % 3 ) );
		CD_Back[i]->Visible = 0;
		CD_Present[i] = MENU.addGPPicture( nullptr, 365, 162 + i * 26, BTNS.GPID, 25 );
		CD_Present[i]->Visible = 0;
		VersTextShadow[i] = MENU.addTextButton( nullptr, 376 + 12 - 12, 161 + 6 + i * 26, "        ", &SmallBlackFont, &SmallBlackFont, &SmallBlackFont, 1 );
		VersTextShadow[i]->Visible = 0;
		VersText[i] = MENU.addTextButton( nullptr, 376 + 12 - 12 - 1, 161 + 6 - 2 + i * 26, "        ", &SmallYellowFont, &SmallYellowFont, &SmallYellowFont, 1 );
		VersText[i]->Visible = 0;
		ReadyBack[i] = MENU.addGPPicture( nullptr, 399 + 4, 161 + i * 26, BTNS.GPID, 15 + ( i % 3 ) );
		ReadyBack[i]->Visible = 0;

		MNAME[i]->Hint = GetTextByID( "MOCHANGENAME" );
		MNAME[i]->Visible = false;
		MNAME[i]->Enabled = false;
		MNATION[i] = MENU.addGP_ComboBox( nullptr, 183, 161 + i * 26, BTNS.GPID, 28 + 8 * ( i % 3 ), 8, 0, &WhiteFont, &YellowFont, nullptr );
		MNATION[i]->Hint = GetTextByID( "MMCHANGENAT" );
		MREADY[i] = MENU.addGPPicture( nullptr, 413, 162 + i * 26, BTNS.GPID, 26 );
		MREADY[i]->Visible = 0;
		MNATION[i]->AddLine( WARS.Battles[BattleID].RedUnits );
		MNATION[i]->AddLine( WARS.Battles[BattleID].BlueUnits );
		MNATION[i]->CurLine = i;
		MNATION[i]->Enabled = Host;

		MNATION[i]->FontDx -= 2;
		ReadyFlow[i] = MENU.addTextButton( nullptr, 332 + 100 - 8, 164 + i * 26, "100%", &YellowFont, &YellowFont, &YellowFont, 1 );
		ReadyFlow[i]->Visible = 0;
		ReadyFlow[i]->Enabled = 0;
	}

    /*GPPicture* DISBL =*/ MENU.addGPPicture( nullptr, 862, 568, DIS.GPID, 0 );
	VideoButton* OkBtn;
	if ( Host )
	{
		OkBtn = MENU.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
		OkBtn->Hint = GetTextByID( "MOSTART" );
	}
	else
	{
		OkBtn = MENU.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
		OkBtn->Hint = GetTextByID( "MOREADY" );
	}

	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	VideoButton* CancelBtn = MENU.addVideoButton( nullptr, 862, 668, CANCEL1.GPID, CANCEL2.GPID );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;
	CancelBtn->Hint = GetTextByID( "MOCANCEL" );

	ItemChoose = -1;

	memset( PINFO, 0, sizeof PINFO );

	GameInProgress = 0;

	PIEnumeratePlayers( PINFO, 0 );
	if ( NPlayers > 2 )
	{
		NPlayers = 2;
	}

	byte CUSED = 0;
	for ( int i = 0; i < NPlayers; i++ )
	{
		if ( PINFO[i].PlayerID != MyDPID )
		{
			CUSED |= ( 1 << PINFO[i].ColorID );
		}
	}

	int cc1 = 0;
	byte mss1 = 1;

	while ( CUSED & mss1 )
	{
		cc1++;
		mss1 <<= 1;
	}

	for ( int i = 0; i < NPlayers; i++ )
	{
		if ( PINFO[i].PlayerID == MyDPID )
		{
			PINFO[i].ColorID = cc1;
			ColorBack[i]->Nation = cc1;
			if ( Host )
			{
				PINFO[i].Ready = 1;
			}
			else
			{
				PINFO[i].Ready = 0;
			}
			PINFO[i].Host = Host;
			PINFO[i].CD = 1;
			PINFO[i].Version = dwVersion;
		}
		else
		{
            memset( PINFO + i, 0, sizeof( PlayerInfo) );
		}
	}

	PIEnumeratePlayers( PINFO, 0 );
	if ( NPlayers > 2 )NPlayers = 2;
	PlayerMenuMode = 1;
	GameInProgress = 0;
	tmtmt = 0;
	REALTIME = 0;
	GLOBALTIME = 0;
	PGLOBALTIME = 0;
	int pp = 1;
	KeyPressed = 0;
	PBACK.Clear();
	PSUMM.ClearPingInfo();
	ReceiveAll();
	byte MSS1 = 0;
	bool GMREADY = true;
	int HostID = -1;
    char oldname[64] = "";
	bool MyOldVers = 0;
	bool PresOldVers = 0;
	int PREVSD = GetTickCount();
	int NOMREADY = -1;
	word MaxVers = 0;
    char* OLDV = GetTextByID( "OLDVER" );
	for ( int i = 0; i < 2; i++ )PINFO[i].MapStyle = BattleID;
	if ( Host )strcpy( MENU.DefaultHint, GetTextByID( "DEVHISUS" ) );
    do
	{
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].PlayerID == MyDPID )
			{
				if ( Host )
				{
					PINFO[i].Ready = 1;
				}
				PINFO[i].Host = Host;
				PINFO[i].CD = 1;
				PINFO[i].Version = dwVersion;
			}
		}

        int NCL = 7;
		bool AddChat = 0;
		HostID = -1;
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].Host )HostID = i;
        }
		ProcessMessages();
		if ( KeyPressed&&Host )
		{
			if ( LastKey == 109 )
			{
				BattleID = SetCurrentBattle( BattleID - 1, &Prev, MNATION, BTTXT );
				KeyPressed = 0;
				PINFO[HostID].MapStyle = BattleID;
            }
			if ( LastKey == 107 )
			{
				BattleID = SetCurrentBattle( BattleID + 1, &Prev, MNATION, BTTXT );
				KeyPressed = 0;
				PINFO[HostID].MapStyle = BattleID;
            }
        }
		if ( GameInProgress )goto FinCLC;
		if ( KeyPressed&&LastKey == 13 )
		{
			if ( CHATBOX->Active&&CHATMESSAGE[0] )
			{
				ControlLingvo( CHATMESSAGE );
				SendChat( CHATMESSAGE, 0 );
				AddChatString(
					GetPlNameByID( MyDPID ),
					CHATMESSAGE, 250,
					&WhiteFont, ChatMess, ChatSender, NCHATS, MAXCHATS );
				AddChat = 1;
				CHATMESSAGE[0] = 0;
			}
			KeyPressed = 0;
			LastKey = 0;
        }
		if ( CHATSTRING[0] )
		{
			AddChatString(
				GetPlNameByID( CHATDPID ),
				CHATSTRING, 250,
				&WhiteFont, ChatMess, ChatSender, NCHATS, MAXCHATS );
			CHATSTRING[0] = 0;
			AddChat = 1;
        }
        if ( NCHATS > NCL )
		{
			CHSCR->SMaxPos = NCHATS - NCL;
			if ( AddChat )CHSCR->SPos = CHSCR->SMaxPos;
			CHSCR->Visible = 1;
			CHVIEW->ChatDY = CHSCR->SPos;
		}
		else
		{
			CHSCR->Visible = 0;
			CHSCR->SPos = 0;
			CHVIEW->ChatDY = 0;
        };

		SendPings();
		StdKeys();
		if ( ItemChoose == mcmOk && !OkBtn->Enabled )ItemChoose = -1;
		AnalyseMessages();
		if ( GameInProgress )goto FinCLC;
		if ( MyOldVers )GPPB->Hint = OLDV;
		MENU.MarkToDraw();
		MENU.ProcessDialogs();
		if ( GameInProgress )goto FinCLC;
		char ccc[128];
		if ( PSUMM.NPL&&PSUMM.PSET[0].NPings )
		{
			int png = GetMaxRealPing();

			//png=GetAveragePing();
			//MaxPingTime=png;
			if ( GetRealTime() - PPTIME > 1000 )
			{
				DWORD lpp[3];
                // BoonXRay 22.08.2017
                //lpp[0] = 'PNTF';
                lpp[0] = ConvCosChar('P', 'N', 'T', 'F');
				lpp[1] = png;
				lpp[2] = lpp[0] + lpp[1];
				SendToAllPlayers( 12, lpp );
				PPTIME = GetRealTime();
				for ( int p = 0; p < NPlayers; p++ )if ( PINFO[p].PlayerID == MyDPID )CurrentMaxPing[p] = png;
            }
			for ( int p = 0; p < NPlayers; p++ )if ( CurrentMaxPing[p] > png )png = CurrentMaxPing[p];
			MaxPingTime = png;
			if ( png < MinPing0 )MaxPingTime = 0;//FIXIT!!!!!!!!!!!
			if ( png >= MinPing0&&png < 300 )MaxPingTime = 300;
			if ( MaxPingTime )MaxPingTime += 500;
			char cc2[300];
			CreateDiffStr( cc2 );
			sprintf( ccc, "max ping: %d  (%s)", png, cc2 );
			ShowString( 10, 768 - 24, ccc, &SmallYellowFont );
        }

		MENU.RefreshView();
		if ( GameInProgress )
		{
			goto FinCLC;
		}

		ReadyFlow[0]->Visible = 0;
		ReadyFlow[1]->Visible = 0;
		if ( Host )
		{
			if ( HostID != -1 )
			{
				PINFO[HostID].MapName[0] = 0;
				strcpy( PINFO[HostID].MapName, WARS.Battles[BattleID].Map );
				strcpy( CurrentMap, PINFO[HostID].MapName );
				MNATION[HostID]->Enabled = 1;
				MNATION[1 - HostID]->Enabled = 0;
				int PPP;
				if ( ( PPP = GetReadyPercent() ) < 100 )
				{
					sprintf( ReadyFlow[HostID]->Message, "%d%%", PPP );
					ReadyFlow[HostID]->Visible = 1;
					MREADY[HostID]->Visible = 0;
					MREADY[HostID]->Enabled = 0;
					NOMREADY = HostID;
				}
				else
				{
					ReadyFlow[HostID]->Visible = 0;
					MREADY[HostID]->Visible = 1;
					MREADY[HostID]->Enabled = 1;
					NOMREADY = -1;;
				}
			}
		}
		else
		{
			MNATION[0]->Enabled = 0;
			MNATION[1]->Enabled = 0;
			ReadyFlow[0]->Visible = 0;
			ReadyFlow[1]->Visible = 0;

		}
		if ( HostID != -1 )
		{
			MNATION[1 - HostID]->CurLine = 1 - MNATION[HostID]->CurLine;
			for ( int i = 0; i < 2; i++ )
			{
				ColorBack[i]->Nation = MNATION[i]->CurLine;
				Flags[i]->Nation = MNATION[i]->CurLine;
			}
		}
		MaxVers = 0;
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].Version > MaxVers )MaxVers = PINFO[i].Version;
		}
		for ( int i = 0; i < 2; i++ )
		{
			if ( i < NPlayers )
			{
				if ( PINFO[i].PlayerID == MyDPID )
				{
					PINFO[i].NationID = MNATION[i]->CurLine;
					PINFO[i].ColorID = ColorBack[i]->Nation;
					bool change = ( memcmp( &MYPINF, PINFO + i, sizeof MYPINF ) != 0 );
					if ( change || GetTickCount() - PREVSD > 3000 )
					{
						SETPLAYERDATA( MyDPID, (void*) &PINFO[i].NationID, sizeof( PlayerInfo ) - 36, change );
						memcpy( &MYPINF, PINFO + i, sizeof MYPINF );
						PREVSD = GetTickCount();
					}
					if ( change && Host )
					{
						PSUMM.ClearPingInfo();
					}
					DPNAME dpName;
					ZeroMemory( &dpName, sizeof( DPNAME ) );
					dpName.dwSize = sizeof( DPNAME );
                    // BoonXRay 20.08.2017
                    //dpName.lpszShortNameA = MNAME[i]->Str;
                    dpName.lpszShortNameA = const_cast<char *>(MNAME[i]->String.data());
					dpName.lpszLongNameA = nullptr;
					if ( strcmp( dpName.lpszShortNameA, oldname ) )
					{
						SETPLAYERNAME( &dpName, strcmp( dpName.lpszShortNameA, oldname ) != 0 );
						strcpy( oldname, dpName.lpszShortNameA );
                    }
					//MNATION[i]->Enabled=true;
					//MGRP[i]->Enabled=true;
					NameBack[i]->Visible = 1;
					MNAME[i]->Visible = 1;
					MNAME[i]->Enabled = 0;//1;
					NationBack[i]->Visible = 1;
					NationBack[i]->Enabled = 1;
					CD_Back[i]->Visible = 1;
					//MNATION[i]->Enabled=1;
					ColorBack[i]->Visible = 1;
					ColorBack[i]->Enabled = 1;
					ReadyBack[i]->Visible = 1;
					MyOldVers = PINFO[i].Version != MaxVers;
				}
				else
				{
					NationBack[i]->Visible = 1;
					NationBack[i]->Enabled = 1;
					MNAME[i]->Visible = 1;
					MNAME[i]->Enabled = 0;
					MNAME[i]->Active = 0;
					NameBack[i]->Visible = 0;
					//MNATION[i]->Enabled=0;
					CD_Back[i]->Visible = 0;
					ColorBack[i]->Visible = 0;
					ColorBack[i]->Enabled = 0;
					ReadyBack[i]->Visible = 0;
					//MGRP[i]->Enabled=false;
                }
				if ( MNAME[i]->Active )
				{
					NameBack[i]->SpriteID = 12 + ( i % 3 );
				}
				else
				{
					NameBack[i]->SpriteID = 9 + ( i % 3 );
				}
				Flags[i]->SpriteID = ( i * 229 + GetRealTime() / 30 ) % 45;
				Flags[i]->Nation = ColorBack[i]->Nation;
				MNATION[i]->Visible = true;
				//MCOLOR[i]->Visible=true;
				MREADY[i]->SpriteID = 26 + ( PINFO[i].Ready != 0 );
				if ( i != NOMREADY )
				{
					MREADY[i]->Visible = true;
					MREADY[i]->Enabled = true;
				}
				Flags[i]->Visible = 1;
				CD_Present[i]->Visible = 1;
				//MREADY[i]->Visible=1;
				MNATION[i]->Visible = 1;
				//ReadyBack[i]->Visible=0;
				//MGRP[i]->Visible=true;
				int v = PINFO[i].Version;
				char VERS[128];
				if ( v > 255 )
				{
					sprintf( VERS, "M%d", v >> 8 );
				}
				else
				{
					sprintf( VERS, "%d.%d%d", v / 100, ( v / 10 ) % 10, v % 10 );
				}
				strcpy( VersText[i]->Message, VERS );
				strcpy( VersTextShadow[i]->Message, VERS );
				if ( v != MaxVers && ( GetTickCount() / 300 ) & 1 )
				{
					VersText[i]->ActiveFont = &SmallRedFont;
					VersText[i]->PassiveFont = &SmallRedFont;
				}
				else
				{
					VersText[i]->ActiveFont = &SmallYellowFont;
					VersText[i]->PassiveFont = &SmallYellowFont;
				}
				VersText[i]->Visible = 1;
				VersTextShadow[i]->Visible = 1;
				PresOldVers = v != MaxVers;
			}
			else
			{
				NameBack[i]->Visible = 0;
				NationBack[i]->Visible = 0;
				ColorBack[i]->Visible = 0;
				Flags[i]->Visible = 0;
				CD_Back[i]->Visible = 0;
				CD_Present[i]->Visible = 0;
				ReadyBack[i]->Visible = 0;
				MNAME[i]->Visible = 0;
				MNAME[i]->Enabled = 0;
				MNAME[i]->Active = 0;
				MREADY[i]->Visible = 0;
				MNATION[i]->Visible = 0;
				//MGRP[i]->Visible=false;
				VersText[i]->Visible = 0;
				VersTextShadow[i]->Visible = 0;
			}
		}
		if ( !PIEnumeratePlayers( PINFO, 1 ) )
		{
			ItemChoose = mcmCancel;
		}
		if ( NPlayers > 2 )
		{
			NPlayers = 2;
		}
		if ( !Host )
		{
			int HostID = -1;
			int MyID = -1;
			for ( int i = 0; i < NPlayers; i++ )
			{
				if ( PINFO[i].Host )
				{
					HostID = i;
				}
				if ( PINFO[i].PlayerID == MyDPID )
				{
					MyID = i;
				}
			}
			if ( HostID != -1 && MyID != -1 )
			{
				if ( PINFO[MyID].MapStyle != PINFO[HostID].MapStyle )
				{
					PINFO[MyID].MapStyle = PINFO[HostID].MapStyle;
					BattleID = SetCurrentBattle( PINFO[MyID].MapStyle, &Prev, MNATION, BTTXT );
					MYPINF.MapStyle = PINFO[MyID].MapStyle;
				}
				strcpy( PINFO[MyID].MapName, PINFO[HostID].MapName );
				//PINFO[MyID].MapStyle=0;
				PINFO[MyID].HillType = 0;
				PINFO[MyID].StartRes = 0;
				PINFO[MyID].ResOnMap = 0;
				strcpy( CurrentMap, PINFO[HostID].MapName );
			}
		}
		for ( int i = 0; i < NPlayers; i++ )
		{
			MNATION[i]->CurLine = PINFO[i].NationID;
			ColorBack[i]->Nation = PINFO[i].ColorID;
			if ( PINFO[i].PlayerID == MyDPID&&ItemChoose == mcmOk && !Host )
			{
				PINFO[i].Ready = 1;
			}
		}
		GMREADY = true;
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].PlayerID != MyDPID && !PINFO[i].Ready )
			{
				GMREADY = false;
			}
		}
		MSS1 = 0;
		for ( int i = 0; i < NPlayers; i++ )
		{
			byte MSS2 = 1 << PINFO[i].ColorID;
			if ( MSS1 & MSS2 )
			{
				GMREADY = false;
			}
			MSS1 |= MSS2;
			if ( !PINFO[i].Ready )
			{
				GMREADY = false;
			}
		}
		if ( Host )
		{
			if ( NPlayers < 2 )
			{
				GMREADY = false;
			}
			if ( !CheckPingsReady() )
			{
				GMREADY = false;
			}
			if ( PresOldVers )
			{
				GMREADY = 0;
			}
			OkBtn->Enabled = GMREADY;
			OkBtn->Visible = GMREADY;
		}
		else
		{
			OkBtn->Enabled = !MyOldVers;
			OkBtn->Visible = !MyOldVers;
		}
		//checking readyness
		if ( ItemChoose == mcmOk && !Host )
		{
			ItemChoose = -1;
		}
		if ( pp )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pp = 0;
			ReceiveAll();
			//PSUMM.ClearPingInfo();
		}
		if ( ItemChoose == 99 )
		{
			ProcessInternetConnection( 0 );
			ItemChoose = -1;
			if ( PlayerMenuMode != 1 )
			{
				ItemChoose = mcmSingle;
			}
		}
	FinCLC:;
	} while ( ItemChoose == -1 && PlayerMenuMode == 1 );
	PINFO[0].GameTime = 0;
	PINFO[0].VictCond = 0;
	if ( NCHATS )
	{
		for ( int i = 0; i < NCHATS; i++ )
		{
			free( ChatMess[i] );
			free( ChatSender[i] );
		}
		free( ChatMess );
		free( ChatSender );
	}
	return ( ItemChoose == mcmOk ) || PlayerMenuMode != 1;//ItemChoose==mcmOk;
}

int DetermineNationAI(uint8_t Nat )
{
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && OB->NNUM == Nat && OB->newMons->Peasant && !OB->Sdoxlo )
		{
			char* name = OB->Ref.General->MonsterID;
			for ( int j = 0; j < GlobalAI.NAi; j++ )
			{
				if ( !strcmp( GlobalAI.Ai[j].Peasant_ID, name ) )
				{
					return j;
				}
			}
		}
	}
	return -1;
}

static bool SingleOptions()
{
	if ( MPL_WaitingGame( 1, 1 ) )
	{
		StartIGame( 1 );
		return true;
	}
	else
	{
		return false;
	}
}

static int MM_ProcessMultiPlayer()
{
	GoHomeAnyway();

	LocalGP BTNS( "Interface\\Multi_Player" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();

	SQPicture MnPanel( "Interface\\Background_Multi_Player.bmp" );
	DialogsSystem MMenu( menu_x_off, menu_y_off );
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;
	int Dy = 110;
    /*Picture* PIC =*/ MMenu.addPicture( nullptr, 0, 0, &MnPanel, &MnPanel, &MnPanel );
	GP_Button* DeathM = MMenu.addGP_Button( nullptr, 76, 140 + Dy, BTNS.GPID, 0, 1 );
	DeathM->UserParam = 1;
	DeathM->OnUserClick = &MMItemChoose;
	DeathM->Hint = GetTextByID( "MDEATHM" );
	DeathM->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );
	GP_Button* HistBatt = MMenu.addGP_Button( nullptr, 76, 140 + 82 + Dy, BTNS.GPID, 2, 3 );
	HistBatt->UserParam = 2;
	HistBatt->OnUserClick = &MMItemChoose;
	HistBatt->Hint = GetTextByID( "MHISTBATT" );
	HistBatt->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );
	GP_Button* Back = MMenu.addGP_Button( nullptr, 76, 140 + 82 * 2 + Dy, BTNS.GPID, 4, 5 );
	Back->UserParam = 5;
	Back->OnUserClick = &MMItemChoose;
	Back->Hint = GetTextByID( "MBACK" );
	Back->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );
	ItemChoose = -1;
	UnPress();
	Lpressed = 0;
	LastKey = 0;
	KeyPressed = 0;
	int pp = 1;
	do
	{
		ProcessMessages();
		if ( KeyPressed&&LastKey == 27 )
		{
			ItemChoose = 5;
			KeyPressed = 0;
        }
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
		if ( pp )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pp = 0;
        }

		if ( ItemChoose == 2 && !WARS.NWars )
		{
			ItemChoose = -1;
		}
	} while ( ItemChoose == -1 );
	SlowUnLoadPalette( "2\\agew_1.pal" );
	if ( ItemChoose == 1 )
	{
		processMultiplayer();
		if ( TOTALEXIT )
		{
			return mcmCancel;
		}
	}

	if ( ItemChoose == 2 )
	{
		if ( WARS.NWars )
		{
			processBattleMultiplayer();
		}
	}
	return ItemChoose;
}


/*
Accept      - 153x239
Cancel      - 304x239
Low_Speed   - 224x120
High_Speed  - 397x120
Low_Scrol   - 224x151
High_Scrol  - 397x151
Low_Music   - 224x182
High_Music  - 397x182
Low_Sound   - 224x214
High_Sound  - 397x214

Слайдер:
Low - левая верхняя точка минимального значения,
High - левая верхняя точка максимально значения.

Low_Speed   - 233x120
High_Speed  - 383x120
Low_Scrol   - 233x151
High_Scrol  - 383x151
Low_Music   - 233x182
High_Music  - 383x182
Low_Sound   - 233x214
High_Sound  - 383x214
*/

int PlayMode = 2;
char RECFILE[128] = "";

static bool ProcessMenuOptions()
{
	LocalGP BTNS( "Interface\\Options" );

	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();


	SQPicture MnPanel( "Interface\\Background_options.bmp" );

	DialogsSystem MMenu( menu_x_off, menu_y_off );
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;

    /*Picture* PIC =*/ MMenu.addPicture( nullptr, 0, 0, &MnPanel, &MnPanel, &MnPanel );

	GP_Button* OkBtn = MMenu.addGP_Button( nullptr, 113, 534 + 10 + 13, BTNS.GPID, 8, 9 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;

	GP_Button* ExitBtn = MMenu.addGP_Button( nullptr, 333, 534 + 10 + 13, BTNS.GPID, 10, 11 );
	ExitBtn->UserParam = mcmExit;
	ExitBtn->OnUserClick = &MMItemChoose;

	if ( ScrollSpeed < 2 )
	{
		ScrollSpeed = 2;
	}

	int X0 = 244 - 2;
	int LX = 484 - 244 + 20;
	int DY = 10;

	GPPicture* P1 = MMenu.addGPPicture( nullptr, 240, 317 + 10 - 13, BTNS.GPID, 13 );
	GPPicture* P2 = MMenu.addGPPicture( nullptr, 240, 343 + 10 - 13, BTNS.GPID, 14 );
	GPPicture* P3 = MMenu.addGPPicture( nullptr, 240, 447 + 10 - 13, BTNS.GPID, 15 );
	P1->Visible = 0;
	P2->Visible = 0;
	P3->Visible = 0;

	//sound volume (#1)
	VScrollBar* SVL = MMenu.addGP_ScrollBarL( nullptr, X0, 346 + DY + 10 - 13, 100, WarSound, BTNS.GPID, 12, LX, 12, 0, 0 );

	//music volume (#2)
	MidiSound = GetCDVolume();
	VScrollBar* MVL = MMenu.addGP_ScrollBarL( nullptr, X0, 320 + DY + 10 - 13, 100, MidiSound, BTNS.GPID, 12, LX, 12, 0, 0 );

	//scroll speed (#3)
	VScrollBar* SSP = MMenu.addGP_ScrollBarL( nullptr, X0, 450 + DY + 10 - 13, 8, ScrollSpeed - 2, BTNS.GPID, 12, LX, 12, 0, 0 );

	//Recording checkbox
	GP_Button* REC = MMenu.addGP_Button( nullptr, 240, 496, BTNS.GPID, 20, 19 );
	REC->OnUserClick = &MMItemChoose;
	REC->UserParam = 77;
	bool RECSTATE = true;//RecordMode;//BUGFIX: leave record mode always turned on
	//Recodring file path
	GP_Button* EDITPAN = MMenu.addGP_Button( nullptr, 270, 496, BTNS.GPID, 24, 23 );
	InputBox* IBREC = MMenu.addInputBox( nullptr, 275, 496, RECFILE, 120, 240, 26, &YellowFont, &WhiteFont );

	//Resloutions drop down list in main menu
	ComboBox* VMode = MMenu.addGP_ComboBox( nullptr, 240, 240 - 16 - 13, BTNS.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	VMode->CurLine = 100;
	for ( int i = 0; i < NModes; i++ )
	{
		char cc[30];
		char c = ' ';
		double scale = 0.01;

		//Mark fitting aspect ratios with an *
		double res_ratio = (double) ModeLX[i] / ModeLY[i];
		res_ratio = (int) ( res_ratio / scale ) * scale;
		if ( res_ratio == screen_ratio )
		{
			c = '*';
		}

		sprintf( cc, "%dx%d %c", ModeLX[i], ModeLY[i], c );

		VMode->AddLine( cc );
		if ( exRealLx == ModeLX[i] && exRealLy == ModeLY[i] )
		{
			VMode->CurLine = i;
		}
	}
	if ( VMode->CurLine == 100 )
	{
		VMode->CurLine = 0;
	}

	//Game speed mode
	ComboBox* FMode = MMenu.addGP_ComboBox( nullptr, 240, 421 + 10 - 13, BTNS.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	FMode->AddLine( SLOWMOD );
	FMode->AddLine( FASTMOD );
	FMode->CurLine = exFMode;

	ComboBox* MUS = MMenu.addGP_ComboBox( nullptr, 240, 240 - 16 + 80 - 3 - 13, BTNS.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	MUS->AddLine( GetTextByID( "NOCD" ) );
	MUS->AddLine( GetTextByID( "NATMUS" ) );
	MUS->AddLine( GetTextByID( "RNDMUS" ) );
	MUS->CurLine = PlayMode;

	ItemChoose = -1;
	int pp = 1;
	KeyPressed = 0;
	int PrevPlayMode = MUS->CurLine != 0;
	int ppp = PlayMode;
	do
	{
		if ( true )//(RECSTATE) //BUGFIX: record mode shall not be turned off
		{
			EDITPAN->Enabled = 1;
			IBREC->Enabled = 1;
			REC->ActiveFrame = 22;
			REC->PassiveFrame = 21;
			IBREC->Active = 1;
		}
		else
		{
			EDITPAN->Enabled = 0;
			IBREC->Enabled = 0;
			REC->ActiveFrame = 20;
			REC->PassiveFrame = 19;
			IBREC->Active = 0;
		}

		P1->Visible = P1->MouseOver;
		P2->Visible = P2->MouseOver;
		P3->Visible = P3->MouseOver;

		ProcessMessages();
		StdKeys();
		MMenu.MarkToDraw();
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
		if ( ItemChoose == 77 )
		{
			RECSTATE = !RECSTATE;
			ItemChoose = -1;
		}
		if ( pp )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pp = 0;
		}
		SetCDVolume( MVL->SPos );
		if ( PrevPlayMode != ( MUS->CurLine != 0 ) )
		{
			PlayMode = MUS->CurLine;
			if ( MUS->CurLine )PlayRandomTrack();
			else StopPlayCD();
			PrevPlayMode = MUS->CurLine != 0;
		}
	} while ( ItemChoose == -1 );

	//Apply game settings
	if ( ItemChoose == mcmOk )
	{
		ScrollSpeed = SSP->SPos + 2;
		exRealLx = ModeLX[VMode->CurLine];
		exRealLy = ModeLY[VMode->CurLine];
		WarSound = SVL->SPos;
		MidiSound = MVL->SPos;
		PlayMode = MUS->CurLine;
		exFMode = FMode->CurLine;
		RecordMode = RECSTATE;
	}
	else
	{
		PlayMode = ppp;
		if ( ( PlayMode != 0 ) != PrevPlayMode )
		{
			if ( PlayMode )
			{
				PlayRandomTrack();
			}
			else
			{
				StopPlayCD();
			}
		}
	}
	SlowUnLoadPalette( "2\\agew_1.pal" );
	return false;
}

static bool ProcessGameOptions()
{
	LocalGP BTNS( "Interface\\Options_for_Game" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );

	hfnt.SetWhiteColor();

	int dx = -63 + 6 + ( ( RealLx - 484 ) / 2 );
	int dy = -166 + ( ( RealLy - 425 ) / 2 );

	DialogsSystem MMenu( dx, dy );
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;

    /*GPPicture* PIC =*/ MMenu.addGPPicture( nullptr, 63, 166, BTNS.GPID, 14 );
	GPS.ShowGP( dx + 63, dy + 166, BTNS.GPID, 13, 0 );

	GP_Button* OkBtn = MMenu.addGP_Button( nullptr, 113, 534 + 11, BTNS.GPID, 8, 9 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;

	GP_Button* ExitBtn = MMenu.addGP_Button( nullptr, 333, 534 + 11, BTNS.GPID, 10, 11 );
	ExitBtn->UserParam = mcmExit;
	ExitBtn->OnUserClick = &MMItemChoose;

	if ( ScrollSpeed < 2 )
	{
		ScrollSpeed = 2;
	}

	int X0 = 244 - 2;
	int LX = 484 - 244 + 20;
	int DY = 10;

	GPPicture* P1 = MMenu.addGPPicture( nullptr, 240, 317 + 11, BTNS.GPID, 15 );
	GPPicture* P2 = MMenu.addGPPicture( nullptr, 240, 343 + 11, BTNS.GPID, 16 );
	GPPicture* P3 = MMenu.addGPPicture( nullptr, 240, 447 + 11, BTNS.GPID, 17 );
	P1->Visible = 0;
	P2->Visible = 0;
	P3->Visible = 0;

	//sound volume (#1)
	MidiSound = GetCDVolume();
	VScrollBar* SVL = MMenu.addGP_ScrollBarL( nullptr, X0, 346 + DY + 11, 100, WarSound, BTNS.GPID, 12, LX, 12, 0, 0 );

	//music volume (#2)
	VScrollBar* MVL = MMenu.addGP_ScrollBarL( nullptr, X0, 320 + DY + 11, 100, MidiSound, BTNS.GPID, 12, LX, 12, 0, 0 );

	//scroll speed (#3)
	VScrollBar* SSP = MMenu.addGP_ScrollBarL( nullptr, X0, 450 + DY + 11, 8, ScrollSpeed - 2, BTNS.GPID, 12, LX, 12, 0, 0 );

	//Resolutions dropdown list in game menu
	ComboBox* VMode = MMenu.addGP_ComboBox( nullptr, 240, 240 - 15, BTNS.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	VMode->CurLine = 100;

	//Calculate native resolution aspect ratio
	double scale = 0.01;
	double screen_ratio = (double) screen_width / screen_height;
	screen_ratio = (int) ( screen_ratio / scale ) * scale;

	for ( int i = 0; i < NModes; i++ )
	{
		char cc[30];
		char c = ' ';

		//Mark fitting aspect ratios with an *
		double res_ratio = (double) ModeLX[i] / ModeLY[i];
		res_ratio = (int) ( res_ratio / scale ) * scale;
		if ( res_ratio == screen_ratio )
		{
			c = '*';
		}

		sprintf( cc, "%dx%d %c", ModeLX[i], ModeLY[i], c );

		VMode->AddLine( cc );
		if ( RealLx == ModeLX[i] && RealLy == ModeLY[i] )
		{
			VMode->CurLine = i;
		}
	}
	if ( VMode->CurLine == 100 )
	{
		VMode->CurLine = 0;
	}

	//Speedmode
	ComboBox* FMode = MMenu.addGP_ComboBox( nullptr, 240, 421 + 11, BTNS.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	FMode->AddLine( SLOWMOD );
	FMode->AddLine( FASTMOD );
	FMode->CurLine = exFMode;
	if ( 1 < NPlayers )
	{//Disable speed mode drop down list in multiplayer
		FMode->Enabled = false;
		FMode->Active = false;
	}

	ComboBox* MUS = MMenu.addGP_ComboBox( nullptr, 240, 240 - 15 + 80 - 3, BTNS.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	MUS->AddLine( GetTextByID( "NOCD" ) );
	MUS->AddLine( GetTextByID( "NATMUS" ) );
	MUS->AddLine( GetTextByID( "RNDMUS" ) );
	MUS->CurLine = PlayMode;

	int PrevPlayMode = MUS->CurLine != 0;
	int ppp = PlayMode;
	int StartTime = GetTickCount();

	ItemChoose = -1;
	KeyPressed = 0;

	do
	{
		if ( GetTickCount() - StartTime > 10000 && NPlayers > 1 )
		{
			ItemChoose = mcmCancel;
			ShowOptMessage( "#LOOKOPT", 0 );
		}
		P1->Visible = P1->MouseOver;
		P2->Visible = P2->MouseOver;
		P3->Visible = P3->MouseOver;
		ProcessMessages();
		StdKeys();
		MMenu.MarkToDraw();
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
		SetCDVolume( MVL->SPos );
		if ( PrevPlayMode != ( MUS->CurLine != 0 ) )
		{
			PlayMode = MUS->CurLine;
			if ( MUS->CurLine )
			{
				PlayRandomTrack();
			}
			else
			{
				StopPlayCD();
			}
			PrevPlayMode = MUS->CurLine != 0;
		}
	} while ( ItemChoose == -1 );

	if ( mcmOk == ItemChoose )
	{
		ScrollSpeed = SSP->SPos + 2;

		//Just in case
		if ( 1 == NPlayers )
		{//Allow ingame speed mode changing only for single player
			exFMode = FMode->CurLine;
		}

		if ( RealLx != ModeLX[VMode->CurLine] || RealLy != ModeLY[VMode->CurLine] )
		{
			SetGameDisplayMode( ModeLX[VMode->CurLine], ModeLY[VMode->CurLine] );
		}

		WarSound = SVL->SPos;
		MidiSound = MVL->SPos;
		PlayMode = MUS->CurLine;
	}
	else
	{
		PlayMode = ppp;
		if ( ( PlayMode != 0 ) != PrevPlayMode )
		{
			if ( PlayMode )
			{
				PlayRandomTrack();
			}
			else
			{
				StopPlayCD();
			}
		}
	}

	return false;
}

static void SlideShow();

static int GetRndVid( int N );
bool InMainMenuLoop = 0;

//Draw main menu and process events
static int processMainMenu()
{
	InGame = false;
	InEditor = false;

	curptr = 0;

	if ( window_mode )
	{//Adjust to default menu size after exiting game
		ResizeAndCenterWindow();
	}

	//Time of inactivity in ms which triggers the credits
	const int kCreditsDemoDelay = 300000;

	int StTime = GetTickCount();

	CloseMPL();

	SetDarkPalette();

	GameMode = 1;

	SetRLCWindow( 0, 0, RealLx, RSCRSizeY, SCRSizeX );

	NPlayers = 1;
	ItemChoose = -1;
	CBar( 0, 0, RealLx, RSCRSizeY, 0 );

	if ( !window_mode )
	{//Calculate offsets for centering menu in fullscreen mode
		menu_x_off = ( screen_width - 1024 ) / 2;
		menu_y_off = ( screen_height - 768 ) / 2;
		menu_hint_x = 18 + menu_x_off;
		menu_hint_y = 701 + menu_y_off;
	}

	//Loading resources.
	LocalGP BTNS( "Interface\\Main_Menu" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();

	SQPicture MnPanel( "Interface\\Background_Main_Menu.bmp" );

	DialogsSystem MMenu( menu_x_off, menu_y_off );
	MMenu.HintFont = &hfnt;
	MMenu.HintX = menu_hint_x;
	MMenu.HintY = menu_hint_y;

    /*Picture* PIC =*/ MMenu.addPicture( nullptr, 0, 0, &MnPanel, &MnPanel, &MnPanel );

	GP_Button* Single = MMenu.addGP_Button( nullptr, 76, 140, BTNS.GPID, 0, 1 );
	Single->UserParam = mcmSingle;
	Single->OnUserClick = &MMItemChoose;
	Single->Hint = GetTextByID( "MMSINGLE" );
	Single->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );

	GP_Button* Multi = MMenu.addGP_Button( nullptr, 76, 140 + 82, BTNS.GPID, 2, 3 );
	Multi->UserParam = mcmMulti;
	Multi->OnUserClick = &MMItemChoose;
	Multi->Hint = GetTextByID( "MMMULTI" );
	Multi->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );

	GP_Button* Load = MMenu.addGP_Button( nullptr, 76, 140 + 82 * 2, BTNS.GPID, 4, 5 );
	Load->UserParam = mcmLoad;
	Load->OnUserClick = &MMItemChoose;
	Load->Hint = GetTextByID( "MMLOAD" );
	Load->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );

	GP_Button* Options = MMenu.addGP_Button( nullptr, 76, 140 + 82 * 3, BTNS.GPID, 6, 7 );
	Options->UserParam = mcmEdit;
	Options->OnUserClick = &MMItemChoose;
	Options->Hint = GetTextByID( "MMOPTIONS" );
	Options->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );

	//Editor button
	GP_Button* Video = MMenu.addGP_Button( nullptr, 76, 140 + 82 * 4, BTNS.GPID, 8, 9 );
	Video->UserParam = mcmVideo;
	Video->OnUserClick = &MMItemChoose;
	Video->Hint = GetTextByID( "MMVIDEO" );
	Video->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );

	GP_Button* Exit = MMenu.addGP_Button( nullptr, 76, 140 + 82 * 5, BTNS.GPID, 10, 11 );
	Exit->UserParam = mcmExit;
	Exit->OnUserClick = &MMItemChoose;
	Exit->Hint = GetTextByID( "MMEXIT" );
	Exit->AssignSound( GETS( "@MOUSESOUND" ), MOUSE_SOUND );
	MMenu.addTextButton( nullptr, 1024 - GetRLCStrWidth( BuildVersion, &SmallYellowFont1 ), 748, BuildVersion, &SmallYellowFont1, &SmallYellowFont1, &SmallYellowFont1, 0 );

	int nn = 0;

	while ( true )
	{
		if ( CurrentCampagin != -1 && CurrentMission != -1 )
		{
			ProcessCampagins( CurrentCampagin );
			nn = 1;
			if ( ItemChoose == mcmSingle )
			{
				return ItemChoose;
			}
		}

		if ( nn )
		{
			nn = 0;
		}

		ItemChoose = -1;

		LoadFog( 2 );

		StTime = GetTickCount();

		LastKey = 0;
		KeyPressed = 0;

		do
		{
			//Check conditions and roll credits
			if ( ADDSH == 1 && bActive
				&& ( GetTickCount() - StTime > kCreditsDemoDelay || RetryVideo )
				&& !( RUNMAPEDITOR || RUNUSERMISSION ) )
			{
				GFILE* f = Gopen( "Demo\\demo.run", "r" );
				if ( f )
				{
					int N;
					int z = Gscanf( f, "%d", &N );
					if ( z == 1 )
					{
						int M = GetRndVid( N );
						char NAME[64];
						for ( int i = 0; i < M; i++ )
						{
							Gscanf( f, "%s", NAME );
						}
						Gscanf( f, "%s", NAME );
						RGAME.ReadStream( NAME );
						if ( RGAME.Size )
						{
							PlayGameMode = 1;
							RecordMode = 0;
							ItemChoose = 43;
							RetryVideo = 1;
							KeyPressed = 0;
						}
					}
					Gclose( f );
					StTime = GetTickCount();
				}
			}

			InMainMenuLoop = 1;

			ProcessMessages();

			MMenu.ProcessDialogs();

			MMenu.RefreshView();

			InMainMenuLoop = 0;
			if ( !nn )
			{
				SlowLoadPalette( "2\\agew_1.pal" );
				LoadFog( 2 );
			}
			nn++;

			if ( RUNMAPEDITOR )
			{
				ItemChoose = mcmVideo;
			}

			if ( RUNUSERMISSION )
			{
				ItemChoose = 111;
			}

			if ( TOTALEXIT )
			{
				ItemChoose = mcmExit;
			}
		} while ( ItemChoose == -1 );

		if ( ItemChoose == mcmSingle )
		{
			SlowUnLoadPalette( "2\\agew_1.pal" );
			if ( EnterName() )
			{
				int res = MM_ProcessSinglePlayer();
				if ( res == 5 )
				{
					continue;
				}
			}
		}

		if ( ItemChoose == mcmMulti )
		{
			SlowUnLoadPalette( "2\\agew_1.pal" );
			int res = MM_ProcessMultiPlayer();
			if ( res == 5 )
			{
				continue;
			}
		}

		if ( ItemChoose == mcmVideo )
		{
			ShowLoading();
			LOADNATMASK = 0xFFFFFFFF;
			PrepareEditMedia( 0 );
			ItemChoose = mcmSingle;
		}

		if ( ItemChoose == mcmEdit )
		{
			SlowUnLoadPalette( "2\\agew_1.pal" );
			ProcessMenuOptions();
			continue;
		}

		if ( ItemChoose == mcmLoad )
		{
			SlowUnLoadPalette( "2\\agew_1.pal" );
			processMLoadGame();
		}

		if ( ItemChoose == mcmExit )
		{
			SlowUnLoadPalette( "2\\agew_1.pal" );
			SlideShow();
		}

		if ( ItemChoose == 43 )
		{
			EditMapMode = 0;
			ItemChoose = mcmSingle;
		}

		if ( ItemChoose == 111 )
		{
			if ( !PlName[0] )
			{
				strcpy( PlName, "Player" );
			}
			memset( PINFO, 0, sizeof PINFO );
			strcpy( PINFO[0].name, PlName );
			EditMapMode = 0;
			ItemChoose = mcmSingle;
			strcpy( CurrentMap, USERMISSPATH );
			CreateNationalMaskForMap( CurrentMap );
			CurrentCampagin = -1;
			CurrentMission = -1;
			PrepareGameMedia( 0, 0 );
            ItemChoose = mcmSingle;
			RUNUSERMISSION = 0;
		}
		break;
	}

	GameMode = 0;
	return ItemChoose;
}
//--------------=< G A M E   S C R E E N >=------------//

static DialogsSystem GSYS( menu_x_off, menu_y_off );

void GSYSDRAW()
{
	GSYS.RefreshView();
}

bool GameExit;
int MenuType;
bool MakeMenu;

static int NNames;
static char* names[128];
static char* fnames[128];
static int   serials[128];

static void ClearNames()
{
	for ( int i = 0; i < 128; i++ )
	{
		names[i] = nullptr;
	}

	NNames = 0;
}

static void FreeNames()
{
	for ( int i = 0; i < 128; i++ )
	{
		if ( names[i] )
		{
			free( names[i] );
			free( fnames[i] );
			names[i] = nullptr;
			fnames[i] = nullptr;
		}
	}
	NNames = 0;
}

static void InstallName( ListBox* LB, WIN32_FIND_DATA* FD, const char* StartDir )
{
	char CCC[256];
	sprintf( CCC, "%s%s", StartDir, FD->cFileName );
	ResFile ff1 = RReset( CCC );
	if ( ff1 != INVALID_HANDLE_VALUE )
	{
		int sig, lap;
		RBlockRead( ff1, &sig, 4 );
        // BoonXRay 22.08.2017
        //if ( sig == 'FSAK' || sig == '[CER' || sig == ']CER' )
        if ( (sig == ConvCosChar('F', 'S', 'A', 'K')) || (sig == ConvCosChar('[', 'C', 'E', 'R')) || (sig == ConvCosChar(']', 'C', 'E', 'R')) )
		{
            // BoonXRay 22.08.2017
            //if ( sig == '[CER' || sig == ']CER' )sig = sfVersion;
            if ( (sig == ConvCosChar('[', 'C', 'E', 'R')) || (sig == ConvCosChar(']', 'C', 'E', 'R')) ) sig = sfVersion;
			else
			{
				RBlockRead( ff1, &sig, 4 );
				RBlockRead( ff1, &lap, 4 );
            }
			if ( sig == sfVersion&&NNames < 512 )
			{
				//int nlen=0;
				//RBlockRead(ff1,&nlen,2);
				//nlen=0;
				//RBlockRead(ff1,&nlen,1);
				//RBlockRead(ff1,nam,nlen);
				RClose( ff1 );
				LB->AddItem( CCC, NNames );
				//names[NNames]=new char[strlen(nam)+1];
				fnames[NNames] = new char[strlen( CCC ) + 1];
				//serials[NNames]=lap;
				//strcpy(names[NNames],nam);
				strcpy( fnames[NNames], CCC );
				NNames++;
			}
			else RClose( ff1 );
		}
		else RClose( ff1 );
	}
}

static void SFLB_CreateGamesList( ListBox* LB )
{
	WIN32_FIND_DATA FD;
	ClearNames();
	HANDLE HF = FindFirstFile( "*.sav", &FD );
	if ( HF != INVALID_HANDLE_VALUE )
	{
		InstallName( LB, &FD, "" );
		while ( FindNextFile( HF, &FD ) )InstallName( LB, &FD, "" );
    }
}

static void CreateRecList( ListBox* LB )
{
	WIN32_FIND_DATA FD;
	//ClearNames();
	HANDLE HF = FindFirstFile( "*.rec", &FD );
	if ( HF != INVALID_HANDLE_VALUE )
	{
		InstallName( LB, &FD, "" );
		while ( FindNextFile( HF, &FD ) )InstallName( LB, &FD, "" );
		FindClose( HF );
    }
	HF = FindFirstFile( "Autorecord\\*.rec", &FD );
	if ( HF != INVALID_HANDLE_VALUE )
	{
		InstallName( LB, &FD, "Autorecord\\" );
		while ( FindNextFile( HF, &FD ) )InstallName( LB, &FD, "Autorecord\\" );
		FindClose( HF );
    }
}

static void ProcessGLoadGame()
{
	LocalGP BTNS( "Interface\\G_Load_Game" );
	LocalGP SCROL( "Interface\\GScroll" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	int dx = ( RealLx - 470 ) >> 1;
	int dy = ( RealLy - 415 ) >> 1;
	DialogsSystem MMenu( dx, dy );
	GPS.ShowGP( dx, dy, BTNS.GPID, 4, 0 );
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;
	MMenu.addGPPicture( nullptr, 0, 0, BTNS.GPID, 5 );
	VScrollBar* VSB = MMenu.addNewGP_VScrollBar( nullptr, 436, 55, 276, 1, 0, SCROL.GPID, 0 );
	ListBox* LB = MMenu.addGP_ListBox( nullptr, 29, 61, 10, BTNS.GPID, 6, 26, &WhiteFont, &YellowFont, VSB );

	GP_Button* OkBtn = MMenu.addGP_Button( nullptr, 49, 353, BTNS.GPID, 0, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	GP_Button* CancelBtn = MMenu.addGP_Button( nullptr, 269, 353, BTNS.GPID, 2, 3 );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;

	SFLB_CreateGamesList( LB );
	ItemChoose = -1;
    LastKey = 0;
	int StartTime = GetTickCount();

	do
	{
		if ( GetTickCount() - StartTime > 10000 && NPlayers > 1 )
		{
			ItemChoose = mcmCancel;
			ShowOptMessage( "#LOOKOPT", 0 );
		}

		MMenu.MarkToDraw();

		ProcessMessages();

		StdKeys();

		if ( LB->CurItem == -1 )
		{
			OkBtn->Enabled = false;
			if ( ItemChoose == mcmOk )
			{
				ItemChoose = -1;
			}
		}
		else
		{
			OkBtn->Enabled = true;
		}

		MMenu.ProcessDialogs();
		MMenu.RefreshView();
	} while ( ItemChoose == -1 );

	if ( ItemChoose == mcmOk&&LB->CurItem != -1 )
	{
		strcpy( SaveFileName, fnames[LB->CurItem] );
		if ( NPlayers > 1 )
		{
			CmdLoadNetworkGame( MyNation, serials[LB->CurItem], SaveFileName );
		}
		else
		{
			SFLB_LoadGame( SaveFileName, 1 );
		}
	}

	ContinueGame = true;
	FreeNames();
	ClearMStack();
}

static void processMLoadGame()
{
	//SlowUnLoadPalette("1\\agew_1.pal");
	LocalGP BTNS( "Interface\\Load_Game" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	SQPicture MnPanel( "Interface\\Background_Load_Game.bmp" );
	DialogsSystem MMenu( menu_x_off, menu_y_off );
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;
	LoadFog( 2 );
	//int dx=((RealLx-GPS.GetGPWidth(BTNS.GPID,1))>>1)-85-125;
	//int dy=(RealLy-GPS.GetGPHeight(BTNS.GPID,1))>>1;
    /*Picture* PIC =*/ MMenu.addPicture( nullptr, 0, 0, &MnPanel, &MnPanel, &MnPanel );
	//MMenu.addGPPicture(nullptr,dx,dy,BTNS.GPID,0);
	//MMenu.addGPPicture(nullptr,dx,dy,BTNS.GPID,1);
	VScrollBar* VS = MMenu.addNewGP_VScrollBar( nullptr, 500, 166, 403, 1000, 0, BTNS.GPID, 7 );
	ListBox* LB = MMenu.addGP_ListBox( nullptr, 92, 172, 15, BTNS.GPID, 15, 26, &WhiteFont, &YellowFont, VS );
	LB->M_Over = 4;
	LB->Active = true;
	//LB->VS=VS;
	GP_Button* OkBtn = MMenu.addGP_Button( nullptr, 113, 594, BTNS.GPID, 0, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	GP_Button* CancelBtn = MMenu.addGP_Button( nullptr, 333, 594, BTNS.GPID, 2, 3 );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;


	SFLB_CreateGamesList( LB );
	CreateRecList( LB );
	ItemChoose = -1;
	int pp = 1;
    VS->ScrDy = 14;
	KeyPressed = 0;
	SetMyNation( 0 );

	do
	{
		MMenu.MarkToDraw();
		ProcessMessages();
		StdKeys();
		if ( LB->CurItem == -1 || !LB->NItems )
		{
			OkBtn->Enabled = false;
			if ( ItemChoose == mcmOk )
			{
				ItemChoose = -1;
			}
		}
		else
		{
			OkBtn->Enabled = true;
		}
		OkBtn->Visible = OkBtn->Enabled;
		MMenu.ProcessDialogs();
		MMenu.RefreshView();

		if ( pp )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pp = 0;
		}
	} while ( ItemChoose == -1 );

	if ( ItemChoose == mcmOk && LB->CurItem != -1 )
	{
		ShowLoading();
		PrepareToGame();
		if ( strstr( fnames[LB->CurItem], ".rec" ) || strstr( fnames[LB->CurItem], ".REC" ) )
		{
			RGAME.ReadStream( fnames[LB->CurItem] );
			if ( RGAME.Size )
			{
				PlayGameMode = 2;
				//ItemChoose=43;
				//RetryVideo=1;
				KeyPressed = 0;
				EditMapMode = 0;
				ItemChoose = mcmSingle;
			}
		}
		else
		{
			SFLB_LoadGame( fnames[LB->CurItem], 1 );
			HideFlags();
			ContinueGame = true;
			ItemChoose = mcmSingle;
		}
	}
	else
	{
		SlowUnLoadPalette( "2\\agew_1.pal" );
	}
	FreeNames();
}

bool ChangeNation;
bool MultiTvar;

bool EnterStr( const char * str, const char* Message )
{
	Lpressed = false;
	ItemChoose = -1;
	DialogsSystem DSY( smapx + ( nsmaplx << 4 ) - ( 288 >> 1 ), smapy + ( mul3( nsmaply ) << 2 ) - ( 288 >> 1 ) );
	//Picture* PIC1=DSY.addPicture(nullptr,0,0,&y288x128,&y288x128,&y288x128);
    /*TextButton* LoadBtn =*/ DSY.addTextButton( nullptr, 288 >> 1, 5, Message, &YellowFont, &YellowFont, &YellowFont, 1 );
	InputBox* IB = DSY.addInputBox( nullptr, 30, 45, str, 20, 224, 28, &WhiteFont, &WhiteFont );
	IB->Active = true;
	TextButton* OkBtn = DSY.addTextButton( nullptr, 55 + 164 / 2, 90, "OK", &WhiteFont, &YellowFont, &YellowFont, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	KeyPressed = false;
	do
	{
		DrawStdBar( DSY.BaseX, DSY.BaseY, DSY.BaseX + 288, DSY.BaseY + 128 );
		DSY.MarkToDraw();
		ProcessMessages();
		if ( KeyPressed )
		{
			if ( LastKey == 13 )
			{
				ItemChoose = mcmOk;
				KeyPressed = false;
            }
			if ( LastKey == 27 )
			{
				ItemChoose = mcmCancel;
				KeyPressed = false;
            }
        }
		DSY.ProcessDialogs();
		DSY.RefreshView();
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
		return true;
	}
	else return false;
}

static void MemScreenPart( int x, int y, int lx, int ly, uint8_t * Ptr )
{
	for ( int yy = 0; yy < ly; yy++ )
	{
		int ofs = x + ( y + yy )*ScrWidth;
		memcpy( Ptr, ( (byte*) ScreenPtr ) + ofs, lx );
		Ptr += lx;
    }
}

int EnterHi( int * val, int Type )
{

	LocalGP LONGBOX( "Interface\\LongBox0" );
	Lpressed = false;
	char passw[128];
	passw[0] = 0;
	ItemChoose = -1;
	sprintf( passw, "%d", *val );
	int LX = 400;
	int LY = 118;
	byte* PTR = new byte[( LX + 64 )*( LY + 200 )];
	MemScreenPart( ( ( RealLx - LX ) >> 1 ) - 32, ( ( RealLy - LY ) >> 1 ) - 16, LX + 64, LY + 200, PTR );
	DialogsSystem DSY( ( RealLx - LX ) >> 1, ( RealLy - LY ) >> 1 );
	DSY.addBPXView( nullptr, -32, -16, LX + 64, LY + 200, 1, 1, 1, PTR, nullptr );
	DSY.addBorder( 0, 0, LX, LY, 0, 1 );
	DSY.addGPPicture( nullptr, LX / 4 - 75, LY + 6, LONGBOX.GPID, 8 );
	DSY.addGPPicture( nullptr, LX / 4 - 75, LY + 6, LONGBOX.GPID, 11 );
	DSY.addGPPicture( nullptr, 3 * LX / 4 - 75, LY + 6, LONGBOX.GPID, 8 );
	DSY.addGPPicture( nullptr, 3 * LX / 4 - 75, LY + 6, LONGBOX.GPID, 11 );
	GP_TextButton* OkBtn = DSY.addGP_TextButton( nullptr, LX / 4 - 75 + 10, LY + 6 + 21, GetTextByID( "@OK" ), LONGBOX.GPID, 9, &WhiteFont, &YellowFont );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	GP_TextButton* CancelBtn = DSY.addGP_TextButton( nullptr, 3 * LX / 4 - 75 + 10, LY + 6 + 21, GetTextByID( "@CANCEL" ), LONGBOX.GPID, 9, &WhiteFont, &YellowFont );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;
	//Picture* PIC1=DSY.addPicture(nullptr,0,0,&y288x256,&y288x256,&y288x256);
	DSY.addTextButton( nullptr, 6, 6, GetTextByID( "$#HIHDR" ), &YellowFont, &YellowFont, &YellowFont, 0 );
    /*TextButton* LoadBtn =*/ DSY.addTextButton( nullptr, 10, 87, GetTextByID( "$#HEIGHT" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	InputBox* IB = DSY.addInputBox( nullptr, 120, 80, passw, 20, 224, 28, &WhiteFont, &WhiteFont );
	ColoredBar* CBR = DSY.addColoredBar( 120, 80, LX - 140, 23, 0x4c );
	CBR->Style = 1;
	DSY.addTextButton( nullptr, 10, 51, GetTextByID( "$#ACTION" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	ComboBox* CB = DSY.addGP_ComboBoxDLX( nullptr, 120, 44, LX - 140, LONGBOX.GPID, 0, 9, 9, &WhiteFont, &YellowFont, nullptr );
	CB->AddLine( "Cubic line" );
	CB->AddLine( "Quad. line" );
	CB->AddLine( "Softing" );
	CB->AddLine( "Road1" );
	CB->AddLine( "Road2" );
	CB->AddLine( "Road3" );
	CB->CurLine = Type - 1;
	IB->Active = true;

	KeyPressed = false;
	do
	{
		//DrawStdBar(DSY.BaseX,DSY.BaseY,DSY.BaseX+288,DSY.BaseY+256);
		DSY.MarkToDraw();
		ProcessMessages();
		if ( KeyPressed )
		{
			if ( LastKey == 13 )
			{
				ItemChoose = mcmOk;
				KeyPressed = false;
            }
			if ( LastKey == 27 )
			{
				ItemChoose = mcmCancel;
				KeyPressed = false;
            }
        }
		DSY.ProcessDialogs();
		DSY.RefreshView();
	} while ( ItemChoose == -1 );
	free( PTR );
	if ( ItemChoose == mcmOk )
	{
		int z = sscanf( passw, "%d", val );
		if ( z == 0 )return false;
		else return CB->CurLine + 1;
	}
	else return false;
}

static void EnterChat()
{
	if ( NPlayers > 1 )return;
	Lpressed = false;
	char passw[128];
	passw[0] = 0;
	ItemChoose = -1;
	DialogsSystem DSY( smapx + ( nsmaplx << 4 ) - ( 288 >> 1 ), smapy + ( mul3( nsmaply ) << 2 ) - ( 288 >> 1 ) );
	//Picture* PIC1=DSY.addPicture(nullptr,0,0,&y288x128,&y288x128,&y288x128);
    /*TextButton* LoadBtn =*/ DSY.addTextButton( nullptr, 288 >> 1, 5, "Enter password:", &FActive, &FPassive, &FDisable, 1 );
	InputBox* IB = DSY.addInputBox( nullptr, 30, 45, passw, 20, 224, 28, &WhiteFont, &WhiteFont );
	IB->Active = true;
	TextButton* OkBtn = DSY.addTextButton( nullptr, 55 + 164 / 2, 90, "OK", &WhiteFont, &YellowFont, &YellowFont, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	KeyPressed = false;
	do
	{
		DrawStdBar( DSY.BaseX, DSY.BaseY, DSY.BaseX + 288, DSY.BaseY + 128 );
		DSY.MarkToDraw();
		ProcessMessages();
		if ( KeyPressed&&LastKey == 13 )
		{
			ItemChoose = mcmOk;
			KeyPressed = false;
        }
		DSY.ProcessDialogs();
		DSY.RefreshView();
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
		if ( !strcmp( passw, "www" ) )
		{
			ChangeNation = 1;
			MultiTvar = 1;
			FogMode = 0;
		}
		else
			if ( !strcmp( passw, "VICTORY" ) )
			{
				if ( NPlayers < 2 )ShowVictory();
			}
			else
				if ( !strcmp( passw, "IZMENA" ) || !strcmp( passw, "izmena" ) )ChangeNation = !ChangeNation;
		if ( !strcmp( passw, "MULTITVAR" ) || !strcmp( passw, "multitvar" ) )MultiTvar = !MultiTvar;
		if ( !strcmp( passw, "SUPERVIZOR" ) || !strcmp( passw, "supervizor" ) )FogMode = !FogMode;
		if ( !strcmp( passw, "MONEY" ) || !strcmp( passw, "money" ) )
		{
			AddXRESRC( MyNation, 0, 150000 );
			AddXRESRC( MyNation, 1, 150000 );
			AddXRESRC( MyNation, 2, 150000 );
			AddXRESRC( MyNation, 3, 150000 );
			AddXRESRC( MyNation, 4, 150000 );
			AddXRESRC( MyNation, 5, 150000 );
		}
		else
			if ( !strcmp( passw, "AI" ) || !strcmp( passw, "ai" ) )
			{
				Nation* NT = &NATIONS[MyNation];
				if ( NT->AI_Enabled )NT->AI_Enabled = false;
				else
				{
					NT->AI_Enabled = true;
					NT->NMask = 0x7E;
					for ( int i = 0; i < MAXOBJECT; i++ )
					{
						OneObject* OB = Group[i];
						if ( OB&&OB->NNUM == MyNation&&OB->BrigadeID == 0xFFFF )
						{
							NT->CITY->RegisterNewUnit( OB );
							OB->NMask = 0x7E;
                        }
                    }

                }
			}
			else
			{
				char cc1[64];
				int n1, n2;
				int z = sscanf( passw, "%s%d%d", cc1, &n1, &n2 );
				if ( z == 3 && !strcmp( cc1, "change" ) )
				{
					if ( n1 >= 0 && n1 < 8 && n2 >= 0 && n2 <= 8 )SelChangeNation( n1, n2 );
                }
				if ( NPlayers == 1 )
				{
					char cc2[128];
					z = sscanf( passw, "%s%s", cc1, cc2 );
					if ( z == 2 && !strcmp( cc1, "changeunit" ) )
					{
                        for ( int i = 0; i < NATIONS->NMon; i++ )
						{
							if ( !strcmp( cc2, NATIONS->Mon[i]->MonsterID ) )
							{
								GeneralObject* GO = NATIONS->Mon[i];
								if ( !GO->newMons->Building )
								{
									//found!!!
									int N = NSL[MyNation];
									word* IDS = Selm[MyNation];
                                    for ( int j = 0; j < N; j++ )
									{
										word MID = IDS[j];
										if ( MID != 0xFFFF )
										{
											OneObject* OB = Group[MID];
											if ( OB && !OB->Sdoxlo )
											{
												OB->Ref.General = GO;
												OB->NIndex = i;
												OB->newMons = GO->newMons;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
    }
}

static void ProcessGSaveGame()
{
	if ( strstr( GameName, ".m3d" ) || strstr( GameName, ".M3D" ) )GameName[0] = 0;
	LocalGP BTNS( "Interface\\G_Save_Game" );
	LocalGP SCROL( "Interface\\GScroll" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	int dx = ( RealLx - 470 ) >> 1;
	int dy = ( RealLy - 415 ) >> 1;
	DialogsSystem MMenu( dx, dy );
	GPS.ShowGP( dx, dy, BTNS.GPID, 4, 0 );
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;
	//Picture* PIC=MMenu.addPicture(nullptr,0,0,&MnPanel,&MnPanel,&MnPanel);
	//MMenu.addGPPicture(nullptr,dx,dy,BTNS.GPID,0);
	MMenu.addGPPicture( nullptr, 0, 0, BTNS.GPID, 5 );
	VScrollBar* VSB = MMenu.addNewGP_VScrollBar( nullptr, 436, 55, 247, 1, 0, SCROL.GPID, 0 );
	ListBox* LB = MMenu.addGP_ListBox( nullptr, 29, 61, 9, BTNS.GPID, 6, 26, &WhiteFont, &YellowFont, VSB );

	//VScrollBar* VS=DSY.addVScrollBar(PIC1,LB->x-DSY.BaseX+LB->oneLx-1,1+LB->y-DSY.BaseY,1000,4,&sb_up_d,&sb_up_e,&sb_up_p,&sb_dn_d,&sb_dn_e,&sb_dn_p,&sb_vbare,&sb_vbar,&sb_btn);
	//LB->VS=VS;226x241
	GP_Button* OkBtn = MMenu.addGP_Button( nullptr, 49, 353, BTNS.GPID, 0, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	GP_Button* CancelBtn = MMenu.addGP_Button( nullptr, 269, 353, BTNS.GPID, 2, 3 );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;

	InputBox* IB = MMenu.addInputBox( nullptr, 114, 306, GameName, 35, 310, 20, &YellowFont, &WhiteFont );
	IB->Active = true;
	ItemChoose = -1;
	SFLB_CreateGamesList( LB );
	LB->CurItem = -1;
    int PrevListVal = LB->CurItem;
	KeyPressed = false;
	int StartTime = GetTickCount();
	do
	{
		if ( GetTickCount() - StartTime > 15000 && NPlayers > 1 )
		{
			ItemChoose = mcmCancel;
			ShowOptMessage( "#LOOKOPT", 0 );
			//SpecCmd=199;
			//IAmLeft();
        }
		if ( PrevListVal != LB->CurItem )
		{
			PrevListVal = LB->CurItem;
			strcpy( GameName, LB->GetItem( LB->CurItem )->Message );
			GameName[strlen( GameName ) - 4] = 0;
        }
		MMenu.MarkToDraw();
		ProcessMessages();
		StdKeys();
		//if(LB->CurItem==-1)OkBtn->Enabled=false;
		//else OkBtn->Enabled=true;
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
        // BoonXRay 20.08.2017
        //if ( strlen( IB->Str ) )
        if ( ! IB->String.empty() )
		{
			OkBtn->Enabled = true;
		}
		else
		{
			OkBtn->Enabled = false;
			if ( ItemChoose == mcmOk )ItemChoose = -1;
        }
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
        // BoonXRay 20.08.2017
        //CmdSaveNetworkGame( MyNation, GetRealTime(), IB->Str );
        CmdSaveNetworkGame( MyNation, GetRealTime(), IB->String.data() );
    }
	ContinueGame = true;
	FreeNames();
	ClearMStack();
}

//----------------SAVE MAP--------------//
static void NormalName( char* s )
{
	if ( !strstr( s, ".m3d" ) )
	{
		strcat( s, ".m3d" );
    }
}

static void PushScreen( uint8_t ** Ptr );
static void PopScreen( uint8_t * Ptr );
static bool ProcessSavingFile(const char *Mask, char* DestName, int Header, bool clear );

static void ProcessGSaveMap()
{
	byte* p;
	PushScreen( &p );
	if ( strstr( GameName, ".sav" ) || strstr( GameName, ".SAV" ) )GameName[0] = 0;
	if ( ProcessSavingFile( "*.m3d", GameName, 7, 0 ) )
	{
		NormalName( GameName );
		ResFile F = RReset( GameName );
		if ( F != INVALID_HANDLE_VALUE )
		{
			RClose( F );
			char ccc[128];
			strcpy( ccc, GameName );
			char* cc = strstr( ccc, ".m3d" );
			if ( cc )
			{
				strcpy( cc, ".bak" );
				DeleteFile( cc );
				rename( GameName, ccc );
            }
        }
		Save3DMap( GameName );
    }
	PopScreen( p );
}

//----------------LOAD Map--------------//
static bool ProcessLoadingFile( const char* Mask, char* DestName, int Header );

static void ProcessGLoadMap()
{
	byte* p;
	PushScreen( &p );
	if ( ProcessLoadingFile( "*.m3d", GameName, 4 ) )
	{
		Load3DMap( GameName );
		PeaceMode = 1;
    }
	PopScreen( p );	
}
//-----------------OPTIONS---------------//


//--------Choose units Dialog------------//
static void CreateUnitList( ListBox* LB, int Country )
{
	Nation* NT = &NATIONS[MyNation];
	LB->CurItem = 0;
	LB->FirstItem = 0;
	LB->FLItem = 0;
	LB->VS->SPos = 0;
	if ( !Country )
	{
		LB->AddStaticItem( "UNITS:    ", 0 );
		for ( int i = 0; i < NT->NMon; i++ )
		{
			NewMonster* NM = NT->Mon[i]->newMons;
			if ( !NM->Building )
				LB->AddItem( NM->Message, i );
        }
		LB->AddStaticItem( "BUILDINGS:   ", 0 );
		for ( int i = 0; i < NT->NMon; i++ )
		{
			NewMonster* NM = NT->Mon[i]->newMons;
			if ( NM->Building )
				LB->AddItem( NM->Message, i );
        }
		return;
    }
	Country--;
	int NUNITS = NT->NUnits[Country];
	if ( NUNITS )
	{
		for ( int j = 0; j < NUNITS; j++ )
		{
			int UIDS = NT->UnitsIDS[Country][j];
			if ( UIDS == 0xFFFF )
			{
				LB->AddStaticItem( NT->UnitNames[Country][j], 0xFFFF );
			}
			else
			{
				NewMonster* NM = NT->Mon[UIDS]->newMons;
				LB->AddItem( NM->Message, UIDS );
            }
        }
    }
}

static void ChooseUnit()
{
	ItemChoose = -1;
	SQPicture Pan( "Units.bpx" );
	//SQPicture Lstr("195x20.bpx");
	SQPicture scr_U( "scr_v_u.bpx" );
	SQPicture scr_D( "scr_v_d.bpx" );
	SQPicture scr_V( "scr_v.bpx" );
	SQPicture scr_Line( "scr_13.bpx" );
	RLCFont BtnF0( GETS( "@CUFONT0" ) );
	RLCFont BtnF1( GETS( "@CUFONT1" ) );
	RLCFont BtnL0( GETS( "@CULISTF0" ) );
	RLCFont BtnL1( GETS( "@CULISTF1" ) );
	RLCFont SelF( GETS( "@CULISTF2" ) );
	int mmlx = Pan.GetLx();
	int mmly = Pan.GetLy();
	DialogsSystem DSY( smapx + ( nsmaplx << 4 ) - ( mmlx >> 1 ), smapy + ( mul3( nsmaply ) << 2 ) - ( mmly >> 1 ) );
	Picture* PIC1 = DSY.addPicture( nullptr, 0, 0, &Pan, &Pan, &Pan );
	//TextButton* LoadBtn=DSY.addTextButton(PIC1,352>>1,30,"ВЫБОР ОБЪЕКТА",&FActive,&FPassive,&FDisable,1);
	ListBox* LBU = DSY.addListBox( PIC1, GETV( "CUUNITX" ), GETV( "CUUNITY" ), GETV( "CUNY" ), GETV( "CULX" ), GETV( "CULY" ), &BtnL1, &BtnL0, nullptr );
	ListBox* LBB = DSY.addListBox( PIC1, GETV( "CUBUILDX" ), GETV( "CUBUILDY" ), GETV( "CUNY" ), GETV( "CULX" ), GETV( "CULY" ), &BtnL1, &BtnL0, nullptr );
	LBB->DFont = &SelF;
	VScrollBar* VSU = DSY.addVScrollBar( PIC1, GETV( "CUUNITSX" ), GETV( "CUUNITSY" ), 1000, 4, &scr_U, &scr_U, &scr_U, &scr_D, &scr_D, &scr_D, &scr_Line, &scr_Line, &scr_V );
	VScrollBar* VSB = DSY.addVScrollBar( PIC1, GETV( "CUBUILDSX" ), GETV( "CUBUILDSY" ), 1000, 4, &scr_U, &scr_U, &scr_U, &scr_D, &scr_D, &scr_D, &scr_Line, &scr_Line, &scr_V );

	LBU->VS = VSU;
	LBB->VS = VSB;

	TextButton* OkBtn = DSY.addTextButton( PIC1, GETV( "CUOKX" ), GETV( "CUOKY" ), GETS( "@CUOKSTR" ), &BtnF0, &BtnF1, &BtnF1, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	TextButton* CancelBtn = DSY.addTextButton( PIC1, GETV( "CUNOX" ), GETV( "CUNOY" ), GETS( "@CUNOSTR" ), &BtnF0, &BtnF1, &BtnF1, 1 );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;
	LBU->AddItem( "ALL UNITS", 0 );
	for ( int j = 0; j < NNations; j++ )
	{
		LBU->AddItem( NatNames[j], j + 1 );
    }
	LBU->CurItem = 0;
	LBU->FLItem = 0;
	LBB->CurItem = 0;
	LBB->FLItem = 0;
	VSU->SPos = 0;
	VSB->SPos = 0;
	int CurNation = 0;
	CreateUnitList( LBB, CurNation );
	ItemChoose = -1;
	do
	{
		ProcessMessages();
		if ( !LBB->NItems )OkBtn->Enabled = false;
		else OkBtn->Enabled = true;
		DSY.MarkToDraw();
		DSY.ProcessDialogs();
		//DSY.MarkToDraw();
		DSY.RefreshView();
		if ( CurNation != LBU->CurItem )
		{
			LBB->ClearItems();
			CurNation = LBU->CurItem;
			CreateUnitList( LBB, CurNation );
        }
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
		if ( LBB->NItems )
		{
			Creator = ( LBB->GetItem( LBB->CurItem ) )->Param1;
		}
		else Creator = 4096 + 255;
		ContinueGame = true;
    }
	UnPress();
}

//------MAIN MENU IN MAP EDITOR----------//

static int ProcessE_GMainMenu();

//returns ProcessE_GMainMenu()
static int ProcessGMapEditMainMenu()
{
	return ProcessE_GMainMenu();
}

//------------------Selecting Texture dialog--------------
uint16_t TexList[128];
uint16_t NTextures;

uint16_t GetTexture()
{
	if ( !NTextures )
		return 0;

	return TexList[( rando()*NTextures ) >> 15];
}

static int SelectTexture()
{
	ContinueGame = false;
	GameMode = 2;
	NTextures = 0;
    SQPicture Pan( "textur.bpx" );
	//SQPicture Lstr("195x20.bpx");
	SQPicture scr_U( "scr_v_u.bpx" );
	SQPicture scr_D( "scr_v_d.bpx" );
	SQPicture scr_V( "scr_v.bpx" );
	SQPicture scr_Line( "scr_12.bpx" );
	RLCFont f24y( GETS( "@STFONT0" ) );
	RLCFont f24w( GETS( "@STFONT1" ) );
	int mmlx = Pan.GetLx();
	int mmly = Pan.GetLy();
	DialogsSystem GMM( smapx + ( nsmaplx << 4 ) - ( mmlx >> 1 ), smapy + ( mul3( nsmaply ) << 2 ) - ( mmly >> 1 ) );
	Picture* PIC1 = GMM.addPicture( nullptr, 0, 0, &Pan, &Pan, &Pan );
	TextButton* OkBtn =
		GMM.addTextButton( PIC1, GETV( "STOKX" ), GETV( "STOKY" ), GETS( "@STOKMESS" ), &f24w, &f24y, &f24y, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	TextButton* CancelBtn =
		GMM.addTextButton( PIC1, GETV( "STCANCELX" ), GETV( "STCANCELY" ), GETS( "@STNOMESS" ), &f24w, &f24y, &f24y, 1 );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;
	VScrollBar* VS = GMM.addVScrollBar( PIC1, GETV( "STSCRX" ), GETV( "STSCRY" ), ( MaxTex >> 2 ) - 4, 0, &scr_U, &scr_U, &scr_U, &scr_D, &scr_D, &scr_D, &scr_Line, &scr_Line, &scr_V );
	BPXView* BV = GMM.addBPXView( PIC1, GETV( "STWINX" ), GETV( "STWINY" ), 48, 48, 4, 4, MaxTex >> 2, tex1, VS );
	BV->DECLX = 16;
	ItemChoose = -1;
	do
	{
		ProcessMessages();
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		GMM.RefreshView();
	} while ( ItemChoose == -1 && BV->ChoosedPos == -1 );
	if ( ItemChoose == mcmCancel )
	{
		UnPress();
		return false;
	}
	if ( BV->ChoosedPos != -1 )
	{
		EditMedia = BV->ChoosedPos;
		HeightEditMode = false;
		MEditMode = true;
		Lpressed = false;
		TexList[0] = BV->ChoosedPos;
		NTextures = 1;
	}
	else
	{
		int ntx = 0;
		int nn = BV->Nx*BV->RealNy;
		for ( int i = 0; i < nn; i++ )
		{
			if ( BV->Choosed[i] )
			{
				for ( int j = 0; j < BV->Choosed[i]; j++ )
				{
					if ( ntx < 256 )
					{
						TexList[ntx] = i;
						ntx++;
					}
				}
			}
		}
		if ( ntx )
		{
			NTextures = ntx;
			HeightEditMode = false;
			MEditMode = true;
			Lpressed = false;
		}
	}
	GameMode = 0;
	UnPress();
	return ItemChoose;
}

//----------MAIN MENU IN GAME------------//
static int LastLookTime = 0;

static int ProcessGMainMenu()
{
	if ( NPlayers > 1 )
	{
		if ( LastLookTime && ( GetTickCount() - LastLookTime < 180000 ) )
		{
			int v = ( 180000 - GetTickCount() + LastLookTime ) / 1000;
			ShowOptMessage( "#F12LOCK", v );
			return mcmResume;
        }
    }
	LastLookTime = GetTickCount();
	LocalGP BTNS( "Interface\\GameMenu" );
	ContinueGame = false;
	GameMode = 2;
	//bool MIMOD=MiniMode;
	int mmmx = mapx;
	int mmmy = mapy;
    //int dx=smapx+(nsmaplx<<4)-(GPS.GetGPWidth(BTNS.GPID,1)>>1);
	//int dy=smapy+(mul3(nsmaply)<<2)-(GPS.GetGPHeight(BTNS.GPID,1)>>1);
	int dx = ( ( RealLx - 470 ) >> 1 ) - 14;
	int dy = ( RealLy - 415 ) >> 1;
	GPS.ShowGP( dx, dy, BTNS.GPID, 0, 0 );
	DialogsSystem GMM( dx, dy );

	//GMM.addGPPicture(nullptr,0,0,BTNS.GPID,0);
	GMM.addGPPicture( nullptr, 0, 0, BTNS.GPID, 1 );

	GP_Button* ResumeBtn = GMM.addGP_Button( nullptr, 43, 70, BTNS.GPID, 3, 2 );
	ResumeBtn->UserParam = mcmResume;
	ResumeBtn->OnUserClick = &MMItemChoose;
	GP_Button* SaveBtn = GMM.addGP_Button( nullptr, 43, 70 + 31 * 1, BTNS.GPID, 7, 6 );
	SaveBtn->UserParam = mcmSave;
	SaveBtn->OnUserClick = &MMItemChoose;
	if ( !( IsGameActive() || use_gsc_network_protocol ) )
	{
		GP_Button* LoadBtn = GMM.addGP_Button( nullptr, 43, 70 + 31 * 2, BTNS.GPID, 5, 4 );
		LoadBtn->UserParam = mcmLoad;
		LoadBtn->OnUserClick = &MMItemChoose;
	}
	else
	{
        /*GP_Button* LoadBtn =*/ GMM.addGP_Button( nullptr, 43, 70 + 31 * 2, BTNS.GPID, 4, 4 );
    }
	GP_Button* OptionsBtn = GMM.addGP_Button( nullptr, 43, 70 + 31 * 3, BTNS.GPID, 9, 8 );
	OptionsBtn->UserParam = mcmOptions;
	OptionsBtn->OnUserClick = &MMItemChoose;
	GP_Button* ExitBtn = GMM.addGP_Button( nullptr, 43, 70 + 31 * 4, BTNS.GPID, 11, 10 );
	ExitBtn->UserParam = mcmExit;
	ExitBtn->OnUserClick = &MMItemChoose;
	//TextButton* HelpBtn=GMM.addTextButton(PIC1,352>>1,30+160,"ПОМОЩЬ",&FActive,&FPassive,&FDisable,1);
	//HelpBtn->UserParam=mcmHelp;
	//HelpBtn->OnUserClick=&MMItemChoose;
stg:
	ItemChoose = -1;
	int StartTime = GetTickCount();
	do
	{
		ProcessMessages();
		GMM.ProcessDialogs();
		GMM.RefreshView();
		if ( GetTickCount() - StartTime > 15000 && NPlayers > 1 )
		{
			ShowOptMessage( "#LOOKOPT", 0 );
			ItemChoose = mcmResume;
			//SpecCmd=199;
			//IAmLeft();
        }
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmLoad )
	{
		ProcessGLoadGame();
		GMM.MarkToDraw();
		if ( !ContinueGame )goto stg;
    }
	if ( ItemChoose == mcmSave )
	{
		ProcessGSaveGame();
		GMM.MarkToDraw();
		if ( !ContinueGame )goto stg;
    }
	if ( ItemChoose == mcmOptions )
	{
		//ProcessOptions(smapx+(nsmaplx<<4)-(288>>1),smapy+(nsmaply<<4)-(288>>1));
		ProcessGameOptions();
		ContinueGame = true;
		ItemChoose = mcmResume;
    }
	if ( ItemChoose == mcmHelp )
	{
		ProcessHelp();
    }
	//if(MIMOD)SetMiniMode();
	mapx = mmmx;
	mapy = mmmy;
	GameMode = 0;
	ClearMStack();
	return ItemChoose;
}

static int ProcessE_GMainMenu()
{
	LocalGP BTNS( "Interface\\E_GameMenu" );
	ContinueGame = false;
	GameMode = 2;
	//bool MIMOD=MiniMode;
	int mmmx = mapx;
	int mmmy = mapy;
    //int dx=smapx+(nsmaplx<<4)-(GPS.GetGPWidth(BTNS.GPID,1)>>1);
	//int dy=smapy+(mul3(nsmaply)<<2)-(GPS.GetGPHeight(BTNS.GPID,1)>>1);
	int dx = ( ( RealLx - 470 ) >> 1 ) - 14;
	int dy = ( RealLy - 415 ) >> 1;
	GPS.ShowGP( dx, dy, BTNS.GPID, 0, 0 );
	DialogsSystem GMM( dx, dy );

	//GMM.addGPPicture(nullptr,0,0,BTNS.GPID,0);
	GMM.addGPPicture( nullptr, 0, 0, BTNS.GPID, 1 );

	GP_Button* ResumeBtn = GMM.addGP_Button( nullptr, 43, 70, BTNS.GPID, 3, 2 );
	ResumeBtn->UserParam = mcmResume;
	ResumeBtn->OnUserClick = &MMItemChoose;
	GP_Button* SaveBtn = GMM.addGP_Button( nullptr, 43, 70 + 31 * 1, BTNS.GPID, 7, 6 );
	SaveBtn->UserParam = mcmSave;
	SaveBtn->OnUserClick = &MMItemChoose;
	GP_Button* LoadBtn = GMM.addGP_Button( nullptr, 43, 70 + 31 * 2, BTNS.GPID, 5, 4 );
	LoadBtn->UserParam = mcmLoad;
	LoadBtn->OnUserClick = &MMItemChoose;
	GP_Button* OptionsBtn = GMM.addGP_Button( nullptr, 43, 70 + 31 * 3, BTNS.GPID, 9, 8 );
	OptionsBtn->UserParam = mcmOptions;
	OptionsBtn->OnUserClick = &MMItemChoose;
	GP_Button* ExitBtn = GMM.addGP_Button( nullptr, 43, 70 + 31 * 4, BTNS.GPID, 11, 10 );
	ExitBtn->UserParam = mcmExit;
	ExitBtn->OnUserClick = &MMItemChoose;
	//TextButton* HelpBtn=GMM.addTextButton(PIC1,352>>1,30+160,"ПОМОЩЬ",&FActive,&FPassive,&FDisable,1);
	//HelpBtn->UserParam=mcmHelp;
	//HelpBtn->OnUserClick=&MMItemChoose;

stg:
	ItemChoose = -1;
	do
	{
		ProcessMessages();
		GMM.ProcessDialogs();
		GMM.RefreshView();
	} while ( ItemChoose == -1 );

	if ( ItemChoose == mcmLoad )
	{
		ProcessGLoadMap();
		GMM.MarkToDraw();
		if ( !ContinueGame )
			goto stg;
	}

	if ( ItemChoose == mcmSave )
	{
		ProcessGSaveMap();
		GMM.MarkToDraw();
		if ( !ContinueGame )
			goto stg;
	}

	if ( ItemChoose == mcmOptions )
	{
		ProcessGameOptions();
		//ProcessOptions(smapx+(nsmaplx<<4)-(288>>1),smapy+(nsmaply<<3)-(288>>1));
		ContinueGame = true;
		ItemChoose = mcmResume;
	}

	mapx = mmmx;
	mapy = mmmy;
	GameMode = 0;
	ClearMStack();
	return ItemChoose;
}
//---------------------------------------//
static uint8_t ZoneOpt[128];
//  /-------------------------------------------------------\    '
//  | /--------\  2    3    4    5    6    7    8   /------\|       
//  | | 1      |                                    |      ||
//  | |        |  9    10   11   12  ...            |      ||
//  | |        |                                    |      ||
//  | \--------/  ...                               \------/|
//  \-------------------------------------------------------/
// ZoneOpt[i]:  0   -   empty
//              |1  -   used now
//              |2  -   was used
int Zone1X;
int Zone1Y;
int Zone1LX;
int Zone1LY;
static int ZonesNy;
static int ZonesNx;
int PanelY;
static int PropertyX;
static int PropertyY;
//---------
int BrigPnX;
int BrigPnY;
int BrigNx;
int BrigNy;

void DrawZones()
{
	if ( PlayGameMode == 1 )
	{
		return;
	}

	if ( PrpPanel.NIcons == 1 )
	{
		int Max1 = AblPanel.GetMaxX( 12 );
		int Max2 = UpgPanel.GetMaxX( 12 );
		if ( Max2 > Max1 )Max1 = Max2;
		int x0 = Zone1X + 6;
		int y0 = Zone1Y + PanelY;
		int x1 = AblX - 4 + 2;
		int y1 = y0 + ( IconLy * 3 ) + 5;
		if ( Max1 )
		{
			DrawLeftRect( x0, y0, x1, y1 );
			DrawRightRect( x1, y0, x1 + ( IconLx + 2 )*Max1 + 2, y1 );
		}
		else
		{
			DrawStdRect( x0, y0, x1, y1 );
		}
		PrpPanel.DrawIconSet( Zone1X + 6, Zone1Y + PanelY, 1, 1, 0 );
		AblPanel.DrawIconSet( AblX, AblY, AblNx, AblNy, 0 );
		UpgPanel.DrawIconSet( PrpX, PrpY, PrpNx, PrpNy, 0 );
		ShowTextDiscription();

		if ( ECOSHOW )
			EconomePostDraw();
	}
	else
	{
		int Max1 = AblPanel.GetMaxX( 12 );
		int Max2 = PrpPanel.GetMaxX( 12 );
		if ( Max2 > Max1 )Max1 = Max2;
		if ( Max1 )
		{
			int y0 = Zone1Y + PanelY;
			int x1 = AblX - 4 + 2;
			int y1 = y0 + ( IconLy * 3 ) + 5;
			DrawStdRect( x1, y0, x1 + ( IconLx + 2 )*Max1 + 2, y1 );
			PrpPanel.DrawIconSet( PrpX, PrpY, PrpNx, PrpNy, 0 );
			AblPanel.DrawIconSet( AblX, AblY, AblNx, AblNy, 0 );
		}
	}
	if ( !EditMapMode )
	{
		BrigPanel.DrawIconSet( BrigPnX, BrigPnY, BrigNx, BrigNy, 0 );
	}
	ECOSHOW = 0;
}

static bool NeedLoadGamePalette;
static constexpr bool FullScreenMode = 1;

static void GSSetup800()
{
	SetDarkPalette();

	CBar( 0, 0, SCRSizeX, SCRSizeY, 0 );

	memset( ZoneOpt, 0, 128 );

	int DMiniX, DMiniY;
	int PanelLy;

	BrigPnX = 6;
	BrigPnY = 16 + 15;
	BrigNx = 5;
	BrigNy = 7;
	InfAddY = 16;
	InfDY = 32 + InfAddY;
	InfY1 = 450 + InfAddY;
	InfY2 = 450 - 22 + InfAddY;
	if ( RealLy <= 600 )
	{
		InfAddY = 8;
		InfDY = 32 + InfAddY;
		InfY2 = 450 - 22 - 12 - 8;
		InfY1 = 450 - 12 - 8;
	}
	else
	{
		InfAddY = 16;
		InfDY = 32 + InfAddY;
		InfY1 = 450 + InfAddY;
		InfY2 = 450 - 22 + InfAddY;
		if ( RealLy < 768 )
		{
			InfY1 += RealLy - 768;
			InfY2 += RealLy - 768;
		}
	}

	CreateINFORM();

	if ( !CHKV( "Zone1X" ) )
	{
		Zone1X = 6;
		Zone1Y = 6;
		Zone1LX = 132;
		Zone1LY = 100;
		ZonesNy = 6;
		ZonesNx = 12;
        PanelLy = 144;
		PropertyX = 142;
		PropertyY = 6;
		IconLx = 40;
		IconLy = 40;
		DMiniX = 152;
		DMiniY = 7;
		AddIconLx = 2;
		AddIconLy = 2;
	}
	else
	{
		Zone1X = GETV( "Zone1X" );;
		Zone1Y = GETV( "Zone1Y" );
		Zone1LX = GETV( "Zone1LX" );
		Zone1LY = GETV( "Zone1LY" );
		ZonesNy = GETV( "ZonesNy" );
		ZonesNx = GETV( "ZonesNx" );
        PanelLy = GETV( "PanelLy" );
		PropertyX = GETV( "PropertyX" );
		PropertyY = GETV( "PropertyY" );
		IconLx = GETV( "IconLx" );
		IconLy = GETV( "IconLy" );
		DMiniX = GETV( "DMiniX" );
		DMiniY = GETV( "DMiniY" );
		AddIconLx = GETV( "AddIconLx" );
		AddIconLy = GETV( "AddIconLy" );
	}

    Shifter = 5;

	int scly = RealLy;

	if ( !FullScreenMode )
	{
		scly -= PanelLy;
	}

	int CLY = mul3( 32 ) >> 2;
	int Y1 = RealLy - PanelLy;
	smaply = div( scly, CLY ).quot;
	smaplx = RealLx >> 5;
	smapx = 0;
	smapy = RealLy - smaply * 16;
	minix = RealLx - DMiniX;
	miniy = RealLy - PanelLy + DMiniY;
	nsmaplx = smaplx;
	nsmaply = smaply;
	HintX = smapx + 4;
	HintY = Y1 - 40;
    Shifter = 5;
    PrpX = PropertyX;
	PrpY = Y1 + PropertyY;
	PrpNx = ZonesNx >> 1;
	PrpNy = ZonesNy;

	AblX = PrpX;
	AblY = Y1 + PropertyY;
	AblNy = ZonesNy;
	AblNx = PrpNx = ZonesNx;

	GSYS.~DialogsSystem();

    /*GVPort =*/ GSYS.addViewPort( smapx, smapy, smaplx << 5, smaply << 4 );
	PanelY = RealLy - PanelLy;

	NeedLoadGamePalette = true;

	if ( mapx + smaplx > msx + 1 )
	{
		mapx = msx - smaplx + 1;
	}
	if ( mapy + smaply > msy + 1 )
	{
		mapy = msy - smaply + 1;
	}
}

//Copy elements to screen buffer (units, minimap, ui)
//Handle mouse events
//Load palettes
static void DrawAllScreen()
{
	//Draw a lot of stuff on screen
	GFieldShow();

	//Copy minimap into screen buffer
	GMiniShow();

	//Draw units?
	ShowProp();

	//Draw upgrades / unit command buttons?
	ShowAbility();

	memset( ZoneOpt, 0, 128 );

	//Draw UI elements?
	DrawZones();

	CopyToScreen( 0, 0, RealLx, RSCRSizeY );

	//Check SpecCmd, read from MouseStack
	GlobalHandleMouse( true );

	//Mouse bug fix?
	MFix();

	if ( NeedLoadGamePalette )
	{
		if ( IgnoreSlow )
		{
			LoadPalette( "0\\agew_1.pal" );
			LoadFog( 0 );
		}
		else
		{
			SlowLoadPalette( "0\\agew_1.pal" );
			LoadFog( 0 );
		}
		CreateMiniMap();
	}
	NeedLoadGamePalette = false;
}

static void FastScreenProcess()
{
	GFieldShow();

	ProcessMessages();

	if ( SHOWSLIDE )
	{
		GMiniShow();

		ShowProp();

		ShowAbility();

		DrawZones();

		GSYS.ProcessDialogs();
	}

	GlobalHandleMouse( true );
	MFix();

	if ( SHOWSLIDE )
	{
		int time0 = GetRealTime();

		GSYS.RefreshView();
		time8 = GetRealTime() - time0;
	}

	if ( NeedLoadGamePalette )
	{
		if ( IgnoreSlow )
		{
			LoadPalette( "0\\agew_1.pal" );
			LoadFog( 0 );
		}
		else
		{
			SlowLoadPalette( "0\\agew_1.pal" );
			LoadFog( 0 );
		}
		CreateMiniMap();
	}
	NeedLoadGamePalette = false;
}

static int StartAboutTime = 0;
static int NAboutLn = -1;

void ShowAbout()
{
	TempWindow TM;
	PushWindow( &TM );
	IntersectWindows( 0, smapy, smaplx * 32, smaply * 16 );

	int y0 = RealLy - ( GetTickCount() - StartAboutTime ) / 50;
	char pp[100];

	if ( NAboutLn == -1 )
	{
		for ( int i = 0; i < 200; i++ )
		{
			sprintf( pp, "ABLINE%d", i );
			if ( pp == GetTextByID( pp ) )
			{
				NAboutLn = i;
				i = 300;
			}
		}
	}

	if ( NAboutLn == -1 )
	{
		return;
	}

	int LDis = 35;
	for ( int i = 0; i < NAboutLn; i++ )
	{
		sprintf( pp, "ABLINE%d", i );
        const char* tx = GetTextByID( pp );
		if ( y0 > -LDis )
		{
			ShowString( ( 2 + ( RealLx - GetRLCStrWidth( tx, &BigYellowFont ) ) ) >> 1, 2 + y0, tx, &BigBlackFont );
			ShowString( ( RealLx - GetRLCStrWidth( tx, &BigYellowFont ) ) >> 1, y0, tx, &BigYellowFont );
		}
		y0 += LDis;
	}
	PopWindow( &TM );
}

bool IngameYesNoDialog( const char* dialog_text )
{
	ItemChoose = -1;
	int dialog_width = 600;
	int dialog_height = 200;

	DialogsSystem GMM( ( RealLx - dialog_width ) / 2, 30 );

	GMM.addTextButton( nullptr, dialog_width / 2, 50, dialog_text,
		&BigWhiteFont, &BigWhiteFont, &BigWhiteFont, 1 );

	TextButton* YesBtn = GMM.addTextButton( nullptr, dialog_width / 4, 120,
		GetTextByID( "@MSYES" ), &BigWhiteFont, &BigYellowFont, &BigYellowFont, 1 );
	YesBtn->UserParam = mcmOk;
	YesBtn->OnUserClick = &MMItemChoose;

	TextButton* NoBtn = GMM.addTextButton( nullptr, dialog_width * 3 / 4, 120,
		GetTextByID( "@MSNO" ), &BigWhiteFont, &BigYellowFont, &BigYellowFont, 1 );
	NoBtn->UserParam = mcmCancel;
	NoBtn->OnUserClick = &MMItemChoose;

	do
	{
		DrawStdBar1( GMM.BaseX, GMM.BaseY, GMM.BaseX + dialog_width, GMM.BaseY + dialog_height );

		ProcessMessages();

		if ( KeyPressed )
		{
			if ( LastKey == 13 )
			{
				ItemChoose = mcmOk;
			}
			else
			{
				if ( LastKey == 27 )
				{
					ItemChoose = mcmCancel;
				}
			}
			KeyPressed = false;
		}
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		GMM.RefreshView();
	} while ( ItemChoose == -1 );

	UnPress();

	if ( ItemChoose == mcmOk )
	{
		ItemChoose = -1;
		return true;
	}
	else
	{
		ItemChoose = -1;
		return false;
	}
}

//Main game loop
void PlayGame()
{
	InGame = true;

	if ( window_mode )
	{//Explicit call in case if game starts at 1024x768
		ResizeAndCenterWindow();
	}

	GSSetup800();

	LoadFog( 0 );

	if ( exRealLx != RealLx )
	{//Use last used game resoultion (loaded from settings)
		SetGameDisplayModeAnyway( exRealLx, exRealLy );
	}

	//Does what the name says
	DrawAllScreen();

	GameNeedToDraw = false;
	GameExit = false;
	MakeMenu = false;

	if ( PlayGameMode )
	{
		StartAboutTime = GetTickCount();
	}

StartPlay://IMPORTANT: Main game loop
	do
	{
		if ( RUNMAPEDITOR || RUNUSERMISSION )
		{
			GameExit = true;
		}

		time1 = GetRealTime();

		ProcessMessages();

		if ( bActive )
		{
			GameMode = 0;
			PreDrawGameProcess();

			ProcessMessages();

			if ( GameNeedToDraw )
			{
				DrawAllScreen();
				GameNeedToDraw = false;
			}
			else
			{
				FastScreenProcess();
			}

			ProcessMessages();

			time1 = GetRealTime();

			PostDrawGameProcess();

			if ( MakeMenu )
			{
				switch ( MenuType )
				{
				case 1:
					if ( mcmExit == ProcessGMainMenu() && IngameYesNoDialog( GetTextByID( "CONFIRM" ) ) )
					{
						if ( NPlayers > 1 && !NATIONS[MyNation].VictState )
						{
							LockPause = 1;
							WaitState = 1;
							if ( SCENINF.hLib && !SCENINF.StandartVictory )
							{
								SCENINF.LooseGame = 1;
							}
							if ( NATIONS[MyNation].VictState != 2 )
							{
								CmdEndGame( MyNation, 1, 101 );
							}
							int t0 = GetRealTime();
							do
							{
								ProcessMessages();
							} while ( GetRealTime() - t0 < 500 );
						}
						GameExit = true;
						ShutdownMultiplayer( 0 );
						CloseMPL();
					}
					break;
				case 2:
					ChooseUnit();
					break;
				case 3:
					EnterChat();
					break;
				case 4:
					ProcessHelp();
					break;
				case 5:
					SelectTexture();
					break;
				}
				MakeMenu = false;
			}
			else
			{
				if ( GameExit )
				{
					IAmLeft();
					LOOSEANDEXITFAST();
					CloseMPL();
					ShutdownMultiplayer( 0 );
				}
			}
		}
		else
		{
			GameNeedToDraw = true;
		}

	resgame:;

	} while ( !GameExit );

    PlayGameMode = 0;

	if ( RecordMode )
	{
		RGAME.Save();
		RecordMode = 0;
	}

	EndGSC_Reporting();
	use_gsc_network_protocol = false;

	if ( RetryVideo && !( RUNMAPEDITOR || RUNUSERMISSION ) )
	{
		GFILE* f = Gopen( "Demo\\demo.run", "r" );
		if ( f )
		{
			int N;
			int z = Gscanf( f, "%d", &N );
			if ( z == 1 )
			{
				int M = GetRndVid( N );
				char NAME[64];
				for ( int i = 0; i < M; i++ )
				{
					Gscanf( f, "%s", NAME );
				}
				Gscanf( f, "%s", NAME );
				RGAME.ReadStream( NAME );
				if ( RGAME.Size )
				{
					PlayGameMode = 1;
					RecordMode = 0;
					RetryVideo = 1;
					KeyPressed = 0;
				}
			}
			Gclose( f );
			GameExit = 0;
			goto StartPlay;
		}
		else
		{
			RetryVideo = 0;
		}
	}

	IgnoreSlow = true;

	int ExRX = RealLx;
	int ExRY = RealLy;

	if ( ShowStat )
	{
		if ( !ShowStatistics() )
		{
			GameExit = false;
			WaitState = 1;
			if ( RealLx != ExRX || RealLy != ExRY )
			{
				SetGameDisplayModeAnyway( ExRX, ExRY );
			}
			if ( ItemChoose == 131 )
			{
				REPLAY();
			}
			goto resgame;
		}
		SlowUnLoadPalette( "0\\agew_1.pal" );
	}

	ClearScreen();
	IgnoreSlow = false;

	//Reset resolution for main menu
	if ( window_mode )
	{//Dont't do in fullscreen to prevent menu stretching
		if ( RealLx != 1024 || RealLy != 768 )
		{
			SetGameDisplayModeAnyway( 1024, 768 );
		}
	}
	else
	{//Always go for native screen resolution when showing menu in fullscreen
		SetGameDisplayModeAnyway( screen_width, screen_height );
	}
}

static void DrawAllEditScreen()
{
	GFieldShow();
	//Pan1->OnDraw(Pan1);
	GMiniShow();
	//Pan2->OnDraw(Pan2);
	ShowProp();
	//Pan3->OnDraw(Pan3);
	ShowAbility();
	//Pan4->OnDraw(Pan4);
	//if(Pan5)Pan5->OnDraw(Pan5);

	CopyToScreen( 0, 0, RealLx, RSCRSizeY );
}

//Main map editor function
static void EditGame()
{
	InEditor = true;

	if ( window_mode )
	{//Explicit call in case if game starts at 1024x768
		ResizeAndCenterWindow();
	}

	LoadFog( 0 );
	GSSetup800();

	if ( exRealLx != RealLx )
	{
		SetGameDisplayModeAnyway( exRealLx, exRealLy );
	}
	DrawAllEditScreen();
	GameNeedToDraw = false;
	GameExit = false;
	MakeMenu = false;
	do
	{
		ProcessMessages();
		if ( bActive )
		{
			if ( RUNUSERMISSION )
			{
				GameExit = true;
			}
			if ( RUNMAPEDITOR )
			{
				Load3DMap( USERMISSPATH );
				strcpy( GameName, USERMISSPATH );
				RUNMAPEDITOR = 0;
			}
			//SERROR2();
			PreDrawGameProcess();
			//SERROR1();

			ProcessMessages();
			//if(GameNeedToDraw){
			//	DrawAllScreen();
			//	GameNeedToDraw=false;
			//}else 

			FastScreenProcess();
			//DrawAllScreen();
			//SERROR();

			ProcessMessages();

			PostDrawGameProcess();
			if ( MakeMenu )
			{
				switch ( MenuType )
				{
				case 1:
					if ( ProcessGMapEditMainMenu() == mcmExit )
					{
						GameExit = true;
					}
					break;
				case 2:
					ChooseUnit();
					break;
				case 3:
					EnterChat();
					break;
				case 31:
					ProcessResEdit();
					break;
				case 5:
					SelectTexture();
					break;
				case 6:
					EditMiniMap();
					break;
				case 7:
					ChooseInterior( 0 );
					break;
				case 8:
					ChooseInterior( 1 );
					break;
				case 9:
					ChooseInterior( 2 );
					break;
				}
				MakeMenu = false;
			}
		}
		else
		{
			GameNeedToDraw = true;
		}
	} while ( !GameExit );

	IgnoreSlow = true;

	//Reset resolution for main menu
	if ( window_mode )
	{//Dont't do in fullscreen to prevent menu stretching
		if ( RealLx != 1024 || RealLy != 768 )
		{
			SetGameDisplayModeAnyway( 1024, 768 );
		}
	}
	else
	{//Always go for native screen resolution when showing menu in fullscreen
		SetGameDisplayModeAnyway( screen_width, screen_height );
	}

	IgnoreSlow = false;
}

static int prevVid = -1;
static int prevVid1 = -1;
static int prevVid2 = -1;
static int prevVid3 = -1;

static int GetRndVid( int N )
{
	int dt = GetTickCount() & 1023;
	for ( int i = 0; i < dt; i++ )
	{
		rand();
	}
	int cv = -1;
	if ( N > 4 )
	{
		int na = 0;
		do
		{
			int n = rand() % N;
			if ( n != prevVid&&n != prevVid1&&n != prevVid2&&n != prevVid3 )
			{
				cv = n;
			}
			na++;
		} while ( cv == -1 && na < 100 );
	}
	if ( cv == -1 && N > 3 )
	{
		int na = 0;
		do
		{
			int n = rand() % N;
			if ( n != prevVid&&n != prevVid1&&n != prevVid2 )
			{
				cv = n;
			}
			na++;
		} while ( cv == -1 && na < 100 );
	}
	if ( cv == -1 && N > 2 )
	{
		int na = 0;
		do
		{
			int n = rand() % N;
			if ( n != prevVid&&n != prevVid1 )
			{
				cv = n;
			}
			na++;
		} while ( cv == -1 && na < 100 );
	}
	if ( cv == -1 && N > 1 )
	{
		int na = 0;
		do
		{
			int n = rand() % N;
			if ( n != prevVid )
			{
				cv = n;
			}
			na++;
		} while ( cv == -1 && na < 100 );
	}
	if ( cv == -1 )
	{
		cv = 0;
	}
	prevVid3 = prevVid2;
	prevVid2 = prevVid1;
	prevVid1 = prevVid;
	prevVid = cv;
	return cv;
}

//Main game loop function
void AllGame()
{
	int menuChoice;
	do
	{
		menuChoice = processMainMenu();
		if ( menuChoice == mcmSingle )
		{
			if ( EditMapMode )
			{
				//Map editor loop
				EditGame();
			}
			else
			{
				//Game loop
				PlayGame();
			}
			//Zero variables and pointers
			UnLoading();
		}
	} while ( mcmExit != menuChoice );
}

static void PrepareEditMedia( uint8_t myid )
{
	FreeArrays();
	ADDSH = 1;
	SetupArrays();
	rpos = 0;
    UnLoading();
	NewMap( 240 << ADDSH, 240 << ADDSH );
	SetMyNation( 0 );

	for ( int i = 0; i < 8; i++ )
	{
		LoadAllNations( i );

		SetMyNation( myid );
	}

	ItemChoose = mcmSingle;
	InitExplosions();
	PrepareToEdit();
	ItemChoose = mcmSingle;
    EraseAreas();
	rando();

	CreateTotalLocking();

	ResearchIslands();
}

void RedrawGameBackground()
{
	if ( GameMode == 1 )
	{//main menu
		FlipPages();
	}
	else
	{
		DrawAllScreen();
	}
}

static void ProcessHelp()
{
	if ( !SCENINF.hLib )return;
	ResFile F = RReset( "miss.txt" );
	int sz = 0;
	if ( F != INVALID_HANDLE_VALUE )
	{
		sz = RFileSize( F );
		RClose( F );
    }
	if ( !sz )return;
	int MPPLX = GETV( "MPPLX" );
	int MPPLY = GETV( "MPPLY" );
	int MPLDX = GETV( "MPLDX" );
	int MPRDX = GETV( "MPRDX" );
	int MPDDY = GETV( "MPDDY" );
	int MPUDY = GETV( "MPDDY" );
	MPUDY += 40;
	//char* BmpName=GETS("@MISSPANEL");
	DialogsSystem GMM( smapx + ( nsmaplx << 4 ) - ( MPPLX >> 1 ), smapy + ( mul3( nsmaply ) << 2 ) - ( MPPLY >> 1 ) );
	//SQPicture SP(BmpName);

	LocalGP FONTB( "vinBig" );
	RLCFont fntb( FONTB.GPID );
	RLCFont fntrb( FONTB.GPID );
	RLCFont fntw( FONTB.GPID );
	fntrb.SetRedColor();
	//LoadOptionalTable(5,"2\\orange.grd");
	//fntw.SetColorTable(5);

	SQPicture SPU;
	SPU.PicPtr = nullptr;
	//LocalGP LGP;
	//LGP.Load(BmpName);
	//GPS.ShowGP(GMM.BaseX,GMM.BaseY,LGP.GPID,0,0);
	//GMM.addGPPicture(nullptr,0,0,LGP.GPID,1);
    int LY = 0;
	TextViewer* TV;
	GMM.addTextButton( nullptr, ( MPPLX >> 1 ) + 1, -20, GetTextByID( "MISSOBJ" ), &BigBlackFont, &BigBlackFont, &BigBlackFont, 1 );
	GMM.addTextButton( nullptr, MPPLX >> 1, -21, GetTextByID( "MISSOBJ" ), &BigYellowFont, &BigYellowFont, &BigYellowFont, 1 );
	if ( SCENINF.hLib )
	{
		TV = GMM.addTextViewer( nullptr, MPLDX, MPUDY + LY, MPPLX - MPLDX - MPRDX, MPPLY - MPDDY - MPUDY - LY - 40, "miss.txt", &BlackFont );
	}
	else
	{
		TV = GMM.addTextViewer( nullptr, MPLDX, MPUDY + LY, MPPLX - MPLDX - MPRDX, MPPLY - MPDDY - MPUDY - LY - 40, "help.txt", &BlackFont );
    }
	TV->SymSize += 3;
	TV->PageSize = div( TV->y1 - TV->y + 1, TV->SymSize ).quot;

	TextButton* YesBtn = GMM.addTextButton( nullptr, MPPLX >> 1, MPPLY - 60, GetTextByID( "@MSOK" ), &fntrb, &fntb, &fntb, 1 );
	YesBtn->UserParam = mcmOk;
	YesBtn->OnUserClick = &MMItemChoose;
	ItemChoose = -1;
	DrawPaperPanelShadow( GMM.BaseX, GMM.BaseY, GMM.BaseX + MPPLX, GMM.BaseY + MPPLY );
	do
	{
		DrawPaperPanel( GMM.BaseX, GMM.BaseY, GMM.BaseX + MPPLX, GMM.BaseY + MPPLY );
		ProcessMessages();
		if ( KeyPressed && ( LastKey == 13 || LastKey == 27 ) )
		{
			KeyPressed = false;
			ItemChoose = mcmOk;
        }
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		GMM.RefreshView();
	} while ( ItemChoose == -1 );
	ContinueGame = true;
	UnPress();
}

int ProcessComplexQuestion( int Nx, char* Bmp1, byte or1, char* Text1, char* Bmp2, byte or2, char* Text2, const char* Quest )
{
	int Lx = Nx << 6;
	DialogsSystem GMM( ( RealLx - Lx ) >> 1, 80 );
	SQPicture SPU;
	SPU.PicPtr = nullptr;
	LocalGP CPIC1;
	LocalGP CPIC2;
	int pid1 = -1;
	int pid2 = -1;
	char cc1[100];
	if ( or1 > 2 )or1 = 0;
	if ( or2 > 2 )or2 = 0;
	if ( strcmp( Bmp1, "NONE" ) )
	{
		sscanf( Bmp1, "%s%d", cc1, &pid1 );
		if ( pid1 != -1 )
		{
			CPIC1.Load( cc1 );
        }
    }
	if ( strcmp( Bmp2, "NONE" ) )
	{
		sscanf( Bmp2, "%s%d", cc1, &pid2 );
		if ( pid2 != -1 )
		{
			CPIC2.Load( cc1 );
        }
    }

	ResFile RF = RRewrite( "tmp1.txt" );
	RBlockWrite( RF, Text1, strlen( Text1 ) );
	RClose( RF );
	RF = RRewrite( "tmp2.txt" );
	RBlockWrite( RF, Text2, strlen( Text2 ) );
	RClose( RF );

	LocalGP LGP;
	LocalGP FONT( "rom11" );
	//LocalGP FONT1("rom11r");
	LocalGP FONTB( "vinBig" );
	LocalGP FONTRB( "vinBig" );

	RLCFont fntb( FONTB.GPID );
	RLCFont fntrb( FONTRB.GPID );
	RLCFont f16x16w( FONT.GPID );
	RLCFont f16x16g( FONT.GPID );
	f16x16w.SetRedColor();
	fntrb.SetRedColor();

	int y0 = 16;
	int LX1 = 0;
	int LY1 = 0;
	int x0 = 16;
	int plx = 0;
	if ( pid1 != -1 )
	{
		LX1 = GPS.GetGPWidth( CPIC1.GPID, pid1 );
		LY1 = GPS.GetGPHeight( CPIC1.GPID, pid1 ) + 16;
		if ( or1 == 1 )
		{
			GMM.addGPPicture( nullptr, x0, y0, CPIC1.GPID, pid1 );
			x0 += LX1;
			plx = LX1;
		}
		else if ( or1 == 2 )
		{
			GMM.addGPPicture( nullptr, Lx - 16 - LX1, y0, CPIC1.GPID, pid1 );
			plx = LX1;
		}
		else
		{
			GMM.addGPPicture( nullptr, ( Lx - LX1 ) >> 1, y0, CPIC1.GPID, pid1 );
			y0 += LY1 + 8;
        }
    }
	TextViewer* TV1;
	TextViewer* TV2;
	TV1 = GMM.addTextViewer( nullptr, x0, y0, Lx - 32 - plx, 1000, "tmp1.txt", &f16x16w );
	TV1->Enabled = 0;
	int dy = TV1->NLines*TV1->SymSize;
	if ( or1 )y0 += dy > LY1 ? dy : LY1;
	else y0 += dy;
	int LX2 = 0;
	int LY2 = 0;
	x0 = 16;
	plx = 0;
	if ( pid2 != -1 )
	{
		LX2 = GPS.GetGPWidth( CPIC2.GPID, pid2 );
		LY2 = GPS.GetGPHeight( CPIC2.GPID, pid2 ) + 16;
		if ( or2 == 1 )
		{
			GMM.addGPPicture( nullptr, x0, y0, CPIC2.GPID, pid2 );
			x0 += LX2;
			plx = LX2;
		}
		else if ( or2 == 2 )
		{
			GMM.addGPPicture( nullptr, Lx - 16 - LX2, y0, CPIC2.GPID, pid2 );
			plx = LX2;
		}
		else
		{
			GMM.addGPPicture( nullptr, ( Lx - LX1 ) >> 1, y0, CPIC2.GPID, pid2 );
			y0 += LY2 + 8;
        }
	}
	else y0 += 18;;
	TV2 = GMM.addTextViewer( nullptr, x0, y0, Lx - 32 - plx, 1000, "tmp2.txt", &f16x16g );
	TV2->Enabled = 0;
	dy = TV2->NLines*TV2->SymSize;
	if ( or1 )y0 += dy > LY2 ? dy : LY2;
	else y0 += dy;
	int Ny = ( ( y0 + 38 ) >> 6 ) + 1;
	int NTB = 1;
	int LL = strlen( Quest );
	for ( int j = 0; j < LL; j++ )if ( Quest[j] == '|' )NTB++;
	x0 = Lx / 2 / NTB;
	int DX = Lx / NTB;
	char cc[128];
	int pos;
	for ( int q = 0; q < NTB; q++ )
	{
		pos = 0;
		char c;
		do
		{
			c = Quest[0];
			if ( c != '|' )
			{
				cc[pos] = c;
				pos++;
				cc[pos] = 0;
            }
			Quest++;
		} while ( c != 0 && c != '|' );
		TextButton* TB = GMM.addTextButton( nullptr, x0 + q*DX, Ny * 64 - 33, GetTextByID( cc ), &fntrb, &fntb, &fntb, 1 );
		TB->OnUserClick = &MMItemChoose;
		TB->UserParam = q;
		if ( PlayGameMode )
		{
			TB->Enabled = 0;
			if ( q == CurrentAnswer )TB->PassiveFont = &fntrb;
        }
    }
	int DDY = ( RealLy - 64 * Ny ) / 2 - 80;
	GMM.BaseY += DDY;
	for ( int p = 0; p < 512; p++ )
	{
		if ( GMM.DSS[p] && GMM.DSS[p]->Visible )
		{
			GMM.DSS[p]->y += DDY;
			GMM.DSS[p]->y1 += DDY;
        }
    }
	ItemChoose = -1;
	DrawPaperPanelShadow( ( RealLx - Lx ) >> 1, 80 + DDY - 32, ( ( RealLx - Lx ) >> 1 ) + Lx, 80 + DDY + Ny * 64 + 20 );
	int T0 = GetTickCount();
	do
	{
		ProcessMessages();
		//if(KeyPressed&&(LastKey==13||LastKey==27)){
		//	KeyPressed=false;
		//	ItemChoose=mcmOk;
        //}
		DrawPaperPanel( ( RealLx - Lx ) >> 1, 80 + DDY - 32, ( ( RealLx - Lx ) >> 1 ) + Lx, 80 + DDY + Ny * 64 + 20 );
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		GMM.RefreshView();
		if ( PlayGameMode&&GetTickCount() - T0 > 5000 )ItemChoose = CurrentAnswer;
	} while ( ItemChoose == -1 );
	ContinueGame = true;
	UnPress();
	return ItemChoose;
}

int ProcessMultilineQuestion(int Nx, char* Bmp1, uint8_t or1, char* Text1, char* Quest )
{
	int Lx = Nx << 6;
	DialogsSystem GMM( ( RealLx - Lx ) >> 1, 80 );
	SQPicture SPU;
	SPU.PicPtr = nullptr;
	LocalGP CPIC1;
	int pid1 = -1;
    char cc1[100];
	if ( or1 > 2 )or1 = 0;
	if ( strcmp( Bmp1, "NONE" ) )
	{
		sscanf( Bmp1, "%s%d", cc1, &pid1 );
		if ( pid1 != -1 )
		{
			CPIC1.Load( cc1 );
        }
    }

	ResFile RF = RRewrite( "tmp1.txt" );
	RBlockWrite( RF, Text1, strlen( Text1 ) );
	RClose( RF );

	LocalGP LGP;
	LocalGP FONT( "rom11" );
	LocalGP FONT1( "rom11r" );
	LocalGP FONTB( "vinBig" );
	LocalGP FONTRB( "vinBig" );

	RLCFont fntb( FONTB.GPID );
	RLCFont fntrb( FONTRB.GPID );
	RLCFont f16x16w( FONT.GPID );
	RLCFont f16x16g( FONT.GPID );
	f16x16w.SetRedColor();
	fntrb.SetRedColor();

	int y0 = 16;
	int LX1 = 0;
	int LY1 = 0;
	int x0 = 16;
	int plx = 0;
	if ( pid1 != -1 )
	{
		LX1 = GPS.GetGPWidth( CPIC1.GPID, pid1 );
		LY1 = GPS.GetGPHeight( CPIC1.GPID, pid1 ) + 16;
		if ( or1 == 1 )
		{
			GMM.addGPPicture( nullptr, x0, y0, CPIC1.GPID, pid1 );
			x0 += LX1;
			plx = LX1;
		}
		else if ( or1 == 2 )
		{
			GMM.addGPPicture( nullptr, Lx - 16 - LX1, y0, CPIC1.GPID, pid1 );
			plx = LX1;
		}
		else
		{
			GMM.addGPPicture( nullptr, ( Lx - LX1 ) >> 1, y0, CPIC1.GPID, pid1 );
			y0 += LY1 + 8;
        }
    }
	TextViewer* TV1;

	TV1 = GMM.addTextViewer( nullptr, x0, y0, Lx - 32 - plx, 1000, "tmp1.txt", &f16x16w );
	TV1->Enabled = 0;
	int dy = TV1->NLines*TV1->SymSize;
	if ( or1 )y0 += dy > LY1 ? dy : LY1;
	else y0 += dy;

	int NTB = 1;
	int LL = strlen( Quest );
	for ( int j = 0; j < LL; j++ )if ( Quest[j] == '|' )NTB++;
	x0 = 0;//Lx/2;
	//int DX=Lx/NTB;
	//int Ny=((y0+38)>>6)+1;
    //int Ny1=Ny+((NTB*dst)>>6);
	int qy0 = y0 + 32;//Ny1*64-dst*NTB+10;
	char cc[128];
	int pos;
	TextViewer* TVQ[16];
	for ( int q = 0; q < NTB; q++ )
	{
		pos = 0;
		char c;
		do
		{
			c = Quest[0];
			if ( c != '|' )
			{
				cc[pos] = c;
				pos++;
				cc[pos] = 0;
            }
			Quest++;
		} while ( c != 0 && c != '|' );
		char fc[100];
		sprintf( fc, "tmpf%d.txt", q );
		GFILE* f = Gopen( fc, "w" );
		Gprintf( f, "%s", GetTextByID( cc ) );
		Gclose( f );
		TVQ[q] = GMM.addTextViewer( nullptr, x0 + 32, qy0, Lx - 64, 300, fc, &f16x16g );
		TVQ[q]->y1 = TVQ[q]->y + TVQ[q]->NLines*TVQ[q]->SymSize + 4;
		qy0 += TVQ[q]->NLines*TVQ[q]->SymSize + 16;
		//TextButton* TB=GMM.addTextButton(nullptr,x0,qy0+q*dst,GetTextByID(cc),&fntrb,&fntb,&fntb,1);
		TVQ[q]->OnUserClick = &MMItemChoose;
		TVQ[q]->UserParam = q;
		if ( PlayGameMode )
		{
			TVQ[q]->Enabled = 0;
        }
    }
	qy0 += 32;
	int DDY = ( RealLy - qy0 ) / 2 - 80;//(RealLy-64*Ny1)/2-80;
	GMM.BaseY += DDY;
	for ( int p = 0; p < 512; p++ )
	{
		if ( GMM.DSS[p] && GMM.DSS[p]->Visible )
		{
			GMM.DSS[p]->y += DDY;
			GMM.DSS[p]->y1 += DDY;
        }
    }
	ItemChoose = -1;
	//DrawPaperPanelShadow((RealLx-Lx)>>1,80+DDY-32,((RealLx-Lx)>>1)+Lx,80+DDY+Ny1*64+20);
	DrawPaperPanel( ( RealLx - Lx ) >> 1, 80 + DDY - 32, ( ( RealLx - Lx ) >> 1 ) + Lx, 80 + DDY + qy0/*Ny1*64+20*/ );
	int T0 = GetTickCount();
	do
	{
		if ( PlayGameMode )
		{
			for ( int i = 0; i < NTB; i++ )
			{
				if ( i == CurrentAnswer )TVQ[i]->Font = &f16x16w;
				else TVQ[i]->Font = &f16x16g;
            }
		}
		else
		{
			for ( int i = 0; i < NTB; i++ )
			{
				if ( TVQ[i]->MouseOver )TVQ[i]->Font = &f16x16w;
				else TVQ[i]->Font = &f16x16g;
            }
        }
		ProcessMessages();
		//if(KeyPressed&&(LastKey==13||LastKey==27)){
		//	KeyPressed=false;
		//	ItemChoose=mcmOk;
        //}
		DrawPaperPanel( ( RealLx - Lx ) >> 1, 80 + DDY - 32, ( ( RealLx - Lx ) >> 1 ) + Lx, 80 + DDY + qy0/*Ny1*64+20*/ );
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		GMM.RefreshView();
		if ( PlayGameMode&&GetTickCount() - T0 > 5000 )ItemChoose = CurrentAnswer;
	} while ( ItemChoose == -1 );
	ContinueGame = true;
	UnPress();
	return ItemChoose;
}

void ProcessMissionText( char* Bmp, char* Text )
{
	ItemChoose = -1;
	int MPPLX = GETV( "MPPLX" );
	int MPPLY = GETV( "MPPLY" );
	int MPLDX = GETV( "MPLDX" );
	int MPRDX = GETV( "MPRDX" );
	int MPDDY = GETV( "MPDDY" );
	int MPUDY = GETV( "MPDDY" );
	//char* BmpName=GETS("@MISSPANEL");
	DialogsSystem GMM( smapx + ( nsmaplx << 4 ) - ( MPPLX >> 1 ), smapy + ( mul3( nsmaply ) << 2 ) - ( MPPLY >> 1 ) );
	//SQPicture SP(BmpName);

	SQPicture SPU;
	SPU.PicPtr = nullptr;
	LocalGP CPIC;
	int pid = -1;
	char cc1[100];
	if ( strcmp( Bmp, "NONE" ) )
	{
		sscanf( Bmp, "%s%d", cc1, &pid );
		if ( pid != -1 )
		{
			CPIC.Load( cc1 );
        }
    }

	ResFile RF = RRewrite( "tmp.txt" );
	RBlockWrite( RF, Text, strlen( Text ) );
	RClose( RF );
	LocalGP LGP;
	LocalGP FONT( "rom11" );
	LocalGP FONTB( "vinBig" );
	//LocalGP FONTRB("vinrBig");
	RLCFont f16x16w( FONT.GPID );//"rom11.rlc");
	RLCFont fntb( FONTB.GPID );
	RLCFont fntrb( FONTB.GPID );
	fntrb.SetRedColor();
	//RLCFont f16x16w(FONT.GPID);//"rom11.rlc");
	/*
	if(strstr(BmpName,".bmp")){
		//GMM.addPicture(nullptr,0,0,&SP,&SP,&SP);
	}else{
		LGP.Load(BmpName);
		GPS.ShowGP(GMM.BaseX,GMM.BaseY,LGP.GPID,0,0);
		GMM.addGPPicture(nullptr,0,0,LGP.GPID,1);
    }
	*/
	//int DY=0;
	//if(SPU.PicPtr){
	//	GMM.addPicture(nullptr,0,0,&SPU,&SPU,&SPU);
	//	DY=SPU.GetLy();
    //}
	int LX = 0;
	int LY = 0;
	if ( pid != -1 )
	{
		LX = GPS.GetGPWidth( CPIC.GPID, pid );
		LY = GPS.GetGPHeight( CPIC.GPID, pid ) + 16;
		GMM.addGPPicture( nullptr, ( MPPLX - LX ) >> 1, MPUDY, CPIC.GPID, pid );
    }
	LocalGP SCROL( "Interface\\PScroll" );
	VScrollBar* SB = GMM.addNewGP_VScrollBar( nullptr, MPPLX - MPRDX + 5 - 4, MPUDY + LY, MPPLY - MPDDY - MPUDY - LY - 40, 1, 0, SCROL.GPID, 0 );
	TextViewer* TV;
	TV = GMM.addTextViewer( nullptr, MPLDX - 6, MPUDY + LY, MPPLX - MPLDX - MPRDX - 6, MPPLY - MPDDY - MPUDY - LY - 40, "tmp.txt", &f16x16w );
	TV->AssignScroll( SB );
	TextButton* YesBtn = GMM.addTextButton( nullptr, MPPLX >> 1, MPPLY - 60, GetTextByID( "@MSOK" ), &fntrb, &fntb, &fntb, 1 );
	YesBtn->UserParam = mcmOk;
	YesBtn->OnUserClick = &MMItemChoose;
	if ( PlayGameMode )
	{
		YesBtn->Enabled = 0;
    }
	ItemChoose = -1;
	DrawPaperPanelShadow( GMM.BaseX, GMM.BaseY, GMM.BaseX + MPPLX, GMM.BaseY + MPPLY );
	int T0 = GetTickCount();
	do
	{
		DrawPaperPanel( GMM.BaseX, GMM.BaseY, GMM.BaseX + MPPLX, GMM.BaseY + MPPLY );
		ProcessMessages();
		if ( KeyPressed && ( LastKey == 13 || LastKey == 27 ) )
		{
			KeyPressed = false;
			ItemChoose = mcmOk;
        }
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		GMM.RefreshView();
		if ( PlayGameMode&&GetTickCount() - T0 > 5000 )ItemChoose = mcmOk;
	} while ( ItemChoose == -1 );
	ContinueGame = true;
	UnPress();
}

int ShowHistryItem( char* Bmp, char* Text )
{
	ItemChoose = -1;
	int MPPLX = GETV( "MPPLX" );
	int MPPLY = GETV( "MPPLY" );
	int MPLDX = GETV( "MPLDX" );
	int MPRDX = GETV( "MPRDX" );
	int MPDDY = GETV( "MPDDY" );
	int MPUDY = GETV( "MPDDY" );

	DialogsSystem GMM( smapx + ( nsmaplx << 4 ) - ( MPPLX >> 1 ), smapy + ( mul3( nsmaply ) << 2 ) - ( MPPLY >> 1 ) );

	SQPicture SPU;
	SPU.PicPtr = nullptr;

	ResFile RF = RRewrite( "tmp.txt" );
	RBlockWrite( RF, Text, strlen( Text ) );
	RClose( RF );

	LocalGP LGP;
	LocalGP FONT( "rom11" );
	LocalGP FONTB( "vinBig" );

	RLCFont f16x16w( FONT.GPID );

	RLCFont fntb( FONTB.GPID );
	RLCFont fntrb( FONTB.GPID );
	fntrb.SetRedColor();

	RLCFont fntw( FONTB.GPID );
	fntw.SetWhiteColor();

	LocalGP CPIC;

	int pid = -1;
	char cc1[100];
	if ( strcmp( Bmp, "NONE" ) )
	{
		sscanf( Bmp, "%s%d", cc1, &pid );
		if ( pid != -1 )
		{
			CPIC.Load( cc1 );
		}
	}

    if ( SPU.PicPtr )
	{
		GMM.addPicture( nullptr, 0, 0, &SPU, &SPU, &SPU );
	}

	int LX = 0;
	int LY = 0;

	if ( pid != -1 )
	{
		LX = GPS.GetGPWidth( CPIC.GPID, pid );
		LY = GPS.GetGPHeight( CPIC.GPID, pid );
		GMM.addGPPicture( nullptr, ( MPPLX - LX ) / 2, MPUDY, CPIC.GPID, pid );
	}

	LocalGP SCROL( "Interface\\PScroll" );
	GMM.addTextButton( nullptr, MPPLX >> 1, -27, GetTextByID( "MHISTORY" ), &fntw, &fntw, &fntw, 1 );
	VScrollBar* SB = GMM.addNewGP_VScrollBar( nullptr, MPPLX - MPRDX + 5 - 6, MPUDY + LY, MPPLY - MPDDY - MPUDY - LY - 40, 1, 0, SCROL.GPID, 0 );
	TextViewer* TV;
	TV = GMM.addTextViewer( nullptr, MPLDX - 6, MPUDY + LY, MPPLX - MPLDX - MPRDX - 6, MPPLY - MPDDY - MPUDY - LY - 40, "tmp.txt", &f16x16w );
	TV->AssignScroll( SB );
	ItemChoose = -1;

	TextButton* YesBtn = GMM.addTextButton( nullptr, MPPLX / 6, MPPLY - 60, GetTextByID( "@MSPREV" ), &fntrb, &fntb, &fntb, 1 );
	YesBtn->UserParam = 1;
	YesBtn->OnUserClick = &MMItemChoose;

	TextButton* NoBtn = GMM.addTextButton( nullptr, MPPLX / 2, MPPLY - 60, GetTextByID( "@MSNEXT" ), &fntrb, &fntb, &fntb, 1 );
	NoBtn->UserParam = 2;
	NoBtn->OnUserClick = &MMItemChoose;

	TextButton* CBtn = GMM.addTextButton( nullptr, 5 * MPPLX / 6, MPPLY - 60, GetTextByID( "@MSCANCEL" ), &fntrb, &fntb, &fntb, 1 );
	CBtn->UserParam = 3;
	CBtn->OnUserClick = &MMItemChoose;

	DrawPaperPanelShadow( GMM.BaseX, GMM.BaseY, GMM.BaseX + MPPLX, GMM.BaseY + MPPLY );
	do
	{
		DrawPaperPanel( GMM.BaseX, GMM.BaseY, GMM.BaseX + MPPLX, GMM.BaseY + MPPLY );
		ProcessMessages();
		if ( KeyPressed )
		{
			if ( LastKey == 13 )ItemChoose = 3;
			else if ( LastKey == 27 )ItemChoose = 3;
			KeyPressed = false;
        }
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		GMM.RefreshView();
	} while ( ItemChoose == -1 );

	return ItemChoose;
}

bool AskMissionQuestion( char* Bmp, char* Text )
{
	ItemChoose = -1;
	int MPPLX = GETV( "MPPLX" );
	int MPPLY = GETV( "MPPLY" );
	int MPLDX = GETV( "MPLDX" );
	int MPRDX = GETV( "MPRDX" );
	int MPDDY = GETV( "MPDDY" );
	int MPUDY = GETV( "MPDDY" );
	//char* BmpName=GETS("@MISSPANEL");
	DialogsSystem GMM( smapx + ( nsmaplx << 4 ) - ( MPPLX >> 1 ), smapy + ( mul3( nsmaply ) << 2 ) - ( MPPLY >> 1 ) );
	//SQPicture SP(BmpName);

	SQPicture SPU;
	SPU.PicPtr = nullptr;
	//if(strcmp(Bmp,"NONE")){
	//	SPU.LoadPicture(Bmp);
    //}

	ResFile RF = RRewrite( "tmp.txt" );
	RBlockWrite( RF, Text, strlen( Text ) );
	RClose( RF );
	LocalGP LGP;
	LocalGP FONT( "rom11" );
	LocalGP FONTB( "vinBig" );
	//LocalGP FONTRB("vinrBig");
	RLCFont f16x16w( FONT.GPID );//"rom11.rlc");
	RLCFont fntb( FONTB.GPID );
	RLCFont fntrb( FONTB.GPID );
	fntrb.SetRedColor();
	LocalGP CPIC;
	int pid = -1;
	char cc1[100];
	if ( strcmp( Bmp, "NONE" ) )
	{
		sscanf( Bmp, "%s%d", cc1, &pid );
		if ( pid != -1 )
		{
			CPIC.Load( cc1 );
        }
    }
	/*
	if(strstr(BmpName,".bmp")){
		//GMM.addPicture(nullptr,0,0,&SP,&SP,&SP);
	}else{
		LGP.Load(BmpName);
		GPS.ShowGP(GMM.BaseX,GMM.BaseY,LGP.GPID,0,0);
		GMM.addGPPicture(nullptr,0,0,LGP.GPID,1);
    }
	*/
    if ( SPU.PicPtr )
	{
		GMM.addPicture( nullptr, 0, 0, &SPU, &SPU, &SPU );
    }
	int LX = 0;
	int LY = 0;
	if ( pid != -1 )
	{
		LX = GPS.GetGPWidth( CPIC.GPID, pid );
		LY = GPS.GetGPHeight( CPIC.GPID, pid );
		GMM.addGPPicture( nullptr, ( MPPLX - LX ) >> 1, MPUDY, CPIC.GPID, pid );
    }
    /*TextViewer* TV =*/ GMM.addTextViewer( nullptr, MPLDX, MPUDY + LY, MPPLX - MPLDX - MPRDX, MPPLY - LY - MPDDY - MPUDY - 40, "tmp.txt", &f16x16w );
	ItemChoose = -1;

	TextButton* YesBtn = GMM.addTextButton( nullptr, MPPLX / 4, 441, GetTextByID( "@MSYES" ), &fntrb, &fntb, &fntb, 1 );
	YesBtn->UserParam = mcmOk;
	YesBtn->OnUserClick = &MMItemChoose;
	if ( PlayGameMode )
	{
		if ( CurrentAnswer )YesBtn->PassiveFont = &fntrb;
		YesBtn->Enabled = 1;
    }

	TextButton* NoBtn = GMM.addTextButton( nullptr, MPPLX * 3 / 4, 441, GetTextByID( "@MSNO" ), &fntrb, &fntb, &fntb, 1 );
	NoBtn->UserParam = mcmCancel;
	NoBtn->OnUserClick = &MMItemChoose;
	if ( PlayGameMode )
	{
		if ( !CurrentAnswer )NoBtn->PassiveFont = &fntrb;
		NoBtn->Enabled = 1;
    }

	DrawPaperPanelShadow( GMM.BaseX, GMM.BaseY, GMM.BaseX + MPPLX, GMM.BaseY + MPPLY );
	int T0 = GetTickCount();
	do
	{
		DrawPaperPanel( GMM.BaseX, GMM.BaseY, GMM.BaseX + MPPLX, GMM.BaseY + MPPLY );
		ProcessMessages();
		if ( KeyPressed )
		{
			if ( LastKey == 13 )ItemChoose = mcmOk;
			else if ( LastKey == 27 )ItemChoose = mcmCancel;
			KeyPressed = false;
        }
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		GMM.RefreshView();
		if ( PlayGameMode&&GetTickCount() - T0 > 5000 )
		{
			ItemChoose = CurrentAnswer == 0 ? mcmCancel : mcmOk;
        }
	} while ( ItemChoose == -1 );
	ContinueGame = true;
	UnPress();
	if ( ItemChoose == mcmOk )return true;
	else return false;
}

//----------------------MiniMap editor interface------------------

static uint8_t * higrad;
static int grsize;

//Load resource file higrad.set
static void SFLB_LoadMEDButtons()
{
	ResFile f1 = RReset( "higrad.set" );
	grsize = RFileSize( f1 );
    higrad = new uint8_t[grsize];
	RBlockRead( f1, higrad, grsize );
	RClose( f1 );
}

//-------------------MiniMap images----------------
static int HiLx;
static int HiLy;
static int HiStepX;
static int HiStepY;
static int winX;
static int winY;
static int winX1;
static int winY1;
static uint8_t * HiMapByte;
static uint8_t * HiMap1;

static void SetupHiMap()
{
	if ( RealLx >= 1024 )
	{
		HiLx = 496;//248;
		HiLy = 496;
	}
	else
	{
		HiLx = 248;
		HiLy = 248;
	}
	HiStepX = div( msx << ( 8 + 5 ), HiLx ).quot;
	HiStepY = div( msy << ( 8 + 5 ), HiLy ).quot;
    HiMapByte = new byte[HiLx * HiLy];
	HiMap1 = new byte[HiLx * HiLy];
}

static void DeleteHiMap()
{
    free( HiMapByte );
	free( HiMap1 );
}

static void CreateHiMap()
{
	int mpx = 256;
	int mpy = 256;
	for ( int iy = 0; iy < HiLy; iy++ )
	{
		for ( int ix = 0; ix < HiLx; ix++ )
		{
			int Hig = GetHeight( mpx >> 8, mpy >> 8 ) >> 4;
			if ( Hig < 0 )
			{
				Hig = 0;
			}

			if ( Hig < grsize )
			{
				Hig = higrad[Hig];
			}
			else
			{
				Hig = higrad[grsize - 1];
			}
            HiMapByte[ix + iy*HiLx] = Hig;
			mpx += HiStepX;
		}
		mpx = 256;
		mpy += HiStepY;
	}
}

//----------------------LOADING THE FILE--------------------//
static void CreateFilesList( const char* Mask, ListBox* LB )
{
	WIN32_FIND_DATA FD;
	HANDLE H = FindFirstFile( Mask, &FD );
	if ( H != INVALID_HANDLE_VALUE )
	{
		do
		{
			LB->AddItem( FD.cFileName, 0 );
		} while ( FindNextFile( H, &FD ) );
		FindClose( H );
	}
}

static bool ProcessLoadingFile( const char* Mask, char* DestName, int Header )
{
	LocalGP HBTN( "Interface\\LoadSaveBM" );
	LocalGP BTNS( "Interface\\G_Load_Game" );
	LocalGP SCROL( "Interface\\GScroll" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	int dx = ( RealLx - 470 ) >> 1;
	int dy = ( RealLy - 415 ) >> 1;
	DialogsSystem MMenu( dx, dy );
	GPS.ShowGP( dx, dy, BTNS.GPID, 4, 0 );
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;
	//Picture* PIC=MMenu.addPicture(nullptr,0,0,&MnPanel,&MnPanel,&MnPanel);
	//MMenu.addGPPicture(nullptr,dx,dy,BTNS.GPID,0);
	MMenu.addGPPicture( nullptr, 0, 0, BTNS.GPID, 5 );
	VScrollBar* VSB = MMenu.addNewGP_VScrollBar( nullptr, 436, 55, 276, 1, 0, SCROL.GPID, 0 );
	ListBox* LB = MMenu.addGP_ListBox( nullptr, 29, 61, 10, BTNS.GPID, 6, 26, &WhiteFont, &YellowFont, VSB );

	//VScrollBar* VS=DSY.addVScrollBar(PIC1,LB->x-DSY.BaseX+LB->oneLx-1,1+LB->y-DSY.BaseY,1000,4,&sb_up_d,&sb_up_e,&sb_up_p,&sb_dn_d,&sb_dn_e,&sb_dn_p,&sb_vbare,&sb_vbar,&sb_btn);
	//LB->VS=VS;226x241
    /*GPPicture* HDR =*/ MMenu.addGPPicture( nullptr, 28, 31, HBTN.GPID, Header );
	GP_Button* OkBtn = MMenu.addGP_Button( nullptr, 49, 353, HBTN.GPID, 2, 0 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	GP_Button* CancelBtn = MMenu.addGP_Button( nullptr, 269, 353, BTNS.GPID, 2, 3 );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;

	CreateFilesList( Mask, LB );
	ItemChoose = -1;
    //int LastCTime = GetRealTime();
	LastKey = 0;
	do
	{
		MMenu.MarkToDraw();
		ProcessMessages();
		StdKeys();
		if ( LB->CurItem == -1 )
		{
			OkBtn->Enabled = false;
			if ( ItemChoose == mcmOk )ItemChoose = -1;
		}
		else OkBtn->Enabled = true;
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk&&LB->CurItem != -1 && LB->NItems )
	{
		strcpy( DestName, LB->GetItem( LB->CurItem )->Message );
		ClearMStack();
		ItemChoose = -1;
		UnPress();
		return true;
    }
	ClearMStack();
	ItemChoose = -1;
	UnPress();
	return false;
}

static bool ProcessSavingFile( const char* Mask, char* DestName, int Header, bool clear )
{
	LocalGP HBTN( "Interface\\LoadSaveBM" );
	LocalGP BTNS( "Interface\\G_Save_Game" );
	LocalGP SCROL( "Interface\\GScroll" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	int dx = ( RealLx - 470 ) >> 1;
	int dy = ( RealLy - 415 ) >> 1;
	DialogsSystem MMenu( dx, dy );
	GPS.ShowGP( dx, dy, BTNS.GPID, 4, 0 );
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;
	//Picture* PIC=MMenu.addPicture(nullptr,0,0,&MnPanel,&MnPanel,&MnPanel);
	//MMenu.addGPPicture(nullptr,dx,dy,BTNS.GPID,0);
	MMenu.addGPPicture( nullptr, 0, 0, BTNS.GPID, 5 );
	VScrollBar* VSB = MMenu.addNewGP_VScrollBar( nullptr, 436, 55, 247, 1, 0, SCROL.GPID, 0 );
	ListBox* LB = MMenu.addGP_ListBox( nullptr, 29, 61, 9, BTNS.GPID, 6, 26, &WhiteFont, &YellowFont, VSB );

    /*GPPicture* HDR =*/ MMenu.addGPPicture( nullptr, 28, 31, HBTN.GPID, Header );
	GP_Button* OkBtn = MMenu.addGP_Button( nullptr, 49, 353, HBTN.GPID, 5, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	GP_Button* CancelBtn = MMenu.addGP_Button( nullptr, 269, 353, BTNS.GPID, 2, 3 );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;

	if ( clear )DestName[0] = 0;
	InputBox* IB = MMenu.addInputBox( nullptr, 114, 306, DestName, 35, 310, 20, &YellowFont, &WhiteFont );
	IB->Active = true;
	ItemChoose = -1;
	CreateFilesList( Mask, LB );
	LB->CurItem = -1;
    int PrevListVal = LB->CurItem;
	KeyPressed = false;
	do
	{
		if ( PrevListVal != LB->CurItem )
		{
			PrevListVal = LB->CurItem;
            // BoonXRay 20.08.2017
            //strcpy( IB->Str, LB->GetItem( LB->CurItem )->Message );
            IB->String = LB->GetItem( LB->CurItem )->Message;
			//IB->Str[strlen(IB->Str)-4]=0;
        }
		MMenu.MarkToDraw();
		ProcessMessages();
		StdKeys();
		//if(LB->CurItem==-1)OkBtn->Enabled=false;
		//else OkBtn->Enabled=true;
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
        // BoonXRay 20.08.2017
        //if ( strlen( IB->Str ) )
        if( ! IB->String.empty() )
		{
			OkBtn->Enabled = true;
		}
		else
		{
			OkBtn->Enabled = false;
			if ( ItemChoose == mcmOk )ItemChoose = -1;
        }
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
		ClearMStack();
		ItemChoose = -1;
		UnPress();
		return true;
    }
	ClearMStack();
	ItemChoose = -1;
	UnPress();
	return false;
}

//------------------TRANSFORMATIONS LIBRARY-----------------
static int xc, yc;//center of the transformation
static int HiMode = 0;
static int tpr1 = 256;//Height
static int tpr2 = 0;  //R1
static int tpr3 = 256;//R2
static int tpr4 = 512;//frquency
static int tpr5 = 512;//Amplitude
static int tpr6 = 512;//Phase
static int HiBrush = 0;
static int HiRound = 0;

static int getR( double a, int r )
{
	int nn = tpr4 >> 6;
	double tm = double( GetRealTime() ) / 1000;
	return int( r*( ( sin( a*(nn) +tpr6 / 256 + tm ) +
		sin( a*( nn + 1 ) + tpr6 / 256 + tm*1.5 ) +
		sin( a*( nn + 2 ) + tpr6 / 256 - tm / 2 ) +
		sin( a*( nn + 3 ) + tpr6 / 256 + tm*1.4 ) +
		sin( a*( nn + 4 ) + tpr6 / 256 - tm*1.5 ) +
		sin( a*( nn + 5 ) + tpr6 / 256 + tm*1.3 ) +
		sin( a*( nn + 6 ) + tpr6 / 256 - tm*1.1 ) +
		sin( a*( nn + 7 ) + tpr6 / 256 + tm*0.8 ) +
		sin( a*( nn + 8 ) + tpr6 / 256 + tm*0.7 ) +
		sin( a*( nn + 9 ) + tpr6 / 256 + tm*0.6 )
		) / 10 +
		+1 + ( tpr5 >> 6 ) ) / ( 2 + ( tpr5 >> 6 ) ) );
}

static void Pix( int x, int y, byte c )
{
	if ( x >= winX&&y >= winY&&x < winX1&&y < winY1 )
	{
		( HiMap1 + x - winX + ( y - winY )*HiLx )[0] = c;
	}
}

static void Circ( int x, int y, int r, int n, byte c )
{
	r = ( r*HiLx ) / 496;
	if ( HiRound == 1 )
	{
		for ( int i = 0; i < n; i++ )
		{
			double alpha = 3.1415 * 2 * i / n;
			int r2 = getR( alpha, r );
			Pix( int( x + r2*sin( alpha ) ), int( y + r2*cos( alpha ) ), c );
		}
	}
	else
	{
        //double time = double( GetRealTime() ) / 1000;
		for ( int i = 0; i < n; i++ )
		{
			double alpha = 3.1415 * 2 * i / n;
			Pix( int( x + r*sin( alpha ) ), int( y + r*cos( alpha ) ), c );
		}
	}
}

static int func1( int r )
{
	if ( r < tpr2 )
		return 1024;

	if ( r > tpr2 + tpr3 )
		return 0;

	int dr = r - tpr2 - ( tpr3 >> 1 );
	if ( dr > 0 )
	{
		return div( ( tpr2 + tpr3 - r )*( tpr2 + tpr3 - r ) << 11, tpr3*tpr3 ).quot;
	}
	else
	{
		return 1024 - div( ( r - tpr2 )*( r - tpr2 ) << 11, tpr3*tpr3 ).quot;
	}
}

static int func2( int r )
{
	if ( r < tpr2 )
		return 1024;

	if ( r > tpr2 + tpr3 )
		return 0;

	int dr = r - tpr2 - tpr3;
	return div( ( dr*dr ) << 10, tpr3*tpr3 ).quot;
}

static int func3( int r )
{
	if ( r < tpr2 )
		return 1024;

	if ( r > tpr2 + tpr3 )
		return 0;

	int dr = tpr2 - r;

	return 1024 - div( ( dr*dr ) << 10, tpr3*tpr3 ).quot;
}

static int func4( int r )
{
	if ( r < tpr2 )
		return 1024;

	if ( r > tpr2 + tpr3 )
		return 0;

	int dr = r - tpr2;
	return div( dr << 10, tpr3 ).quot;
}

static int func5( int r )
{
	if ( r < tpr2 )
		return 1024;

	if ( r > tpr2 + tpr3 )
		return 0;

	int dr = r - tpr2 - ( tpr3 >> 1 );

	if ( dr > 0 )
	{
		return div( ( tpr2 + tpr3 - r )*( tpr2 + tpr3 - r ) << 11, tpr3*tpr3 ).quot;
	}
	else
	{
		return 1024 - div( ( r - tpr2 )*( r - tpr2 ) << 11, tpr3*tpr3 ).quot;
	}
}

typedef int fpar( int );

static int UniTrans( int x, int y, int Hi, fpar* ff1 )
{
	if ( !tpr3 )tpr3 = 1;
	int pk;
	int r = int( sqrt( ( x - xc )*( x - xc ) + ( y - yc )*( y - yc ) ) );
	if ( HiRound == 1 )
	{
		int tp2 = tpr2;
		int tp3 = tpr3;
		double ang = atan2( x - xc, y - yc );
		tpr2 = getR( ang, tpr2 );
		tpr3 = getR( ang, tpr3 );
		pk = ff1( r );
		tpr2 = tp2;
		tpr3 = tp3;
	}
	else pk = ff1( r );
	switch ( HiMode )
	{
	case 0:
		return Hi + ( ( ( tpr1 - Hi )*pk ) >> 10 );
	case 1:
	{
		int za = ( GetHeight( x - 32, y ) + GetHeight( x + 32, y ) +
			GetHeight( x, y - 32 ) + GetHeight( x, y + 32 ) ) >> 2;
		return div( tpr1*za + ( Hi << 9 ), 512 + tpr1 ).quot;
    }
	case 2:
		return Hi + ( ( tpr1*pk ) >> 15 );
	case 3:
		pk = Hi - ( ( tpr1*pk ) >> 15 );
		if ( pk < 0 )pk = 0;
		return pk;
	default:
		return Hi;
    }
}

static int Transf1( int x, int y, int Hi )
{
	return UniTrans( x, y, Hi, &func1 );
}

static int Transf2( int x, int y, int Hi )
{
	return UniTrans( x, y, Hi, &func2 );
}

static int Transf3( int x, int y, int Hi )
{
	return UniTrans( x, y, Hi, &func3 );
}

static int Transf4( int x, int y, int Hi )
{
	return UniTrans( x, y, Hi, &func4 );
}

static int Transf5( int x, int y, int Hi )
{
	return UniTrans( x, y, Hi, &func5 );
}

//----------------------------------------------------------
static bool PrepareMap( SimpleDialog* /*SD*/ )
{
	for ( int x = 0; x < VertInLine; x++ )
	{
		for ( int y = MaxTH - 2; y > 0; y-- )
		{
			int vert = x + y*VertInLine;
			int hi0 = ( mul3( GetTriY( vert ) ) >> 2 ) - THMap[vert];
			int hi1 = ( mul3( GetTriY( vert + VertInLine ) ) >> 2 ) - THMap[vert + VertInLine];
			if ( hi1 - hi0 < 0 )
			{
				THMap[vert] += hi0 - hi1;
			}
		}
	}
	ItemChoose = -2;
	return true;
}

static bool LoadClick( SimpleDialog* /*SD*/ )
{
	char BMPS[128];
	if ( ProcessLoadingFile( "UserBitmaps\\*.bmp", BMPS, 3 ) )
	{
		byte* DATA;
		char NAME[128];
		sprintf( NAME, "UserBitmaps\\%s", BMPS );
		int LX, LY;
		if ( LoadBitmapLikeGrayscale( NAME, &LX, &LY, &DATA ) )
		{
			int maxx = 7680 << ADDSH;
			int maxv = ( MaxTH + 1 )*MaxTH;
			for ( int i = 0; i < maxv; i++ )
			{
				int x = GetTriX( i );
				int y = GetTriY( i );
				THMap[i] = GetResVal( DATA, LX, LY, maxx, maxx, x, y );
			}
		}
	}
	ItemChoose = -2;
	return true;
}

static int MapGP = -1;

static void EditMiniMap()
{
	SetupHiMap();
	CreateHiMap();
	//Loading resources
	LocalGP BTNS( "Interface\\EditRelief" );
	int BTDX;
	int BTDY;
	int SCROLDX;
    int TLX;
	int TLY;
	int SID;
	int pdx;
	int pdy;
	int LOADDX;
	if ( RealLx < 1024 )
	{
		BTDX = 0;
		BTDY = 0;
		SCROLDX = 0;
        TLX = 519;
		TLY = 466;
		SID = 0;
		pdx = 4;
		pdy = 3;
		LOADDX = 0;
	}
	else
	{
		BTDX = 121;
		BTDY = 659 - 421 + 2;
		SCROLDX = 540 - 301;
        TLX = 760;
		TLY = 698;
		SID = 47;
		pdx = 0;
		pdy = 0;
		LOADDX = 656 - 415 - 1;
	}

	//Implementation
	short* VERHI = new short[( MaxTH + 1 )*MaxTH];
	memcpy( VERHI, THMap, ( MaxTH + 1 )*MaxTH * 2 );
	DialogsSystem EMM( ( RealLx - TLX ) >> 1, ( RealLy - TLY ) >> 1 );
    /*GPPicture* PIC =*/ EMM.addGPPicture( nullptr, 0, 0, BTNS.GPID, SID );

	//EMM.addGPPicture(nullptr,0,0,BTNS.GPID,1);

	GP_Button* PrepareBtn = EMM.addGP_Button( nullptr, 199 + BTDX, 421 + BTDY, BTNS.GPID, 6, 7 );
	PrepareBtn->OnUserClick = &PrepareMap;

	GP_Button* CancelBtn = EMM.addGP_Button( nullptr, 361 + BTDX, 421 + BTDY, BTNS.GPID, 3, 2 );
	CancelBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;

	GP_Button* OkBtn = EMM.addGP_Button( nullptr, 37 + BTDX, 421 + BTDY, BTNS.GPID, 5, 4 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	GP_Button* LoadBtn = EMM.addGP_Button( nullptr, 415 + LOADDX, 343 + BTDY, BTNS.GPID, 33, 32 );
	LoadBtn->OnUserClick = &LoadClick;

    /*int scrox = GETV( "RESCRX" );
    int scroy = GETV( "RESCRY" );*/
	int DYY = 12;

	VScrollBar* HB1 = EMM.addGP_ScrollBarL( nullptr, 302 + SCROLDX, 71 + DYY, 1024, 64, MapGP + 1, 4, 195, 12, 0, 0 );
	VScrollBar* HB2 = EMM.addGP_ScrollBarL( nullptr, 302 + SCROLDX, 116 + DYY, 1024, 256, MapGP + 1, 4, 195, 12, 0, 0 );
	VScrollBar* HB3 = EMM.addGP_ScrollBarL( nullptr, 302 + SCROLDX, 161 + DYY, 1024, 512, MapGP + 1, 4, 195, 12, 0, 0 );
	VScrollBar* HB4 = EMM.addGP_ScrollBarL( nullptr, 302 + SCROLDX, 206 + DYY, 1024, 64, MapGP + 1, 4, 195, 12, 0, 0 );
	VScrollBar* HB5 = EMM.addGP_ScrollBarL( nullptr, 302 + SCROLDX, 251 + DYY, 1024, 64, MapGP + 1, 4, 195, 12, 0, 0 );
	VScrollBar* HB6 = EMM.addGP_ScrollBarL( nullptr, 302 + SCROLDX, 296 + DYY, 1024, 64, MapGP + 1, 4, 195, 12, 0, 0 );
	HB1->SPos = tpr1;
	HB2->SPos = tpr2;
	HB3->SPos = tpr3;
	HB4->SPos = tpr4;
	HB5->SPos = tpr5;
    HB6->SPos = tpr6;
	CheckBox* TRN1 = EMM.addGP_CheckBox( nullptr, 31, 358 + BTDY, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 8, 9, 10 );
	CheckBox* TRN3 = EMM.addGP_CheckBox( nullptr, 59, 358 + BTDY, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 11, 12, 13 );
	CheckBox* TRN4 = EMM.addGP_CheckBox( nullptr, 87, 358 + BTDY, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 14, 15, 16 );
	CheckBox* TRN5 = EMM.addGP_CheckBox( nullptr, 115, 358 + BTDY, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 17, 18, 19 );
	CheckBox* TRN6 = EMM.addGP_CheckBox( nullptr, 143, 358 + BTDY, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 20, 21, 22 );
	if ( HiBrush == 0 )TRN1->State = 1;
	if ( HiBrush == 1 )TRN3->State = 1;
	if ( HiBrush == 2 )TRN4->State = 1;
	if ( HiBrush == 3 )TRN5->State = 1;
	if ( HiBrush == 4 )TRN6->State = 1;

	CheckBox* Absolut = EMM.addGP_CheckBox( nullptr, 183, 358 + BTDY, "", &YellowFont, &YellowFont, 2, 0, BTNS.GPID, 35, 36, 37 );
	CheckBox* Soft = EMM.addGP_CheckBox( nullptr, 211, 358 + BTDY, "", &YellowFont, &YellowFont, 2, 0, BTNS.GPID, 38, 39, 40 );
	CheckBox* More = EMM.addGP_CheckBox( nullptr, 239, 358 + BTDY, "", &YellowFont, &YellowFont, 2, 0, BTNS.GPID, 41, 42, 43 );
	CheckBox* Less = EMM.addGP_CheckBox( nullptr, 267, 358 + BTDY, "", &YellowFont, &YellowFont, 2, 0, BTNS.GPID, 44, 45, 46 );
	if ( HiMode == 0 )Absolut->State = 1;
	if ( HiMode == 1 )Soft->State = 1;
	if ( HiMode == 2 )More->State = 1;
	if ( HiMode == 3 )Less->State = 1;
	CheckBox* PEN1 = EMM.addGP_CheckBox( nullptr, 307, 358 + BTDY, "", &YellowFont, &YellowFont, 3, 0, BTNS.GPID, 23, 24, 25 );
	CheckBox* PEN2 = EMM.addGP_CheckBox( nullptr, 335, 358 + BTDY, "", &YellowFont, &YellowFont, 3, 0, BTNS.GPID, 26, 27, 28 );
	if ( HiRound == 0 )PEN1->State = 1;
	if ( HiRound == 1 )PEN2->State = 1;
	if ( HiMode == 0 )Absolut->State = 1;
	CheckBox* GETCL = EMM.addGP_CheckBox( nullptr, 375, 358 + BTDY, "", &YellowFont, &YellowFont, 4, 0, BTNS.GPID, 29, 30, 31 );
	GETCL->Transparent = false;
	BPXView* BV = EMM.addBPXView( nullptr, 33 + pdx, 59 + pdy, HiLx, HiLy, 1, 1, 1, HiMap1, nullptr );
	BV->EnableSelection = false;
	ItemChoose = -1;
	winX = BV->x;
	winY = BV->y;
	winX1 = BV->x1;
	winY1 = BV->y1;
	do
	{
		tpr1 = HB1->SPos;
		tpr2 = HB2->SPos;
		tpr3 = HB3->SPos;
		tpr4 = HB4->SPos;
		tpr5 = HB5->SPos;
		tpr6 = HB6->SPos;
		if ( Absolut->State )HiMode = 0;
		if ( Soft->State )HiMode = 1;
		if ( More->State )HiMode = 2;
		if ( Less->State )HiMode = 3;
		if ( PEN1->State )HiRound = 0;
		if ( PEN2->State )HiRound = 1;
		if ( TRN1->State )HiBrush = 0;
		if ( TRN3->State )HiBrush = 1;
		if ( TRN4->State )HiBrush = 2;
		if ( TRN5->State )HiBrush = 3;
		if ( TRN6->State )HiBrush = 4;
		if ( ItemChoose == -2 )
		{
			CreateHiMap();
			ItemChoose = -1;
        }
		if ( BV->MouseOver&&Lpressed && !GETCL->State )
		{
			xc = int( ( mouseX - BV->x )*HiStepX ) >> 8;
			yc = int( ( mouseY - BV->y )*HiStepY ) >> 8;
			if ( TRN1->State )
				CreateEffect( xc, yc, tpr2 + tpr3, &Transf1 );
			//if(TRN2->State)
			//	CreateEffect(xc,yc,tpr2+tpr3,&Transf2);
			if ( TRN3->State )
				CreateEffect( xc, yc, tpr2 + tpr3, &Transf2 );
			if ( TRN4->State )
				CreateEffect( xc, yc, tpr2 + tpr3, &Transf3 );
			if ( TRN5->State )
				CreateEffect( xc, yc, tpr2 + tpr3, &Transf4 );
			if ( TRN6->State )
				CreateEffect( xc, yc, tpr2 + tpr3, &Transf5 );
			if ( !HiMode )Lpressed = false;
			CreateHiMap();
			//EMM.MarkToDraw();
			GETCL->State = 0;
			GETCL->NeedToDraw = true;
        }
		if ( BV->MouseOver&&GETCL->State )
		{
			xc = int( ( mouseX - BV->x )*HiStepX ) >> 8;
			yc = int( ( mouseY - BV->y )*HiStepY ) >> 8;
			HB1->SPos = GetHeight( xc, yc );
			CreateHiMap();
			if ( Lpressed )GETCL->State = 0;
			Lpressed = false;
			//EMM.MarkToDraw();
        }
        memcpy( HiMap1, HiMapByte, HiLx*HiLy );
		Circ( mouseX, mouseY, tpr2 >> 5, tpr2 >> 2, 255 );
		Circ( mouseX, mouseY, ( tpr2 + tpr3 ) >> 5, ( tpr2 + tpr3 ) >> 2, 255 );
		ProcessMessages();
		EMM.MarkToDraw();
		EMM.ProcessDialogs();
		//CopyToRealScreen(EMM.BaseX+24,EMM.BaseY+24,HiLx,HiLy,HiMap);
		EMM.RefreshView();
	} while ( ItemChoose < 0 );
	DeleteHiMap();
	if ( ItemChoose == mcmCancel )
	{
		memcpy( THMap, VERHI, ( MaxTH + 1 )*MaxTH * 2 );
    }
	free( VERHI );
	ItemChoose = -1;
	CreateMiniMap();
	ClearRender();
	UnPress();
}

//---------------------------------------------------------------//
//-------------/    Choosing elements of interface     \---------//
//-------------\         trees,stones,....             /---------//
//---------------------------------------------------------------//
int InteriorType;
static uint16_t TreeItems[256];
static uint16_t NTreeItems;

static void ChooseTree()
{
	SQPicture PAD( "trees.bpx" );
	SQPicture scr_Line( "Scr_10.bpx" );
	SQPicture scr_L( "Scr_L.bpx" );
	SQPicture scr_R( "Scr_R.bpx" );
	SQPicture scr_U_G( "Scr_U_G.bpx" );
	RLCFont f24y( "gf24y.rlc" );
	RLCFont f24w( "gf24w.rlc" );
	int mmlx = PAD.GetLx();
	int mmly = PAD.GetLy();
	DialogsSystem GMM( smapx + ( smaplx << 4 ) - ( mmlx >> 1 ), smapy + ( mul3( smaply ) << 2 ) - ( mmly >> 1 ) );
	Picture* PIC1 = GMM.addPicture( nullptr, 0, 0, &PAD, &PAD, &PAD );
	int RT;
	if ( !GPS.RLCImage[TREES.FileID] )
	{
		GPS.LoadGP( TREES.FileID );
    }
	RT = TREES.FileID;
	RLCListBox* RB = GMM.addRLCListBox( PIC1, 83, 62, 384, 255, RT, 0, 254 );
	//RLCListBox* RB=GMM.addRLCListBox(PIC1,83,62,384,127,RT,0,254);
	//RLCListBox* RB1=GMM.addRLCListBox(PIC1,83,62+128,384,127,RT,0,254);
	VScrollBar* SB = GMM.addHScrollBar( PIC1, 65, 328, RB->MaxXpos, 0, &scr_L, &scr_L, &scr_L, &scr_R, &scr_R, &scr_R, &scr_Line, &scr_Line, &scr_U_G );
	TextButton* OkBtn = GMM.addTextButton( PIC1, 181, 379 - 3, "Да", &f24w, &f24y, &f24y, 1 );
	TextButton* CancelBtn = GMM.addTextButton( PIC1, 365 - 3, 379 - 3, "Отмена", &f24w, &f24y, &f24y, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;
	ItemChoose = -1;
	do
	{
		ProcessMessages();
		RB->XPos = SB->SPos;
		//RB1->XPos=SB->SPos+322;
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		//if(RB->Done){
		//	memset(RB1->Choosed,0,RB->NItems);
		//	RB->Done=false;
        //}
		//if(RB1->Done){
		//	memset(RB->Choosed,0,RB->NItems);
		//	RB1->Done=false;
        //}
		//for(int z=0;z<RB->NItems;z++){
		//	RB->Choosed[z]|=RB1->Choosed[z];
		//	RB1->Choosed[z]|=RB->Choosed[z];
        //}
		GMM.RefreshView();
	} while ( ItemChoose == -1 );
	NTreeItems = 0;
	if ( ItemChoose == mcmOk )
	{
		for ( int z = 0; z < RB->NItems; z++ )if ( RB->Choosed[z] && NTreeItems < 256 )
		{
			TreeItems[NTreeItems] = z;
			NTreeItems++;
        }
		if ( NTreeItems )
		{
			HeightEditMode = 2;
			MEditMode = false;
			BlobMode = -1;
			InteriorType = 0;
        }
    }
	UnPress();
}

static void ChooseStone()
{
	SQPicture PAD( "stones.bpx" );
	SQPicture scr_Line( "Scr_10.bpx" );
	SQPicture scr_L( "Scr_L.bpx" );
	SQPicture scr_R( "Scr_R.bpx" );
	SQPicture scr_U_G( "Scr_U_G.bpx" );
	RLCFont f24y( "gf24y.rlc" );
	RLCFont f24w( "gf24w.rlc" );
	int mmlx = PAD.GetLx();
	int mmly = PAD.GetLy();
	DialogsSystem GMM( smapx + ( smaplx << 4 ) - ( mmlx >> 1 ), smapy + ( mul3( smaply ) << 2 ) - ( mmly >> 1 ) );
	Picture* PIC1 = GMM.addPicture( nullptr, 0, 0, &PAD, &PAD, &PAD );
	int RT;
	if ( !GPS.RLCImage[STONES.FileID] )
	{
		GPS.LoadGP( STONES.FileID );
    }
	RT = STONES.FileID;
	RLCListBox* RB = GMM.addRLCListBox( PIC1, 83, 62, 384, 127, RT, 0, 254 );
	RLCListBox* RB1 = GMM.addRLCListBox( PIC1, 83, 62 + 128, 384, 127, RT, 0, 254 );
	VScrollBar* SB = GMM.addHScrollBar( PIC1, 65, 328, RB->MaxXpos, 0, &scr_L, &scr_L, &scr_L, &scr_R, &scr_R, &scr_R, &scr_Line, &scr_Line, &scr_U_G );
	TextButton* OkBtn = GMM.addTextButton( PIC1, 181, 379 - 3, "Да", &f24w, &f24y, &f24y, 1 );
	TextButton* CancelBtn = GMM.addTextButton( PIC1, 365 - 3, 379 - 3, "Отмена", &f24w, &f24y, &f24y, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;
	ItemChoose = -1;
	do
	{
		ProcessMessages();
		RB->XPos = SB->SPos;
		RB1->XPos = SB->SPos + 322;
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		if ( RB->Done )
		{
			memset( RB1->Choosed, 0, RB->NItems );
			RB->Done = false;
        }
		if ( RB1->Done )
		{
			memset( RB->Choosed, 0, RB->NItems );
			RB1->Done = false;
        }
		for ( int z = 0; z < RB->NItems; z++ )
		{
			RB->Choosed[z] |= RB1->Choosed[z];
			RB1->Choosed[z] |= RB->Choosed[z];
        }
		GMM.RefreshView();
	} while ( ItemChoose == -1 );
	NTreeItems = 0;
	if ( ItemChoose == mcmOk )
	{
		for ( int z = 0; z < RB->NItems; z++ )if ( RB->Choosed[z] && NTreeItems < 256 )
		{
			TreeItems[NTreeItems] = z;
			NTreeItems++;
        }
		if ( NTreeItems )
		{
			HeightEditMode = 2;
			MEditMode = false;
			BlobMode = -1;
			InteriorType = 1;
        }
    }
	UnPress();
}

static void ChooseHole()
{
	SQPicture PAD( "stones.bpx" );
	SQPicture scr_Line( "Scr_10.bpx" );
	SQPicture scr_L( "Scr_L.bpx" );
	SQPicture scr_R( "Scr_R.bpx" );
	SQPicture scr_U_G( "Scr_U_G.bpx" );
	RLCFont f24y( "gf24y.rlc" );
	RLCFont f24w( "gf24w.rlc" );
	int mmlx = PAD.GetLx();
	int mmly = PAD.GetLy();
	DialogsSystem GMM( smapx + ( smaplx << 4 ) - ( mmlx >> 1 ), smapy + ( mul3( smaply ) << 2 ) - ( mmly >> 1 ) );
	Picture* PIC1 = GMM.addPicture( nullptr, 0, 0, &PAD, &PAD, &PAD );
	int RT;
	if ( !GPS.RLCImage[HOLES.FileID] )
	{
		GPS.LoadGP( HOLES.FileID );
	}
	RT = HOLES.FileID;
	RLCListBox* RB = GMM.addRLCListBox( PIC1, 83, 62, 384, 127, RT, 0, 254 );
	RLCListBox* RB1 = GMM.addRLCListBox( PIC1, 83, 62 + 128, 384, 127, RT, 0, 254 );
	VScrollBar* SB = GMM.addHScrollBar( PIC1, 65, 328, RB->MaxXpos, 0, &scr_L, &scr_L, &scr_L, &scr_R, &scr_R, &scr_R, &scr_Line, &scr_Line, &scr_U_G );
	TextButton* OkBtn = GMM.addTextButton( PIC1, 181, 379 - 3, "Да", &f24w, &f24y, &f24y, 1 );
	TextButton* CancelBtn = GMM.addTextButton( PIC1, 365 - 3, 379 - 3, "Отмена", &f24w, &f24y, &f24y, 1 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;
	ItemChoose = -1;
	do
	{
		ProcessMessages();
		RB->XPos = SB->SPos;
		RB1->XPos = SB->SPos + 322;
		GMM.MarkToDraw();
		GMM.ProcessDialogs();
		if ( RB->Done )
		{
			memset( RB1->Choosed, 0, RB->NItems );
			RB->Done = false;
		}
		if ( RB1->Done )
		{
			memset( RB->Choosed, 0, RB->NItems );
			RB1->Done = false;
		}
		for ( int z = 0; z < RB->NItems; z++ )
		{
			RB->Choosed[z] |= RB1->Choosed[z];
			RB1->Choosed[z] |= RB->Choosed[z];
		}
		GMM.RefreshView();
	} while ( ItemChoose == -1 );
	NTreeItems = 0;
	if ( ItemChoose == mcmOk )
	{
		for ( int z = 0; z < RB->NItems; z++ )if ( RB->Choosed[z] && NTreeItems < 256 )
		{
			TreeItems[NTreeItems] = z;
			NTreeItems++;
		}
		if ( NTreeItems )
		{
			HeightEditMode = 2;
			MEditMode = false;
			BlobMode = -1;
			InteriorType = 2;
		}
	}
	UnPress();
}

static void ChooseInterior( int Type )
{
	switch ( Type )
	{
	case 0:
		ChooseTree();
		return;
	case 1:
		ChooseStone();
		return;
	case 2:
		ChooseHole();
		return;
	}
}

int GetTreeItem()
{
	return TreeItems[( rando()*NTreeItems ) >> 15];
}
//Multiplayer Interface

//-------------Multiplayer settings--------------

static bool WaitingHostGame( int ID )
{
	if ( ID )
	{
		MPL_WaitingBattleGame( true, ID - 1 );
	}
	else
	{
		MPL_WaitingGame( true, 0 );
	}

	if ( ItemChoose == mcmCancel )
	{
		CloseMPL();
		ItemChoose = -1;
	}
	else
	{
		if ( ItemChoose == mcmOk )
		{
			StopConnectionToSession( lpDirectPlay3A );
			StartIGame( 0 );
			return true;
		}
	}
	return false;
}

static bool WaitingJoinGame( int ID )
{
	PlayerMenuMode = 1;
	if ( ID )
	{
		MPL_WaitingBattleGame( false, ID - 1 );
	}
	else
	{
		MPL_WaitingGame( false, 0 );
	}

	if ( ItemChoose == mcmCancel )
	{
		CloseMPL();
		ItemChoose = -1;
	}
	else
	{
		//ShowLoading();
		if ( !PlayerMenuMode )
		{
			return true;
		}
	}
	return false;
}

static void GetOptionsFromMap( char* Name )
{
	if ( Name[0] == 'R' && Name[1] == 'N' && Name[3] == ' ' )
	{
		/*
		int v1, v2, v3, options;
		char ccc[32];
		int z = sscanf( Name, "%s%x%x%x%d", ccc, &v1, &v2, &v3, &options );
		//BUG: 4th part (%x) is the nations string which can containt chars
		//above F and cause sscanf to stop at z = 4.
		//Game options won't be set, which leads to different AI decisions and
		//unsync in recordings.
		if ( z == 5)
		{
		*/
		int options = 0;
		int z = sscanf( Name, "%*s %*s %*s %*s %d", &options );//BUGFIX: proper parsing
		if ( 1 == z )
		{
			//Decode 7-digit number into game settings
            int option_values[11] = { 0 };

			DecodeOptionsFromNumber( options, option_values );

			BalloonState = option_values[0];
			CannonState = option_values[1];
			NoArtilleryState = option_values[2];
			XVIIIState = option_values[4];
			CaptState = option_values[5];
			SaveState = option_values[6];
			DipCentreState = option_values[7];
			ShipyardState = option_values[8];
			MarketState = option_values[9];
		}
	}
}

void PrepareGameMedia( uint8_t myid, bool SaveNR )
{
	RandomMapDesc RMP;
	RMP.Load( "Terrain\\Res.dat" );
	rpos = 0;

	if ( !( strstr( CurrentMap, ".SAV" ) || strstr( CurrentMap, ".sav" ) ) )
	{
		if ( CurrentMap[0] == 'R' && CurrentMap[1] == 'N' && CurrentMap[3] == ' ' )
		{
			ADDSH = CurrentMap[2] - '0' + 1;//Map size
		}
		else
		{
		}
	}
	else
	{
		NoWinner = 1;
	}

	//Prepare memory for map data to come
	SetupArrays();
	NewMap( 240 << ADDSH, 240 << ADDSH );

	byte NRTB[8];

	memcpy( NRTB, NatRefTBL, 8 );

	UnLoading();

	if ( SaveNR )
	{
		memcpy( NatRefTBL, NRTB, 8 );
	}

	SetMyNation( 0 );
	SetMyNation( PINFO[myid].ColorID );

	for ( int i = 0; i < 8; i++ )
	{
		//Loading nation
		LoadAllNations( i );
	}

	PrepareToGame();

    //RUN DATA
	RunMethod = 0xFFFFFFFE;
	RunData[0] = 101;//version of the run data
	RunData[1] = dwVersion;//version of the game
	RunData[2] = MyNation;
	RunDataSize = 3;
	memcpy( RunData + RunDataSize, PINFO, sizeof PINFO );
	RunDataSize += sizeof PINFO;
	memcpy( RunData + RunDataSize, COMPSTART, sizeof COMPSTART );
	RunDataSize += sizeof COMPSTART;
	memcpy( RunData + RunDataSize, &RM_LandType, 4 );
	RunDataSize += 4;
	memcpy( RunData + RunDataSize, &RM_Resstart, 4 );
	RunDataSize += 4;
	memcpy( RunData + RunDataSize, &RM_Restot, 4 );
	RunDataSize += 4;
	memcpy( RunData + RunDataSize, NRTB, 8 );
	RunDataSize += 8;
	assert( RunDataSize < 2048 );
	//----------
	//Aliances
	for ( int i = NPlayers; i < 7; i++ )
	{
		if ( COMPSTART[i] )
		{
			word W = COMPSTART[i];
			int Nat = W >> 11;
			int Color = W & 15;
			int Diff = ( W >> 8 ) & 7;
			int Team = ( W >> 4 ) & 15;
			PINFO[i].ColorID = Color;
			PINFO[i].NationID = Nat;
			PINFO[i].GroupID = Team;
			PINFO[i].MapStyle = Diff;
			switch ( Diff )
			{
			case 1:
				strcpy( PINFO[i].name, "Easy" );
				break;
			case 2:
				strcpy( PINFO[i].name, "Normal" );
				break;
			case 3:
				strcpy( PINFO[i].name, "Hard" );
				break;
			case 4:
				strcpy( PINFO[i].name, "Very hard" );
				break;
            }
		}
		else
		{
			PINFO[i].MapStyle = 0;
		}
	}

	for ( int i = 0; i < 7; i++ )
	{
		if ( PINFO[i].name[0] || i < NPlayers || PINFO[i].MapStyle )
		{
			if ( PINFO[i].GroupID == 0 )
			{
				NATIONS[NatRefTBL[PINFO[i].ColorID]].NMask = 1 << NatRefTBL[PINFO[i].ColorID];
			}
			else
			{
				int gid = PINFO[i].GroupID;
				byte mas = 0;
				for ( int j = 0; j < 7; j++ )
				{
					if ( PINFO[j].name[0] || j < NPlayers || PINFO[j].MapStyle )
					{
						if ( PINFO[j].GroupID == gid )
						{
							mas |= 1 << NatRefTBL[PINFO[j].ColorID];
						}
					}
				}
				NATIONS[NatRefTBL[PINFO[i].ColorID]].NMask = mas;
			}
		}
	}

    ItemChoose = mcmSingle;

	//Load animations
	InitExplosions();

    if ( !( strstr( CurrentMap, ".SAV" ) || strstr( CurrentMap, ".sav" ) ) )
	{
		byte RRR[8];
		memcpy( RRR, NatRefTBL, 8 );
		byte NMASKS[8];
		for ( int i = 0; i < 8; i++ )
		{
			NMASKS[i] = NATIONS[i].NMask;
		}

		Load3DMap( CurrentMap );
		GetOptionsFromMap( CurrentMap );

		for ( int i = 0; i < 8; i++ )
		{
			NATIONS[i].NMask = NMASKS[i];
		}

		if ( MapScenaryDLL[0] )
		{
			char ccc[128];
			char cc3[128];
			strcpy( ccc, MapScenaryDLL );
			char* cc1 = strstr( ccc, "." );
			if ( cc1 )
			{
				strcpy( cc1, ".txt" );
			}
			strcpy( cc3, MapScenaryDLL );
			cc1 = strstr( cc3, "." );
			if ( cc1 )
			{
				strcpy( cc1, ".str" );
			}
			FILE* F = fopen( cc1, "r" );
			if ( F )
			{
				fclose( F );
				LoadMessagesFromFile( cc3 );
			}
			SCENINF.Load( MapScenaryDLL, ccc );//"Battles\\Battles.txt");
		}
		else
		{
			for ( int i = NPlayers; i < 7; i++ )
			{
				if ( COMPSTART[i] )
				{
					int Diff = PINFO[i].MapStyle;
					int c = PINFO[i].ColorID;
					int Nat = -1;
					for ( int j = 0; j < MAXOBJECT; j++ )
					{
						OneObject* OB = Group[j];
						if ( OB&&OB->NNUM == c && ( !OB->Sdoxlo ) && OB->newMons->Peasant )
						{
							Nat = OB->Ref.General->NatID;
							j = MAXOBJECT + 1;
						}
					}
					if ( Nat != -1 )
					{
						char ccx[48];
						char* NtID = NatsIDS[Nat];
						sprintf( ccx, "%s.%d", NtID, 0 );
						StartAIEx( PINFO[i].ColorID, ccx, RMP.STY[RM_LandType].AI_Style, RM_Resstart, RM_Restot, Diff - 1 );
					}
				}
			}
		}
		memcpy( NatRefTBL, RRR, 8 );
	}
	else
	{
		NoWinner = 1;
	}
	GNFO.Clear();
	GNFO.Setup();
	//#ifdef SINGLETESTNETWORK
			//running AI

			/*
	#else
			if(NPlayers>1){
				//running AI
				for(int i=NPlayers;i<7;i++){
					if(COMPSTART[i]){
						int Diff=PINFO[i].MapStyle;
						int Nat=PINFO[i].NationID;
						char ccx[48];
						char* NtID=GlobalAI.Ai[Nat].NationID;
						sprintf(ccx,"%s.%d",NtID,0);
						StartAI(PINFO[i].ColorID,ccx,RM_LandType,RM_Resstart,RM_Restot,Diff-1);
                    }
                }
            }
	#endif
			*/
	HideFlags();
    for ( int q = 0; q < 8; q++ )
	{
		for ( int p = 0; p < 8; p++ )
		{
			SetXRESRC( q, p, RES[q][p] );
		}
	}

	CenterScreen();
	CreateInfoMap();
	InitExplosions();

	for ( int i = 0; i < 8; i++ )
	{
		CITY[i].EnumUnits();
		CITY[i].RefreshAbility();
	}

	if ( XVIIIState == 2 )
	{
		int N = NATIONS[0].NUpgrades;
		for ( int i = 0; i < N; i++ )
		{
			for ( int NI = 0; NI < 7; NI++ )
			{
				NewUpgrade* NU = NATIONS[NI].UPGRADE[i];
				if ( ( !NU->Done ) && NU->Options & 8 )
				{
					PerformNewUpgrade( NATIONS + NI, i, nullptr );
					NU->Done = 1;
					NU->PermanentEnabled = 0;
				}
			}
		}
	}
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB )
		{
			OB->NMask = NATIONS[OB->NNUM].NMask;
		}
	}
}

int RM_LandType = 0;
int RM_Resstart = 0;
int RM_Restot = 0;

int CurrentCampagin = -1;
int CurrentMission = -1;

static bool ProcessSingleMission( int n, int Diff )
{
	MaxPingTime = 0;
	SingleMission* SMS = MISSLIST.MISS + n;

	if ( SMS->NIntro == 4 )
	{
		ResFile f = RReset( SMS->Intro[2] );
		if ( f != INVALID_HANDLE_VALUE )
		{
            byte crlf[6] = { 0x20,0x0D,0x0A,0x0D,0x0A };
			int sz = RFileSize( f );
			byte* buf = new byte[sz];
			RBlockRead( f, buf, sz );
			RClose( f );
			f = RRewrite( "tmp.txt" );
			RBlockWrite( f, crlf, sizeof crlf );
			RBlockWrite( f, buf, sz );
			RBlockWrite( f, crlf, sizeof crlf );
			RClose( f );
			free( buf );
		}

		LoadOptionalTable( 5, "2\\dark.grd" );
		LocalGP BTNS( "Interface\\Mission_text" );
		LocalGP PIC( SMS->Intro[1] );
		LocalGP ITL( "it_Small" );
		RLCFont itsm( ITL.GPID );
		LocalGP HDR( "vinBig" );
		RLCFont hdrf( HDR.GPID );
		SQPicture Back( "Interface\\Background_Mission_Text.bmp" );
		DialogsSystem MISS( menu_x_off, menu_y_off );
		MISS.addPicture( nullptr, 0, 0, &Back, &Back, &Back );
		MISS.addGPPicture( nullptr, 48, 119, PIC.GPID, 0 );
		TextViewer* TV = MISS.addTextViewer( nullptr, 532, 258, 445, 424, "tmp.txt", &itsm );
		TV->SymSize = 25;
		TV->PageSize = 18;
		TV->Active = 0;
		TV->Enabled = 0;

		MISS.addGPPicture( nullptr, 522, 258 - 25, BTNS.GPID, 2 );
		MISS.addGPPicture( nullptr, 522 + 233, 258 - 25, BTNS.GPID, 3 );
		MISS.addGPPicture( nullptr, 522, 656, BTNS.GPID, 0 );
		MISS.addGPPicture( nullptr, 522 + 233, 656, BTNS.GPID, 1 );
		MISS.addTextButton( nullptr, 522, 204, SMS->Name, &hdrf, &hdrf, &hdrf, 0 );
		int z = 0;
		sscanf( SMS->Intro[0], "%d", &z );
		int LX = GPS.GetGPWidth( BTNS.GPID, 8 + z );
		MISS.addGPPicture( nullptr, 512 - LX / 2, 7, BTNS.GPID, 8 + z );
		MISS.addGPPicture( nullptr, 512 - LX / 2 - 425, 7, BTNS.GPID, 6 );
		MISS.addGPPicture( nullptr, 512 + LX / 2, 7, BTNS.GPID, 7 );
        const char* tex = GetTextByID( SMS->Intro[3] );
		LX = GetRLCStrWidth( tex, &YellowFont );
		MISS.addGPPicture( nullptr, 512 - LX / 2 - 502, 736, BTNS.GPID, 4 );
		MISS.addGPPicture( nullptr, 512 + LX / 2, 736, BTNS.GPID, 5 );
		MISS.addTextButton( nullptr, 512, 736 + 3, tex, &YellowFont, &YellowFont, &YellowFont, 1 );

		int LinesUp = 0;
		int LinesDn = 0;
		int ShiftY = 0;
		bool exit = false;
		do
		{
			if ( KeyPressed&&LastKey == 27 )exit = true;
			if ( Lpressed || ( KeyPressed && ( LastKey == 34 || LastKey == 13 || LastKey == ' ' ) ) )
			{
				LinesUp = 8;
				LinesDn = 0;
				KeyPressed = 0;
				if ( TV->NLines - TV->Line < 16 )
				{
					if ( !ShiftY )LinesUp = 0;
					else LinesUp = 1;
					if ( Lpressed )exit = true;
                }
            }
			if ( KeyPressed&&LastKey == 33 )
			{
				LinesUp = 0;
				LinesDn = 9;
				KeyPressed = 0;
            }
			if ( LinesUp )
			{
				if ( ShiftY < 25 )
				{
					switch ( ShiftY )
					{
					case 0:
						ShiftY = 9;
						break;
					case 9:
						ShiftY = 9 + 8;
						break;
					case 9 + 8:
						ShiftY = 25;
						break;
                    }
				}
				else
				{
					TV->Line++;
					LinesUp--;
					if ( !TV->Line )LinesUp = 0;
					ShiftY = 0;
                }
            }
			if ( LinesDn )
			{
				if ( !ShiftY )
				{
					if ( TV->Line )
					{
						TV->Line--;
						ShiftY = 9 + 8;
						LinesDn--;
					}
					else LinesDn = 0;
				}
				else
				{
					switch ( ShiftY )
					{
					case 9 + 8:
						ShiftY = 9;
						break;
					case 9:
						ShiftY = 0;
						break;
                    }
                }
            }
			TV->y = 258 - ShiftY;
			ProcessMessages();
			MISS.MarkToDraw();
			MISS.ProcessDialogs();
			MISS.RefreshView();
		} while ( !exit );
	}

	SetMyNation( 0 );
	ShowLoading();
	strcpy( CurrentMap, MISSLIST.MISS[n].MapName );
	CreateNationalMaskForMap( CurrentMap );
	PrepareGameMedia( 0, 0 );
    ItemChoose = mcmSingle;

	for ( int i = 0; i < 7; i++ )
	{
		CITY[i].Difficulty = Diff;
		PINFO[i].ColorID = i;
		if ( i )
		{
			strcpy( PINFO[i].name, "???" );
		}
	}

	SetMyNation( 0 );

	strcpy( PINFO[0].name, PlName );

	PINFO[0].GameTime = 0;
	PINFO[0].VictCond = 0;
	MISSLIST.CurrentMission = n;
	SCENINF.Load( MISSLIST.MISS[n].DLLPath, MISSLIST.MISS[n].Description );
	UnPress();
	RunMethod = 0x12340000 + n + ( Diff << 8 );

	return true;
}

static bool ProcessSingleCampagin( int n )
{
	KeyPressed = 0;
	LastKey = 0;

	SQPicture Back( CAMPAGINS.SCamp[n].CampBmp );

	LocalGP BTNS( "Interface\\One_Campaign" );
	LocalGP BTN1( "Interface\\Single_mission" );
	LocalGP OK1( "Interface\\anm_start_1_on" );
	LocalGP OK2( "Interface\\anm_start_1_off" );
	LocalGP CANCEL1( "Interface\\anm_cancel_on" );
	LocalGP CANCEL2( "Interface\\anm_cancel_off" );
	LocalGP ENC1( "Interface\\anm_enc_on" );
	LocalGP ENC2( "Interface\\anm_enc_off" );
	LocalGP SCROL( "Interface\\_Slider" );
	LoadFog( 2 );

	_strupr( CAMPAGINS.SCamp[n].CampBmp );

	bool Style = strstr( CAMPAGINS.SCamp[n].CampBmp, ".NEW." ) != nullptr;

	DialogsSystem MMenu( menu_x_off, menu_y_off );
	MMenu.OkSound = GetSound( "START" );
	MMenu.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();

	MMenu.HintFont = &hfnt;
	MMenu.HintX = menu_hint_x;
	MMenu.HintY = menu_hint_y;

	SingleCampagin* SCM = CAMPAGINS.SCamp + n;
	TextViewer* TV[32];
	VScrollBar* TSB[32];

	VScrollBar* SB;
	if ( Style )SB = MMenu.addNewGP_VScrollBar( nullptr, 450, 157, 221 - 52, 1, 0, SCROL.GPID, 0 );
	else SB = MMenu.addNewGP_VScrollBar( nullptr, 450, 157, 221, 1, 0, SCROL.GPID, 0 );
	for ( int i = 0; i < SCM->NMiss&&i < 8; i++ )
	{
		if ( ( SCM->OpenIndex[i] & 255 ) && i < 8 - Style - Style )MMenu.addGPPicture( nullptr, 46, 166 + 26 * i, BTNS.GPID, 6 );
    }
	ListBox* LB;
	if ( Style )LB = MMenu.addGP_ListBox( nullptr, 46, 166, 6, BTNS.GPID, 3, 26, &WhiteFont, &YellowFont, SB );
	else LB = MMenu.addGP_ListBox( nullptr, 46, 166, 8, BTNS.GPID, 3, 26, &WhiteFont, &YellowFont, SB );
	LB->M_Over = 0;
	LB->Active = 1;
	LB->FontDy = -2;
	int MaxMiss = 0;
	for ( int i = 0; i < SCM->NMiss; i++ )
	{
		if ( SCM->OpenIndex[i] & 255 )MaxMiss = i + 1;
    }
	bool VISMASK[64];
	memset( VISMASK, 0, sizeof VISMASK );
	for ( int i = 0; i < MaxMiss; i++ )
	{
		//if(SCM->OpenIndex[i]&255){
		LB->AddItem( MISSLIST.MISS[SCM->Miss[i]].Name, 0 );
		LB->CurItem = LB->NItems - 1;
		if ( LB->CurItem - LB->FLItem > LB->ny )
		{
			LB->FLItem = LB->CurItem - LB->ny + 1;
			SB->SPos = LB->FLItem * 32;
        }
        //}
		if ( MISSLIST.MISS[SCM->Miss[i]].NIntro&&strstr( MISSLIST.MISS[SCM->Miss[i]].Intro[0], ".txt" ) )
		{
			if ( Style )TV[i] = MMenu.addTextViewer( nullptr, 54, 431 - 52, 386, 196 - 12, MISSLIST.MISS[SCM->Miss[i]].Intro[0], &YellowFont );
			else TV[i] = MMenu.addTextViewer( nullptr, 54, 431, 386, 196, MISSLIST.MISS[SCM->Miss[i]].Intro[0], &YellowFont );
			TV[i]->SymSize = 21;
			TV[i]->PageSize = ( TV[i]->y1 - TV[i]->y ) / TV[i]->SymSize;
			TV[i]->Visible = 0;
			if ( Style )TSB[i] = MMenu.addNewGP_VScrollBar( nullptr, 450, 416 - 52, 213 - 18, 1, 0, SCROL.GPID, 0 );
			else TSB[i] = MMenu.addNewGP_VScrollBar( nullptr, 450, 416, 213, 1, 0, SCROL.GPID, 0 );
			TSB[i]->Visible = 0;
			TSB[i]->ScrDy = 7;
			TV[i]->AssignScroll( TSB[i] );
		}
		else
		{
			TV[i] = nullptr;
			TSB[i] = nullptr;
        }
		VISMASK[i] = strstr( MISSLIST.MISS[SCM->Miss[i]].ID, "@" ) != nullptr;
    }
	ComboBox* DIFF;
	if ( Style )DIFF = MMenu.addGP_ComboBoxDLX( nullptr, 143, 601, 314, BTN1.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	else DIFF = MMenu.addGP_ComboBox( nullptr, 584, 422, BTN1.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	DIFF->CurLine = 0;
	FILE* f = fopen( "smiss.opt", "r" );
	if ( f )
	{
		int v = 0;
		fscanf( f, "%d", &v );
		fclose( f );
		DIFF->CurLine = v;
    }
	DIFF->AddLine( GetTextByID( "@RMID_EASY" ) );
	DIFF->AddLine( GetTextByID( "@RMID_NORMAL" ) );
	DIFF->AddLine( GetTextByID( "@RMID_HARD" ) );
	DIFF->AddLine( GetTextByID( "@RMID_VERYHARD" ) );
	DIFF->Hint = GetTextByID( "DIFFICULTY" );
	VideoButton* OkBtn = MMenu.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
	OkBtn->OnUserClick = &MMItemChoose;
	OkBtn->UserParam = mcmOk;
	OkBtn->Hint = GetTextByID( "MISS_RUN" );
	VideoButton* CancelBtn = MMenu.addVideoButton( nullptr, 862, 668, CANCEL1.GPID, CANCEL2.GPID );
	CancelBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->Hint = GetTextByID( "MISS_CANCEL" );
HHH1:
	ItemChoose = -1;
	int mm = 1;
	KeyPressed = 0;
	do
	{
		ProcessMessages();
		StdKeys();
		MMenu.MarkToDraw();
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
		for ( int j = 0; j < MaxMiss; j++ )
		{
			if ( TV[j] )
			{
				if ( j == LB->CurItem )
				{
					TV[j]->Visible = 1;
					TSB[j]->Visible = 1;
					TV[j]->Active = 0;
				}
				else
				{
					TV[j]->Visible = 0;
					TSB[j]->Visible = 0;
					TV[j]->Active = 0;
                }
            }
			if ( j == LB->CurItem )
			{
				DIFF->Visible = VISMASK[j];
				DIFF->Enabled = DIFF->Visible;
            }
        }
		LB->Active = 1;
		if ( mm )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			mm = 0;
        }
		StdKeys();
	} while ( ItemChoose == -1 );
	f = fopen( "smiss.opt", "w" );
	if ( f )
	{
        fprintf( f, "%d", DIFF->CurLine );
		fclose( f );
    }
	if ( ItemChoose == mcmOk )
	{
		CurrentCampagin = n;
		CurrentMission = LB->CurItem;
		if ( !ProcessSingleMission( SCM->Miss[LB->CurItem], DIFF->CurLine ) )
		{
			SlowUnLoadPalette( "2\\agew_1.pal" );
			CurrentCampagin = -1;
			CurrentMission = -1;
			goto HHH1;
		}
		else return true;
	}
	else
	{
		CurrentCampagin = -1;
		CurrentMission = -1;
    }
	SlowUnLoadPalette( "2\\agew_1.pal" );
	return false;
}

struct OneAddMission
{
	char Title[128];
	char Map[128];
	char Preview[128];
	char Bitmap[128];
};

class AddMissionsPack
{
public:
	OneAddMission* Pack;
	int NMiss;
	AddMissionsPack();
	~AddMissionsPack();
};

AddMissionsPack::AddMissionsPack()
{
	Pack = nullptr;
	NMiss = 0;
	WIN32_FIND_DATA SR;
	HANDLE H = FindFirstFile( "UserMissions\\*.add", &SR );
	if ( H != INVALID_HANDLE_VALUE )
	{
		do
		{
			char ccc[128];
			sprintf( ccc, "UserMissions\\%s", SR.cFileName );
			GFILE* F = Gopen( ccc, "r" );
			if ( F )
			{
				Pack = (OneAddMission*) realloc( Pack, sizeof( OneAddMission )*( NMiss + 1 ) );
                memset( Pack + NMiss, 0, sizeof( OneAddMission) );
				ReadWinString( F, Pack[NMiss].Title, 127 );
				ReadWinString( F, Pack[NMiss].Map, 127 );
				ReadWinString( F, ccc, 127 );
				_strupr( ccc );
				char* c1 = strstr( ccc, ".DLL" );
				if ( !c1 )c1 = strstr( ccc, ".CMS" );
				if ( c1 )
				{
					strcpy( c1, ".dsc" );
					strcpy( Pack[NMiss].Preview, ccc );
					strcpy( c1, ".bmp" );
					FILE* F1 = fopen( ccc, "r " );
					if ( F1 )
					{
						strcpy( Pack[NMiss].Bitmap, ccc );
						fclose( F1 );
                    }
                }
				Gclose( F );
				NMiss++;
            }
		} while ( FindNextFile( H, &SR ) );
		FindClose( H );
    }
}

AddMissionsPack::~AddMissionsPack()
{
	if ( Pack )free( Pack );
    memset( this, 0, sizeof( AddMissionsPack) );
}

static bool SelectSingleMission()
{
	if ( !MISSLIST.MSMiss )return false;
	KeyPressed = 0;
	LastKey = 0;
	/*
	byte* Preview=new byte[(292*190)+4];
	((word*)Preview)[0]=292;
	((word*)Preview)[1]=190;
	bool PrPresent=false;
	char LastPrName[200]="";
	byte XLAT[256];
	ResFile fx=RReset("2\\0to1.xlt");
	RBlockRead(fx,XLAT,256);
	RClose(fx);
	*/

	//SlowUnLoadPalette("1\\agew_1.pal");
	AddMissionsPack AMSP;
	SQPicture Back( "Interface\\Single_Mission_Background.bmp" );

	LocalGP BTNS( "Interface\\Single_mission" );
	LocalGP OK1( "Interface\\anm_start_1_on" );
	LocalGP OK2( "Interface\\anm_start_1_off" );
	LocalGP CANCEL1( "Interface\\anm_cancel_on" );
	LocalGP CANCEL2( "Interface\\anm_cancel_off" );
	LocalGP ENC1( "Interface\\anm_enc_on" );
	LocalGP ENC2( "Interface\\anm_enc_off" );
	LocalGP SCROL( "Interface\\_Slider" );
	LocalGP MASKA( "Interface\\preview_mask" );
	LoadFog( 2 );

	/*
		LocalGP FONT("romw");
		RLCFont FontY(FONT.GPID);
		FontY.SetColorTable(3);
		LoadOptionalTable(3,"1\\orange.grd");
		RLCFont FontW(FONT.GPID);
		FontW.SetColorTable(4);
		LoadOptionalTable(4,"1\\white.grd");
	*/
	DialogsSystem MMenu( menu_x_off, menu_y_off );
	MMenu.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

	SQPicture PRV( "Maps2\\Single1.bmp" );
	BPXView* PREV = MMenu.addBPXView( nullptr, 495, 162, 292, 190, 1, 1, 1, (byte*) ( PRV.PicPtr + 2 )/*Preview+4*/, nullptr );
	//PREV->Visible=false;
    /*GPPicture* PMASK =*/ MMenu.addGPPicture( nullptr, 495, 162, MASKA.GPID, 0 );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;

	VScrollBar* SB = MMenu.addNewGP_VScrollBar( nullptr, 445, 157, 274, 1, 0, SCROL.GPID, 0 );
	ListBox* LB = MMenu.addGP_ListBox( nullptr, 41, 166, 10, BTNS.GPID, 8, 26, &WhiteFont, &YellowFont, SB );
	LB->M_Over = 11;
	LB->Active = 1;
	LB->FontDy = -2;
	LB->FontDx -= 1;
	ComboBox* DIFF = MMenu.addGP_ComboBox( nullptr, 584, 402, BTNS.GPID, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	DIFF->CurLine = 0;
	FILE* f = fopen( "smiss.opt", "r" );
	if ( f )
	{
		int v = 0;
		fscanf( f, "%d", &v );
		fclose( f );
		DIFF->CurLine = v;
    }
	DIFF->AddLine( GetTextByID( "@RMID_EASY" ) );
	DIFF->AddLine( GetTextByID( "@RMID_NORMAL" ) );
	DIFF->AddLine( GetTextByID( "@RMID_HARD" ) );
	DIFF->AddLine( GetTextByID( "@RMID_VERYHARD" ) );
	DIFF->Hint = GetTextByID( "DIFFICULTY" );
	TextViewer* TV[128];
	VScrollBar* TSB[128];
	for ( int i = 0; i < MISSLIST.MSMiss; i++ )
	{
		LB->AddItem( MISSLIST.MISS[MISSLIST.SingleMS[i]].Name, 0 );
		if ( MISSLIST.MISS[MISSLIST.SingleMS[i]].NIntro&&strstr( MISSLIST.MISS[MISSLIST.SingleMS[i]].Intro[0], ".txt" ) )
		{
			TV[i] = MMenu.addTextViewer( nullptr, 49, 478, 716, 160, MISSLIST.MISS[MISSLIST.SingleMS[i]].Intro[0], &YellowFont );
			TV[i]->SymSize = 21;
			TV[i]->PageSize = ( TV[i]->y1 - TV[i]->y ) / TV[i]->SymSize;
			TV[i]->Visible = 0;
			TSB[i] = MMenu.addNewGP_VScrollBar( nullptr, 777, 468, 161, 1, 0, SCROL.GPID, 0 );
			TSB[i]->Visible = 0;
			TSB[i]->ScrDy = 7;
			TV[i]->AssignScroll( TSB[i] );
		}
		else
		{
			TV[i] = nullptr;
			TSB[i] = nullptr;
        }
    }
	int N0 = MISSLIST.MSMiss;
	for ( int i = 0; i < AMSP.NMiss; i++ )
	{
		LB->AddItem( AMSP.Pack[i].Title, 0 );
		TV[N0 + i] = MMenu.addTextViewer( nullptr, 49, 478, 716, 160, AMSP.Pack[i].Preview, &YellowFont );
		TV[N0 + i]->SymSize = 21;
		TV[N0 + i]->PageSize = ( TV[i]->y1 - TV[i]->y ) / TV[i]->SymSize;
		TV[N0 + i]->Visible = 0;
		TSB[N0 + i] = MMenu.addNewGP_VScrollBar( nullptr, 777, 468, 161, 1, 0, SCROL.GPID, 0 );
		TSB[N0 + i]->Visible = 0;
		TSB[N0 + i]->ScrDy = 7;
		TV[N0 + i]->AssignScroll( TSB[i] );
    }
	VideoButton* OkBtn = MMenu.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
	OkBtn->OnUserClick = &MMItemChoose;
	OkBtn->UserParam = mcmOk;
	OkBtn->Hint = GetTextByID( "MISS_RUN" );
	VideoButton* CancelBtn = MMenu.addVideoButton( nullptr, 862, 668, CANCEL1.GPID, CANCEL2.GPID );
	CancelBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->Hint = GetTextByID( "MISS_CANCEL" );
	int prmiss = -1;
HHH1:
	ItemChoose = -1;
	int mm = 1;
	KeyPressed = 0;
	do
	{
		if ( prmiss != LB->CurItem )
		{
			//if(GetPreview(MISSLIST.MISS[MISSLIST.SingleMS[LB->CurItem]].MapName,Preview+4)){
			//	PREV->Visible=1;
			//	Decode(Preview+4,XLAT,292*190);
			//}else{
			//	PREV->Visible=false;
            //}
			if ( LB->CurItem >= N0 )
			{
				PRV.LoadPicture( AMSP.Pack[LB->CurItem - N0].Bitmap );
				PREV->Ptr = (byte*) ( PRV.PicPtr + 2 );
				prmiss = LB->CurItem;
			}
			else
			{
				char ccc[200];
				sprintf( ccc, "Maps2\\single%d.bmp", LB->CurItem + 1 );
				PRV.LoadPicture( ccc );
				PREV->Ptr = (byte*) ( PRV.PicPtr + 2 );
				prmiss = LB->CurItem;
            }
        }
		ProcessMessages();
		StdKeys();
		MMenu.MarkToDraw();
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
		for ( int j = 0; j < MISSLIST.MSMiss + AMSP.NMiss; j++ )
		{
			if ( TV[j] )
			{
				if ( j == LB->CurItem )
				{
					TV[j]->Visible = 1;
					TSB[j]->Visible = 1;
					TV[j]->Active = 0;
				}
				else
				{
					TV[j]->Visible = 0;
					TSB[j]->Visible = 0;
					TV[j]->Active = 0;
                }
            }
        }
		LB->Active = 1;
		if ( mm )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			mm = 0;
        }
		StdKeys();
	} while ( ItemChoose == -1 );
	f = fopen( "smiss.opt", "w" );
	if ( f )
	{
		fprintf( f, "%d", DIFF->CurLine );
		fclose( f );
    }
	if ( ItemChoose == mcmOk )
	{
		CurrentCampagin = -1;
		CurrentMission = -1;
		if ( LB->CurItem < N0 )
		{
			if ( !ProcessSingleMission( MISSLIST.SingleMS[LB->CurItem], DIFF->CurLine ) )
			{
				SlowUnLoadPalette( "2\\agew_1.pal" );
				CurrentCampagin = -1;
				CurrentMission = -1;
				goto HHH1;
			}
			else return true;
		}
		else
		{
			ShowLoading();
			ItemChoose = mcmSingle;
			strcpy( CurrentMap, AMSP.Pack[LB->CurItem - N0].Map );
			CreateNationalMaskForMap( CurrentMap );
			PrepareGameMedia( 0, 0 );
            for ( int i = 0; i < 7; i++ )
			{
				CITY[i].Difficulty = DIFF->CurLine;
				PINFO[i].ColorID = i;
				if ( i )strcpy( PINFO[i].name, "???" );
            }
			SetMyNation( 0 );
			strcpy( PINFO[0].name, PlName );
			PINFO[0].GameTime = 0;
			PINFO[0].VictCond = 0;
        }
	}
	else
	{
		CurrentCampagin = -1;
		CurrentMission = -1;
    }
	SlowUnLoadPalette( "2\\agew_1.pal" );
	return false;
}

static void ProcessCampagins( int Options )
{
	if ( !CAMPAGINS.NCamp )
	{
		return;
	}

	KeyPressed = 0;
	LastKey = 0;
	LoadFog( 2 );

	SQPicture Back( "Interface\\Background_Campaign_Menu.bmp" );
	LocalGP BTNS( "Interface\\Campaigns" );
	LocalGP SCROL( "Interface\\_Slider" );
	LocalGP OK1( "Interface\\anm_start_on" );
	LocalGP OK2( "Interface\\anm_start_off" );
	LocalGP CANCEL1( "Interface\\anm_cancel_on" );
	LocalGP CANCEL2( "Interface\\anm_cancel_off" );
	LocalGP ENC1( "Interface\\anm_enc_on" );
	LocalGP ENC2( "Interface\\anm_enc_off" );
	LocalGP FONT( "romw" );
	RLCFont FontY( FONT.GPID );
	FontY.SetColorTable( 3 );
	LoadOptionalTable( 3, "2\\orange.grd" );

	RLCFont FontW( FONT.GPID );
	FontW.SetWhiteColor();

	DialogsSystem MMenu( menu_x_off, menu_y_off );

	MMenu.OkSound = GetSound( "START" );
	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;

	MMenu.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

	VScrollBar* SB = MMenu.addNewGP_VScrollBar( nullptr, 755, 156, 119 + 26, 100, 0, SCROL.GPID, 0 );
	SB->ScrDy = 3;
	ListBox* LB = MMenu.addGP_ListBox( nullptr, 68, 166, 5, BTNS.GPID, 3, 26, &FontW, &FontY, SB );
	LB->FontDx--;
	LB->M_Over = 0;
	LB->Active = true;
	LB->FontDy -= 2;
	TextViewer* TV[32];
	VScrollBar* VS[32];

	for ( int i = 0; i < CAMPAGINS.NCamp; i++ )
	{
		LB->AddItem( CAMPAGINS.SCamp[i].CampMessage, 0 );
		VS[i] = MMenu.addNewGP_VScrollBar( nullptr, 755, 312 + 26, 316 - 26, 1, 0, SCROL.GPID, 0 );
		VS[i]->ScrDy = 13;
		TV[i] = MMenu.addTextViewer( nullptr, 77, 327 + 26 - 2, 665, 310 - 26, CAMPAGINS.SCamp[i].CampText, &FontY );
		TV[i]->AssignScroll( VS[i] );
		TV[i]->SymSize = 21;
		TV[i]->PageSize = ( TV[i]->y1 - TV[i]->y ) / TV[i]->SymSize;
		TV[i]->Enabled = false;
		TV[i]->Visible = false;
	}

	VideoButton* OkBtn = MMenu.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
	OkBtn->OnUserClick = &MMItemChoose;
	OkBtn->UserParam = mcmOk;
	OkBtn->Hint = GetTextByID( "WAR_RUN" );
	VideoButton* CancelBtn = MMenu.addVideoButton( nullptr, 862, 668, CANCEL1.GPID, CANCEL2.GPID );
	CancelBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->Hint = GetTextByID( "WAR_CANCEL" );

HHH1:
	ItemChoose = -1;
	KeyPressed = 0;
	if ( Options == -1 )
	{
		int mm = 1;
		do
		{
			ProcessMessages();
			StdKeys();
			MMenu.MarkToDraw();
			MMenu.ProcessDialogs();
			MMenu.RefreshView();
			for ( int i = 0; i < CAMPAGINS.NCamp; i++ )
			{
				TV[i]->Enabled = false;
				TV[i]->Visible = false;
				TV[i]->Active = false;
				VS[i]->Visible = false;
            }
			TV[LB->CurItem]->Enabled = 1;
			TV[LB->CurItem]->Visible = 1;
			if ( VS[LB->CurItem]->SMaxPos > 0 )VS[LB->CurItem]->Visible = 1;
			else VS[LB->CurItem]->Visible = 0;
			if ( mm )
			{
				SlowLoadPalette( "2\\agew_1.pal" );
				mm = 0;
            }
			StdKeys();
		} while ( ItemChoose == -1 );

		if ( ItemChoose == mcmOk )
		{
			Options = LB->CurItem;
		}
	}

	if ( Options != -1 )
	{
		SlowUnLoadPalette( "2\\agew_1.pal" );
		if ( !ProcessSingleCampagin( Options ) )
		{
			Options = -1;
			goto HHH1;
		}
	}
	else
	{
		SlowUnLoadPalette( "2\\agew_1.pal" );
	}
}

static bool ProcessOneBattle( int BtlID )
{
	KeyPressed = 0;
	LastKey = 0;
	SQPicture Back( "Interface\\Background_Historical_Battle_01.bmp" );
	LocalGP BTNS( "Interface\\Wars" );
	LocalGP Mask( "Interface\\Historical_mask" );
	LocalGP SCROL( "Interface\\_Slider" );
	LocalGP OK1( "Interface\\anm_start_on" );
	LocalGP OK2( "Interface\\anm_start_off" );
	LocalGP CANCEL1( "Interface\\anm_cancel_on" );
	LocalGP CANCEL2( "Interface\\anm_cancel_off" );
	LocalGP ENC1( "Interface\\anm_enc_on" );
	LocalGP ENC2( "Interface\\anm_enc_off" );
	LocalGP FONT( "romw" );
	RLCFont FontY( FONT.GPID );
	FontY.SetColorTable( 3 );
	LoadOptionalTable( 3, "2\\orange.grd" );
	SQPicture Preview( WARS.Battles[BtlID].BigMap );

	RLCFont FontW( FONT.GPID );
	FontW.SetWhiteColor();
	DialogsSystem MMenu( menu_x_off, menu_y_off );

	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;

	MMenu.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

	OneBattle* OB = WARS.Battles + BtlID;
	MMenu.addTextButton( nullptr, 48, 147 - 11, OB->BigHeader, &WhiteFont, &WhiteFont, &WhiteFont, 0 );
	if ( Preview.PicPtr )
	{
		MMenu.addPicture( nullptr, 39, 134 + 26, &Preview, &Preview, &Preview );
		MMenu.addGPPicture( nullptr, 39, 134 + 26, Mask.GPID, 0 );
		for ( int i = 0; i < OB->NHints; i++ )
		{
			MMenu.addTextButton( nullptr, 40 + OB->Coor[i + i], 133 + 26 + OB->Coor[i + i + 1], OB->Hints[i], &SmallBlackFont1, &SmallBlackFont1, &SmallBlackFont1, 0 );
			MMenu.addTextButton( nullptr, 38 + OB->Coor[i + i], 133 + 26 + OB->Coor[i + i + 1], OB->Hints[i], &SmallBlackFont1, &SmallBlackFont1, &SmallBlackFont1, 0 );
			MMenu.addTextButton( nullptr, 38 + OB->Coor[i + i], 135 + 26 + OB->Coor[i + i + 1], OB->Hints[i], &SmallBlackFont1, &SmallBlackFont1, &SmallBlackFont1, 0 );
			MMenu.addTextButton( nullptr, 40 + OB->Coor[i + i], 135 + 26 + OB->Coor[i + i + 1], OB->Hints[i], &SmallBlackFont1, &SmallBlackFont1, &SmallBlackFont1, 0 );
			MMenu.addTextButton( nullptr, 39 + OB->Coor[i + i], 134 + 26 + OB->Coor[i + i + 1], OB->Hints[i], &SmallWhiteFont1, &SmallWhiteFont1, &SmallWhiteFont1, 0 );
        }
    }
	VideoButton* OkBtn = MMenu.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
	OkBtn->OnUserClick = &MMItemChoose;
	OkBtn->UserParam = mcmOk;
	OkBtn->Hint = GetTextByID( "BTL_RUN" );
	VideoButton* CancelBtn = MMenu.addVideoButton( nullptr, 862, 668, CANCEL1.GPID, CANCEL2.GPID );
	CancelBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->Hint = GetTextByID( "BTL_CANCEL" );
	TextViewer* TV = MMenu.addTextViewer( nullptr, 50, 520 + 3, 720, 118, OB->Text, &YellowFont );
	TV->SymSize = 20;
	TV->PageSize = ( TV->y1 - TV->y ) / TV->SymSize;
	VScrollBar* SB = MMenu.addNewGP_VScrollBar( nullptr, 777, 511, 118, 0, 1, SCROL.GPID, 0 );
	TV->AssignScroll( SB );
	TV->Active = 1;

	ItemChoose = -1;
	int mm = 1;
	KeyPressed = 0;
	do
	{
		ProcessMessages();
		StdKeys();
		MMenu.MarkToDraw();
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
		if ( mm )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			mm = 0;
        }
		StdKeys();
	} while ( ItemChoose == -1 );
	SlowUnLoadPalette( "2\\agew_1.pal" );
	return ItemChoose == mcmOk;
}

static int ProcessWars()
{
	if ( !WARS.NWars )return false;
	KeyPressed = 0;
	LastKey = 0;
	SQPicture Back( "Interface\\Background_Historical_Wars.bmp" );
	LocalGP BTNS( "Interface\\Wars" );
	LocalGP OCAM( "Interface\\One_Campaign" );
	LocalGP SCROL( "Interface\\_Slider" );
	LocalGP OK1( "Interface\\anm_start_on" );
	LocalGP OK2( "Interface\\anm_start_off" );
	LocalGP CANCEL1( "Interface\\anm_cancel_on" );
	LocalGP CANCEL2( "Interface\\anm_cancel_off" );
	LocalGP ENC1( "Interface\\anm_enc_on" );
	LocalGP ENC2( "Interface\\anm_enc_off" );
	LocalGP FONT( "romw" );
	RLCFont FontY( FONT.GPID );
	FontY.SetColorTable( 3 );
	LoadOptionalTable( 3, "2\\orange.grd" );

	RLCFont FontW( FONT.GPID );
	FontW.SetWhiteColor();
	DialogsSystem MMenu( menu_x_off, menu_y_off );

	LocalGP HFONT( "rom10" );
	RLCFont hfnt( HFONT.GPID );
	hfnt.SetWhiteColor();
	MMenu.HintFont = &hfnt;
	MMenu.HintY = menu_hint_y;
	MMenu.HintX = menu_hint_x;

	MMenu.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

	VideoButton* OkBtn = MMenu.addVideoButton( nullptr, 862, 568, OK1.GPID, OK2.GPID );
	OkBtn->OnUserClick = &MMItemChoose;
	OkBtn->UserParam = mcmOk;
	OkBtn->Hint = GetTextByID( "WAR_RUN" );
	VideoButton* CancelBtn = MMenu.addVideoButton( nullptr, 862, 668, CANCEL1.GPID, CANCEL2.GPID );
	CancelBtn->OnUserClick = &MMItemChoose;
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->Hint = GetTextByID( "WAR_CANCEL" );

	for ( int i = 0; i < WARS.NWars&&i < 5; i++ )
		MMenu.addGPPicture( nullptr, 68, 166 + i * 26, OCAM.GPID, 6 );

	GPPicture* ZBTL[5];
	for ( int i = 0; i < 5; i++ )
	{
		ZBTL[i] = MMenu.addGPPicture( nullptr, 444, 166 + 26 * i, OCAM.GPID, 6 );
		ZBTL[i]->Visible = 0;
    }
	VScrollBar* CWS = MMenu.addNewGP_VScrollBar( nullptr, 395, 156, 146, 1, 0, SCROL.GPID, 0 );
	ListBox* CWar = MMenu.addGP_ListBox( nullptr, 68, 166, 5, BTNS.GPID, 6, 26, &WhiteFont, &YellowFont, CWS );
	CWar->FontDy -= 2;
	CWar->M_Over = 9;
	VScrollBar* CBS = MMenu.addNewGP_VScrollBar( nullptr, 755, 156, 146, 1, 0, SCROL.GPID, 0 );
	ListBox* CBat = MMenu.addGP_ListBox( nullptr, 444, 166, 5, BTNS.GPID, 0, 26, &WhiteFont, &YellowFont, CBS );
	CBat->FontDy -= 2;
	CBat->M_Over = 3;
	TextViewer* TV[32];
	VScrollBar* VS[32];
	for ( int i = 0; i < WARS.NWars; i++ )
	{
		CWar->AddItem( WARS.Wars[i].Name, 0 );
		TV[i] = MMenu.addTextViewer( nullptr, 78, 353, 663, 275, WARS.Wars[i].Text, &YellowFont );
		TV[i]->Visible = 0;
		TV[i]->Active = 0;
		TV[i]->Enabled = 0;
		TV[i]->SymSize = 19;
		TV[i]->PageSize = ( TV[i]->y1 - TV[i]->y ) / TV[i]->SymSize;
		TV[i]->Enabled = false;
		TV[i]->Visible = false;
		VS[i] = MMenu.addNewGP_VScrollBar( nullptr, 755, 338, 291, 1, 0, SCROL.GPID, 0 );
		TV[i]->AssignScroll( VS[i] );
		VS[i]->Visible = 0;
    }
	int CurWar = -1;
WStart:;
	int mm = 1;
	ItemChoose = -1;
	KeyPressed = 0;
	do
	{
		if ( CWar->CurItem != CurWar )
		{
			CBat->ClearItems();
			CurWar = CWar->CurItem;
			for ( int j = 0; j < WARS.Wars[CurWar].NBatles; j++ )
			{
				CBat->AddItem( WARS.Battles[WARS.Wars[CurWar].BattleList[j]].SmallHeader, 0 );
            }
        }
		ProcessMessages();
		StdKeys();
		MMenu.MarkToDraw();
		MMenu.ProcessDialogs();
		MMenu.RefreshView();
		for ( int i = 0; i < WARS.NWars; i++ )
		{
			TV[i]->Enabled = false;
			TV[i]->Visible = false;
			TV[i]->Active = false;
			VS[i]->Visible = false;
        }
		TV[CWar->CurItem]->Enabled = 1;
		TV[CWar->CurItem]->Visible = 1;
		TV[CWar->CurItem]->Active = 1;
		VS[CWar->CurItem]->Visible = VS[CWar->CurItem]->SMaxPos > 0;
		int NBTL = WARS.Wars[CWar->CurItem].NBatles;
		for ( int i = 0; i < 5; i++ )ZBTL[i]->Visible = i < NBTL;
		if ( mm )
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			mm = 0;
        }
		StdKeys();
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
		SlowUnLoadPalette( "2\\agew_1.pal" );
		if ( !ProcessOneBattle( WARS.Wars[CWar->CurItem].BattleList[CBat->CurItem] ) )
		{
			//SlowUnLoadPalette("2\\agew_1.pal");
			goto WStart;
        }
		return WARS.Wars[CWar->CurItem].BattleList[CBat->CurItem];
    }
	SlowUnLoadPalette( "2\\agew_1.pal" );
	return -1;
}

static void SlideShow()
{
	GFILE* f = Gopen( "Slides\\pic.lst", "r" );
	if ( f )
	{
		char cc[128];
		int z;
		do
		{
			z = Gscanf( f, "%s", cc );
			if ( z == 1 )
			{
				ResFile F = RReset( cc );
				if ( F != INVALID_HANDLE_VALUE )
				{
					RClose( F );
					DialogsSystem DSS( menu_x_off, menu_y_off );
					SQPicture PIC;
					PIC.LoadPicture( cc );
					DSS.addPicture( nullptr, 0, 0, &PIC, &PIC, &PIC );
					Lpressed = 0;
					KeyPressed = 0;
					PIC.Draw( 0, 0 );
					FlipPages();
					SlowLoadPalette( "2\\agew_1.pal" );
					do
					{
						DSS.ProcessDialogs();
						DSS.RefreshView();
						ProcessMessages();
					} while ( !( KeyPressed || Lpressed ) );
					SlowUnLoadPalette( "2\\agew_1.pal" );
                }
            }
		} while ( z == 1 );
    }
}

static int ShowStatScreen( bool Next, bool Prev, byte Kind );
static int ShowUserStat( bool Prev, bool Next );
static int LPAL = 1;

static void PrintNum( int N, char* res )
{
	char tmp[32];
	int ps = 0;
	while ( N != 0 )
	{
		int v = N % 10;
		N = N / 10;
		tmp[ps] = '0' + v;
		ps++;
		if ( ( ps == 3 || ps == 7 || ps == 11 || ps == 15 ) && N != 0 )
		{
			tmp[ps] = '.';
			ps++;
        }
    }
	for ( int j = 0; j < ps; j++ )res[j] = tmp[ps - j - 1];
	res[ps] = 0;
}

//Draws post game stats screen
bool ShowStatistics()
{
	curptr = 0;

	int res = 0;
	int cur = 0;
	LPAL = 1;

	do
	{
		switch ( cur )
		{
		case 0:
			res = ShowStatScreen( true, false, 0 );
			break;
		case 1:
			res = ShowStatScreen( true, true, 1 );
			break;
		case 2:
			res = ShowUserStat( true, false );
			break;
		}

		if ( res == 1 )
			cur--;

		if ( res == 4 )
			cur++;

		cur %= 3;

	} while ( res != 2 && res != 3 && res != 131 );
	return res == 3;
}

static void CreateCommName( byte Nat, char* Res )
{
	Res[0] = 0;
	for ( int i = 0; i < 7; i++ )
	{
		if ( NatRefTBL[i] == Nat )
		{
			if ( Res[0] )strcat( Res, "+" );
			strcat( Res, GetPName( i ) );
        }
    }
}

static int stats_x_off, stats_y_off;
static void CalculateStatsScreenOffsets( int stats_width )
{
	if ( window_mode )
	{//Show in top center of window
		stats_x_off = RealLx / 2 - stats_width / 2;
		stats_y_off = 25;
	}
	else
	{//Fullscreen, center stats
		stats_x_off = ( RealLx - 1024 ) / 2;
		stats_y_off = ( RealLy - 768 ) / 2;
	}
	if ( 768 == RealLy )
	{//Smallest resolution, don't offset stats vertically or the buttons won't fit
		stats_y_off = 0;
	}
}

static int ShowStatScreen( bool Next, bool Prev, byte Kind )
{
	bool ExStyle = ( NPlayers < 2 && !PlayGameMode );

	int x = 9;
	int y = 13;
	int Lx = 1006;
	int Ly = 690;

	int NNAT = 0;
	int NATS[7];
	int MaxPop = 0;
	int MaxUpgTime = 0;
	int MaxGrTime = 0;
	int ODY = 24;

	LocalGP BTNS;
	if ( ExStyle )
	{
		BTNS.Load( "Interface\\Statistic_Screen" );
	}
	else
	{
		BTNS.Load( "Interface\\Statistic_ScreenX" );
	}

	y += ODY;
	Ly -= ODY;
	word* Val[8];
	int NVal[8];
    const char* Header;

	if ( Kind )
	{//Score timeline
		for ( int k = 0; k < 7; k++ )
		{
			Val[k] = NATIONS[k].Account;
			NVal[k] = NATIONS[k].NAccount;
		}
		Header = GetTextByID( "ST_ACC" );
	}
	else
	{//Population and upgrades timeline
		for ( int k = 0; k < 7; k++ )
		{
			Val[k] = NATIONS[k].Popul;
			NVal[k] = NATIONS[k].NPopul;
		}
		Header = GetTextByID( "ST_MAX" );
	}

	for ( int i = 0; i < 7; i++ )
	{
		int max = 0;
		Nation* NT = NATIONS + i;
		int N = NVal[i];
		word* VAL = Val[i];
		MaxGrTime = N << 8;

		for ( int j = 0; j < N; j++ )
		{
			if ( VAL[j] > max )
			{
				max = VAL[j];
			}
		}

		if ( max )
		{
			NATS[NNAT] = i;
			NNAT++;
			if ( max > MaxPop )
				MaxPop = max;
		}

		N = NT->NUpgMade;

		if ( N && NT->UpgTime[N - 1] > MaxUpgTime )
		{
			MaxUpgTime = NT->UpgTime[N - 1];
		}
	}

	int sdx = 3;
	int slx = Lx - sdx * 2;

	MaxUpgTime += ( 40 * MaxUpgTime ) / ( slx - 40 );

	if ( MaxGrTime > MaxUpgTime )
	{
		MaxUpgTime = MaxGrTime;
	}

	int LXX[8];
    int YYI[8];
	if ( MaxPop < 5 )
	{
		MaxPop = 5;
	}

	if ( NNAT )
	{
		int OneLy = 59 + 4;
		int OneDy = 8 + 2;
		int GrLy = Ly - NNAT * OneLy;
		int GrpX = 20;
		int GrpY = 20;
		int GrpLx = Lx - 2 * GrpX;
		int GrpLy = GrLy - 2 * GrpY;

		int deal = ( MaxPop * 40 ) / GrpLy;
		if ( deal < 5 )deal = 5;
		else if ( deal < 10 )deal = 10;
		else if ( deal < 15 )deal = 15;
		else if ( deal < 20 )deal = 20;
		else if ( deal < 25 )deal = 25;
		else if ( deal < 40 )deal = 40;
		else if ( deal < 50 )deal = 50;
		else if ( deal < 75 )deal = 75;
		else if ( deal < 100 )deal = 100;
		else if ( deal < 150 )deal = 150;
		else if ( deal < 200 )deal = 200;
		else if ( deal < 250 )deal = 250;
		else if ( deal < 300 )deal = 300;
		else if ( deal < 400 )deal = 400;
		else if ( deal < 500 )deal = 500;
		else deal = 750;

		int NGridY = ( MaxPop / deal ) + 1;
		MaxPop = NGridY*deal;

		CalculateStatsScreenOffsets( Lx );

		DialogsSystem STAT( stats_x_off, stats_y_off );

		int DDX = 110;
		if ( !ExStyle )
		{
			DDX = 0;
		}

		if ( ExStyle )
		{
			GP_TextButton* BT_REPLAY = STAT.addGP_TextButton( nullptr, 415, 723, GetTextByID( "REPLAY" ), BTNS.GPID, 9, &WhiteFont, &YellowFont );
			BT_REPLAY->OnUserClick = &MMItemChoose;
			BT_REPLAY->UserParam = 131;
		}

		GP_TextButton* RGAME = STAT.addGP_TextButton( nullptr, 263 - DDX, 723, GetTextByID( "ST_GAM" ), BTNS.GPID, 0, &WhiteFont, &YellowFont );
		RGAME->OnUserClick = &MMItemChoose;
		RGAME->UserParam = 2;

		GP_TextButton* RMENU;

		if ( PlayGameMode )
		{
			RMENU = STAT.addGP_TextButton( nullptr, 526 + DDX, 723, GetTextByID( "ST_MEN" ), BTNS.GPID, 0, &YellowFont, &YellowFont );
			RMENU->Enabled = 0;
		}
		else
		{
			RMENU = STAT.addGP_TextButton( nullptr, 526 + DDX, 723, GetTextByID( "ST_MEN" ), BTNS.GPID, 0, &YellowFont, &WhiteFont );
		}

		RMENU->OnUserClick = &MMItemChoose;
		RMENU->UserParam = 3;

		GP_Button* BACK = STAT.addGP_Button( nullptr, 196 - DDX, 723, BTNS.GPID, 8, 6 );
		BACK->OnUserClick = &MMItemChoose;
		BACK->UserParam = 1;
		BACK->Enabled = Prev;
		BACK->Visible = Prev;

		GP_Button* NEXT = STAT.addGP_Button( nullptr, 788 + DDX, 723, BTNS.GPID, 7, 5 );
		NEXT->OnUserClick = &MMItemChoose;
		NEXT->UserParam = 4;
		NEXT->Enabled = Next;
		NEXT->Visible = Next;

		STAT.HintFont = &SmallBlackFont1;
		STAT.HintX = -1;
		STAT.HintY = 250;
		char ccc[100];

		STAT.addTextButton( nullptr, x + 14, y - 17, Header, &WhiteFont, &WhiteFont, &WhiteFont, 0 );

		int yy = y + GrLy;
		int xp = -1000;

		for ( int i = 0; i < NNAT; i++ )
		{
			xp = -1000;
			Nation* NT = NATIONS + NATS[i];
			char commname[128];
			CreateCommName( NATS[i], commname );

			LXX[i] = GetRLCStrWidth( commname, &SmallWhiteFont1 ) + 40;

			int dx = x + GrpX + ( GrpLx - LXX[i] ) / 2;

			STAT.addTextButton( nullptr, dx + 33, yy - 8, commname, &SmallWhiteFont1, &SmallWhiteFont1, &SmallWhiteFont1, 0 );

			STAT.addColoredBar( dx + 6, yy - 2, 24, 1, 0xD0 + NATS[i] * 4 );

            YYI[i] = yy;
			int N = NT->NUpgMade;

			for ( int j = 0; j < N; j++ )
			{
				int xx = x + 3 + ( NT->UpgTime[j] * slx ) / MaxUpgTime;
				if ( xx < xp )
				{
					xx = xp + 2;
				}
				xp = xx;
				NewUpgrade* NU = NT->UPGRADE[NT->UpgIDS[j]];

				if ( NU->Level < 2 && NU->CtgUpgrade != 23 )
				{
					SimpleDialog* SD;
					if ( NU->IconFileID != 0xFFFF )
					{
						SD = STAT.addGPPicture( nullptr, xx, yy + OneDy, NU->IconFileID, NU->IconSpriteID );
					}
					else
					{
						SD = STAT.addColoredBar( xx, yy + OneDy, 40, 40, 0 );
					}

					if ( SD )
					{
						SD->Hint = NU->Message;
					}

					ColoredBar* CB = STAT.addColoredBar( xx, yy + OneDy, 40, 40, 0xD0 + NATS[i] * 4 );

					if ( CB )
					{
						CB->Style = 1;
					}

					if ( SD )
					{
						SD->Child = CB;
					}
				}
			}
			yy += OneLy;
		}

		ItemChoose = -1;
		KeyPressed = 0;
		Lpressed = 0;

		do
		{
			//Backgrounds for UI elements
			GPS.ShowGP( stats_x_off, stats_y_off, BTNS.GPID, 2, 0 );

			//Stat table background
			DrawStatTable( x + stats_x_off, y - ODY + stats_y_off, Lx, GrLy + ODY, OneLy, NNAT );

			//Draw boxes with player names for update timelines
			for ( int i = 0; i < NNAT; i++ )
			{
				DrawStHeader( GrpX + x + GrpLx / 2 + stats_x_off, YYI[i] + stats_y_off, LXX[i] );
			}

			//Draw borders around timeline table
			int gx0 = GrpX + x + stats_x_off;
			int gx1 = GrpX + x + GrpLx - 1 + stats_x_off;
			int gy0 = GrpY + y + stats_y_off;
			int gy1 = GrpY + y + GrpLy - 1 + stats_y_off;
			Xbar( gx0 - 1, gy0 - 1, gx1 - gx0 + 3, gy1 - gy0 + 3, 0x3B );
			Xbar( gx0 - 2, gy0 - 2, gx1 - gx0 + 5, gy1 - gy0 + 5, 0x3A );
			Xbar( gx0 - 3, gy0 - 3, gx1 - gx0 + 7, gy1 - gy0 + 7, 0x39 );
			Xbar( gx0 - 4, gy0 - 4, gx1 - gx0 + 9, gy1 - gy0 + 9, 0xAE );
			Xbar( gx0 - 5, gy0 - 5, gx1 - gx0 + 11, gy1 - gy0 + 11, 0xAE );
			Xbar( gx0 - 6, gy0 - 6, gx1 - gx0 + 13, gy1 - gy0 + 13, 0xAE );

			//Draw grids for timelines
			for ( int ix = 0; ix < 8; ix++ )
			{
				int gx0 = x + GrpX + ( ( GrpLx*ix ) >> 3 ) + stats_x_off;
				int gx1 = x + GrpX + ( ( GrpLx*( ix + 1 ) ) >> 3 ) + stats_x_off;
				for ( int iy = 0; iy < NGridY; iy++ )
				{
					int gy0 = y + GrpY + ( ( GrpLy*iy ) / NGridY ) + stats_y_off;
					int gy1 = y + GrpY + ( ( GrpLy*( iy + 1 ) ) / NGridY ) + stats_y_off;
					Xbar( gx0, gy0, gx1 - gx0 + 1, gy1 - gy0 + 1, 0x3E );
					Xbar( gx0 + 1, gy0 + 1, gx1 - gx0 - 1, gy1 - gy0 - 1, 0x3B );
					Xbar( gx0 + 2, gy0 + 2, gx1 - gx0 - 3, gy1 - gy0 - 3, 0x3A );
					Xbar( gx0 + 3, gy0 + 3, gx1 - gx0 - 5, gy1 - gy0 - 5, 0x39 );
					Xbar( gx0 + 4, gy0 + 4, gx1 - gx0 - 7, gy1 - gy0 - 7, 0xAE );
					Xbar( gx0 + 5, gy0 + 5, gx1 - gx0 - 9, gy1 - gy0 - 9, 0xAE );
					Xbar( gx0 + 6, gy0 + 6, gx1 - gx0 - 11, gy1 - gy0 - 11, 0xAE );
					if ( ix == 0 )
					{
						sprintf( ccc, "%d", ( NGridY - iy - 1 )*deal );
						ShowString( gx0 + 3, gy1 - 14, ccc, &SmallBlackFont );
					}
				}
			}

			int X0 = 0;
			int Y0 = 0;
			int X1 = 0;
			int Y1 = 0;

			//Plot timelines
			for ( int i = 0; i < NNAT; i++ )
			{
                byte c = NATS[i] * 4 + 0xD0;
				int N = NVal[NATS[i]];
				word* VAL = Val[NATS[i]];

				for ( int j = 0; j < N; j++ )
				{
					if ( j > 0 )
					{
						X0 = X1;
						Y0 = Y1;
					}
					X1 = x + GrpX + ( ( j << 8 )*GrpLx ) / MaxUpgTime + stats_x_off;
					Y1 = y + GrpY + GrpLy - ( VAL[j] * ( GrpLy - 10 ) ) / MaxPop + stats_y_off;

					if ( j > 0 )
					{
						DrawLine( X0, Y0, X1, Y1, c );
						DrawLine( X0 + 1, Y0, X1 + 1, Y1, c );
						DrawLine( X0 + 1, Y0 + 1, X1 + 1, Y1 + 1, c );
						DrawLine( X0, Y0 + 1, X1, Y1 + 1, c );
					}
				}
			}

			ProcessMessages();

			STAT.MarkToDraw();

			STAT.ProcessDialogs();

			STAT.RefreshView();

			if ( LPAL )
			{
				SlowLoadPalette( "0\\agew_1.pal" );
				LPAL = 0;
			}

		} while ( ItemChoose == -1 );
	}
	else
	{
		return 3;
	}

	KeyPressed = false;
	Lpressed = 0;

	UnPress();

	return ItemChoose;
}

//Draws page 3 of stats screen
static int ShowUserStat( bool Prev, bool Next )
{
	bool ExStyle = ( NPlayers < 2 && !PlayGameMode );

	int x = 9;
	int y = 13;
	int Lx = 1006;
	int Ly = 690;

	LocalGP BTNS;
	if ( ExStyle )
	{
		BTNS.Load( "Interface\\Statistic_Screen" );
	}
	else
	{
		BTNS.Load( "Interface\\Statistic_ScreenX" );
	}

	CalculateStatsScreenOffsets( Lx );

	DialogsSystem STAT( stats_x_off, stats_y_off );

	int DDX = 110;
	if ( !ExStyle )
		DDX = 0;

	if ( ExStyle )
	{
		GP_TextButton* BT_REPLAY = STAT.addGP_TextButton( nullptr, 415, 723, GetTextByID( "REPLAY" ), BTNS.GPID, 9, &WhiteFont, &YellowFont );
		BT_REPLAY->OnUserClick = &MMItemChoose;
		BT_REPLAY->UserParam = 131;
	}

	GP_TextButton* RGAME = STAT.addGP_TextButton( nullptr, 263 - DDX, 723, GetTextByID( "ST_GAM" ), BTNS.GPID, 0, &WhiteFont, &YellowFont );
	RGAME->OnUserClick = &MMItemChoose;
	RGAME->UserParam = 2;

	GP_TextButton* RMENU;
	if ( PlayGameMode )
	{
		RMENU = STAT.addGP_TextButton( nullptr, 526 + DDX, 723, GetTextByID( "ST_MEN" ), BTNS.GPID, 0, &YellowFont, &YellowFont );
		RMENU->Enabled = 0;
	}
	else
	{
		RMENU = STAT.addGP_TextButton( nullptr, 526 + DDX, 723, GetTextByID( "ST_MEN" ), BTNS.GPID, 0, &WhiteFont, &YellowFont );
	}
	RMENU->OnUserClick = &MMItemChoose;
	RMENU->UserParam = 3;

	GP_Button* BACK = STAT.addGP_Button( nullptr, 196 - DDX, 723, BTNS.GPID, 8, 6 );
	BACK->OnUserClick = &MMItemChoose;
	BACK->UserParam = 1;
	BACK->Enabled = Prev;
	BACK->Visible = Prev;

	GP_Button* NEXT = STAT.addGP_Button( nullptr, 788 + DDX, 723, BTNS.GPID, 7, 5 );
	NEXT->OnUserClick = &MMItemChoose;
	NEXT->UserParam = 4;
	NEXT->Enabled = Next;
	NEXT->Visible = Next;
	ListBox* LB = STAT.addListBox( nullptr, x + 25, y + 35, 30, 180, 26, &WhiteFont, &YellowFont, nullptr );
	byte NATS[8];
	int NNAT = 0;
	for ( int i = 0; i < 7; i++ )
	{
		int max = 0;
		Nation* NT = NATIONS + i;
		int N = NT->NPopul;
		word* VAL = NT->Popul;
		for ( int j = 0; j < N; j++ )
		{
			if ( VAL[j] > max )
			{
				max = VAL[j];
			}
		}
		if ( max )
		{
			NATS[NNAT] = i;
			NNAT++;
			char commname[256];
			CreateCommName( i, commname );
			LimitString( commname, &YellowFont, 200 );
			LB->AddItem( commname, i );
        }
    }
	LB->CurItem = 0;
	ItemChoose = -1;
    const char* hdrs[8];

	//Resource statistics strings
	hdrs[0] = GetTextByID( "ST_TOTAL" );
	hdrs[1] = GetTextByID( "ST_UPGR" );
	hdrs[2] = GetTextByID( "ST_MINE" );
	hdrs[3] = GetTextByID( "ST_UNITS" );
	hdrs[4] = GetTextByID( "ST_BLD" );
	hdrs[5] = GetTextByID( "ST_LIFE" );
	hdrs[6] = GetTextByID( "ST_BUY" );
	hdrs[7] = GetTextByID( "ST_SELL" );

    const char* STHDR = GetTextByID( "ST_UHDR" );//Statistics
    const char* KHDR = GetTextByID( "ST_KILL" );//List of units lost
    const char* PHDR = GetTextByID( "ST_PROD" );//List of units created

	int curnat = 0;

	int ux = 230 + stats_x_off;
	int uy = 320 + stats_y_off;
	int ulx = 353;
	int uly = 373;
	int kx = ux + ulx + 45;

	//Extra values for the two scrollbars because this ui is like a cardhouse >_<
	int ux_scroll = 230;
	int uy_scroll = 320;
	int kx_scroll = ux_scroll + ulx + 45;
	VScrollBar* PBR = STAT.addNewGP_VScrollBar( nullptr,
		ux_scroll + ulx + 11, uy_scroll + 1,
		uly - 2, 1, 0, 3, 0 );
	VScrollBar* KBR = STAT.addNewGP_VScrollBar( nullptr,
		kx_scroll + ulx + 11, uy_scroll + 1,
		uly - 2, 1, 0, 3, 0 );

	PBR->ScrDy = 200;
	KBR->ScrDy = 200;
	do
	{
		curnat = LB->CurItem;
		int cnt = NATS[curnat];
		int* ResSrc = (int*) ( &NATIONS[cnt].ResTotal );

		//Backgrounds for UI elements
		GPS.ShowGP( stats_x_off, stats_y_off, BTNS.GPID, 2, 0 );

		//Stat table background
		DrawStatTable1( x + stats_x_off, y + stats_y_off,
			x + Lx - 1 + stats_x_off, y + Ly - 1 + stats_y_off,
			x + 200 + stats_x_off );

		for ( int i = 0; i < LB->NItems; i++ )
		{
			CBar( LB->x - 15, LB->y + 15 + i * 25, 12, 3, 0xD0 + NatRefTBL[NATS[i]] * 4 );
		}

		ShowString( x + 10 + stats_x_off, y + 8 + stats_y_off, STHDR, &YellowFont );
		ShowString( ux + ( ulx / 2 ) + 7 - ( GetRLCStrWidth( PHDR, &YellowFont ) / 2 ), uy - 22, PHDR, &YellowFont );
		ShowString( kx + ( ulx / 2 ) + 7 - ( GetRLCStrWidth( KHDR, &YellowFont ) / 2 ), uy - 22, KHDR, &YellowFont );

		int rx = 220 + stats_x_off;
		int ry = y + 40 + stats_y_off;
		int rx1 = rx + 219 + 5;
		int rxl = 80;
		for ( int q = 0; q < 6; q++ )
		{
			ShowString( rx1 + rxl*q + rxl + ( rxl / 2 ) - ( GetRLCStrWidth( RDS[q].Name, &YellowFont ) / 2 ), ry + 6, RDS[q].Name, &YellowFont );
		}

		Hline( rx, ry, rx1 + rxl * 7, 70 );
		ry += 26;
		Vline( rx, ry - 26, ry + 8 * 26, 70 );
		for ( int q = 0; q < 8; q++ )
		{
			Hline( rx, ry, rx1 + rxl * 7, 70 );
			ShowString( rx + 15, ry + 6, hdrs[q], &YellowFont );
			for ( int j = 0; j < 6; j++ )
			{
				char cc1[32];
				PrintNum( ResSrc[q * 8 + j], cc1 );
				ShowString( rx1 + rxl*j + rxl + ( rxl / 2 ) - ( GetRLCStrWidth( cc1, &YellowFont ) / 2 ), ry + 6, cc1, &YellowFont );
				if ( q == 0 )
				{
					Vline( rx1 + rxl*j + rxl, ry - 26, ry + 8 * 26, 70 );
				}
			}
			if ( q == 0 )
			{
				Vline( rx1 + rxl * 7, ry - 26, ry + 8 * 26, 70 );
			}
			ry += 26;
		}
		Hline( rx, ry, rx1 + rxl * 7, 70 );
		TempWindow TMW;
		PushWindow( &TMW );
		Xbar( ux - 10, uy, ulx + 25 + 10, uly, 70 );
		IntersectWindows( ux - 8, uy + 1, ux + ulx + 15, uy + uly - 2 );
		int y0 = uy - PBR->SPos;
		int yfin = ShowUnitsList( ux, y0, ulx, NATIONS[cnt].NProduced );
		int mps = yfin - uly - 2;
		if ( mps > 0 )
		{
			PBR->Visible = 1;
			PBR->Enabled = 1;
			PBR->SMaxPos = mps;
		}
		else
		{
			PBR->Visible = 0;
			PBR->Enabled = 0;
			PBR->SPos = 0;
		}

		PopWindow( &TMW );

		Xbar( kx - 10, uy, ulx + 25 + 10, uly, 70 );

		IntersectWindows( kx - 8, uy + 1, kx + ulx + 15, uy + uly - 2 );

		y0 = uy - KBR->SPos;

		yfin = ShowUnitsList( kx, y0, ulx, NATIONS[cnt].NKilled );

		mps = yfin - uly - 2;
		if ( mps > 0 )
		{
			KBR->Visible = 1;
			KBR->Enabled = 1;
			KBR->SMaxPos = mps;
		}
		else
		{
			KBR->Visible = 0;
			KBR->Enabled = 0;
			KBR->SPos = 0;
		}

		PopWindow( &TMW );

		ProcessMessages();

		STAT.MarkToDraw();

		STAT.ProcessDialogs();

		STAT.RefreshView();
	} while ( ItemChoose == -1 );

	KeyPressed = false;
	Lpressed = 0;

	UnPress();

	return ItemChoose;
}

void CenterScreen()
{
	int xc = 0;
	int yc = 0;
	int N = 0;
	int mnt = NatRefTBL[MyNation];
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == mnt )
		{
			xc += OB->RealX >> 4;
			yc += OB->RealY >> 4;
			N++;
		}
	}

	if ( N )
	{
		int x = xc / N;
		int y = yc / N;
		x = ( x - ( RealLx >> 1 ) ) >> 5;
		y = ( y - RealLy ) >> 5;
		if ( x < 1 )x = 1;
		if ( y < 1 )y = 1;
		if ( x + smaplx >= msx - 1 )x = msx - smaplx - 1;
		if ( y + smaply >= msy - 1 )y = msy - smaply - 1;
		mapx = x;
		mapy = y;
	}
}

static char ChatString[128];
static wchar_t unicode_chat_string[128];
static size_t ChatCursPos = 0;
static wchar_t last_unicode = 0;

void ProcessChatKeys()
{
KRT:
	if ( !NKeys )
	{
		return;
	}

	LastKey = ReadKey();
	KeyPressed = 0;

	if ( 27 == LastKey )
	{//Escape key
		EnterChatMode = 0;
		ChatString[0] = 0;
		return;
	}

	if ( 13 == LastKey )
	{//Enter key
		HandleSMSChat( ChatString );

		EnterChatMode = 0;

		CreateTimedHintEx( ChatString, kChatMessageDisplayTime, 16 + NatRefTBL[MyNation] );//Chat message

		bool ally = ( ( GetKeyState( VK_CONTROL ) & 0x8000 ) != 0 );
		SendChat( ChatString, ally );
		ChatCursPos = 0;

		if ( NPlayers < 2 )
		{//IMPORTANT: Handling cheats
			if ( !strcmp( ChatString, "www" ) )
			{
				ChangeNation = 1;
				MultiTvar = 1;
				FogMode = 0;
			}

			if ( !strcmp( ChatString, "victory" ) )
			{
				ShowVictory();
			}

			if ( !strcmp( ChatString, "izmena" ) )
			{
				ChangeNation = !ChangeNation;
			}

			if ( !strcmp( ChatString, "multitvar" ) )
			{
				MultiTvar = !MultiTvar;
			}

			if ( !strcmp( ChatString, "supervizor" ) )
			{
				FogMode = !FogMode;
			}

			if ( !strcmp( ChatString, "money" ) )
			{
				CmdMoney( MyNation );
			}
			else
			{
				char cc1[64];
				int n1, n2;
				int z = sscanf( ChatString, "%s%d%d", cc1, &n1, &n2 );
				if ( z == 3 && !strcmp( cc1, "change" ) )
				{
					if ( n1 >= 0 && n1 < 8 && n2 >= 0 && n2 <= 8 )
					{
						SelChangeNation( n1, n2 );
					}
				}

				char cc2[128];
				z = sscanf( ChatString, "%s%s", cc1, cc2 );
				if ( z == 2 && !strcmp( cc1, "changeunit" ) )
				{
                    for ( int i = 0; i < NATIONS->NMon; i++ )
					{
						if ( !strcmp( cc2, NATIONS->Mon[i]->MonsterID ) )
						{
							GeneralObject* GO = NATIONS->Mon[i];
							if ( !GO->newMons->Building )
							{
								//found!!!
								int N = NSL[MyNation];
								word* IDS = Selm[MyNation];
                                for ( int j = 0; j < N; j++ )
								{
									word MID = IDS[j];
									if ( MID != 0xFFFF )
									{
										OneObject* OB = Group[MID];
										if ( OB && !OB->Sdoxlo )
										{
											OB->Ref.General = GO;
											OB->NIndex = i;
											OB->newMons = GO->newMons;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if ( ChatString[0] == 'g' && ChatString[1] == 'i' && ChatString[2] == 'v'
			&& ChatString[3] == 'e' && ChatString[4] == ' ' )
		{
			char* sss = ChatString + 5;
			char cc1[100];
			int p = 0;
			char c;
			while ( ' ' == sss[0] )
			{
				sss++;
			}

			do
			{
				c = sss[0];
				cc1[p] = c;
				if ( ' ' != c )
				{
					p++;
				}
				sss++;
			} while ( 0 != c && ' ' != c && 20 > p );

			cc1[p] = 0;
			if ( c == 0 )
			{
				return;
			}

			int res = -1;
			if ( !strcmp( cc1, "wood" ) )res = 0;
			else if ( !strcmp( cc1, "gold" ) )res = 1;
			else if ( !strcmp( cc1, "stone" ) )res = 2;
			else if ( !strcmp( cc1, "food" ) )res = 3;
			else if ( !strcmp( cc1, "iron" ) )res = 4;
			else if ( !strcmp( cc1, "coal" ) )res = 5;
			if ( res == -1 )
			{
				return;
			}

			while ( sss[0] == ' ' )
			{
				sss++;
			}
			p = 0;
			do
			{
				c = sss[0];
				cc1[p] = c;
				if ( c >= '0'&&c <= '9' )c = 0;
				cc1[p] = c;
				if ( c )
				{
					sss++;
					p++;
				}
			} while ( c != 0 && p < 40 );

			int L;
			do
			{
				L = strlen( cc1 );
				if ( L && cc1[L - 1] == ' ' )
				{
					cc1[L - 1] = 0;
				}
				else
				{
					L = 0;
				}
			} while ( L );

			int Him = -1;
			for ( int i = 0; i < NPlayers; i++ )
			{
				if ( !strcmp( cc1, PINFO[i].name ) )
				{
					Him = PINFO[i].ColorID;
				}
			}

			if ( Him == -1 )
			{
				return;
			}

			while ( sss[0] == ' ' )
			{
				sss++;
			}

			int Amn;
			int z = sscanf( sss, "%d", &Amn );
			if ( z != 1 )
			{
				return;
			}

			CmdGiveMoney( MyNation, Him, res, Amn );
		}
		ChatString[0] = 0;
		return;
	}

	if ( LastKey >= VK_F1 && LastKey < VK_F9 )
	{
		if ( CheckFNSend( LastKey - VK_F1 ) )
		{
			EnterChatMode = 0;
			NKeys = 0;
			return;
		}
	}

	if ( LastKey == VK_LEFT )
	{
		if ( ChatCursPos > 0 )
		{
			ChatCursPos--;
		}
		KeyPressed = 0;
		LastKey = 0;
		goto KRT;
	}
	else
		if ( LastKey == VK_RIGHT )
		{
			if ( ChatCursPos < strlen( ChatString ) )ChatCursPos++;
			KeyPressed = 0;
			LastKey = 0;
			goto KRT;
		}
		else
			if ( LastKey == VK_END )
			{
				ChatCursPos = strlen( ChatString );
				KeyPressed = 0;
				LastKey = 0;
				goto KRT;
			}
			else
				if ( LastKey == VK_HOME )
				{
					ChatCursPos = 0;
					KeyPressed = 0;
					LastKey = 0;
					goto KRT;
				}
				else
					if ( LastKey == VK_BACK )
					{
						if ( ChatCursPos > 0 )
						{
							strcpy( ChatString + ChatCursPos - 1, ChatString + ChatCursPos );
							ChatCursPos--;
                        }
						goto KRT;
					}
					else
						if ( 46 == LastKey )
						{//VK_DEL
							if ( ChatCursPos < strlen( ChatString ) )
							{
								strcpy( ChatString + ChatCursPos, ChatString + ChatCursPos + 1 );
							}
							goto KRT;
						}
						else
						{
							if ( last_unicode )
							{
								wchar_t one_char_str[2];
								one_char_str[0] = last_unicode;
								one_char_str[1] = 0;
								if ( 120 > wcslen( unicode_chat_string ) + sizeof( wchar_t ) )
								{
									wchar_t temp[2048];
									wcscpy( temp, unicode_chat_string );
									temp[ChatCursPos] = 0;
									wcscat( temp, one_char_str );
									wcscat( temp, unicode_chat_string + sizeof( wchar_t )*ChatCursPos );
									wcscpy( unicode_chat_string, temp );
								}
							}

							if ( LastAscii )
							{//Ordinary letter, append to chat string
								LastKey = LastAscii;
								char xx[2];
								xx[1] = 0;
								xx[0] = char( LastKey );

								if ( 120 > strlen( ChatString ) + 1 )
								{
									char ccc[2048];
									strcpy( ccc, ChatString );
									ccc[ChatCursPos] = 0;
									strcat( ccc, xx );
									strcat( ccc, ChatString + ChatCursPos );
									strcpy( ChatString, ccc );
									ChatCursPos++;
								}

								goto KRT;
							}
						}
	if ( NKeys )
	{
		goto KRT;
	}

	if ( ChatCursPos > strlen( ChatString ) )
	{
		ChatCursPos = strlen( ChatString );
	}
}

static void ShowMultiplayerChat();

void ShowChat()
{
	if ( NPlayers > 0 )
	{
		ShowMultiplayerChat();
		return;
	}

	if ( EnterChatMode )
	{
		int y0 = smapy + ( smaply << ( Shifter - 2 ) );
		int x0 = smapx + ( smaplx << ( Shifter - 1 ) );
        /*int Dy =*/ GetRLCHeight( BigYellowFont.RLC, 'y' )/* + 5*/;
		int L0 = GetRLCStrWidth( ENCHAT, &BigYellowFont );
		int L = ( GetRLCStrWidth( ChatString, &YellowFont ) + L0 ) / 2;
		if ( L < 100 )
		{
			L = 100;
		}

		DrawStdBar( x0 - L - 16, y0 - 8, x0 + L + 16, y0 + 20 + 7 );

		ShowString( x0 - L - 4, y0 - 5, ENCHAT, &BigWhiteFont );

		if ( ( GetRealTime() / 200 ) & 1 )
		{
			ShowString( x0 - L + L0 + 4, y0 - 4 + 7, ChatString, &YellowFont );
		}
		else
		{
			int xx = 0;
			for ( size_t i = 0; i < ChatCursPos; i++ )
			{
				xx += GetRLCWidth( YellowFont.RLC, ChatString[i] );
			}
			ShowString( x0 - L + L0 + 4, y0 - 4 + 7, ChatString, &YellowFont );
			ShowString( x0 - L + L0 + xx + 4, y0 - 4 + 7, "|", &YellowFont );
		}
	}

	if ( CHATSTRING[0] )
	{
		for ( int i = 0; i < NPlayers; i++ )
		{//Find out, whose message it is
			if ( PINFO[i].PlayerID == CHATDPID )
			{//Show chat message with national color border
				char cc1[250];
				sprintf( cc1, "%s: %s", PINFO[i].name, CHATSTRING );
				CreateTimedHintEx( cc1, kChatMessageDisplayTime, 16 + NatRefTBL[PINFO[i].ColorID] );//Chat message
				PlayEffect( GetSound( "START" ), 0, 0 );
				CHATSTRING[0] = 0;
				i = NPlayers;
			}
		}
	}
}

static int NGSPlayers;
static char GSPLAYERS[9][32];

static const char* CHATHELP1 = nullptr;
static const char* CHATHELP2 = nullptr;
static const char* CHATHELP3 = nullptr;

static void ShowMultiplayerChat()
{
	if ( EnterChatMode )
	{
		if ( !CHATHELP1 )
		{
			CHATHELP1 = GetTextByID( "CHATHELP1" );
			CHATHELP2 = GetTextByID( "CHATHELP2" );
			CHATHELP3 = GetTextByID( "CHATHELP3" );
        }
		int DDD = 20;
		int y0 = smapy + ( smaply << ( Shifter - 2 ) ) - 50 - DDD / 2;
		int x0 = smapx + ( smaplx << ( Shifter - 1 ) );
        /*int Dy =*/ GetRLCHeight( BigYellowFont.RLC, 'y' )/* + 5*/;
		int L0 = GetRLCStrWidth( ENCHAT, &BigYellowFont );
		int L = GetRLCStrWidth( ChatString, &YellowFont ) + L0;
		int L1 = GetRLCStrWidth( CHATHELP1, &SmallWhiteFont );
		int L2 = GetRLCStrWidth( CHATHELP2, &SmallWhiteFont );
		if ( L2 > L1 )
		{
			L1 = L2;
		}

		char cc3[256];
		for ( int i = 0; i < NGSPlayers; i++ )
		{
			sprintf( cc3, CHATHELP3, i + 1, GSPLAYERS );
			L2 = GetRLCStrWidth( cc3, &SmallWhiteFont );
			if ( L2 > L1 )
			{
				L1 = L2;
			}
		}
		if ( L < L1 )
		{
			L = L1;
		}
		L /= 2;
		if ( L < 100 )
		{
			L = 100;
		}

		DrawDoubleTable2( x0 - L - 16, y0 - 8, x0 + L + 16, y0 + 20 + 7 + DDD*( 2 + NGSPlayers ) + 16, y0 + 20 + 7 );
		ShowString( x0 - L - 4, y0 - 5, ENCHAT, &BigWhiteFont );

		//Show string inside chat message box with blinking carriage bar
		if ( ( GetRealTime() / 200 ) % 2 )
		{
			ShowString( x0 - L + L0 + 4, y0 - 4 + 7, ChatString, &YellowFont );
		}
		else
		{//Show carriage bar
			int xx = 0;
			for ( size_t i = 0; i < ChatCursPos; i++ )
			{
				xx += GetRLCWidth( YellowFont.RLC, ChatString[i] );
			}
			ShowString( x0 - L + L0 + 4, y0 - 4 + 7, ChatString, &YellowFont );
			ShowString( x0 - L + L0 + xx + 4, y0 - 4 + 7, "|", &YellowFont );
		}

		ShowString( x0 - GetRLCStrWidth( CHATHELP1, &SmallYellowFont ) / 2, y0 + 20 + 7 + DDD / 2, CHATHELP1, &SmallYellowFont );
		ShowString( x0 - GetRLCStrWidth( CHATHELP2, &SmallYellowFont ) / 2, y0 + 20 + 7 + DDD / 2 + DDD, CHATHELP2, &SmallYellowFont );
		for ( int i = 0; i < NGSPlayers; i++ )
		{
			sprintf( cc3, CHATHELP3, i + 1, GSPLAYERS[i] );
			L2 = GetRLCStrWidth( cc3, &SmallWhiteFont );
			ShowString( x0 - L2 / 2, y0 + 20 + 7 + DDD / 2 + DDD*( i + 2 ), cc3, &SmallYellowFont );
		}
	}

	if ( CHATSTRING[0] )
	{
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].PlayerID == CHATDPID )
			{//FUNNY: buffer cc1[250] for a sprintf call with name[32] and CHATSTRING[256]. Srsly?!
				char cc1[250];
				sprintf( cc1, "%s: %s", PINFO[i].name, CHATSTRING );//CHATSTRING is limited to 120 in input window
				CreateTimedHintEx( cc1, kChatMessageDisplayTime, 16 + NatRefTBL[PINFO[i].ColorID] );//Chat message
				PlayEffect( GetSound( "START" ), 0, 0 );
				CHATSTRING[0] = 0;
				i = NPlayers;
			}
		}
	}
}

bool CheckFNSend( int idx )
{
	if ( !EnterChatMode )
	{
		return false;
	}

	if ( idx < NGSPlayers )
	{
		EnterChatMode = 0;
		return true;
	}
	return false;
}

//---------------------MAP EDITOR INTERFACE--------------//
uint8_t MI_Mode = 1;

static int CurrentEditMode;
static DialogsSystem MAPTOOLS( 2, 0 );

void LoadMapInterface()
{
	MapGP = GPS.PreLoadGPImage( "Interface\\ToolBox" );
	GPS.PreLoadGPImage( "Interface\\ToolOpt" );
}

static void CreateTOOL_PARAM_UNIT();
static void CreateTOOL_PARAM_SPRITE( uint8_t ID );
static void CreateTOOL_PARAM_DEFAULT();
static void CreateTOOL_PARAM_TEXTURE();
static void CreateTOOL_PARAM_PIECES();
static void CreateTOOL_PARAM_WATER();
static void CreateTOOL_PARAM_BRIGHTNESS(uint8_t type );
static void CreateTOOL_PARAM_RELIEF(uint8_t );

static void PressCurrentEdModeButton( int cn )
{
	if ( !EditMapMode )return;
	GP_Button* GB = (GP_Button*) MAPTOOLS.DSS[cn + 1];
	if ( CurrentEditMode != -1 )
	{
		GP_Button* GBO = (GP_Button*) MAPTOOLS.DSS[CurrentEditMode + 1];
		GBO->PassiveFrame = GBO->UserParam * 3 + 2;
		GBO->ActiveFrame = GBO->UserParam * 3 + 3;
    }
	GB->PassiveFrame = GB->UserParam * 3 + 1;
	GB->ActiveFrame = GB->PassiveFrame;
	CurrentEditMode = GB->UserParam;
}

static bool ON_TOOL_CLICK( SimpleDialog* SD )
{
	GP_Button* GB = (GP_Button*) SD;
	if ( CurrentEditMode != -1 )
	{
		GP_Button* GBO = (GP_Button*) MAPTOOLS.DSS[CurrentEditMode + 1];
		GBO->PassiveFrame = GBO->UserParam * 3 + 2;
		GBO->ActiveFrame = GBO->UserParam * 3 + 3;
    }
	GB->PassiveFrame = GB->UserParam * 3 + 1;
	GB->ActiveFrame = GB->PassiveFrame;
	CurrentEditMode = GB->UserParam;
	ClearModes();
	Creator = 4096 + 255;
	OptHidden = 0;
    Inform_Var = 0;
	switch ( GB->UserParam )
	{
	case 0:
		CreateTOOL_PARAM_DEFAULT();
		break;
	case 1://SelectTexture
		CreateTOOL_PARAM_TEXTURE();
		break;
	case 2://select trees
		CreateTOOL_PARAM_SPRITE( 0 );
		break;
	case 3://select stones
		CreateTOOL_PARAM_SPRITE( 1 );
		break;
	case 4://erase objects
		CreateTOOL_PARAM_RELIEF( 1 );
		HeightEditMode = 3;
		MEditMode = false;
		EditMedia = 5;
		break;
	case 5:
		CreateTOOL_PARAM_DEFAULT();
		RiverEditMode = 1;
		break;
	case 6://set water
		CreateTOOL_PARAM_WATER();
		WaterEditMode = 1;
		break;
	case 7://clear water
		CreateTOOL_PARAM_WATER();
		WaterEditMode = 2;
		break;
	case 8://set bright water
		CreateTOOL_PARAM_BRIGHTNESS( 0 );
		WaterEditMode = 3;
		break;
	case 9://clear bright water
		CreateTOOL_PARAM_BRIGHTNESS( 1 );
		WaterEditMode = 4;
		break;
	case 10://Hill up/down
		CreateTOOL_PARAM_RELIEF( 0 );
		HeightEditMode = 1;
		HiStyle = 0;
		break;
	case 11://plane up/down
		CreateTOOL_PARAM_RELIEF( 0 );
		HeightEditMode = 10;
		HiStyle = 0;
		break;
	case 12://equalise/distort surface
		CreateTOOL_PARAM_RELIEF( 0 );
		HeightEditMode = 11;
		HiStyle = 0;
		break;
	case 13://soft/distort surface
		CreateTOOL_PARAM_RELIEF( 0 );
		HeightEditMode = 12;
		HiStyle = 0;
		break;
	case 14://draw curve with relief
		CreateTOOL_PARAM_DEFAULT();
		HeightEditMode = 13;
		HiStyle = 0;
		break;
	case 15://paste piece
		CreateTOOL_PARAM_PIECES();
		break;
	case 16://unblock mode
		CreateTOOL_PARAM_DEFAULT();
		LockMode = 1;
		break;
	case 17://block mode
		CreateTOOL_PARAM_DEFAULT();
		LockMode = 2;
		break;
	case 18://default blocking
		CreateTOOL_PARAM_DEFAULT();
		LockMode = 3;
		break;
	case 19://set unit
		CreateTOOL_PARAM_UNIT();
		break;
	case 20://create zone
		CreateTOOL_PARAM_DEFAULT();
		ZoneCmd = 1;
		break;
	case 21://create group
		CreateTOOL_PARAM_DEFAULT();
		ZoneCmd = 2;
		break;
    }
	Lpressed = 0;
	UnPress();
	return true;
}

// char* UNKED = "???";
static char UNKED[] = "???";

bool ToolsHidden = 0;
bool OptHidden = 0;

static void ProcessTerrainEditor();

static bool TBOX_DIALOGS( SimpleDialog* SD )
{
	switch ( SD->UserParam )
	{
	case 1:
		EditMiniMap();
		break;
	case 2:
		ProcessTerrainEditor();
		break;
	case 3:
		EraseAreas();
		rando();//!!
		CreateTotalLocking();
		ResearchIslands();
		break;
	case 4:
		Reset3D();
		break;
	case 5:
		ProcessResEdit();
		break;
	case 6:
		PeaceMode = !PeaceMode;
		break;
    }
	return true;
}

static bool HideTool( SimpleDialog* /*SD*/ )
{
	ToolsHidden = 1;
	return true;
}

static void CreateMapInterface()
{
	MAPTOOLS.CloseDialogs();
	MAPTOOLS.HintX = -1;
	MAPTOOLS.HintY = 280;
	MAPTOOLS.HintFont = &SmallBlackFont1;
	int x0 = 0;
	int y0 = 32;
	MAPTOOLS.addGPPicture( nullptr, x0, y0, MapGP, 0 );

	for ( int i = 0; i < 22; i++ )
	{
		GP_Button* BT = MAPTOOLS.addGP_Button( nullptr, x0 + 13 + ( i & 1 ) * 30, y0 + 42 + ( i / 2 ) * 30, MapGP, 3 + i * 3, 2 + i * 3 );
		BT->OnUserClick = &ON_TOOL_CLICK;
		BT->UserParam = i;
		char HINID[32];
		sprintf( HINID, "$EHIN_%d", i );
		BT->Hint = GetTextByID( HINID );
		if ( BT->Hint[0] == '$' )BT->Hint = UNKED;
    }
	int DYY = 132;
	GP_Button* B2 = MAPTOOLS.addGP_Button( nullptr, 13, 279 + DYY, MapGP, 69, 68 );
	B2->OnUserClick = &TBOX_DIALOGS;
	B2->UserParam = 1;
	B2->Hint = GetTextByID( "$EHIN__0" );

	B2 = MAPTOOLS.addGP_Button( nullptr, 13 + 30, 279 + DYY, MapGP, 72, 71 );
	B2->OnUserClick = &TBOX_DIALOGS;
	B2->UserParam = 2;
	B2->Hint = GetTextByID( "$EHIN__1" );

	B2 = MAPTOOLS.addGP_Button( nullptr, 13, 279 + 30 + DYY, MapGP, 75, 74 );
	B2->OnUserClick = &TBOX_DIALOGS;
	B2->UserParam = 3;
	B2->Hint = GetTextByID( "$EHIN__2" );

	B2 = MAPTOOLS.addGP_Button( nullptr, 13 + 30, 279 + 30 + DYY, MapGP, 78, 77 );
	B2->OnUserClick = &TBOX_DIALOGS;
	B2->UserParam = 4;
	B2->Hint = GetTextByID( "$EHIN__3" );

	B2 = MAPTOOLS.addGP_Button( nullptr, 13, 279 + 60 + DYY, MapGP, 81, 80 );
	B2->OnUserClick = &TBOX_DIALOGS;
	B2->UserParam = 5;
	B2->Hint = GetTextByID( "$EHIN__4" );

	B2 = MAPTOOLS.addGP_Button( nullptr, 13 + 30, 279 + 60 + DYY, MapGP, 84, 83 );
	B2->OnUserClick = &TBOX_DIALOGS;
	B2->UserParam = 6;
	B2->Hint = GetTextByID( "$EHIN__5" );

	GP_Button* HIDE = MAPTOOLS.addGP_Button( nullptr, 48, 13 + 32, MapGP, 85, 86 );
	HIDE->OnUserClick = &HideTool;
}

bool DRAWLOCK = 0;

static void ClearUnderDialog( DialogsSystem* DSS )
{
	for ( int i = 0; i < 512; i++ )
	{
		SimpleDialog* SD = DSS->DSS[i];
		if ( SD&&SD->Visible&&mouseX >= SD->x&&mouseY >= SD->y&&mouseX <= SD->x1&&mouseY <= SD->y1 )
		{
			UnPress();
			Lpressed = 0;
			ClearMStack();
			DRAWLOCK = 1;
        }
    }
}

static void ProcessTOOL_PARAM();
void ProcessMapInterface()
{
	DRAWLOCK = 0;
	if ( PlayGameMode )
	{
		if ( CUR_TOOL_MODE )
		{
			NeedToPopUp = 1;
        }
    }
	if ( EditMapMode )
	{
		if ( MI_Mode == 1 )
		{
			MI_Mode = 2;
			CreateMapInterface();
        }
		if ( MI_Mode == 2 && !ToolsHidden )
		{
			MAPTOOLS.ProcessDialogs();
			ClearUnderDialog( &MAPTOOLS );
        }
    }
	ProcessTOOL_PARAM();
}

static ColoredBar* NCLB = nullptr;
static DialogsSystem TOOL_PARAM( 0, 0 );
bool TP_Made = 0;
int CUR_TOOL_MODE = 0;

static bool CH_NATION_CLICK( SimpleDialog* SD )
{
	NCLB->x = SD->x - 2;
	NCLB->y = SD->y - 2;
	NCLB->x1 = NCLB->x + 23 - 1;
	NCLB->y1 = NCLB->y + 17 - 1;
	SetMyNation( SD->UserParam );
	NCLB->color = ( (ColoredBar*) SD )->color;

	return true;
}

static bool CH_NATION_CLICK_MP( SimpleDialog* SD )
{
	BuildMode = 0;
	NCLB->x = SD->x - 2;
	NCLB->y = SD->y - 2;
	NCLB->x1 = NCLB->x + 23 - 1;
	NCLB->y1 = NCLB->y + 17 - 1;
	byte RTB[8];
	memcpy( RTB, NatRefTBL, 8 );
	RTB[MyNation] = SD->UserParam;
	CmdChangeNatRefTBL( RTB );
	NCLB->color = ( (ColoredBar*) SD )->color;

	return true;
}

static bool CloseToolOpt( SimpleDialog* /*SD*/ )
{
	OptHidden = 1;
	return true;
}

bool CheckFlagsNeed()
{
	if ( NPlayers < 2 )
		return false;

	if ( SCENINF.hLib )
		return false;

	byte Mask = NATIONS[NatRefTBL[MyNation]].NMask;

	int N = 0;
	for ( int i = 0; i < 7; i++ )
	{
		if ( NATIONS[i].NMask & Mask && NATIONS[i].VictState != 1 && NATIONS[i].VictState != 2 )
		{
			N++;
		}
	}

	return N > 1;
}

static void CreateFlags( int x0, int y0 )
{
	if ( NPlayers > 1 )
	{
		byte Mask = NATIONS[NatRefTBL[MyNation]].NMask;
		for ( int i = 0; i < 8; i++ )
		{
			if ( NATIONS[i].NMask&Mask&&NATIONS[i].VictState != 1 && NATIONS[i].VictState != 2 )
			{
				ColoredBar* BCX = TOOL_PARAM.addColoredBar( x0 + 2 + i * 28, y0 + 2, 18, 12, 0xD0 + i * 4 );
				BCX->OnUserClick = &CH_NATION_CLICK_MP;
				BCX->UserParam = i;
				if ( i == NatRefTBL[MyNation] )
				{
					NCLB = TOOL_PARAM.addColoredBar( x0 + i * 24, y0, 22, 16, 0xD0 + i * 4 );
					NCLB->x = BCX->x - 2;
					NCLB->y = BCX->y - 2;
					NCLB->x1 = NCLB->x + 23 - 1;
					NCLB->y1 = NCLB->y + 17 - 1;
					NCLB->Style = 1;
                }
			}
			else
			{
				ColoredBar* BCX = TOOL_PARAM.addColoredBar( x0 + 2 + i * 28 + 7, y0 + 2 + 4, 18 - 14, 12 - 8, 0xD0 + i * 4 );
				BCX->Style = 1;
            }
        }
	}
	else
	{
		for ( int i = 0; i < 8; i++ )
		{
			ColoredBar* BCX = TOOL_PARAM.addColoredBar( x0 + 2 + i * 28, y0 + 2, 18, 12, 0xD0 + i * 4 );
			BCX->OnUserClick = &CH_NATION_CLICK;
			BCX->UserParam = i;
			if ( i == MyNation )
			{
				NCLB = TOOL_PARAM.addColoredBar( x0 + i * 24, y0, 22, 16, 0xD0 + i * 4 );
				NCLB->x = BCX->x - 2;
				NCLB->y = BCX->y - 2;
				NCLB->x1 = NCLB->x + 23 - 1;
				NCLB->y1 = NCLB->y + 17 - 1;
				NCLB->Style = 1;
            }
        }
    }
	GP_Button* B = TOOL_PARAM.addGP_Button( nullptr, InfDX - 32, -27, MapGP, 86, 85 );
	B->OnUserClick = &CloseToolOpt;
}

static void StdWheel()
{
	if ( WheelDelta )
	{
		ReliefBrush += WheelDelta / 120;
    }
	if ( ReliefBrush < 1 )ReliefBrush = 1;
	if ( ReliefBrush > 110 )ReliefBrush = 110;
	WheelDelta = 0;
}

static void CreateTOOL_PARAM_DEFAULT()
{
	CUR_TOOL_MODE = 0;

	TOOL_PARAM.CloseDialogs();
    int x0 = RealLx - InfDX;
	int y0 = InfDY;
	TOOL_PARAM.BaseX = x0 - 1;
	TOOL_PARAM.BaseY = y0 + 31;
	TOOL_PARAM.addBorder( 0, -31, InfDX - 10, 38, 0, 1 );
	TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR00" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	CreateFlags( 10, 10 );
}

//UNIT parameters
static int TP_CurNation = 0;
static ComboBox* TP_GPB = nullptr;
static VScrollBar* TP_Scroll = nullptr;
static SimpleDialog* TP_STARTSCROLL;
static SimpleDialog* TP_ENDSCROLL;
static int TP_YPREV = 0;
//----------------

static bool TP_UNIT_CLICK( SimpleDialog* SD )
{
	ColoredBar* CB = (ColoredBar*) SD;
	byte CC = 0x65;
	for ( int i = 0; i < 512; i++ )
	{
		if ( TOOL_PARAM.DSS[i] && TOOL_PARAM.DSS[i]->UserParam >= 10000 )
		{
			( (ColoredBar*) TOOL_PARAM.DSS[i] )->color = CC;
		}
	}
	CB->color = 0xFF;
	Creator = CB->UserParam - 10000;
	return true;
}

static void CreateTOOL_PARAM_UNIT()
{
	CUR_TOOL_MODE = 1;

	TOOL_PARAM.CloseDialogs();
    int x0 = RealLx - InfDX;
	int y0 = InfDY;
	TOOL_PARAM.HintFont = &SmallBlackFont;
	TOOL_PARAM.HintX = -1;
	TOOL_PARAM.HintY = 450;
	TOOL_PARAM.BaseX = x0 - 1;
	TOOL_PARAM.BaseY = y0 + 31;
	TOOL_PARAM.addBorder( 0, -31, InfDX - 10, 386, 48 + 26, 3 );
	TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR01" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	CreateFlags( 10, 10 );
	TP_GPB = TOOL_PARAM.addGP_ComboBoxDLX( nullptr, 10, 10 + 26, 224, CBB_GPFILE, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	for ( int i = 0; i < NNations; i++ )TP_GPB->AddLine( NatNames[i] );
	TP_GPB->CurLine = TP_CurNation;
	//Units list
	TP_Scroll = TOOL_PARAM.addNewGP_VScrollBar( nullptr, InfDX - 21, 48 + 26 + 1, 312, 100, 0, 3, 0 );
	TP_Scroll->ScrDy = 200;
	TP_Scroll->OnesDy = 32;
	TOOL_PARAM.addClipper( TOOL_PARAM.BaseX + 4, TOOL_PARAM.BaseY + 48 + 26 + 1, RealLx, TOOL_PARAM.BaseY + 311 + 48 + 26 + 1 );
	TP_STARTSCROLL = nullptr;
	int NUNITS = NATIONS->NUnits[TP_CurNation];
	int xx = 0;
	int yy = 48 + 26 + 6;
	byte CC = 0x65;
	for ( int j = 0; j < NUNITS; j++ )
	{
		int UIDS = NATIONS->UnitsIDS[TP_CurNation][j];
		if ( UIDS == 0xFFFF )
		{
			if ( xx )yy += 44;
			TextButton* TB = TOOL_PARAM.addTextButton( nullptr, 10, yy, GetTextByID( NATIONS->UnitNames[TP_CurNation][j] ), &YellowFont, &YellowFont, &YellowFont, 0 );
			if ( !TP_STARTSCROLL )TP_STARTSCROLL = TB;
			yy += 20;
			xx = 0;
		}
		else
		{
			GeneralObject* GO = NATIONS->Mon[UIDS];
			if ( xx > 4 )
			{
				xx = 0;
				yy += 44;
            }
            /*if ( GO->newMons->IconID >= 0 )*/TOOL_PARAM.addGPPicture( nullptr, 6 + xx * 44, yy, 0, GO->newMons->IconID );
			ColoredBar* CB = TOOL_PARAM.addColoredBar( 5 + xx * 44, yy - 1, 41, 41, CC );
			CB->Style = 1;
			CB->OnUserClick = &TP_UNIT_CLICK;
			CB->UserParam = 10000 + UIDS;
			CB->Hint = GO->Message;
			xx++;
        }
    }
	if ( xx )yy += 44;
	TP_ENDSCROLL = TOOL_PARAM.addClipper( 0, 0, RealLx - 1, RealLy - 1 );
	TP_YPREV = 0;
	TP_Scroll->SMaxPos = yy - 48 - 26 - 4 - 312 + 8;
	if ( TP_Scroll->SMaxPos < 0 )TP_Scroll->Visible = 0;
}

static void ProcessTOOL_PARAM_UNIT()
{
	if ( TP_GPB )
	{
		if ( TP_GPB->CurLine != TP_CurNation )
		{
			TP_CurNation = TP_GPB->CurLine;
			CreateTOOL_PARAM_UNIT();
        }
		bool StartChange = 0;
		int CURRPOS = TP_Scroll->SPos;
		for ( int i = 0; i < 512; i++ )
		{
			SimpleDialog* SD = TOOL_PARAM.DSS[i];
			if ( SD )
			{
				if ( SD == TP_STARTSCROLL )StartChange = 1;
				if ( SD == TP_ENDSCROLL )StartChange = 0;
				if ( StartChange )
				{
					SD->y += TP_YPREV - CURRPOS;
					SD->y1 += TP_YPREV - CURRPOS;
                }
            }
        }
		TP_YPREV = CURRPOS;
    }
}

//---------SPRITES---------
static SprGroup* CURSG;
static constexpr uint8_t AC_COLOR = 0xFF;
static constexpr uint8_t PS_COLOR = 0x5C;

static bool ON_SPRITE_CLICK( SimpleDialog* SD )
{
	ColoredBar* CB = (ColoredBar*) SD;
	if ( GetKeyState( VK_CONTROL ) & 0x8000 )
	{
		if ( CB->color == PS_COLOR )CB->color = AC_COLOR;
		else CB->color = PS_COLOR;
	}
	else
	{
		for ( int i = 0; i < 512; i++ )if ( TOOL_PARAM.DSS[i] )
		{
			if ( TOOL_PARAM.DSS[i]->UserParam >= 10000 )
			{
				( (ColoredBar*) TOOL_PARAM.DSS[i] )->color = PS_COLOR;
            }
        }
		CB->color = AC_COLOR;
    }
	NTreeItems = 0;
	for ( int i = 0; i < 512; i++ )if ( TOOL_PARAM.DSS[i] )
	{
		if ( TOOL_PARAM.DSS[i]->UserParam >= 10000 )
		{
			if ( ( (ColoredBar*) TOOL_PARAM.DSS[i] )->color == AC_COLOR )
			{
				TreeItems[NTreeItems] = TOOL_PARAM.DSS[i]->UserParam - 10000;
				NTreeItems++;
            }
        }
    }
	if ( NTreeItems )
	{
		HeightEditMode = 2;
		MEditMode = false;
		BlobMode = -1;
		if ( CURSG == &TREES )InteriorType = 0;
		if ( CURSG == &STONES )InteriorType = 1;
		if ( CURSG == &HOLES )InteriorType = 2;
	}
	else
	{
		ClearModes();
		HeightEditMode = false;
		BlobMode = -1;
		WaterEditMode = 0;
		Creator = 4096 + 255;
    }
	return true;
}

static VScrollBar* TP_BRS;
static int PrevRelBr = 10;

static void CreateTOOL_PARAM_SPRITE( uint8_t ID )
{

	CUR_TOOL_MODE = 2;
	ClearModes();
	BlobMode = -1;
	WaterEditMode = 0;
	Creator = 4096 + 255;
	HeightEditMode = 2;

	TOOL_PARAM.CloseDialogs();
    int x0 = RealLx - InfDX;
	int y0 = InfDY;
	TOOL_PARAM.HintFont = &SmallBlackFont;
	TOOL_PARAM.HintX = -1;
	TOOL_PARAM.HintY = 450;
	TOOL_PARAM.BaseX = x0 - 1;
	TOOL_PARAM.BaseY = y0 + 31;
	int DL = 24;
	TOOL_PARAM.addBorder( 0, -31, InfDX - 10, 386, 48 + DL, 3 );
	TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR02" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	CreateFlags( 10, 10 );

	int Y0 = 36;
	TOOL_PARAM.addGP_Button( nullptr, 10, Y0, MapGP + 1, 0, 0 );
	TOOL_PARAM.addGP_Button( nullptr, 10 + 34, Y0, MapGP + 1, 3, 2 );
	TP_BRS = TOOL_PARAM.addGP_ScrollBarL( nullptr, 10 + 34, Y0 + 12, 110, ReliefBrush, MapGP + 1, 4, 195, 12, 0, 0 );


	//Units list
	TP_Scroll = TOOL_PARAM.addNewGP_VScrollBar( nullptr, InfDX - 21, 48 + 1 + DL, 312 + 26 - DL, 100, 0, 3, 0 );
	TP_Scroll->ScrDy = 200;
	TP_Scroll->OnesDy = 32;
	TOOL_PARAM.addClipper( TOOL_PARAM.BaseX, TOOL_PARAM.BaseY + 48 + 1 + DL, RealLx, TOOL_PARAM.BaseY + 311 + 48 + 26 + 1 );
	if ( ID == 0 )CURSG = &TREES;
	else if ( ID == 1 )CURSG = &STONES;
	else CURSG = &HOLES;
	int xx = 0;
	int yy = 48 + 4 + DL;
	int MaxLy = 0;
	int Nx = 0;
	int TotLx = 0;
	int MaxLx = InfDX - 25;
	TP_STARTSCROLL = nullptr;
	for ( int i = 0; i <= CURSG->NSpr; i++ )
	{
		int Lx = GPS.GetGPWidth( CURSG->FileID, i );
		int Ly = GPS.GetGPHeight( CURSG->FileID, i );
		int dxx, dyy;
		GetRealGPSize( CURSG->FileID, i, &dxx, &dyy );
		Lx -= dxx;
		Ly -= dyy;

		if ( ( Nx == 0 || TotLx + Lx < MaxLx ) && i < CURSG->NSpr )
		{
			Nx++;
			TotLx += Lx;
			if ( Ly > MaxLy )MaxLy = Ly;
		}
		else
		{
			if ( Nx )i--;
			int dx = MaxLx - TotLx;
			for ( int j = i - Nx + 1; j <= i; j++ )
			{
				int tdx0 = ( ( j - i + Nx - 1 )*dx ) / Nx;
				int tdx1 = ( ( j - i + Nx )*dx ) / Nx;
				int L1 = tdx1 - tdx0;
				Lx = GPS.GetGPWidth( CURSG->FileID, j );
				Ly = GPS.GetGPHeight( CURSG->FileID, j );
				int dxx, dyy;
				GetRealGPSize( CURSG->FileID, j, &dxx, &dyy );
				Lx -= dxx;
				Ly -= dyy;
				GPPicture* GPP = TOOL_PARAM.addGPPicture( nullptr, xx + tdx0 + L1 / 2 - dxx, yy - dyy + ( MaxLy - Ly ) / 2, CURSG->FileID, j );
				if ( !TP_STARTSCROLL )TP_STARTSCROLL = GPP;
				ColoredBar* CB = TOOL_PARAM.addColoredBar( xx + tdx0, yy, L1 + Lx - 1, MaxLy - 1, PS_COLOR );
				CB->OnUserClick = &ON_SPRITE_CLICK;
				CB->UserParam = j + 10000;
				CB->Style = 1;
				xx += Lx;
            }
			xx = 0;
			yy += MaxLy;
			TotLx = 0;
			MaxLy = 0;
			Nx = 0;
        }
    }
	TP_ENDSCROLL = TOOL_PARAM.addClipper( 0, 0, RealLx - 1, RealLy - 1 );
	TP_YPREV = 0;
	TP_Scroll->SMaxPos = yy - 48 - 4 - 312 + 8 - 20;
	if ( TP_Scroll->SMaxPos < 0 )TP_Scroll->Visible = 0;
}

static void ProcessTOOL_PARAM_SPRITES()
{
	if ( ReliefBrush != PrevRelBr )
	{
		PrevRelBr = ReliefBrush;
		TP_BRS->SPos = ReliefBrush;
    }
	ReliefBrush = TP_BRS->SPos;
	bool StartChange = 0;
	int CURRPOS = TP_Scroll->SPos;
	for ( int i = 0; i < 512; i++ )
	{
		SimpleDialog* SD = TOOL_PARAM.DSS[i];
		if ( SD )
		{
			if ( SD == TP_STARTSCROLL )StartChange = 1;
			if ( SD == TP_ENDSCROLL )StartChange = 0;
			if ( StartChange )
			{
				SD->y += TP_YPREV - CURRPOS;
				SD->y1 += TP_YPREV - CURRPOS;
            }
        }
    }
	TP_YPREV = CURRPOS;
	StdWheel();
}

char HEIGHTSTR[12] = "";

static ComboBox* ED_CB0;
static ComboBox* ED_CB1;
static ComboBox* ED_CB2;
static ComboBox* ED_CB3;

static BPXView* ED_BV;
static int CurrTexSet = 0;

static void CreateTOOL_PARAM_TEXTURE()
{
	char ccc[128];
	sprintf( ccc, "Data\\TEXSET%d.DAT", CurrTexSet );
	FILE* F = fopen( ccc, "r" );
	if ( F )
	{
		int v;
		fscanf( F, "%d %d %d %d %d ", &TM_Height0, &TM_Height1, &TM_Angle0, &TM_Angle1, &v );
		NTextures = v;
		for ( int i = 0; i < NTextures; i++ )
		{
			fscanf( F, "%d", &v );
			TexList[i] = v;
        }
		fclose( F );
    }
	CUR_TOOL_MODE = 3;
	ClearModes();
	HeightEditMode = false;
	BlobMode = -1;
	WaterEditMode = 0;
	Creator = 4096 + 255;

	TOOL_PARAM.CloseDialogs();
    int x0 = RealLx - InfDX;
	int y0 = InfDY;
	TOOL_PARAM.HintFont = &SmallBlackFont;
	TOOL_PARAM.HintX = -1;
	TOOL_PARAM.HintY = 450;
	TOOL_PARAM.BaseX = x0 - 1;
	TOOL_PARAM.BaseY = y0 + 31;
	int DLY = 107 + 56;
	TOOL_PARAM.addBorder( 0, -31, InfDX - 10, 386, 48 + DLY, 3 );
	TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR04" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	//CreateFlags(10,10);

	int Y0 = 4;
	int X0 = 5;
	int ODY = 26;
	int maxx = 0;
	char ccx[128];
	for ( int i = 0; i < 5; i++ )
	{
		sprintf( ccx, "EDTP_0%d", i );
		int LX = GetRLCStrWidth( GetTextByID( ccx ), &YellowFont );
		if ( LX > maxx )maxx = LX;
    }
	maxx += 16;

	VScrollBar* VS;
	int CMLX = InfDX - 20 - maxx;

	TOOL_PARAM.addTextButton( nullptr, X0, Y0, GetTextByID( "EDTP_01" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	ED_CB0 = TOOL_PARAM.addGP_ComboBoxDLX( nullptr, X0 + maxx, Y0, CMLX, CBB_GPFILE, 0, 9, 0, &YellowFont, &WhiteFont, nullptr );
	ED_CB0->CreateRuler( -200, 1200 );
	ED_CB0->AssignScroll( &TOOL_PARAM, &VS, 3, 0, 10 );
	ED_CB0->CurLine = TM_Height0 + 200;

	Y0 += ODY;
	TOOL_PARAM.addTextButton( nullptr, X0, Y0, GetTextByID( "EDTP_02" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	ED_CB1 = TOOL_PARAM.addGP_ComboBoxDLX( nullptr, X0 + maxx, Y0, CMLX, CBB_GPFILE, 0, 9, 0, &YellowFont, &WhiteFont, nullptr );
	ED_CB1->CreateRuler( -200, 1200 );
	ED_CB1->AssignScroll( &TOOL_PARAM, &VS, 3, 0, 10 );
	ED_CB1->CurLine = TM_Height1 + 200;


	Y0 += ODY;
	TOOL_PARAM.addTextButton( nullptr, X0, Y0, GetTextByID( "EDTP_03" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	ED_CB2 = TOOL_PARAM.addGP_ComboBoxDLX( nullptr, X0 + maxx, Y0, CMLX, CBB_GPFILE, 0, 9, 0, &YellowFont, &WhiteFont, nullptr );
	ED_CB2->CreateRuler( 0, 90 );
	ED_CB2->AssignScroll( &TOOL_PARAM, &VS, 3, 0, 12 );
	ED_CB2->CurLine = TM_Angle0;

	Y0 += ODY;
	TOOL_PARAM.addTextButton( nullptr, X0, Y0, GetTextByID( "EDTP_04" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	ED_CB3 = TOOL_PARAM.addGP_ComboBoxDLX( nullptr, X0 + maxx, Y0, CMLX, CBB_GPFILE, 0, 9, 0, &YellowFont, &WhiteFont, nullptr );
	ED_CB3->CreateRuler( 0, 90 );
	ED_CB3->AssignScroll( &TOOL_PARAM, &VS, 3, 0, 12 );
	ED_CB3->CurLine = TM_Angle1;

	Y0 += ODY;
	TOOL_PARAM.addTextButton( nullptr, X0, Y0, GetTextByID( "EDTP_05" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	InputBox* IB = TOOL_PARAM.addInputBox( nullptr, X0 + maxx, Y0 - 5, HEIGHTSTR, 20, 150, 20, &YellowFont, &YellowFont );
	IB->Enabled = 0;
	Y0 += ODY;
	TP_GPB = TOOL_PARAM.addGP_ComboBoxDLX( nullptr, 0, Y0, InfDX - 15, CBB_GPFILE, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	TP_GPB->CurLine = CurrTexSet;
    const char* IDX = GetTextByID( "EDTX_S0" );

	for ( int i = 0; i < 10; i++ )
	{
		sprintf( ccx, IDX, i + 1 );
		TP_GPB->AddLine( ccx );
    }

	Y0 += ODY + 10;
	TOOL_PARAM.addGP_Button( nullptr, 10, Y0, MapGP + 1, 0, 0 );
	TOOL_PARAM.addGP_Button( nullptr, 10 + 34, Y0, MapGP + 1, 3, 2 );
	TP_BRS = TOOL_PARAM.addGP_ScrollBarL( nullptr, 10 + 34, Y0 + 12, 110, ReliefBrush, MapGP + 1, 4, 195, 12, 0, 0 );

	TP_Scroll = TOOL_PARAM.addNewGP_VScrollBar( nullptr, InfDX - 21, 48 + 1 + DLY, 312 + 26 - DLY, 100, 0, 3, 0 );
	TP_Scroll->ScrDy = 3;
	ED_BV = TOOL_PARAM.addBPXView( nullptr, 2, 48 + 1 + DLY + 4, 56, 56, 4, 3, MaxTex >> 2, tex1, TP_Scroll );
	ED_BV->DECLX = 8;
	int N = ED_BV->Nx*ED_BV->RealNy;
	for ( int i = 0; i < N; i++ )
	{
		ED_BV->Choosed[i] = 0;
    }
	for ( int i = 0; i < NTextures; i++ )
	{
		ED_BV->Choosed[TexList[i]]++;
    }

}

static void ProcessTOOL_PARAM_TEXTURE()
{
	if ( ReliefBrush != PrevRelBr )
	{
		PrevRelBr = ReliefBrush;
		TP_BRS->SPos = ReliefBrush;
    }
	ReliefBrush = TP_BRS->SPos;
	if ( TP_GPB->CurLine != CurrTexSet )
	{
		CurrTexSet = TP_GPB->CurLine;
		char ccc[128];
		sprintf( ccc, "Data\\TEXSET%d.DAT", CurrTexSet );
		FILE* F = fopen( ccc, "r" );
		if ( F )
		{
			int v;
			fscanf( F, "%d %d %d %d %d ", &TM_Height0, &TM_Height1, &TM_Angle0, &TM_Angle1, &v );
			NTextures = v;
			for ( int i = 0; i < NTextures; i++ )
			{
				fscanf( F, "%d", &v );
				TexList[i] = v;
            }
			fclose( F );
			ED_CB0->CurLine = TM_Height0 + 200;
			ED_CB1->CurLine = TM_Height1 + 200;
			ED_CB2->CurLine = TM_Angle0;
			ED_CB3->CurLine = TM_Angle1;
			int N = ED_BV->Nx*ED_BV->RealNy;
			for ( int i = 0; i < N; i++ )
			{
				ED_BV->Choosed[i] = 0;
            }
			for ( int i = 0; i < NTextures; i++ )
			{
				ED_BV->Choosed[TexList[i]]++;
            }
        }
    }
	bool Change = 0;
	if ( TM_Height0 != ED_CB0->CurLine + ED_CB0->MinDeal )
	{
		TM_Height0 = ED_CB0->CurLine + ED_CB0->MinDeal;
		Change = 1;
    }
	if ( TM_Height1 != ED_CB1->CurLine + ED_CB1->MinDeal )
	{
		TM_Height1 = ED_CB1->CurLine + ED_CB1->MinDeal;
		Change = 1;
    }
	if ( TM_Angle0 != ED_CB2->CurLine + ED_CB2->MinDeal )
	{
		TM_Angle0 = ED_CB2->CurLine + ED_CB2->MinDeal;
		Change = 1;
    }
	if ( TM_Angle1 != ED_CB3->CurLine + ED_CB3->MinDeal )
	{
		TM_Angle1 = ED_CB3->CurLine + ED_CB3->MinDeal;
		Change = 1;
    }

	if ( TM_Angle0 != 90 )TM_Tan0 = int( tan( double( TM_Angle0 )*3.1415 / 180 ) * 256 ); else TM_Tan0 = 100000;
	if ( TM_Angle1 != 90 )TM_Tan1 = int( tan( double( TM_Angle1 )*3.1415 / 180 ) * 256 ); else TM_Tan1 = 100000;
	int ntx = 0;
	int nn = ED_BV->Nx*ED_BV->RealNy;
	for ( int i = 0; i < nn; i++ )
	{
		if ( ED_BV->Choosed[i] )
		{
			for ( int j = 0; j < ED_BV->Choosed[i]; j++ )
			{
				if ( ntx < 256 )
				{
					TexList[ntx] = i;
					ntx++;
                }
            }
        }
    }
	if ( ntx != NTextures )Change = 1;
	if ( ntx )
	{
		NTextures = ntx;
		HeightEditMode = false;
		MEditMode = true;
		TexMapMod = true;
	}
	else
	{
		HeightEditMode = false;
		MEditMode = false;
		TexMapMod = false;
    }
	if ( Change )
	{
		char ccc[128];
		sprintf( ccc, "Data\\TEXSET%d.DAT", CurrTexSet );
		FILE* F = fopen( ccc, "w" );
		if ( F )
		{
			fprintf( F, "%d %d %d %d %d ", TM_Height0, TM_Height1, TM_Angle0, TM_Angle1, NTextures );
			for ( int i = 0; i < NTextures; i++ )
			{
				fprintf( F, "%d ", TexList[i] );
            }
			fclose( F );
        }
    }
	StdWheel();
}

uint8_t NeedToPopUp = 0;
//------------------COPY PIECES------------------//
static char** PIECES_NAMES = nullptr;
static int NPIECES = 0;
static int MAX_PIECES = 0;

static void AddPiece( char* str )
{
	if ( NPIECES >= MAX_PIECES )
	{
		MAX_PIECES += 256;
		PIECES_NAMES = (char**) realloc( PIECES_NAMES, MAX_PIECES << 2 );
    }
	for ( int i = 0; i < NPIECES; i++ )
	{
		if ( _stricmp( str, PIECES_NAMES[i] ) < 0 )
		{
			if ( i < NPIECES )
			{
				for ( int j = NPIECES - 1; j >= i; j-- )PIECES_NAMES[j + 1] = PIECES_NAMES[j];
				PIECES_NAMES[i] = new char[strlen( str ) + 1];
				strcpy( PIECES_NAMES[i], str );
				NPIECES++;
				return;
            }
        }
    }
	PIECES_NAMES[NPIECES] = new char[strlen( str ) + 1];
	strcpy( PIECES_NAMES[NPIECES], str );
	NPIECES++;
}

static void ClearPieces()
{
	for ( int i = 0; i < NPIECES; i++ )free( PIECES_NAMES[i] );
	if ( PIECES_NAMES )free( PIECES_NAMES );
	PIECES_NAMES = nullptr;
	NPIECES = 0;
	MAX_PIECES = 0;
}

static void ReadAllPieces()
{
	ClearPieces();
	WIN32_FIND_DATA FD;
	HANDLE H = FindFirstFile( "UserPieces\\*.smp", &FD );
	if ( H != INVALID_HANDLE_VALUE )
	{
		do
		{
			AddPiece( FD.cFileName );
		} while ( FindNextFile( H, &FD ) );
		FindClose( H );
    }
}

static ListBox* ED_PLIST;
static int CurrPiece = -1;

static void CreateTOOL_PARAM_PIECES()
{
	CurrPiece = -1;
	CUR_TOOL_MODE = 4;

	TOOL_PARAM.CloseDialogs();
    int x0 = RealLx - InfDX;
	int y0 = InfDY;
	TOOL_PARAM.HintFont = &SmallBlackFont;
	TOOL_PARAM.HintX = -1;
	TOOL_PARAM.HintY = 450;
	TOOL_PARAM.BaseX = x0 - 1;
	TOOL_PARAM.BaseY = y0 + 31;
	TOOL_PARAM.addBorder( 0, -31, InfDX - 10, 386, 48 + 26, 3 );
	TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR05" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	CreateFlags( 10, 10 );
	TP_Scroll = TOOL_PARAM.addNewGP_VScrollBar( nullptr, InfDX - 21, 48 + 26 + 1, 312, 100, 0, 3, 0 );
	TP_Scroll->ScrDy = 20;
	ED_PLIST = TOOL_PARAM.addListBox( nullptr, 5, 48 + 26 + 1, 16, 200, 20, &WhiteFont, &YellowFont, TP_Scroll );
	ReadAllPieces();
	for ( int i = 0; i < NPIECES; i++ )
	{
		ED_PLIST->AddItem( PIECES_NAMES[i], i );
    }
	CurrPiece = -1;
	ED_PLIST->CurItem = -1;
}

static void ProcessTOOL_PARAM_PIECES()
{
	if ( ED_PLIST->NItems )
	{
		if ( WheelDelta < 0 )
		{
			ED_PLIST->SetCurrentItem( ED_PLIST->CurItem + 1 );
			//if(ED_PLIST->CurItem>=ED_PLIST->NItems)ED_PLIST->CurItem=ED_PLIST->NItems-1;
			if ( ED_PLIST->CurItem >= ED_PLIST->FLItem + ED_PLIST->ny )ED_PLIST->FLItem = ED_PLIST->CurItem - ED_PLIST->ny + 1;
			if ( ED_PLIST->FLItem < 0 )ED_PLIST->FLItem = 0;
			TP_Scroll->SPos = ED_PLIST->FLItem * 32;
        }
		if ( WheelDelta > 0 )
		{
			ED_PLIST->SetCurrentItem( ED_PLIST->CurItem - 1 );
			//if(ED_PLIST->CurItem<0)ED_PLIST->CurItem=0;
			if ( ED_PLIST->CurItem <= ED_PLIST->FLItem )ED_PLIST->FLItem = ED_PLIST->CurItem;
			TP_Scroll->SPos = ED_PLIST->FLItem * 32;
        }
		WheelDelta = 0;
	}

	if ( CurrPiece != ED_PLIST->CurItem )
	{
		CurrPiece = ED_PLIST->CurItem;
		ProcessRM_LoadEx( ( ED_PLIST->GetItem( CurrPiece ) )->Message );
	}
}

//-----------------------WATER EDITOR---------------------//
static VScrollBar* TP_BRS2;

static void CreateTOOL_PARAM_WATER()
{

	CUR_TOOL_MODE = 5;
	ClearModes();
	BlobMode = -1;
	WaterEditMode = 0;
	Creator = 4096 + 255;

	TOOL_PARAM.CloseDialogs();
    int x0 = RealLx - InfDX;
	int y0 = InfDY;
	TOOL_PARAM.HintFont = &SmallBlackFont;
	TOOL_PARAM.HintX = -1;
	TOOL_PARAM.HintY = 450;
	TOOL_PARAM.BaseX = x0 - 1;
	TOOL_PARAM.BaseY = y0 + 31;
    TOOL_PARAM.addBorder( 0, -31, InfDX - 10, 150, 0, 1 );
	TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR06" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	CreateFlags( 10, 10 );

	int Y0 = 36;
	TOOL_PARAM.addTextButton( nullptr, 5, Y0, GetTextByID( "ED_PARM01" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	Y0 += 24;
	TOOL_PARAM.addGP_Button( nullptr, 10, Y0, MapGP + 1, 0, 0 );
	TOOL_PARAM.addGP_Button( nullptr, 10 + 34, Y0, MapGP + 1, 3, 2 );
	TP_BRS = TOOL_PARAM.addGP_ScrollBarL( nullptr, 10 + 34, Y0 + 12, 110, ReliefBrush, MapGP + 1, 4, 195, 12, 0, 0 );
	Y0 += 26;
	TOOL_PARAM.addTextButton( nullptr, 5, Y0, GetTextByID( "ED_PARM02" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	Y0 += 24;
	TOOL_PARAM.addGP_Button( nullptr, 10, Y0, MapGP + 1, 0, 0 );
	TOOL_PARAM.addGP_Button( nullptr, 10 + 34, Y0, MapGP + 1, 3, 2 );
	TP_BRS2 = TOOL_PARAM.addGP_ScrollBarL( nullptr, 10 + 34, Y0 + 12, 25, CostThickness, MapGP + 1, 4, 195, 12, 0, 0 );
	Y0 += 26;
}

static int PrevCostW = -1;
static void ProcessTOOL_PARAM_WATER()
{
	StdWheel();
	if ( ReliefBrush != PrevRelBr )
	{
		PrevRelBr = ReliefBrush;
		TP_BRS->SPos = ReliefBrush;
    }
	ReliefBrush = TP_BRS->SPos;
	if ( CostThickness != PrevCostW )
	{
		PrevCostW = CostThickness;
		TP_BRS2->SPos = CostThickness;
    }
	CostThickness = TP_BRS2->SPos;
}

static void CreateTOOL_PARAM_BRIGHTNESS( uint8_t type )
{

	CUR_TOOL_MODE = 6;
	ClearModes();
	BlobMode = -1;
	WaterEditMode = 0;
	Creator = 4096 + 255;

	TOOL_PARAM.CloseDialogs();
    int x0 = RealLx - InfDX;
	int y0 = InfDY;
	TOOL_PARAM.HintFont = &SmallBlackFont;
	TOOL_PARAM.HintX = -1;
	TOOL_PARAM.HintY = 450;
	TOOL_PARAM.BaseX = x0 - 1;
	TOOL_PARAM.BaseY = y0 + 31;
    TOOL_PARAM.addBorder( 0, -31, InfDX - 10, 95, 0, 1 );
	int Y0 = 36;
	if ( type )
	{
		TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR08" ), &YellowFont, &YellowFont, &YellowFont, 0 );
		TOOL_PARAM.addTextButton( nullptr, 5, Y0, GetTextByID( "ED_PARM04" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	}
	else
	{
		TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR07" ), &YellowFont, &YellowFont, &YellowFont, 0 );
		TOOL_PARAM.addTextButton( nullptr, 5, Y0, GetTextByID( "ED_PARM03" ), &YellowFont, &YellowFont, &YellowFont, 0 );
    }
	CreateFlags( 10, 10 );
	Y0 += 24;
	if ( ReliefBrush > 20 )ReliefBrush = 20;
	TOOL_PARAM.addGP_Button( nullptr, 10, Y0, MapGP + 1, 0, 0 );
	TOOL_PARAM.addGP_Button( nullptr, 10 + 34, Y0, MapGP + 1, 3, 2 );
	TP_BRS = TOOL_PARAM.addGP_ScrollBarL( nullptr, 10 + 34, Y0 + 12, 20, ReliefBrush, MapGP + 1, 4, 195, 12, 0, 0 );
	Y0 += 26;
}

static void ProcessTOOL_PARAM_BRIGHTNESS()
{
	StdWheel();
	if ( ReliefBrush > 20 )ReliefBrush = 20;
	if ( ReliefBrush != PrevRelBr )
	{
		PrevRelBr = ReliefBrush;
		TP_BRS->SPos = ReliefBrush;
    }
	ReliefBrush = TP_BRS->SPos;
}

static void CreateTOOL_PARAM_RELIEF( uint8_t type )
{

	CUR_TOOL_MODE = 7;
	if ( !type )
	{
		ClearModes();
		BlobMode = -1;
		WaterEditMode = 0;
		Creator = 4096 + 255;
    }

	TOOL_PARAM.CloseDialogs();
    int x0 = RealLx - InfDX;
	int y0 = InfDY;
	TOOL_PARAM.HintFont = &SmallBlackFont;
	TOOL_PARAM.HintX = -1;
	TOOL_PARAM.HintY = 450;
	TOOL_PARAM.BaseX = x0 - 1;
	TOOL_PARAM.BaseY = y0 + 31;
    TOOL_PARAM.addBorder( 0, -31, InfDX - 10, 95, 0, 1 );
	int Y0 = 36;
	if ( type )
	{
		TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR09" ), &YellowFont, &YellowFont, &YellowFont, 0 );
		TOOL_PARAM.addTextButton( nullptr, 5, Y0, GetTextByID( "ED_PARM01" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	}
	else
	{
		TOOL_PARAM.addTextButton( nullptr, 10, -24, GetTextByID( "ED_HDR03" ), &YellowFont, &YellowFont, &YellowFont, 0 );
		TOOL_PARAM.addTextButton( nullptr, 5, Y0, GetTextByID( "ED_PARM01" ), &YellowFont, &YellowFont, &YellowFont, 0 );
    }
	CreateFlags( 10, 10 );
	Y0 += 24;
	TOOL_PARAM.addGP_Button( nullptr, 10, Y0, MapGP + 1, 0, 0 );
	TOOL_PARAM.addGP_Button( nullptr, 10 + 34, Y0, MapGP + 1, 3, 2 );
	TP_BRS = TOOL_PARAM.addGP_ScrollBarL( nullptr, 10 + 34, Y0 + 12, 110, ReliefBrush, MapGP + 1, 4, 195, 12, 0, 0 );
	Y0 += 26;
}

static void ProcessTOOL_PARAM_RELIEF()
{
	StdWheel();
	if ( ReliefBrush != PrevRelBr )
	{
		PrevRelBr = ReliefBrush;
		TP_BRS->SPos = ReliefBrush;
    }
	ReliefBrush = TP_BRS->SPos;
}

static int PrevScrLx = 1024;

static void ProcessTOOL_PARAM()
{
	if ( NoPress&&mouseX > RealLx - InfDX - 32 )return;
	if ( TP_Made&&PrevScrLx != RealLx )
	{
		TP_Made = 0;
    }
	PrevScrLx = RealLx;
	if ( NeedToPopUp )
	{
        Inform_Var = 0;
		OptHidden = 0;
		ClearModes();
		HeightEditMode = false;
		BlobMode = -1;
		WaterEditMode = 0;
		Creator = 4096 + 255;
		switch ( NeedToPopUp )
		{
		case 1://default
			CreateTOOL_PARAM_DEFAULT();
			PressCurrentEdModeButton( 0 );
			break;
		case 2://units
			CreateTOOL_PARAM_UNIT();
			PressCurrentEdModeButton( 19 );
			break;
		case 3://trees
			CreateTOOL_PARAM_SPRITE( 0 );
			PressCurrentEdModeButton( 2 );
			break;
		case 4://stones
			CreateTOOL_PARAM_SPRITE( 1 );
			PressCurrentEdModeButton( 3 );
			break;
		case 5://holes???
			CreateTOOL_PARAM_SPRITE( 2 );
			PressCurrentEdModeButton( 3 );
			break;
		case 6://texture
			CreateTOOL_PARAM_TEXTURE();
			PressCurrentEdModeButton( 1 );
			break;
		case 7://erase sprites
			CreateTOOL_PARAM_RELIEF( 1 );
			PressCurrentEdModeButton( 4 );
			HeightEditMode = 3;
			MEditMode = false;
			EditMedia = 5;
			break;
		case 8://Set water
			CreateTOOL_PARAM_WATER();
			PressCurrentEdModeButton( 6 );
			WaterEditMode = 1;
			break;
		case 9://clear water
			CreateTOOL_PARAM_WATER();
			PressCurrentEdModeButton( 7 );
			WaterEditMode = 2;
			break;
		case 10://Set brightness
			CreateTOOL_PARAM_BRIGHTNESS( 0 );
			PressCurrentEdModeButton( 8 );
			WaterEditMode = 3;
			break;
		case 11://clear brightness
			CreateTOOL_PARAM_BRIGHTNESS( 1 );
			PressCurrentEdModeButton( 9 );
			WaterEditMode = 4;
			break;
		case 12://unlock mode
			CreateTOOL_PARAM_DEFAULT();
			PressCurrentEdModeButton( 16 );
			LockMode = 1;
			break;
		case 13://lock mode
			CreateTOOL_PARAM_DEFAULT();
			PressCurrentEdModeButton( 17 );
			LockMode = 2;
			break;
		case 14://default lock mode
			CreateTOOL_PARAM_DEFAULT();
			PressCurrentEdModeButton( 18 );
			LockMode = 3;
			break;
		case 15://edit height (1) hill
			CreateTOOL_PARAM_RELIEF( 0 );
			HeightEditMode = 1;
			HiStyle = 0;
			break;
		case 16://edit height (2) plane
			CreateTOOL_PARAM_RELIEF( 0 );
			HeightEditMode = 10;
			HiStyle = 0;
			break;
		case 17://edit height (3) equ
			CreateTOOL_PARAM_RELIEF( 0 );
			HeightEditMode = 11;
			HiStyle = 0;
			break;
		case 18://edit height (4) soft
			CreateTOOL_PARAM_RELIEF( 0 );
			HeightEditMode = 12;
			HiStyle = 0;
			break;
		case 19://edit height (5) line
			CreateTOOL_PARAM_DEFAULT();
			HeightEditMode = 13;
			HiStyle = 0;
			break;
		case 20:
			CreateTOOL_PARAM_PIECES();
			PressCurrentEdModeButton( 15 );
			break;
        }
		NeedToPopUp = 0;
    }
	if ( !TP_Made )
	{
		TP_GPB = nullptr;
		switch ( CUR_TOOL_MODE )
		{
		case 0:
			CreateTOOL_PARAM_DEFAULT();
			break;
		case 1:
			CreateTOOL_PARAM_UNIT();
			break;
		case 2:
			CreateTOOL_PARAM_SPRITE( 0 );
			break;
		case 3:
			CreateTOOL_PARAM_TEXTURE();
			break;
		case 4:
			CreateTOOL_PARAM_PIECES();
			break;
		case 5:
			CreateTOOL_PARAM_WATER();
			break;
		case 6:
			CreateTOOL_PARAM_BRIGHTNESS( 0 );
			break;
		case 7:
			CreateTOOL_PARAM_RELIEF( 0 );
			break;
        }
		TP_Made = 1;
    }
	if ( !OptHidden )TOOL_PARAM.ProcessDialogs();
	//ProcessTOOL_PARAM_UNIT();
	switch ( CUR_TOOL_MODE )
	{
	case 1:
		ProcessTOOL_PARAM_UNIT();
		break;
	case 2:
		ProcessTOOL_PARAM_SPRITES();
		break;
	case 3:
		ProcessTOOL_PARAM_TEXTURE();
		break;
	case 4:
		ProcessTOOL_PARAM_PIECES();
		break;
	case 5:
		ProcessTOOL_PARAM_WATER();
		break;
	case 6:
		ProcessTOOL_PARAM_BRIGHTNESS();
		break;
	case 7:
		ProcessTOOL_PARAM_RELIEF();
		break;
    }
	if ( !OptHidden )
	{
		if ( mouseY<TOOL_PARAM.BaseY + 394 && mouseX>RealLx - InfDX - 7 )ClearUnderDialog( &TOOL_PARAM );
    }
}

static constexpr uint8_t PenLX = 60;
static constexpr uint8_t Pen2 = 30;
static constexpr uint8_t PenLY = 60;

static void ClearPen( byte* BUF )
{
	memset( BUF, 0, PenLX*PenLY );
}

static void DrawTreePen( byte* BUF, int v1, int v2 )
{
	ClearPen( BUF );
	int r = ( Pen2*v1 ) >> 10;
	int N = v2;
	BUF[30 + 30 * PenLX] = 0x9A;
	for ( int i = 0; i < N; i++ )
	{
		int x = ( int( randoma[i + i] ) % PenLX ) - Pen2;
		int y = int( randoma[i + i + 1] ) % PenLY - Pen2;
		int rr = sqrt( x*x + y*y );
		if ( rr < r )BUF[x + 30 + ( y + 30 )*PenLX] = 0x9A;
    }
}

static void DrawPenCircle( byte* BUF, int r, byte c )
{
	ClearPen( BUF );
	int r2 = r*r + r;
	for ( int i = 0; i <= r; i++ )
	{
		int v = sqrt( r2 - i*i );
		memset( BUF + ( Pen2 - i )*PenLX + Pen2 - v, c, v + v );
		memset( BUF + ( Pen2 + i )*PenLX + Pen2 - v, c, v + v );
    }
}

static void DrawWaterPen( byte* BUF, int v1 )
{
	int r = ( Pen2*v1 ) >> 10;
	DrawPenCircle( BUF, r, 0xB4 );
}

static void DrawHillPen( byte* BUF, int v1 )
{
	int r = ( Pen2*v1 ) >> 10;
	DrawPenCircle( BUF, r, 0x78 );
}

static void DrawSoftPen( byte* BUF, int v1 )
{
	int r = ( Pen2*v1 ) >> 10;
	DrawPenCircle( BUF, r, 0xA7 );
}

static double GetPenH( int x, int y, int r, int v );

static void DrawHeightPen( byte* BUF, int v1, int v2 )
{
	ClearPen( BUF );
	int r0 = ( Pen2*v1 ) >> 10;
	if ( r0 < 1 )r0 = 1;
	int pos = 0;
	for ( int ix = 0; ix < PenLX; ix++ )
	{
		for ( int iy = 0; iy < PenLY; iy++ )
		{
			BUF[pos] = 0x60 + byte( GetPenH( ix - Pen2, iy - Pen2, r0, v2 ) * 15 );
			pos++;
		}
	}
}

static constexpr uint8_t TERRSH = 8;

static uint8_t * TerrHI;
static uint8_t * SoftArea;
static uint8_t * TerrMap;
static uint8_t * ResultMap;

static void RenderToResult( int x0, int y0, int x1, int y1 )
{
	for ( int iy = y0; iy <= y1; iy++ )
	{
		int ofs = ( iy << TERRSH ) + x0;
		for ( int ix = x0; ix <= x1; ix++ )
		{
			byte b = TerrMap[ofs];
			if ( b == 255 )b = 0x60 + ( TerrHI[ofs] >> 4 );
			if ( SoftArea[ofs] )b = trans8[b + 0xA700];
			ResultMap[ofs] = b;
			ofs++;
		}
	}
}

struct UndoChunk
{
	byte* UndoData;
	int Size;
	int MaxSize;
};

class PixUndo
{
public:
	UndoChunk* Chunks;
	int NChunks;
	PixUndo();
	~PixUndo();
	void AddUndoData( byte Type, byte x, byte y, byte color );
	void OpenNewChunk();
	void PerformUndo();
	void Clear();
};

void PixUndo::Clear()
{
	for ( int i = 0; i < NChunks; i++ )
	{
		if ( Chunks[i].UndoData )free( Chunks[i].UndoData );
    }
	if ( NChunks )free( Chunks );
	Chunks = nullptr;
	NChunks = 0;
}

PixUndo::PixUndo()
{
	NChunks = 0;
	Chunks = nullptr;
}

PixUndo::~PixUndo()
{
	Clear();
}

void PixUndo::OpenNewChunk()
{
	if ( ( !NChunks ) || Chunks[NChunks - 1].Size )
	{
        Chunks = (UndoChunk*) realloc( Chunks, ( NChunks + 1 ) * sizeof( UndoChunk) );
		Chunks[NChunks].Size = 0;
		Chunks[NChunks].MaxSize = 0;
		Chunks[NChunks].UndoData = nullptr;
		NChunks++;
	}
}

void PixUndo::AddUndoData( byte Type, byte x, byte y, byte color )
{
	if ( NChunks )
	{
		if ( Chunks[NChunks - 1].Size + 5 > Chunks[NChunks - 1].MaxSize )
		{
			Chunks[NChunks - 1].MaxSize += 512;
			Chunks[NChunks - 1].UndoData = (byte*) realloc( Chunks[NChunks - 1].UndoData, Chunks[NChunks - 1].MaxSize );
        }
		byte* data = Chunks[NChunks - 1].UndoData + Chunks[NChunks - 1].Size;
		Chunks[NChunks - 1].Size += 4;
		data[3] = Type;
		data[0] = x;
		data[1] = y;
		data[2] = color;
	}
}

void PixUndo::PerformUndo()
{
	if ( NChunks )
	{
		byte* data = Chunks[NChunks - 1].UndoData;
		int sz = Chunks[NChunks - 1].Size;
		int sz0 = sz;
		while ( sz > 0 )
		{
			switch ( data[sz - 1] )
			{
			case 0:
				TerrHI[( int( data[sz - 3] ) << 8 ) + data[sz - 4]] = data[sz - 2];
				sz -= 4;
				break;
			case 1:
				SoftArea[( int( data[sz - 3] ) << 8 ) + data[sz - 4]] = data[sz - 2];
				sz -= 4;
				break;
			case 2:
				TerrMap[( int( data[sz - 3] ) << 8 ) + data[sz - 4]] = data[sz - 2];
				sz -= 4;
				break;
            }
        }
		Chunks[NChunks - 1].Size = 0;
		Chunks[NChunks - 1].MaxSize = 0;
		free( Chunks[NChunks - 1].UndoData );
		Chunks[NChunks - 1].UndoData = nullptr;
		NChunks--;
		if ( !sz0 )PerformUndo();
	}
}

static PixUndo* CURRUNDO;

static void PutTerrPixel( byte Map, int x, int y, byte c )
{
	if ( x < 0 || x>255 || y < 0 || y>255 )return;
	int ofs = x + ( y << TERRSH );
	switch ( Map )
	{
	case 0:
		if ( TerrHI[ofs] != c )
		{
			CURRUNDO->AddUndoData( 0, x, y, TerrHI[ofs] );
			TerrHI[ofs] = c;
		}
		break;
	case 1:
		if ( SoftArea[ofs] != c )
		{
			CURRUNDO->AddUndoData( 1, x, y, SoftArea[ofs] );
			SoftArea[ofs] = c;
		}
		break;
	case 2:
		if ( TerrMap[ofs] != c )
		{
			CURRUNDO->AddUndoData( 2, x, y, TerrMap[ofs] );
			TerrMap[ofs] = c;
		}
		break;
	}
}

static void DrawTerrCircle( byte Type, int x, int y, int r, byte c )
{
    int r2 = r*r + r;
    for ( int i = 0; i <= r; i++ )
    {
        int v = sqrt( r2 - i*i );
        for ( int j = -v; j <= v; j++ )
        {
            PutTerrPixel( Type, x + j, y - i, c );
            PutTerrPixel( Type, x + j, y + i, c );
        }
    }
}

static void TerrDrawTrees( int x, int y, int v1, int v2 )
{
	if ( realRpressed )
	{
		DrawTerrCircle( 2, x, y, ( Pen2*v1 ) >> 10, 0 );
		return;
    }
	int r = ( Pen2*v1 ) >> 10;
	int N = v2;
	PutTerrPixel( 2, x, y, 0x9A );
	for ( int i = 0; i < N; i++ )
	{
		int xx = ( int( randoma[i + i] ) % PenLX ) - Pen2;
		int yy = int( randoma[i + i + 1] ) % PenLY - Pen2;
		int rr = sqrt( xx*xx + yy*yy );
		if ( rr < r )PutTerrPixel( 2, x + xx, y + yy, 0x9A );
	}
}

static void TerrDrawHills( int x, int y, int v1 )
{
	if ( realRpressed )DrawTerrCircle( 2, x, y, ( Pen2*v1 ) >> 10, 0 );
	else DrawTerrCircle( 2, x, y, ( Pen2*v1 ) >> 10, 0xFF );
}

static void TerrDrawWater( int x, int y, int v1 )
{
	if ( realRpressed )DrawTerrCircle( 2, x, y, ( Pen2*v1 ) >> 10, 0 );
	else DrawTerrCircle( 2, x, y, ( Pen2*v1 ) >> 10, 0xB4 );
}

static void TerrDrawSoft( int x, int y, int v1 )
{
	if ( realRpressed )DrawTerrCircle( 1, x, y, ( Pen2*v1 ) >> 10, 0 );
	else DrawTerrCircle( 1, x, y, ( Pen2*v1 ) >> 10, 0xFF );
}

static double GetPenH( int x, int y, int r, int v )
{
	int rr = sqrt( x*x + y*y );
	if ( rr > r )return 0;
	if ( !r )return 255;
	double D = double( rr ) / double( r );
	return 1.0 - exp( ( double( v ) / 128.0 + 1 )*log( D ) );
}

static void TerrDrawHi( int x, int y, int v1, int v2 )
{
	int r = ( Pen2*v1 ) >> 10;
	double DD = double( GetTickCount() % 10 ) / 10.0;
	for ( int ix = -r; ix < r; ix++ )
	{
		for ( int iy = -r; iy < r; iy++ )
		{
			int xx = x + ix;
			int yy = y + iy;
			if ( xx >= 0 && yy >= 0 && xx < 256 && yy < 256 )
			{
				double DH = GetPenH( ix, iy, r, v2 );
				int DY = int( DH*5.0 + DD );
				int ofs = xx + ( yy << TERRSH );
				int v = TerrHI[ofs];
				if ( realRpressed )v -= DY;
				else v += DY;
				if ( v < 0 )v = 0;
				if ( v > 255 )v = 255;
				PutTerrPixel( 0, xx, yy, v );
            }
        }
    }
}

static void SaveHData( const char* Name, byte* data )
{
	ResFile F = RRewrite( Name );
	if ( F != INVALID_HANDLE_VALUE )
	{
		RBlockWrite( F, data, 65535 );
		RClose( F );
    }
}

static void LoadHData( const char* Name, byte* data )
{
	ResFile F = RReset( Name );
	if ( F != INVALID_HANDLE_VALUE )
	{
		RBlockRead( F, data, 65535 );
		RClose( F );
    }
}

static void SetupTerr()
{
	TerrHI = new byte[65536];
	SoftArea = new byte[65536];
	TerrMap = new byte[65536];
	ResultMap = new byte[65536];
	memset( TerrHI, 0xFF, 65536 );
	memset( SoftArea, 0, 65536 );
	memset( TerrMap, 0, 65536 );
	memset( ResultMap, 0, 65536 );
	LoadHData( "Data\\TerrHI.dat", TerrHI );
	LoadHData( "Data\\SoftArea.dat", SoftArea );
	LoadHData( "Data\\TerrMap.dat", TerrMap );
	RenderToResult( 0, 0, 255, 255 );
}

static void FreeTerr()
{
	SaveHData( "Data\\TerrHI.dat", TerrHI );
	SaveHData( "Data\\SoftArea.dat", SoftArea );
	SaveHData( "Data\\TerrMap.dat", TerrMap );
	free( TerrHI );
	free( SoftArea );
	free( TerrMap );
	free( ResultMap );
}

static void DrawCircle( int x, int y, int r, byte c )
{
	char* xi = Rarr[r].xi;
	char* yi = Rarr[r].yi;
	int N = Rarr[r].N;
	for ( int i = 0; i < N; i++ )Hline( x + xi[i], y + yi[i], x + xi[i], c );
}

//-------------------TERRAIN EDITOR----------------//
static void PushScreen( uint8_t ** Ptr )
{
	*Ptr = new byte[ScrWidth*RealLy];
	memcpy( *Ptr, ScreenPtr, ScrWidth*RealLy );
}

static void PopScreen( uint8_t * Ptr )
{
	memcpy( ScreenPtr, Ptr, ScrWidth*RealLy );
	free( Ptr );
}

static void SaveTerrainBitmap( char* Name )
{
	byte* TMP24 = new byte[256 * ( 256 + 1 ) * 3 * 3];
	byte* DATA24 = TMP24;
	int ofs3 = 0;
	for ( int i = 0; i < 65536; i++ )
	{
		byte v = TerrMap[i];
		if ( v == 0x9A )
		{//tree
			DATA24[ofs3] = 0;
			DATA24[ofs3 + 1] = 255;
			DATA24[ofs3 + 2] = 0;
		}
		else if ( v == 0xFF )
		{//hills
			DATA24[ofs3] = 0xFF;
			DATA24[ofs3 + 1] = 0xFF;
			DATA24[ofs3 + 2] = 0xFF;
		}
		else if ( v == 0xB4 )
		{//water
			DATA24[ofs3] = 0xFF;
			DATA24[ofs3 + 1] = 0;
			DATA24[ofs3 + 2] = 0;
		}
		else
		{
			DATA24[ofs3] = 0;
			DATA24[ofs3 + 1] = 0;
			DATA24[ofs3 + 2] = 0;
        }
		ofs3 += 3;
    }
	DATA24 += 65536 * 3;
	for ( int i = 0; i < 768; i++ )
	{
		if ( ( i & 3 ) == 1 )DATA24[0] = 0xFF;
		else DATA24[0] = 0;
		DATA24++;
    }
	ofs3 = 0;
	for ( int i = 0; i < 65536; i++ )
	{
		byte v = TerrHI[i];
		DATA24[ofs3] = v;
		DATA24[ofs3 + 1] = v;
		DATA24[ofs3 + 2] = v;
		ofs3 += 3;
    }
	DATA24 += 65536 * 3;
	for ( int i = 0; i < 768; i++ )
	{
		if ( ( i & 3 ) == 1 )DATA24[0] = 0xFF;
		else DATA24[0] = 0;
		DATA24++;
    }
	ofs3 = 0;
	for ( int i = 0; i < 65536; i++ )
	{
		byte v = SoftArea[i];
		if ( v )
		{
			DATA24[ofs3] = 0;
			DATA24[ofs3 + 1] = 0;
			DATA24[ofs3 + 2] = 0xFF;
		}
		else
		{
			DATA24[ofs3] = 0;
			DATA24[ofs3 + 1] = 0;
			DATA24[ofs3 + 2] = 0;
        }
		ofs3 += 3;
    }
	DATA24 += 65536 * 3;
	for ( int i = 0; i < 768; i++ )
	{
		if ( ( i & 3 ) == 1 )DATA24[0] = 0xFF;
		else DATA24[0] = 0;
		DATA24++;
    }
	SaveToBMP24( Name, 256, 256 * 3 + 3, TMP24 );
	free( TMP24 );
}

static bool LoadTerrainBitmap( char* Name )
{
	byte* TMP24;
	BMPformat BM;
	if ( ReadBMP24( Name, &BM, &TMP24 ) )
	{
		if ( BM.biWidth != 256 || BM.biHeight != 256 * 3 + 3 )
		{
			free( TMP24 );
			return false;
        }
		byte* DATA24 = TMP24;
		int ofs3 = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			int B = DATA24[ofs3];
			int G = DATA24[ofs3 + 1];
			int R = DATA24[ofs3 + 2];
			byte v = 0;
			if ( G > ( B + R ) * 2 )v = 0x9A;
			else if ( B > 200 && G > 200 && R > 200 )v = 0xFF;
			else if ( B > ( G + R ) * 2 )v = 0xB4;
			TerrMap[i] = v;
			ofs3 += 3;
        }
		DATA24 += 65536 * 3 + 256 * 3;
		ofs3 = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			TerrHI[i] = DATA24[ofs3 + 1];
			ofs3 += 3;
        }
		DATA24 += 65536 * 3 + 256 * 3;
		ofs3 = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			if ( DATA24[ofs3 + 2] >= 128 )SoftArea[i] = 0xFF;
			else SoftArea[i] = 0;
			ofs3 += 3;
        }
		free( TMP24 );
		return true;
	}
	else return false;
}

static void ProcessLoadBitmap()
{
	byte* p;
	PushScreen( &p );
	LocalGP BOR2( "Interface\\Bor2" );
	char BMPS[128];
	if ( ProcessLoadingFile( "UserTerrain\\*.bmp", BMPS, 3 ) )
	{
		char ccc[128];
		sprintf( ccc, "UserTerrain\\%s", BMPS );
		if ( !LoadTerrainBitmap( ccc ) )
		{
			WaitWithError( "INCRBMFORM", BOR2.GPID );
        }
    }
	RenderToResult( 0, 0, 255, 255 );
	PopScreen( p );
	CURRUNDO->Clear();
	ItemChoose = -1;
}

static void ProcessSaveBitmap()
{
	byte* p;
	PushScreen( &p );
	char BMPS[128];
	if ( ProcessSavingFile( "UserTerrain\\*.bmp", BMPS, 6, 1 ) )
	{
		char ccc[128];
		_strupr( BMPS );
		if ( !strstr( BMPS, ".BMP" ) )strcat( BMPS, ".BMP" );
		sprintf( ccc, "UserTerrain\\%s", BMPS );
		SaveTerrainBitmap( ccc );
    }
	RenderToResult( 0, 0, 255, 255 );
	PopScreen( p );
	ItemChoose = -1;
}

static void ProcessTerrainEditor()
{
	PixUndo PUNDO;
	CURRUNDO = &PUNDO;
	SetupTerr();
	byte PenBuf[PenLX*PenLY];
	LocalGP BTNS( "Interface\\EditTerra" );
	DialogsSystem DSS( ( RealLx - 524 ) >> 1, ( RealLy - 400 ) >> 1 );
	DSS.HintX = -1;
	DSS.HintY = 180;
	DSS.HintFont = &SmallBlackFont;
	byte* BACK = new byte[640 * 400];
	MemScreenPart( DSS.BaseX, DSS.BaseY, 640, 400, BACK );
	//GPS.ShowGP((RealLx-524)>>1,(RealLy-400)>>1,BTNS.GPID,1,0);
	DSS.addBPXView( nullptr, 0, 0, 640, 400, 1, 1, 1, BACK, nullptr );
	DSS.addGPPicture( nullptr, 0, 0, BTNS.GPID, 0 );
	DSS.addGPPicture( nullptr, 0, 0, BTNS.GPID, 1 );
	GP_Button* OkBtn = DSS.addGP_Button( nullptr, 79, 356, BTNS.GPID, 5, 4 );
	OkBtn->UserParam = mcmOk;
	OkBtn->OnUserClick = &MMItemChoose;
	GP_Button* CancelBtn = DSS.addGP_Button( nullptr, 300, 356, BTNS.GPID, 3, 2 );
	CancelBtn->UserParam = mcmCancel;
	CancelBtn->OnUserClick = &MMItemChoose;
	GP_Button* Load = DSS.addGP_Button( nullptr, 301, 260, BTNS.GPID, 7, 6 );
	Load->UserParam = 1;
	Load->OnUserClick = &MMItemChoose;
	GP_Button* Save = DSS.addGP_Button( nullptr, 301, 291, BTNS.GPID, 9, 8 );
	Save->UserParam = 2;
    Save->OnUserClick = &MMItemChoose;
	CheckBox* TREES = DSS.addGP_CheckBox( nullptr, 301, 58, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 10, 11, 12 );
	TREES->Hint = GetTextByID( "$TE_HN1" );
	CheckBox* WATER = DSS.addGP_CheckBox( nullptr, 333, 58, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 13, 14, 15 );
	WATER->Hint = GetTextByID( "$TE_HN2" );
	CheckBox* HILLS = DSS.addGP_CheckBox( nullptr, 365, 58, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 16, 17, 18 );
	HILLS->Hint = GetTextByID( "$TE_HN3" );
	CheckBox* SOFT = DSS.addGP_CheckBox( nullptr, 397, 58, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 19, 20, 21 );
	SOFT->Hint = GetTextByID( "$TE_HN4" );
	CheckBox* HEIGHT = DSS.addGP_CheckBox( nullptr, 429, 58, "", &YellowFont, &YellowFont, 1, 0, BTNS.GPID, 22, 23, 24 );
	HEIGHT->Hint = GetTextByID( "$TE_HN5" );
	GP_Button* UNDO = DSS.addGP_Button( nullptr, 467, 58, BTNS.GPID, 27, 26 );
	UNDO->Hint = GetTextByID( "$TE_HN6" );
	UNDO->UserParam = 3;
	UNDO->OnUserClick = &MMItemChoose;
	VScrollBar* HB1 = DSS.addGP_ScrollBarL( nullptr, 301, 116 + 8, 1024, 64, MapGP + 1, 4, 195, 12, 0, 0 );
	VScrollBar* HB2 = DSS.addGP_ScrollBarL( nullptr, 301, 162 + 7, 1024, 64, MapGP + 1, 4, 195, 12, 0, 0 );
	DSS.addBPXView( nullptr, 302, 190, 60, 60, 1, 1, 1, PenBuf, nullptr );
	ItemChoose = -1;
	int CurPen = -1;
	ClearPen( PenBuf );
	BPXView* VIEW = DSS.addBPXView( nullptr, 31, 59, 256, 256, 1, 1, 1, ResultMap, nullptr );
	int spp1 = -1;
	int spp2 = -1;
	int pcp = -1;
	do
	{
		if ( !( realLpressed || realRpressed ) )PUNDO.OpenNewChunk();
		ProcessMessages();
		if ( KeyPressed&&LastKey == 'Z' )
		{
			PUNDO.PerformUndo();
			RenderToResult( 0, 0, 255, 255 );
			LastKey = 0;
			KeyPressed = 0;
        }
		DSS.ProcessDialogs();
		if ( VIEW->MouseOver )
		{
			TempWindow TW;
			PushWindow( &TW );
			IntersectWindows( VIEW->x, VIEW->y, VIEW->x1, VIEW->y1 );
			DrawCircle( mouseX, mouseY, ( HB1->SPos * 30 ) >> 10, 0xFF );
			PopWindow( &TW );
        }
		DSS.RefreshView();
		if ( TREES->State )CurPen = 0;
		if ( WATER->State )CurPen = 1;
		if ( HILLS->State )CurPen = 2;
		if ( SOFT->State )CurPen = 3;
		if ( HEIGHT->State )CurPen = 4;
		//DrawTreePen(PenBuf,HB1->SPos,HB2->SPos);
		//DrawWaterPen(PenBuf,HB1->SPos);
		if ( spp1 != HB1->SPos || spp2 != HB2->SPos || pcp != CurPen )
		{
			spp1 = HB1->SPos;
			spp2 = HB2->SPos;
			pcp = CurPen;
			switch ( CurPen )
			{
			case 0:
				DrawTreePen( PenBuf, HB1->SPos, HB2->SPos );
				break;
			case 1:
				DrawWaterPen( PenBuf, HB1->SPos );
				break;
			case 2:
				DrawHillPen( PenBuf, HB1->SPos );
				break;
			case 3:
				DrawSoftPen( PenBuf, HB1->SPos );
				break;
			case 4:
				DrawHeightPen( PenBuf, HB1->SPos, HB2->SPos );
				break;
            }
        }
		if ( ( realLpressed || realRpressed ) && VIEW->MouseOver )
		{
			int x = mouseX - VIEW->x;
			int y = mouseY - VIEW->y;
			switch ( CurPen )
			{
			case 0:
				TerrDrawTrees( x, y, HB1->SPos, HB2->SPos );
				break;
			case 1:
				TerrDrawWater( x, y, HB1->SPos );
				break;
			case 2:
				TerrDrawHills( x, y, HB1->SPos );
				break;
			case 3:
				TerrDrawSoft( x, y, HB1->SPos );
				break;
			case 4:
				TerrDrawHi( x, y, HB1->SPos, HB2->SPos );
				//DrawHeightPen(PenBuf,HB1->SPos,HB2->SPos);
				break;
            }
			RenderToResult( 0, 0, 255, 255 );
        }
		HB1->SPos += WheelDelta / 4;
		if ( HB1->SPos < 0 )HB1->SPos = 0;
		if ( HB1->SPos > HB1->SMaxPos )HB1->SPos = HB1->SMaxPos;
		WheelDelta = 0;
		if ( ItemChoose == 1 )
		{
			ProcessLoadBitmap();
			ItemChoose = -1;
        }
		if ( ItemChoose == 2 )
		{
			ProcessSaveBitmap();
			ItemChoose = -1;
        }
		if ( ItemChoose == 3 )
		{
			PUNDO.PerformUndo();
			RenderToResult( 0, 0, 255, 255 );
			LastKey = 0;
			KeyPressed = 0;
			ItemChoose = -1;
        }

	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
		//generating
		byte* DATA24 = new byte[65536 * 3];
		int ofs3 = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			byte v = TerrMap[i];
			if ( v == 0x9A )
			{//tree
				DATA24[ofs3] = 0;
				DATA24[ofs3 + 1] = 255;
				DATA24[ofs3 + 2] = 0;
			}
			else if ( v == 0xFF )
			{//hills
				DATA24[ofs3] = 0xFF;
				DATA24[ofs3 + 1] = 0xFF;
				DATA24[ofs3 + 2] = 0xFF;
			}
			else if ( v == 0xB4 )
			{//water
				DATA24[ofs3] = 0xFF;
				DATA24[ofs3 + 1] = 0;
				DATA24[ofs3 + 2] = 0;
			}
			else
			{
				DATA24[ofs3] = 0;
				DATA24[ofs3 + 1] = 0;
				DATA24[ofs3 + 2] = 0;
            }
			ofs3 += 3;
        }
		SaveToBMP24( "Data\\01.bmp", 256, 256, DATA24 );
		ofs3 = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			byte v = TerrHI[i];
			DATA24[ofs3] = v;
			DATA24[ofs3 + 1] = v;
			DATA24[ofs3 + 2] = v;
			ofs3 += 3;
        }
		SaveToBMP24( "Data\\02.bmp", 256, 256, DATA24 );
		ofs3 = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			byte v = SoftArea[i];
			if ( v )
			{
				DATA24[ofs3] = 0;
				DATA24[ofs3 + 1] = 0;
				DATA24[ofs3 + 2] = 0xFF;
			}
			else
			{
				DATA24[ofs3] = 0;
				DATA24[ofs3 + 1] = 0;
				DATA24[ofs3 + 2] = 0;
            }
			ofs3 += 3;
        }
		SaveToBMP24( "Data\\03.bmp", 256, 256, DATA24 );
		free( DATA24 );
		GenerateMapForMission( "Data\\01.bmp", "hehe", "Data\\02.bmp", "Data\\03.bmp" );
    }
	free( BACK );
	FreeTerr();
}

static constexpr uint16_t CM_WLx = 352;
static constexpr uint16_t CM_WLy = 90;

void CreateNEWMAP()
{
	int X0 = ( RealLx - CM_WLx ) >> 1;
	int Y0 = ( RealLy - CM_WLy ) >> 1;

	byte* p = new byte[( CM_WLx + 64 )*( CM_WLy + 140 )];
	MemScreenPart( X0 - 32, Y0 - 32, ( CM_WLx + 64 ), ( CM_WLy + 140 ), p );
	DialogsSystem DSS( X0, Y0 );
	DSS.addBPXView( nullptr, -32, -32, ( CM_WLx + 64 ), ( CM_WLy + 140 ), 1, 1, 1, p, nullptr );
	DSS.addBorder( 0, 0, CM_WLx, CM_WLy, 0, 1 );
	int FDY = 40;
	DSS.addTextButton( nullptr, 16, 5, GetTextByID( "NEWMPHDR" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	DSS.addTextButton( nullptr, 16, FDY, GetTextByID( "SELMSZ" ), &YellowFont, &YellowFont, &YellowFont, 0 );
	ComboBox* CB = DSS.addGP_ComboBoxDLX( nullptr, 150, FDY, 180, CBB_GPFILE, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );
	CB->AddLine( GetTextByID( "SEL_MS0" ) );
	CB->AddLine( GetTextByID( "SEL_MS1" ) );
	CB->AddLine( GetTextByID( "SEL_MS2" ) );

	int XB = 10;
	int YB = CM_WLy + 7;
	DSS.addGPPicture( nullptr, XB, YB, CBB_GPFILE, 8 );
	DSS.addGPPicture( nullptr, XB, YB, CBB_GPFILE, 11 );
	GP_TextButton* OKBTN = DSS.addGP_TextButton( nullptr, XB + 9, YB + 20, GetTextByID( "@OK" ), CBB_GPFILE, 9, &WhiteFont, &YellowFont );
	OKBTN->OnUserClick = &MMItemChoose;
	OKBTN->UserParam = mcmOk;
	XB = 180;
	YB = CM_WLy + 7;
	DSS.addGPPicture( nullptr, XB, YB, CBB_GPFILE, 8 );
	DSS.addGPPicture( nullptr, XB, YB, CBB_GPFILE, 11 );
	GP_TextButton* CANCELBTN = DSS.addGP_TextButton( nullptr, XB + 9, YB + 20, GetTextByID( "@CANCEL" ), CBB_GPFILE, 9, &WhiteFont, &YellowFont );
	CANCELBTN->OnUserClick = &MMItemChoose;
	CANCELBTN->UserParam = 0;
	ItemChoose = -1;
	do
	{
		ProcessMessages();
		DSS.ProcessDialogs();
		DSS.RefreshView();
	} while ( ItemChoose == -1 );

	if ( ItemChoose == mcmOk )
	{
		DeleteAllSprites();
		DeleteAllUnits();
		ClearAllZones();
		FreeArrays();
		ADDSH = 1 + CB->CurLine;
		msx = 240 << ADDSH;
		msy = 240 << ADDSH;
		SetupArrays();
		EraseAreas();
		rando();//!!
		CreateTotalLocking();

		ResearchIslands();
	}
	free( p );
}
//-----------------Clans support----------------//
struct ClanInfo
{
	char* ClanName;
	SQPicture Passive;
	SQPicture Active;
	SQPicture Over;
};

static ClanInfo* CLINFO = nullptr;

static int NClans = 0;

void ReadClanData()
{
	SafeLoad = 1;
	WIN32_FIND_DATA FD;
	HANDLE H = FindFirstFile( "Clans\\*.clan", &FD );
	if ( H != INVALID_HANDLE_VALUE )
	{
		do
		{
			char ccc[128];
			sprintf( ccc, "Clans\\%s", FD.cFileName );
			FILE* F = fopen( ccc, "r" );
			if ( F )
			{
				char cc1[128];
				char cc2[128];
				char cc3[128];
				int z = fscanf( F, "%s%s%s", cc1, cc2, cc3 );
				if ( z == 3 )
				{
					SQPicture P1;
					sprintf( ccc, "Clans\\%s", cc1 );
					P1.LoadPicture( ccc );
					if ( P1.GetLx() < 256 && P1.GetLy() == 20 )
					{
						sprintf( ccc, "Clans\\%s", cc2 );
						P1.LoadPicture( ccc );
						if ( P1.GetLx() < 256 && P1.GetLy() == 20 )
						{
							sprintf( ccc, "Clans\\%s", cc3 );
							P1.LoadPicture( ccc );
							if ( P1.GetLx() < 256 && P1.GetLy() == 20 )
							{
                                CLINFO = (ClanInfo*) realloc( CLINFO, ( NClans + 1 ) * sizeof( ClanInfo) );
                                memset( CLINFO + NClans, 0, sizeof( ClanInfo) );
								sprintf( ccc, "Clans\\%s", cc1 );
								CLINFO[NClans].Over.LoadPicture( ccc );
								sprintf( ccc, "Clans\\%s", cc2 );
								CLINFO[NClans].Active.LoadPicture( ccc );
								sprintf( ccc, "Clans\\%s", cc3 );
								CLINFO[NClans].Passive.LoadPicture( ccc );
								strcpy( ccc, FD.cFileName );
								char* ccx = strstr( ccc, ".clan" );
								if ( !ccx )ccx = strstr( ccc, ".CLAN" );
								if ( !ccx )ccx = strstr( ccc, ".Clan" );
								if ( ccx )ccx[0] = 0;
								CLINFO[NClans].ClanName = new char[strlen( ccc ) + 1];
								strcpy( CLINFO[NClans].ClanName, ccc );
								NClans++;
							}
						}
					}
				}
				fclose( F );
			}
		} while ( FindNextFile( H, &FD ) );

		FindClose( H );
	}

	SafeLoad = 0;
	//sorting
	bool change;
	ClanInfo CIN;
	do
	{
		change = 0;
		for ( int i = 1; i < NClans; i++ )
		{
			if ( strlen( CLINFO[i - 1].ClanName ) < strlen( CLINFO[i].ClanName ) )
			{
				CIN = CLINFO[i - 1];
				CLINFO[i - 1] = CLINFO[i];
				CLINFO[i] = CIN;
				change = 1;
			}
		}
	} while ( change );

	memset( &CIN, 0, sizeof CIN );
}

void ShowClanString( int x, int y, char* s, uint8_t State, RLCFont* Fn, RLCFont* Fn1, int DY )
{
	char ccc[64];
	char NICK[128];
	char CLAN[128];

	strcpy( NICK, s );
	_strupr( NICK );
	strcpy( ccc, s );

	for ( int i = 0; i < NClans; i++ )
	{
		strcpy( CLAN, CLINFO[i].ClanName );
		_strupr( CLAN );
		char* cc = strstr( NICK, CLAN );
		if ( cc && cc == NICK )
		{
			//ok, found!
			cc = ccc;
			cc += strlen( CLINFO[i].ClanName );

			SQPicture* p = nullptr;
			switch ( State )
			{
			case 0:
				p = &CLINFO[i].Passive;
				break;
			case 1:
				p = &CLINFO[i].Active;
				break;
			case 2:
				p = &CLINFO[i].Over;
				break;
			}

			p->Draw( x, y - 2 );

			if ( Fn1 )
			{
				ShowString( x + p->GetLx() + 1 + 3, y + DY + 1, cc, Fn1 );
			}

			ShowString( x + p->GetLx() + 3, y + DY, cc, Fn );
			return;
		}
	}

	if ( Fn1 )
	{
		ShowString( x + 1, y + DY + 1, s, Fn1 );
	}

	ShowString( x, y + DY, s, Fn );
	return;
}

bool CheckUsingAI()
{
	for ( int i = 0; i < 8; i++ )
	{
		if ( NATIONS[i].AI_Enabled )
		{
			return true;
		}
	}
	return false;
}
