#ifndef MPLAYER_H
#define MPLAYER_H

#include <cstdint>

#include <dplay.h>

extern uint16_t PlayerMenuMode;

constexpr int MaxPL = 8;

#pragma pack( push, 1 )

struct EXBUFFER
{
    uint32_t Size;
	bool Enabled;
    uint32_t Sign;//0xF376425E
    uint32_t RealTime;//if(??==0xFFFFFFFF)-empty buffer
    uint32_t RandIndex;
    uint8_t  Data[4096];
};

struct OnePing
{
	int FromTime;
	int ToTime;
	int BackTime;
};

class PingsSet
{
public:
    uint32_t DPID;
	int NPings;
	int MaxPings;
	OnePing* Pings;
};

class PingSumm
{
public:
	int NPL;
	PingsSet* PSET;
	PingSumm();
	~PingSumm();
	void ClearPingInfo();
    void AddPing( uint32_t DPID, uint32_t From, uint32_t To, uint32_t Back );
    void AddPlayer( uint32_t DPID );
    int GetTimeDifference( uint32_t DPID );
    int CheckPlayer( uint32_t DPID );
};
extern PingSumm PSUMM;

struct BACKUPSTR
{
    uint32_t  ID;
    uint32_t RealTime;
	byte* Data;
	int   L;
};

class PLAYERSBACKUP
{
public:
	BACKUPSTR BSTR[32];
	int NBDATA;
	PLAYERSBACKUP();
	~PLAYERSBACKUP();
	void Clear();
    void AddInf( byte* BUF, int L, uint32_t ID, int RT );
    void SendInfoAboutTo( uint32_t ID, uint32_t TO, uint32_t RT );
};

struct SingleRetr
{
    uint32_t IDTO;
    uint32_t IDFROM;
    uint32_t RT;
};

class RETRANS
{
public:
	SingleRetr* TOT;
	int NRET;
	int MaxRET;
	RETRANS();
	~RETRANS();
    void AddOneRet( uint32_t TO, uint32_t From, uint32_t RT );
    void AddSection( uint32_t TO, uint32_t From, uint32_t RT );
    void CheckRetr( uint32_t From, uint32_t RT );
	void Clear();
};

extern PLAYERSBACKUP PBACK;
extern RETRANS RETSYS;

struct RoomInfo
{
	char Name[128];
	char Nick[64];
	char RoomIP[32];
    uint32_t Profile;
	char GameID[64];
	int MaxPlayers;

	//Additional members to pass data from server to main exe / CommCore
	long player_id; //Necessary for host to send udp hole punching packets
	unsigned short port; //Udp hole punching port or real port of game host
	unsigned udp_interval; //Udp hole punching packet interval
	char udp_server[16]; //IP of udp hole punching server
};

int Process_GSC_ChatWindow( bool Active, RoomInfo* RIF );
void LeaveGSCRoom();
void StartGSCGame( const char* Options, char* Map, int NPlayers, int* Profiles, char** Nations, int* Teams, int* Colors );

struct OnePlayerReport
{
    uint32_t Profile;
	byte State;
    uint16_t Score;
    uint16_t Population;
    uint32_t ReachRes[6];
    uint16_t NBornP;
    uint16_t NBornUnits;
};

void ReportGSCGame( int time, int NPlayers, OnePlayerReport* OPR );
void ReportAliveState( int NPlayers, int* Profiles );
void SendVictoryState( int PlayerID, uint8_t State );

