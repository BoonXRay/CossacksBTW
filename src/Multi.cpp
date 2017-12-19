//MULTIPLAYER ORGANIZATION

#include <cassert>

#include "IntExplorer/Explore.h"

#include "3DGraph.h"
#include "3DRandMap.h"
#include "ActiveScenary.h"
#include "BoonUtils.hpp"
#include "Build.h"
#include "Ddini.h"
#include "Economy.h"
#include "Fonts.h"
#include "DrawForm.h"
#include "FastDraw.h"
#include "Fog.h"
#include "GameSound.h"
#include "Groups.h"
#include "Hint.h"
#include "Interface.h"
#include "LoadSave.h"
#include "main.h"
#include "Mapa.h"
#include "MapSprites.h"
#include "Mine.h"
#include "Mouse_X.h"
#include "MPlayer.h"
#include "Nation.h"
#include "Nature.h"
#include "NewMon.h"
#include "Path.h"
#include "Recorder.h"
#include "SelProp.h"
#include "Sort.h"
#include "Transport.h"
#include "UnSyncro.h"
#include "Walls.h"
#include "Weapon.h"


#include "Multi.h"


uint8_t * NPresence;
Nation NATIONS[8];
uint8_t  MYNATION;
uint16_t * Selm[8];
uint16_t * SerN[8];
uint16_t * ImSelm[8];
uint16_t * ImSerN[8];
uint16_t NSL[8];
uint16_t ImNSL[8];
bool CmdDone[ULIMIT];

//Execute buffer 
uint8_t ExBuf[8192];
int EBPos;

void InitEBuf()
{
	EBPos = 0;
}

//[2][ni][x][y]
void CmdSendToXY(uint8_t NI, int x, int y, short Dir )
{
	//if(!NOPAUSE)return;//!!!

	byte Type = 0;
	if ( GetKeyState( VK_SHIFT ) & 0x8000 )Type = 2;
	if ( GoAndAttackMode )Type |= 8;
	if ( GetKeyState( VK_CONTROL ) & 0x8000 )Type |= 16;
	GoAndAttackMode = 0;
	ExBuf[EBPos] = 2;
	ExBuf[EBPos + 1] = NI;
	*(int*) ( &ExBuf[EBPos + 2] ) = x;
	*(int*) ( &ExBuf[EBPos + 6] ) = y;
	*(short*) ( &ExBuf[EBPos + 10] ) = Dir;
	ExBuf[EBPos + 12] = Type;
	EBPos += 13;
	if ( !curptr )AddXYPulse( x >> 4, y >> 4 );

}

//[3][ni][w:ObjID]
void CmdAttackObj(uint8_t NI, uint16_t ObjID, short DIR )
{
	//if(!NOPAUSE)return;//!!!

	byte Type = 0;
	if ( GetKeyState( VK_SHIFT ) & 0x8000 )Type = 2;
	ExBuf[EBPos] = 3;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = ObjID;
	ExBuf[EBPos + 4] = Type;
	*(short*) ( &ExBuf[EBPos + 5] ) = DIR;
	EBPos += 7;

}

//[5][ni][x:32][y:32][Type:16][OrderType]
void CmdCreateBuilding( uint8_t NI, int x, int y, uint16_t Type )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 5;
	ExBuf[EBPos + 1] = NI;
	*(int*) ( &ExBuf[EBPos + 2] ) = x;
	*(int*) ( &ExBuf[EBPos + 6] ) = y;
	*(word*) ( &ExBuf[EBPos + 10] ) = Type;
	if ( GetKeyState( VK_SHIFT ) & 0x8000 )ExBuf[EBPos + 12] = 2;
	else ExBuf[EBPos + 12] = 0;
	EBPos += 13;

}

//[6][ni][w:ObjID]
static int PrevProdPos = -1;
static int PrevProdUnit = 0;
void CmdProduceObj(uint8_t NI, uint16_t Type )
{
	//if(!NOPAUSE)return;//!!!
	if ( GetKeyState( VK_CONTROL ) & 0x8000 )Type |= 8192;
	if ( !AddUnitsToCash( NI, Type ) )return;
	if ( PrevProdPos != -1 && PrevProdUnit == Type )
	{
		int N = ExBuf[PrevProdPos];
		if ( N < 255 )N++;
		ExBuf[PrevProdPos]++;
		return;
    }

	ExBuf[EBPos] = 78;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = 1;
	PrevProdPos = EBPos + 2;
	PrevProdUnit = Type;
	*(word*) ( &ExBuf[EBPos + 3] ) = Type;
	EBPos += 5;

}

//[7][ni][index]
void CmdMemSelection( uint8_t NI, uint8_t Index )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 7;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = Index;
	EBPos += 3;

}

//[8][ni][Index]
void CmdRememSelection( uint8_t NI, uint8_t Index )
{
	//if(!NOPAUSE)return;//!!!

	byte SHIFT = ( GetKeyState( VK_SHIFT ) & 0x8000 ) != 0;
	bool shift = 0 != SHIFT;
	SelSet[NI * 10 + ( Index & 127 )].ImSelectMembers( NI, shift );
	ImCorrectBrigadesSelection( NI );
	ExBuf[EBPos] = 8;
	ExBuf[EBPos + 1] = NI + ( SHIFT * 32 );
	ExBuf[EBPos + 2] = Index;
	EBPos += 3;

}

//[9][ni][ObjID:16][OT]
void CmdBuildObj( uint8_t NI, uint16_t ObjID )
{
	ExBuf[EBPos] = 9;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = ObjID;
	if ( GetKeyState( VK_SHIFT ) & 0x8000 )
	{
		ExBuf[EBPos + 4] = 2;
	}
	else
	{
		ExBuf[EBPos + 4] = 0;
	}
	EBPos += 5;
}

//[0B][NI][xx:16][yy:16][OrdType]
void CmdRepairWall( uint8_t NI, short xx, short yy )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 0xB;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = xx;
	*(word*) ( &ExBuf[EBPos + 4] ) = yy;
	if ( GetKeyState( VK_SHIFT ) & 0x8000 )ExBuf[EBPos + 6] = 2;
	else ExBuf[EBPos + 6] = 0;
	EBPos += 7;

}

//[0D][NI][x][y][ResID]
void CmdTakeRes( uint8_t NI, int x, int y, uint8_t ResID )
{
	ExBuf[EBPos] = 0xD;
	ExBuf[EBPos + 1] = NI;
	*(int*) ( &ExBuf[EBPos + 2] ) = x;
	*(int*) ( &ExBuf[EBPos + 6] ) = y;
	ExBuf[EBPos + 10] = ResID;
	EBPos += 11;
}

//[0E][NI][UpgradeIndex:16]
void CmdPerformUpgrade( uint8_t NI, uint16_t UI )
{
	ExBuf[EBPos] = 0xE;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = UI;
	EBPos += 4;
}

static bool FnKind( OneObject* OB, int N )
{
	return ( OB->Kind == N );
}

static bool FnType( OneObject* OB, int N )
{
	return ( OB->NIndex == N );
}

static void CreateGoodSelection( byte NI, word xx, word yy, word xx1, word yy1, CHOBJ* FN, int NN, bool Addon );
//[12][ni][x:16][y:16][x1:16][y1:16]
void CmdCreateGoodSelection( uint8_t NI, uint16_t x, uint16_t y, uint16_t x1, uint16_t y1 )
{
	//if(!NOPAUSE)return;//!!!
	bool Addon = false;
	if ( GetKeyState( VK_SHIFT ) & 0x8000 )Addon = true;
	CreateGoodSelection( NI, x, y, x1, y1, nullptr, 0, Addon );
}

//[13][NI][x][y][x1][y1][Kind]
void CmdCreateGoodKindSelection( uint8_t NI, uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint8_t Kind )
{
	//if(!NOPAUSE)return;//!!!
	bool Addon = false;
	if ( GetKeyState( VK_SHIFT ) & 0x8000 )Addon = true;
	CreateGoodSelection( NI, x, y, x1, y1, &FnKind, Kind, Addon );
}

//[14][NI][x][y][x1][y1][Type]
void CmdCreateGoodTypeSelection( uint8_t NI, uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint16_t Type )
{
	//if(!NOPAUSE)return;//!!!
	bool Addon = false;
	if ( GetKeyState( VK_SHIFT ) & 0x8000 )Addon = true;
	CreateGoodSelection( NI, x, y, x1, y1, &FnType, Type, Addon );
}

//[15][NI][x][y]
void CmdSetDst( uint8_t NI, int x, int y )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 21;
	ExBuf[EBPos + 1] = NI;
	*(int*) ( &ExBuf[EBPos + 2] ) = x;
	*(int*) ( &ExBuf[EBPos + 6] ) = y;
	EBPos += 10;

}

//[18][ni]
void CmdStop( uint8_t NI )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 24;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

//[19][ni]
void CmdStandGround( uint8_t NI )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 25;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

//[1A][ni][x][y]
void CmdPatrol( uint8_t NI, int x, int y )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 26;
	ExBuf[EBPos + 1] = NI;
	*(int*) ( &ExBuf[EBPos + 2] ) = x;
	*(int*) ( &ExBuf[EBPos + 6] ) = y;
	EBPos += 10;

}

//[1B][ni][x][y]
void CmdRepair( uint8_t NI, uint8_t x, uint8_t y )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 27;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = x;
	ExBuf[EBPos + 3] = y;
	EBPos += 4;

}

//[20)][ni][x][y]
void CmdUnload( uint8_t NI, uint8_t x, uint8_t y )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 32;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = x;
	ExBuf[EBPos + 3] = y;
	EBPos += 4;

}

//[21][ni]
void CmdDie( uint8_t NI )
{//Called each time 'Del' is pressed
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 33;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

//[25][ni][x][y]
void CmdNucAtt( uint8_t NI, uint8_t x, uint8_t y )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 37;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = x;
	ExBuf[EBPos + 3] = y;
	EBPos += 4;

}

//[26][ni][w:ObjID]
void CmdUnProduceObj( uint8_t NI, uint16_t Type )
{
	ExBuf[EBPos] = 38;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = Type;
	EBPos += 4;
}

//[27][ni][w:ObjID]
void CmdSetRprState( uint8_t NI, uint8_t State )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 39;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = State;
	EBPos += 3;

}

char LASTSAVEFILE[64] = "";
//[28][ni][ID:32][Length:8][Name...]
void CmdSaveNetworkGame( uint8_t NI, int ID, const char* Name )
{
	ExBuf[EBPos] = 40;
	ExBuf[EBPos + 1] = NI;
	*(int*) ( &ExBuf[EBPos + 2] ) = ID;
	byte Len = (byte) strlen( Name );
	ExBuf[EBPos + 6] = Len;
	strcpy( (char*) ( &ExBuf[EBPos + 7] ), Name );
	EBPos += 7 + Len;
}

//[29][ni][ID:32][Length:8][Name...]
void CmdLoadNetworkGame( uint8_t NI, int ID, char* Name )
{

	ExBuf[EBPos] = 41;
	ExBuf[EBPos + 1] = NI;
	*(int*) ( &ExBuf[EBPos + 2] ) = ID;
	byte Len = (byte) strlen( Name );
	ExBuf[EBPos + 6] = Len;
	strcpy( (char*) ( &ExBuf[EBPos + 7] ), Name );
	EBPos += 7 + Len;

}

//[2A][ni][ID]
static void ImChooseUnSelectType( byte NI, word ID );
static void ImChooseSelectType( byte NI, word ID );
void CmdChooseSelType( uint8_t NI, uint16_t ID )
{
	//if(!NOPAUSE)return;//!!!

	ImChooseSelectType( NI, ID );
	ExBuf[EBPos] = 42;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = ID;
	EBPos += 4;

}

//[2B][ni][ID]
void CmdChooseUnSelType( uint8_t NI, uint16_t ID )
{
	//if(!NOPAUSE)return;//!!!

	ImChooseUnSelectType( NI, ID );
	ExBuf[EBPos] = 43;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = ID;
	EBPos += 4;
}

//[2C][NI][x:16][y:16][ID:16]
void CmdCreateNewTerr( uint8_t NI, int x, int y, uint16_t ID )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 44;
	ExBuf[EBPos + 1] = NatRefTBL[NI];
	*(int*) ( &ExBuf[EBPos + 2] ) = x;
	*(int*) ( &ExBuf[EBPos + 6] ) = y;
	*(word*) ( &ExBuf[EBPos + 10] ) = ID;
	EBPos += 12;

}

//[2D][NI][ID:16]
void CmdGoToMine( uint8_t NI, uint16_t ID )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 45;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = ID;
	EBPos += 4;

}

//[2E][NI][Type:16]
void CmdLeaveMine( uint8_t NI, uint16_t Type )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 46;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = Type;
	EBPos += 4;

}

void CmdGoToTransport( uint8_t NI, uint16_t ID )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 48;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = ID;
	EBPos += 4;

}

void CmdCreateGates( uint8_t NI )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 49;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

void CmdOpenGates( uint8_t NI )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 50;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

void CmdCloseGates( uint8_t NI )
{
	ExBuf[EBPos] = 51;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

void CmdCreateWall( uint8_t NI )
{
	ExBuf[EBPos] = 47;
	TMPCluster.NI = NI;
	EBPos += 1 + TMPCluster.CreateData( (word*) ( &ExBuf[EBPos + 1] ), 0 );
}

static void ImSelectAllBuildings( uint8_t NI );

void CmdSelectBuildings( uint8_t NI )
{
	ImSelectAllBuildings( NI );
	ExBuf[EBPos] = 52;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

static void ImUnSelectBrig( byte NI, word ID );

static void ImSelectBrig( byte NI, word ID );

void CmdChooseSelBrig( uint8_t NI, uint16_t ID )
{
	//if(!NOPAUSE)return;//!!!

	ImSelectBrig( NI, ID );
	ExBuf[EBPos] = 53;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = ID;
	EBPos += 4;

}

void CmdChooseUnSelBrig( uint8_t NI, uint16_t ID )
{
	//if(!NOPAUSE)return;//!!!

	ImUnSelectBrig( NI, ID );
	ExBuf[EBPos] = 54;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = ID;
	EBPos += 4;

}

void CmdErseBrigs( uint8_t NI )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 55;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

void CmdMakeStandGround( uint8_t NI )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 56;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

void CmdCancelStandGround( uint8_t NI )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 57;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;

}

void CmdCrBig( uint8_t NI, int i )
{
	//if(!NOPAUSE)return;//!!!

	ExBuf[EBPos] = 58;
	ExBuf[EBPos + 1] = NI;
	*(int*) ( &ExBuf[EBPos + 2] ) = i;
	EBPos += 6;
	if ( ImNSL[NI] )
	{
		int N = ImNSL[NI];
		for ( int j = 1; j < N; j++ )
		{
			word MID = ImSelm[NI][j];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB )
				{
					OB->ImSelected &= ~GM( NI );
				}
			}
		}
		ImNSL[NI] = 1;
	}
}

//Place unit selection order in ExBuf
void CmdSelBrig( uint8_t NI, uint8_t Type, uint16_t ID )
{
	ImSelBrigade( NI, Type, ID );
	ExBuf[EBPos] = 59;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = Type;
	*(word*) ( &ExBuf[EBPos + 3] ) = ID;
	EBPos += 5;
}

//Adds given market exchange order to execute buffer
void CmdTorg( uint8_t NI, uint8_t SellRes, uint8_t BuyRes, int SellAmount )
{
	ExBuf[EBPos] = 60;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = SellRes;
	ExBuf[EBPos + 3] = BuyRes;
	*(int*) ( &ExBuf[EBPos + 4] ) = SellAmount;
	EBPos += 8;
}

void CmdFieldBar( uint8_t NI, uint16_t n )
{
	ExBuf[EBPos] = 61;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = n;
	EBPos += 4;
}

void CmdSetSrVictim( uint8_t NI, uint8_t val )
{
	ExBuf[EBPos] = 62;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = val;
	EBPos += 3;
}

