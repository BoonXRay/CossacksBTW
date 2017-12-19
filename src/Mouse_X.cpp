#include "Ddini.h"
#include "FastDraw.h"
#include "Hint.h"
#include "main.h"
#include "Mapa.h"
#include "NewMon.h"


#include "Mouse_X.h"


bool realLpressed;
bool realRpressed;

//Current cursor image
//0: Default arrow cursor
//1: Sword
//2: Shackles
//3: Hammer
//4: Enter mine
//5: Pickaxe
//6: Axe
//7: Scythe
//8: Gathering point
//9: Artillery barrage
//10: Guard (shield)
//11: Enter transport
//12: Guard (highlighted shield)
//13: Patrol (shield and sword)
int curptr;

int mouseX;
int	mouseY;
bool Lpressed;
bool Rpressed;
static char buf1[1024];
static char buf2[1024];
static char buf1o[1024];
static char buf2o[1024];
static int OldMX;
static int OldMY;
static int MX;
static int MY;

bool LockMouse = false;

int curdx;
int curdy;

//retreives data from the screen buffer to field 32x32
static void GetMData( void* dest, void* src, int x, int y, int SSizeX, int SSizeY )
{
	if (!bActive)
		return;

	int Lx = 32;
	int Ly = 32;
	int x1 = x;
	int y1 = y;
	int bx = 0;//x-coord. on bitbap 32x32
	int by = 0;
	if (x1 < 0)
	{
		bx = -x1;
		Lx += x1;
		x1 = 0;
	}
	if (y1 < 0)
	{
		by = -y1;
		Ly += y1;
		y1 = 0;
	}
	if (x1 + 32 > SSizeX)Lx = SSizeX - x1;
	if (y1 + 32 > SSizeY)Ly = SSizeY - y1;
	if (Lx <= 0 || Ly <= 0)return;
	// BoonXRay 13.08.2017
	//int sofs = int( src ) + x1 + y1*SSizeX;
	//int dofs = int( dest ) + bx + ( by << 5 );
	//int Lx4 = Lx >> 2;
	//int Lx1 = Lx & 3;
	//int adds = SSizeX - Lx;
	//int addd = 32 - Lx;

	//__asm
	//{
	//	push	esi
	//	push	edi
	//	mov		edx, Ly
	//	mov		esi, sofs
	//	mov		edi, dofs
	//	cld
	//	lpp1 : mov		ecx, Lx4
	//		   jcxz	lpp2
	//		   rep		movsd
	//		   lpp2 : mov		ecx, Lx1
	//				  jcxz	lpp3
	//				  rep		movsb
	//				  lpp3 : add		esi, adds
	//						 add		edi, addd
	//						 dec		edx
	//						 jnz		lpp1
	//						 pop		edi
	//						 pop		esi
	//}	
	unsigned char * SrcPtr = reinterpret_cast<unsigned char *>(src)+x1 + y1*SSizeX;
	unsigned char * DestPtr = reinterpret_cast<unsigned char *>(dest) + bx + (by << 5);
	for (int i = 0; i < Ly; i++)
	{
		memcpy(DestPtr, SrcPtr, Lx);
		SrcPtr += SSizeX;
		DestPtr += 32;
	}
}

