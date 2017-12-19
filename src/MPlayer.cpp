#include <cmath>

// BoonXRay 09.09.2017
#pragma pack(4)
#include "CommCore/CommCore.h"
#include "Pinger.h"
#pragma pack(1)



#include "IntExplorer/Explore.h"
#include "3DGraph.h"
#include "3DRandMap.h"
#include "ActiveScenary.h"
#include "BoonUtils.hpp"
#include "ConstStr.h"
#include "Ddini.h"
#include "Dialogs.h"
#include "Dpchat.h"
#include "DrawForm.h"
#include "FastDraw.h"
#include "GP_Draw.h"
#include "Hint.h"
#include "Interface.h"
#include "LoadSave.h"
#include "main.h"
#include "Mapa.h"
#include "MapSprites.h"
#include "Megapolis.h"
#include "Multi.h"
#include "NewMon.h"
#include "SaveNewMap.h"
#include "SelProp.h"
#include "Sort.h"
#include "Weapon.h"


#include "MPlayer.h"


PlayerInfo PINFO[ 8 ];

CCommCore IPCORE;

PLAYERSBACKUP PBACK;
uint16_t PlayerMenuMode;
uint16_t PrevRpos;

bool GameInProgress;

LPDIRECTPLAY3A lpDirectPlay3A;
static LPDPLAYINFO lpDPInfo = 0;
static constexpr uint8_t MAXNAMELEN = 200; // max size of a session or player name
char szSessionName[MAXNAMELEN];
static const uint32_t PlExitID = 0xEA3521BC;
static constexpr uint32_t NON = 0xFFFFFFFF;
static uint8_t PrevEB[4096];
static int PrevEBSize;
static uint8_t PrevPrevEB[4096];
static int PrevPrevEBSize;
//data transferring format
// int ExDataID
// int Time 
// int Rand_Index
// data

static uint16_t NProviders;
uint16_t NPlayers;
static uint16_t NSessions;
uint32_t RealTime;

static LPVOID lplpConnectionBuffer[16];
static GUID SessionsGUID[32];
static DPID PlayersID[32];
static uint8_t * PData[32];

uint32_t MyDPID;
DPID ServerDPID;
static uint8_t * MyData;
static char PlayersList[512];
static char SessionsList[512];
static uint32_t ExBuf1[2193];
uint32_t EBPos1;

// message structure used to send a chat string to another player
typedef struct
{
	DWORD dwType; // message type (APPMSG_CHATSTRING)
	char szMsg[1]; // message string (variable length)
} MSG_CHATSTRING, *LPMSG_CHATSTRING;

// globals
static DPLAYINFO DPInfo;
//execute bufferisation

EXBUFFER EBufs[MaxPL];
EXBUFFER EBufs1[MaxPL];
//network errors:
int SeqErrorsCount;
static int LastRandDif;

static void SetupEBufs()
{
	SeqErrorsCount = 0;
	LastRandDif = 0;
	PrevRpos = 0;
	for (int i = 0; i < MaxPL; i++)
	{
		EBufs[i].RealTime = NON;
		EBufs1[i].RealTime = NON;
		EBufs[i].Enabled = true;
		EBufs1[i].Enabled = true;
		EBufs[i].Size = 0;
		EBufs1[i].Size = 0;
	}
}


//Describes which players are out of sync
uint8_t SYNBAD[8] = {};

//Copies EBufs[i].Data into ExBuf
//Performs synchronization check
static void UpdateEBufs()
{
	//copy containig to ExBuf
	byte* EPOS = ExBuf;
	EBPos = 0;
	word FirstRand = 0xFFFF;
    memset( SYNBAD, 0, 8 );
	for (int i = 0; i < MaxPL; i++)
	{
		//checking random sequence
		if (EBufs[i].Enabled && NON != EBufs[i].RealTime)
		{
			if (0xFFFF == FirstRand)
			{
				FirstRand = word( EBufs[i].RandIndex );
			}
			else
			{
				if (EBufs[i].RandIndex != FirstRand)
				{//IMPORTANT: Synchronization warning!
                    SeqErrorsCount++;
					LastRandDif = int( EBufs[i].RandIndex ) - int( FirstRand );
					rpos = FirstRand;
					SYNBAD[PINFO[i].ColorID] = 1;
				}
			}
			//copy data to execute buffer
			memcpy( EPOS, &EBufs[i].Data, EBufs[i].Size );
			EPOS += EBufs[i].Size;
			EBPos += EBufs[i].Size;
		}

		EBufs[i].RealTime = EBufs1[i].RealTime;

		if (NON != EBufs1[i].RealTime)
		{
			memcpy( &EBufs[i].Data, &EBufs1[i].Data, EBufs1[i].Size );
			EBufs[i].Size = EBufs1[i].Size;
			EBufs[i].RandIndex = EBufs1[i].RandIndex;
			EBufs1[i].Size = 0;
			EBufs1[i].RealTime = NON;
		}
	}
}

static bool PresentEmptyBuf()
{
	bool retval = false;
	for (int i = 0; i < NPlayers; i++)
	{
		if (NON == EBufs[i].RealTime && EBufs[i].Enabled)
		{
			retval = true;
		}
	}
	return retval;
}

static int GetPIndex( DPID PD )
{
	int i = 0;
	while (i < NPlayers && PD != PlayersID[i])
	{
		i++;
	}
	if (i < NPlayers)
	{
		return i;
	}
	else
	{
		return -1;
	}
}

char CHATSTRING[256] = "";

uint32_t CHATDPID = 0;
static PlayerInfo* PINFLOC;

static BOOL WINAPI PIEnumPlayersCallback2( DPID dpId,
    DWORD /*dwPlayerType*/, LPCDPNAME lpName,
    DWORD /*dwFlags*/, LPVOID /*lpContext*/ )
{
	if (NPlayers >= 8)return false;
	PlayersID[NPlayers] = dpId;
	PINFLOC[NPlayers].PlayerID = dpId;
	DWORD pisize = sizeof( PlayerInfo ) - 36;
	strcpy( PINFLOC[NPlayers].name, lpName->lpszShortNameA );
	lpDirectPlay3A->GetPlayerData( dpId, &PINFLOC[NPlayers].NationID, &pisize, DPGET_REMOTE );
	if (PINFLOC[NPlayers].Host)ServerDPID = dpId;
	//assert(pisize==3);
	NPlayers++;
	return true;
}

static DPID PLIDS[8];
static int PLFRQ[8];

static void SortPLIDS()
{
	bool plprs[10];
	int plidx[10];
	memset( plprs, 0, sizeof plprs );
	for (int i = 0; i < NPlayers; i++)
	{
		DPID pid = PINFO[i].PlayerID;
		bool pinfn = 1;
		for (int j = 0; j < 10 && pinfn; j++)if (PLIDS[j] == pid)
		{
			PLFRQ[j]++;
			pinfn = 0;
			plprs[j] = 1;
			plidx[i] = j;
        }
		if (pinfn)
		{
			for (int j = 0; j < 10 && pinfn; j++)if (!PLIDS[j])
			{
				PLFRQ[j] = 1;
				pinfn = 0;
				plprs[j] = 1;
				plidx[i] = j;
				PLIDS[j] = pid;
            }
        }
    }
	for (int i = 0; i < 8; i++)if (!plprs[i])
	{
		PLFRQ[i] = 0;
		PLIDS[i] = 0;
    }
	SortClass SC;
	SC.CheckSize( NPlayers );
	SC.NUids = NPlayers;
	for (int i = 0; i < NPlayers; i++)
	{
		SC.Uids[i] = i;
		SC.Parms[i] = -PLFRQ[plidx[i]];
    }
	SC.Sort();
	PlayerInfo TPI[8];
	memcpy( TPI, PINFO, sizeof TPI );
	memset( PINFO, 0, sizeof PINFO );
	for (int i = 0; i < NPlayers; i++)
	{
		PINFO[i] = TPI[SC.Uids[i]];
		PlayersID[i] = PINFO[i].PlayerID;
    }
}

static int prevtime = 0;

//Recieves lobby player data via IPCORE.GetUserData and stores it in PINFLOC[]
static BOOL __stdcall IPCORE_EnumProc( const PEER_ID PeerID, LPCSTR lpcszPeerName )
{
	if (NPlayers >= 7)
	{
		return false;
	}

	PlayersID[NPlayers] = PeerID;
	PINFLOC[NPlayers].PlayerID = PeerID;
	strcpy( PINFLOC[NPlayers].name, lpcszPeerName );

	word pisize = sizeof( PlayerInfo ) - 36;
	memset( &PINFLOC[NPlayers].NationID, 0, pisize );
	IPCORE.GetUserData( PeerID, &PINFLOC[NPlayers].NationID, &pisize );

	if (PINFLOC[NPlayers].Host)
	{
		ServerDPID = PeerID;
	}

	NPlayers++;
	return true;
}

static int GetLastAnswerT( DWORD ID );
static void DelBADPL();

bool PIEnumeratePlayers( PlayerInfo* PIN, bool DoMsg )
{
	if (GameInProgress)
	{
		return true;
	}
	if (GetTickCount() - prevtime < 250)
	{
		return true;
	}
	if (DoNewInet && DoMsg)
	{
		if (MyDPID == ServerDPID)
		{
			for (int i = 0; i < NPlayers; i++)
			{
				if (PINFO[i].PlayerID != MyDPID)
				{
					int T = GetLastAnswerT( PINFO[i].PlayerID );
					if (T && GetTickCount() - T > 20000)
					{
						IPCORE.DeletePeer( PINFO[i].PlayerID );
					}
				}
			}
		}
		else
		{
			int T = GetLastAnswerT( ServerDPID );
			if (T && GetTickCount() - T > 20000)
			{
				LocalGP BARS( "Interface\\bor2" );
				ShowCentralMessage( GetTextByID( "NOSRCONN" ), BARS.GPID );
				FlipPages();
				int tt = GetTickCount();
				do
				{
					ProcessMessages();
				} while (GetTickCount() - tt < 3000);
				return false;
			}
		}
	}

	prevtime = GetTickCount();
	NPlayers = 0;
	PlayersList[0] = 0;
	PINFLOC = PIN;

	if (DoNewInet)
	{
		IPCORE.lpEnumProc = &IPCORE_EnumProc;
		IPCORE.EnumPeers();
	}
	else
	{
		if (int( lpDirectPlay3A ))
		{
			lpDirectPlay3A->EnumPlayers( (GUID*) &DPCHAT_GUID,
				PIEnumPlayersCallback2, nullptr, 0 );
		}
	}

	SortPLIDS();
	DelBADPL();
	return true;
}

PingSumm PSUMM;
int CurrentStartTime[8];
static int GameLength[8];
static int NextGameLength[8];
int NextStartTime[8];
int CurStatus = 0;
static int PlState[MaxPL];
static DWORD LastAccess[MaxPL];
static bool LockPing = 0;
int CurrentMaxPing[8] = { -1,-1,-1,-1,-1,-1,-1,-1 };

static int NFROMID[8] = { 0 };
static int NTOTFROM[8] = { 0 };
static int NFROMID1[8] = { 0 };
static int NFROMID2[8] = { 0 };
static int NFAIL[8] = { 0 };
static int NBACK[8] = { 0 };
static int NSEN1 = 0;
static int NSEN2 = 0;
static int NSENB1 = 0;
static int NSENB2 = 0;
static int NSENF1 = 0;
static int NSENF2 = 0;

static void ClearFAILS()
{
	memset( NFROMID, 0, 4 * 8 );
	memset( NTOTFROM, 0, 4 * 8 );
	memset( NFROMID1, 0, 4 * 8 );
	memset( NFROMID2, 0, 4 * 8 );
	memset( NFAIL, 0, 4 * 8 );
	memset( NBACK, 0, 4 * 8 );
	NSEN1 = 0;
	NSEN2 = 0;
	NSENB1 = 0;
	NSENB2 = 0;
	NSENF1 = 0;
	NSENF2 = 0;
}

int ExitNI = -1;
static void EndPing( int ID );
static bool SendToPlayer( DWORD Size, LPVOID lpData, DWORD DPID );