void CmdStopUpgrade( uint8_t NI )
{
	ExBuf[EBPos] = 63;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

void CmdUnloadAll( uint8_t NI )
{
	ExBuf[EBPos] = 64;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

void CmdMakeReformation( uint8_t NI, uint16_t BrigadeID, uint8_t FormType )
{
	ExBuf[EBPos] = 65;
	ExBuf[EBPos + 1] = NI;
	*(word*) ( &ExBuf[EBPos + 2] ) = BrigadeID;
	ExBuf[EBPos + 4] = FormType;
	EBPos += 5;
}

static void ImSelectIdlePeasants( uint8_t NI );

void CmdSelectIdlePeasants( uint8_t NI )
{
	ImSelectIdlePeasants( NI );
	ExBuf[EBPos] = 66;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

static void ImSelectIdleMines( uint8_t NI );

void CmdSelectIdleMines( uint8_t NI )
{
	ImSelectIdleMines( NI );
	ExBuf[EBPos] = 67;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

void CmdSetSpeed( uint8_t Speed )
{
	ExBuf[EBPos] = 69;
	ExBuf[EBPos + 1] = Speed;
	EBPos += 2;
}

void CmdPause( uint8_t NI )
{
	ExBuf[EBPos] = 70;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

static void CmdFreeSelected( uint8_t NI )
{
	ExBuf[EBPos] = 71;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

static void SelectAllUnits( byte NI, bool Re );
static void SelectAllShips( byte NI, bool Re );

void CmdSelAllUnits( uint8_t NI )
{
	SelectAllUnits( NI, 0 );
	ExBuf[EBPos] = 72;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

void CmdSelAllShips( uint8_t NI )
{
	SelectAllShips( NI, 0 );
	ExBuf[EBPos] = 73;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

void CmdEndGame(uint8_t NI, uint8_t state, uint8_t cause )
{
	ExBuf[EBPos] = 92;
	ExBuf[EBPos + 1] = NI;
	ExBuf[EBPos + 2] = state;
	ExBuf[EBPos + 3] = cause;
	EBPos += 4;

	if ( use_gsc_network_protocol && !PlayGameMode )
	{
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].ColorID == NI )
			{
				SendVictoryState( PINFO[i].ProfileID, state );
				ProcessMessages();
				break;
			}
		}
	}
}

void CmdMoney( uint8_t NI )
{
	ExBuf[EBPos] = 75;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

static void DoCmdAddMoney( uint8_t NI, int Value )
{
	int MAX = 0;
	for ( int i = 0; i < 6; i++ )
	{
		if ( XRESRC( NI, i ) > MAX )
		{
			MAX = XRESRC( NI, i );
		}
	}

	MAX = ( MAX*( Value - 100 ) ) / 100;

	for ( int i = 0; i < 6; i++ )
	{
		AddXRESRC( NI, i, MAX );
	}
}

static void DoCmdMoney( uint8_t NI )
{
	for ( int i = 0; i < 6; i++ )
	{
		AddXRESRC( NI, i, 50000 );
	}
}

void CmdGiveMoney( uint8_t SrcNI, uint8_t DstNI, uint8_t Res, int Amount )
{
	if ( Amount < 0 )
	{
		return;
	}

	ExBuf[EBPos] = 76;
	ExBuf[EBPos + 1] = SrcNI;
	ExBuf[EBPos + 2] = DstNI;
	ExBuf[EBPos + 3] = Res;
	*( (int*) ( ExBuf + EBPos + 4 ) ) = Amount;
	EBPos += 8;
}

void CmdSetStartTime( int* MASK )
{
	ExBuf[EBPos] = 77;
	ExBuf[EBPos + 1] = MyNation;
	memcpy( ExBuf + EBPos + 2, MASK, 4 * 8 );
	EBPos += 34;
}

void CmdSetMaxPingTime( int Time )
{
	ExBuf[EBPos] = 79;
	ExBuf[EBPos + 1] = MyNation;
	*( (word*) ( ExBuf + EBPos + 2 ) ) = Time;
	EBPos += 4;
}

void CmdDoItSlow( uint16_t DT )
{
	ExBuf[EBPos] = 80;
	*( (word*) ( ExBuf + EBPos + 1 ) ) = DT;
	EBPos += 3;
}

void CmdFillFormation( uint8_t NI, uint16_t BrigadeID )
{
	ExBuf[EBPos] = 82;
	ExBuf[EBPos + 1] = NI;
	*( (word*) ( ExBuf + EBPos + 2 ) ) = BrigadeID;
	EBPos += 4;
}

void CmdOfferVoting()
{

	ExBuf[EBPos] = 83;
	ExBuf[EBPos + 1] = 0;
	*( (DWORD*) ( ExBuf + EBPos + 2 ) ) = MyDPID;
	EBPos += 6;

}

static void ComDoVote( uint32_t DPID, uint8_t result );

void CmdVote( uint8_t result )
{
	ExBuf[EBPos] = 84;
	ExBuf[EBPos + 1] = 0;
	*( (DWORD*) ( ExBuf + EBPos + 2 ) ) = MyDPID;
	ExBuf[EBPos + 6] = result;
	EBPos += 7;
	ComDoVote( MyDPID, result );
}

void CmdEndPT()
{
	ExBuf[EBPos] = 85;
	ExBuf[EBPos + 1] = 0;
	EBPos += 2;
}

static void ComEndPT()
{
	PeaceTimeLeft = 0;
}

void CmdSetGuardState( uint8_t NI, uint16_t State )
{
	ExBuf[EBPos] = 86;
	ExBuf[EBPos + 1] = NI;
	*( (word*) ( ExBuf + EBPos + 2 ) ) = State;
	EBPos += 4;
}

void CmdAttackGround( uint8_t NI, int x, int y )
{
	ExBuf[EBPos] = 87;
	ExBuf[EBPos + 1] = NI;
	*( (int*) ( ExBuf + EBPos + 2 ) ) = x;
	*( (int*) ( ExBuf + EBPos + 6 ) ) = y;
	EBPos += 10;
}

void CmdDoGroup( uint8_t NI )
{
	ExBuf[EBPos] = 88;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

void CmdUnGroup( uint8_t NI )
{
	ExBuf[EBPos] = 89;
	ExBuf[EBPos + 1] = NI;
	EBPos += 2;
}

void CmdChangeNatRefTBL( uint8_t * TBL )
{
	ExBuf[EBPos] = 90;
	ExBuf[EBPos + 1] = MyNation;
	memcpy( ExBuf + EBPos + 2, TBL, 8 );
	EBPos += 10;
}

void CmdChangePeaceTimeStage( int Stage )
{
	ExBuf[EBPos] = 91;
	memcpy( ExBuf + EBPos + 1, &Stage, 2 );
	EBPos += 3;
}

void CmdAddMoney( uint8_t NI, uint32_t Value )
{
	ExBuf[EBPos] = 96;
	ExBuf[EBPos + 1] = NI;
    *( (uint32_t*) ( ExBuf + EBPos + 2 ) ) = Value;
	EBPos += 6;
}

static void ComChangePeaceTimeStage( int Stage )
{
	PeaceTimeStage = Stage;
}

static void ChangeNatRefTBL( byte NI, byte* TBL )
{
	int N = NSL[NI];
	if ( N )
	{
		word* mon = Selm[NI];
		word* msn = SerN[NI];
		if ( mon )
		{
			for ( int i = 0; i < N; i++ )
			{
				word ID = mon[i];
				if ( ID != 0xFFFF )
				{
					OneObject* OB = Group[ID];
					if ( OB )
					{
						OB->Selected &= ~GM( NI );
					}
				}
			}
			free( mon );
			free( msn );
		}
		Selm[NI] = nullptr;
		SerN[NI] = nullptr;
		NSL[NI] = 0;
	}
	N = ImNSL[NI];
	if ( N )
	{
		word* mon = ImSelm[NI];
		word* msn = ImSerN[NI];
		if ( mon )
		{
			for ( int i = 0; i < N; i++ )
			{
				word ID = mon[i];
				if ( ID != 0xFFFF )
				{
					OneObject* OB = Group[ID];
					if ( OB )
					{
						OB->ImSelected &= ~GM( NI );
					}
				}
			}
			free( mon );
			free( msn );
		}
		ImSelm[NI] = nullptr;
		ImSerN[NI] = nullptr;
		ImNSL[NI] = 0;
	}
	memcpy( NatRefTBL, TBL, 8 );
}

static void ComAttackGround( byte NI, int x, int y )
{
	x >>= 4;
	y >>= 4;
	int N = NSL[NI];
	word* IDS = Selm[NI];
	word* SNS = SerN[NI];
	for ( int i = 0; i < N; i++ )
	{
		word MID = IDS[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == SNS[i] && OB->newMons->Artpodgotovka && !OB->Sdoxlo )
			{
				OB->NewAttackPoint( x, y, 16 + 128, 0, 0 );
			}
		}
	}
}

static void ComSetGuardState( byte NI, word State )
{
	int N = NSL[NI];
	word* IDS = Selm[NI];
	word* SNS = SerN[NI];

	if ( !( SNS && IDS ) )
	{
		return;
	}

	for ( int i = 0; i < N; i++ )
	{
		word MID = IDS[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !( OB->Sdoxlo || OB->newMons->Capture || OB->NewBuilding || OB->LockType ) )
			{
				OB->Guard = State;
			}
		}
	}
}

bool VotingMode = 0;
static unsigned long LastVotingTime = 0;
static uint32_t VotersList[8];
static uint32_t VotingResult[8];
static char VotingText[500];

static void ComOfferVoting( DWORD DPID )
{
	LastVotingTime = GetTickCount();
	VotingMode = 1;
	sprintf( VotingText, GetTextByID( "PSR_NOREG1" ), "???" );
	for ( int i = 0; i < 8; i++ )
	{
		if ( i < NPlayers )
		{
			VotersList[i] = PINFO[i].PlayerID;
			VotingResult[i] = 0;//PINFO[i].PlayerID==DPID;
		}
		else
		{
			VotersList[i] = 0;
			VotingResult[i] = 0;
        }
		if ( PINFO[i].PlayerID == DPID )
		{
			sprintf( VotingText, GetTextByID( "PSR_NOREG1" ), PINFO[i].name );
        }
    }
}

static void ComDoVote( uint32_t DPID, uint8_t result )
{
	LastVotingTime = GetTickCount();
	for ( int i = 0; i < NPlayers; i++ )if ( PINFO[i].PlayerID == DPID )
	{
		VotingResult[i] = result;
    }
}

void ProcessVotingKeys()
{
	if ( VotingMode )
	{
		if ( !EnterChatMode )
		{
			if ( KeyPressed )
			{
				if ( LastKey == 'y' || LastKey == 'Y' )
				{
					SpecCmd = 171;
					LastKey = 0;
					KeyPressed = 0;
					UnPress();
                }
				if ( LastKey == 'n' || LastKey == 'N' )
				{
					SpecCmd = 172;
					LastKey = 0;
					KeyPressed = 0;
					UnPress();
                }
            }
        }
		bool ALLYES = 1;
		for ( int i = 0; i < NPlayers; i++ )if ( EBufs[i].Enabled )
		{
			if ( VotingResult[i] != 1 )ALLYES = 0;
			if ( VotingResult[i] == 2 )
			{
				if ( GetTickCount() < LastVotingTime + 60000 * 3 - 4000 )
				{
					LastVotingTime = GetTickCount() - 60000 * 3 + 3000;
					char buf[200];
					sprintf( buf, GetTextByID( "PSR_DENY" ), PINFO[i].name );
					CreateTimedHintEx( buf, kSystemMessageDisplayTime, 32 );//Player %s has rejected the offer.
                }
            }
        }
		if ( ALLYES&&GetTickCount() < LastVotingTime + 60000 * 3 - 4000 )
		{
			LastVotingTime = GetTickCount() - 60000 * 3 + 3000;
			char buf[200];
			sprintf( buf, GetTextByID( "PSR_ACCEPT" ) );
			CreateTimedHintEx( buf, kSystemMessageDisplayTime, 32 );//All players have accepted the offer. This game will not be rated.
			DontMakeRaiting();
			if ( use_gsc_network_protocol )
			{
                OpenRef( 0, "GW|norate" );
            }
        }
		if ( GetTickCount() - LastVotingTime > 60000 * 3 )
		{
			VotingMode = 0;
        }
    }
}

void ProcessVotingView()
{
	if ( !EnterChatMode )
	{
		if ( VotingMode )
		{
			//drawing the results of voiting
			RLCFont* FNT = &YellowFont;
			int LocL = 0;
			int MaxLocLx = 0;
			int NL = 1;
			int L = strlen( VotingText );
			for ( int i = 0; i < L; i++ )
			{
				if ( VotingText[i] == '\\' )
				{
					if ( LocL > MaxLocLx )MaxLocLx = LocL;
					LocL = 0;
					NL++;
				}
				else
				{
					int DL;
					LocL += GetRLCWidthUNICODE( FNT->RLC, (byte*) ( VotingText + i ), &DL );
					i += DL - 1;
                }
            }
			if ( LocL > MaxLocLx )MaxLocLx = LocL;
			int xc = RealLx >> 1;
			int y0 = RealLy / 2 - NPlayers * 13 - NL * 13 - 32;
			int x0 = xc - MaxLocLx / 2 - 20;
			DrawDoubleTable2( x0, y0 - 16, x0 + MaxLocLx + 40, y0 + 16 + 16 + 4 + ( NL + NPlayers ) * 26, y0 + 16 + NL * 26 );
			char TempStr[128];
            int TempPos = 0;
			for ( int i = 0; i < L; i++ )
			{
				if ( VotingText[i] == '\\' || i == L - 1 )
				{
					if ( VotingText[i] != '\\' )
					{
						TempStr[TempPos] = VotingText[i];
						TempStr[TempPos + 1] = 0;
						TempPos++;
                    }
					ShowString( xc - GetRLCStrWidth( TempStr, FNT ) / 2, y0, TempStr, FNT );
					y0 += 26;
					TempPos = 0;
					TempStr[0] = 0;
				}
				else
				{
					TempStr[TempPos] = VotingText[i];
					TempStr[TempPos + 1] = 0;
					TempPos++;
                }
            }
			y0 += 32;
			for ( int i = 0; i < NPlayers; i++ )
			{
				char ccx[65];
				switch ( VotingResult[i] )
				{
				case 1:
					sprintf( ccx, "%s:%s", PINFO[i].name, GetTextByID( "VOTE_YES" ) );
					break;
				case 2:
					sprintf( ccx, "%s:%s", PINFO[i].name, GetTextByID( "VOTE_NO" ) );
					break;
				default:
					sprintf( ccx, "%s:%s", PINFO[i].name, GetTextByID( "VOTE_UNK" ) );
					break;
                }
				ShowString( xc - GetRLCStrWidth( ccx, FNT ) / 2, y0, ccx, FNT );
				y0 += 26;
            }
        }
    }
}

static void FillFormation( Brigade* BR )
{
    word ULOCAL[512];
    int N1 = 0;
    int N2 = 0;
    int xx = 0;
    int yy = 0;
    bool SELECTED = 0;
    bool ImSELECTED = 0;
    byte NI = BR->CT->NI;
    int NN = ElementaryOrders[BR->WarType - 1].NUnits + 2;
    for ( int i = 2; i < NN; i++ )
    {
        if ( BR->Memb[i] == 0xFFFF )N1++;
        else
        {
            OneObject* OB = Group[BR->Memb[i]];
            if ( OB && !OB->Sdoxlo )
            {
                xx += OB->RealX;
                yy += OB->RealY;
                N2++;
                if ( OB->Selected&GM( NI ) )SELECTED = 1;
                if ( OB->ImSelected&GM( NI ) )ImSELECTED = 1;
            }
            else N1++;
        }
    }
    if ( !( N2&&N1 ) )return;
    xx /= N2;
    yy /= N2;

    int N = FindUnits( xx, yy, MobilR * 16, BR->MembID, BR->CT->NI, N1, ULOCAL );
    N2 = 0;
    for ( int i = 2; i < NN&&N; i++ )
    {
        bool INS = 0;
        if ( BR->Memb[i] == 0xFFFF )INS = 1;
        else
        {
            OneObject* OB = Group[BR->Memb[i]];
            INS = !( OB && !OB->Sdoxlo );
        }
        if ( INS )
        {
            OneObject* OB = Group[ULOCAL[N2]];
            BR->Memb[i] = ULOCAL[N2];
            BR->MembSN[i] = OB->Serial;
            OB->BrigadeID = BR->ID;
            OB->BrIndex = i;
            OB->InArmy = true;
            OB->AddDamage = BR->AddDamage;
            OB->AddShield = BR->AddShield;
            OB->StandGround = 0;
            ( &BR->BM.Peons )[GetBMIndex( OB )]++;
            N2++;
            N--;
        }
    }
    assert( BR->NMemb == NN );
    CancelStandGroundAnyway( BR );
    assert( BR->NMemb == NN );
    BR->CreateOrderedPositions( xx, yy, BR->Direction );
    assert( BR->NMemb == NN );
    BR->KeepPositions( 0, 128 + 16 );
    assert( BR->NMemb == NN );
    if ( SELECTED )
    {
    }
    if ( ImSELECTED )
    {
    }
    BR->NMemb = NN;
}

static void DoFillFormation( byte NI, word BrigadeID )
{
	Brigade* BR = CITY[NatRefTBL[NI]].Brigs + BrigadeID;
	if ( BR->WarType )FillFormation( BR );
}

static bool SlowMade = 0;

static void DoItSlow( word /*DT*/ )
{
	if ( !SlowMade )
	{
		if ( CurrentStepTime < 160 )CurrentStepTime += CurrentStepTime >> 4;
		//if(DT>3000)NeedCurrentTime+=DT>>1;
		SlowMade = 1;
    }
}

static int MaxDCT = 0;

static void DoTimeStep( int DT )
{
	if ( DT > MaxDCT )MaxDCT = DT;
	/*
	assert(MaxDCT<40000);
	FILE* F=fopen("log1.log","a");
	if(F){
		fprintf(F,"DT=%d Max=%d t=%d\n",DT,MaxDCT,tmtmt);
		fclose(F);
    }
	*/
}

static int CURMAXP = -1;

static void SETPING( int T )
{
	for ( int i = 0; i < EBPos; i++ )
	{
		if ( T > CURMAXP )
		{
			CURMAXP = T;
			MaxPingTime = T;
		}
	}
}

static void __DoStartTime( int* /*MASK*/ )
{
	PSUMM.ClearPingInfo();
}

static void DoCmdGiveMoney( byte SrcNI, byte DstNI, byte Res, int Amount )
{
	int r = XRESRC( SrcNI, Res );
	if ( r < Amount )Amount = r;
	AddXRESRC( SrcNI, Res, -Amount );
	AddXRESRC( DstNI, Res, Amount - ( 3 * ( Amount / 10 ) ) );
	CreateTimedHintEx( GetTextByID( "MONTR" ), kSystemMessageDisplayTime, 32 );//Resources have been successfully transferred.
}

int WaitState = 0;
bool PreNoPause = 0;
int ShowGameScreen = 0;

static void __EndGame( byte NI, byte state )
{
	int state0 = state;
	if ( state > 2 )
	{
		SetBrokenState();
	}

	if ( state == 3 || state == 4 )
	{
		//is there alive friends?
		byte mask = NATIONS[NI].NMask;
		int MaxSC = -1000;
		int BestID = -1;
		for ( int i = 0; i < 7; i++ )
		{
			if ( NATIONS[i].NMask&mask&&i != NI&&NATIONS[i].VictState == 0 )
			{
				//yes, friends are.
				int score = CITY[i].Account;
				if ( score > MaxSC )
				{
					MaxSC = score;
					BestID = i;
				}
			}
		}

		if ( BestID == -1 )
		{
			//need to eliminate all
			for ( int i = 0; i < 7; i++ )
			{
				if ( NATIONS[i].NMask&mask )
				{
					CITY[i].AutoEraseTime = 80;
				}
			}
		}

		if ( state == 4 )
		{
			state = 1;
		}
	}

	if ( state == 5 )
	{
		state = 3;
	}
	NATIONS[NI].VictState = state;
	if ( NI == MyNation )
	{
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( EBufs[i].Enabled )
			{
				EBufs[i].Enabled = PINFO[i].ColorID == NI;
			}
		}
		PreNoPause = 1;

		if ( NPlayers > 1 )
		{
			FogMode = 0;
		}

		WaitState = 1;
		for ( int i = 0; i < NPlayers; i++ )if ( PINFO[i].ColorID == NI )
		{
			if ( state == 1 )
			{
				LOOSEIGAME( PINFO[i].name );
			}
			else
			{
				if ( state == 2 )
				{
					WINIGAME( PINFO[i].name );
				}
			}
		}
	}
	else
	{
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( EBufs[i].Enabled )
			{
				EBufs[i].Enabled = PINFO[i].ColorID != NI;
			}
		}
	}

	WinnerControl( true );
	MI_Mode = 1;
	CUR_TOOL_MODE = 0;
	TP_Made = 0;

	if ( NPlayers == 2 && SCENINF.hLib && state == 1 && NI != MyNation )
	{
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( PINFO[i].PlayerID == MyDPID )
			{
				SCENINF.Victory = 1;
				NOPAUSE = 0;
				LockPause = 1;
				ShowStat = 0;
				NATIONS[PINFO[i].ColorID].VictState = 2;
			}
		}
	}

	if ( use_gsc_network_protocol && !PlayGameMode )
	{
		for ( int i = 0; i < NPlayers; i++ )if ( PINFO[i].ColorID == NI )
		{
			SendVictoryState( PINFO[i].ProfileID, state0 );
			ProcessMessages();
			break;
		}
	}
}


static void ChangePauseState()
{
	NOPAUSE = !NOPAUSE;
}

static void InitSelection()
{
	int sz = ( MAXOBJECT >> 2 ) + 128;
	if ( sz > 16384 )sz = 16384;
	// BoonXRay 13.08.2017
	//__asm {
	//	push	edi
	//	mov		ecx, sz
	//	cld
	//	xor		eax, eax
	//	mov		edi, offset CmdDone
	//	rep		stosd
	//	pop		edi
    //}
	memset(CmdDone, 0, sz * sizeof(int));
}

static void ImCorrectShipsSelection( byte NI )
{
	int N = ImNSL[NI];
	word* SU = ImSelm[NI];
	word* SN = ImSerN[NI];
	int NB = 0;
	int NU = 0;
	int NS = 0;
	bool Usual = 0;
    for ( int i = 0; i < N; i++ )
	{
		word MID = SU[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == SN[i] && !OB->Sdoxlo )
			{
				byte Use = OB->newMons->Usage;
				if ( OB->LockType )
				{
					if ( Use == FisherID )NU++;
					else NS++;
				}
				else
				{
					if ( OB->NewBuilding || OB->Wall )NB++;
					else
					{
						NU++;
                        if ( OB->Guard == 0xFFFF ) Usual = 1;
                    }
                }
            }
        }
    }
	//if(NU>1)Usual=1;
	int RealN = 0;
	if ( NU && ( NU > NS ) )
	{
		//select only units
		for ( int i = 0; i < N; i++ )
		{
			word MID = SU[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->Serial == SN[i] && !OB->Sdoxlo )
				{
					OB->ImSelected &= ~GM( NI );
					byte Use = OB->newMons->Usage;
					if ( OB->LockType )
					{
						if ( Use == FisherID )
						{
							SU[RealN] = OB->Index;
							SN[RealN] = OB->Serial;
							RealN++;
							OB->ImSelected |= GM( NI );
                        }
					}
					else
					{
						if ( !( OB->NewBuilding || OB->Wall ) )
						{
							if ( !( Usual&&OB->Guard != 0xFFFF ) )
							{
								SU[RealN] = OB->Index;
								SN[RealN] = OB->Serial;
								RealN++;
								OB->ImSelected |= GM( NI );
                            }
                        }
                    }
                }
            }
        }
		ImNSL[NI] = RealN;
	}
	else
	{
		if ( NS )
		{
			//select ships only
			for ( int i = 0; i < N; i++ )
			{
				word MID = SU[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB&&OB->Serial == SN[i] && !OB->Sdoxlo )
					{
						OB->ImSelected &= ~GM( NI );
						byte Use = OB->newMons->Usage;
						if ( OB->LockType&&Use != FisherID )
						{
							SU[RealN] = OB->Index;
							SN[RealN] = OB->Serial;
							RealN++;
							OB->ImSelected |= GM( NI );
                        }
                    }
                }
            }
			ImNSL[NI] = RealN;
        }
    }
}

