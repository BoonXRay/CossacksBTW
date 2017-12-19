#ifndef MINE_H
#define MINE_H

extern bool SetDestMode;
extern bool AttGrMode;

extern int DecOrdID;
extern int IncOrdID;

void ATTGR_PRO( int p );

//Displays special abilities for various units or buildings
//Curious: handles officers' abilities, but not for existing formations
class IconSet;
bool CreateInsideList( IconSet* IS, uint8_t NI );

char* GetSprResourceName( OneObject* OB );

void GoToMineLink( OneObject* OBJ );

void LeaveMineLink( OneObject* OB );
void TakeResourceFromSpriteLink( OneObject* OBJ );

void HandleMines();

#endif // MINE_H