//Beware: Sensitive to pragma pack(1) and exact structure size
//Can cause bugs. Fixed one in StartIGame()
//Original size: 132 bytes
//Used in MPlayer.cpp & CommCore with offset/size calculations
//e.g. memcpy(x, &PINFO[i].NationID, sizeof(PlayerInfo)-36)
struct PlayerInfo
{
    char name[32];
    // BoonXRay 26.08.2017
    //DPID1 PlayerID;
    uint32_t PlayerID;
    uint8_t NationID;
    uint8_t ColorID;//7 player colors coded as 0x00 - 0x06
    uint8_t GroupID;
    char MapName[36 + 8];//60-16-1-7-4 (?)
    int ProfileID;
    uint32_t Game_GUID;
    uint8_t UserParam[7];//Values of additional game options
    uint8_t Rank;
    uint16_t COMPINFO[8];
    int  CHKSUMM;
    uint8_t MapStyle;
    uint8_t HillType;
    uint8_t StartRes;
    uint8_t ResOnMap;
    uint8_t Ready;
    uint8_t Host;
    uint8_t Page;
    uint8_t CD;
    uint16_t Version;
    uint8_t VictCond;
    uint16_t GameTime;

    //New elements added after this point

    //3 elements for additional options
    //Didn't want to resize UserParam because 'uint16_t Version' would be moved.
    //This way you can recognize and be recognized by old 1.35 as new version.
    uint8_t UserParam2[3];

    int speed_mode;
};

#pragma pack(pop)

extern PlayerInfo PINFO[ 8 ];

extern uint16_t COMPSTART[8];
extern uint16_t PrevRpos;

extern uint32_t EBPos1;

extern uint16_t NPlayers;

extern char CHATSTRING[256];
extern uint32_t CHATDPID;

extern bool GameInProgress;

extern LPDIRECTPLAY3A		lpDirectPlay3A;

extern uint32_t MyDPID;

extern int GMTYPE;

extern char SaveFileName[128];

extern int CurrentStartTime[8];
extern int NextStartTime[8];

extern int CurStatus;

extern int CurrentMaxPing[8];

extern DPID ServerDPID;

extern uint32_t RealTime;
extern int PREVGLOBALTIME;

extern int ExitNI;

//network errors:
extern int SeqErrorsCount;

extern EXBUFFER EBufs[MaxPL];
extern EXBUFFER EBufs1[MaxPL];

extern bool LockFog;
//Describes which players are out of sync
extern uint8_t SYNBAD[8];

void SendChat( char* str, bool );

__declspec( dllexport ) void CloseMPL();
class ListBox;
void LBEnumerateSessions( ListBox* LB, int );

void AnalyseMessages();

bool CreateNamedSession( char* Name, DWORD User2, int Max );
bool JoinNameToSession( int ns, char* Name );

__declspec( dllexport ) bool StartIGame( bool SINGLE );

void ProcessNetCash();
void ProcessReceive();
void ProcessNewInternet();

void IAmLeft();

bool FindSessionAndJoin( char* Name, char* Nick, bool Style, unsigned short port );
bool CreateSession( char* SessName, char* Name, uint32_t User2, bool Style, int MaxPlayers );
void StopConnectionToSession( LPDIRECTPLAY3A lpDirectPlay3A );

extern char ACCESS[16];
bool ProcessNewInternetLogin();
bool PIEnumeratePlayers( PlayerInfo* PIN, bool DoMsg );

__declspec( dllexport ) void SendPings();
bool CheckPingsReady();

bool CheckForPersonalChat( char* STR );
bool CheckPersonality( char* MESSAGE );

void ReceiveAll();

void CreateDiffStr( char* str );

int GetMaxRealPing();

bool SendToAllPlayers( uint32_t Size, void * lpData );
void SETPLAYERNAME( DPNAME* lpdpName, bool );

int GetReadyPercent();

void SETPLAYERDATA( uint32_t ID, void* Data, int size, bool change );

void ClearLPACK();

void DeepDeletePeer( uint32_t ID );

void ChatProcess();

void GoHomeAnyway();

bool CreateMultiplaterInterface();
void SetupMultiplayer( HINSTANCE hInstance );
void InitMultiDialogs();
void ShutdownMultiplayer( bool Final );

void GetGameID( char* s );

void HandleMultiplayer();
void SyncroDoctor();
int GetPing( DPID pid );

void LoadSaveFile();

#endif // MPLAYER_H