static void SmartSelectionCorrector( byte NI, word* Mon, int N );

static void CreateGoodSelection( byte NI, word xx, word yy, word xx1, word yy1, CHOBJ* FN, int NN, bool Addon )
{
    int x = xx >> 5;
	int y = yy >> 5;
	int x1 = xx1 >> 5;
	int y1 = yy1 >> 5;
	if ( x > 1 )x--;
	if ( y > 1 )y--;
	if ( x1 < msx )x1++;
	if ( y1 < msy )y1++;
	//if(xx1&31)x1++;
	//if(yy1&31)y1++;
	word MID;
	OneObject* OB;
	word ns = 0;
	word Nsel = ImNSL[NI];
	word* SMon = ImSelm[NI];
	word* ser = ImSerN[NI];
	word* tmpSelm = nullptr;
	word* tmpSerN = nullptr;
	int tmpN = 0;
	if ( Nsel )
	{
		tmpSelm = new word[Nsel];
		tmpSerN = new word[Nsel];
		memcpy( tmpSelm, SMon, Nsel << 1 );
		memcpy( tmpSerN, ser, Nsel << 1 );
		tmpN = Nsel;
    }
	if ( ( !Addon ) && Nsel )
	{
		//CmdFreeSelected(NI);
		for ( int i = 0; i < Nsel; i++ )
		{
			MID = SMon[i];
			if ( MID != 0xFFFF )
			{
				OB = Group[MID];
				if ( int( OB ) && OB->NNUM == NatRefTBL[NI] && ser[i] == OB->Serial )OB->ImSelected &= ~GM( NI );
            }
        }
		if ( int( ImSelm[NI] ) )
		{
			free( ImSelm[NI] );
			ImSelm[NI] = nullptr;
        }
		if ( int( ImSerN[NI] ) )
		{
			free( ImSerN[NI] );
			ImSerN[NI] = nullptr;
        }
    }

	int Olds = 0;
	if ( Addon )Olds = Nsel;
	word nnm = GoodSelectNewMonsters( NI, xx, yy, xx1, yy1, nullptr, nullptr, false, FN, NN, ULIMIT );
	if ( Olds + ns + nnm )
	{
		ImSelm[NI] = new word[Olds + ns + nnm];
		ImSerN[NI] = new word[Olds + ns + nnm];
	}
	else
	{
		ImNSL[NI] = 0;
		SmartSelectionCorrector( NI, tmpSelm, tmpN );
		if ( tmpSelm )
		{
			free( tmpSelm );
			free( tmpSerN );
        }
		return;
    }
	if ( Addon&&Nsel )
	{
		memcpy( ImSelm[NI], SMon, Olds << 1 );
		memcpy( ImSerN[NI], ser, Olds << 1 );
    }
	word* SM = ImSelm[NI];
	word* SR = ImSerN[NI];
	ns = Olds;

	GoodSelectNewMonsters( NI, xx, yy, xx1, yy1, &SM[ns], &SR[ns], true, FN, NN, nnm );

	ImNSL[NI] = ns + nnm;

	if ( Addon&&Nsel )
	{
		free( SMon );
		free( ser );
    }
	ImCorrectShipsSelection( NI );
	ImCorrectBrigadesSelection( NI );
	SmartSelectionCorrector( NI, tmpSelm, tmpN );
	//ImCorrectBrigadesSelection(NI);
	if ( tmpSelm )
	{
		free( tmpSelm );
		free( tmpSerN );
    }
}

static OneObject* SearchGate( int x, int y )
{
	for ( int dx = -1; dx <= 1; dx++ )
		for ( int dy = -1; dy <= 1; dy++ )if ( dx || dy )
		{
			WallCell* WCL = WRefs[GetLI( x + dx, y + dy )];
			if ( WCL&&WCL->Sprite >= 32 && WCL->Sprite < 64 )
			{
				OneObject* OB = Group[WCL->OIndex];
				if ( OB&&OB->Wall )return OB;
            }
        }
	return nullptr;
}

static void RefreshSelected( byte NI )
{
	int RNSel = 0;
	word* SMon = Selm[NI];
	word* ser = SerN[NI];
	if ( !SMon )NSL[NI] = 0;
	word Nsel = NSL[NI];
	word MID;
	for ( int k = 0; k < Nsel; k++ )
	{
		MID = SMon[k];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == ser[k] && !OB->Sdoxlo )
			{
				if ( OB->Wall )
				{
					int id = GetLI( OB->WallX, OB->WallY );
					WallCell* WCL = WRefs[id];
					if ( WCL )
					{
						if ( WCL->Sprite >= 64 )
						{
							OneObject* GT = SearchGate( OB->WallX, OB->WallY );
							if ( GT && !( GT->Selected&GM( NI ) ) )
							{
								OB->Selected &= ~GM( NI );
								GT->Selected |= GM( NI );
								SMon[k] = GT->Index;
								ser[k] = GT->Serial;
                            }
                        }
                    }
                }
				bool Good = true;
				if ( OB->InArmy&&OB->BrIndex < 2 && OB->BrigadeID != 0xFFFF )
				{
					OB->Selected &= ~GM( NI );
					Good = false;
                }
				if ( Good )
				{
					SMon[RNSel] = SMon[k];
					ser[RNSel] = ser[k];
					RNSel++;
                }
            }
        }
    }
	NSL[NI] = RNSel;
	//----------------------------IM-------------------------//
	RNSel = 0;
	Nsel = ImNSL[NI];
	SMon = ImSelm[NI];
	ser = ImSerN[NI];
	for ( int k = 0; k < Nsel; k++ )
	{
		MID = SMon[k];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == ser[k] && !OB->Sdoxlo )
			{
				if ( OB->Wall )
				{
					int id = GetLI( OB->WallX, OB->WallY );
					WallCell* WCL = WRefs[id];
					if ( WCL )
					{
						if ( WCL->Sprite >= 64 )
						{
							OneObject* GT = SearchGate( OB->WallX, OB->WallY );
							if ( GT && !( GT->ImSelected&GM( NI ) ) )
							{
								OB->ImSelected &= ~GM( NI );
								GT->ImSelected |= GM( NI );
								SMon[k] = GT->Index;
								ser[k] = GT->Serial;
                            }
                        }
                    }
                }
				bool Good = true;
				if ( OB->InArmy&&OB->BrIndex < 2 && OB->BrigadeID != 0xFFFF )
				{
					OB->ImSelected &= ~GM( NI );
					Good = false;
                }
				if ( Good )
				{
					SMon[RNSel] = SMon[k];
					ser[RNSel] = ser[k];
					RNSel++;
                }
            }
        }
    }
	ImNSL[NI] = RNSel;
}

bool OneDirection = 0;

void SendSelectedToXY(uint8_t NI, int xx, int yy, short Dir, uint8_t Prio, uint8_t Type )
{
	OneDirection = 0 != ( Type & 16 );
	LastDirection = Dir;
	GroupSendSelectedTo( NI, xx, yy, Prio, Type & 7 );
	LastDirection = 512;
	return;
}

void SendSelectedToXY( byte NI, int xx, int yy, short Dir, byte Type )
{
	SendSelectedToXY( NI, xx, yy, Dir, 16 + 128, Type );
}

/*
//Adjusts FrmDec, SpeedSh and exFMode
//Called only from ExecuteBuffer()
//void ChGSpeed()//BUGFIX: real time speed changing
{
	if ( 1 == NPlayers )
	{//Allow ingame speed changing only in single player
		if ( FrmDec == 1 )
		{
			FrmDec = 2;
			SpeedSh = 1;
		}
		else
		{
			FrmDec = 1;
			SpeedSh = 0;
		}
		exFMode = SpeedSh;
	}
}
*/

//Adjusts FrmDec, SpeedSh and exFMode
//Called only from ExecuteBuffer()
static void SetGSpeed( byte speed )
{
	if ( 1 == NPlayers )//BUGFIX: allow ingame speed changing in single player only
	{
		FrmDec = speed + 1;
		SpeedSh = speed;
		exFMode = speed;
	}
}

static void AttackToXY( byte NI, byte x, byte y )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	int Sqs = int( sqrt( Nsel ) );
	int Glx = Sqs;
	int Gly = Sqs;
	if ( Glx*Gly > Nsel )Glx--;
	if ( Glx*Gly < Nsel )Glx++; else
	{
		if ( Glx*Gly > Nsel )Gly--;
		if ( Glx*Gly < Nsel )Gly++;
    }
	int gx1 = x - ( Glx >> 1 );
	int gy1 = y - ( Gly >> 1 );
	if ( gx1 <= 0 )gx1 = 1;
	if ( gy1 <= 0 )gy1 = 1;
	if ( gx1 + Glx > msx )gx1 = msx - Glx + 1;
	if ( gy1 + Gly > msy )gy1 = msy - Gly + 1;
	int zx = gx1;
	int zy = gy1;
	for ( int i = 0; i < Nsel; i++ )
	{
		if ( zx - gx1 + 1 > Glx )
		{
			zx = gx1; zy++;
        }
		MID = SMon[i];
		if ( !( CmdDone[MID] || MID == 0xFFFF ) && int( Group[MID] ) )
			//Group[SMon[i]]->SendTo(zx,zy,129);
			zx++;
    }
}

static void AttackSelected( byte NI, word ObjID, byte OrdType, short DIR )
{
	LastDirection = DIR;
	OneObject* OB = Group[ObjID];
	if ( OB && !OB->Sdoxlo )GroupAttackSelectedBrigadesTo( NI, OB->RealX, OB->RealY, 128 + 16, 0 );
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
		{
			if ( Group[MID]->AttackObj( ObjID, 126 + 128, OrdType ) )CmdDone[MID] = 1;
        }
    }
	LastDirection = 512;
}

void BuildWithSelected(uint8_t NI, uint16_t ObjID, uint8_t OrdType )
{
	if ( ObjID == 0xFFFF )return;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* Sm = SerN[NI];
	if ( !Nsel )return;
	OneObject* OBJ = Group[ObjID];
	if ( !OBJ )return;
	Nsel = SortUnitsByR( SMon, Sm, Nsel, OBJ->RealX, OBJ->RealY );
	NSL[NI] = Nsel;
	word MID;
	bool CanAct = false;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( ( !( MID == 0xFFFF/*||CmdDone[MID]*/ ) ) && int( Group[MID] ) )
        {
			if ( OrdType != 1 )
			{
				rando();
				if ( Group[MID]->BuildObj( ObjID, 128 + 16, true, OrdType ) )
					CanAct = true;
			}
			else
			{
				OneObject* OB = Group[MID];
				if ( OB )
				{
					Order1* OR1 = OB->LocalOrder;
					if ( OR1 )
					{
						if ( OR1->DoLink == NewMonsterSendToLink&&OR1->NextOrder )OR1 = OR1->NextOrder;
						if ( OR1->DoLink == NewMonsterSmartSendToLink&&OR1->NextOrder )OR1 = OR1->NextOrder;
                    }
					if ( OR1&&OR1->DoLink == &BuildObjLink )
					{
						rando();
						if ( Group[MID]->BuildObj( ObjID, 128 + 16, true, 2 ) )
							CanAct = true;
					}
					else
					{
						rando();
						if ( NATIONS[NI].AI_Enabled )
						{
							if ( Group[MID]->BuildObj( ObjID, 128 + 127, true, OrdType ) )
								CanAct = true;
						}
						else
						{
							if ( Group[MID]->BuildObj( ObjID, 128 + 16, true, OrdType ) )
								CanAct = true;
                        }
                    }
                }
            }
        }
    }
	if ( CanAct )
	{
		for ( int i = 0; i < Nsel; i++ )
		{
			MID = SMon[i];
			if ( MID != 0xFFFF && Group[MID] )CmdDone[MID] = true;
        }
    }
	OneObject* OB = Group[ObjID];
	if ( OB )OB->ClearBuildPt();
}

static void CreateTerrainMons( byte NI, byte x, byte y )
{
	NATIONS[NI].CreateNewMonsterAt( int( x ) * 512, int( y ) * 512, 0, false );
}

static void CreateNewTerrMons( byte NI, int x, int y, word Type )
{
	if ( Type & 32768 )
	{
		int ID = NATIONS[NI].CreateNewMonsterAt( int( x ) << 4, int( y ) << 4, Type & 8191, false );
		if ( ID != -1 )
		{
			OneObject* OB = Group[ID];
			if ( OB )
			{
				NewMonster* NM = OB->newMons;
				if ( NM->Building )
				{
					do
					{
						OB->NextStage();
					} while ( OB->Life < OB->MaxLife );
                }
            }
        }
		return;
    }
	//NATIONS[NI].CreateTerrainAt(x,y,Type);
	//CreateUnit(&NATIONS[NI],x,y,Type);
	NATIONS[NI].CreateNewMonsterAt( int( x ) << 4, int( y ) << 4, Type, false );
}

int CreateNewTerrMons2(uint8_t NI, int x, int y, uint16_t Type )
{
	int ID = NATIONS[NI].CreateNewMonsterAt( x, y, Type & 8191, true );
	if ( ID != -1 )
	{
		OneObject* OB = Group[ID];
		if ( OB )
		{
			NewMonster* NM = OB->newMons;
			if ( NM->Building )
			{
				do
				{
					OB->NextStage();
				} while ( OB->Life < OB->MaxLife );
            }
        }
		return ID;
    }
	return -1;
}

static void CreateBuilding( byte NI, int x, int y, word Type, byte OrdType )
{
	int j = NATIONS[NatRefTBL[NI]].CreateNewMonsterAt( x, y, Type, false );
	if ( j != -1 )
	{
		BuildWithSelected( NI, j, OrdType );
	}
}

void ProduceObject(uint8_t NI, uint16_t Type )
{
	int maxp = 1000;
	int kk = -1;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	for ( int k = 0; k < Nsel; k++ )
	{
		word MID = SMon[k];
		if ( !( MID == 0xFFFF/*||CmdDone[MID]*/ ) )
		{
			OneObject* OB = Group[SMon[k]];
			if ( int( OB ) )
			{
				int pp = OB->CheckAbility( Type & 8191, false );
				if ( pp == -1 && OB->Ready )
				{
					if ( Type & 8192 )OB->Produce( Type & 8191, 0xFFFE );
					else OB->Produce( Type & 8191, 0xFFFF );
					CmdDone[MID] = true;
					return;
                }
				if ( pp > 0 && pp < maxp )
				{
					maxp = pp;
					kk = k;
                }
            }
        }
    }
	if ( kk != -1 )
	{
		word MID = SMon[kk];
		if ( !( MID == 0xFFFF/*||CmdDone[MID]*/ ) )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( Type & 8192 )OB->Produce( Type & 8191, 0xFFFE );
				else OB->Produce( Type & 8191, 0xFFFF );
				CmdDone[MID] = true;
            }
        }
    }
}

static void ProduceObjects( byte NI, byte N, word Type )
{
	for ( int i = 0; i < N; i++ )
	{
		TakeUnitFromCash( NI, Type );
		ProduceObject( NI, Type );
	}
}

//Cancels queued production of selected unit in selected building
//OneObject *OB: Building with production order to be canceled
//word Type: Object index of the unit type to be canceled
//Iterates through all queued productions in the building
//Unlinks the order with provided unit type from the chain
//Refunds resources spent on unit construction
void UnProduce( OneObject* OB, uint16_t Type )
{
	if ( !( OB->NewBuilding && OB->LocalOrder ) )
	{
		return;
	}

	Order1* order = OB->LocalOrder;
	if ( !order )
	{
		return;
	}
	Order1* prev_order = order;

	//True if there are production orders waiting for the running production to finish
	bool no_queued_orders = false;

	order = order->NextOrder;
	if ( !order )
	{//There are no orders queued after current production order
		no_queued_orders = true;
		order = prev_order;
		prev_order = nullptr;
	}

StartUn:
	do
	{
		if ( 13 == order->OrderType )
		{
			if ( order->info.Produce.ObjIndex == Type )
			{
				if ( prev_order )
				{//Link the order before the one beeing canceled to the next one in chain
					prev_order->NextOrder = order->NextOrder;
				}
				else
				{//This is the oldest order, just link to the next one in chain
					OB->LocalOrder = order->NextOrder;
				}

				word Power = order->info.Produce.Power;

				if ( OB->newMons->Port )
				{
					if ( order->info.Produce.Progress )
					{
						MFIELDS[1].BClrBar( OB->WallX - 7, OB->WallY - 7, 15 );
					}
				}

				//Free the unlinked order struct
				OB->FreeOrdBlock( order );

				//Refund resources spent on queued unit
				if ( Power != 0xFFFF )
				{
                    //GeneralObject* GO = OB->Nat->Mon[Type];
					int Cost[8];
					GetUnitCost( OB->NNUM, Type, Cost, Power );
					for ( int p = 0; p < 8; p++ )
					{
						AddXRESRC( OB->NNUM, p, Cost[p] );
					}
				}

				return;
			}
		}

		if ( no_queued_orders )
		{//Nothing to iterate through after this one
			return;
		}

		//Still here, continue with the next order in chain
		prev_order = order;
		order = order->NextOrder;
	} while ( order );

	if ( !no_queued_orders )
	{
		order = OB->LocalOrder;
		prev_order = nullptr;
		no_queued_orders = true;
		goto StartUn;
	}
}