static void HandleApplicationMessage( LPDPLAYINFO /*lpDPInfo*/, LPDPMSG_GENERIC lpMsg, DWORD dwMsgSize,
	DPID idFrom, DPID idTo )
{
	DWORD SDP = idTo == DPID_ALLPLAYERS ? MyDPID : idTo;
	if (SDP == MyDPID)
	{
		int player_index = GetPIndex( idFrom );
		if (player_index < 0)
		{
			return;
		}

		NTOTFROM[player_index]++;

		DWORD* lp = (DWORD*) (LPMSG_CHATSTRING) lpMsg;
		byte* BUF = (byte*) (LPMSG_CHATSTRING) lpMsg;

		bool CheckRetrans = 0;
		DPID RealIDFROM = idFrom;

        //if (lp[0] == 'SIAP')
        if( lp[ 0 ] == ConvCosChar( 'S', 'I', 'A', 'P' ) )
		{
			idFrom = lp[1];
			player_index = GetPIndex( idFrom );
			lp += 2;
			BUF += 8;
			dwMsgSize -= 8;
			CheckRetrans = 1;
		}

		byte* BUFX = BUF;
		word* WWW = (word*) ( BUFX + 1 );
		byte STAGE = 0;
		if (BUFX[0] == 0xBA && BUFX[1] == BUFX[2])
		{
			byte msk = BUFX[1];
			for (int i = 0; i < NPlayers; i++)
			{
				if (msk&( 1 << i ))
				{
					PBACK.SendInfoAboutTo( PINFO[i].PlayerID, idFrom, *( (DWORD*) ( BUFX + 3 ) ) );
				}
			}
		}

STAGENEXT:
		if (BUFX[0] == 0xBB && WWW[0] + WWW[1] == WWW[2])
		{
			if (!STAGE)
			{
				NBACK[player_index]++;
			}
			STAGE++;
			if (1 == STAGE)
			{
				BUF = BUFX + 7;
				dwMsgSize = WWW[0];
				if (!dwMsgSize)
				{
					STAGE++;
				}
			}
			if (2 == STAGE)
			{
				BUF = BUFX + 7 + WWW[0];
				dwMsgSize = WWW[1];
				if (!dwMsgSize)
				{
					STAGE++;
				}
			}
		}

		if (( BUF[0] == 0xAF ) && dwMsgSize < 2048 && STAGE != 3)
		{
            // BoonXRay 26.08.2017
            /*int mycl = 7;
			for (int j = 0; j < 8; j++)
			{
				if (PINFO[j].PlayerID == idFrom)
				{
					mycl = j;
				}
			}
            word s1 = 0xAE;
			for (DWORD j = 3; j < dwMsgSize; j++)
			{
				s1 += BUF[j];
            }*/
		}
		else
		{
			if (( BUF[0] == 0xAE || BUF[0] == 0xBF ) && dwMsgSize < 2048)
			{
				NFROMID[player_index]++;
				int mycl = 7;
				for (int j = 0; j < 8; j++)
				{
					if (PINFO[j].PlayerID == idFrom)
					{
						mycl = j;
					}
				}
				word s = ( (word*) ( BUF + 1 ) )[0];
				word s1 = 0xAE;
				for (DWORD j = 3; j < dwMsgSize; j++)
				{
					s1 += BUF[j];
				}
				byte rtx = RealTime;
				if (rtx == BUF[3])
				{
					NFROMID1[player_index]++;
				}
				rtx++;
				if (rtx == BUF[3])
				{
					NFROMID2[player_index]++;
				}
				if (s1 == s)
				{
					byte rt = RealTime;
					if (rt == BUF[3])
					{
						EBufs[player_index].Size = dwMsgSize - 7;
						EBufs[player_index].RandIndex = ( (word*) ( BUF + 4 ) )[0];
						int dt = int( BUF[6] ) << 3;
						if (CurrentStartTime[mycl] == -1)
						{
							CurrentStartTime[mycl] = dt;
						}
						GameLength[mycl] = 0;
						memcpy( &EBufs[player_index].Data, BUF + 7, dwMsgSize - 7 );
						int prt = EBufs[player_index].RealTime;
						EBufs[player_index].RealTime = RealTime;
						PBACK.AddInf( BUF, dwMsgSize, idFrom, RealTime );
						if (CheckRetrans&&prt == -1)
						{
							DWORD RTR[2];
                            //RTR[0] = 'RETR';
                            RTR[0] = ConvCosChar( 'R', 'E', 'T', 'R' );
							RTR[1] = idFrom;
							SendToPlayer( 8, &RTR, RealIDFROM );
						}
						RETSYS.CheckRetr( idFrom, RealTime );
					}

					rt++;

					if (rt == BUF[3])
					{
						EBufs1[player_index].Size = dwMsgSize - 7;
						EBufs1[player_index].RandIndex = ( (word*) ( BUF + 4 ) )[0];
						int dt = int( BUF[6] ) << 3;
						if (NextStartTime[mycl] == -1)
						{
							NextStartTime[mycl] = dt;
						}
						NextGameLength[mycl] = 0;
						memcpy( &EBufs1[player_index].Data, BUF + 7, dwMsgSize - 7 );
						int prt = EBufs[player_index].RealTime;
						EBufs1[player_index].RealTime = RealTime + 1;
						PBACK.AddInf( BUF, dwMsgSize, idFrom, RealTime + 1 );
						if (CheckRetrans&&prt == -1)
						{
							DWORD RTR[2];
                            //RTR[0] = 'RETR';
                            RTR[0] = ConvCosChar( 'R', 'E', 'T', 'R' );
							RTR[1] = idFrom;
							SendToPlayer( 8, &RTR, RealIDFROM );
						}
						RETSYS.CheckRetr( idFrom, RealTime + 1 );
					}
				}
				else
				{
					NFAIL[player_index]++;
				}

				if (STAGE == 1)
				{
					goto STAGENEXT;
				}

			}
			else
			{
				if (!STAGE)
				{
                    //if (lp[0] == 'RETR')
                    if( lp[ 0 ] == ConvCosChar( 'R', 'E', 'T', 'R' ) )
					{
						RETSYS.AddSection( idFrom, lp[1], RealTime );
					}
					else
					{
						if (lp[0] == PlExitID)
						{
							EBufs[player_index].Enabled = false;
							char buf[200];
							sprintf( buf, GetTextByID( "PLEXIT" ), PINFO[player_index].name );
							CreateTimedHintEx( buf, kSystemMessageDisplayTime, 32 );//Player %s has left the game.
							for (int i = 0; i < NPlayers; i++)
							{
								if (PINFO[i].PlayerID == idFrom)
								{
									int ni = PINFO[i].ColorID;
									if (NATIONS[ni].VictState != 2 && NATIONS[MyNation].VictState == 0)
									{
										NATIONS[ni].VictState = 1;
										ExitNI = ni;
									}
								}
							}
							AutoTime = GetTickCount() - 20000;
						}
						else
						{
                            //if (lp[0] == 'CHAT')
                            if( lp[ 0 ] == ConvCosChar( 'C', 'H', 'A', 'T' ) )
							{
								memcpy( CHATSTRING, (char*) ( lp + 2 ), lp[1] );
								CHATSTRING[lp[1]] = 0;
								CHATDPID = idFrom;
							}
							else
							{
                                //if (lp[0] == 'ALLY')
                                if( lp[0] == ConvCosChar( 'A', 'L', 'L', 'Y' ) )
								{
									if (NATIONS[NatRefTBL[PINFO[player_index].ColorID]].NMask & NATIONS[NatRefTBL[MyNation]].NMask)
									{
										memcpy( CHATSTRING, (char*) ( lp + 2 ), lp[1] );
										CHATSTRING[lp[1]] = 0;
										CHATDPID = idFrom;
									}
								}
								else
								{
                                    //if (lp[0] == 'PING' && lp[0] + lp[1] + lp[2] + lp[3] + lp[4] == lp[5])
                                    if( ( lp[0] == ConvCosChar( 'P', 'I', 'N', 'G' ) ) && ( lp[0] + lp[1] + lp[2] + lp[3] + lp[4] == lp[5] ) )
									{
                                        lp[3] = GetRealTime();
										lp[4] = MyDPID;
                                        //lp[0] = 'ANSW';
                                        lp[0] = ConvCosChar( 'A', 'N', 'S', 'W' );
										lp[5] = lp[0] + lp[1] + lp[2] + lp[3] + lp[4];
										SendToPlayer( 24, lp, lp[1] );
									}
									else
									{
                                        //if (lp[0] == 'ANSW' && lp[0] + lp[1] + lp[2] + lp[3] + lp[4] == lp[5])
                                        if( ( lp[0] == ConvCosChar( 'A', 'N', 'S', 'W' ) ) && ( lp[0] + lp[1] + lp[2] + lp[3] + lp[4] == lp[5] ) )
										{
                                            PSUMM.AddPing( lp[4], lp[2], lp[3], GetRealTime() );
											EndPing( lp[2] );
										}
										else
										{
                                            //if (lp[0] == 'ALIV')
                                            if (lp[0] == ConvCosChar( 'A', 'L', 'I', 'V' ))
											{//Alive request recieved, send answer
												DWORD ANSW[4];
                                                //ANSW[0] = 'ALIA';
                                                ANSW[0] = ConvCosChar( 'A', 'L', 'I', 'A' );
												ANSW[1] = MyDPID;
												ANSW[2] = CurStatus;

												if (!bActive)
												{
													ANSW[2] = 17;
												}

												ANSW[3] = ANSW[0] + ANSW[1] + ANSW[2];
												SendToPlayer( 16, ANSW, idFrom );
											}
											else
											{
                                                //if (lp[0] == 'ALIA' && lp[3] == lp[0] + lp[1] + lp[2])
                                                if( ( lp[0] == ConvCosChar( 'A', 'L', 'I', 'A' ) ) && ( lp[3] == lp[0] + lp[1] + lp[2] ) )
												{//Answer to alive request
													const DPID sender_id = lp[1];
													for (int i = 0; i < NPlayers; i++)
													{
														if (sender_id == PINFO[i].PlayerID)
														{//Assign alive answer value to player
															//0: OK
															//1: Looking at menu
															//17: Alt-tab
															PlState[i] = lp[2];
															LastAccess[i] = GetRealTime();
														}
													}
												}
												else
												{
                                                    //if (lp[0] == 'PNTF' && lp[2] == lp[0] + lp[1])
                                                    if( ( lp[0] == ConvCosChar( 'P', 'N', 'T', 'F' ) ) && ( lp[2] == lp[0] + lp[1] ) )
													{
														CurrentMaxPing[player_index] = lp[1];
													}
													else
													{
                                                        //if (lp[0] == 'FIDN')
                                                        if (lp[0] == ConvCosChar( 'F', 'I', 'D', 'N' ) )
														{
                                                            //lp[0] = 'FRPL';
                                                            lp[0] = ConvCosChar( 'F', 'R', 'P', 'L' );
															lp[1] = GetMapSUMM( (char*) ( lp + 2 ) );
															SendToPlayer( 8, lp, idFrom );
                                                        }//FIDN
													}//PNTF
												}//ALIA
											}//ALIV
										}//ANSW
									}//PING
								}//ALLY
							}//CHAT
						}//PLEXIT
					}//RETR

					if (MyData)
					{
						free( MyData );
					}

					MyData = new byte[dwMsgSize];
					memcpy( MyData, lp, dwMsgSize );
				}
			}
		}
	}
}

static void HandleSystemMessage( LPDPLAYINFO lpDPInfo, LPDPMSG_GENERIC lpMsg, DWORD /*dwMsgSize*/,
    DPID /*idFrom*/, DPID /*idTo*/ )
{
	LPSTR lpszStr = nullptr;

	// The body of each case is there so you can set a breakpoint and examine
	// the contents of the message received.
	switch (lpMsg->dwType)
	{
	case DPSYS_CREATEPLAYERORGROUP:
	{
		LPDPMSG_CREATEPLAYERORGROUP lp = (LPDPMSG_CREATEPLAYERORGROUP) lpMsg;
        LPCSTR lpszPlayerName;
        LPCSTR szDisplayFormat = "\"%s\" has joined\r\n";

		// get pointer to player name
		if (lp->dpnName.lpszShortNameA)
			lpszPlayerName = lp->dpnName.lpszShortNameA;
		else
			lpszPlayerName = "unknown";

		// allocate space for string
		lpszStr = new char[lstrlen( szDisplayFormat ) +
			lstrlen( lpszPlayerName ) + 1];
		if (lpszStr == nullptr)
			break;

		// build string
		wsprintf( lpszStr, szDisplayFormat, lpszPlayerName );
	}
	break;

	case DPSYS_DESTROYPLAYERORGROUP:
	{
		LPDPMSG_DESTROYPLAYERORGROUP lp = (LPDPMSG_DESTROYPLAYERORGROUP) lpMsg;
        LPCSTR lpszPlayerName;
        LPCSTR szDisplayFormat = "\"%s\" has left\r\n";

		// get pointer to player name
		if (lp->dpnName.lpszShortNameA)
			lpszPlayerName = lp->dpnName.lpszShortNameA;
		else
			lpszPlayerName = "unknown";

		// allocate space for string
		lpszStr = new char[lstrlen( szDisplayFormat ) +
			lstrlen( lpszPlayerName ) + 1];
		if (lpszStr == nullptr)
			break;

		// build string
		wsprintf( lpszStr, szDisplayFormat, lpszPlayerName );
	}
	break;

    case DPSYS_ADDPLAYERTOGROUP:
	break;

    case DPSYS_DELETEPLAYERFROMGROUP:
	break;

    case DPSYS_SESSIONLOST:
	break;

	case DPSYS_HOST:
	{
        LPCSTR szDisplayFormat = "You have become the host\r\n";

		// allocate space for string
		lpszStr = new char[strlen( szDisplayFormat ) + 1];
		if (lpszStr == nullptr)
			break;

		// build string
		lstrcpy( lpszStr, szDisplayFormat );

		// we are now the host
		lpDPInfo->bIsHost = TRUE;
	}
	break;

    case DPSYS_SETPLAYERORGROUPDATA:
	break;

    case DPSYS_SETPLAYERORGROUPNAME:
	break;
	}

	// post string to chat window
	if (lpszStr)
	{
		// make sure window is still valid
		//if (ghChatWnd)
		// PostMessage(ghChatWnd, WM_USER_ADDSTRING, (WPARAM) 0, (LPARAM) lpszStr);
		//else
		free( lpszStr );
	}
}

static bool NeedMoreReceive;
static uint8_t BUFFERX[40000];
//--------------Temporary for testing---------------
static int NMessM;
static int MessSize[64];
static DWORD MessIDFrom[64];
static DWORD MessIDTo[64];
static int MessData[64][64];
//--------------------------------------------------
static int PREVGRAPHRT = 0;
static int PREVGRAPHST = 0;
static int PREVGRAPHSSZ = 0;
static int NSENDP = 0;
static unsigned long MAXSENDP = 0;
static int PREVTIME = 0;

