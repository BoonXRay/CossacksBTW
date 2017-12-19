#ifndef WEAPON_H
#define WEAPON_H

#include <cstdint>

constexpr int MaxExpl = 8192;
extern bool EUsage[MaxExpl];

extern char* WeaponIDS[32];

class AnmObject;
extern AnmObject* GAnm[8192];

extern short TSin[257];
extern short TCos[257];
extern short TAtg[257];

class NewAnimation;
extern NewAnimation** FiresAnm[2];
extern NewAnimation** PreFires[2];
extern NewAnimation** PostFires[2];
extern int            NFiresAnm[2];

extern uint16_t rpos;

extern bool ShotRecommended;

void InitExplosions();
void ProcessExpl();

int GetWeaponType( char* Name );
void CloseExplosions();
void ShowExpl();

//x,y-coordinates of point on the 2D plane (unit:pix)
//returnfs index of building,otherwise 0xFFFF
class Weapon;
class OneObject;
bool Create3DAnmObject( Weapon* Weap, int xs, int ys, int zs1, int xd, int yd, int zd, OneObject* OB, uint8_t AttType, uint16_t DestObj );
bool Create3DAnmObject( Weapon* Weap, int xs, int ys, int zs, int xd, int yd, int zd, OneObject* OB );

int PredictShot( Weapon* Weap, int xs, int ys, int zs, int xd, int yd, int zd, uint16_t Index );

#endif // WEAPON_H