//Cancels production of units in selected buildings
//byte NI: nation index of player
//word Type: id of unit to be cancelled
//Iterates through player's selection and looks for
//matching production queues, then calls UnProduce()
static void UnProduceObject( byte NI, word Type )
{
	//Index of building in selection, whose queue will be canceled
	int target_index = -1;

	int max_queue_count = 0;

	word* selection = Selm[NI];

	const int selection_size = NSL[NI];
	for ( int i = 0; i < selection_size; i++ )
	{
		const int global_index = selection[i];
		if ( !( 0xFFFF == global_index || CmdDone[global_index] ) )
		{
			OneObject* object = Group[global_index];
			if ( nullptr != object )
			{
				//Get the amount of queued unit productions in this building
				const int queue_count = object->CheckAbility( Type, true );
				if ( queue_count > 0 && queue_count > max_queue_count )
				{
					max_queue_count = queue_count;
					target_index = i;
				}
			}
		}
	}

	if ( target_index != -1 )
	{
		word MID = selection[target_index];
		if ( !( MID == 0xFFFF || CmdDone[MID] ) )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				UnProduce( OB, Type );
			}
		}
	}
}

static void MemSelection( byte NI, byte Index )
{
	if ( NI > 7 )return;
	SelSet[NI * 10 + Index].CreateFromSelection( NI );
}

static void RememSelection( byte NI, byte Index )
{
	if ( ( ( NI & 31 ) * 10 + Index ) >= 80 )
	{
		return;
	}
	bool shift = 0 != ( NI >> 5 );
	SelSet[( NI & 31 ) * 10 + Index].SelectMembers( NI & 31, shift );
	CorrectBrigadesSelection( NI );
}

static int CreateWall( byte NI, byte* lp )
{
	WallCluster WCLT;
	WCLT.CreateByData( (word*) lp );
	WCLT.CreateSprites();
	WSys.AddCluster( &WCLT );
	WCLT.SendSelectedToWork( NI, 0 );

	return ( (word*) lp )[0];
}

static void RepairWall( byte NI, short xx, short yy, byte OrdType )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* Sm = SerN[NI];
	if ( !Nsel )return;
	Nsel = SortUnitsByR( SMon, Sm, Nsel, ( xx << 10 ) + 512, ( yy << 10 ) + 512 );
	NSL[NI] = Nsel;
	word MID;
	bool CanAct = false;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( ( !( MID == 0xFFFF || CmdDone[MID] ) ) && int( Group[MID] ) )
			if ( Group[MID]->BuildWall( xx, yy, 16, OrdType, true ) )
				CanAct = true;
    }
	if ( CanAct )
	{
		for ( int i = 0; i < Nsel; i++ )
		{
			MID = SMon[i];
			if ( MID != 0xFFFF && Group[MID] )
			{
				CmdDone[MID] = true;
				TempUnLock( Group[MID] );
            }
        }
    }
}

static void PerformUpgr( byte NI, word UI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
			Group[SMon[i]]->PerformUpgrade( UI, MID );
    }
}

static void SetDestination( byte NI, int x, int y )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
			Group[MID]->SetDstPoint( x, y );
    }
}

static void Stopp( byte NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				OB->ClearOrders();
            }
        }
    }
}

static void StandGround( byte NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
		{
			OneObject* OB = Group[MID];
			OB->StandGround = true;
        }
    }
}

void PatrolGroup(uint8_t NI, int x1, int y1, uint8_t Dir )
{
	LastDirection = Dir;
	GroupPatrolSelected( NI, x1, y1, 0 );
	LastDirection = 512;
}

static void GroupAttackPoint( byte NI, byte x, byte y, byte kind )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
		{
			OneObject* OB = Group[MID];
			if ( OB )OB->AttackPoint( x, y, kind, 16 );
        }
    }
}

//Kills units / buildings which are destroyed manually through DEL key
void DieSelected(uint8_t NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];

	if ( !Nsel )
	{
		return;
	}

	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				DestructBuilding( OB );
			}
		}
	}
}

void EraseSelected(uint8_t NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				OB->delay = 6000;
				if ( OB->LockType )OB->RealDir = 32;
				OB->Die();
				OB = Group[MID];
				if ( OB )
				{
					OB->Sdoxlo = 2500;
					if ( OB->NewBuilding )
					{
						EliminateBuilding( OB );
						Group[MID] = nullptr;
                    }
                }
            }
        }
    }
}

static void ContinueAttPoint( byte NI, byte x, byte y )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
			Group[MID]->ContinueAttackPoint( x, y, 16 );
    }
}

static void ContinueAttWall( byte NI, byte x, byte y )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
			Group[MID]->ContinueAttackWall( x, y, 16 );
    }
}

static void SitDown( byte /*NI*/ )
{
    // BoonXRay 10.09.2017
    /*word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
		{
			OneObject* OB = Group[MID];
			//if(OB)OB->MakeMeSit();
        }
    }*/
}

//*******************************************************************************//
//****************                                                 **************//
//*******                          SAVING IPX GAME                         ******//
//****************                                                 **************//
//*******************************************************************************//

static void SaveNetworkGame( int ID, const char* Name )
{
	ShowCentralText0( GetTextByID( "Autosaving" ) );

	FlipPages();

	char str[128];
	strcpy( str, Name );
	strcat( str, ".sav" );
	char cc1[128];
	strcpy( cc1, str );
	cc1[12] = 0;
	if ( !strcmp( cc1, "NetAutoSave " ) )
	{
		int N = 0;
		for ( int i = 0; i < NPlayers; i++ )
		{
			if ( EBufs[i].Enabled )N++;
        }
		cc1[12] = '0' + N;
		strcpy( str, cc1 );
    }
	strcpy( LASTSAVEFILE, str );
	SaveGame( str, Name, ID );
}

static void LoadNetworkGame( char* Name )
{
	if ( Name[0] == 0 )
	{
		return;
	}

	PlayerInfo LOC_PINFO[8];
	memcpy( LOC_PINFO, PINFO, sizeof LOC_PINFO );

	byte mm[8];
	memcpy( mm, NatRefTBL, 8 );

	PBACK.Clear();
	RETSYS.Clear();

	int T0 = GetRealTime();
	do
	{
		ProcessMessages();
	} while ( GetRealTime() - T0 < 5000 );

	PBACK.Clear();
	RETSYS.Clear();
	strcpy( SaveFileName, Name );

	if ( MaxPingTime )
	{
		MaxPingTime = 700;
		PitchTicks = 8;
	}

	PREVGLOBALTIME = 0;
	LoadSaveFile();
	NoWinner = 0;

	for ( int i = 0; i < NPlayers; i++ )
	{
		EBufs[i].RealTime = 0;
		EBufs[i].Size = 0;
		EBufs[i].RandIndex = 0;
		EBufs1[i].RealTime = 0xFFFFFFFF;
		EBufs1[i].Size = 0;
		EBufs[i].RandIndex = 0;
	}

	EBPos1 = 0;
	PrevRpos = 0;
	RealTime += 64;
	memcpy( NatRefTBL, mm, 8 );

	if ( NPlayers > 1 )
	{
		memcpy( PINFO, LOC_PINFO, sizeof LOC_PINFO );
		byte MyExNation = MyNation;
		for ( int i = 0; i < NPlayers; i++ )
		{
			char* loop_name = PINFO[i].name;
			for ( int j = 0; j < PL_NPlayers; j++ )
			{
				if ( !strcmp( loop_name, PL_Names[j] ) )
				{
					if ( MyExNation == LOC_PINFO[i].ColorID )
					{
						SetMyNation( PL_Colors[j] );
					}
					PINFO[i].ColorID = PL_Colors[j];
				}
			}
		}
		for ( int i = NPlayers; i < PL_NPlayers; i++ )
		{
			char* S = PL_Names[i];
			if ( !( strcmp( S, "Very hard" ) && strcmp( S, "Hard" ) && strcmp( S, "Normal" ) && strcmp( S, "Easy" ) ) )
			{
				strcpy( PINFO[i].name, PL_Names[i] );
				PINFO[i].ColorID = PL_Colors[i];
			}
			else
			{
				PINFO[i].name[0] = 0;
			}
		}
		memcpy( NatRefTBL, PL_NatRefTBL, 8 );
	}
}

static void ChooseUnSelectType( byte NI, word ID )
{
	int RNSel = 0;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* SNM = SerN[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->NIndex != ID || OB->InArmy )
				{
					SMon[RNSel] = SMon[i];
					SNM[RNSel] = SNM[i];
					RNSel++;
				}
				else
				{
					OB->Selected &= ~GM( NI );
                }
            }
        }
    }
	NSL[NI] = RNSel;
}

static void ImChooseUnSelectType( byte NI, word ID )
{
	int RNSel = 0;
	word Nsel = ImNSL[NI];
	word* SMon = ImSelm[NI];
	word* SNM = ImSerN[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->NIndex != ID || OB->InArmy )
				{
					SMon[RNSel] = SMon[i];
					SNM[RNSel] = SNM[i];
					RNSel++;
				}
				else
				{
					OB->ImSelected &= ~GM( NI );
                }
            }
        }
    }
	ImNSL[NI] = RNSel;
}

static void UnSelectBrig( byte NI, word ID )
{
	int RNSel = 0;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* SNM = SerN[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->BrigadeID != ID )
				{
					SMon[RNSel] = SMon[i];
					SNM[RNSel] = SNM[i];
					RNSel++;
				}
				else
				{
					OB->Selected &= ~GM( NI );
                }
            }
        }
    }
	NSL[NI] = RNSel;
}

static void ImUnSelectBrig( byte NI, word ID )
{
	int RNSel = 0;
	word Nsel = ImNSL[NI];
	word* SMon = ImSelm[NI];
	word* SNM = ImSerN[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->BrigadeID != ID )
				{
					SMon[RNSel] = SMon[i];
					SNM[RNSel] = SNM[i];
					RNSel++;
				}
				else
				{
					OB->ImSelected &= ~GM( NI );
                }
            }
        }
    }
	ImNSL[NI] = RNSel;
}

static void ChooseSelectType( byte NI, word ID )
{
	int RNSel = 0;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* SNM = SerN[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->NIndex == ID && !OB->InArmy )
				{
					SMon[RNSel] = SMon[i];
					SNM[RNSel] = SNM[i];
					RNSel++;
				}
				else
				{
					OB->Selected &= ~GM( NI );
                }
            }
        }
    }
	NSL[NI] = RNSel;
	CorrectBrigadesSelection( NI );
}

static void ImChooseSelectType( byte NI, word ID )
{
	int RNSel = 0;
	word Nsel = ImNSL[NI];
	word* SMon = ImSelm[NI];
	word* SNM = ImSerN[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->NIndex == ID && !OB->InArmy )
				{
					SMon[RNSel] = SMon[i];
					SNM[RNSel] = SNM[i];
					RNSel++;
				}
				else
				{
					OB->ImSelected &= ~GM( NI );
                }
            }
        }
    }
	ImNSL[NI] = RNSel;
	ImCorrectBrigadesSelection( NI );
}

static void SelectBrig( byte NI, word ID )
{
	int RNSel = 0;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* SNM = SerN[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->BrigadeID == ID )
				{
					SMon[RNSel] = SMon[i];
					SNM[RNSel] = SNM[i];
					RNSel++;
				}
				else
				{
					OB->Selected &= ~GM( NI );
                }
            }
        }
    }
	NSL[NI] = RNSel;
}

static void ImSelectBrig( byte NI, word ID )
{
	int RNSel = 0;
	word Nsel = ImNSL[NI];
	word* SMon = ImSelm[NI];
	word* SNM = ImSerN[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->BrigadeID == ID )
				{
					SMon[RNSel] = SMon[i];
					SNM[RNSel] = SNM[i];
					RNSel++;
				}
				else
				{
					OB->ImSelected &= ~GM( NI );
                }
            }
        }
    }
	ImNSL[NI] = RNSel;
}

static void EraseBrigs( byte NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	Brigade* BR0 = NATIONS[NI].CITY->Brigs;
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && int( Group[MID] ) )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo&&OB->BrigadeID != 0xFFFF )
			{
				Brigade* BR = BR0 + OB->BrigadeID;
				if ( BR->Enabled )
				{
					SetAttState( BR, 0 );
					EraseBrigade( BR );
                }
            }
        }
    }
}

static bool CheckWorker( OneObject* OB )
{
	Order1* OR = OB->LocalOrder;
	while ( OR )
	{
		if ( OR->DoLink == &GoToMineLink )return false;
		OR = OR->NextOrder;
    }
	return true;
}

void GoToMineWithSelected( uint8_t NI, uint16_t ID )
{
	OneObject* OB = Group[ID];
	if ( !OB )return;
	NewMonster* NM = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	if ( !( NM->UnitAbsorber || NM->PeasantAbsorber ) )return;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	int Maxins = ADC->MaxInside + OB->AddInside - OB->NInside;
	if ( !( NATIONS[NI].NMask&OB->NMask ) )Maxins += 1000;
	word MID;
	if ( Maxins <= 0 )
	{
		if ( OB->NNUM == NatRefTBL[NI] )
		{
			for ( int i = 0; i < Nsel&&Maxins; i++ )
			{
				MID = SMon[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB )CmdDone[MID] = true;
                }
            }

        }
		return;
    }
	for ( int i = 0; i < Nsel&&Maxins; i++ )
	{
		MID = SMon[i];
		if ( !( MID == 0xFFFF || CmdDone[MID] ) )
		{
			OneObject* OB = Group[MID];
			if ( OB&&CheckWorker( OB ) )
			{
				if ( OB->GoToMine( ID, 16 ) )Maxins--;
            }

        }
    }
	if ( OB->NNUM == NatRefTBL[NI] )
	{
		for ( int i = 0; i < Nsel; i++ )
		{
			MID = SMon[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB )CmdDone[MID] = true;
            }
        }
    }
}

static void LeaveMineWithSelected( byte NI, word Type )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )OB->LeaveMine( Type );
        }
    }
}

static void GoToTransportWithSelected( byte NI, word ID )
{
	OneObject* OB = Group[ID];
	if ( !OB )return;
	NewMonster* NM = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	if ( !( NM->Transport ) )return;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	int Maxins = ADC->MaxInside + OB->AddInside - OB->NInside;
	word MID;
	if ( Maxins <= 0 )
	{
		if ( OB->NNUM == NatRefTBL[NI] )
		{
			for ( int i = 0; i < Nsel&&Maxins; i++ )
			{
				MID = SMon[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB )CmdDone[MID] = true;
                }
            }
        }
		return;
    }
	for ( int i = 0; i < Nsel&&Maxins; i++ )
	{
		MID = SMon[i];
		if ( !( MID == 0xFFFF || CmdDone[MID] ) )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->GoToTransport( ID, 16 ) )Maxins--;
            }

        }
    }
	if ( OB->NNUM == NatRefTBL[NI] )
	{
		for ( int i = 0; i < Nsel; i++ )
		{
			MID = SMon[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB )CmdDone[MID] = true;
            }
        }
    }
}

static void TakeRes( byte NI, int x, int y, byte ResID )
{
	int DObj = INITBEST;
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	word MID;
	for ( int i = 0; i < Nsel; i++ )
	{
		MID = SMon[i];
		if ( MID != 0xFFFF && !CmdDone[MID] )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				int DO = OB->TakeResource( x, y, ResID, 128, 0 );
				if ( DO == DObj&&DO != INITBEST )
				{
					Sprites[DO].Surrounded = true;
                }
				DObj = DO;
				if ( DO != INITBEST )CmdDone[MID] = true;
            }
        }
    }
	OneSprite* OS = Sprites;
	for ( int i = 0; i < MaxSprt; i++ )
	{
		OS->Surrounded = false;
		OS++;
    }
}

static void CreateGatesFromSelected( byte NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( CreateGates( OB ) )return;
            }
        }
    }
}

static void SelectAllBuildings( uint8_t NI )
{
	int N = NSL[NI];
	if ( N )
	{
		word* SMN = Selm[NI];
		word* SRN = SerN[NI];
		for ( int i = 0; i < N; i++ )
		{
			word MID = SMN[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->Serial == SRN[i] )OB->Selected &= ~GM( NI );
            }
        }
    }
	if ( Selm[NI] )
	{
		free( Selm[NI] );
		Selm[NI] = nullptr;
    }
	if ( SerN[NI] )
	{
		free( SerN[NI] );
		SerN[NI] = nullptr;
    }
	NSL[NI] = 0;
	N = 0;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == NatRefTBL[NI] && OB->NewBuilding )N++;
    }
	Selm[NI] = new word[N];
	SerN[NI] = new word[N];
	NSL[NI] = N;
	N = 0;
	word* SMN = Selm[NI];
	word* SRN = SerN[NI];
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == NatRefTBL[NI] && OB->NewBuilding )
		{
			SMN[N] = OB->Index;
			SRN[N] = OB->Serial;
			N++;
			OB->Selected |= GM( NI );
        }
    }
}

static void ImSelectAllBuildings( uint8_t NI )
{
	int N = ImNSL[NI];
	if ( N )
	{
		word* SMN = ImSelm[NI];
		word* SRN = ImSerN[NI];
		for ( int i = 0; i < N; i++ )
		{
			word MID = SMN[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->Serial == SRN[i] )OB->ImSelected &= ~GM( NI );
            }
        }
    }
	if ( ImSelm[NI] )
	{
		free( ImSelm[NI] );
		ImSelm[NI] = nullptr;
    }
	if ( ImSerN[NI] )
	{
		free( ImSerN[NI] );
		ImSerN[NI] = nullptr;
    }
	ImNSL[NI] = 0;
	N = 0;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == NatRefTBL[NI] && OB->NewBuilding && !OB->Sdoxlo )N++;
    }
	ImSelm[NI] = new word[N];
	ImSerN[NI] = new word[N];
	ImNSL[NI] = N;
	N = 0;
	word* SMN = ImSelm[NI];
	word* SRN = ImSerN[NI];
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == NatRefTBL[NI] && OB->NewBuilding && !OB->Sdoxlo )
		{
			SMN[N] = OB->Index;
			SRN[N] = OB->Serial;
			N++;
			OB->ImSelected |= GM( NI );
        }
    }
}

static void SelectAllUnits( byte NI, bool Re )
{
	word* lpN;
	word** UNI;
	word** USN;
	if ( Re )
	{
		lpN = NSL + NI;
		UNI = Selm + NI;
		USN = SerN + NI;
	}
	else
	{
		lpN = ImNSL + NI;
		UNI = ImSelm + NI;
		USN = ImSerN + NI;
    }
	int N = *lpN;
	if ( N )
	{
		word* SMN = *UNI;
		word* SRN = *USN;
		for ( int i = 0; i < N; i++ )
		{
			word MID = SMN[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->Serial == SRN[i] )
				{
					if ( Re )OB->Selected &= ~GM( NI );
					else OB->ImSelected &= ~GM( NI );
                }
            }
        }
    }
	if ( UNI[0] )
	{
		free( UNI[0] );
		UNI[0] = nullptr;
    }
	if ( USN[0] )
	{
		free( USN[0] );
		USN[0] = nullptr;
    }
	lpN[0] = 0;
	N = 0;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == NatRefTBL[NI] && !( OB->LockType || OB->NewBuilding || OB->Sdoxlo || OB->Wall || OB->UnlimitedMotion || OB->Guard != 0xFFFF || OB->UnlimitedMotion ) )N++;
    }
	UNI[0] = new word[N];
	USN[0] = new word[N];
	lpN[0] = N;
	N = 0;
	word* SMN = UNI[0];
	word* SRN = USN[0];
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == NatRefTBL[NI] && !( OB->LockType || OB->NewBuilding || OB->Sdoxlo || OB->Wall || OB->UnlimitedMotion || OB->Guard != 0xFFFF || OB->UnlimitedMotion ) )
		{
			SMN[N] = OB->Index;
			SRN[N] = OB->Serial;
			if ( Re )OB->Selected |= GM( NI );
			else OB->ImSelected |= GM( NI );
			N++;
        }
    }
}