static HRESULT ReceiveMessage( LPDPLAYINFO lpDPInfo )
{
	DPID idFrom, idTo;
	LPVOID lpvMsgBuffer;
	DWORD dwMsgBufferSize;
	HRESULT hr;

	if (!PREVTIME)
	{
		PREVTIME = GetRealTime();
	}

	if (!( lpDPInfo->lpDirectPlay3A || DoNewInet ))
	{
		return 0;
	}

	lpvMsgBuffer = BUFFERX;
	dwMsgBufferSize = 40000;

	// loop until a single message is successfully read
	do
	{
		// read messages from any player, including system player
		idFrom = 0;
		idTo = 0;
		dwMsgBufferSize = 40000;

		if (DoNewInet)
		{
			u_short peer;
			dwMsgBufferSize = IPCORE.ReceiveData( (byte*) lpvMsgBuffer, &peer );
			idFrom = peer;
			idTo = MyDPID;

			if (!dwMsgBufferSize)
			{
				hr = DPERR_NOMESSAGES;
			}
			else
			{
				hr = DP_OK;
			}
		}
		else
		{
			if (!dwMsgBufferSize)
			{
				hr = lpDPInfo->lpDirectPlay3A->Receive( &idFrom, &idTo, DPRECEIVE_ALL || DPRECEIVE_PEEK,
					lpvMsgBuffer, &dwMsgBufferSize );
			}
			else
			{
				hr = lpDPInfo->lpDirectPlay3A->Receive( &idFrom, &idTo, DPRECEIVE_ALL,
					lpvMsgBuffer, &dwMsgBufferSize );
			}
		}

		if (dwMsgBufferSize != 40000 && NMessM < 64)
		{
			if (idTo)
			{
				MessSize[NMessM] = dwMsgBufferSize;
				MessIDFrom[NMessM] = idFrom;
				MessIDTo[NMessM] = idTo;
				int sz = dwMsgBufferSize;
				if (sz > 64)sz = 64;
				memcpy( MessData[NMessM], lpvMsgBuffer, sz );
				NMessM++;
			}
		}
	} while (hr == DPERR_BUFFERTOOSMALL);

	if (( SUCCEEDED( hr ) ) && // successfully read a message
		( dwMsgBufferSize >= sizeof( DPMSG_GENERIC ) )) // and it is big enough
	{
		// check for system message
		int tt = GetTickCount();
		if (!PREVGRAPHRT)
		{
			PREVGRAPHRT = tt;

		}

		if (idFrom == DPID_SYSMSG)
		{
			HandleSystemMessage( lpDPInfo, (LPDPMSG_GENERIC) lpvMsgBuffer,
				dwMsgBufferSize, idFrom, idTo );
		}
		else
		{
			HandleApplicationMessage( lpDPInfo, (LPDPMSG_GENERIC) lpvMsgBuffer,
				dwMsgBufferSize, idFrom, idTo );
		}

		if (tt - PREVGRAPHRT > 1000)
		{
			PREVGRAPHRT = tt;
		}
	}

	if (SUCCEEDED( hr ))
	{
		NeedMoreReceive = true;
	}
	else
	{
		NeedMoreReceive = false;
	}

	return ( DP_OK );
}


void ProcessReceive()
{
	if (lpDPInfo && lpDPInfo->lpDirectPlay3A)
	{
		AnalyseMessages();
	}

	if (DoNewInet)
	{
		AnalyseMessages();
	}
}

void ReceiveAll()
{
	LockPing = ( GetRealTime() - PREVTIME ) > 50;
	PREVTIME = GetRealTime();
	do
	{
		ReceiveMessage( lpDPInfo );
	} while (NeedMoreReceive);
}

static HRESULT SetupConnection( HINSTANCE /*hInstance*/, LPDPLAYINFO lpDPInfo )
{
	ZeroMemory( lpDPInfo, sizeof( DPLAYINFO ) );
	ZeroMemory( PData, sizeof( PData ) );
	ZeroMemory( PlayersID, sizeof( PlayersID ) );
	ZeroMemory( &MyData, sizeof( 4 * 32 ) );

	NPlayers = 0;
	ServerDPID = 0xFFFFFFFF;
	MyDPID = 0;
	GameInProgress = false;

	// create event used by DirectPlay to signal a message has arrived
	return ( DP_OK );
}

//Init DirectPlay and DPInfo structure
void SetupMultiplayer( HINSTANCE hInstance )
{
	SetupConnection( hInstance, &DPInfo );
	lpDPInfo = &DPInfo;
}

static bool IPCORE_INIT = 0;
static bool NETWORK_INIT = 0;
static HRESULT ShutdownConnection( LPDPLAYINFO lpDPInfo );

void ShutdownMultiplayer( bool Final )
{
	IAmLeft();
	ShutdownConnection( lpDPInfo );
	if (DoNewInet&&IPCORE_INIT)
	{
		if (IPCORE.IsServer())IPCORE.DoneServer();
		else IPCORE.DoneClient();
    }
	if (NETWORK_INIT&&Final)
	{
		IPCORE.CloseNetwork();
		NETWORK_INIT = 0;
    }
}

static HRESULT ShutdownConnection( LPDPLAYINFO lpDPInfo )
{
	if (lpDPInfo->lpDirectPlay3A)
	{
		if (lpDPInfo->dpidPlayer)
		{
			lpDPInfo->lpDirectPlay3A->DestroyPlayer( lpDPInfo->dpidPlayer );
			lpDPInfo->dpidPlayer = 0;
		}
		lpDPInfo->lpDirectPlay3A->Close();
		lpDPInfo->lpDirectPlay3A->Release();
		lpDPInfo->lpDirectPlay3A = nullptr;
		lpDirectPlay3A = nullptr;
	}

	if (DoNewInet&&IPCORE_INIT)
	{
		if (IPCORE.IsServer())
		{
			IPCORE.DoneServer();
		}
		else
		{
			IPCORE.DoneClient();
		}
	}

	return ( DP_OK );
}

void InitMultiDialogs()
{
    lpDirectPlay3A = nullptr;
	NProviders = 0;
	PlayerMenuMode = 1;
	NPlayers = 0;
	GameInProgress = false;
}

static HRESULT CreateDirectPlayInterface( LPDIRECTPLAY3A *lplpDirectPlay3A )
{
	HRESULT hr;
	LPDIRECTPLAY3A lpDirectPlay3A = nullptr;
	hr = CoInitialize( nullptr );
	// Create an IDirectPlay3 interface
	hr = CoCreateInstance( CLSID_DirectPlay, nullptr, CLSCTX_INPROC_SERVER,
		IID_IDirectPlay3A, (LPVOID*) &lpDirectPlay3A );

	// return interface created
	*lplpDirectPlay3A = lpDirectPlay3A;

	return ( hr );
}

static BOOL FAR PASCAL DirectPlayEnumConnectionsCallback(
	LPCGUID lpguidSP,
	LPVOID lpConnection,
	DWORD dwConnectionSize,
    LPCDPNAME /*lpName*/,
    DWORD /*dwFlags*/,
    LPVOID /*lpContext*/ )
{
	// store service provider name in list
	//strcat(ProvidersList,lpName->lpszShortNameA);
	//strcat(ProvidersList,"|");
	// make space for connection shortcut
	if (*lpguidSP == DPSPGUID_IPX)
	{
		lplpConnectionBuffer[0] = new byte[dwConnectionSize];
		memcpy( lplpConnectionBuffer[0], lpConnection, dwConnectionSize );
	}

	if (*lpguidSP == DPSPGUID_TCPIP)
	{
		lplpConnectionBuffer[1] = new byte[dwConnectionSize];
		memcpy( lplpConnectionBuffer[1], lpConnection, dwConnectionSize );
	}

	return ( TRUE );
}

bool CreateMultiplaterInterface()
{
	if (DoNewInet)
	{
		if (IPCORE_INIT)CloseMPL();
		if (!NETWORK_INIT)
		{
			bool res = ( IPCORE.InitNetwork() != 0 );
			if (!res)
			{
				LocalGP BOR2( "Interface\\Bor2" );
				WaitWithError( "SOCKERROR", BOR2.GPID );
			}
			else
			{
				NETWORK_INIT = 1;
			}
			return res;
		}
		else
		{
			return 1;
		}
	}

	if FAILED( CreateDirectPlayInterface( &lpDirectPlay3A ) )
	{
		return false;
	}

	if (NProviders > 0)
	{
		for (int i = 0; i < NProviders; i++)
		{
			free( lplpConnectionBuffer[i] );
		}
	}

	lplpConnectionBuffer[0] = nullptr;
	lplpConnectionBuffer[1] = nullptr;
	//ProvidersList[0]=0;
	//NProviders=0;
	lpDirectPlay3A->EnumConnections( &DPCHAT_GUID,
		DirectPlayEnumConnectionsCallback, hwnd, 0 );
	return true;
}

static ListBox* LBBX;

static BOOL FAR PASCAL LBEnumSessionsCallback(
	LPCDPSESSIONDESC2 lpSessionDesc,
    LPDWORD /*lpdwTimeOut*/,
	DWORD dwFlags,
	LPVOID lpContext )
{
	// see if last session has been enumerated
	int ID = int( lpContext );
	if (dwFlags & DPESC_TIMEDOUT)
		return ( FALSE );

	if (ID)
	{
		if (!lpSessionDesc->dwUser2)return true;
	}
	else
	{
		if (lpSessionDesc->dwUser2)return true;
    }
	// store session name in list
	if (lpSessionDesc->dwUser1)return true;
	if (ID)
	{
		char cc1[200];
		sprintf( cc1, "%s (%s)", WARS.Battles[lpSessionDesc->dwUser2 - 1].SmallHeader, lpSessionDesc->lpszSessionNameA );
		LBBX->AddItem( cc1, lpSessionDesc->dwCurrentPlayers + 256 * lpSessionDesc->dwMaxPlayers + 65536 * lpSessionDesc->dwUser2 );
	}
	else
	{
		LBBX->AddItem( lpSessionDesc->lpszSessionNameA, lpSessionDesc->dwCurrentPlayers + 256 * lpSessionDesc->dwMaxPlayers );
	}
	SessionsGUID[NSessions] = lpSessionDesc->guidInstance;
	NSessions++;
	return ( TRUE );
}

int GMTYPE = 0;
static int SSMAXPL = 0;

static BOOL FAR PASCAL SR_EnumSessionsCallback(
	LPCDPSESSIONDESC2 lpSessionDesc,
    LPDWORD /*lpdwTimeOut*/,
	DWORD dwFlags,
	LPVOID lpContext )
{
    char* Name = (char*) lpContext;
	// see if last session has been enumerated
	if (dwFlags & DPESC_TIMEDOUT)
		return ( FALSE );

	if (lpSessionDesc->dwCurrentPlayers >= lpSessionDesc->dwMaxPlayers)
	{
		return true;
	}
	// store session name in list
	if (lpSessionDesc->dwUser1)
	{
		return true;
	}
	if (lpSessionDesc->lpszSessionName
		&& lpSessionDesc->dwCurrentPlayers < lpSessionDesc->dwMaxPlayers
		&& !strcmp( Name, lpSessionDesc->lpszSessionNameA ))
	{
		SessionsGUID[0] = lpSessionDesc->guidInstance;
		NSessions = 1;
		GMTYPE = lpSessionDesc->dwUser2;
		SSMAXPL = lpSessionDesc->dwMaxPlayers;
	}
	return ( TRUE );
}

static HRESULT JoinSession( LPDIRECTPLAY3A lpDirectPlay3A, LPGUID lpguidSessionInstance,
	LPSTR lpszPlayerName, LPDPLAYINFO lpDPInfo );

static void NORMNICK1( char* Nick )
{
	int L = strlen( Nick );
	if (L > 3)
	{
		if (Nick[L - 1] == '}' && Nick[L - 3] == '{')
		{
			Nick[L - 3] = 0;
		}
	}
}

bool FindSessionAndJoin( char* Name, char* Nick, bool Style, unsigned short port )
{
	NORMNICK1( Nick );
	if (Style)
	{
		if (!NETWORK_INIT)
		{
			IPCORE.InitNetwork();
			NETWORK_INIT = 1;
		}

		if (IPCORE.InitClient( IPADDR, Nick, port ))
		{
			//IPCORE.SetUserName(Nick);
			MyDPID = IPCORE.GetPeerID();
			GMTYPE = IPCORE.GetOptions();
			//if(GMTYPE!=1)GMTYPE=0;
			IPCORE_INIT = 1;
			return true;
		}
		return false;
	}

	DPSESSIONDESC2 sessionDesc;
	SessionsList[0] = 0;
	NSessions = 0;
	// add sessions to session list
	ZeroMemory( &sessionDesc, sizeof( DPSESSIONDESC2 ) );
	sessionDesc.dwSize = sizeof( DPSESSIONDESC2 );
	sessionDesc.guidApplication = DPCHAT_GUID;
	// start enumerating the sessions
	int T0 = GetTickCount();
	do
	{
		lpDirectPlay3A->EnumSessions( &sessionDesc, 0,
			SR_EnumSessionsCallback, LPVOID( Name ),
			DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_ASYNC );
		ProcessMessages();
	} while (!( NSessions || GetTickCount() - T0 > 10000 ));

	if (NSessions)
	{
		if FAILED( JoinSession( lpDirectPlay3A,
			SessionsGUID, Nick,
			lpDPInfo ) )
		{
			lpDirectPlay3A->EnumSessions( &sessionDesc, 0,
				SR_EnumSessionsCallback, LPVOID( Name ),
				DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_STOPASYNC );
			return false;
		}

		lpDirectPlay3A->EnumSessions( &sessionDesc, 0,
			SR_EnumSessionsCallback, LPVOID( Name ),
			DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_STOPASYNC );

		PIEnumeratePlayers( PINFO, 0 );

		if (NPlayers > SSMAXPL)
		{
			lpDirectPlay3A->Close();
			return false;
		}

	}
	else
	{
		return false;
	}
	return true;
}

void LBEnumerateSessions( ListBox* LB, int ID )
{
	if (!lpDirectPlay3A)CreateMultiplaterInterface();
	LBBX = LB;
	LB->ClearItems();
	DPSESSIONDESC2 sessionDesc;
	PlayersList[0] = 0;
	NPlayers = 0;
	SessionsList[0] = 0;
	NSessions = 0;
	// add sessions to session list
	ZeroMemory( &sessionDesc, sizeof( DPSESSIONDESC2 ) );
	sessionDesc.dwSize = sizeof( DPSESSIONDESC2 );
	sessionDesc.guidApplication = DPCHAT_GUID;
	// start enumerating the sessions
	lpDirectPlay3A->EnumSessions( &sessionDesc, 0,
		LBEnumSessionsCallback, LPVOID( ID ),
		DPENUMSESSIONS_AVAILABLE );
}

