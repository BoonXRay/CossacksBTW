#ifndef REALWATER_H
#define REALWATER_H

#include <cstdint>

extern uint8_t * WaterDeep;
extern uint8_t * WaterBright;

void HandleWater();
void InitWater();
void SetWaterSpot(int x,int y,int r);
void EraseWaterSpot(int x,int y,int r);
void DeepWaterSpot(int x,int y,int r);
void MakeBrod(int x,int y,int r);
void SetupSpot();
void SpotByUnit(int x,int y,int r,uint8_t dir);
void SetBrightSpot(int x,int y,int Brightness,bool dir);
void CreateWaterLocking(int x,int y,int x1,int y1);

class NewMonster;
bool FindPortPlace(NewMonster* NM, int x, int y, int* BuiX, int* BuiY);

#endif // REALWATER_H
