#include <cstdio>
#include <windows.h>

#include "SelProp.h"

#include "ConstStr.h"

char * DELSYNC = nullptr;
char * STOPUPG = nullptr;
char * HIEDMOD = nullptr;
char * ADDSPOB = nullptr;
char * DELSPOB = nullptr;
char * TEXMODE = nullptr;
char * UNBLOCK = nullptr;
char * DOBLOCK = nullptr;
char * DFBLOCK = nullptr;
char * PEACMOD = nullptr;
char * SETWATT = nullptr;
char * DELWATT = nullptr;
char * SETCOST = nullptr;
char * DELCOST = nullptr;
char * GETSPOS = nullptr;
char * PLAYMES = nullptr;
char * ECONOM1 = nullptr;
char * ECONOM2 = nullptr;
char * ECONOM3 = nullptr;
char * ECONOM4 = nullptr;
char * ECONOM5 = nullptr;
char * ECONOM6 = nullptr;
char * ECONOM7 = nullptr;
char * ECONOM8 = nullptr;
char * ECONOM9 = nullptr;
char * ECONOMA = nullptr;
char * ECOSELL = nullptr;
char * ECOBUY = nullptr;
char * VERYHUN = nullptr;
char * MESHPOV = nullptr;
char * MESHOBJ = nullptr;
char * NETRESS = nullptr;
char * NETRES1 = nullptr;
char * OPENGAT = nullptr;
char * CLOSGAT = nullptr;
char * STANDGR = nullptr;
char * NOSTAND = nullptr;
char * SFREORD = nullptr;
char * ARMYSTR = nullptr;
char * KILLUNI = nullptr;
char * FISHSTR = nullptr;
char * HABITAN = nullptr;
char * MAXHABI = nullptr;
char * INSISTR = nullptr;
char * MAXINSI = nullptr;
char * MAXINS1 = nullptr;
char * SDS_Str = nullptr;
char * GOATTA = nullptr;
char * PATROL = nullptr;
char * EATTACK = nullptr;
char * DATTACK = nullptr;
char * NOFARMS = nullptr;
char * UNLOADALL = nullptr;
char * IDLE_P = nullptr;
char * IDLE_M = nullptr;
char * INFO_G1 = nullptr;
char * INFO_G2 = nullptr;
char * INFO_G3 = nullptr;
char * INFO_G4 = nullptr;
char * NationalForces = nullptr;
char * Economica = nullptr;
char * E_Comment = nullptr;
char * Doxod = nullptr;
char * INFO_R0 = nullptr;
char * INFO_R1 = nullptr;
char * INFO_R2 = nullptr;
char * INFO_R3 = nullptr;
char * INFO_R4 = nullptr;
char * INFO_R5 = nullptr;
char * INFO_R6 = nullptr;
char * INFO_R7 = nullptr;
char * INFO_R8 = nullptr;
char * UPGMADE = nullptr;
char * CAPBLD = nullptr;
char * LOSTBLD = nullptr;
char * BLDBLD = nullptr;
char * FINBLD = nullptr;
char * INFO_W = nullptr;
char * INFO_S = nullptr;
char * INFO_F = nullptr;
char * GONATT = nullptr;
char * TUTORIAL = nullptr;
char * UNILIMIT = nullptr;
char * SLOWMOD = nullptr;
char * FASTMOD = nullptr;
char * BESTPL = nullptr;
char * GOLDFIN = nullptr;
char * PAUSETEXT = nullptr;
char * BTN_General = nullptr;
char * BTN_Descr = nullptr;
char * BTN_Upgr = nullptr;
char * BTN_Next = nullptr;
char * BTN_Prev = nullptr;
char * BTN_PriceUpg = nullptr;
char * GI_UNITS = nullptr;
char * GI_BLD = nullptr;
char * ENCHAT = nullptr;
char * HDOGUARD = nullptr;
char * HNOGUARD = nullptr;
char * HPATROL = nullptr;
char * ATGHINT = nullptr;
char * SER_SAVE = nullptr;
char * DISCONNECTED = nullptr;

bool LOADSC_Bool( const char * ID, char ** str )
{
    * str = GetTextByID( ID );
    if( strcmp( * str, ID ) == 0 )  // Equal -> error
    {
        char cc[128];
        sprintf( cc, "Unknown string: %s (see COMMENT.TXT)", ID );
        MessageBox( NULL, cc, "String not found...", 0 );
        return false;
    }
    return true;
}

#define LoadSCB(x) LOADSC_Bool( #x, & x )