static HRESULT HostSession( LPDIRECTPLAY3A lpDirectPlay3A,
	LPSTR lpszSessionName, LPSTR lpszPlayerName,
	LPDPLAYINFO lpDPInfo, DWORD User2 )
{
	DPID dpidPlayer;
	DPNAME dpName;
	DPSESSIONDESC2 sessionDesc;
	HRESULT hr;

	// check for valid interface
	if (lpDirectPlay3A == nullptr)
		return ( DPERR_INVALIDOBJECT );

	// host a new session
	ZeroMemory( &sessionDesc, sizeof( DPSESSIONDESC2 ) );
	sessionDesc.dwSize = sizeof( DPSESSIONDESC2 );
	sessionDesc.dwFlags = DPSESSION_MIGRATEHOST | DPSESSION_KEEPALIVE;
	sessionDesc.guidApplication = DPCHAT_GUID;
	if (User2)sessionDesc.dwMaxPlayers = 2;
	else sessionDesc.dwMaxPlayers = MAXPLAYERS;
	sessionDesc.lpszSessionNameA = lpszSessionName;
	sessionDesc.dwUser2 = User2;

	hr = lpDirectPlay3A->Open( &sessionDesc, DPOPEN_CREATE );
	if FAILED( hr )
		goto OPEN_FAILURE;

	// fill out name structure
	ZeroMemory( &dpName, sizeof( DPNAME ) );
	dpName.dwSize = sizeof( DPNAME );
	dpName.lpszShortNameA = lpszPlayerName;
	dpName.lpszLongNameA = nullptr;

	// create a player with this name
	hr = lpDirectPlay3A->CreatePlayer( &dpidPlayer, &dpName, 0, nullptr, 0, 0 );
	if FAILED( hr )
		goto CREATEPLAYER_FAILURE;
	MyDPID = dpidPlayer;
	// return connection info
	lpDPInfo->lpDirectPlay3A = lpDirectPlay3A;
	lpDPInfo->dpidPlayer = dpidPlayer;
	lpDPInfo->bIsHost = TRUE;

	return ( DP_OK );

CREATEPLAYER_FAILURE:
OPEN_FAILURE:
	lpDirectPlay3A->Close();
	lpDirectPlay3A = nullptr;
	return ( hr );
}

void StopConnectionToSession( LPDIRECTPLAY3A lpDirectPlay3A )
{
	if (DoNewInet)
	{
		IPCORE.CloseSession();
    }
	if (!lpDirectPlay3A)return;
	DPSESSIONDESC2 sessionDesc[4];
	DWORD SDSize = sizeof( sessionDesc );
	lpDirectPlay3A->GetSessionDesc( nullptr, &SDSize );
	HRESULT hr = lpDirectPlay3A->GetSessionDesc( sessionDesc, &SDSize );
    if (hr != DP_OK) return;
	sessionDesc->dwMaxPlayers = sessionDesc->dwCurrentPlayers;
	sessionDesc->dwUser1 = 1;
	hr = lpDirectPlay3A->SetSessionDesc( sessionDesc, 0 );
	/*
	switch(hr){
	case DPERR_ACCESSDENIED:
	 v=0;
	 break;
	case DPERR_INVALIDPARAMS :
	 v=1;
	 break;
	case DPERR_NOSESSIONS:
	 v=2;
	 break;
    }
	*/
}

static HRESULT JoinSession( LPDIRECTPLAY3A lpDirectPlay3A,
	LPGUID lpguidSessionInstance, LPSTR lpszPlayerName,
	LPDPLAYINFO lpDPInfo )
{
	DPID dpidPlayer;
	DPNAME dpName;
	DPSESSIONDESC2 sessionDesc;
	HRESULT hr;

	// check for valid interface
	if (lpDirectPlay3A == nullptr)
		return ( DPERR_INVALIDOBJECT );

	// join existing session
	ZeroMemory( &sessionDesc, sizeof( DPSESSIONDESC2 ) );
	sessionDesc.dwSize = sizeof( DPSESSIONDESC2 );
	sessionDesc.guidInstance = *lpguidSessionInstance;

	hr = lpDirectPlay3A->Open( &sessionDesc, DPOPEN_JOIN );
	if FAILED( hr )
		goto OPEN_FAILURE;

	// fill out name structure
	ZeroMemory( &dpName, sizeof( DPNAME ) );
	dpName.dwSize = sizeof( DPNAME );
	dpName.lpszShortNameA = lpszPlayerName;
	dpName.lpszLongNameA = nullptr;

	// create a player with this name
	hr = lpDirectPlay3A->CreatePlayer( &dpidPlayer, &dpName, 0, nullptr, 0, 0 );
	if FAILED( hr )
		goto CREATEPLAYER_FAILURE;

	// return connection info
	MyDPID = dpidPlayer;
	lpDPInfo->lpDirectPlay3A = lpDirectPlay3A;
	lpDPInfo->dpidPlayer = dpidPlayer;
	lpDPInfo->bIsHost = FALSE;

	return ( DP_OK );

CREATEPLAYER_FAILURE:
OPEN_FAILURE:
	lpDirectPlay3A->Close();
	lpDirectPlay3A = nullptr;
	return ( hr );
}

bool CreateSession( char* SessName, char* Name, uint32_t User2, bool Style, int MaxPlayers )
{
	NORMNICK1( Name );
	// use computer name for session name
    // host a new session on this service provider
	if (DoNewInet && !NETWORK_INIT)
	{
		IPCORE.InitNetwork();
		NETWORK_INIT = 1;
    }
	if (Style)
	{
		IPCORE.SetOptions( User2 );
		char SESS[256];
		strcpy( SESS, SessName );
		SESS[28] = 0;
		bool res = ( IPCORE.InitServer( SESS, Name ) != 0 );
		if (res)
		{
			if (User2)IPCORE.SetMaxPeers( 2 );
			else IPCORE.SetMaxPeers( MaxPlayers );
			//IPCORE.SetUserName(Name);
			MyDPID = IPCORE.GetPeerID();
			IPCORE_INIT = 1;
        }
		return res;
	}
	else
	{
		HRESULT hr = HostSession( lpDirectPlay3A, SessName, Name, lpDPInfo, User2 );
		if FAILED( hr )return false;
		else return true;
	}
}

bool CreateNamedSession( char* Name, DWORD User2, int Max )
{
	// use computer name for session name
	if (DoNewInet && !NETWORK_INIT)
	{
		IPCORE.InitNetwork();
		NETWORK_INIT = 1;
    }
	DWORD dwNameSize = MAXNAMELEN;
	if (!GetComputerName( szSessionName, &dwNameSize ))
		lstrcpy( szSessionName, "Session" );
	// host a new session on this service provider
	if (DoNewInet)
	{
		IPCORE.SetOptions( User2 );
		IPCORE.SetMaxPeers( Max );
		bool r = ( IPCORE.InitServer( Name, Name ) != 0 );
		if (r)IPCORE_INIT = 1;
		return r;
	}
	else
	{
		HRESULT hr = HostSession( lpDirectPlay3A, szSessionName, Name, lpDPInfo, User2 );
		if FAILED( hr )return false;
		else return true;
	}
}

bool JoinNameToSession( int ns, char* Name )
{
	if (ns >= NSessions)return false;
	if FAILED( JoinSession( lpDirectPlay3A,
		&SessionsGUID[ns], Name,
		lpDPInfo ) )return false;
	else return true;
}

struct NetCell
{
	byte* Data;
	int size;
	DWORD SendTime;
	DPID idTo;
};

class NetCash
{
public:
	int NCells;
	int MaxCells;
	NetCell* CELLS;
	NetCash();
	~NetCash();
	void Add( byte* Data, int size, DPID idTo );
	void AddOne( byte* Data, int size, DPID idTo );
	void AddWithDelay( byte* Data, int size, DPID idTo, int TimeDelay );
	void Process();
};

NetCash::NetCash()
{
	NCells = 0;
	MaxCells = 0;
	CELLS = nullptr;
}

NetCash::~NetCash()
{
	for (int i = 0; i < NCells; i++)free( CELLS[i].Data );
	if (CELLS)free( CELLS );
}

static bool SendToAllPlayersEx( DWORD Size, LPVOID lpData, bool G )
{
	if (( !DoNewInet ) && ( ( !int( lpDirectPlay3A ) ) || NPlayers < 2 ))return false;
	int count = 0;
	HRESULT hr;
    do
	{
		if (DoNewInet)
		{
			hr = IPCORE.SendToAll( (byte*) lpData, Size, G );
			if (hr)hr = DP_OK;
			else hr = 1;
		}
		else
		{
			if (G)hr = lpDirectPlay3A->Send( MyDPID, DPID_ALLPLAYERS, DPSEND_GUARANTEED, lpData, Size );
			else hr = lpDirectPlay3A->Send( MyDPID, DPID_ALLPLAYERS, 0, lpData, Size );
        }
		if (hr == DP_OK)
		{
			int tt = GetTickCount();
			if (!PREVGRAPHST)
			{
				PREVGRAPHST = tt;
            }
			PREVGRAPHSSZ += ( Size + 32 )*( NPlayers - 1 );
			NSENDP++;
			if (Size > MAXSENDP)MAXSENDP = Size;
			if (tt - PREVGRAPHST > 1000)
			{
				PREVGRAPHST = tt;
				PREVGRAPHSSZ = 0;
				MAXSENDP = 0;
				NSENDP = 0;
            }
        }
		ProcessMessagesEx();
		count++;
	} while (hr != DP_OK&&count < 1);
	if (hr != DP_OK)return false;
	return true;
}

static bool SendToAllPlayersExNew( DWORD Size, LPVOID lpData, bool G )
{
	if (( !DoNewInet ) && ( ( !int( lpDirectPlay3A ) ) || NPlayers < 2 ))return false;
	//-------
	int Type = 0;
	if (Size >= 7)
	{
		byte rt = RealTime;
		byte* BUF = (byte*) lpData;
		if (rt == BUF[3])Type = 2;
		rt--;
		if (rt == BUF[3])Type = 1;
    }
	//-------
	int count = 0;
	HRESULT hr;
    if (DoNewInet)
	{
		hr = IPCORE.SendToAll( (byte*) lpData, Size, G );
		if (hr)hr = DP_OK;
		else hr = 1;
	}
	else
	{
		if (G)hr = lpDirectPlay3A->Send( MyDPID, DPID_ALLPLAYERS, DPSEND_GUARANTEED, lpData, Size );
		else hr = lpDirectPlay3A->Send( MyDPID, DPID_ALLPLAYERS, 0, lpData, Size );
    }
	if (hr == DP_OK)
	{
		if (Type == 1)NSEN1++;
		if (Type == 2)NSEN2++;
		int tt = GetTickCount();
		if (!PREVGRAPHST)
		{
			PREVGRAPHST = tt;
        }
		PREVGRAPHSSZ += ( Size + 32 )*( NPlayers - 1 );
		NSENDP++;
		if (Size > MAXSENDP)
		{
			MAXSENDP = Size;
		}
		if (tt - PREVGRAPHST > 1000)
		{
			PREVGRAPHST = tt;
			PREVGRAPHSSZ = 0;
			MAXSENDP = 0;
			NSENDP = 0;
        }
	}
	else
	{
		if (hr == DPERR_BUSY)
		{
			if (Type == 1)NSENB1++;
			if (Type == 2)NSENB2++;
		}
		else
		{
			if (Type == 1)NSENF1++;
			if (Type == 2)NSENF2++;
        }
    }
	ProcessMessagesEx();
	count++;
	if (hr == DPERR_BUSY)return false;
	else return true;
}

static bool SendToPlayerEx( DWORD Size, LPVOID lpData, DWORD DPID )
{
	if (( !DoNewInet ) && ( ( !int( lpDirectPlay3A ) ) || NPlayers < 2 ))return false;
	int count = 0;
	HRESULT hr;
	do
	{
		if (DoNewInet)
		{
			hr = IPCORE.SendToPeer( DPID, (byte*) lpData, Size, 0 );
			if (hr)hr = DP_OK;
			else hr = 1;
		}
		else hr = lpDirectPlay3A->Send( MyDPID, DPID, 0, lpData, Size );
		if (hr == DP_OK)
		{
			int tt = GetTickCount();
			if (!PREVGRAPHST)
			{
				PREVGRAPHST = tt;
            }
			PREVGRAPHSSZ += Size + 32;
			NSENDP++;
			if (tt - PREVGRAPHST > 1000)
			{
				PREVGRAPHST = tt;
				PREVGRAPHSSZ = 0;
				MAXSENDP = 0;
				NSENDP = 0;
            }
        }
		count++;
		ProcessMessagesEx();
	} while (hr != DP_OK&&count < 1);
	if (hr != DP_OK)return false;
	return true;
}

static bool SendToPlayerExNew( DWORD Size, LPVOID lpData, DWORD DPID )
{
	if (( !DoNewInet ) && ( ( !int( lpDirectPlay3A ) ) || NPlayers < 2 ))return false;
	int count = 0;
	HRESULT hr;
	if (DoNewInet)
	{
		hr = IPCORE.SendToPeer( DPID, (byte*) lpData, Size, 0 );
		if (hr)hr = DP_OK;
		else hr = 1;
	}
	else hr = lpDirectPlay3A->Send( MyDPID, DPID, 0, lpData, Size );
	if (hr == DP_OK)
	{
		int tt = GetTickCount();
		if (!PREVGRAPHST)
		{
			PREVGRAPHST = tt;
        }
		PREVGRAPHSSZ += Size + 32;
		NSENDP++;
		if (tt - PREVGRAPHST > 1000)
		{
			PREVGRAPHST = tt;
			PREVGRAPHSSZ = 0;
			MAXSENDP = 0;
			NSENDP = 0;
        }
    }
	ProcessMessagesEx();
	count++;
	if (hr == DPERR_BUSY)
		return false;
	else
		return true;
}