static bool CmpMData( void* dest, void* src, int x, int y, int SSizeX, int SSizeY )
{
	int Lx = 32;
	int Ly = 32;
	int x1 = x;
	int y1 = y;
	int bx = 0;//x-coord. on bitbap 32x32
	int by = 0;
	if (x1 < 0)
	{
		bx = -x1;
		Lx += x1;
		x1 = 0;
	}
	if (y1 < 0)
	{
		by = -y1;
		Ly += y1;
		y1 = 0;
	}
	if (x1 + 32 > SSizeX)Lx = SSizeX - x1;
	if (y1 + 32 > SSizeY)Ly = SSizeY - y1;
	if (Lx <= 0 || Ly <= 0)return false;
	// BoonXRay 13.08.2017
	//int sofs = int( src ) + x1 + y1*SSizeX;
	//int dofs = int( dest ) + bx + ( by << 5 );
	//int Lx4 = Lx >> 2;
	//int Lx1 = Lx & 3;
	//int adds = SSizeX - Lx;
	//int addd = 32 - Lx;
	//bool notequal = false;
	//__asm
	//{
	//	push	esi
	//	push	edi
	//	mov		edx, Ly
	//	mov		esi, sofs
	//	mov		edi, dofs
	//	cld
	//	lpp1 : mov		ecx, Lx4
	//		   jcxz	lpp2
	//		   repe	cmpsd
	//		   jne		noteq
	//		   lpp2 : mov		ecx, Lx1
	//				  jcxz	lpp3
	//				  repe	cmpsb
	//				  jne		noteq
	//				  lpp3 : add		esi, adds
	//						 add		edi, addd
	//						 dec		edx
	//						 jnz		lpp1
	//						 jmp		lpp4
	//						 noteq : mov		notequal, 1
	//								 lpp4 : pop		edi
	//										pop		esi
	//}
	//return notequal;
	unsigned char * SrcPtr = reinterpret_cast<unsigned char *>(src) + x1 + y1*SSizeX;
	unsigned char * DestPtr = reinterpret_cast<unsigned char *>(dest) + bx + (by << 5);
	for (int i = 0; i < Ly; i++)
	{
		if (memcmp(DestPtr, SrcPtr, Lx) != 0) return true;
		SrcPtr += SSizeX;
		DestPtr += 32;
	}
	return false;
}

static void RestoreMData( void* scrn, void* buf, void* comp, int x, int y, int SSizeX, int SSizeY )
{
	if (!bActive)
	{
		return;
	}

	int Lx = 32;
	int Ly = 32;
	int x1 = x;
	int y1 = y;
	int bx = 0;//x-coord. on bitbap 32x32
	int by = 0;

	if (x1 < 0)
	{
		bx = -x1;
		Lx += x1;
		x1 = 0;
	}
	if (y1 < 0)
	{
		by = -y1;
		Ly += y1;
		y1 = 0;
	}

	if (x1 + 32 > SSizeX)
	{
		Lx = SSizeX - x1;
	}
	if (y1 + 32 > SSizeY)
	{
		Ly = SSizeY - y1;
	}

	if (Lx <= 0 || Ly <= 0)
	{
		return;
	}

	// BoonXRay 13.08.2017
	//int src1 = int( buf ) + bx + ( by << 5 );
	//int srcom = int( comp ) + bx + ( by << 5 );
	//int scrof = int( scrn ) + x1 + y1*SSizeX;
	int addscr = SSizeX - Lx;
	int add32 = 32 - Lx;

	// BoonXRay 13.08.2017
	//__asm
	//{
	//	push	esi
	//	push	edi
	//	mov		edx, Ly
	//	mov		esi, src1
	//	mov		ebx, srcom
	//	mov		edi, scrof
	//	cld
	//	lpp0 : mov		ecx, Lx
	//		   lpp1 : lodsb
	//				  mov		ah, [edi]
	//				  cmp		ah, [ebx]
	//				  jnz		lpp2
	//				  mov[edi], al
	//				  lpp2 : inc		edi
	//						 inc		ebx
	//						 dec		ecx
	//						 jnz		lpp1
	//						 add		edi, addscr
	//						 add		ebx, add32
	//						 add		esi, add32
	//						 dec		edx
	//						 jnz		lpp0
	//						 pop		edi
	//						 pop		esi
	//}
	unsigned char * Ptr1 = reinterpret_cast<unsigned char *>(buf) + bx + (by << 5);
	unsigned char *  PtrOm = reinterpret_cast<unsigned char *>(comp) + bx + (by << 5);
	unsigned char *  PtrOf = reinterpret_cast<unsigned char *>(scrn) + x1 + y1*SSizeX;;
	for (int i = 0; i < Ly; i++)
	{
		for (int j = 0; j < Lx; j++)
		{
			if (*PtrOf == *PtrOm)
				*PtrOf = *Ptr1;
			Ptr1++;
			PtrOf++;
			PtrOm++;
		}
		PtrOf += addscr;
		PtrOm += add32;
		Ptr1 += add32;
	}
}

//Sets mouse[X|Y] & real[L|R]pressed variables according to mouse state
void SetMPtr( int x, int y, int key )
{
	if (x > RSCRSizeX - 1)
	{
		x = RSCRSizeX - 1;
	}

	if (( x != mouseX ) | ( y != mouseY ))
	{
		mouseX = x;
		mouseY = y;
		realLpressed = ( ( key & MK_LBUTTON ) != 0 );
		realRpressed = ( ( key & MK_RBUTTON ) != 0 );
	}
}

