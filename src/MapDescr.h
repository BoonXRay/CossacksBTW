#ifndef MAPDESCR_H
#define MAPDESCR_H

/*                    Map description
 *
 * This file describes map cells, animations, monsters, buildings,
 * flying monsters, on-water monsters...
 */

#include <cstdint>

#include <stdlib.h>

#pragma pack( push, 1 )

//Duration in ms for various messages and hints
constexpr int kChatMessageDisplayTime = 10000;
constexpr int kImportantMessageDisplayTime = 5000;
constexpr int kSystemMessageDisplayTime = 3000;
constexpr int kMinorMessageDisplayTime = 1500;



 //Catch division by zero by "overloading" div()
 //(in case we will be able to link dynamically sometime, this causes no linking warnings)
div_t __cdecl SecureDivision( int const numerator, int const denominator );
#define div(x,y) SecureDivision(x,y)

typedef unsigned short word;

#include "Icons.h"
#include <stdio.h>

#define NBRANCH 4
#define ULIMIT 65535
#define LULIMIT 65000

#include "gFile.h"

//maximal amount of units in cell
#define MAXINCELL 64

#define SHFCELL 6

struct Coor3D
{
	int x, y, z;
};

typedef void HandlePro( int );

class Weapon;
class SelGroup;
class NewMonster;

#include "Upgrade.h"

//Описание типов объектов(идентификаторы)
#define EmptyID 0

//New monster format
//<ANMNAME> ShiftX ShiftY Rotations FileID NFrames frame1 ... frameN
//SETACTIVE <ANMNAME> ActiveFrame ActivePtX ActivePtY 
class NewFrame
{
public:
	word FileID;
	word SpriteID;
	short dx;
	short dy;
};

class NewAnimation
{
public:
	bool Enabled;

	byte NFrames;
	byte Parts;
	byte PartSize;
	byte ActiveFrame;

	short StartDx;
	short StartDy;
	short* ActivePtX;
	short* ActivePtY;
	short* LineInfo;

	byte Rotations;
	byte TicksPerFrame;
	byte HotFrame;
	NewFrame* Frames;

	short SoundID;

	NewAnimation();
};

#define NAttTypes 4
struct FogRec
{
	word  NWeap;
	int   WProb;
	word* Weap;
};

class AdvCharacter;

class StroiDescription
{
public:
	byte ID;
	word NAmount;
	word* Amount;
	word* LocalID;
	word NUnits;
	word* Units;
};

class OfficerRecord
{
public:
	word BarabanID;
	word NStroi;
	StroiDescription SDES[3];
};

struct Flags3D
{
	int N;
	short Xr;
	short Points[48];
};

struct OneAddSprite
{
	word SpriteID;
	short SortX;
	short SortY;
};

struct OneAddStage
{
	word GPID;

	OneAddSprite Empty;
	OneAddSprite Stage1;
	OneAddSprite Ready;
	OneAddSprite Dead;

	word AddPoints;

	int NExplPoints;
	short* ExplCoor;

	short* FireX[2];
	short* FireY[2];
	short  NFires[2];

	int Cost[8];
};

#define MaxAStages 5
struct ComplexBuilding
{
	byte Mask;
	OneAddStage Stages[MaxAStages];
};

class NewMonster
{
public:
	NewAnimation MotionL;//first part of motion
	NewAnimation MotionR;//second part of motion
	NewAnimation MotionLB;
	NewAnimation MotionRB;
	NewAnimation MiniStepL;
	NewAnimation MiniStepR;
	NewAnimation MiniStepLB;
	NewAnimation MiniStepRB;
	NewAnimation Fist;
	NewAnimation Death;
	NewAnimation DeathLie1;
	NewAnimation DeathLie2;
	NewAnimation DeathLie3;
	NewAnimation Stand;
	NewAnimation Work;
	NewAnimation Trans01;
	NewAnimation Trans10;
	NewAnimation Attack[NAttTypes];
	NewAnimation PAttack[NAttTypes];
	NewAnimation UAttack[NAttTypes];
	NewAnimation PStand[NAttTypes];
	NewAnimation PMotionL[NAttTypes];
	NewAnimation PMotionR[NAttTypes];
	NewAnimation PMotionLB[NAttTypes];
	NewAnimation PMotionRB[NAttTypes];
	NewAnimation PMiniStepL[NAttTypes];
	NewAnimation PMiniStepR[NAttTypes];
	NewAnimation PMiniStepLB[NAttTypes];
	NewAnimation PMiniStepRB[NAttTypes];

	NewAnimation StandHi;
	NewAnimation StandLo;
	NewAnimation Build;
	NewAnimation BuildHi;
	NewAnimation Damage;

	NewAnimation Rest;
	NewAnimation Rest1;
	NewAnimation RotateR;
	NewAnimation RotateL;

	NewAnimation WorkTree;
	NewAnimation WorkStone;
	NewAnimation WorkField;

	NewAnimation TransX3;
	NewAnimation Trans3X;

	byte TransXMask;

	word AttackRadius1[NAttTypes];		//начальный радиус атаки в зависимости от типа
	word AttackRadius2[NAttTypes];		//конечный  радиус атаки в зависимости от типа
	word DetRadius1[NAttTypes];			//Радиус, по которому определяется какой тип атаки использовать
	word DetRadius2[NAttTypes];
	word AttackRadiusAdd[NAttTypes];	//добавка к радиусу атаки при условии, 
										//что юнит в сострянии немедленно атаковать
	Weapon* DamWeap[NAttTypes];			//оружие для поражения(пуля,ядро,...)