void NetCash::AddOne( byte* Data, int size, DPID idTo )
{
	int idf = 0;
	int idt = 0;
	for (int i = 0; i < NPlayers; i++)
	{
		if (idTo == PINFO[i].PlayerID)idt = i;
		if (MyDPID == PINFO[i].PlayerID)idf = i;
	}

	if (NCells >= MaxCells)
	{
		MaxCells += 100;
        CELLS = (NetCell*) realloc( CELLS, MaxCells * sizeof( NetCell) );
	}

	int T0 = int( 250 * ( sin( float( GetTickCount() ) / 20000 ) + 1 ) ) + ( idt + idf ) * 95;
	if (!T0)
	{
		T0 = 1;
	}

	CELLS[NCells].Data = new byte[size];
	memcpy( CELLS[NCells].Data, Data, size );
	CELLS[NCells].size = size;
	CELLS[NCells].idTo = idTo;
	CELLS[NCells].SendTime = GetRealTime() + T0 + ( srando() % T0 );
	NCells++;
}

void NetCash::AddWithDelay( byte* Data, int size, DPID idTo, int dt )
{
	if (NCells >= MaxCells)
	{
		MaxCells += 100;
        CELLS = (NetCell*) realloc( CELLS, MaxCells * sizeof( NetCell) );
	}

	CELLS[NCells].Data = new byte[size];
	memcpy( CELLS[NCells].Data, Data, size );
	CELLS[NCells].size = size;
	CELLS[NCells].idTo = idTo;
	CELLS[NCells].SendTime = GetRealTime() + dt;
	NCells++;
}

void NetCash::Add( byte* Data, int size, DPID idTo )
{
	if (idTo == DPID_ALLPLAYERS)
	{
		for (int i = 0; i < NPlayers; i++)
		{
			if (PINFO[i].PlayerID != MyDPID)
			{
				AddOne( Data, size, PINFO[i].PlayerID );
			}
		}
	}
	else
	{
		AddOne( Data, size, idTo );
	}
}

void NetCash::Process()
{
	unsigned long t0 = GetRealTime();
	for (int i = 0; i < NCells; i++)
	{
		if (CELLS[i].SendTime < t0)
		{
			bool done;
			if (CELLS[i].idTo)
			{
				done = SendToPlayerExNew( CELLS[i].size, CELLS[i].Data, CELLS[i].idTo );
			}
			else
			{
				done = SendToAllPlayersExNew( CELLS[i].size, CELLS[i].Data, 0 );
			}

			if (done)
			{
				free( CELLS[i].Data );
				if (i < NCells - 1)
				{
                    memcpy( CELLS + i, CELLS + i + 1, ( NCells - i - 1 ) * sizeof( NetCell) );
				}
				NCells--;
				i--;
			}
		}
	}
}

static NetCash NCASH;

void ProcessNetCash()
{
	NCASH.Process();
}

bool SendToAllPlayers( uint32_t Size, void * lpData )
{
	if (( !DoNewInet ) && ( ( !int( lpDirectPlay3A ) ) || NPlayers < 2 ))
		return false;

	if (!SendToAllPlayersExNew( Size, lpData, 0 ))
	{
		NCASH.AddWithDelay( (byte*) lpData, Size, DPID_ALLPLAYERS, 0 );
	}

	return true;
}

static bool SendToAllPlayersWithDelay( DWORD Size, LPVOID lpData, int dt )
{
	if (( !DoNewInet ) && ( ( !int( lpDirectPlay3A ) ) || NPlayers < 2 ))
		return false;

	NCASH.AddWithDelay( (byte*) lpData, Size, 0, dt );
	//SendToAllPlayersEx(Size,lpData,0);

	return true;
}

static bool SendToAllPlayers( uint32_t Size, void * lpData, bool G )
{
	if (( !DoNewInet ) && ( ( !int( lpDirectPlay3A ) ) || NPlayers < 2 ))
		return false;

	SendToAllPlayersEx( Size, lpData, G );

	return true;
}

static bool SendToPlayer( DWORD Size, LPVOID lpData, DWORD DPID )
{
	if (( !DoNewInet ) && ( ( !int( lpDirectPlay3A ) ) || NPlayers < 2 ))
		return false;

	SendToPlayerEx( Size, lpData, DPID );

	return true;
}

static void FreePDatas()
{
	for (int i = 0; i < NPlayers; i++)
	{
		if (int( PData[i] ))
		{
			free( PData[i] );
		}
		PData[i] = nullptr;
	}
}

static void ComeInGame()
{
	ShowLoading();

	int myid = 0;
	for (int i = 0; i < NPlayers; i++)
	{
		if (MyDPID == PlayersID[i])
		{
			myid = i;
		}
	}

	FlipPages();

	if (CurrentMap[0] == 'R' && CurrentMap[1] == 'N' && CurrentMap[3] == ' ')
	{
		CreateNationalMaskForRandomMap( CurrentMap );
	}
	else
	{
		if (strstr( CurrentMap, ".sav" ) || strstr( CurrentMap, ".SAV" ))
		{
			CreateMaskForSaveFile( CurrentMap );
		}
		else
		{
			CreateNationalMaskForMap( CurrentMap );
		}
	}

	byte NRTBL[8];
	memcpy( NRTBL, NatRefTBL, 8 );

	PrepareGameMedia( myid, 1 );

	memcpy( NatRefTBL, NRTBL, 8 );

	CenterScreen();

	FlipPages();

	//-----------
	EBPos = 0;
	EBPos1 = 5;

	SetupEBufs();

	RealTime = 0;
	GLOBALTIME = 0;
	PGLOBALTIME = 0;

	byte* BUF = (byte*) ExBuf1;
	BUF[0] = 0xAE;
	BUF[1] = 0;
	BUF[2] = 0;
	BUF[3] = 0;
	BUF[4] = 0;
	BUF[5] = 0;
	BUF[6] = 0;
	BUF[7] = 81;
	*( (int*) ( BUF + 8 ) ) = GetRealTime() - NeedCurrentTime + 4000;

	word s = 0;
	for (int i = 0; i < 12; i++)
	{
		s += BUF[i];
	}
	( (word*) ( BUF + 1 ) )[0] = s;

	rpos = 0;

	if (lpDirectPlay3A)
	{
		SendToAllPlayers( 12, ExBuf1, 1 );
	}

	memcpy( PrevEB, ExBuf1, 12 );
	PrevEBSize = 12;
	PrevPrevEBSize = 0;
    PlayerMenuMode = 0;
}

static int StartTime[8];

static void CreateStartTime()
{
	memset( StartTime, 0, sizeof StartTime );
	for (int i = 0; i < NPlayers; i++)
	{
		DWORD DPID = PINFO[i].PlayerID;
		if (DPID != MyDPID)
		{
			int dt = PSUMM.GetTimeDifference( PINFO[i].PlayerID );
			//StartTime[PINFO[i].ColorID]=GetRealTime()+dt;
			StartTime[i] = GetRealTime() + dt;//!!!CHANGED!!!
		}
		else
		{
			//StartTime[PINFO[i].ColorID]=GetRealTime();
			StartTime[i] = GetRealTime();//!!!CHANGED!!!
		}
	}
}

static bool CheckSender();

static void ReCreateStartTime()
{
	if (!( CheckSender() && CheckPingsReady() ))
	{
		return;
	}
	int OLDSTART[8];
	memset( OLDSTART, 0, sizeof StartTime );
    int ST0 = 0;
	for (int i = 0; i < NPlayers; i++)
	{
		DWORD DPID = PINFO[i].PlayerID;
		if (DPID == MyDPID)
		{
			//ST0=StartTime[PINFO[i].ColorID];
			ST0 = StartTime[i];//!!!CHANGED!!!
		}
	}
	for (int i = 0; i < NPlayers; i++)
	{
        //int c=PINFO[i].ColorID;
		int c = i;//!!!CHANGED!!!
		int dt = PSUMM.GetTimeDifference( PINFO[i].PlayerID );
		int dt0 = StartTime[c] - ST0;
		OLDSTART[c] = dt0 - dt;
	}
	CmdSetStartTime( OLDSTART );
	PSUMM.ClearPingInfo();
}

static bool CheckSender()
{
	if (NPlayers < 2)
	{
		return 0;
	}
	DPID MINVAL = 0xFFFFFFFF;
	int MINC = 9;
	for (int i = 0; i < NPlayers; i++)
	{
		if (EBufs[i].Enabled)
		{
			if (MINC > PINFO[i].ColorID)
			{
				MINC = PINFO[i].ColorID;
				MINVAL = PINFO[i].PlayerID;
			}
		}
	}
	return MINVAL == MyDPID;
}

uint16_t COMPSTART[8];

static uint8_t MyRace;

bool StartIGame( bool SINGLE )
{
	memcpy( NatRefTBL, MPL_NatRefTBL, 8 );

	CreateStartTime();

	for (int j = 0; j < NPlayers; j++)
	{
		if (PINFO[j].PlayerID == MyDPID)
		{
			NeedCurrentTime = StartTime[j];
		}
	}

	/*
	65*4=260 bytes big buffer for lpData in SendToAllPlayers()
	Contents:
	index offset size
	[ 0] 0 start game code 4 bytes
	[ 1] 4 MyRace 4 bytes
	[ 2] 8 CurrentMap 32 bytes
	[10] 40 MPL_NatRefTBL 8 bytes unnecessary?
		 42 COMPINFO 16 bytes overwrites MPL_NatRefTBL!
	[11]
		 46 ColorID, GroupID 16 bytes overwrites COMPINFO!
	[16] 64 MyDPID 4 bytes
	[17] 68 NPlayers 1 byte
	[18] 72 PlayersID[0 to 22] 88 bytes
	[50] 200 StartTime 32 bytes
	[58] 232 MaxPingTime 4 bytes
	[59] 236 lpdata_buf_checksum 4 bytes
	*/
	DWORD BUF[48 + 8 + 8 + 1];
	memset( BUF, 0, sizeof BUF );

	PrepareToGame();

	HideFlags();

	if (( !SINGLE ) && NPlayers < 2)
	{
		return false;
	}
	//BUF[0]=.....
	//BUF[1]=Nation
	//BUF[2],[3],[4],[5],[6],[7]-map name
	ServerDPID = MyDPID;
	BUF[0] = 0x037242F3;//start game code
	BUF[1] = MyRace;
	for (int i = 2; i < 8; i++)
	{
		BUF[i] = 0;
	}
	strcpy( (char*) &BUF[2], CurrentMap );
	BUF[8 + 8] = MyDPID;
	BUF[9 + 8] = NPlayers;
	memcpy( &BUF[10 + 8], PlayersID, ( 32 - 10 ) * 4 );
	byte* BUFB = (byte*) ( BUF + 10 + 8 + 32 - 10 );
	memcpy( BUFB, MPL_NatRefTBL, 8 );

	BUFB = (byte*) ( BUF + 10 + 8 + 32 - 8 );

	int HostID = 0;
	for (int i = 0; i < NPlayers; i++)
	{
		if (PINFO[i].Host)
		{
			HostID = i;
		}
	}

	//memcpy( BUFB, PINFO[HostID].MapName + 44 + 16, 16 );//FUNNY: Never do sth like this. Please.
	memcpy( BUFB, PINFO[HostID].COMPINFO, 16 );//BUGFIX: Dependence on PlayerInfo memory alignment

	memcpy( COMPSTART, BUFB, 16 );

	BUFB = (byte*) ( BUF + 10 + 8 + 32 - 4 );
	for (int p = 0; p < 8; p++)
	{
		BUFB[p + p] = PINFO[p].ColorID;
		BUFB[p + p + 1] = PINFO[p].GroupID;
	}

	memcpy( &BUF[10 + 8 + 32], StartTime, 8 * 4 );
	BUF[10 + 8 + 32 + 8] = MaxPingTime;

	DWORD SUM = 0;
	for (int i = 0; i < 42 + 8 + 8 + 1; i++)
	{
		SUM += BUF[i];
	}
	BUF[42 + 8 + 8 + 1] = SUM;

	FreePDatas();

	if (SINGLE || ( ( !SINGLE ) && SendToAllPlayers( ( 43 + 8 + 8 + 1 ) * 4, BUF, 1 ) ))
	{
		int ttm = GetRealTime();
		do
		{
		} while (GetRealTime() - ttm < 400);

		GameInProgress = true;

		PlayerMenuMode = 0;

		ComeInGame();

		tmtmt = 0;
		RealTime = 0;
		GLOBALTIME = 0;
		PGLOBALTIME = 0;

		if (strstr( CurrentMap, ".sav" ) || strstr( CurrentMap, ".SAV" ))
		{
			strcpy( SaveFileName, CurrentMap );
			CmdLoadNetworkGame( MyNation, 0, CurrentMap );
		}

		return true;
	}
	else
	{
		return false;
	}
}

void AnalyseMessages()
{
    LockPing = ( GetRealTime() - PREVTIME ) > 50;
	PREVTIME = GetRealTime();
	int CC = 0;
	do
	{
		CC++;
		ReceiveMessage( lpDPInfo );
		if (MyDPID != ServerDPID)
		{
			DWORD* lp = (DWORD*) MyData;
			if (lp)
			{
				if (lp[0] == 0x037242F3 && !GameInProgress)
				{
					DWORD SUM = 0;
					for (int i = 0; i < ( 42 + 8 + 8 + 1 ); i++)
					{
						SUM += lp[i];
					}
					if (SUM == lp[42 + 8 + 8 + 1])
					{
						//start game
						ServerDPID = lp[8 + 8];
						NPlayers = word( lp[9 + 8] );
						memcpy( PlayersID, &lp[10 + 8], ( 32 - 10 ) * 4 );
						byte* BUFB = (byte*) ( lp + 10 + 8 + 32 - 10 );
						memcpy( NatRefTBL, BUFB, 8 );
						BUFB = (byte*) ( lp + 10 + 8 + 32 - 8 );
						memcpy( COMPSTART, BUFB, 16 );
						BUFB = (byte*) ( lp + 10 + 8 + 32 - 4 );
						strcpy( CurrentMap, (char*) &lp[2] );
						memcpy( StartTime, &lp[10 + 8 + 32], 4 * 8 );
						MaxPingTime = lp[10 + 8 + 32 + 8];

						if (MaxPingTime)
						{
							PitchTicks = 8;
						}
						else
						{
							PitchTicks = 0;
						}

						PlayerInfo PINFO2[8];
						memcpy( PINFO2, PINFO, sizeof PINFO );
                        for (int j = 0; j < NPlayers; j++)
						{
							DPID id = PlayersID[j];
							for (int i = 0; i < NPlayers&&id; i++)if (PINFO2[i].PlayerID == id)
							{
                                memcpy( PINFO + j, PINFO2 + i, sizeof( PlayerInfo) );
								id = 0;
							}
						}
						for (int j = 0; j < 8; j++)
						{
							PINFO[j].ColorID = BUFB[j + j];
							PINFO[j].GroupID = BUFB[j + j + 1];

						}
						for (int i = 0; i < NPlayers; i++)if (PINFO[i].PlayerID == MyDPID)
						{
							NeedCurrentTime = StartTime[i];
						}
                        GameInProgress = true;
						PlayerMenuMode = 0;
						ComeInGame();
					}
				}
			}
		}
	} while (NeedMoreReceive);
}

