#ifndef MOUSE_H
#define MOUSE_H

#include <cstdint>

extern int curptr;
extern bool LockMouse;

void LoadPointer(LPCSTR s);
void SetMPtr(int x,int y,int key);
//void ShowMPtr();
//void ShowMPtra();
extern int mouseX;
extern int mouseY;
extern bool Lpressed;
extern bool Rpressed;

extern bool realLpressed;
extern bool realRpressed;

extern int curdx;
extern int curdy;

extern uint8_t ScanPressed[256];

extern bool MouseOverZone;

struct MouseStack{
	int x,y;
	bool Lpressed:1;
	bool Rpressed:1;
	bool rLpressed:1;
	bool rRpressed:1;
	bool Control:1;
	bool Shift:1;
};

//Redraws mouse in the offscreen buffer
//and prepares data for onscreen transferring
void RedrawOffScreenMouse();

void OnMouseMoveRedraw();
void PostRedrawMouse();

void InitZones();

typedef void HandlePro( int );

int CreateZone(int x, int y, int lx, int ly, HandlePro* HPro, int Index, char* Hint, char* HintLo);
int CreateRZone( int x, int y, int lx, int ly, HandlePro* HPro, HandlePro* RHPro, int Index, char* Hint, char* HintLo );
int CreateZone( int x, int y, int lx, int ly, HandlePro* HPro, int Index, char* Hint );
void ControlZones();

bool CheckSpritePressed( int sp );

void AssignMovePro( int i, HandlePro* HPro, int id );
void AssignKeys( int i, byte Scan, byte State );

#endif // MOUSE_H