//Redraws mouse in the offscreen buffer
//and prepares data for onscreen transferring 
void RedrawOffScreenMouse()
{
	if (!bActive)
	{
		return;
	}

	if (curptr == 8)
	{
		curdx = 16;
		curdy = 17;
	}
	else
	{
		curdx = 5;
		curdy = 5;
	}

	LockMouse = true;
	MX = mouseX - curdx;
	MY = mouseY - curdy;

	RestoreMData( ScreenPtr, (void*) buf1, (void*) buf2, OldMX, OldMY, SCRSizeX, SCRSizeY );

	GetMData( (void*) buf1, ScreenPtr, MX, MY, SCRSizeX, SCRSizeY );

	GPS.ShowGP( MX, MY, CurrentCursorGP, curptr, 0 );

	GetMData( (void*) buf2, ScreenPtr, MX, MY, SCRSizeX, SCRSizeY );
}

static void RedrawScreenMouse()
{
	if (!bActive || window_mode)//BUGFIX: Cursor shadow trail while showing ingame menues
	{
		return;
	}

	RestoreMData( RealScreenPtr, (void*) buf1o, (void*) buf2o, OldMX, OldMY, RSCRSizeX, RSCRSizeY );
	GetMData( (void*) buf1o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
	void* osp = ScreenPtr;
	int osx = SCRSizeX;
	int sw = ScrWidth;
	ScrWidth = RSCRSizeX;
	ScreenPtr = RealScreenPtr;
	SCRSizeX = RSCRSizeX;
	GPS.ShowGP( MX, MY, CurrentCursorGP, curptr, 0 );
	SCRSizeX = osx;
	ScreenPtr = osp;
	ScrWidth = sw;
	GetMData( (void*) buf2o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
}

void OnMouseMoveRedraw()
{
	if (LockMouse)
	{
		return;
	}

	RedrawOffScreenMouse();
	RedrawScreenMouse();
	LockMouse = false;
	OldMX = MX;
	OldMY = MY;
}

void PostRedrawMouse()
{
	bool need = true;
	if (MX == OldMX && MY == OldMY)
	{
		need = CmpMData( (void*) buf2o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
	}

	if (need)
	{
		RestoreMData( RealScreenPtr, (void*) buf1, (void*) buf2, MX, MY, RSCRSizeX, RSCRSizeY );
		RestoreMData( RealScreenPtr, (void*) buf1o, (void*) buf2o, OldMX, OldMY, RSCRSizeX, RSCRSizeY );
		GetMData( (void*) buf1o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
		void* osp = ScreenPtr;
		int osx = SCRSizeX;
		int sw = ScrWidth;
		ScrWidth = RSCRSizeX;
		ScreenPtr = RealScreenPtr;
		SCRSizeX = RSCRSizeX;
		GPS.ShowGP( MX, MY, CurrentCursorGP, curptr, 0 );
		SCRSizeX = osx;
		ScreenPtr = osp;
		ScrWidth = sw;
		GetMData( (void*) buf2o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
		OldMX = MX;
		OldMY = MY;
	}

	LockMouse = false;
}



class MouseZone
{
public:
    int x, y, x1, y1, Index, MoveIndex;
    byte ScanCode;
    byte KeyState;
    byte Pressed;
    HandlePro* Pro;
    HandlePro* RPro;
    HandlePro* MoveOver;
    char* Hint;
    char* HintLo;
    MouseZone();
};

MouseZone::MouseZone()
{
    Index = -1;
    Hint = NULL;
}

static constexpr int NZones = 64;
static MouseZone Zones[NZones];

void InitZones()
{
    for (int i = 0; i < NZones; i++)
    {
        Zones[i].Index = -1;
    }
}

int CreateRZone(int x, int y, int lx, int ly, HandlePro* HPro, HandlePro* RHPro, int Index, char* Hint)
{
    int i;
    for (i = 0; i < NZones; i++)
    {
        if (Zones[i].Index == -1)
        {
            break;
        }
    }
    if (i < NZones)
    {
        MouseZone* Z = &(Zones[i]);
        Z->x = x;
        Z->y = y;
        Z->x1 = x + lx - 1;
        Z->y1 = y + ly - 1;
        Z->Pro = HPro;
        Z->RPro = RHPro;
        Z->MoveOver = NULL;
        Z->Index = Index;
        Z->Pressed = false;
        if (int(Z->Hint))
            free(Z->Hint);
        Z->Hint = new char[strlen(Hint) + 1];
        strcpy(Z->Hint, Hint);
        Z->KeyState = 0;
        Z->ScanCode = 0xFF;
        return i;
    }
    return -1;
}

int CreateRZone( int x, int y, int lx, int ly, HandlePro* HPro, HandlePro* RHPro, int Index, char* Hint, char* HintLo )
{
    int i;
    for (i = 0; i < NZones; i++)
    {
        if (Zones[i].Index == -1)
            break;
    }
    if (i < NZones)
    {
        MouseZone* Z = &(Zones[i]);

        Z->x = x;
        Z->y = y;
        Z->x1 = x + lx - 1;
        Z->y1 = y + ly - 1;
        Z->Pro = HPro;
        Z->RPro = RHPro;
        Z->MoveOver = NULL;
        Z->Index = Index;
        Z->Pressed = false;
        Z->KeyState = 0;
        Z->ScanCode = 0xFF;

        if (Z->Hint)
        {
            free(Z->Hint);
            Z->Hint = NULL;
        }
        if (Z->HintLo)
        {
            free(Z->HintLo);
            Z->HintLo = NULL;
        }
        if (Hint)
        {
            Z->Hint = new char[strlen(Hint) + 1];
            strcpy(Z->Hint, Hint);
        }
        if (HintLo)
        {
            Z->HintLo = new char[strlen(HintLo) + 1];
            strcpy(Z->Hint, Hint);
        }
        return i;
    }
    return -1;
}

int CreateZone(int x, int y, int lx, int ly, HandlePro* HPro, int Index, char* Hint)
{
    int i;
    for (i = 0; i < NZones; i++)
    {
        if (Zones[i].Index == -1)break;
    }
    if (i < NZones)
    {
        MouseZone* Z = &(Zones[i]);
        Z->x = x;
        Z->y = y;
        Z->x1 = x + lx - 1;
        Z->y1 = y + ly - 1;
        Z->Pro = HPro;
        Z->RPro = NULL;
        Z->Index = Index;
        Z->Pressed = false;
        Z->KeyState = 0;
        Z->ScanCode = 0xFF;
        if (int(Z->Hint))
            free(Z->Hint);
        Z->Hint = new char[strlen(Hint) + 1];
        strcpy(Z->Hint, Hint);
        return i;
    }
    return -1;
}

void AssignMovePro(int i, HandlePro* HPro, int id)
{
    if (i != -1)
    {
        Zones[i].MoveOver = HPro;
        Zones[i].MoveIndex = id;
    }
}

void AssignKeys(int i, byte Scan, byte State)
{
    if (i != -1)
    {
        Zones[i].ScanCode = Scan;
        Zones[i].KeyState = State;
    }
}

int CreateZone(int x, int y, int lx, int ly, HandlePro* HPro, int Index, char* Hint, char* HintLo)
{
    int i;
    for (i = 0; i < NZones; i++) {
        if (Zones[i].Index == -1)break;
    }
    if (i < NZones) {
        MouseZone* Z = &(Zones[i]);
        Z->x = x;
        Z->y = y;
        Z->x1 = x + lx - 1;
        Z->y1 = y + ly - 1;
        Z->Pro = HPro;
        Z->RPro = NULL;
        Z->MoveOver = NULL;
        Z->Index = Index;
        Z->Pressed = false;
        Z->KeyState = 0;
        Z->ScanCode = 0xFF;
        if (int(Z->Hint)) {
            free(Z->Hint);
            Z->Hint = NULL;
        }
        if (int(Z->HintLo)) {
            free(Z->HintLo);
            Z->HintLo = NULL;
        }
        if (Hint) {
            Z->Hint = new char[strlen(Hint) + 1];
            strcpy(Z->Hint, Hint);
        }
        if (HintLo) {
            Z->HintLo = new char[strlen(HintLo) + 1];
            strcpy(Z->HintLo, HintLo);
        }
        return i;
    }
    return -1;
}

bool MouseOverZone = 0;
static uint8_t LastPressedCodes[8] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
uint8_t ScanPressed[256];

static int CheckZonePressed(int i)
{
    if (EnterChatMode || EditMapMode)return false;
    for (int j = 0; j < 8; j++)if (LastPressedCodes[j] != 0xFF) {
        if (!(GetKeyState(LastPressedCodes[j]) & 0x8000))LastPressedCodes[j] = 0xFF;
    }
    if (i < NZones&&Zones[i].Index != -1) {
        if (Zones[i].ScanCode != 0xFF) {
            if ((GetKeyState(Zones[i].ScanCode) & 0x8000) || ScanPressed[Zones[i].ScanCode]) {
                byte State = Zones[i].KeyState;
                byte Scan = Zones[i].ScanCode;

                if (State & 1) {
                    if (!(GetKeyState(VK_CONTROL) & 0x8000))return false;
                }
                else if (GetKeyState(VK_CONTROL) & 0x8000)return false;
                if (State & 2) {
                    if (!(GetKeyState(VK_MENU) & 0x8000))return false;
                }
                else if (GetKeyState(VK_MENU) & 0x8000)return false;
                if (State & 4) {
                    if (!(GetKeyState(VK_SHIFT) & 0x8000))return false;
                }
                else if (GetKeyState(VK_SHIFT) & 0x8000)return false;

                for (int j = 0; j < 8; j++)if (LastPressedCodes[j] == Scan)return 1;
                for (int j = 0; j < 8; j++)if (LastPressedCodes[j] == 0xFF) {
                    LastPressedCodes[j] = Scan;
                    return 2;
                }
                return 2;
            }
            else return false;
        }
        else return false;
    }
    else return false;
}

bool CheckSpritePressed(int sp) {
    if (sp < 0 || sp >= 512 || EnterChatMode || EditMapMode)return false;
    if (KeyCodes[sp][0]) {
        if ((GetKeyState(ScanKeys[KeyCodes[sp][0]]) & 0x8000) || ScanPressed[ScanKeys[KeyCodes[sp][0]]]) {
            byte State = KeyCodes[sp][1];
            //byte Scan = ScanKeys[KeyCodes[sp][0]];
            if (State & 1) {
                if (!(GetKeyState(VK_CONTROL) & 0x8000))return false;
            }
            else if (GetKeyState(VK_CONTROL) & 0x8000)return false;
            if (State & 2) {
                if (!(GetKeyState(VK_MENU) & 0x8000))return false;
            }
            else if (GetKeyState(VK_MENU) & 0x8000)return false;
            if (State & 4) {
                if (!(GetKeyState(VK_SHIFT) & 0x8000))return false;
            }
            else if (GetKeyState(VK_SHIFT) & 0x8000)return false;
            return 1;
        }
    }
    return false;
}

void ControlZones()
{
    MouseOverZone = 0;
    if (LockMouse)
    {
        return;
    }
    int i;
    MouseZone* Z = nullptr;
    if (!Lpressed)
    {
        for (i = 0; i < NZones; i++)
        {
            Zones[i].Pressed = CheckZonePressed(i);
        }
    }
    for (i = 0; i < NZones; i++)
    {
        Z = &(Zones[i]);
        if ((Z->Index != -1 && mouseX >= Z->x&&mouseX <= Z->x1
            && mouseY >= Z->y&&mouseY <= Z->y1)
            || Z->Pressed)
        {
            break;
        }
    }
    if (i < NZones)
    {
        MouseOverZone = 1;
        if (SpecCmd == 241)
        {
            SpecCmd = 0;
        }
        if (CheckZonePressed(i) == 2 || Z->Pressed != 1)
        {
            if (Lpressed)
            {
                Z->Pressed = true;
            }
            if (int(Z->Hint))
            {
                AssignHint(Z->Hint, 3);
            }
            if (Z->HintLo)
            {
                AssignHintLo(Z->HintLo, 3);
            }
            if ((Lpressed || Z->Pressed == 2) && int(Z->Pro))
            {//Handle mouse clicks?
                (*Z->Pro)(Z->Index);
            }
            Lpressed = false;
            if (Rpressed&&Z->RPro)
            {
                (*Z->RPro)(Z->Index);
            }
            if (Z->MoveOver)
            {
                Z->MoveOver(Z->MoveIndex);
            }
            Rpressed = false;
            if (Z->Pressed == 2)
            {
                Z->Pressed = 0;
            }
        }
    }
    for (int i = 0; i < NZones; i++)
    {
        Zones[i].Pressed = 0;
    }
    memset(ScanPressed, 0, 256);
}