bool InitConstStr()
{
    return LoadSCB( DELSYNC ) &&
           LoadSCB( STOPUPG ) &&
           LoadSCB( HIEDMOD ) &&
           LoadSCB( ADDSPOB ) &&
           LoadSCB( DELSPOB ) &&
           LoadSCB( TEXMODE ) &&
           LoadSCB( UNBLOCK ) &&
           LoadSCB( DOBLOCK ) &&
           LoadSCB( DFBLOCK ) &&
           LoadSCB( PEACMOD ) &&
           LoadSCB( SETWATT ) &&
           LoadSCB( DELWATT ) &&
           LoadSCB( SETCOST ) &&
           LoadSCB( DELCOST ) &&
           LoadSCB( GETSPOS ) &&
           LoadSCB( PLAYMES ) &&
           LoadSCB( ECONOM1 ) &&
           LoadSCB( ECONOM2 ) &&
           LoadSCB( ECONOM3 ) &&
           LoadSCB( ECONOM4 ) &&
           LoadSCB( ECONOM5 ) &&
           LoadSCB( ECONOM6 ) &&
           LoadSCB( ECONOM7 ) &&
           LoadSCB( ECONOM8 ) &&
           LoadSCB( ECONOM9 ) &&
           LoadSCB( ECONOMA ) &&
           LoadSCB( ECOSELL ) &&
           LoadSCB( ECOBUY ) &&
           LoadSCB( VERYHUN ) &&
           LoadSCB( MESHPOV ) &&
           LoadSCB( MESHOBJ ) &&
           LoadSCB( NETRESS ) &&
           LoadSCB( NETRES1 ) &&
           LoadSCB( OPENGAT ) &&
           LoadSCB( CLOSGAT ) &&
           LoadSCB( STANDGR ) &&
           LoadSCB( NOSTAND ) &&
           LoadSCB( SFREORD ) &&
           LoadSCB( ARMYSTR ) &&
           LoadSCB( KILLUNI ) &&
           LoadSCB( FISHSTR ) &&
           LoadSCB( HABITAN ) &&
           LoadSCB( MAXHABI ) &&
           LoadSCB( INSISTR ) &&
           LoadSCB( MAXINSI ) &&
           LoadSCB( MAXINS1 ) &&
           LoadSCB( SDS_Str ) &&
           LoadSCB( GOATTA ) &&
           LoadSCB( PATROL ) &&
           LoadSCB( EATTACK ) &&
           LoadSCB( DATTACK ) &&
           LoadSCB( NOFARMS ) &&
           LoadSCB( UNLOADALL ) &&
           LoadSCB( IDLE_P ) &&
           LoadSCB( IDLE_M ) &&
           LoadSCB( INFO_G1 ) &&
           LoadSCB( INFO_G2 ) &&
           LoadSCB( INFO_G3 ) &&
           LoadSCB( INFO_G4 ) &&
           LoadSCB( NationalForces ) &&
           LoadSCB( Economica ) &&
           LoadSCB( E_Comment ) &&
           LoadSCB( Doxod ) &&
           LoadSCB( INFO_R0 ) &&
           LoadSCB( INFO_R1 ) &&
           LoadSCB( INFO_R2 ) &&
           LoadSCB( INFO_R3 ) &&
           LoadSCB( INFO_R4 ) &&
           LoadSCB( INFO_R5 ) &&
           LoadSCB( INFO_R6 ) &&
           LoadSCB( INFO_R7 ) &&
           LoadSCB( INFO_R8 ) &&
           LoadSCB( UPGMADE ) &&
           LoadSCB( CAPBLD ) &&
           LoadSCB( LOSTBLD ) &&
           LoadSCB( BLDBLD ) &&
           LoadSCB( FINBLD ) &&
           LoadSCB( INFO_W ) &&
           LoadSCB( INFO_S ) &&
           LoadSCB( INFO_F ) &&
           LoadSCB( GONATT ) &&
           LoadSCB( TUTORIAL ) &&
           LoadSCB( UNILIMIT ) &&
           LoadSCB( SLOWMOD ) &&
           LoadSCB( FASTMOD ) &&
           LoadSCB( BESTPL ) &&
           LoadSCB( GOLDFIN ) &&
           LoadSCB( PAUSETEXT ) &&
           LoadSCB( BTN_General ) &&
           LoadSCB( BTN_Descr ) &&
           LoadSCB( BTN_Upgr ) &&
           LoadSCB( BTN_Next ) &&
           LoadSCB( BTN_Prev ) &&
           LoadSCB( BTN_PriceUpg ) &&
           LoadSCB( GI_UNITS ) &&
           LoadSCB( GI_BLD ) &&
           LoadSCB( ENCHAT ) &&
           LoadSCB( HDOGUARD ) &&
           LoadSCB( HNOGUARD ) &&
           LoadSCB( HPATROL ) &&
           LoadSCB( ATGHINT ) &&
           LoadSCB( SER_SAVE ) &&
           LoadSCB( DISCONNECTED );
}
