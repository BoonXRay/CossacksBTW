#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <cstdint>

extern uint8_t * FishMap;
extern int FishLx;

class OneObject;

void HandleTransport();
bool CheckTransportOnParking(OneObject* TRA,int x,int y);
bool CheckTransportEntering( OneObject* OB );
bool ParkWaterNewMonster( OneObject* OB, int x, int y, uint8_t Prio, uint8_t OrdType );

void CreateFishMap();
void InitFishMap();
void ProcessFishing();

void LeaveAll( OneObject* OB );

void WaterNewMonsterSendToLink( OneObject* OB );
void AbsorbObjectLink( OneObject* OB );
void LeaveShipLink( OneObject* OB );
void LeaveTransportLink( OneObject* OB );
void GoToTransportLink( OneObject* OBJ );
void FishingLink( OneObject* OBJ );


#endif // TRANSPORT_H