void IAmLeft()
{
	if (DoNewInet)
	{
		if (IPCORE.IsServer() || IPCORE.IsClient())
		{
            SendToAllPlayers( 4, (void*) &PlExitID, 1 );
		}
	}
	else
	{
		if (lpDirectPlay3A)
		{
            SendToAllPlayers( 4, (void*) &PlExitID, 1 );
		}
	}
}

static void RetryNet( bool GUAR );

static void ShowCString( int x, int y, const char* cc, lpRLCFont f )
{
	ShowString( x - ( GetRLCStrWidth( cc, f ) >> 1 ), y, cc, f );
}

static int PREVPINGT = 0;
int PREVGLOBALTIME = 0;
bool LockFog = 0;
static const char* READYTX = nullptr;
static const char* txREADYTX = "READYTX";
static const char* LOADTX = nullptr;
static const char* txLOADTX = "LOADTX";
static const char* NOANSTX = nullptr;
static const char* txNOANSTX = "NOANSTX";
static const char* LOOKSTX = nullptr;
static const char* txLOOKSTX = "LOOKSTX";
static const char* NOPLAYTX = nullptr;
static const char* txNOPLAYTX = "NOPALTX";
static const char* DISCNTX = nullptr;
static const char* txDISCNTX = "DISCNTX";
static const char* ALTTAB = nullptr;
static const char* txALTTAB = "ALT-TAB";

void HandleMultiplayer()
{
    if (PlayGameMode && MaxPingTime)
	{
		if (!PREVGLOBALTIME)
		{
			PREVGLOBALTIME = GLOBALTIME;
		}
		else
		{
			if (GLOBALTIME - PREVGLOBALTIME > 30)
			{
				PREVGLOBALTIME = GLOBALTIME;
			}
		}
	}

	if (!( lpDirectPlay3A || DoNewInet ))
	{
		return;
	}

	int GRTBW = GetRealTime();

	if (( !PREVPINGT ) || ( GetRealTime() - PREVPINGT ) > 1000)
	{
		SendPings();
		PREVPINGT = GetRealTime();
	}

	if (MaxPingTime)
	{
		if (!PREVGLOBALTIME)
		{
			PREVGLOBALTIME = GLOBALTIME;
		}
		else
		{
			if (GLOBALTIME - PREVGLOBALTIME > 30)
			{
				int p = GetMaxRealPing();
				if (p)
				{
					if (p < 300)p = 300;
					CmdSetMaxPingTime( p );
				}
				PREVGLOBALTIME = GLOBALTIME;
				ReCreateStartTime();
			}
		}
	}

	CurStatus = 1;
	if (NPlayers < 2)
	{
		return;
	}

	DWORD ii = GetPIndex( MyDPID );
    if ( ii >= NPlayers || !EBufs[ii].Enabled)
	{
		return;
	}

	EBufs[ii].RealTime = RealTime;
	EBufs[ii].RandIndex = PrevRpos;
	memcpy( &EBufs[ii].Data, ( (byte*) ExBuf1 ) + 7, EBPos1 );
	EBufs[ii].Size = EBPos1;
	int mpl_time_4 = GetRealTime();

	int mpl_time_1;
	int mpl_time_2;
	int mpl_time_3;

	mpl_time_3 = mpl_time_4;
	int RetryAttempts = 0;
	GFILE* f = nullptr;

	if (!READYTX)
	{
		READYTX = GetTextByID( txREADYTX );
		LOADTX = GetTextByID( txLOADTX );
		NOANSTX = GetTextByID( txNOANSTX );
		LOOKSTX = GetTextByID( txLOOKSTX );
		NOPLAYTX = GetTextByID( txNOPLAYTX );
		DISCNTX = GetTextByID( txDISCNTX );
		ALTTAB = GetTextByID( txALTTAB );
	}

	for (int i = 0; i < NPlayers; i++)
	{
		PlState[i] = -1;
	}

	LockPing = ( GetRealTime() - PREVTIME ) > 50;
	PREVTIME = GetRealTime();

	if (LastKey == VK_F3)
	{
		KeyPressed = 0;
	}

	ClearFAILS();
	CPinger PINGS;
	int T0 = PREVTIME;
	bool PStart = 0;
	int FPT0 = 0;
	int NPRECV = 0;
	do
	{
		do
		{
			if (DoNewInet)
			{
				int T1 = GetTickCount();
				if (T1 - T0 > 15000 && !PStart)
				{
					PINGS.InitNetwork();
					PINGS.SetTargetName( "peerchat.gamespy.com" );
					FPT0 = T1;
					PINGS.SendEcho();
					PStart = 1;
				}

				if (PStart&&T1 - FPT0 > 1000)
				{
					FPT0 = T1;
				}

				if (PStart)
				{
					byte TRY = 0;
					byte REC = 0;
					PINGS.RecvEcho();
					PINGS.GetStatistic( &REC, &TRY );
					NPRECV = REC;
				}
			}

			ReceiveMessage( lpDPInfo );

			mpl_time_1 = ( GetRealTime() - mpl_time_4 ) >> 6;
			mpl_time_2 = ( GetRealTime() - mpl_time_3 ) >> 6;

			ProcessMessages();

			if (20 < mpl_time_2 || ( 2 < mpl_time_2  && mpl_time_3 == mpl_time_4 ))
			{
				RetryNet( 60 < mpl_time_2 );
				RetryAttempts++;

				mpl_time_3 = GetRealTime();

                //DWORD ANSW = 'ALIV';
                DWORD ANSW = ConvCosChar( 'A', 'L', 'I', 'V' );
				byte mask = 0;
				for (int i = 0; i < NPlayers; i++)
				{
					if (EBufs[i].Enabled && NON == EBufs[i].RealTime)
					{
						SendToPlayer( 4, &ANSW, PlayersID[i] );
						mask |= 1 << i;
					}
				}

				if (mask)
				{
					byte GETIN[7];
					GETIN[0] = 0xBA;
					GETIN[1] = mask;
					GETIN[2] = mask;
					*( (DWORD*) ( GETIN + 3 ) ) = RealTime;
					for (int i = 0; i < NPlayers; i++)
					{
						if (EBufs[i].Enabled && NON != EBufs[i].RealTime)
						{
							SendToPlayer( 7, GETIN, PlayersID[i] );
						}
					}
				}
			}

			LockFog = 1;
			if (PresentEmptyBuf())
			{
				ProcessScreen();
			}
			LockFog = 0;

			if (80 < mpl_time_1)
			{
				if (!f)
				{
					f = Gopen( "dump.txt", "w" );
				}

				int xc = RealLx / 2;
				int yc = 42;
				if (!MiniActive)
				{
					DrawStdBar( xc - 150, yc, xc + 150, yc + 30 + NPlayers * 18 + 30 );
				}

				int y0 = yc + 12;
				if (!MiniActive)
				{
					ShowCString( xc, y0 - 8, PAUSETEXT, &BigWhiteFont );
				}
				y0 += 24;

				int Problem = -1;

                byte Erased[8] = { 0,0,0,0,0,0,0,0 };

				for (int i = 0; i < NPlayers; i++)
				{
					if (6000 < GetRealTime() - LastAccess[i])
					{
						PlState[i] = -1;
					}

					char ccc[128];
					sprintf( ccc, "%s:", PINFO[i].name );

					if (EBufs[i].Enabled)
					{
						if (EBufs[i].RealTime != NON)
						{
							strcat( ccc, READYTX );
						}
						else
						{
							if (-1 == PlState[i])
							{
								strcat( ccc, NOANSTX );
								Problem = 4;
							}
							else
							{
								if (0 == PlState[i])
								{
									strcat( ccc, LOADTX );
									if (Problem < 3)
									{
										Problem = 3;
									}
								}
								else
								{
									if (1 == PlState[i])
									{
										strcat( ccc, LOOKSTX );//Looking at the menu.
									}
									else
									{
										if (17 == PlState[i])
										{
											strcat( ccc, ALTTAB );
										}
									}

									if (Problem < 2)
									{
										Problem = 2;
									}
								}
							}

							if (PlState[i] == -1)
							{
								if (mpl_time_1 > 650 && GLOBALTIME > 40)
								{
									int NS = 0;
									byte ms = 0;
									byte PColor = PINFO[i].ColorID;
									for (int v = 0; v < 7; v++)
									{
										if (NATIONS[v].ThereWasUnit && NATIONS[v].VictState == 0 && v != PColor && ( !Erased[v] ))
										{
											if (!( NATIONS[v].NMask&ms ))
											{
												NS++;
												ms |= NATIONS[v].NMask;
											}
										}
									}

									if (NS < 2)
									{
										//it is final of the game
										int MAXACC = 0;
										for (int v = 0; v < NPlayers; v++)
										{
											int ACC = SumAccount[NatRefTBL[PINFO[v].ColorID]];
											if (ACC > MAXACC)
											{
												MAXACC = ACC;
											}
										}
										//checking if there is acces to internet
										if (NPRECV > 1)
										{
											//checking if disconnected player has maximal score
											//1.search for maximal account
											byte NI = PINFO[i].ColorID;
											byte NIR = NatRefTBL[NI];
											if (SumAccount[NIR] == MAXACC)
											{
												if (IsGameActive())
												{
													DontMakeRaiting();
													ShowCentralText( "NORATE", 400 );
												}
												CmdEndGame( PINFO[i].ColorID, 5, 104 );
											}
											else
											{
												CmdEndGame( PINFO[i].ColorID, 4, 105 );//defeat
											}
										}
										else
										{
											if (CITY[NatRefTBL[MyNation]].Account < ( MAXACC * 2 ) / 4)
											{
												CmdEndGame( MyNation, 4, 106 );//defeat
												if (IsGameActive())
												{
													ShowCentralText( "SPECIALDEFEAT", 400 );
												}
											}
											else
											{
												if (IsGameActive())
												{
													StopRaiting();
													ShowCentralText( "INCOMTXT", 400 );
												}
												CmdEndGame( PINFO[i].ColorID, 5, 107 );
											}
										}
									}
									else
									{
										CmdEndGame( PINFO[i].ColorID, 3, 108 );
									}

									EBufs[i].Enabled = 0;
									Erased[PINFO[i].ColorID] = 1;
									char buf[200];
									sprintf( buf, GetTextByID( "PLEXIT" ), PINFO[i].name );
									CreateTimedHintEx( buf, kSystemMessageDisplayTime, 32 );//Player %s has left the game.
								}
							}
						}
					}
					else
					{
						strcat( ccc, NOPLAYTX );
					}

					if (!MiniActive)
					{
						ShowCString( xc, y0, ccc, &YellowFont );
					}
					y0 += 18;
				}

				bool UseF3 = 0;
				if (Problem == 4 && mpl_time_1 > 350)UseF3 = 1;
				if (Problem == 3 && mpl_time_1 > 400)UseF3 = 1;
				if (Problem == 2 && mpl_time_1 > 400)UseF3 = 1;
				if (UseF3)
				{
					if (!MiniActive)
					{
						ShowCString( xc, y0, DISCNTX, &WhiteFont );
					}
				}

				mpl_time_1 = 100;
				NMessM = 0;
                //FlipPages();
				if (UseF3)
				{
					if (LastKey == VK_F3)
					{
						mpl_time_1 = 10000000;
						for (int i = 0; i < NPlayers; i++)
						{
							EBufs[i].Enabled = PINFO[i].PlayerID == MyDPID;
                        }
						CmdEndGame( MyNation, 1, 109 );

						IAmLeft();
					}
				}
			}

			GSYSDRAW();
		} while (PresentEmptyBuf() && mpl_time_1 < 1000000);
	} while (NeedMoreReceive);

	if (PStart)
	{
		PINGS.DoneNetwork();
	}

	int MyColor = 7;
	for (int i = 0; i < NPlayers; i++)
	{
		if (PINFO[i].PlayerID == MyDPID)MyColor = i;
	}

	int MaxDiff = 0;
	for (int i = 0; i < NPlayers; i++)
	{
		if (EBufs[i].Enabled)
		{
			int c = i;//PINFO[i].ColorID;//!!!CHANGED!!!
			int d = CurrentStartTime[c];//-GameLength[c];
			if (d > MaxDiff)MaxDiff = d;
		}
	}

	if (MaxDiff > CurrentStepTime)
	{
		if (MaxDiff > 60000)
		{
			MaxDiff = 60000;
		}
		CmdDoItSlow( MaxDiff );
	}

    memcpy( CurrentStartTime, NextStartTime, 4 * 8 );
	memset( NextStartTime, 0xFF, 4 * 8 );
    memcpy( GameLength, NextGameLength, 4 * 8 );
	memset( NextGameLength, 0xFF, 4 * 8 );

	if (f)
	{
		Gclose( f );
	}

	EBPos1 = EBPos;
	byte* BUF = (byte*) ExBuf1;
	BUF[0] = 0xAE;
	BUF[1] = 0;
	BUF[2] = 0;
	BUF[3] = ( RealTime + 1 ) & 255;
	( (word*) ( BUF + 4 ) )[0] = rpos;
	int TT = GRTBW - SUBTIME;
	if (TT < NeedCurrentTime)
	{
		SUBTIME = NeedCurrentTime - TT;
		TT = NeedCurrentTime;
		if (SUBTIME > 80)SUBTIME = 80;
	}
	else
	{
		SUBTIME = 0;
	}

	int dt = ( TT - NeedCurrentTime ) >> 3;
	if (dt > 255)
	{
		dt = 255;
	}

	if (dt < 0)
	{
		dt = 0;
	}

	BUF[6] = dt;
	memcpy( BUF + 7, ExBuf, EBPos1 );
	word s = 0;
	int szz1 = 7 + EBPos1;

	for (int i = 0; i < szz1; i++)
	{
		s += BUF[i];
	}

	( (word*) ( BUF + 1 ) )[0] = s;

	PrevRpos = rpos;

	CurrentStartTime[MyColor] = dt << 3;//GetRealTime();
	GameLength[MyColor] = 0;//NeedCurrentTime;

	if (MaxPingTime)
	{
		SendToAllPlayers( EBPos1 + 7, ExBuf1 );
		SendToAllPlayersWithDelay( EBPos1 + 7, ExBuf1, 80 );
		SendToAllPlayersWithDelay( EBPos1 + 7, ExBuf1, 160 );
	}
	else
	{
		SendToAllPlayers( EBPos1 + 7, ExBuf1 );
		SendToAllPlayers( EBPos1 + 7, ExBuf1 );
	}

	if (PrevPrevEBSize)
	{
        //memcpy( PrevPrevPrevEB, PrevPrevEB, PrevPrevEBSize );
	}

    memcpy( PrevPrevEB, PrevEB, PrevEBSize );
	PrevPrevEBSize = PrevEBSize;

	memcpy( PrevEB, ExBuf1, EBPos1 + 7 );
	PrevEBSize = EBPos1 + 7;

	EBPos = 0;
	UpdateEBufs();
	RealTime++;
}

