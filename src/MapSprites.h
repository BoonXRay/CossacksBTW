#ifndef MAPSPRITES_H
#define MAPSPRITES_H

#include <cstdint>

#include "LoadSave.h"

//#define sprNx  (64<<ADDSH)
#define SprShf (6+ADDSH)
extern int MaxSprt;

class NewAnimation;

class ObjCharacter{
public:
    uint8_t ResType;//0-wood,1-gold,2-stone,3-food,0xFE-no resource,0xFF-removable
    uint8_t WorkRadius;
    uint8_t ResPerWork;
	int WorkAmount;
    uint16_t WNextObj;
    uint16_t DamageAmount;
    uint16_t DNextObj;
    uint16_t TimeAmount;
    uint16_t TNextObj;
    uint8_t IntResType;
    uint16_t IntResPerWork;
    uint16_t IntWorkRadius;
    NewAnimation* Stand;
    NewAnimation* Shadow;
    short Z0;
    short DZ;
    uint8_t Parts;
    uint8_t Delay;
    uint8_t Frames;
    uint8_t Discret;
    uint8_t LockRadius;
    uint16_t NLockPt;
	short* LockX;
	short* LockY;

	char* Name;
};

class SprGroup{
public:
	int  NSpr;
    uint16_t FileID;
    uint16_t * Items;
	short* Dx;
	short* Dy;
	short* Radius;
	ObjCharacter* ObjChar;
    void LoadSprites(const char* fname);
    int GetIndexByName(const char *Name);
};

class OneSprite{
public:
	bool Enabled:1;
	bool Surrounded:1;
	int x;
	int y;
	short z;
    uint16_t Radius;
	SprGroup* SG;
	ObjCharacter* OC;
	int Index;
    uint16_t SGIndex;
    uint8_t WorkOver;
    uint16_t TimePassed;
    uint8_t Damage;
    int PerformWork(uint16_t);
    int PerformIntWork(int work);
};

class TimeReq{
public:
    int NMembers;
    int MaxMembers;
    int* IDS;
    uint8_t * Kinds;
    TimeReq();
    ~TimeReq();
    void Handle();
    void Add(int ID,uint8_t Kind);
    void Del(int ID,uint8_t Kind);
};

extern TimeReq ObjTimer;
extern uint8_t * NSpri;
extern int** SpRefs;

extern int MAXSPR;

extern OneSprite* Sprites;
void InitSprites();
void addSprite(int x,int y,SprGroup* SG,word id);
int GetHeight(int x,int y);
void PreShowSprites();
extern OneSprite* Sprites;
extern SprGroup TREES;
extern SprGroup STONES;
extern SprGroup HOLES;
extern SprGroup COMPLEX;

extern int LastSpriteIndex;

void ProcessSprites();
uint8_t DetermineResource(int x,int y);
bool CheckDist(int x,int y,word r);
void HideFlags();

int GetUnitHeight(int x,int y);

extern bool SpriteSuccess;
extern int LastAddSpr;
void EraseSprite( int Index );
void addSpriteAnyway( int x, int y, SprGroup* SG, word id );

void UnregisterSprite( int N );
void DeleteAllSprites();

void TakeResLink( OneObject* OBJ );

void delTrees( int x, int y, int r );
void addTrees( int x, int y, int r );

bool CheckSpritesInArea( int x, int y, int r );
void EraseTreesInPoint( int x, int y );
bool CheckSpritesInAreaNew( int x, int y, int r, bool Erase );
int CheckMinePosition( NewMonster* NM, int* xi, int* yi, int r );

#endif // MAPSPRITES_H