static void SelectAllShips( byte NI, bool Re )
{
	word* lpN;
	word** UNI;
	word** USN;
	if ( Re )
	{
		lpN = NSL + NI;
		UNI = Selm + NI;
		USN = SerN + NI;
	}
	else
	{
		lpN = ImNSL + NI;
		UNI = ImSelm + NI;
		USN = ImSerN + NI;
    }
	int N = *lpN;
	if ( N )
	{
		word* SMN = *UNI;
		word* SRN = *USN;
		for ( int i = 0; i < N; i++ )
		{
			word MID = SMN[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->Serial == SRN[i] )
				{
					if ( Re )OB->Selected &= ~GM( NI );
					else OB->ImSelected &= ~GM( NI );
                }
            }
        }
    }
	if ( UNI[0] )
	{
		free( UNI[0] );
		UNI[0] = nullptr;
    }
	if ( USN[0] )
	{
		free( USN[0] );
		USN[0] = nullptr;
    }
	lpN[0] = 0;
	N = 0;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == NatRefTBL[NI] && OB->LockType && !OB->Sdoxlo )
		{
			byte Usage = OB->newMons->Usage;
			if ( Usage != FisherID&&Usage != ParomID )N++;
        }
    }
	UNI[0] = new word[N];
	USN[0] = new word[N];
	lpN[0] = N;
	N = 0;
	word* SMN = UNI[0];
	word* SRN = USN[0];
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NNUM == NatRefTBL[NI] && OB->LockType && !OB->Sdoxlo )
		{
			byte Usage = OB->newMons->Usage;
			if ( Usage != FisherID&&Usage != ParomID )
			{
				SMN[N] = OB->Index;
				SRN[N] = OB->Serial;
				if ( Re )OB->Selected |= GM( NI );
				else OB->ImSelected |= GM( NI );
				N++;
            }
        }
    }
}

void ComOpenGates(uint8_t NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				OpenGates( OB );
            }
        }
    }
}

void ComCloseGates(uint8_t NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	if ( !Nsel )return;
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				CloseGates( OB );
            }
        }
    }
}

void MakeStandGround( Brigade* BR )
{
	if ( BR->LastOrderTime == tmtmt )return;
	int N = BR->NMemb;
	word* Memb = BR->Memb;
	word* MembSN = BR->MembSN;
	int addD = 0;
	int addS = 0;
	if ( BR->WarType )
	{
		OrderDescription* ODS = ElementaryOrders + BR->WarType - 1;
		addD = ODS->AddDamage1;
		addS = ODS->AddShield1;
    }
	BR->AddDamage = addD;
	BR->AddShield = addS;
	for ( int i = 0; i < N; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MembSN[i] )
			{
				OB->StandGround = true;
				OB->AddShield = addS;
				OB->AddDamage = addD;
            }
        }
    }
	BR->LastOrderTime = tmtmt;
}

static void MakeStandGroundTemp( Brigade* BR )
{
	if ( BR->BrigDelay > 1 )return;
	if ( BR->LastOrderTime == tmtmt )return;
	//if(BR->BOrder){
	//	BR->ClearBOrders();
    //}
	int N = BR->NMemb;
	word* Memb = BR->Memb;
	word* MembSN = BR->MembSN;
	int addD = 0;
	int addS = 0;
	if ( BR->WarType )
	{
		OrderDescription* ODS = ElementaryOrders + BR->WarType - 1;
		addD = ODS->AddDamage2;
		addS = ODS->AddShield2;
    }
	BR->AddDamage = addD;
	BR->AddShield = addS;
	for ( int i = 0; i < N; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MembSN[i] )
			{
				OB->StandGround = true;
				OB->AddShield = addS;
				OB->AddDamage = addD;
				if ( OB->LocalOrder && !OB->NewBuilding )OB->ClearOrders();
            }
        }
    }
	BR->LastOrderTime = tmtmt;
	BR->BrigDelay = 150;
}

void SetAttState( Brigade* BR, bool Val )
{
	if ( BR->LastOrderTime == tmtmt )return;
	int N = BR->NMemb;
	word* Memb = BR->Memb;
	word* MembSN = BR->MembSN;
	for ( int i = 0; i < N; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MembSN[i] )
			{

				if ( !OB->newMons->Priest )OB->NoSearchVictim = Val;

				if ( Val )
				{
					if ( OB->EnemyID != 0xFFFF )OB->ClearOrders();
                }
            }
        }
    }
	BR->LastOrderTime = tmtmt;
}

void SetSearchVictim(uint8_t NI, uint8_t Val )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	Brigade* BR0 = NATIONS[NI].CITY->Brigs;
	if ( !Nsel )return;
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->BrigadeID != 0xFFFF )
				{
					Brigade* BR = BR0 + OB->BrigadeID;
					SetAttState( BR, Val != 0 );
				}
				else
				{
					if ( Val & 2 )
					{

						OB->NoSearchVictim = Val & 1;

						if ( OB->EnemyID != 0xFFFF )OB->ClearOrders();
					}
					else
					{
						if ( !OB->newMons->Priest )
						{

							OB->NoSearchVictim = Val & 1;

							if ( OB->EnemyID != 0xFFFF )OB->ClearOrders();
							if ( OB->LocalOrder&&OB->LocalOrder->DoLink == &NewAttackPointLink )
								OB->ClearOrders();
                        }
                    }
                }
            }
        }
    }
}

void MakeStandGround( uint8_t NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	Brigade* BR0 = NATIONS[NatRefTBL[NI]].CITY->Brigs;
	if ( !Nsel )return;
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->BrigadeID != 0xFFFF )
				{
					Brigade* BR = BR0 + OB->BrigadeID;
					MakeStandGroundTemp( BR );
				}
				else
				{
					OB->StandGround = true;
                }
            }
        }
    }
}

void CancelStandGround( Brigade* BR )
{
	if ( BR->LastOrderTime == tmtmt )return;
	int N = BR->NMemb;
	word* Memb = BR->Memb;
	word* MembSN = BR->MembSN;
	int addD = 0;
	int addS = 0;
	if ( BR->WarType )
	{
		OrderDescription* ODS = ElementaryOrders + BR->WarType - 1;
		addD = ODS->AddDamage2;
		addS = ODS->AddShield2;
    }
	BR->AddDamage = addD;
	BR->AddShield = addS;
	for ( int i = 0; i < N; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MembSN[i] )
			{
				OB->StandGround = false;
				OB->AddShield = addS;
				OB->AddDamage = addD;
            }
        }
    }
	BR->LastOrderTime = tmtmt;
	BR->BrigDelay = 0;
}

void CancelStandGroundAnyway( Brigade* BR )
{
	int N = BR->NMemb;
	word* Memb = BR->Memb;
	word* MembSN = BR->MembSN;
	int addD = 0;
	int addS = 0;
	if ( BR->WarType )
	{
		OrderDescription* ODS = ElementaryOrders + BR->WarType - 1;
		addD = ODS->AddDamage2;
		addS = ODS->AddShield2;
    }
	BR->AddDamage = addD;
	BR->AddShield = addS;
	BR->BrigDelay = 0;
	for ( int i = 0; i < N; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MembSN[i] )
			{
				OB->StandGround = false;
				OB->AddShield = addS;
				OB->AddDamage = addD;
            }
        }
    }
}

static void CreateFieldsBar( byte NI, word n )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->newMons->Usage == MelnicaID )CreateFields( NI, OB->RealX, OB->RealY, n );
        }
    }
}

void CancelStandGround(uint8_t NI )
{
	word Nsel = NSL[NI];
	word* SMon = Selm[NI];
	Brigade* BR0 = NATIONS[NI].CITY->Brigs;
	if ( !Nsel )return;
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->BrigadeID != 0xFFFF )
				{
					Brigade* BR = BR0 + OB->BrigadeID;
					CancelStandGround( BR );
				}
				else
				{
					OB->StandGround = false;
                }
            }
        }
    }
}

void SelBrigade( uint8_t NI, uint8_t Type, int ID )
{
	if ( !Type )
	{
		if ( NSL[NI] )
		{
			int N = NSL[NI];
			word* SEL = Selm[NI];
			word* SSN = SerN[NI];
			for ( int j = 0; j < N; j++ )
			{
				word MID = SEL[j];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB&&OB->Serial == SSN[j] )
					{
						OB->Selected &= ~GM( NI );
                    }
                }
            }
			free( Selm[NI] );
			free( SerN[NI] );
			Selm[NI] = nullptr;
			SerN[NI] = nullptr;
			NSL[NI] = 0;
        }
    }
	Brigade* BR = NATIONS[NatRefTBL[NI]].CITY->Brigs + ID;
	if ( BR->Enabled )
	{
		int NAdd = 0;
		for ( int i = 2; i < BR->NMemb; i++ )
		{
			word MID = BR->Memb[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && !( OB->Selected&GM( NI ) ) )
				{
					NAdd++;
                }
            }
        }
		if ( NAdd )
		{
			int N = NSL[NI];
			Selm[NI] = (word*) realloc( Selm[NI], ( N + NAdd ) * 2 );
			SerN[NI] = (word*) realloc( SerN[NI], ( N + NAdd ) * 2 );
			for ( int i = 2; i < BR->NMemb; i++ )
			{
				word MID = BR->Memb[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && !( OB->Selected&GM( NI ) ) )
					{
						int N = NSL[NI];
						Selm[NI][N] = OB->Index;
						SerN[NI][N] = OB->Serial;
						NSL[NI]++;
						OB->Selected |= GM( NI );
                    }
                }
            }
        }
    }
}

void ImSelBrigade( uint8_t NI, uint8_t Type, int ID )
{
	if ( !Type )
	{
		if ( ImNSL[NI] )
		{
			int N = ImNSL[NI];
			word* SEL = ImSelm[NI];
			word* SSN = ImSerN[NI];
			for ( int j = 0; j < N; j++ )
			{
				word MID = SEL[j];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB&&OB->Serial == SSN[j] )
					{
						OB->ImSelected &= ~GM( NI );
                    }
                }
            }
			free( ImSelm[NI] );
			free( ImSerN[NI] );
			ImSelm[NI] = nullptr;
			ImSerN[NI] = nullptr;
			ImNSL[NI] = 0;
        }
    }
    Brigade* BR = NATIONS[NatRefTBL[NI]].CITY->Brigs + ID;
	if ( BR->Enabled )
	{
		int NAdd = 0;
		for ( int i = 2; i < BR->NMemb; i++ )
		{
			word MID = BR->Memb[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && !( OB->ImSelected&GM( NI ) ) )
				{
					NAdd++;
                }
            }
        }
		if ( NAdd )
		{
			int N = ImNSL[NI];
			ImSelm[NI] = (word*) realloc( ImSelm[NI], ( N + NAdd ) * 2 );
			ImSerN[NI] = (word*) realloc( ImSerN[NI], ( N + NAdd ) * 2 );
			for ( int i = 2; i < BR->NMemb; i++ )
			{
				word MID = BR->Memb[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && !( OB->ImSelected&GM( NI ) ) )
					{
						int N = ImNSL[NI];
						ImSelm[NI][N] = OB->Index;
						ImSerN[NI][N] = OB->Serial;
						ImNSL[NI]++;
						OB->ImSelected |= GM( NI );
                    }
                }
            }
        }
    }
}

static void CreateBrOrder( byte NI, int i )
{
	DWORD p = i;

	word UID = word( p & 8191 );
	word ort = word( ( p >> 13 ) & 255 );
	word Type = word( p >> 21 );

	//detecting officer
	for ( int q = 0; q < 8; q++ )
	{
		OneObject* OOB = Group[UID];
		if ( OOB&&OOB->Ref.General->OFCR&&OOB->BrigadeID == 0xFFFF )
		{
			q = 8;
		}
		else
		{
			UID += 8192;
        }
    }
	OneObject* OB = Group[UID];
	if ( NSL[NI] )
	{
		int N = NSL[NI];
		for ( int j = 1; j < N; j++ )
		{
			word MID = Selm[NI][j];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB )OB->Selected &= ~GM( NI );
            }
        }
		NSL[NI] = 1;
    }
	if ( OB && ( !OB->Sdoxlo ) && OB->BrigadeID == 0xFFFF )
	{
		int bind = CITY[NatRefTBL[NI]].GetFreeBrigade();
		Brigade* BR = CITY[NatRefTBL[NI]].Brigs + bind;
		BR->Enabled = true;
		if ( !BR->CreateNearOfficer( Group[UID], Type, ort ) )
		{
			BR->Enabled = 0;
		}
		else
		{
			CorrectBrigadesSelection( NI );
			ImCorrectBrigadesSelection( NI );
        }
    }
}

//Stops running upgrade in any building in game
//Made to solve 'captured / destroyed while upgrading' bugs
//OneObject *OB = pointer to instance of the building
void StopUpgradeInBuilding( OneObject *OB )
{
	//Get player's nation from his building
	byte NationID = OB->Nat->NNUM;

	if ( OB //Instance exists
		&& ( !OB->Ready ) //Upgrade still in progress
		&& OB->NUstages //Has progress bar (?)
		&& OB->LocalOrder //Has an assigned order for the upgrade
		&& OB->LocalOrder->DoLink == &PerformUpgradeLink ) //Upgrade is linked to callback
	{
		//Get this upgrade from global upgrade array
		NewUpgrade* NU = NATIONS[NationID].UPGRADE[OB->LocalOrder->info.PUpgrade.NewUpgrade];

		for ( int j = 0; j < 8; j++ )
		{//Return spent resources to the owner of the building
			AddXRESRC( NationID, j, NU->Cost[j] );
		}

		OB->DeleteLastOrder();
		OB->Ready = true;
		OB->NUstages = 0;
		OB->Ustage = 0;

		if ( !( NU->Individual || NU->StageUp ) )
		{
			NU->Enabled = true;
			NU->PermanentEnabled = true;
			NU->IsDoing = false;
			NU->CurStage = 0;
		}
	}
}

//Stops running upgrade in the selected unit / building
//byte Nat = Player who canceled his upgrade
static void StopUpg( byte Nat )
{
	//Get national ID of the player
	byte NAT1 = NatRefTBL[Nat];

	//Get amount of selected units
	int N = NSL[Nat];

	//Get MosterIDs of selected units
	word* Uni = Selm[Nat];

	//Get Serials (?) of selected units
	word* USN = SerN[Nat];

	//Iterate through selected units
	for ( int i = 0; i < N; i++ )
	{
		//Get MonsterID
		word MID = Uni[i];

		if ( MID != 0xFFFF )
		{//Sanity check ok

			//Get OneObject instance from MonsterID
			OneObject* OB = Group[MID];

			if ( OB //Instance exists
				&& OB->Serial == USN[i] //Equals the unit we are looking for
				&& ( !OB->Ready ) //Upgrade still in progress
				&& OB->NUstages //Has progress bar (?)
				&& OB->LocalOrder //Has an assigned order for the upgrade
				&& OB->LocalOrder->DoLink == &PerformUpgradeLink ) //Upgrade is linked to callback
			{
				//Get this upgrade from global upgrade array
				NewUpgrade* NU = NATIONS[NAT1].UPGRADE[OB->LocalOrder->info.PUpgrade.NewUpgrade];

				for ( int j = 0; j < 8; j++ )
				{//Return spent resources to the player
					AddXRESRC( NAT1, j, NU->Cost[j] );
				}

				OB->DeleteLastOrder();
				OB->Ready = true;
				OB->NUstages = 0;
				OB->Ustage = 0;

				if ( !( NU->Individual || NU->StageUp ) )
				{
					NU->Enabled = true;
					NU->PermanentEnabled = true;
					NU->IsDoing = false;
					NU->CurStage = 0;
				}
			}
		}
	}
}

static void SelUnloadAll( byte Nat )
{
	int N = NSL[Nat];
	word* Uni = Selm[Nat];
	word* USN = SerN[Nat];
	for ( int i = 0; i < N; i++ )
	{
		word MID = Uni[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == USN[i] && OB->NInside&&OB->Transport&&OB->DstX > 0 )
			{
				LeaveAll( OB );
            }
        }
    }
}

static void MakeReformation( byte NI, word BrigadeID, byte FormType )
{
	Brigade* BR = CITY[NatRefTBL[NI]].Brigs + BrigadeID;
	if ( BR->Enabled&&BR->WarType )
	{
		int RealNM = 0;
		for ( int i = 2; i < BR->NMemb; i++ )
		{
			word MID = BR->Memb[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && ( !OB->Sdoxlo ) && OB->Serial == BR->MembSN[i] )
				{
					RealNM++;
                }
            }
        }
		//search for appropriate formation
		SingleGroup* SG = FormGrp.Grp + FormType;
		int j;
		for ( j = 0; j<SG->NForms&&RealNM>ElementaryOrders[SG->Forms[j]].NUnits; j++ );
		if ( j < SG->NForms )
		{
			int WT = SG->Forms[j] + 1;
			OrderDescription* ODE = ElementaryOrders + SG->Forms[j];
			word* TMP = new word[ODE->NUnits];
			memset( TMP, 0xFF, ODE->NUnits * 2 );
			RealNM = 0;
			int xc = 0;
			int yc = 0;
			for ( int q = 2; q < BR->NMemb; q++ )
			{
				word MID = BR->Memb[q];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && ( !OB->Sdoxlo ) && OB->Serial == BR->MembSN[q] )
					{
						TMP[RealNM] = MID;
						RealNM++;
						xc += OB->RealX;
						yc += OB->RealY;
                    }
                }
            }
			if ( RealNM == 0 )return;
			if ( BR->PosCreated )
			{
				BR->PosCreated = false;
				free( BR->posX );
				free( BR->posY );
				BR->posX = nullptr;
				BR->posY = nullptr;
            }
			if ( BR->MaxMemb < ODE->NUnits + 2 )
			{
				BR->MaxMemb = ODE->NUnits + 2;
				BR->Memb = (word*) realloc( BR->Memb, BR->MaxMemb * 2 );
				BR->MembSN = (word*) realloc( BR->MembSN, BR->MaxMemb * 2 );
            }
			memcpy( BR->Memb + 2, TMP, ODE->NUnits * 2 );
			for ( int q = 2; q < RealNM; q++ )BR->MembSN[q] = Group[BR->Memb[q]]->Serial;
			BR->WarType = WT;
			BR->CreateOrderedPositions( xc / RealNM, yc / RealNM, BR->Direction );
			BR->KeepPositions( 0, 128 + 16 );
        }
    }
}

