#ifndef STRECOG_H
#define STRECOG_H

#include <cstdint>

class Weapon;
extern Weapon * WPLIST[1024];

extern uint32_t LOADNATMASK;

void normstr(char* str);

void LoadNation(char* fn, uint8_t NIndex, uint8_t NatID);
void LoadWeapon();
void LoadAllNations( uint8_t NIndex );

int SearchStr(char** Res, const char* s, int count);

int GetWeaponIndex( const char* str );

class GeneralObject;
void InitFlags( GeneralObject* GO );

#endif // STRECOG_H
