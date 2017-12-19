#ifndef NATURE_H
#define NATURE_H

#include <cstdint>

class ResDiscr{
public:
	bool Enabled;
	char Name[32];
	char ResIDS[32];
	ResDiscr();
};

extern int URESRC[8][8];
#define XRESRC(i,j) (URESRC[i][j]^134525)
#define SetXRESRC(i,j,k) URESRC[i][j]=(k^134525)
#define AddXRESRC(i,j,k) URESRC[i][j]=(((URESRC[i][j]^134525)+k)^134525)

extern int RESADD[8][8];
extern int  NWeaponIDS;
extern short ProtectIcn[32];
extern int SGenIcon;

extern int ATTGR_ICON;
extern int AttGrPos;

extern int SET_DEST_ICON;
extern int SDS_X;
extern int SDS_Y;
extern int STOP_ICON;
extern int UNLOAD_ICON;
extern int IDLE_PICON;
extern int IDLE_MICON;
extern int IDLE_PX;
extern int IDLE_PY;
extern int IDLE_MX;
extern int IDLE_MY;
extern int NOGUARD_ICON;
extern int DOGUARD_ICON;

void LoadRDS();

//bool CheckCost(byte NI,byte ResID,word Amount);
extern ResDiscr RDS[8];
extern int OrderIcon[4];
extern int FREE_ORDER_ICON;
extern int STAND_GROUND_ICON;
extern int NOT_STAND_GROUND_ICON;
extern int ORDERS_LIST_ICON;
extern int EATT_X;
extern int EATT_Y;
extern int EATT_Icon;
extern int DATT_X;
extern int DATT_Y;
extern int DATT_Icon;

extern int GOAT_PX;
extern int GOAT_PY;
extern int GOAT_Icon;

extern int FILLFORM_ICON;
extern int PATROL_ICON;
extern int ADDRANGE;
extern int ADDPAUSE;
extern int UNGROUP_ICON;
extern int DOGROUP_ICON;
extern int GroupIconPos;

struct SingleAI_Desc{
	char* NationID;
	char* Message;
	char* Peasant_ID;
	int NPeas;
	int NLandAI;
	char** LandAI;
	int NWaterAI;
	char** WaterAI;
};
class AI_Description{
public:
	int NAi;
	int NComp;
	SingleAI_Desc* Ai;
	AI_Description();
	~AI_Description();
};
extern AI_Description GlobalAI;

int GetUnitKind( char* Name );
int GetExMedia( char* Name );

extern short AlarmSoundID;

extern int DiffP[4];

extern short WeaponIcn[32];

extern uint8_t WeaponFlags[32];

extern int UnitsPerFarm;
extern int ResPerUnit;
extern int EatenRes;
extern int NEOrders;

struct SingleGroup
{
    int ClassIndex;
    int NCommon;
    int NForms;

    uint8_t * IDX;
    uint16_t * Forms;
};

class FormGroupDescription
{
public:
    int NGrp;
    SingleGroup* Grp;

    FormGroupDescription();
    ~FormGroupDescription();

    void Load( GFILE* f );
};

extern FormGroupDescription FormGrp;

class OrderClassDescription
{
public:
    char* ID;
    const char* Message;

    int IconPos;
    int IconID;

    OrderClassDescription();
    ~OrderClassDescription();
};
extern OrderClassDescription OrderDesc[16];

class OrderDescription
{
public:
    char* ID;

    int NLines;
    short** Lines;
    uint16_t * LineNU;

    int NCom;
    short* ComX;
    short* ComY;
    short YShift;
    int NUnits;

    short BarX0;
    short BarY0;
    short BarX1;
    short BarY1;

    //symmetry groups
    uint16_t * Sym4f;
    uint16_t * Sym4i;
    uint16_t * SymInv;

    //additional parameters
    char AddDamage1;
    char AddShield1;
    char AddDamage2;
    char AddShield2;
    uint8_t GroupID;

    OrderDescription();
    ~OrderDescription();
};

extern OrderDescription ElementaryOrders[128];

extern int NOClasses;

int GetResID( const char* Name );

void ProcessNature();
void ShowRMap();

#endif // NATURE_H