static void RetryNet( bool GUAR )
{
	//FUNNY: srsly? byte COMP -> (word*) (COMP + 2*i+1)?!
	byte COMP[4096];
	COMP[0] = 0xBB;
	*( (word*) ( COMP + 1 ) ) = PrevPrevEBSize;
	*( (word*) ( COMP + 3 ) ) = PrevEBSize;
	*( (word*) ( COMP + 5 ) ) = PrevEBSize + PrevPrevEBSize;
	memcpy( COMP + 7, PrevPrevEB, PrevPrevEBSize );
	memcpy( COMP + 7 + PrevPrevEBSize, PrevEB, PrevEBSize );

	if (DoNewInet)
	{
		SendToAllPlayers( PrevPrevEBSize + PrevEBSize + 7, COMP );
	}
	else
	{
		if (GUAR)
		{
			SendToAllPlayers( PrevPrevEBSize + PrevEBSize + 7, COMP, 1 );
		}
		else
		{
			SendToAllPlayers( PrevPrevEBSize + PrevEBSize + 7, COMP );
		}
	}
}

void SendChat( char* str, bool Ally )
{
	DWORD ps[300];
    /*if (Ally)ps[0] = 'ALLY';
    else ps[0] = 'CHAT';*/
    if (Ally)ps[0] = ConvCosChar( 'A', 'L', 'L', 'Y' );
        else ps[0] = ConvCosChar( 'C', 'H', 'A', 'T' );
	ps[1] = strlen( str ) + 1;
	memcpy( ps + 2, str, ps[1] );
	SendToAllPlayers( ps[1] + 8, ps, 0 );
}

void CloseMPL()
{
	if (int( lpDirectPlay3A ))
	{
		lpDirectPlay3A->Close();
	}

	lpDirectPlay3A = nullptr;

	if (DoNewInet&&IPCORE_INIT)
	{
		if (IPCORE.IsServer())
		{
			IPCORE.DoneServer();
		}
		else
		{
			IPCORE.DoneClient();
		}
		IPCORE_INIT = 0;
	}
}

static int PS_TIME1 = 0;
static int PS_TIME2 = 0;
static bool PS1_change = 0;
static bool PS2_change = 0;

void SETPLAYERDATA( uint32_t ID, void* Data, int size, bool change )
{
	if (change)
	{
		PS1_change = 1;
	}

	int TT = GetTickCount();

	if (!PS_TIME1)
	{
		PS_TIME1 = TT;
	}

	if (DoNewInet)
	{
		IPCORE.SetUserData( (byte*) Data, size );
		if (PS1_change)
		{
			if (TT - PS_TIME1 > 10000)
			{
				IPCORE.SendUserData();
				PS_TIME1 = TT;
			}
		}
		else
		{
			if (TT - PS_TIME1 > 2000)
			{
				IPCORE.SendUserData();
				PS_TIME1 = TT;
			}
		}
		PS1_change = 0;
	}
	else
	{
		lpDirectPlay3A->SetPlayerData( ID, Data, size, DPSET_REMOTE );
	}
}

void SETPLAYERNAME( DPNAME* lpdpName, bool change )
{
	if (change)
	{
		PS2_change = 1;
	}

	int TT = GetTickCount();
	if (!PS_TIME2)
	{
		PS_TIME2 = TT;
	}

	if (DoNewInet)
	{
		IPCORE.SetUserName( lpdpName->lpszShortNameA );
		if (PS2_change)
		{
			if (TT - PS_TIME2 > 10000)
			{
				IPCORE.SendUserName();
				PS_TIME2 = TT;
			}
		}
		else
		{
			if (TT - PS_TIME2 > 3000)
			{
				IPCORE.SendUserName();
				PS_TIME2 = TT;
			}
		}
		PS2_change = 0;
	}
	else
	{
		lpDirectPlay3A->SetPlayerName( MyDPID, lpdpName, DPSET_REMOTE );
	}
}

//Calls IPCORE.QueueProcess()
void ProcessNewInternet()
{
	if (DoNewInet)
	{
		IPCORE.QueueProcess();
	}
}

char SaveFileName[128];

void LoadSaveFile()
{
	SFLB_LoadGame( SaveFileName, 0 );
	SaveFileName[0] = 0;
}

static int LastSynTime = 0;
static int PREVSYNC = 0;

void SyncroDoctor()
{
	//return;
	//if(MaxPingTime)return;
	int tc = GetRealTime();
	if (tc - LastSynTime > 30000 || tc < LastSynTime)
	{
		if (LASTSAVEFILE[0])
		{
			DontMakeRaiting();
			//CmdLoadNetworkGame(MyNation,0,LASTSAVEFILE);
		}
		else
		{
			DontMakeRaiting();
        }
		//if(MyDPID==PlayersID[0]){
		// SaveGame("SYNCRO.sav","SYNCRO",0);
		// strcpy(SaveFileName,"SYNCRO.sav");
		// CmdLoadNetworkGame(MyNation,0,"SYNCRO.sav");
        //}
		if (!PREVSYNC)PREVSYNC = GetTickCount() - 30000;
		if (use_gsc_network_protocol && ( GetTickCount() - PREVSYNC > 20000 ))
		{
            OpenRef( 0, "GW|unsync" );
			PREVSYNC = GetTickCount();
        }
		LastSynTime = GetRealTime();
    }
}

PingSumm::PingSumm()
{
	NPL = 0;
	PSET = nullptr;
}

PingSumm::~PingSumm()
{
	for (int i = 0; i < NPL; i++)
	{
		free( PSET[i].Pings );
    }
	if (PSET)
	{
		free( PSET );
    }
}

void PingSumm::AddPing( uint32_t DPID, uint32_t From, uint32_t To, uint32_t Back )
{
	if (Back - From > 2500)return;
	int curr = -1;
	for (int i = 0; i < NPL&&curr == -1; i++)
	{
		if (PSET[i].DPID == DPID)curr = i;
    }
	if (curr == -1)
	{
		curr = NPL;
		NPL++;
        PSET = (PingsSet*) realloc( PSET, NPL * sizeof( PingsSet) );
		PSET[curr].DPID = DPID;
		PSET[curr].NPings = 0;
		PSET[curr].MaxPings = 0;
		PSET[curr].Pings = nullptr;
    }
	if (PSET[curr].NPings > 100000)return;
	if (PSET[curr].NPings >= PSET[curr].MaxPings)
	{
		PSET[curr].MaxPings += 50;
        PSET[curr].Pings = (OnePing*) realloc( PSET[curr].Pings, PSET[curr].MaxPings * sizeof( OnePing) );
    }
	int np = PSET[curr].NPings;
	PSET[curr].Pings[np].BackTime = int( Back );
	PSET[curr].Pings[np].FromTime = int( From );
	PSET[curr].Pings[np].ToTime = int( To );
	PSET[curr].NPings++;
}

int PingSumm::GetTimeDifference( uint32_t DPID )
{
	for (int i = 0; i < NPL; i++)
	{
		if (PSET[i].DPID == DPID)
		{
			OnePing* OP = PSET[i].Pings;
			int np = PSET[i].NPings;
			if (np < 2)return 0;
			int s = 0;
			int nn = 1;
			int m0 = np - 30;
			if (m0 < 1)m0 = 1;
			int dt0 = OP[m0].ToTime - ( OP[m0].FromTime >> 1 ) - ( OP[m0].BackTime >> 1 );
			for (int j = m0; j < np; j++)
			{
				int avt = ( OP[j].FromTime >> 1 ) + ( OP[j].BackTime >> 1 );
				s += OP[j].ToTime - avt - dt0;
				nn++;
            }
			s /= nn;
            return dt0 + s;
        }
    }
	return 0;
}

static void StartPing( DWORD DPID, int ID );
static char* GetLString( DWORD DPID );
static const char* LOSS_PN = nullptr;

void CreateDiffStr( char* str )
{
	if (!LOSS_PN)LOSS_PN = GetTextByID( "LOSS_PN" );
	if (( GetKeyState( VK_CONTROL ) & 0x8000 ) && ( GetKeyState( VK_SHIFT ) & 0x8000 ))
	{
		str[0] = 0;
		for (int i = 0; i < NPlayers; i++)
		{
			strcat( str, PINFO[i].name );
			strcat( str, " : " );
			int dt = PSUMM.GetTimeDifference( PINFO[i].PlayerID );
			sprintf( str + strlen( str ), "%4X %d", PINFO[i].PlayerID, dt );
			strcat( str, " " );
        }
	}
	else
	{
		str[0] = 0;
		int N = 0;
		for (int i = 0; i < NPlayers; i++)
		{
			strcat( str, PINFO[i].name );
            if (PINFO[i].Rank)
			{
				char cc3[128];
				sprintf( cc3, "RS_RANK_%d", PINFO[i].Rank );
				sprintf( str + strlen( str ), " (%s)", GetTextByID( cc3 ) );
            }
			if (PINFO[i].PlayerID != MyDPID)
			{
				strcat( str, " : " );
				int dt = GetPing( PINFO[i].PlayerID );
				sprintf( str + strlen( str ), "%d", dt );
				strcat( str, " ," );
				sprintf( str + strlen( str ), LOSS_PN, GetLString( PINFO[i].PlayerID ) );
            }
			strcat( str, " " );
			N++;
			if (N > 3)
			{
				strcat( str, "\\" );
				N = 0;
			}
		}
	}
}

int PingSumm::CheckPlayer( uint32_t DPID )
{
	for (int i = 0; i < NPL; i++)
	{
		if (PSET[i].DPID == DPID)
		{
			return PSET[i].NPings;
		}
	}

	return 0;
}

void PingSumm::AddPlayer( uint32_t DPID )
{
	for (int i = 0; i < NPL; i++)
	{
		if (PSET[i].DPID == DPID)
		{
			return;
		}
	}
	int curr = NPL;
	NPL++;
    PSET = (PingsSet*) realloc( PSET, NPL * sizeof( PingsSet) );
	PSET[curr].DPID = DPID;
	PSET[curr].NPings = 0;
	PSET[curr].MaxPings = 0;
	PSET[curr].Pings = nullptr;
}

void PingSumm::ClearPingInfo()
{
	for (int i = 0; i < NPL; i++)
	{
		free( PSET[i].Pings );
	}
	if (PSET)
	{
		free( PSET );
	}
	PSET = nullptr;
	NPL = 0;
}

static int PrevPingTime = 0;

void SendPings()
{
	if (NPlayers < 2)
	{
		return;
	}
	if (GetRealTime() - PrevPingTime < 1000)
	{
		return;
	}
    int stv = GetRealTime();
	for (int i = 0; i < NPlayers; i++)
	{
		if (PINFO[i].PlayerID != MyDPID)
		{
			DWORD lp[6];
            //lp[0] = 'PING';
            lp[0] = ConvCosChar( 'P', 'I', 'N', 'G' );
			lp[1] = MyDPID;
			lp[2] = stv;
			stv++;
			lp[3] = 0;
			lp[4] = 0;
			lp[5] = lp[0] + lp[1] + lp[2];
			SendToPlayer( 24, lp, PINFO[i].PlayerID );
			StartPing( PINFO[i].PlayerID, lp[2] );
		}
	}
	PrevPingTime = GetRealTime();
}

bool CheckPingsReady()
{
	for (int i = 0; i < NPlayers; i++)
	{
		if (PINFO[i].PlayerID != MyDPID &&
			PSUMM.CheckPlayer( PINFO[i].PlayerID ) < 12)
		{
			return false;
		}
	}
	return true;
}

int GetReadyPercent()
{
	int MINP = 100;
	for (int i = 0; i < NPlayers; i++)
	{
		if (PINFO[i].PlayerID != MyDPID)
		{
			int P = ( PSUMM.CheckPlayer( PINFO[i].PlayerID ) * 100 ) / 12;
			if (P < MINP)
			{
				MINP = P;
			}
		}
	}
	return MINP;
}