	byte Rate[NAttTypes];			//16=x1 rate
	word AttackPause[NAttTypes];
	short AngleUp[NAttTypes];		//64=45degrees,32=arctan(1/2)degrees
	short AngleDn[NAttTypes];
	short MinDamage[NAttTypes];
	short MaxDamage[NAttTypes];
	short DamageRadius[NAttTypes];
	word  DamageDecr[NAttTypes];
	byte  WeaponKind[NAttTypes];
	byte  AttackMask[NAttTypes];

	word  MyIndex;
	short SrcZPoint;				//additional height of the weapon
	short DstZPoint;
	word NLockPt;
	byte* LockX;
	byte* LockY;

	word NSLockPt[MaxAStages];
	byte* SLockX[MaxAStages];
	byte* SLockY[MaxAStages];

	word NBLockPt;
	byte* BLockX;
	byte* BLockY;

	word NCheckPt;
	byte* CheckX;
	byte* CheckY;

	ComplexBuilding* CompxCraft;

    char* Message;
	bool Officer : 1;
	bool Baraban : 1;
	bool Building : 1;
	bool Peasant : 1;
	bool UnitAbsorber : 1;
	bool PeasantAbsorber : 1;
	bool Transport : 1;
	bool Producer : 1;
	bool SpriteObject : 1;
	bool Wall : 1;
	bool RiseUp : 1;
	bool SelfProduce : 1;
	bool WaterActive : 1;
	bool TwoParts : 1;
	bool Port : 1;
	bool Farm : 1;
	bool ShowDelay : 1;
	bool Capture : 1;
	bool NotHungry : 1;
	bool ShotForward : 1;
	bool Artilery : 1;
	bool Rinok : 1;
	bool SlowDeath : 1;
	bool AutoNoAttack : 1;
	bool AutoStandGround : 1;
	bool AttBuild : 1;
	bool CanStandGr : 1;
	bool Priest : 1;
	bool ResSubst : 1;
	bool Archer : 1;
	bool ArtDepo : 1;
	bool Artpodgotovka : 1;
	bool CanBeKilledInside : 1;
	bool CanBeCapturedWhenFree : 1;
	bool CanShoot : 1;
	bool CanStorm : 1;
	bool NoDestruct : 1;
	bool SlowRecharge : 1;

	byte NInArtDepot;
	byte NInFarm;
	byte ArtSet;
	byte TransMask[MaxAStages];
	NewAnimation* UpperPart;
	short UpperPartShift;
	NewAnimation* BuiAnm;
	byte ArtCap[4];
	short BuiDist;
	short MaxPortDist;
	byte NRiseFrames;
	byte RiseStep;
	byte ProdType;
	byte SelfProduceStep;
	word FreeAdd;
	word PeasantAdd;
	word MaxInside;
	byte ResConcentrator;
	byte MaxZalp;
	byte NoWaitMask;
	byte UnitRadius;
	int Ves;

	short AddShotRadius;
	byte  PromaxPercent;

	word NBuildPt;
	char* BuildPtX;
	char* BuildPtY;

	word NConcPt;
	char* ConcPtX;
	char* ConcPtY;

	word NDamPt;
	char* DamPtX;
	char* DamPtY;

	word NBornPt;
	char* BornPtX;
	char* BornPtY;

	word NShotPt;
	short* ShotPtX;
	short* ShotPtY;
	short* ShotPtZ;
	byte* ShotDir;
	byte* ShotDiff;
	word* ShotMinR;
	word* ShotMaxR;

	short* FireX[2];
	short* FireY[2];
	short  NFires[2];
	byte MaxResPortion[8];
	int  NeedRes[8];
	int CenterMX;
	int CenterMY;
	int BRadius;
	word ProduceStages;
	word IconFileID;
	word IconID;
	int Radius1;
	int Radius2;
	int MotionDist;
	int OneStepDX[256];
	int OneStepDY[256];
	int POneStepDX[256];
	int POneStepDY[256];
	int PicDx, PicDy, PicLx, PicLy;
	short BuildX0, BuildY0, BuildX1, BuildY1;
	byte KillMask;
	byte MathMask;

	//rectangle around the monster
	int RectLx, RectLy, RectDx, RectDy;

	//Monster characteristics
	word Res1cost, Res2cost, Res3cost;
	word Life, Shield;
	word AttRange, VisRange;
	word MinAttRange, MaxAttRange, MaxNearRange;
	word AttType;
	word Time, Kind;
	short AnmUpShift;
	short* Bars3D;
	short  NBars;
	char* Name;

	//fogging & fire
	FogRec Fogging;
	FogRec Destruct;
	FogRec Fire;
	word Protection[16];
	word Sprite;
	word SpriteVisual;
	byte ExplosionMedia;
	byte EMediaRadius;
	byte LockType;		//0-Land,1-Water
	byte MotionStyle;	//0-Soldiers,1-Cavalery,2-Sheeps,3-Fly
	NewAnimation* Veslo;
	NewAnimation* Reflection;
	byte VisionType;
	short VesloRotX;
	short VesloRotY;
	byte NVesel;
	short* VesloX;
	short* VesloY;
	short LinearLength;
	short* MultiWpX;
	short* MultiWpY;
	short NMultiWp;
	short MultiWpZ;
	short ResEff;
	short SelectSoundID;
	short AttackSoundID;
	short BornSoundID;
	short OrderSoundID;
	short DeathSoundID;
	short StrikeSoundID;

