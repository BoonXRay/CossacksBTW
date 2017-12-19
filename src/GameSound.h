#ifndef GSOUND_H
#define GSOUND_H

#include <cstdint>

constexpr int MaxSnd = 1024;

extern int FieldDelay;

class CDirSound;
extern CDirSound * CDS;

extern char * SoundID[ MaxSnd ];

void LoadSounds( const char* fn);
void PlayEffect(int n,int pan,int vol);
void AddSingleEffect(int x,int y,int id);
void AddEffect(int x,int y,int id);
void AddEffectV( int x, int y, int vx, int id );
void AddUnlimitedEffect( int x, int y, int id );
void AddOrderEffect(int x,int y,int id);

void PrepareSound();
extern uint16_t NSounds;
extern int CenterX;

#endif // GSOUND_H