static void SelectIdlePeasants( byte NI )
{
	ReClearSelection( NI );
	int N = 0;
	word* Units = NatList[NatRefTBL[NI]];
	int Nu = NtNUnits[NatRefTBL[NI]];
	for ( int i = 0; i < Nu; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB && ( !OB->Sdoxlo ) )
		{
			NewMonster* NM = OB->newMons;
			if ( NM->Usage == PeasantID && !OB->LocalOrder )N++;
        }
    }
	if ( N )
	{
		Selm[NI] = new word[N];
		SerN[NI] = new word[N];
		NSL[NI] = N;
		N = 0;
		for ( int i = 0; i < Nu; i++ )
		{
			OneObject* OB = Group[Units[i]];
			if ( OB && ( !OB->Sdoxlo ) )
			{
				NewMonster* NM = OB->newMons;
				if ( NM->Usage == PeasantID && !OB->LocalOrder )
				{
					Selm[NI][N] = OB->Index;
					SerN[NI][N] = OB->Serial;
					OB->Selected &= ~GM( NI );
					N++;
                }
            }
        }
    }
}

static void ImSelectIdlePeasants( uint8_t NI )
{
	ImClearSelection( NI );
	int N = 0;
	word* Units = NatList[NatRefTBL[NI]];
	int Nu = NtNUnits[NatRefTBL[NI]];
	for ( int i = 0; i < Nu; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB && ( !OB->Sdoxlo ) )
		{
			NewMonster* NM = OB->newMons;
			if ( NM->Usage == PeasantID && !OB->LocalOrder )N++;
        }
    }
	if ( N )
	{
		ImSelm[NI] = new word[N];
		ImSerN[NI] = new word[N];
		ImNSL[NI] = N;
		N = 0;
		for ( int i = 0; i < Nu; i++ )
		{
			OneObject* OB = Group[Units[i]];
			if ( OB && ( !OB->Sdoxlo ) )
			{
				NewMonster* NM = OB->newMons;
				if ( NM->Usage == PeasantID && !OB->LocalOrder )
				{
					ImSelm[NI][N] = OB->Index;
					ImSerN[NI][N] = OB->Serial;
					OB->ImSelected |= GM( NI );
					N++;
                }
            }
        }
    }
}

static void SelectIdleMines( byte NI )
{
	ReClearSelection( NI );
	int N = 0;
	word* Units = NatList[NatRefTBL[NI]];
	int Nu = NtNUnits[NatRefTBL[NI]];
	for ( int i = 0; i < Nu; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB && ( !OB->Sdoxlo ) )
		{
			NewMonster* NM = OB->newMons;
			if ( NM->Usage == MineID&&OB->Ready&&OB->NInside < OB->AddInside + OB->Ref.General->MoreCharacter->MaxInside )N++;
        }
    }
	if ( N )
	{
		Selm[NI] = new word[N];
		SerN[NI] = new word[N];
		NSL[NI] = N;
		N = 0;
		for ( int i = 0; i < Nu; i++ )
		{
			OneObject* OB = Group[Units[i]];
			if ( OB && ( !OB->Sdoxlo ) )
			{
				NewMonster* NM = OB->newMons;
				if ( NM->Usage == MineID&&OB->Ready&&OB->NInside < OB->AddInside + OB->Ref.General->MoreCharacter->MaxInside )
				{
					Selm[NI][N] = OB->Index;
					SerN[NI][N] = OB->Serial;
					OB->Selected |= GM( NI );
					N++;
                }
            }
        }
    }
}

static void ImSelectIdleMines( uint8_t NI )
{
	ImClearSelection( NI );
	int N = 0;
	word* Units = NatList[NatRefTBL[NI]];
	int Nu = NtNUnits[NatRefTBL[NI]];
	for ( int i = 0; i < Nu; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB && ( !OB->Sdoxlo ) )
		{
			NewMonster* NM = OB->newMons;
			if ( NM->Usage == MineID&&OB->Ready&&OB->NInside < OB->AddInside + OB->Ref.General->MoreCharacter->MaxInside )N++;
        }
    }
	if ( N )
	{
		ImSelm[NI] = new word[N];
		ImSerN[NI] = new word[N];
		ImNSL[NI] = N;
		N = 0;
		for ( int i = 0; i < Nu; i++ )
		{
			OneObject* OB = Group[Units[i]];
			if ( OB && ( !OB->Sdoxlo ) )
			{
				NewMonster* NM = OB->newMons;
				if ( NM->Usage == MineID&&OB->Ready&&OB->NInside < OB->AddInside + OB->Ref.General->MoreCharacter->MaxInside )
				{
					ImSelm[NI][N] = OB->Index;
					ImSerN[NI][N] = OB->Serial;
					OB->ImSelected |= GM( NI );
					N++;
                }
            }
        }
	}
	else
	{
        // BoonXRay 05.11.2017
        //PlayEffect( NoMineSound, 0, 0 );
        PlayEffect( -1, 0, 0 );
    }
}

static void FreeSelected( byte NI )
{
	int Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* ser = SerN[NI];
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->NNUM == NatRefTBL[NI] && ser[i] == OB->Serial )OB->Selected &= ~GM( NI );
        }
    }
	if ( Selm[NI] )
	{
		free( Selm[NI] );
		Selm[NI] = nullptr;
    }
	if ( SerN[NI] )
	{
		free( SerN[NI] );
		SerN[NI] = nullptr;
    }
	NSL[NI] = 0;
}

static int _implSelBrigade( byte* Ptr );
static int _implUnSelBrigade( byte* Ptr );
static int _implUnSelUnitsSet( byte* Ptr );
static int _implSelUnitsSet( byte* Ptr );
static int _implSelUnitsSet_OLD( byte* Ptr );

static int PREVSEUSETPOS = -1;

//Process orders from execution buffer
void ExecuteBuffer()
{
	if ( PlayGameMode )
	{//A recording is beeing played
		if ( !RGAME.Extract() )
		{
			GameExit = true;
			ShowStat = 0;
			EBPos = 0;
		}
	}
	else
	{
		if ( RecordMode )
		{
			RGAME.AddRecord();
		}
	}

	MaxDCT = 0;
	SlowMade = 0;
	InitSelection();
	PrevProdPos = -1;
	PREVSEUSETPOS = -1;
	PrevProdUnit = 0;

	for ( int p = 0; p < 8; p++ )
	{
		RefreshSelected( p );
	}

	int pos = 0;
	int len;
	byte tp;
	CURMAXP = -1;

	char sss[128];
	while ( pos < EBPos )
	{
		byte cmd = ExBuf[pos];
		pos++;
		switch ( cmd )
		{
		case 1://Create selection
			pos += 5;
			break;

		case 2://Send group to (X,Y)
			tp = ExBuf[pos + 11];
			if ( ( tp & 15 ) < 8 )SendSelectedToXY( ExBuf[pos], *(int*) ( &ExBuf[pos + 1] ), *(int*) ( &ExBuf[pos + 5] ), *(short*) ( &ExBuf[pos + 9] ), tp );
			else SendSelectedToXY( ExBuf[pos], *(int*) ( &ExBuf[pos + 1] ), *(int*) ( &ExBuf[pos + 5] ), *(short*) ( &ExBuf[pos + 9] ), 128, tp );
			pos += 12;
			break;

		case 3://Attack object with group
			AttackSelected( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ), ExBuf[pos + 3], *(short*) ( &ExBuf[pos + 4] ) );
			pos += 6;
			break;

		case 4://Create terrain object
			CreateTerrainMons( ExBuf[pos], ExBuf[pos + 1], ExBuf[pos + 2] );
			pos += 5;
			break;

		case 5://create building
			CreateBuilding( ExBuf[pos], *(int*) ( &ExBuf[pos + 1] ), *(int*) ( &ExBuf[pos + 5] ),
				*(word*) ( &ExBuf[pos + 9] ), ExBuf[pos + 11] );
			pos += 12;
			break;

		case 6://produce object
			ProduceObject( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 7://member selection
			MemSelection( ExBuf[pos], ExBuf[pos + 1] );
			pos += 2;
			break;

		case 8://rememhber selection
			RememSelection( ExBuf[pos], ExBuf[pos + 1] );
			pos += 2;
			break;

		case 9://build or repair object
			BuildWithSelected( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ), ExBuf[pos + 3] );
			pos += 4;
			break;

		case 11:
			RepairWall( ExBuf[pos], *(short*) ( &ExBuf[pos + 1] ), *(short*) ( &ExBuf[pos + 3] ), ExBuf[pos + 5] );
			pos += 6;
			break;

		case 12:
			pos += 3;
			break;

		case 13:
			TakeRes( ExBuf[pos], *(int*) ( &ExBuf[pos + 1] ), *(int*) ( &ExBuf[pos + 5] ), ExBuf[pos + 9] );
			pos += 10;
			break;

		case 14:
			PerformUpgr( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 15:
			pos += 3;
			break;

		case 16://Create kind selection
			pos += 6;
			break;

		case 17://Create type selection
			pos += 7;
			break;

		case 21://set destination
			SetDestination( ExBuf[pos], *(int*) ( &ExBuf[pos + 1] ), *(int*) ( &ExBuf[pos + 5] ) );
			pos += 9;
			break;

		case 22://send to point
			pos += 3;
			break;

		case 23://Send group to (X,Y)
			AttackToXY( ExBuf[pos], ExBuf[pos + 1], ExBuf[pos + 2] );
			pos += 3;
			break;

		case 24://STOP
			Stopp( ExBuf[pos] );
			pos++;
			break;

		case 25://STAND GROUND
			StandGround( ExBuf[pos] );
			pos++;
			break;

		case 26://Patrol
			PatrolGroup( ExBuf[pos], *( (int*) ( ExBuf + pos + 1 ) ), *( (int*) ( ExBuf + pos + 5 ) ), 0 );
			pos += 9;
			break;

		case 28://
			break;

		case 29://Complex attack point
			GroupAttackPoint( ExBuf[pos], ExBuf[pos + 1], ExBuf[pos + 2], ExBuf[pos + 3] );
			pos += 4;
			break;

		case 30://Complex attack object
			pos += 4;
			break;

		case 31://GoToTransport
			pos += 3;
			break;

		case 32://Unload Transport
			pos += 3;
			break;

		case 33://Kill selected units / buildings (DEL key), set through CmdDie( byte NI )
			DieSelected( ExBuf[pos] );
			pos++;
			break;

		case 34://attack point
			ContinueAttPoint( ExBuf[pos], ExBuf[pos + 1], ExBuf[pos + 2] );
			pos += 3;
			break;

		case 35://attack point
			ContinueAttWall( ExBuf[pos], ExBuf[pos + 1], ExBuf[pos + 2] );
			pos += 3;
			break;

		case 36://sit down
			SitDown( ExBuf[pos] );
			pos++;
			break;

		case 37:
			pos += 3;
			break;

		case 38://produce object
			UnProduceObject( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 39:
			pos += 2;
			break;

		case 40://save network game
			len = ExBuf[pos + 5];
			memcpy( sss, &ExBuf[pos + 6], len );
			sss[len] = 0;
			SaveNetworkGame( *(int*) ( &ExBuf[pos + 1] ), sss );
			pos += 6 + len;
			break;

		case 41://load network game
			len = ExBuf[pos + 5];
			memcpy( sss, &ExBuf[pos + 6], len );
			sss[len] = 0;
			LoadNetworkGame( sss );
			StartTmtmt = tmtmt;
			tmtmt = 0;
			REALTIME = 0;
			rpos = 0;
			pos += len + 6;
			break;

		case 42://select type
			ChooseSelectType( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 43://select type
			ChooseUnSelectType( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 44:
			CreateNewTerrMons( ExBuf[pos], *(int*) ( &ExBuf[pos + 1] ), *(int*) ( &ExBuf[pos + 5] ), *(word*) ( &ExBuf[pos + 9] ) );
			pos += 11;
			break;

		case 45:
			GoToMineWithSelected( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 46:
			LeaveMineWithSelected( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 47:
			pos += CreateWall( ExBuf[pos + 2], &ExBuf[pos] );
			break;

		case 48:
			GoToTransportWithSelected( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 49:
			CreateGatesFromSelected( ExBuf[pos] );
			pos++;
			break;

		case 50:
			ComOpenGates( ExBuf[pos] );
			pos++;
			break;

		case 51:
			ComCloseGates( ExBuf[pos] );
			pos++;
			break;

		case 52:
			SelectAllBuildings( ExBuf[pos] );
			pos++;
			break;

		case 53:
			SelectBrig( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 54:
			UnSelectBrig( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 55:
			EraseBrigs( ExBuf[pos] );
			pos++;
			break;

		case 56:
			MakeStandGround( ExBuf[pos] );
			pos++;
			break;

		case 57:
			CancelStandGround( ExBuf[pos] );
			pos++;
			break;

		case 58:
			CreateBrOrder( ExBuf[pos], *(int*) ( &ExBuf[pos + 1] ) );
			pos += 5;
			break;

		case 59:
			SelBrigade( ExBuf[pos], ExBuf[pos + 1], *(word*) ( &ExBuf[pos + 2] ) );
			pos += 4;
			break;

		case 60://Market resource exchange order
			PerformTorg( ExBuf[pos], ExBuf[pos + 1], ExBuf[pos + 2], *(int*) ( &ExBuf[pos + 3] ) );
			pos += 7;
			break;

		case 61:
			CreateFieldsBar( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ) );
			pos += 3;
			break;

		case 62:
			SetSearchVictim( ExBuf[pos], ExBuf[pos + 1] );
			pos += 2;
			break;

		case 63:
			StopUpg( ExBuf[pos] );
			pos++;
			break;

		case 64:
			SelUnloadAll( ExBuf[pos] );
			pos++;
			break;

		case 65:
			MakeReformation( ExBuf[pos], *(word*) ( &ExBuf[pos + 1] ), ExBuf[pos + 3] );
			pos += 4;
			break;

		case 66:
			SelectIdlePeasants( ExBuf[pos] );
			pos++;
			break;

		case 67:
			SelectIdleMines( ExBuf[pos] );
			pos++;
			break;

		/*
		case 68://Toggle speed (Ctrl+D)
			//ChGSpeed();//BUGFIX: real time speed changing
			pos++;
			break;
		*/

		case 69://Change speed to specified value
			SetGSpeed( ExBuf[pos] );
			pos++;
			break;

		case 70:
			ChangePauseState();
			HISPEED = 0;
			pos++;
			break;

		case 71:
			FreeSelected( ExBuf[pos] );
			pos++;
			break;

		case 72:
			SelectAllUnits( ExBuf[pos], 1 );
			pos++;
			break;

		case 73:
			SelectAllShips( ExBuf[pos], 1 );
			pos++;
			break;

		case 74:
			__EndGame( ExBuf[pos], ExBuf[pos + 1] );
			HISPEED = 0;
			pos += 2;
			break;

		case 75:
			DoCmdMoney( ExBuf[pos] );
			pos++;
			break;

		case 76:
			DoCmdGiveMoney( ExBuf[pos], ExBuf[pos + 1], ExBuf[pos + 2], *( (int*) ( ExBuf + pos + 3 ) ) );
			pos += 7;
			break;

		case 77:
			__DoStartTime( (int*) ( ExBuf + pos + 1 ) );
			pos += 33;
			break;

		case 78:
			ProduceObjects( ExBuf[pos], ExBuf[pos + 1], *(word*) ( &ExBuf[pos + 2] ) );
			pos += 4;
			break;

		case 79:
			SETPING( *( (word*) ( ExBuf + pos + 1 ) ) );
			pos += 3;
			break;

		case 80:
			DoItSlow( *( (word*) ( ExBuf + pos ) ) );
			pos += 2;
			break;

		case 81:
			DoTimeStep( *( (int*) ( ExBuf + pos ) ) );
			pos += 4;
			break;

		case 82:
			DoFillFormation( ExBuf[pos], *( (word*) ( ExBuf + pos + 1 ) ) );
			pos += 3;
			break;

		case 83:
			ComOfferVoting( *( (DWORD*) ( ExBuf + pos + 1 ) ) );
			pos += 5;
			break;

		case 84:
			ComDoVote( *( (DWORD*) ( ExBuf + pos + 1 ) ), ExBuf[pos + 5] );
			pos += 6;
			break;

		case 85:
			ComEndPT();
			pos++;
			break;

		case 86:
			ComSetGuardState( ExBuf[pos], *( (word*) ( ExBuf + pos + 1 ) ) );
			pos += 3;
			break;

		case 87:
			ComAttackGround( ExBuf[pos], *( (int*) ( ExBuf + pos + 1 ) ), *( (int*) ( ExBuf + pos + 5 ) ) );
			pos += 9;
			break;

		case 88:
			GroupSelectedFormations( ExBuf[pos] );
			pos++;
			break;

		case 89:
			UnGroupSelectedUnits( ExBuf[pos] );
			pos++;
			break;

		case 90:
			ChangeNatRefTBL( ExBuf[pos], ExBuf + pos + 1 );
			pos += 9;
			break;

		case 91:
			ComChangePeaceTimeStage( *( (word*) ( ExBuf + pos ) ) );
			pos += 2;
			break;

		case 92:
			__EndGame( ExBuf[pos], ExBuf[pos + 1] );
			HISPEED = 0;
			pos += 3;
			break;

		case 96:
			DoCmdAddMoney( ExBuf[pos], *( (int*) ( ExBuf + pos + 1 ) ) );
			pos += 5;
			break;

		case 200:
			pos += _implSelBrigade( ExBuf + pos );
			break;

		case 201:
			pos += _implUnSelBrigade( ExBuf + pos );
			break;

		case 202:
			pos += _implUnSelUnitsSet( ExBuf + pos );
			break;

		case 203:
			pos += _implSelUnitsSet_OLD( ExBuf + pos );
			break;

		case 204:
			pos += _implSelUnitsSet( ExBuf + pos );
			break;
		}
	}

	InitEBuf();

	memset( NPresence, 0, VAL_MAXCX*VAL_MAXCX );

	int ofs;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB )
		{
			ofs = ( ( OB->RealY >> 11 ) << VAL_SHFCX ) + ( OB->RealX >> 11 );
			if ( ofs >= 0 && ofs < VAL_MAXCIOFS )
			{
				NPresence[ofs] |= OB->NMask;
			}
		}
	}

	NeedCurrentTime += MaxDCT;

	for ( int i = 0; i < MaxDCT; i++ )
	{
		rando();
	}

    if ( PlayGameMode == 1 )
	{
		for ( int i = 0; i < MAXOBJECT; i++ )
		{
			OneObject* OB = Group[i];
			if ( OB )OB->ImSelected = 0;
        }

        // BoonXRay 10.09.2017 Always true
        //if ( ImSelm )
		{
			ImNSL[MyNation] = 0;
			free( ImSelm[MyNation] );
			free( ImSerN[MyNation] );
			ImSelm[MyNation] = nullptr;
			ImSerN[MyNation] = nullptr;
		}
	}
}

static int _implSelBrigade( byte* Ptr )
{
	byte NI = Ptr[0];
	word ID = *( (word*) ( Ptr + 1 ) );
	Brigade* BR = CITY[NI].Brigs + ID;
	word* MEM = BR->Memb;
	int N = BR->NMemb;
	int NAdd = 0;
	for ( int i = 2; i < N; i++ )
	{
		word MID = MEM[i];
		if ( MID != 0xFFFF && Group[MID] )NAdd++;
    }
	Selm[NI] = (word*) realloc( Selm[NI], ( NAdd + NSL[NI] ) << 1 );
	SerN[NI] = (word*) realloc( Selm[NI], ( NAdd + NSL[NI] ) << 1 );
	int NS = NSL[NI];
	for ( int i = 2; i < N; i++ )
	{
		word MID = MEM[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				Selm[NI][NS] = MID;
				SerN[NI][NS] = OB->Serial;
				NS++;
            }
        }
    }
	NSL[NI] = NS;
	return 3;
}

static int _implUnSelBrigade( byte* Ptr )
{
	byte NI = Ptr[0];
	word ID = *( (word*) ( Ptr + 1 ) );
	word* SU = Selm[NI];
	word* SN = SerN[NI];
	int N = NSL[NI];
	int N1 = 0;
	for ( int i = 0; i < N; i++ )
	{
		word MID = SU[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( !( OB&&OB->BrigadeID == ID&&OB->NNUM == NatRefTBL[NI] ) )
			{
				SU[N1] = MID;
				SN[N1] = SN[i];
				N1++;
            }
        }
    }
	NSL[NI] = N1;
	if ( N && !N1 )
	{
		free( Selm[NI] );
		Selm[NI] = nullptr;
		free( SerN[NI] );
		SerN[NI] = nullptr;
    }
	Brigade* BR = CITY[NI].Brigs + ID;
	SU = BR->Memb;
	SN = BR->MembSN;
	N = BR->NMemb;

	return 3;
}

void CmdUnSelUnitsSet( uint8_t NI, uint16_t * BUF, int NU )
{
	ExBuf[EBPos] = 202;
	ExBuf[EBPos + 1] = NI;
	*( (word*) ( ExBuf + EBPos + 2 ) ) = word( NU );
	memcpy( ExBuf + EBPos + 4, BUF, NU << 1 );
	EBPos += 4 + NU * 2;

}

static int _implUnSelUnitsSet( byte* Ptr )
{
	byte NI = Ptr[0];
	word N = *( (word*) ( Ptr + 1 ) );
	word* Units = (word*) ( Ptr + 3 );
	for ( int i = 0; i < N; i++ )
	{
		word MID = Units[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				OB->NNUM += 32;
				OB->Selected = 0;
            }
        }
    }
	word* SU = Selm[NI];
	word* SN = SerN[NI];
	int N2 = NSL[NI];
	int N1 = 0;
	for ( int i = 0; i < N2; i++ )
	{
		word MID = SU[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( !( OB&&OB->NNUM >= 32 ) )
			{
				SU[N1] = MID;
				SN[N1] = SN[i];
				N1++;
            }
        }
    }
	NSL[NI] = N1;
	if ( N && !N1 )
	{
		free( Selm[NI] );
		Selm[NI] = nullptr;
		free( SerN[NI] );
		SerN[NI] = nullptr;
	}
	for ( int i = 0; i < N; i++ )
	{
		word MID = Units[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->NNUM >= 32 )
			{
				OB->NNUM -= 32;
            }
        }
    }

	return 3 + N * 2;
}

static int PACKSET( word* Set, int N, byte* outbuf, bool Write )
{
	byte BASESET[8192];
	memset( BASESET, 0, 8192 );
	for ( int i = 0; i < N; i++ )
	{
		int p = Set[i];
		if ( p < ULIMIT )BASESET[p >> 3] |= 1 << ( p & 7 );
    }
	byte LEVEL2[128 * 8];
	memset( LEVEL2, 0, 128 * 8 );
	int N2 = 0;
	for ( int i = 0; i < 1024 * 8; i++ )
	{
		if ( BASESET[i] )
		{
			LEVEL2[i >> 3] |= 1 << ( i & 7 );
			N2++;
        }
    }
	byte LEVEL1[16 * 8];
	memset( LEVEL1, 0, 16 * 8 );
	int N1 = 0;
	for ( int i = 0; i < 128 * 8; i++ )
	{
		if ( LEVEL2[i] )
		{
			LEVEL1[i >> 3] |= 1 << ( i & 7 );
			N1++;
        }
    }
	byte LEVEL0[16];
	memset( LEVEL0, 0, 16 );
	int N0 = 0;
	for ( int i = 0; i < 16 * 8; i++ )
	{
		if ( LEVEL1[i] )
		{
			LEVEL0[i >> 3] |= 1 << ( i & 7 );
			N0++;
        }
    }
	word LEVELX = 0;
	int NX = 0;
	for ( int i = 0; i < 16; i++ )
	{
		if ( LEVEL0[i] )
		{
			LEVELX |= 1 << i;
			NX++;
        }
    }
	if ( !Write )return 2 + NX + N0 + N1 + N2;
	memcpy( outbuf, &LEVELX, 2 );
	int pos = 2;
	for ( int i = 0; i < 16; i++ )
	{
		if ( LEVEL0[i] )
		{
			outbuf[pos] = LEVEL0[i];
			pos++;
        }
    }
	for ( int i = 0; i < 16 * 8; i++ )
	{
		if ( LEVEL1[i] )
		{
			outbuf[pos] = LEVEL1[i];
			pos++;
        }
    }
	for ( int i = 0; i < 128 * 8; i++ )
	{
		if ( LEVEL2[i] )
		{
			outbuf[pos] = LEVEL2[i];
			pos++;
        }
    }
	for ( int i = 0; i < 1024 * 8; i++ )
	{
		if ( BASESET[i] )
		{
			outbuf[pos] = BASESET[i];
			pos++;
        }
    }
	return pos;
}

static int UNPACKSET( byte* Data, int* L, word* Out, bool Write )
{
	byte BASESET[1024 * 8];
	memset( BASESET, 0, 1024 * 8 );
	byte LEVEL2[128 * 8];
	memset( LEVEL2, 0, 128 * 8 );
	byte LEVEL1[16 * 8];
	memset( LEVEL1, 0, 16 * 8 );
	byte LEVEL0[16];
	memset( LEVEL0, 0, 8 * 2 );
	word LEVELX = ( (word*) Data )[0];
	word msk = 1;
	int L0pos = 0;
	int L1pos = 0;
	int L2pos = 0;
	int L3pos = 0;
	int N0 = 0;
	int N1 = 0;
	int N2 = 0;
	int N3 = 0;
	byte* CBF = Data + 2;
	for ( int i = 0; i < 16; i++ )
	{
		if ( LEVELX&msk )N0++;
		msk <<= 1;
    }
	for ( int i = 0; i < N0; i++ )
	{
		byte ms = 1;
		byte cb = CBF[i];
		for ( int j = 0; j < 8; j++ )
		{
			if ( cb&ms )N1++;
			ms <<= 1;
        }
    }
	CBF += N0;
	for ( int i = 0; i < N1; i++ )
	{
		byte ms = 1;
		byte cb = CBF[i];
		for ( int j = 0; j < 8; j++ )
		{
			if ( cb&ms )N2++;
			ms <<= 1;
        }
    }
	CBF += N1;
	for ( int i = 0; i < N2; i++ )
	{
		byte ms = 1;
		byte cb = CBF[i];
		for ( int j = 0; j < 8; j++ )
		{
			if ( cb&ms )N3++;
			ms <<= 1;
        }
    }
	byte* L0BUF = Data + 2;
	byte* L1BUF = Data + 2 + N0;
	byte* L2BUF = Data + 2 + N0 + N1;
	byte* L3BUF = Data + 2 + N0 + N1 + N2;
	msk = 1;
	for ( int i = 0; i < 16; i++ )
	{
		if ( LEVELX&msk )
		{
			byte b0 = LEVEL0[i] = L0BUF[L0pos];
			L0pos++;
			byte m0 = 1;
			for ( int h = 0; h < 8; h++ )
			{
				if ( b0&m0 )
				{
					byte b = LEVEL1[i] = L1BUF[L1pos];
					L1pos++;
					byte m = 1;
					for ( int j = 0; j < 8; j++ )
					{
						if ( b&m )
						{
							byte b1 = LEVEL2[( i << 3 ) + j] = L2BUF[L2pos];
							L2pos++;
							byte m1 = 1;
							for ( int k = 0; k < 8; k++ )
							{
								if ( b1&m1 )
								{
									BASESET[k + ( j << 3 ) + ( h << 6 ) + ( i << 9 )] = L3BUF[L3pos];
									L3pos++;
                                }
								m1 <<= 1;
                            }
                        }
						m <<= 1;
                    }
                }
				m0 <<= 1;
            }
        }
		msk <<= 1;
    }
	int sz = 0;
	for ( int i = 0; i < 1024 * 8; i++ )
	{
		if ( BASESET[i] )
		{
			msk = 1;
			byte b = BASESET[i];
			for ( int j = 0; j < 8; j++ )
			{
				if ( b&msk )
				{
					if ( Write )
					{
						Out[sz] = j + ( i << 3 );
                    }
					sz++;
                }
				msk <<= 1;
            }
        }
    }
	*L = 2 + N0 + N1 + N2 + L3pos;
	return sz;
}

static int PREVSEUSETPOSTART = -1;
static int PREVNAT = -1;
static uint16_t PREVSTREAM[2048];
static int SUSTREAMSZ = 0;

static void CmdSelUnitsSet( byte NI, word* BUF, int NU )
{
	if ( EBPos == PREVSEUSETPOS&&SUSTREAMSZ + NU < 2048 && PREVNAT == NI )
	{
		memcpy( PREVSTREAM + SUSTREAMSZ, BUF, NU + NU );
		SUSTREAMSZ += NU;
		EBPos = PREVSEUSETPOSTART;

		int sz = PACKSET( PREVSTREAM, SUSTREAMSZ, ExBuf + EBPos + 2, 1 );
		EBPos += 2 + sz;
		PREVSEUSETPOS = EBPos;
		PREVNAT = NI;

	}
	else
	{
		ExBuf[EBPos] = 204;
		ExBuf[EBPos + 1] = NI + 32;
		int sz = PACKSET( BUF, NU, ExBuf + EBPos + 2, 1 );
		PREVSEUSETPOSTART = EBPos;
		EBPos += 2 + sz;
		PREVSEUSETPOS = EBPos;
		PREVNAT = NI;
		SUSTREAMSZ = NU;
		memcpy( PREVSTREAM, BUF, NU + NU );
	}
}

static int _implSelUnitsSet( byte* Ptr )
{
	byte NI = Ptr[0];
	word N;
	word* Units;
	word* tempbuf = nullptr;
	int sz = 0;
	if ( NI >= 32 )
	{
		N = UNPACKSET( Ptr + 1, &sz, nullptr, 0 );
		tempbuf = new word[N];
		UNPACKSET( Ptr + 1, &sz, tempbuf, 1 );
		Units = tempbuf;
		NI &= 15;
	}
	else
	{
		N = *( (word*) ( Ptr + 1 ) );
		Units = (word*) ( Ptr + 3 );
    }
	int N1 = 0;
	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB&&OB->NNUM == NatRefTBL[NI] )N1++;
	}

	int N0 = NSL[NI];
	Selm[NI] = (word*) realloc( Selm[NI], ( N0 + N1 ) * 2 );
	SerN[NI] = (word*) realloc( SerN[NI], ( N0 + N1 ) * 2 );
	N1 = N0;

	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB&&OB->NNUM == NatRefTBL[NI] )
		{
			Selm[NI][N1] = OB->Index;
			SerN[NI][N1] = OB->Serial;
			OB->Selected |= GM( NI );
			N1++;
		}
	}

	NSL[NI] = N1;

	if ( tempbuf )
	{
		free( tempbuf );
		return 1 + sz;
	}

	return 3 + N * 2;
}

