#ifndef SELPROP_H
#define SELPROP_H

#include "IconTool.h"

extern bool BreefUInfo;

extern int PrpX;
extern int PrpY;
extern int PrpNx;
extern int PrpNy;
extern int AblX;
extern int AblY;
extern int AblNx;
extern int AblNy;

extern int IconLx;
extern int IconLy;

extern bool ECOSHOW;

extern IconSet PrpPanel;
extern IconSet AblPanel;
extern IconSet UpgPanel;
extern IconSet BrigPanel;

extern bool GetCoord;
extern bool GoAndAttackMode;

typedef void UniqMethood( int x, int y );
extern UniqMethood* UNIM;

extern int SelSoundType;
extern bool SelSoundReady;

void InitPrpBar();
void ShowProp();
void ShowAbility();

char * GetTextByID( const char * ID );
void ShowTextDiscription();
void LoadMessagesFromFile( const char* Name );
void LoadMessages();

#endif // SELPROP_H