	short* HideTriX;
	short* HideTriY;
	short NHideTri;

	word  Razbros;
	word  ExplRadius;

	word ResConsumer;
	byte ResConsID;

	byte MaxAIndex;
	byte Behavior;
	byte ResAttType;
	byte ResAttType1;
	byte NShotRes;
	word* ShotRes;
	word CostPercent;
	byte VesStart;
	byte VesFin;
	byte MinRotator;
	byte FishSpeed;
	word FishAmount;
	byte InfType;
	word PictureID;
	byte Force;//for AI
	//for strongholds siege
	byte MinOposit;
	byte MaxOposit;
	byte StormForce;

	short* WavePoints;
	byte NWaves;
	char WaveDZ;
	byte Usage;
	byte BattleForce;
	Flags3D* FLAGS;
	char* MD_File;
	NewMonster();
	bool CreateFromFile( char* Name );
	NewAnimation* LoadNewAnimationByName( char* Name );
	AdvCharacter* AdvChar;
};

class AdvCharacter
{
public:
	word		AttackRadius1[NAttTypes];
	word		AttackRadius2[NAttTypes];
	word		DetRadius1[NAttTypes];
	word		DetRadius2[NAttTypes];
	word		AttackPause[NAttTypes];
	short		MaxDamage[NAttTypes];
	byte		WeaponKind[NAttTypes];
	byte		Rate[NAttTypes];
	byte		Protection[16];
	int			NeedRes[8];
	word		MaxInside;
	word		ProduceStages;
	word		Life;
	word		Shield;
	byte		FishSpeed;
	word		FishAmount;
	word        Razbros;

	word        MinR_Attack;
	word		MaxR_Attack;
	word        MaxDam;
	short       ResEff;

	bool		Changed;
};

class NewUpgrade
{
public:
	//information
	char* Name;
	char* Message;
	byte Level;
	byte Branch;
	word IconFileID;
	word IconSpriteID;
	char IconPosition;
	byte NatID;
	word  Cost[8];
	byte CtgUpgrade;
	byte CtgType;
	union
	{
		int  NCtg;
		int  CtgValue;
	};
	word*    CtgGroup;
	byte UnitType;
	union
	{
		int NUnits;
		int UnitValue;
	};
	word* UnitGroup;
	byte ValueType;
	union
	{
		int  Value;
		int  NValues;
	};
	int* ValGroup;

	//mechanics
	bool Done : 1;
	bool Enabled : 1;
	bool PermanentEnabled : 1;
	bool IsDoing : 1;
	bool Gray : 1;
	bool Individual : 1;
	bool ManualDisable : 1;
	bool StageUp : 1;
	word NStages;
	word CurStage;
	byte NAutoPerform;
	byte NAutoEnable;
	word* AutoPerform;
	word* AutoEnable;
	byte Options;
};

//Описание базового типа, общего для всех объектов
class GeneralObject
{
public:
    char * Message;

	bool NoSearchVictim : 1;
	bool Enabled : 1;
	bool CondEnabled : 1;
	bool Transport : 1;
	int Options : 1;
	bool WATT : 1;
	bool AATT : 1;
	bool P100 : 1;
	bool T3X3 : 1;
	bool FWEAP : 1;//огненное оружие
	bool AGold : 1;
	bool AWood : 1;
	bool Submarine : 1;
	bool RefreshLife : 1;
	bool CanFly : 1;
	bool CanAttWall : 1;
	bool CanFear : 1;
	bool canNucAttack : 1;
	bool AntiNuc : 1;
	bool UFO : 1;
	bool UFOTrans : 1;
	bool CanRepair : 1;
	bool ExtMenu : 1;
	bool CanDest : 1;
	bool ManualDisable : 1;
	byte NatID;
	byte Country;
	OfficerRecord* OFCR;
	NewMonster* newMons;
	char* MonsterID;
	word MagAgainst;
	word MagWith;
	word IDforUFO;
	byte Kind;
	word MaxAutoAmount;
	word Useful[NBRANCH];//полезность объекта для каждой из отраслей
	word SpUsef[NBRANCH];//удельная полезность
	word AbsNeedPrio;
	word AbsNeedCount;
	word LifeShotLost;
	int	 cost;
	word delay;
	byte WepSpeed;
	byte WepDelay;
	word NUpgrades;
	word* Upg;
	char Wdx[8];
	char Wdy[8];
	byte NIcons;
	word* IRefs;
	word NWeap;
	byte NStages;
	Weapon* MWeap[12];
	byte SWPIndex[12];
	short HitSound;
	short ClickSound;
	short OrderSound;
	short DeathSound;
	short BornSound;
	short AttackSound;
	short TreeSound;
	short GoldSound;
	word LockID;
	byte NLockUnits;
	byte Branch;

	byte SizeX;
	byte SizeY;

	byte ResourceID[4];		//Индекс необходимого ресурса
	word ResAmount[4];		//Сколько необходимо для постройки 
	word NAnm;				//Количество доступных анимаций
    AdvCharacter* MoreCharacter;

    void CloseGO()
    {
        if (NUpgrades)free( Upg );
        if (NIcons)free( IRefs );
        free( MoreCharacter );
        free( MonsterID );
    }
};

