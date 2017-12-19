#ifndef FOG_H
#define FOG_H

#include <cstdint>

extern int FMSX;
extern int FMSX2;

extern uint16_t * fmap;
extern uint16_t * fmap1;

extern uint8_t Optional1[8192];
extern uint8_t Optional2[8192];
extern uint8_t Optional3[8192];

extern uint8_t fog[ 8192 + 1024 ];
extern uint8_t wfog[ 8192 ];

extern uint8_t darkfog[ 40960 ];
extern uint8_t yfog[ 8192 ];
extern uint8_t trans4[ 65536 ];
extern uint8_t trans8[ 65536 ];
extern uint8_t AlphaR[ 65536 ];
extern uint8_t AlphaW[ 65536 ];

extern uint8_t refl[3072];
extern uint8_t WaterCost[65536];
extern uint8_t Bright[8192];

extern int FogMode;

extern uint8_t graysc[256];

void LoadOptionalTable(int n, const char *Name);

void ClearFog();
void ProcessFog1();
__declspec( dllexport ) void LoadFog(int);
void SetupFog();
void TurnFogOff();
void DrawFog();
uint16_t GetFog( int x, int y );
void DrawMiniFog();
void FogSpot( int x, int y );
void ShowFoggedBattle();
void AcceptMiniMap();

void makeFden();

class OneObject;
void UnitLight( OneObject * OB );
void CreateFogImage();

#endif // FOG_H