static void SmartSelectionCorrector( byte NI, word* M0, int N0 )
{
	word* M1 = ImSelm[NI];
    word NADD = 0;
	word NSUB = 0;
	word SUBM[2048];
	word ADDM[2048];

	int N1 = ImNSL[NI];
	for ( int i = 0; i < N0; i++ )
	{
		word MID = M0[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )OB->NNUM |= 32;
        }
    }
	for ( int i = 0; i < N1; i++ )
	{
		word MID = M1[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				OB->NNUM |= 64;
				if ( NADD < 2048 && !( OB->NNUM & 32 ) )
				{
					ADDM[NADD] = OB->Index;
					NADD++;
                }
            }
        }
    }
	for ( int i = 0; i < N0; i++ )
	{
		word MID = M0[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				if ( NSUB < 2048 && !( OB->NNUM & 64 ) )
				{
					SUBM[NSUB] = OB->Index;
					NSUB++;
                }
            }
        }
    }
	if ( NSUB || NADD )
	{
		if ( NSUB + NADD > N1 )
		{
			CmdFreeSelected( NI );
			if ( N1 )CmdSelUnitsSet( NI, M1, N1 );
		}
		else
		{
			if ( NSUB )CmdUnSelUnitsSet( NI, SUBM, NSUB );
			if ( NADD )CmdSelUnitsSet( NI, ADDM, NADD );
        }
    }

	for ( int i = 0; i < N0; i++ )
	{
		word MID = M0[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				OB->NNUM &= 15;
			}
		}
	}

	for ( int i = 0; i < N1; i++ )
	{
		word MID = M1[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )OB->NNUM &= 15;
		}
	}
}

//-----------------OLD UNITS COMPRESSION----------------
static int UNPACKSET_OLD( byte* Data, int* L, word* Out, bool Write )
{
	byte BASESET[1024];
	memset( BASESET, 0, 1024 );
	byte LEVEL2[128];
	memset( LEVEL2, 0, 128 );
	byte LEVEL1[16];
	memset( LEVEL1, 0, 16 );
	word LEVEL0 = *( (word*) ( Data ) );
	word msk = 1;
	int L1pos = 0;
	int L2pos = 0;
	int L3pos = 0;
	int N1 = 0;
	int N2 = 0;
	int N3 = 0;
	byte* CBF = Data + 2;
	for ( int i = 0; i < 16; i++ )
	{
		if ( LEVEL0&msk )N1++;
		msk <<= 1;
    }
	for ( int i = 0; i < N1; i++ )
	{
		byte ms = 1;
		byte cb = CBF[i];
		for ( int j = 0; j < 8; j++ )
		{
			if ( cb&ms )N2++;
			ms <<= 1;
        }
    }
	CBF += N1;
	for ( int i = 0; i < N2; i++ )
	{
		byte ms = 1;
		byte cb = CBF[i];
		for ( int j = 0; j < 8; j++ )
		{
			if ( cb&ms )N3++;
			ms <<= 1;
        }
    }
	byte* L1BUF = Data + 2;
	byte* L2BUF = Data + 2 + N1;
	byte* L3BUF = Data + 2 + N1 + N2;
	msk = 1;
	for ( int i = 0; i < 16; i++ )
	{
		if ( LEVEL0&msk )
		{
			byte b = LEVEL1[i] = L1BUF[L1pos];
			L1pos++;
			byte m = 1;
			for ( int j = 0; j < 8; j++ )
			{
				if ( b&m )
				{
					byte b1 = LEVEL2[( i << 3 ) + j] = L2BUF[L2pos];
					L2pos++;
					byte m1 = 1;
					for ( int k = 0; k < 8; k++ )
					{
						if ( b1&m1 )
						{
							BASESET[k + ( j << 3 ) + ( i << 6 )] = L3BUF[L3pos];
							L3pos++;
                        }
						m1 <<= 1;
                    }
                }
				m <<= 1;
            }
        }
		msk <<= 1;
    }
	int sz = 0;
	for ( int i = 0; i < 1024; i++ )
	{
		if ( BASESET[i] )
		{
			msk = 1;
			byte b = BASESET[i];
			for ( int j = 0; j < 8; j++ )
			{
				if ( b&msk )
				{
					if ( Write )
					{
						Out[sz] = j + ( i << 3 );
                    }
					sz++;
                }
				msk <<= 1;
            }
        }
    }
	*L = 2 + N1 + N2 + L3pos;
	return sz;
}

static int _implSelUnitsSet_OLD( byte* Ptr )
{
	byte NI = Ptr[0];
	word N;
	word* Units;
	word* tempbuf = nullptr;
	int sz = 0;
	if ( NI >= 32 )
	{
		N = UNPACKSET_OLD( Ptr + 1, &sz, nullptr, 0 );
		tempbuf = new word[N];
		UNPACKSET_OLD( Ptr + 1, &sz, tempbuf, 1 );
		Units = tempbuf;
		NI &= 15;
	}
	else
	{
		N = *( (word*) ( Ptr + 1 ) );
		Units = (word*) ( Ptr + 3 );
    }
	int N1 = 0;
	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB&&OB->NNUM == NatRefTBL[NI] )N1++;
    }
	int N0 = NSL[NI];
	Selm[NI] = (word*) realloc( Selm[NI], ( N0 + N1 ) * 2 );
	SerN[NI] = (word*) realloc( SerN[NI], ( N0 + N1 ) * 2 );
	N1 = N0;
	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB&&OB->NNUM == NatRefTBL[NI] )
		{
			Selm[NI][N1] = OB->Index;
			SerN[NI][N1] = OB->Serial;
			OB->Selected |= GM( NI );
			N1++;
        }
    }
	NSL[NI] = N1;
	//CorrectBrigadesSelection(NI);
	if ( tempbuf )
	{
		free( tempbuf );
		return 1 + sz;
    }
	return 3 + N * 2;
}



// BoonXRay: code below from IR_Main.cpp

//#pragma pack(1)
struct PLAYERLOGIN
{
    bool Logged;
    byte Rank;
};
#warning Not initialized but used... (BoonXRay)
static PLAYERLOGIN LOGIN;


static void GetAllPar3( int LXMAX, int* N, int* LMaxX, const char* Message )
{
    int NStr = 1;
    int pos = 0;
    int LM = 0;
    int L0 = 0;
    do
    {
        if (L0 > LM)LM = L0;
        if (Message[pos] == ' '&&L0 > LXMAX)
        {
            NStr++;
            L0 = 0;
            pos++;
        }
        else
        {
            int DX;
            L0 += GetRLCWidthUNICODE( YellowFont.RLC, (byte*) ( Message + pos ), &DX );
            pos += DX;
        }
        if (L0 > LM)LM = L0;
    } while (Message[pos]);
    *N = NStr;
    *LMaxX = LM;
}