class Visuals : public GeneralObject
{
};

//Upgrade-замена одного типа объектов на другой
class ObjectUpgrade : public GeneralObject
{
public:
	word SourceTypeIndex[8];
	word FinalTypeIndex[8];
};

//Upgrade - для Visuals-улучшение параметров для группы объектов;
class ParameterUpgrade : public GeneralObject
{
	word ObjectIndex[8];
	byte ChangedParameter[8];
	byte AdditionalValue[8];
};

class OneObject;
typedef void ReportFn( OneObject* Sender );

//Описание командного блока
//Приказы 1-го уровня
struct Order1
{
	//Указатель на отложенный приказ, если NULL то нет 
	//отложенного приказа 
	Order1* NextOrder;

	//Уровень приоритета выполнения команды
	//0-нижайший уровень приоритета
	//Данное задание может быть отложено только в том случае,
	//если приоритет прерывания выше приоритета выполнения
	byte PrioryLevel;
	byte OrderType;
	byte OrderTime;//=0 if very fast
	//=1 - стоять на месте 
	//=2 - перейти в точку с координатами (x,y)
	//=3 - следовать за объектом (obj)
	//=4 - преследовать и атаковать объект (obj)
	//=5 - построить объект
	//=6 - создать объект
	//=7 - патрулировать

	//Информация по каждому типу приказов
	ReportFn* DoLink;
	union
	{
		struct
		{
			byte VisibilityRadius;
		} Stand;

		struct
		{
			int x;
			int y;
			word PrevDist;
			byte Times;
		} MoveToXY;

		struct
		{
			byte xd;
			byte yd;
			byte time;
			word BuildID;
			word BSN;
		} UFO;

		struct
		{
			word ox;
			word oy;
			word x, y, z;
			byte wep;
		} AttackXY;

		struct
		{
			word ObjIndex;
			word SN;
			word PrevDist;
			byte wep;
		} MoveToObj;

		struct
		{
			int ObjIndex;
			word SN;
			short ObjX;
			short ObjY;
			byte AttMethod;
		} BuildObj;

		struct
		{
			word x;
			word y;
			word x1;
			word y1;
			byte dir;
		} Patrol;

		struct
		{
			word ObjIndex;
			word Progress;
			word NStages;
			word ID;
			byte PStep;
			word Power;
		} Produce;

		struct
		{
			word OldUpgrade;
			word NewUpgrade;
			word Stage;
			word NStages;
		} PUpgrade;

		struct
		{
			byte dir;
		} MoveFrom;

		struct
		{
			int x;
			int y;
			int SprObj;
			byte ResID;
		} TakeRes;

		struct
		{
			short LockX;
			short LockY;
			short EndX;
			short EndY;
		} DelBlock;

		struct
		{
			word x, y;
			short dx, dy;
			word NextX, NextY, NextTop;
		} SmartSend;

	} info;
};

class GOrder;
struct GlobalIconInfo
{
	HandlePro* HPLeft;
	HandlePro* HPRight;
	int LParam;
	int RParam;
	int IconSpriteID;
};

typedef bool GOrderFn( OneObject* OB, GOrder* GOR, int LParam, int RParam );

class GOrder
{
public:
	GOrderFn* CheckDisconnectionAbility;
	GOrderFn* Disconnect;
    void*     Data;
	GOrder();
	~GOrder();
};

class Legion;
class Nation;

class FireObjectInfo
{
public:
	byte* FireSprite;
	byte* FireStage;
	byte delay;
	short NeedFires;
	short RealFires;
	void  Erase();
};

class FireInfo
{
public:
	byte BStage;
	FireObjectInfo* Objs[2];
	void Erase();
};

//Информация для каждого индивидуума
class OneObject
{
public:
	Nation* Nat;
	union
	{
		GeneralObject* General;
		Visuals* Visual;
		ObjectUpgrade* OUpgrade;
		ParameterUpgrade* PUpgrade;
	} Ref;
	NewMonster* newMons;
	//-----------new path algoritm variables----------------
	short* PathX;
	short* PathY;
	word Index;//номер этого монстра
//--------------------------------------------//
//-----------begin of saving cluster----------//
//--------------------------------------------//
	bool NeedPath;
	word CPdestX;
	word CPdestY;
	word NIPoints;
	word CurIPoint;
	word NIndex;	//индекс в национальном списке
	byte Selected;
	byte ImSelected;
	bool SelectedTemp : 1;
	bool Borg : 1;
	bool Invert : 1;
	bool Attack : 1;
	bool NoMotion : 1;		//не сдвигаться ни прт каких условиях,кроме приказа по армии
	bool NoFire : 1;		//не открывать огонь по врагу и не привлекать внимания
	bool NoInitiate : 1;	//не инициировать сражение
	bool WasInMobilZone : 1;//Был в зоне мобилизации
	bool TempFlag : 1;		//временный флаг, используемый для сортировки отряда
	bool Mobilised : 1;
	bool MoveInRect : 1;	//установлен, если среди риказов есть приказ о перемещении группы
	bool DrawUp : 1;		//СТРРPPОИТЬСЯ!!!
	bool PathBroken : 1;	//Если хоть раз была преграда на пути
	bool capBuilding : 1;
	bool capBuild : 1;
	bool capBase : 1;
	bool Ready : 1;
	bool NoSearchVictim : 1;
	bool AskMade : 1;
	bool SafeWall : 1;
	bool StandGround : 1;
	bool Invisible : 1;
	bool Zombi : 1;

