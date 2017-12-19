#ifndef GROUPS_H
#define GROUPS_H

#include <cstdint>

class OrderDescription;
class PositionOrder{
public:
    int NUnits;
    int MaxUnit;
    int CenterX;
    int CenterY;
    uint16_t *  Ids;
    int* px;
    int* py;
    PositionOrder();
    ~PositionOrder();
    void CheckSize(int Size);
    void Create(uint16_t *  IDS,int NIDS);
    uint16_t CreateLinearPositions(int x,int y,uint16_t *  IDS,int NIDS,int dx,int dy);
    uint16_t CreateRotatedPositions(int x,int y,uint16_t *  IDS,int NIDS,int dx,int dy);
    uint16_t CreateRotatedPositions2(int x,int y,uint16_t *  IDS,int NIDS,int dx,int dy);
    uint16_t CreateOrdPos(int x,int y, uint8_t dir,int NIDS,uint16_t *  IDS,OrderDescription* ODS);
    uint16_t CreateSimpleOrdPos(int x,int y,uint8_t dir,int NIDS,uint16_t *  IDS,OrderDescription* ODS);
    void SendToPosition( uint8_t Prio, uint8_t OrdType);
    void Patrol();
};
void GroupSendSelectedTo( uint8_t NI,int x,int y, uint8_t Prio, uint8_t OrdType);
extern PositionOrder PORD;
extern short LastDirection;

class Brigade;
void SetStandState(Brigade* BR, uint8_t State);

void GroupAttackSelectedBrigadesTo( uint8_t NI, int x, int y, uint8_t /*Prio*/, uint8_t OrdType);
void GroupPatrolSelected( uint8_t NI, int x, int y, uint8_t /*Prio*/) ;

#endif // GROUPS_H