void ShowCentralMessage4( const char* Message, int GPIDX, int DX )
{
    int LXMAX = 400;
    char ccx[256];
    int L0 = 0;
    int NStr = 1;
    int pos = 0;
    int LM = 0;
    GetAllPar3( LXMAX, &NStr, &LM, Message );
    int N1, LM1;
    int DXXX = 32;
    do
    {
        GetAllPar3( LXMAX - DXXX, &N1, &LM1, Message );
        DXXX += 32;
    } while (N1 == NStr&&N1 > 1);
    LXMAX -= DXXX - 64;
    GetAllPar3( LXMAX, &NStr, &LM, Message );
    ccx[0] = 0;
    pos = 0;
    int pos1 = 0;

    int DY = 25;
    if (CurPalette == 2)
    {
        DrawStdBar2(
            ( RealLx - 64 - LM ) / 2 - DX,
            ( RealLy - 70 ) / 2 + 18 - DY - ( NStr - 1 ) * 13,
            ( RealLx + 64 + LM ) / 2 - DX,
            ( RealLy - 70 ) / 2 + 55 - DY + ( NStr - 1 ) * 13,
            GPIDX
        );
    }
    else
    {
        DX = 0;
        DrawStdBar(
            ( RealLx - 64 - LM ) / 2 - DX,
            ( RealLy - 70 ) / 2 + 18 - DY - ( NStr - 1 ) * 13,
            ( RealLx + 64 + LM ) / 2 - DX,
            ( RealLy - 70 ) / 2 + 55 - DY + ( NStr - 1 ) * 13
        );
    }
    int y0 = ( RealLy - GetRLCHeight( YellowFont.RLC, 'W' ) ) / 2 + 1 - DY - ( NStr - 1 ) * 13;
    L0 = 0;
    do
    {
        if (( Message[pos] == ' '&&L0 > LXMAX ))
        {
            int LL = GetRLCStrWidth( ccx, &YellowFont );
            ShowString( ( RealLx - LL ) / 2 + 1 - DX, y0 + 1, ccx, &BlackFont );
            ShowString( ( RealLx - LL ) / 2 - DX, y0, ccx, &YellowFont );
            pos1 = 0;
            ccx[0] = 0;
            L0 = 0;
            pos++;
            y0 += 26;
        }
        else
        {
            int length;
            L0 += GetRLCWidthUNICODE( YellowFont.RLC, (byte*) ( Message + pos ), &length );
            memcpy( ccx + pos1, Message + pos, length );
            pos += length;
            pos1 += length;
            ccx[pos1] = 0;
        }
        if (!Message[pos])
        {
            int LL = GetRLCStrWidth( ccx, &YellowFont );
            ShowString( ( RealLx - LL ) / 2 + 1 - DX, y0 + 1, ccx, &BlackFont );
            ShowString( ( RealLx - LL ) / 2 - DX, y0, ccx, &YellowFont );
        }
    } while (Message[pos]);
}

void WaitWithError( const char* ID, int GPID )
{
    char ccc[1256];
    strcpy( ccc, GetTextByID( ID ) );
    ShowCentralMessage4( GetTextByID( ID ), GPID, 206 );
    FlipPages();
    KeyPressed = 0;
    do
    {
        ProcessMessages();
    } while (!KeyPressed);
}

//----------------------Personal user data--------------------//
struct OneRatedPlayer
{
    byte State;
    byte Mask;
    byte Color;
    byte NationID;
    int  Profile;
    int  Score;
    char Nick[32];
    word MaxScore;
    word MaxPopul;
    byte ScoreG[32];
    byte Popul[32];
};

struct SysTime
{
    word Year;
    word Month;
    word Day;
    word DayOfWeek;
    word Hour;
    word Min;
};

class CurrentGame
{
public:
    bool Active;
    char GameName[128];
    char MapName[128];
    int  ProfileID;
    char Nick[64];
    char InGameNick[64];
    char password[64];
    char email[127];
    bool Broken;
    bool Finished;

    SysTime StartTime;
    SysTime LastLoadTime;
    SysTime CurTime;

    int PlayingTime;
    int  NPL;
    byte GameType;
    OneRatedPlayer PLAYERS[8];
    //-----------------------------//
    void UpdateGame();
    void SaveGameToFile();
    void LoadGameFromFile();
    void DeleteThisGameFromFile();
    void AssignVictory( char* Nick );
    void AssignDefeat( char* Nick );
};

void CurrentGame::UpdateGame()
{
    if (Active)
    {
        PlayingTime = GameTime;
        byte ord[8];
        int  par[8];
        int no = SortPlayers( ord, par );
        byte prevms = 0;
        int CurTm = 0;
        SYSTEMTIME SYSTM;
        GetSystemTime( &SYSTM );
        CurTime.Year = SYSTM.wYear;
        CurTime.Month = SYSTM.wMonth;
        CurTime.Day = SYSTM.wDay;
        CurTime.Hour = SYSTM.wHour;
        CurTime.Min = SYSTM.wMinute;
        CurTime.DayOfWeek = SYSTM.wDayOfWeek;

        if (no)
        {
            for (int q = 0; q < no; q++)
            {
                byte ms = NATIONS[NatRefTBL[ord[q]]].NMask;
                if (NATIONS[NatRefTBL[ord[q]]].AI_Enabled)
                {
                    Active = 0;
                    DeleteThisGameFromFile();
                    return;
                }
                byte c = NatRefTBL[ord[q]];
                if (!( ms&prevms ))
                {
                    CurTm++;
                }
                int PIDIDX = -1;
                for (int w = 0; w < NPlayers; w++)if (PINFO[w].ColorID == ord[q])PIDIDX = w;
                if (PIDIDX == -1)
                {
                    if (NATIONS[c].AI_Enabled)
                    {
                        strcpy( PLAYERS[q].Nick, "Computer" );
                    }
                    else
                    {
                        strcpy( PLAYERS[q].Nick, "???" );
                    }
                }
                else
                {
                    strcpy( PLAYERS[q].Nick, PINFO[PIDIDX].name );
                    PLAYERS[q].Profile = PINFO[PIDIDX].ProfileID;
                }
                PLAYERS[q].Mask = NATIONS[c].NMask;
                PLAYERS[q].Color = c;
                PLAYERS[q].State = 3;
                if (CITY[c].Account >= 0)PLAYERS[q].Score = CITY[c].Account;
                else PLAYERS[q].Score = 0;

                //determining the nation
                int NID = 255;

                for (int j = 0; j < MAXOBJECT; j++)
                {
                    OneObject* OB = Group[j];
                    if (OB&&OB->NNUM == c&&OB->newMons->Peasant && !OB->Sdoxlo)
                    {
                        NID = OB->Ref.General->NatID;
                    }
                }
                PLAYERS[q].NationID = NID;
                if (PINFO[q].PlayerID == MyDPID)
                {
                    strcpy( InGameNick, PINFO[q].name );
                }

                memset( PLAYERS[q].Popul, 0, sizeof PLAYERS[q].Popul );
                memset( PLAYERS[q].ScoreG, 0, sizeof PLAYERS[q].ScoreG );

                if (NATIONS[c].NAccount > 1)
                {
                    int NN = NATIONS[c].NAccount - 1;
                    int MaxP = 1;
                    for (int i = 0; i < NN; i++)
                    {
                        int v = NATIONS[c].Account[i];
                        if (v > MaxP)MaxP = v;
                    }
                    for (int i = 0; i < 32; i++)
                    {
                        PLAYERS[q].ScoreG[i] = ( NATIONS[c].Account[( NN*i ) / 31] * 100 ) / MaxP;
                    }
                    PLAYERS[q].MaxScore = MaxP;
                }
                if (NATIONS[c].NPopul > 1)
                {
                    int NN = NATIONS[c].NPopul - 1;
                    int MaxP = 1;
                    for (int i = 0; i < NN; i++)
                    {
                        int v = NATIONS[c].Popul[i];
                        if (v > MaxP)MaxP = v;
                    }
                    for (int i = 0; i < 32; i++)
                    {
                        PLAYERS[q].Popul[i] = ( NATIONS[c].Popul[( NN*i ) / 31] * 100 ) / MaxP;
                    }
                    PLAYERS[q].MaxPopul = MaxP;
                }
            }
            NPL = no;
        }
        SaveGameToFile();
    }
}

void CurrentGame::AssignVictory( char* pNick )
{
    byte MyMask = 0;
    for (int i = 0; i < NPL; i++)if (!strcmp( PLAYERS[i].Nick, pNick ))
    {
        PLAYERS[i].State = 0;
        MyMask = PLAYERS[i].Mask;
    }
    for (int i = 0; i < NPL; i++)if (PLAYERS[i].State != 1 && PLAYERS[i].Mask&MyMask)
    {
        PLAYERS[i].State = 0;
    }
    else
    {
        PLAYERS[i].State = 1;
    }
    if (!Broken)
    {
        Finished = 1;
    }
}

void CurrentGame::AssignDefeat( char* pNick )
{
    bool ALLMYLOOSE = 1;
    byte MyMask = 0;
    for (int i = 0; i < NPL; i++)
    {
        if (!strcmp( PLAYERS[i].Nick, pNick ))
        {
            PLAYERS[i].State = 1;
            MyMask = PLAYERS[i].Mask;
        }
    }

    if (MyMask)
    {
        byte HimMask1 = 0;
        byte HimMask2 = 0;
        for (int i = 0; i < NPL; i++)
        {
            if (PLAYERS[i].Mask&MyMask)
            {
                if (PLAYERS[i].State != 1)
                {
                    ALLMYLOOSE = 0;
                }
            }
            else
            {
                if (HimMask1)
                {
                    if (!( PLAYERS[i].Mask&HimMask1 ))
                    {
                        HimMask2 = PLAYERS[i].Mask;
                    }
                }
                else
                {
                    HimMask1 = PLAYERS[i].Mask;
                }
            }
        }

        if (ALLMYLOOSE && !HimMask2)
        {
            for (int i = 0; i < NPL; i++)
            {
                if (PLAYERS[i].State != 1 && !( PLAYERS[i].Mask&MyMask ))
                {
                    PLAYERS[i].State = 0;
                }
            }
        }
    }

    if (!Broken)
    {
        Finished = 1;
    }
}

static void SetNormAttr();

void CurrentGame::SaveGameToFile()
{
    if (!Active)
    {
        return;
    }

    int t0 = ( StartTime.Min + StartTime.Hour * 60 ) % 200;

    ResFile F1 = RReset( "cew.dll" );
    if (F1 == INVALID_HANDLE_VALUE)
    {
        return;
    }

    RSeek( F1, 20532 + t0 );
    int v = 1;
    RBlockWrite( F1, &v, 1 );
    RSeek( F1, 20532 + 200 + t0*( sizeof( CurrentGame ) + 4 ) );

    CurrentGame XXX;
    XXX = *this;
    byte* DATA = (byte*) ( &XXX );
    byte* Key = (byte*) randoma;
    for ( size_t i = 0; i < sizeof XXX; i++)
    {
        DATA[i] ^= Key[i];
    }

    RBlockWrite( F1, &XXX, sizeof XXX );

    int S = 0;
    for ( size_t i = 0; i < sizeof( CurrentGame); i++)
    {
        S += DATA[i];
    }

    RBlockWrite( F1, &S, 4 );
    RClose( F1 );
    SetNormAttr();
}

void CurrentGame::LoadGameFromFile()
{
    ResFile F = RReset( "cew.dll" );
    if (F == INVALID_HANDLE_VALUE)
    {
        return;
    }

    RSeek( F, 20532 );
    int cur = 0;
    int t = GetTickCount() & 4096;

    for (int p = 0; p < t; p++)
    {
        rand();
    }

    int NMAX = 0;
    byte HEADR[200];
    RBlockRead( F, HEADR, 200 );

NEXTFILE:
    bool SUCCESS = 0;
    do
    {
        cur = rand() % 200;
        SUCCESS = 0 != HEADR[cur];
        NMAX++;
    } while (( !SUCCESS ) && NMAX < 5000);

    if (SUCCESS)
    {
        RSeek( F, 20532 + 200 + cur*( sizeof( CurrentGame ) + 4 ) );
        RBlockRead( F, this, sizeof( CurrentGame) );
        int S;
        RBlockRead( F, &S, 4 );
        byte* Key = (byte*) randoma;
        byte* xx = ( byte* ) this;

        int S1 = 0;
        for ( size_t i = 0; i < sizeof( CurrentGame); i++)
        {
            S1 += xx[i];
        }
        if (S != S1)
        {
            Active = false;
        }
        else
        {
            for ( size_t i = 0; i < sizeof( CurrentGame); i++)
            {
                xx[i] ^= Key[i];
            }
        }

        if (!Finished)
        {
            //checking the start time
            SYSTEMTIME ST;
            GetSystemTime( &ST );
            FILETIME FT;
            SystemTimeToFileTime( &ST, &FT );
            SYSTEMTIME ST0;
            memset( &ST0, 0, sizeof ST0 );
            ST0.wMinute = StartTime.Min;
            ST0.wHour = StartTime.Hour;
            ST0.wDay = StartTime.Day;
            ST0.wMonth = StartTime.Month;
            ST0.wYear = StartTime.Year;
            FILETIME FT0;
            SystemTimeToFileTime( &ST0, &FT0 );
            LARGE_INTEGER LI0;
            LARGE_INTEGER LI;
            memcpy( &LI0, &FT0, 8 );
            memcpy( &LI, &FT, 8 );
            LI.QuadPart -= LI0.QuadPart;
            memcpy( &FT, &LI, 8 );

            int dt = 10000000;
            if (FileTimeToSystemTime( &FT, &ST ))
            {
                dt = ( ST.wDay - 1 ) * 24 + ST.wHour;
            }

            if (dt < 24)
            {
                RClose( F );
                Active = 0;
                F = INVALID_HANDLE_VALUE;
                goto NEXTFILE;
            }
        }
    }
    else
    {
        Active = false;
    }
    RClose( F );
    SetNormAttr();
}

void CurrentGame::DeleteThisGameFromFile()
{
    ResFile F = RReset( "cew.dll" );
    if (F == INVALID_HANDLE_VALUE)
    {
        return;
    }

    int t0 = ( StartTime.Min + StartTime.Hour * 60 ) % 200;
    RSeek( F, 20532 + t0 );
    int v = 0;
    RBlockWrite( F, &v, 1 );
    RClose( F );
    SetNormAttr();
}

static CurrentGame CURIGAME;

static void CheckExistingSaves()
{
    if (!LOGIN.Logged)return;
    //CurrentGame CGM;
    CURIGAME.Active = 0;
    CURIGAME.LoadGameFromFile();
    CURIGAME.Active = 0;
}

void LOOSEIGAME( char* Nick )
{
    if (CURIGAME.Active)
    {
        CURIGAME.UpdateGame();
        CURIGAME.AssignDefeat( Nick );
        CURIGAME.SaveGameToFile();
        ShowGameScreen = 1;
        CheckExistingSaves();
        ShowGameScreen = 0;
        CURIGAME.Active = 0;
    }
}

void LOOSEANDEXITFAST()
{
    if (CURIGAME.Active)
    {
        for (int i = 0; i < NPlayers; i++)
        {
            if (PINFO[i].ColorID == MyNation)
            {
                CURIGAME.UpdateGame();
                CURIGAME.AssignDefeat( PINFO[i].name );
                CURIGAME.SaveGameToFile();
                CURIGAME.Active = 0;
            }
        }
    }
}

void WINIGAME( char* Nick )
{
    if (CURIGAME.Active)
    {
        CURIGAME.UpdateGame();
        CURIGAME.AssignVictory( Nick );
        CURIGAME.SaveGameToFile();
        ShowGameScreen = 1;
        CheckExistingSaves();
        ShowGameScreen = 0;
        CURIGAME.Active = 0;
    }
}

int GetLogRank()
{
    if (LOGIN.Logged)
    {
        return LOGIN.Rank;
    }
    else
    {
        return 0;
    }
}

void DontMakeRaiting()
{
    if (CURIGAME.Active)
    {
        CURIGAME.DeleteThisGameFromFile();
        CURIGAME.Active = 0;
    }
}

void StopRaiting()
{
    if (CURIGAME.Active)
    {
        CURIGAME.Active = 0;
    }
}

static int PBackTime = 0;

void ProcessUpdate()
{
    if (PBackTime == 0)
    {
        PBackTime = GetTickCount() - 100000;
    }

    if (GetTickCount() - PBackTime > 100000)
    {
        if (NPlayers <= 1)
            DontMakeRaiting();

        PBackTime = GetTickCount();

        if (CURIGAME.Active)
            CURIGAME.UpdateGame();
    }
}
//---------------------TOP100-------------------//

static constexpr int SECCOD0 = 0;
static constexpr int SECCOD1 = 0;
static constexpr int SECCOD2 = 0;
static constexpr int SECCOD3 = 0;

bool IsGameActive()
{
    return CURIGAME.Active;
}

int GetCurGamePtr( uint8_t ** Ptr )
{
    *Ptr = (byte*) &CURIGAME;
    return sizeof CURIGAME;
}

void UpdateCurGame()
{
    if (CURIGAME.Active)
    {
        SYSTEMTIME SYSTM;
        GetSystemTime( &SYSTM );
        CURIGAME.LastLoadTime.Year = SYSTM.wYear;
        CURIGAME.LastLoadTime.Month = SYSTM.wMonth;
        CURIGAME.LastLoadTime.Day = SYSTM.wDay;
        CURIGAME.LastLoadTime.Hour = SYSTM.wHour;
        CURIGAME.LastLoadTime.Min = SYSTM.wMinute;
        CURIGAME.LastLoadTime.DayOfWeek = SYSTM.wDayOfWeek;
        memcpy( &CURIGAME.CurTime, &CURIGAME.LastLoadTime, sizeof CURIGAME.LastLoadTime );
    }
}

bool CheckGameTime()
{
    if (CURIGAME.Active)
    {
        SYSTEMTIME ST;
        memset( &ST, 0, sizeof ST );
        ST.wMinute = CURIGAME.CurTime.Min;
        ST.wHour = CURIGAME.CurTime.Hour;
        ST.wDay = CURIGAME.CurTime.Day;
        ST.wMonth = CURIGAME.CurTime.Month;
        ST.wYear = CURIGAME.CurTime.Year;
        FILETIME FT;
        SystemTimeToFileTime( &ST, &FT );
        SYSTEMTIME ST0;
        memset( &ST0, 0, sizeof ST0 );
        ST0.wMinute = CURIGAME.LastLoadTime.Min;
        ST0.wHour = CURIGAME.LastLoadTime.Hour;
        ST0.wDay = CURIGAME.LastLoadTime.Day;
        ST0.wMonth = CURIGAME.LastLoadTime.Month;
        ST0.wYear = CURIGAME.LastLoadTime.Year;
        FILETIME FT0;
        SystemTimeToFileTime( &ST0, &FT0 );
        LARGE_INTEGER LI0;
        LARGE_INTEGER LI;
        memcpy( &LI0, &FT0, 8 );
        memcpy( &LI, &FT, 8 );
        LI.QuadPart -= LI0.QuadPart;
        memcpy( &FT, &LI, 8 );

        int dt = 0;
        if (FileTimeToSystemTime( &FT, &ST ))
        {
            dt = ( ST.wDay - 1 ) * 24 * 60 + ST.wHour * 60 + ST.wMinute;
        }
        return dt >= 10;
    }
    else
    {
        return false;
    }
}

static void SetNormAttr()
{
    HANDLE H = CreateFile( "cew.dll",
        GENERIC_WRITE, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if (H == INVALID_HANDLE_VALUE)
    {
        return;
    }

    FILETIME F1, F2, F3;
    GetFileTime( H, &F1, &F2, &F3 );
    F3 = F1;
    F2 = F2;
    SetFileTime( H, &F1, &F2, &F3 );
    CloseHandle( H );
}

//Returns GetGSC_Profile()
int GetMyProfile()
{
    return GetGSC_Profile();
}

void SetBrokenState()
{
    if (CURIGAME.Active)
    {
        CURIGAME.Broken = 1;
    }
}