	bool Transport : 1;
	bool Absent : 1;
	bool InFire : 1;
	bool RefreshLife : 1;
	bool DoWalls : 1;
	bool Use_AI : 1;
	bool AntiNuc : 1;

	bool NewMonst : 1;
	bool Collision : 1;
	bool LeftLeg : 1;
	//--------------New
	bool InMotion : 1;	//Выполняется перемещениe(New)
	bool BackMotion : 1;//шаг назад
	bool HasStatePA : 1;
	bool NewBuilding : 1;
	bool Hidden;
	bool Wall : 1;
	bool UnlimitedMotion : 1;
	bool LLock : 1;
	bool GLock : 1;
	bool AlwaysLock : 1;
	bool InternalLock : 1;
	bool TurnOff : 1;
	bool InArmy : 1;
	bool InPatrol : 1;

	//------------------for AI----------------
	bool NoBuilder : 1;
	bool DoNotCall : 1;
	bool AutoKill : 1;

	//-----------For best motion--------------
	short bm_DestX;
	short bm_DestY;
	short bm_NextX;
	short bm_NextY;
	short bm_dx;
	short bm_dy;
	short bm_PrevTopDst;
	word Guard;
	word bm_NextTop;
	//----------------------------------------

	byte NZalp;
	byte RotCntr;
	int NextForceX;
	int NextForceY;
	int BestNX;
	int BestNY;

	char OverEarth;
	word Kills;
	byte LockType;//0-Land,1-Water
	byte NothingTime;
	word BlockInfo;
	word Sdoxlo;
	byte BackSteps;
	byte BackReserv;
	byte NewState;//0-Normal,1-prepare to attack
	byte LocalNewState;
	byte LeftVeslo;
	byte RightVeslo;
	byte Usage;
	char AddDamage;
	char AddShield;

	//------------Extended animation-----------//
	byte CurAIndex;
	byte MaxAIndex;
	byte MoveStage;
	//-----------------End New-----------------//

	byte BackDelay;

	byte AbRes;

	short WallX;
	short WallY;

	word NUstages;
	word Ustage;
	word Serial;
	byte Kind;
	short Lx;
	short Ly;

	word Life;
	word MaxLife;
	int x;
	int y;
	int DstX;
	int DstY;

	word delay;
	byte PathDelay;
	word MaxDelay;

	byte StandTime;
	byte NStages;
	word Stage;
	byte NNUM;

	word EnemyID;
	word EnemySN;
	byte NMask;
	byte RStage;
	byte RType;
	byte RAmount;
	word NearBase;

	word BrigadeID;
	word BrIndex;
	byte Media;//=0-terrain,=1-on water,=2-on air
	word AddInside;
	byte PersonalDelay;
	short RZ;

	//Flying objects only:
	int  RealX;//1 pixel=16 length units
	int  RealY;
	int  DestX;
	int  DestY;
	int  RealVx;
	int  RealVy;
	int  BestDist;
	int  BestHeight;
	int	 Height;
	int  ForceX;
	int  ForceY;
	int  Radius1;
	int  Radius2;
	int  Speed;
	byte GroundState;
	byte SingleUpgLevel;
	byte RealDir;
	byte GraphDir;
	byte NewCurSprite;
	byte MotionDir;
	byte PrioryLevel;
	word NInside;
	//-----------------------------------------------//
	//-------------end of saving cluster-------------//
	//-----------------------------------------------//

	word* Inside;

	//for all objects
	SelGroup* GroupIndex;
	FireInfo* IFire;
	//Указатель на текущий приказ 1-го уровня
	//Типы приказов:
	//	Перейти(x,y)->(x1,y1) по оптимальному пути;
	//	Атаковать объект(obj_index)- удаленный или близкий
	//	Умереть
	//	Создать экземпляр объекта(obj)...
	Order1* LocalOrder;
	GOrder* GlobalOrder;
	//Weapon* Weap;
	//New animation settings
	NewAnimation* HiLayer;
	NewAnimation* NewAnm;
	NewAnimation* LoLayer;

