#ifndef BUILD_H
#define BUILD_H

#include <cstdint>

class OneObject;

bool ShowProducedShip( OneObject* Port, int CX, int CY );
void GotoFinePosition( OneObject* OB );
void GotoFinePositionLink( OneObject* OB );
void ProduceObjLink( OneObject* OBJ );
void GetCorrectMoney( uint8_t NI, int* MONEY );
void TakeUnitFromCash( uint8_t NI, uint16_t ID );
bool AddUnitsToCash( uint8_t NI, uint16_t ID );
void ClearUniCash();
int GetAmount( uint16_t ID );
int GetProgress( uint16_t ID, int* MaxProgress );


#endif // BUILD_H