int GetMaxRealPing()
{
	//for(int i=0;i<NPlayers;i++){
	// if(PINFO[i].PlayerID!=MyDPID)return PSUMM.GetTimeDifference(PINFO[i].PlayerID);
    //}

	int nn = 0;
	int sp = 0;
	int maxsp = 0;
	for (int i = 0; i < PSUMM.NPL; i++)
	{
		int m = PSUMM.PSET[i].NPings;
		int m0 = 0;
		if (m > 20)m0 = m - 20;
		sp = 0;
		nn = 0;
		OnePing* OP = PSUMM.PSET[i].Pings + m0;
		for (int j = m0; j < m; j++)
		{
			sp += OP->BackTime - OP->FromTime;
			nn++;
			OP++;
		}
		if (nn)
		{
			sp /= nn;
			if (sp > maxsp)
			{
				maxsp = sp;
			}
		}
	}
	return maxsp;
}

int GetPing( DPID pid )
{
	int nn = 0;
	int sp = 0;
	int maxsp = 0;
	for (int i = 0; i < PSUMM.NPL; i++)
	{
		if (PSUMM.PSET[i].DPID == pid)
		{
			int m = PSUMM.PSET[i].NPings;
			int m0 = 0;
			if (m > 20)
			{
				m0 = m - 20;
			}
			sp = 0;
			nn = 0;
			OnePing* OP = PSUMM.PSET[i].Pings + m0;
			for (int j = m0; j < m; j++)
			{
				sp += OP->BackTime - OP->FromTime;
				nn++;
				OP++;
			}
			if (nn)
			{
				sp /= nn;
				if (sp > maxsp)maxsp = sp;
			}
		}
	}
	return maxsp;
}

//--------------------------PLAYERS MESSAGES BACKUP-------------------------//

PLAYERSBACKUP::PLAYERSBACKUP()
{
	NBDATA = 0;
}

PLAYERSBACKUP::~PLAYERSBACKUP()
{
	for (int i = 0; i < NBDATA; i++)
	{
		free( BSTR[i].Data );
    }
	NBDATA = 0;
}

void PLAYERSBACKUP::Clear()
{
	for (int i = 0; i < NBDATA; i++)
	{
		free( BSTR[i].Data );
    }
	NBDATA = 0;
}

void PLAYERSBACKUP::AddInf( byte* BUF, int L, uint32_t ID, int RT )
{
	if (!L)
	{
		return;
	}
	for (int i = 0; i < NBDATA; i++)
	{
		if (BSTR[i].ID == (DWORD) ID && BSTR[i].RealTime == (DWORD) RT)
		{
			return;
		}
	}
	if (NBDATA == 32)
	{
		free( BSTR[0].Data );
        memcpy( BSTR, BSTR + 1, 31 * sizeof( BACKUPSTR) );
		NBDATA--;
	}
	BSTR[NBDATA].Data = new byte[L];
	memcpy( BSTR[NBDATA].Data, BUF, L );
	BSTR[NBDATA].ID = ID;
	BSTR[NBDATA].RealTime = RT;
	BSTR[NBDATA].L = L;
	NBDATA++;
}

void PLAYERSBACKUP::SendInfoAboutTo( uint32_t ID, uint32_t TO, uint32_t RT )
{
	for (int i = 0; i < NBDATA; i++)if (BSTR[i].ID == ID&&BSTR[i].RealTime == RT)
	{
		DWORD* DATA = new DWORD[( BSTR[i].L >> 2 ) + 8];
        //DATA[0] = 'SIAP';
        DATA[0] = ConvCosChar( 'S', 'I', 'A', 'P' );
		DATA[1] = ID;
		memcpy( DATA + 2, BSTR[i].Data, BSTR[i].L );
		SendToPlayer( 8 + BSTR[i].L, DATA, TO );
		free( DATA );
		return;
	}
}
//--------------------RETRANSLATION ORDERS-----------------------//

RETRANS::RETRANS()
{
	TOT = nullptr;
	NRET = 0;
	MaxRET = 0;
}

RETRANS::~RETRANS()
{
	Clear();
}

void RETRANS::Clear()
{
	if (MaxRET)
	{
		free( TOT );
    }
	NRET = 0;
	MaxRET = 0;
}

void RETRANS::AddOneRet( uint32_t TO, uint32_t FROM, uint32_t RT )
{
	for (int i = 0; i < NRET; i++)if (TOT[i].IDFROM == FROM&&TOT[i].IDTO == TO&&TOT[i].RT == RT)return;
	if (NRET >= MaxRET)
	{
        TOT = (SingleRetr*) realloc( TOT, ( MaxRET + 64 ) * sizeof( SingleRetr) );
		MaxRET += 64;
    }
	TOT[NRET].IDFROM = FROM;
	TOT[NRET].IDTO = TO;
	TOT[NRET].RT = RT;
	NRET++;
}

void RETRANS::AddSection( uint32_t TO, uint32_t FROM, uint32_t RT )
{
	for (int i = 0; i < 50; i++)AddOneRet( TO, FROM, RT + i );
}

void RETRANS::CheckRetr( uint32_t From, uint32_t RT )
{
	for (int i = 0; i < NRET; i++)if (TOT[i].IDFROM == From&&TOT[i].RT == RT)
	{
		PBACK.SendInfoAboutTo( From, TOT[i].IDTO, RT );
		if (i < NRET - 1)
		{
            memcpy( TOT + i, TOT + i + 1, ( NRET - i - 1 ) * sizeof( SingleRetr) );
        }
		NRET--;
		i--;
    }
}

RETRANS RETSYS;
//------------------CALCULATING LOOSING PACKETS---------------//
static constexpr int PingTimeout = 2000;

struct OneLPing
{
	DWORD UniqID;
	int StartTime;
};

class OneLostID
{
public:
	DWORD DPID;
	int NSent;
	int NReceived;
	OneLPing* PING;
	int NPings;
	int LastAccessTime;
	int LastReceiveTime;
};

class LoosedPack
{
public:
	LoosedPack();
	~LoosedPack();
	OneLostID OLID[16];
	int GetLoosePercent( DWORD ID );
	int GetLastAnswerTime( DWORD ID );
	void Clear();
	void DeleteBadPlayers();
	void Add( DWORD DPID, int ID );
	void Remove( int ID );
	void Process();
};

LoosedPack::LoosedPack()
{
    memset( OLID, 0, 16 * sizeof( OneLostID) );
}

LoosedPack::~LoosedPack()
{
	Clear();
}

void LoosedPack::DeleteBadPlayers()
{
    for (int i = 0; i < 16; i++)
	{
		int ID = OLID[i].DPID;
		if (ID)
		{
			bool Present = 0;
			for (int j = 0; j < NPlayers; j++)
			{
				if (PINFO[j].PlayerID == (DPID1) ID)
				{
					Present = true;
				}
			}
			if (!Present)
			{
				if (OLID[i].PING)
				{
					free( OLID[i].PING );
				}
                memset( OLID + i, 0, sizeof( OneLostID) );
			}
		}
	}
}

int LoosedPack::GetLoosePercent( DWORD ID )
{
	for (int i = 0; i < 16; i++)if (OLID[i].DPID == ID)
	{
		if (OLID[i].NSent)return ( ( OLID[i].NSent - OLID[i].NReceived ) * 100 ) / OLID[i].NSent;
    }
	return -1;
}

void LoosedPack::Clear()
{
	for (int i = 0; i < 8; i++)
	{
		if (OLID[i].PING)
		{
			free( OLID[i].PING );
        }
    }
    memset( OLID, 0, 8 * sizeof( OneLostID) );
}

void LoosedPack::Add( DWORD DPID, int ID )
{
	int idx = -1;
	int CurTime = GetTickCount();
	for (int i = 0; i < 16; i++)
	{
		if (OLID[i].DPID == DPID)
		{
			idx = i;
			i = 16;
        }
    }
	if (idx == -1)
	{
		for (int i = 0; i < 16; i++)if (!OLID[i].LastAccessTime)
		{
			idx = i;
			i = 16;
        }
		if (idx == -1)
		{
			int MaxDiff = 0;
			for (int i = 0; i < 16; i++)
			{
				int dt = CurTime - OLID[i].LastAccessTime;
				if (dt > MaxDiff)
				{
					MaxDiff = dt;
					idx = i;
                }
            }
			if (idx != -1)
			{
				if (OLID[idx].PING)
				{
					free( OLID[idx].PING );
					OLID[idx].PING = nullptr;
					OLID[idx].NPings = 0;
					OLID[idx].NSent = 0;
					OLID[idx].NReceived = 0;
					OLID[idx].LastReceiveTime = GetTickCount();
                }
            }
        }
    }
	if (idx != -1)
	{
		OLID[idx].DPID = DPID;
        OLID[idx].PING = (OneLPing*) realloc( OLID[idx].PING, ( OLID[idx].NPings + 1 ) * sizeof( OneLPing) );
		OneLPing* OP = OLID[idx].PING + OLID[idx].NPings;
		OLID[idx].NPings++;
		OLID[idx].LastAccessTime = CurTime;
		OP->StartTime = CurTime;
		OP->UniqID = ID;
		if (!OLID[idx].LastReceiveTime)
			OLID[idx].LastReceiveTime = GetTickCount();
    }
}

void LoosedPack::Remove( int ID )
{
	for (int i = 0; i < 16; i++)
	{
		int N = OLID[i].NPings;
		for (int j = 0; j < N; j++)
		{
			if (OLID[i].PING[j].UniqID == (DWORD) ID)
			{
				OLID[i].NReceived++;
				OLID[i].NSent++;
				OLID[i].LastReceiveTime = GetTickCount();
				if (OLID[i].NReceived > OLID[i].NSent)
				{
					OLID[i].NReceived = OLID[i].NSent;
				}
				if (j < N - 1)
				{
                    memcpy( OLID[i].PING + j, OLID[i].PING + j + 1, ( N - j - 1 ) * sizeof( OneLPing) );
				}
				OLID[i].NPings--;
				if (!OLID[i].NPings)
				{
					free( OLID[i].PING );
					OLID[i].PING = nullptr;
				}
			}
		}
	}
}

void LoosedPack::Process()
{
	int CT = GetTickCount();
	for (int i = 0; i < 16; i++)
	{
		int N = OLID[i].NPings;
		for (int j = 0; j < N; j++)
		{
			if (CT - OLID[i].PING[j].StartTime > PingTimeout)
			{
				if (OLID[i].NReceived)OLID[i].NSent++;
				if (j < N - 1)
				{
                    memcpy( OLID[i].PING + j, OLID[i].PING + j + 1, ( N - j - 1 ) * sizeof( OneLPing) );
                }
				OLID[i].NPings--;
				if (!OLID[i].NPings)
				{
					free( OLID[i].PING );
					OLID[i].PING = nullptr;
                }
            }
        }
    }
}

int LoosedPack::GetLastAnswerTime( DWORD ID )
{
	for (int i = 0; i < 16; i++)if (OLID[i].DPID == ID)
	{
		return OLID[i].LastReceiveTime;
    }
	return 0;
}

static LoosedPack LPACK;

static void StartPing( DWORD DPID, int ID )
{
	LPACK.Add( DPID, ID );
	LPACK.Process();
}

static void EndPing( int ID )
{
	LPACK.Remove( ID );
}

static char tmp1[12];
static char* GetLString( DWORD DPID )
{
	int p = LPACK.GetLoosePercent( DPID );
	if (p == -1)strcpy( tmp1, "???" );
	else sprintf( tmp1, "%d%%", p );
	return tmp1;
}

static int GetLastAnswerT( DWORD ID )
{
	return LPACK.GetLastAnswerTime( ID );
}

void ClearLPACK()
{
	LPACK.Clear();
}

static void DelBADPL()
{
	LPACK.DeleteBadPlayers();
}

void DeepDeletePeer( uint32_t ID )
{
	IPCORE.DeletePeer( ID );
}

char ACCESS[16] = "";

bool ProcessNewInternetLogin()
{
    DialogsSystem DSS( menu_x_off, menu_y_off );

    SQPicture Back( "Interface\\Background_Wizard.bmp" );

    LocalGP WBT( "Interface\\LogWizard" );
    LocalGP BTNS( "Interface\\Login" );

    DSS.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

    char REQ[1024];
    char REQ1[1024];
    char DATA[512] = "";

    ResFile F = RReset( "Internet\\Cash\\login.cmd" );
    if (F != INVALID_HANDLE_VALUE)
    {
        int sz = RFileSize( F );
        if (sz > 511)
        {
            sz = 511;
        }
        RBlockRead( F, REQ1, sz );
        REQ1[sz] = 0;
        RClose( F );
    }
    else
    {
        return false;
    }

    F = RReset( "Internet\\Cash\\LGDTA.LOG" );

    if (F != INVALID_HANDLE_VALUE)
    {
        int sz = RFileSize( F );
        if (sz > 511)
        {
            sz = 511;
        }
        RBlockRead( F, DATA, sz );
        DATA[sz] = 0;
        RClose( F );
    }

    sprintf( REQ, REQ1, DATA );

    RunSXP( 2, REQ, menu_x_off, menu_y_off, 1024, 768 );

    ItemChoose = -1;
    for (int i = 0; i < 8; i++)
        {
            SetAccessKey( i, "" );
        }

    char* AKEY = nullptr;
    ACCESS[0] = 0;
    bool pal = 0;

    do
    {
        ProcessMessages();

        DSS.ProcessDialogs();

        ProcessSXP( 2, nullptr );

        DSS.RefreshView();

        AKEY = GetAccessKey( 2 );

        if (!pal)
        {
            SlowLoadPalette( "2\\agew_1.pal" );
            pal = 1;
        }
    } while (ItemChoose == -1 && AKEY[0] == 0);

    if (!strcmp( AKEY, "#CANCEL" ))
    {
        AKEY[0] = 0;
    }

        for (int i = 0; i < 8; i++)
        {
            SetAccessKey( i, AKEY );
        }

    strcpy( ACCESS, AKEY );

    DSS.ProcessDialogs();

    ProcessSXP( 2, nullptr );

    DSS.RefreshView();

    return AKEY[0] != 0;
}