	//Methods declaration
	void DefaultSettings( GeneralObject* GO );
	void NewMonsterSendTo( int x, int y, byte Prio, byte OrdType );
	void NewMonsterPreciseSendTo( int x, int y, byte Prio, byte OrdType );
	void NewMonsterSmartSendTo( int x, int y, int dx, int dy, byte Prio, byte OrdType );
	void MakeProcess();
	void MakePreProcess();
	void WSendTo( int x2, int y2, int Prio );
	bool AttackObj( word OID, int Prio );
	bool AttackObj( word OID, int Prio, byte OrdType );
	bool AttackObj( word OID, int Prio, byte OrdType, word NTimes );
	void WAttackObj( word OID, int Prio );
	void FlyAttack( word OID, byte Prio );
	void AttackPoint( byte x, byte y, byte wep, int Prio );
	bool AttackPoint( int x, int y, int z, byte Times, byte Flags, byte OrdType );
	bool NewAttackPoint( int x, int y, int Prio1, byte OrdType, word NTimes );
	void ProcessFly();
	void CreatePath( int x1, int y1 );
	void CreateSimplePath( int x1, int y1 );
	bool CreatePrePath( int x1, int y1 );
	bool CreatePrePath2( int x1, int y1 );
	void FreeAsmLink();
	void Die();
	void Eliminate();
	void MakeDamage( int Fundam, int Persist, OneObject* Sender );
	void MakeDamage( int Fundam, int Persist, OneObject* Sender, byte AttType );
	void SearchVictim();
	void FreeOrdBlock( Order1* p );
	void ClearOrders();
	void ProcessMotion();
	void ProcessAttackMotion();
	void SendInGroup( byte tx, byte ty, byte x0, byte y0, byte x1, byte y1 );
	void NextStage();
	bool BuildObj( word OID, int Prio, bool LockPoint, byte OrdType );
	int CheckAbility( word ID, const bool running_production );
	void Produce( word ID );
	void Produce( word ID, word GroupID );
	bool BuildWall( int xx, int yy, byte Prio, byte OrdType, bool TempBlock );
	bool DamageWall( word OID, int Prio );
	void PerformUpgrade( word NewU, word MakerID );
	void SetDstPoint( int x, int y );
	void Patrol( int x1, int y1, int x2, int y2, byte prio );
    void ContinueAttackPoint( byte x, byte y, int Prio );
	void ContinueAttackWall( byte x, byte y, int Prio );
	void MakeMeUFO();
    void MakeMeSit();
	//-------New---------
	void BlockUnit();
	void WeakBlockUnit();
	void UnBlockUnit();
	void DeletePath();
	void CheckState();
	bool CheckLocking( int dx, int dy );
	void SetDestCoor( int x, int y );
	void EscapeLocking();
	bool FindPoint( int* x1, int* y1, byte Flags );
	void ClearBuildPt();
	bool CheckBlocking();
	void DeleteBlocking();

	//returns points for damage
	bool GetDamagePoint( Coor3D* dp, int Precise );

	void TakeResourceFromSprite( int SID );
	void SetOrderedUnlimitedMotion( byte OrdType );
	void ClearOrderedUnlimitedMotion( byte OrdType );
	void ClearOrderedUnlimitedMotion( byte OrdType, word GroupID );
	int TakeResource( int x, int y, byte ResID, int Prio, byte OrdType );
	word FindNearestBase();

	//Type:
	//0 - single order (previous orders will be erased)
	//1 - add order to the head of link
	//2 - add order to the tile of link
	Order1* CreateOrder( byte Type );

	void DeleteLastOrder();
	void GetCornerXY( int* x, int* y );
	bool GoToMine( word ID, byte Prio );
	bool GoToMine( word ID, byte Prio, byte Type );
	bool GoToTransport( word ID, byte Prio );
	void LeaveMine( word Type );
	void LeaveTransport( word Type );
	void HideMe();
	void ShowMe();
	bool CheckOrderAbility( int LParam, int RParam );
	bool CheckOrderAbility();
	void GlobLock();
	void GlobUnlock();
	void Fishing();

	inline int DistTo( int xx, int yy )
	{
		// BoonXRay 06.08.2017
		//__asm
		//{
		//	mov		eax, xx
		//	mov		ebx, this
		//	mov		edx, [ebx]this.x
		//	sub		eax, edx
		//	jge		uui
		//	neg		eax
		//	uui : mov		ecx, yy
		//		  mov		edx, [ebx]this.y
		//		  sub		ecx, edx
		//		  jge		uux
		//		  neg		ecx
		//		  uux : cmp		ecx, eax
		//				jl		uuz
		//				mov		eax, ecx
		//				uuz :
		//}
		//__asm
		{
			//mov		eax, xx
			int TmpEAX = xx;
			//mov		ebx, this
			//mov		edx, [ebx]this.x
			int TmpEDX = this->x;
			//sub		eax, edx
			bool flag1 = TmpEAX >= TmpEDX;
			TmpEAX -= TmpEDX;
			//jge		uui
			if (flag1) goto uui;
			//neg		eax
			TmpEAX *= -1;
			int TmpECX;
		uui :
			//mov		ecx, yy
			TmpECX = yy;
			//mov		edx, [ebx]this.y
			TmpEDX = this->y;
			//sub		ecx, edx
			bool flag2 = TmpECX >= TmpEDX;
			TmpECX -= TmpEDX;
			//jge		uux
			if (flag2) goto uux;
			//neg		ecx
			TmpECX *= -1;
		uux :
			//cmp		ecx, eax
			//jl		uuz
			//mov		eax, ecx
			if (TmpECX < TmpEAX) goto uuz;
			TmpEAX = TmpECX;
		uuz:
			return TmpEAX;
		}
    }

	void CloseObject();
};

//Описание оружия
class Nation;
class ChildWeapon
{
public:
	Weapon* Child[4];
	byte    NChildWeapon;
	byte    MaxChild;
	byte    MinChild;
	byte    Type;
	void InitChild();
};

class Weapon
{
public:
	NewAnimation* NewAnm;
	ChildWeapon Default;
	ChildWeapon* CustomEx;
	byte    NCustomEx;
	Weapon* ShadowWeapon;
	Weapon* TileWeapon[4];
	word    TileProbability;
	byte	NTileWeapon;
	byte    MinChild;
	byte    MaxChild;
	byte    HotFrame;
	Weapon* SyncWeapon[4];
	byte	NSyncWeapon;
	char    dz;
	//see scripts
	short   Damage;
	short   Radius;
	short   Speed;
	short   Times;
	byte    Transparency;
	byte    Gravity;
	byte    Propagation;
	bool    FullParent : 1;
	bool    HiLayer : 1;
	short	SoundID;
	word MyIndex;
};

