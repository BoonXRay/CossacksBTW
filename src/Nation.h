#ifndef NATION_H
#define NATION_H

#include <cstdint>

extern uint16_t MAXOBJECT;

extern int NNations;

extern char NatCharLo[32][8];
extern char NatCharHi[32][8];

extern int AlarmDelay;

extern char** NatsIDS;
extern char** NatNames;
extern char** NatScripts;

extern uint16_t * NatList[8];
extern int   NtNUnits[8];
extern int   NtMaxUnits[8];

extern int LastAttackX;
extern int LastAttackY;

extern int NThemUnits;
extern int NMyUnits;

extern uint8_t NTex1[32];
extern uint8_t NTex2[32];

class OneObject;
bool CheckAttAbility( OneObject* OB, uint16_t Patient );

void SetupNatList();
void InitNatList();
class OneObject;
void AddObject( OneObject* OB );
void DelObject( OneObject* OB );

void DestructBuilding( OneObject* OB );
void EliminateBuilding( OneObject* OB );

void InitDeathList();

void PatrolLink( OneObject* OBJ );

void WinnerControl( bool Anyway );

void LoadNations();
int GetNationByID(char* Name);

class OneObject;
typedef void ReportFn( OneObject* Sender );
extern ReportFn* AttackLink;

void HealWalls();

void ProcessDeathList();
void EnumPopulation();

void __stdcall CDGINIT_INIT2();

#endif // NATION_H