//Описание элемента анимации(оружие в действии)
class AnmObject
{
public:
	NewAnimation* NewAnm;
	int x, y, z;//координаты
	int vx, vy, vz;//скорости
	int az;//ускорение
	int xd, yd, zd;//точка назначения
	short Frame;
	short NTimes;
	Weapon* Weap;
	word    Damage;
	OneObject* Sender;
	word ASerial;
	word DestObj;
	word DestSN;
	byte AttType;
	char WeaponKind;
};

class City;

struct sAI_Req
{
	byte Kind;//0-unit,1-upgrade,2-group
	word ObjID;
	word Amount;//if upgrade:1-Done 2-Enabled
};

struct sAI_Devlp
{
	byte Kind;//0-unit,1-upgrade
	byte Source;//0-general,1-army,2-selo,3-science
	byte ConKind;//0-unit,2-group
	word ObjID;
	word ConID;
	word Amount;
	word GoldPercent;
	word AtnPercent;
};

struct sAI_Cmd
{
	byte Kind;//1-army,2-selo,3-science
	word Info[8];
};

class Branch
{
public:
	int  RESAM[8];
	word RESP[8];
	int  RESRM[8];
	void AddTo( byte ResID, int Amount );
	void AddPerc( byte ResID, int Amount, int perc );
	void AddEntire( byte ResID, int Amount );
    void Init();
	int GetMonsterCostPercent( byte NI, word NIndex );
	int GetUpgradeCostPercent( byte NI, word NIndex );
};

//Описание нации в целом
struct U_Grp
{
	word N;
	word* UIDS;
	word* UVAL;
};

typedef void VoidProc();

class Nation
{
public:
	char SCRIPT[16];
	int NMon;
	bool GoldBunt : 1;
	GeneralObject* Mon[2048];
	word NKilled[2048];
	word NProduced[2048];
	byte SoundMask[2048];
	byte VictState;//0-? 1-defeat 2-victory

	//----Resource control-------
	int ResTotal[8];
	int ResOnUpgrade[8];
	int ResOnMines[8];
	int ResOnUnits[8];
	int ResOnBuildings[8];
	int ResOnLife[8];
	int ResBuy[8];
	int ResSell[8];
	//---------------------------

	City* CITY;
	int NGidot;
	int NFarms;
	word NArtdep;
	word NArtUnits[4];
	word* PAble[2048];
	word PACount[2048];
	char* AIndex[2048];
	char* AHotKey[2048];
	int BranchPercent[NBRANCH];
	int NUpgrades;
	NewUpgrade* UPGRADE[4096];
	int NOct;
	int NSlides;

	//AI Statements
	int CasheSize;
	int TAX_PERCENT;
	int CASH_PUSH_PROBABILITY;
	int NationalAI;//0..32768-determines speed of development
	int AI_Level_MIN;
	int AI_Level_MAX;
	int AI_Forward;
	short DangerSound;
	short VictorySound;
	short ConstructSound;
	short BuildDieSound;
	short UnitDieSound;
	word LastAttackTime;
	word Harch;
	word NLmenus;
	word* Lmenus;
	word NAmenus;
	word* Amenus;
	word NWmenus;
	word* Wmenus;
	word NCmenus;
	word* Cmenus;
	word NNeed;
    int ResRem[8];
	int ResSpeed[8];

	byte NNUM;
	int  NFinf;
	byte palette[256];
	byte NMask;
	word NIcons;
	WIcon* wIcons[1024];
	word NCOND;
	word CLSize[4096];//Access controlling
	word* CLRef[4096];

	//Strange weapon prameters
	word SWRange[256];

	//-------------NEW AI--------------
	word NGrp; //Groups of types definition
	word GRSize[32];
	word* GRRef[32];
	word  GAmount[32];//Result of calculation
	word N_AI_Levels;
	word N_AI_Req[256];
	word N_AI_Devlp[256];
	word N_AI_Cmd[256];
	sAI_Req* AI_Req[256];
	sAI_Devlp* AI_Devlp[256];
	sAI_Cmd* AI_Cmd[256];
	word AI_Level;
	word NPBal;
	word* PBalance;
	word NMineBL;
	word* PBL;

	int POnFood;
	int POnWood;
	int POnStone;

    const char* DLLName;
	VoidProc* ProcessAIinDLL;
	HINSTANCE hLibAI;

	//------------------SHAR----------------//StartSave
	byte SharStage;
	int SearchRadius;
	int SharX;
	int SharY;
	int SharZ;
	int SharVx;
	int SharVy;
	int SharVz;
	int SharAx;
	int SharAy;
	int SharAz;
	bool Vision : 1;
	bool SharAllowed : 1;
	bool SharPlaceFound : 1;
	bool AI_Enabled : 1;
	//---------Upgradable properties--------//
	word FoodEff;
	word WoodEff;
	word StoneEff;
	bool Geology;
	//---------------Constants--------------//
	word UID_PEASANT;//EndSave
	word UID_WALL;
	word UID_MINE;
	word UID_HOUSE;

	U_Grp UGRP_MINEUP;

	word  MINE_CAPTURE_RADIUS;
	word  MINE_UPGRADE1_RADIUS;
	word  MINE_UPGRADE2_RADIUS;
	word  DEFAULT_MAX_WORKERS;
	word  MIN_PBRIG;

	word  MU1G_PERCENT[3];
	word  MU1I_PERCENT[3];
	word  MU1C_PERCENT[3];

	word  MU2G_PERCENT[3];
	word  MU2I_PERCENT[3];
	word  MU2C_PERCENT[3];

	word  MU3G_PERCENT[3];
	word  MU3I_PERCENT[3];
	word  MU3C_PERCENT[3];
	//--------------------------------------

	char** History;
	int NHistory;

	//-----------------XRONIKA--------------
	byte ThereWasUnit;
	int NPopul;
	int MaxPopul;
	word* Popul;

	int NAccount;
	int MaxAccount;
	word* Account;

	int NUpgMade;
	int MaxUpgMade;
	word* UpgIDS;

	int*  UpgTime;
	void AddUpgrade( word ID, int time );
	void AddPopul( word N );

	//---------------NEW resource-----------
	Branch SELO;
	Branch ARMY;
	Branch SCIENCE;
	Branch GENERAL;
	//----------------------choose unit menu
	char***UnitNames;
	int*   NUnits;
	word** UnitsIDS;
	word FormUnitID;
	//---------------------------------

	void CreateNation( byte NMask, byte NIndex );
	int  CreateNewMonsterAt( int x, int y, int n, bool Anyway );
	void AssignWeapon( Weapon* Wpn, int i );
	int CreateBuilding( word ID, byte x, byte y );
	bool CheckBuilding( word ID, byte x, byte y );
	void GetUpgradeCostString( char* st, word UI );
	void CloseNation();
	void AddResource( byte Rid, int Amount );
	void ControlProduce( byte Branch, byte ResID, int Amount );
};

typedef char** lplpCHAR;
typedef char*  lpCHAR;
typedef int*   lpINT;

class SelGroup
{
public:
    uint16_t * Member;
    uint16_t * SerialN;
    uint16_t NMemb;
	bool CanMove : 1;
	bool CanSearchVictim : 1;
	bool CanHelpToFriend : 1;
	bool Egoizm : 1;
	SelGroup();
    void CreateFromSelection( uint8_t NI );
    void SelectMembers( uint8_t NI, bool Shift );
	void DeleteMembers();
    void ImSelectMembers( uint8_t NI, bool Shift );
};

//Массив все монстров на карте
#define MaxObj ULIMIT

//Получить блок для LocalOrder
inline Order1* GetOrdBlock()
{
    Order1* OR1 = new Order1;
    memset( OR1, 0, sizeof( Order1) );
    return OR1;
}

typedef NewAnimation* lpNewAnimation;

//------------sorting by nations-------------

//------------------IDS-----------------//
#define MelnicaID	0x01
#define MelnicaIDS  "MELNICA"

#define FarmID		0x02
#define FarmIDS		"FARM"

#define CenterID	0x03
#define CenterIDS	"CENTER"

#define SkladID		0x04
#define SkladIDS	"SKLAD"

#define TowerID		0x05
#define TowerIDS	"TOWER"

#define FieldID		0x06
#define FieldIDS	"FIELD"

#define MineID		0x07
#define MineIDS		"MINE"

#define FastHorseID	0x08
#define FastHorseIDS "FASTHORSE"

#define MortiraID	0x09
#define MortiraIDS  "MORTIRA"

#define PushkaID	0x0A
#define PushkaIDS   "PUSHKA"

#define GrenaderID  0x0B
#define GrenaderIDS "GRENADER"

#define HardWallID  0x0C
#define HardWallIDS "HARDWALL"

#define WeakWallID  0x0D
#define WeakWallIDS "WEAKWALL"

#define LinkorID	0x0E
#define LinkorIDS	"LINKOR"

#define WeakID		0x0F
#define WeakIDS	    "WEAK"

#define FisherID	0x10
#define FisherIDS	"FISHER"

#define ArtDepoID	0x11
#define ArtDepoIDS  "ARTDEPO"

#define SupMortID	0x12
#define SupMortIDS	"SUPERMORTIRA"

#define PortID		0x13
#define PortIDS	    "PORT"

#define LightInfID	0x14
#define LightInfIDS	"LIGHTINFANTRY"

#define StrelokID	0x15
#define StrelokIDS	"STRELOK"

#define HardHorceID	0x16
#define HardHorceIDS "HARDHORCE"

#define PeasantID	0x17
#define PeasantIDS	"PEASANT"

#define HorseStrelokID	0x18
#define HorseStrelokIDS "HORSE-STRELOK"

#define FregatID	0x19
#define FregatIDS   "FREGAT"

#define GaleraID	0x1B
#define GaleraIDS   "GALERA"

#define IaxtaID	    0x1C
#define IaxtaIDS    "IAXTA"

#define ShebekaID	 0x1E
#define ShebekaIDS   "SHEBEKA"

#define ParomID      0x1F
#define ParomIDS     "PAROM"

#define ArcherID    0x20
#define ArcherIDS   "ARCHER"

#define MultiCannonID 0x1E
#define MultiCannonIDS "MCANNON"

inline int OScale( int x )
{
    return ( x * 256 ) >> 8;
}

#define MobilR 1024

typedef DWORD DPID1, FAR *LPDPID;

//-----------------New text files------------------//

#define GM(x) (1<<x)
#define INITBEST 0x0FFFFFFF

#pragma pack(pop)

#endif // MAPDESCR_H
