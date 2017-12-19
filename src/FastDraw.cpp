/*               Fast raster graphics routine
 *
 *  This module presents several useful procedures for fast output,
 * using RLC graphics format (see the discription below). Before the
 * wor you must call SetRlcWindow to set properly the screen
 * capabilities.After this you mast call SetScreenPtr to set the
 * output distanation. Uou can use LoadRLC to load the set of RLC
 * pictures into the RLCTable structure. Then you can draw the
 * picture on the screen wit the procedure ShowRLCItem
 */

#include <cstdio>

#include "Ddini.h"
#include "GP_Draw.h"
#include "Interface.h"
#include "ResFile.h"


#include "FastDraw.h"

static uint8_t PAL1[256];
static uint8_t PAL2[256];
static uint8_t PAL3[256];
static uint8_t PAL4[256];
static uint8_t PAL5[256];
static uint8_t PAL6[256];
static uint8_t PAL7[256];

typedef RLCHeader* lpRLCHeader;

typedef RLCFont* lpRLCFont;
//Window parameters
int WindX = 0;
int WindY = 0;
int WindLx;
int WindLy;
int WindX1;
int WindY1;
int ScrWidth;
int ScrHeight;
int BytesPerPixel = 1;
void* ScreenPtr = nullptr;
void* RealScreenPtr = nullptr;

void InitRLCWindows()
{
	WindLx = RealLx;
	WindLy = RealLy;
	WindX1 = RealLx - 1;
	WindY1 = RealLy - 1;
	ScrWidth = SCRSizeX;
	ScrHeight = SCRSizeY;
}
/*  Creating window
 */
static int cntr;
static bool InCycle;

void SetRLCWindow( int x, int y, int lx, int ly, int slx )
{
	WindX = x;
	WindY = y;
	WindX1 = x + lx - 1;
	WindY1 = y + ly - 1;
	ScrWidth = slx;
	if (WindX < 0)
		WindX = 0;
	if (WindY < 0)
		WindY = 0;
	if (WindX1 >= RealLx)
		WindX1 = RealLx - 1;
	if (WindY1 >= RealLy)
		WindY1 = RealLy - 1;
	WindLx = WindX1 - WindX + 1;
	WindLy = WindY1 - WindY + 1;
}

void ClearScreen()
{
    if ( RealScreenPtr )
    {
        memset( RealScreenPtr, 0, RSCRSizeX*SCRSZY );
    }
}

//Showing RLC image with clipping
//Shows RLC picture
//This procedure needs window parameters to be initializated
static void ShowRLC( int x, int y, void* PicPtr )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x + PLX <= WindX ) || ( x > WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		// BoonXRay 06.08.2017
		//__asm
		{
			//mov		edx, PicPtr
			//add		edx, 4
			unsigned int TmpEDX = reinterpret_cast<unsigned int>(PicPtr) + 4;
			//xor eax, eax
			//mov		ecx, WindY
			//sub		ecx, y
			unsigned int TmpECX = WindY - y;
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			//xor		eax, eax
			//xor		ebx, ebx
			unsigned int TmpEAX = 0, TmpEBX = 0;
			unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
		Loop1xx1 :
			//mov		al, [edx]
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEDX);
			//inc		edx
			TmpEDX++;
			//or eax, eax
			//jz		Loop1xx3
			if (TmpEAX == 0) goto Loop1xx3;
		Loop1xx2 : 
			//mov		bl, [edx + 1]
			TmpBL = *reinterpret_cast<unsigned char *>(TmpEDX+1);
			//add		edx, ebx
			TmpEDX += TmpEBX;
			//add		edx, 2
			TmpEDX += 2;
			//dec		eax
			TmpEAX--;
			//jnz		Loop1xx2
			if (TmpEAX != 0) goto Loop1xx2;
		Loop1xx3 : 
			//dec		cx
			TmpCX--;
			//jnz		Loop1xx1
			if (TmpCX != 0) goto Loop1xx1;
			//sub		edx, PicPtr
			TmpEDX -= reinterpret_cast<unsigned int>(PicPtr);
			//sub		edx, 4
			TmpEDX -= 4;
			//mov		addofs, edx
			addofs = TmpEDX;
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x < WindX)
		{
			int roff = WindX - x;
			// BoonXRay 06.08.2017			
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				unsigned int TmpEDI = ScrOfst;
				//mov		esi, PicPtr
				//add		esi, addofs
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				//xor		ecx, ecx
				//xor		eax, eax
				//cld
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
				unsigned short & TmpAX = *reinterpret_cast<unsigned short *>(&TmpEAX);
				unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned int PushTmpEDI;
			ScanLineLoop1 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd1
				if (PLY == 0) goto ScanLineLoopEnd1;
				//push	edi
				PushTmpEDI = TmpEDI;
				//mov		dl, [esi]
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				//xor		ebx, ebx
				TmpEBX = 0;
				//inc		esi
				TmpESI++;
				//or	dl, dl
				//jz		NextLine1
				if (TmpDL == 0) goto NextLine1;
				//mov		bx, word ptr roff
				//TmpBX = *reinterpret_cast<unsigned short *>(roff);
				TmpBX = roff;
			BeginLine1 : 
				//mov		cl, [esi]
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				//sub		bx, cx
				TmpBX -= TmpCX;
				//add		edi, ecx
				TmpEDI += TmpECX;
				//mov		cl, [esi + 1]
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				//add		esi, 2
				TmpESI += 2;
				//clipping left code
				//cmp		bx, 0
				//jle		ok1
				if (static_cast<short>(TmpBX) <= 0) goto ok1;
				//cmp		bx, cx
				//jl		hdraw1
				if (TmpBX < TmpCX) goto hdraw1;
				//nothing to draw
				//sub		bx, cx
				TmpBX -= TmpCX;
				//add		esi, ecx
				TmpESI += TmpECX;
				//add		edi, ecx
				TmpEDI += TmpECX;
				//dec		dl
				TmpDL--;
				//jnz		BeginLine1
				if (TmpDL != 0) goto BeginLine1;
				//pop		edi
				TmpEDI = PushTmpEDI;
				//add		edi, ScrWidth
				TmpEDI += ScrWidth;
				//dec     PLY
				PLY--;
				//jmp		ScanLineLoop1
				goto ScanLineLoop1;
			hdraw1 :			//draw only small part of line
				//sub		cx, bx
				TmpCX -= TmpBX;
				//mov		ax, bx
				TmpAX = TmpBX;
				//xor		bx, bx
				TmpBX = 0;
				//add		esi, eax
				TmpESI += TmpEAX;
				//add		edi, eax
				TmpEDI += TmpEAX;
			ok1 :
				//mov		eax, ecx
				TmpEAX = TmpECX;
				//shr		ecx, 2
				TmpECX >>= 2;
				//rep		movsd
				for (; TmpECX != 0; TmpECX--, TmpESI += 4 /*sizeof(int)*/, TmpEDI += 4 /*sizeof(int)*/)
					*reinterpret_cast<unsigned int *>(TmpEDI) = *reinterpret_cast<unsigned int *>(TmpESI);
				//mov		ecx, eax
				TmpECX = TmpEAX;
				//and		ecx, 3
				TmpECX &= 3;
				//rep		movsb
				for (; TmpECX != 0; TmpECX--, TmpESI++, TmpEDI++)
					*reinterpret_cast<unsigned char *>(TmpEDI) = *reinterpret_cast<unsigned char *>(TmpESI);
				//dec		dl
				TmpDL--;
				//jnz		BeginLine1
				if (TmpDL != 0) goto BeginLine1;
			NextLine1 : 
				//pop		edi
				TmpEDI = PushTmpEDI;
				//add		edi, ScrWidth
				TmpEDI += ScrWidth;
				//dec     PLY
				PLY--;
				//jmp		ScanLineLoop1
				goto ScanLineLoop1;
			ScanLineLoopEnd1 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else if (x + PLX >= WindX1)
		{
			int roff = WindX1 - x + 1;
			int part;
			// BoonXRay 06.08.2017			
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
                unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned int PushTmpEDI;

			ScanLineLoop2 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd2
				//push	edi
				//mov		dl, [esi]
				//inc		esi
				//xor     ebx, ebx
				//or dl, dl
				//jz		NextLine2
				//mov		bx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd2;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpEBX = 0;
				if (TmpDL == 0) goto NextLine2;
				TmpBX = roff;
			BeginLine2 :
				//mov		cl, [esi]
				//sub		bx, cx
				//add		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpBX -= TmpCX;
				TmpEDI += TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping right code
				//cmp		bx, cx
				//jge		ok2
				if (TmpBX >= TmpCX) goto ok2;
				//clipping
				//cmp		bx, 0
				//jle		ntd2
				if (static_cast<short>(TmpBX) <= 0) goto ntd2;
				//partial drawing
				//sub		cx, bx
				TmpCX -= TmpBX;
				//mov		part, ecx
				part = TmpECX;
				//mov		cx, bx
				TmpCX = TmpBX;
				//mov		eax, ecx
				//shr		ecx, 2
				//rep		movsd
				//mov		ecx, eax
				//and		ecx, 3
				//rep		movsb
				TmpEAX = TmpECX;
				TmpECX >>= 2;
				for (; TmpECX != 0; TmpECX--, TmpESI += 4 /*sizeof(int)*/, TmpEDI += 4 /*sizeof(int)*/)
					*reinterpret_cast<unsigned int *>(TmpEDI) = *reinterpret_cast<unsigned int *>(TmpESI);
				TmpECX = TmpEAX;
				TmpECX &= 3;
				for (; TmpECX != 0; TmpECX--, TmpESI++, TmpEDI++)
					*reinterpret_cast<unsigned char *>(TmpEDI) = *reinterpret_cast<unsigned char *>(TmpESI);
				//add		esi, part
				TmpESI += part;
				//jmp		ntd4
				goto ntd4;
			ntd2 :			//scanning to the next line
				//add		esi, ecx
				TmpESI += TmpECX;
			ntd4 : 
				//dec		dl
				TmpDL--;
				//jz		NextLine2
				if (TmpDL == 0) goto NextLine2;
			ntd22 : 
				//mov		cl, [esi + 1]
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				//add		esi, 2
				//add		esi, ecx
				TmpESI += 2 + TmpECX;
				//dec		dl
				TmpDL--;
				//jnz		ntd22
				if (TmpDL != 0) goto ntd22;
				//jmp		NextLine2
				goto NextLine2;
			ok2 : 
				//sub		bx, cx
				//mov		eax, ecx
				//shr		ecx, 2
				//rep		movsd
				//mov		ecx, eax
				//and		ecx, 3
				//rep		movsb

				TmpBX -= TmpCX;
				TmpEAX = TmpECX;
				TmpECX >>= 2;
				for (; TmpECX != 0; TmpECX--, TmpESI += 4 /*sizeof(int)*/, TmpEDI += 4 /*sizeof(int)*/)
					*reinterpret_cast<unsigned int *>(TmpEDI) = *reinterpret_cast<unsigned int *>(TmpESI);
				TmpECX = TmpEAX;
				TmpECX &= 3;
				for (; TmpECX != 0; TmpECX--, TmpESI++, TmpEDI++)
					*reinterpret_cast<unsigned char *>(TmpEDI) = *reinterpret_cast<unsigned char *>(TmpESI);
				//dec		dl
				TmpDL--;
				//jnz		BeginLine2
				if (TmpDL != 0) goto BeginLine2;
			NextLine2 : 
				//pop		edi
				TmpEDI = PushTmpEDI;
				//add		edi, ScrWidth
				TmpEDI += ScrWidth;
				//dec     PLY
				PLY--;
				//jmp		ScanLineLoop2
				goto ScanLineLoop2;
			ScanLineLoopEnd2 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else
			// BoonXRay 06.08.2017		
		//__asm
		{
			//push	esi
			//push	edi
			//mov		edi, ScrOfst
			//mov		esi, PicPtr
			//add		esi, addofs
			//xor		ecx, ecx
			//xor		ebx, ebx
			//cld
			unsigned int TmpEDI = ScrOfst;
			unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
            unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0;
            unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
			unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned int PushTmpEDI;
		ScanLineLoop :
			//cmp		PLY, 0
			//je		ScanLineLoopEnd
			//push	edi
			//mov		bl, [esi]
			//inc		esi
			//or bl, bl
			//jz		NextLine
			if (PLY == 0) goto ScanLineLoopEnd;
			PushTmpEDI = TmpEDI;
			TmpBL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			if (TmpBL == 0) goto NextLine;
		BeginLine : 
			//mov		cl, [esi]
			//add		edi, ecx
			//mov		cl, [esi + 1]
			//add		esi, 2
			//mov		eax, ecx
			//shr		ecx, 2
			//jcxz	Lx1
			//rep		movsd
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpEDI += TmpECX;
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
			TmpESI += 2;
			TmpEAX = TmpECX;
			TmpECX >>= 2;
			if (TmpCX == 0) goto Lx1;
			for (; TmpECX != 0; TmpECX--, TmpESI += 4 /*sizeof(int)*/, TmpEDI += 4 /*sizeof(int)*/)
				*reinterpret_cast<unsigned int *>(TmpEDI) = *reinterpret_cast<unsigned int *>(TmpESI);
		Lx1 : 
			//mov		ecx, eax
			//and		ecx, 3
			//rep		movsb
			//dec		ebx
			//jnz		BeginLine
			TmpECX = TmpEAX;
			TmpECX &= 3;
			for (; TmpECX != 0; TmpECX--, TmpESI++, TmpEDI++)
				*reinterpret_cast<unsigned char *>(TmpEDI) = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpEBX--;
			if (TmpEBX != 0) goto BeginLine;
		NextLine : 
			//pop		edi
			TmpEDI = PushTmpEDI;
			//add		edi, ScrWidth
			TmpEDI += ScrWidth;
			//dec     PLY
			PLY--;
			//jmp		ScanLineLoop
			goto ScanLineLoop;
		ScanLineLoopEnd :
			//pop		edi
			//pop		esi
			;
		}
	}
}

//End of RLC with clipping
//Showing inverse RLC image with clipping
static void ShowRLCi( int x, int y, void* PicPtr )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x < WindX ) || ( x - PLX + 1 >= WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		// BoonXRay 06.08.2017		
		//__asm
		{
			//mov		edx, PicPtr
			//add		edx, 4
			unsigned int TmpEDX = reinterpret_cast<unsigned int>(PicPtr) + 4;
			//xor eax, eax
			//mov		ecx, WindY
			//sub		ecx, y
			//xor		eax, eax
			//xor		ebx, ebx
			unsigned int TmpECX = WindY - y;
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned int TmpEAX = 0, TmpEBX = 0;
			unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
		Loop1xx1 : 
			//mov		al, [edx]
			//inc		edx
			//or eax, eax
			//jz		Loop1xx3
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEDX);
			TmpEDX++;
			if (TmpEAX == 0) goto Loop1xx3;
		Loop1xx2 : 
			//mov		bl, [edx + 1]
			//add		edx, ebx
			//add		edx, 2
			//dec		eax
			//jnz		Loop1xx2
			TmpBL = *reinterpret_cast<unsigned char *>(TmpEDX + 1);
			TmpEDX += TmpEBX;
			TmpEDX += 2;
			TmpEAX--;
			if (TmpEAX != 0) goto Loop1xx2;
		Loop1xx3 : 
			//dec		cx
			//jnz		Loop1xx1
			//sub		edx, PicPtr
			//sub		edx, 4
			//mov		addofs, edx
			TmpCX--;
			if (TmpCX != 0) goto Loop1xx1;
			TmpEDX -= reinterpret_cast<unsigned int>(PicPtr);
			TmpEDX -= 4;
			addofs = TmpEDX;
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x > WindX1)
		{
			int roff = x - WindX1;
			// BoonXRay 06.08.2017			
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
				unsigned short & TmpAX = *reinterpret_cast<unsigned short *>(&TmpEAX);
				unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned int PushTmpEDI;
			ScanLineLoop1 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd1
				//push	edi
				//mov		dl, [esi]
				//inc		esi
				//or dl, dl
				//jz		NextLine1
				//xor     ebx, ebx
				//mov		bx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd1;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				if (TmpDL == 0) goto NextLine1;
				TmpEBX = 0;
				TmpBX = roff;
			BeginLine1 : 
				//mov		cl, [esi]
				//sub		bx, cx
				//sub		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpBX -= TmpCX;
				TmpEDI -= TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping left code
				//cmp		bx, 0
				//jle		ok1_1
				//cmp		bx, cx
				//jl		hdraw1
				if (static_cast<short>(TmpBX) <= 0) goto ok1_1;
				if (TmpBX < TmpCX) goto hdraw1;
				//nothing to draw
				//sub		bx, cx
				//add		esi, ecx
				//sub		edi, ecx
				//dec		dl
				//jnz		BeginLine1
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpBX -= TmpCX;
				TmpESI += TmpECX;
				TmpEDI -= TmpECX;
				TmpDL--;
				if (TmpDL != 0) goto BeginLine1;
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			hdraw1 :			//draw only small part of line
				//sub		cx, bx
				//mov		ax, bx
				//xor		bx, bx
				//add		esi, eax
				//sub		edi, eax
				TmpCX -= TmpBX;
				TmpAX = TmpBX;
				TmpBX = 0;
				TmpESI += TmpEAX;
				TmpEDI -= TmpEAX;
			ok1_1 : 
				//jcxz    Lx21
				if (TmpCX == 0) goto Lx21;
			ok1 :
				//movsb
				*reinterpret_cast<unsigned char *>(TmpEDI) = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpEDI++;
				//sub		edi, 2
				TmpEDI -= 2;
				//dec		cl
				TmpCL--;
				//jnz		ok1
				if (TmpCL != 0) goto ok1;
			Lx21 : 
				//dec		dl
				TmpDL--;
				//jnz		BeginLine1
				if (TmpDL != 0) goto BeginLine1;
			NextLine1 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			ScanLineLoopEnd1 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else if (x - PLX + 1 < WindX)
		{
			int roff = x - WindX + 1;
			int part;
			// BoonXRay 06.08.2017			
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
                unsigned int TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
                unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned int PushTmpEDI;
			ScanLineLoop2 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd2
				//push	edi
				//mov		dl, [esi]
				//inc		esi
				//or dl, dl
				//jz		NextLine2
				//xor		ebx, ebx
				//mov		bx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd2;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				if (TmpDL == 0) goto NextLine2;
				TmpEBX = 0;
				TmpBX = roff;
			BeginLine2 : 
				//mov		cl, [esi]
				//sub		bx, cx
				//sub		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpBX -= TmpCX;
				TmpEDI -= TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping right code
				//cmp		bx, cx
				//jge		ok2
				if (TmpBX >= TmpCX) goto ok2;
				//clipping
				//cmp		bx, 0
				//jle		ntd2
				if (static_cast<short>(TmpBX) <= 0) goto ntd2;
				//partial drawing
				//sub		cx, bx
				//mov		part, ecx
				//mov		cx, bx
				//jcxz    lxsd1_1
				TmpCX -= TmpBX;
				part = TmpECX;
				TmpCX = TmpBX;
				if (TmpCX == 0) goto lxsd1_1;
			lxsd1 : 
				//movsb
				//sub		edi, 2
				//dec		cl
				//jnz		lxsd1
				*reinterpret_cast<unsigned char *>(TmpEDI) = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpEDI++;
				TmpEDI -= 2;
				TmpCL--;
				if (TmpCL != 0) goto lxsd1;
			lxsd1_1 : 
				//add		esi, part
				//jmp		ntd4
				TmpESI += part;
				goto ntd4;
			ntd2 :			//scanning to the next line
				//add		esi, ecx
				TmpESI += TmpECX;
			ntd4 : 
				//dec		dl
				//jz		NextLine2
				TmpDL--;
				if (TmpDL == 0) goto NextLine2;
			ntd22 : 
				//mov		cl, [esi + 1]
				//add		esi, 2
				//add		esi, ecx
				//dec		dl
				//jnz		ntd22
				//jmp		NextLine2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2 + TmpECX;
				TmpDL--;
				if (TmpDL != 0) goto ntd22;
				goto NextLine2;
			ok2 : 
				//sub		bx, cx
				//jcxz    Lx22
				TmpBX -= TmpCX;
				if (TmpCX == 0) goto Lx22;
			lkfr1 : 
				//movsb
				//sub		edi, 2
				//dec		cl
				//jnz		lkfr1
				*reinterpret_cast<unsigned char *>(TmpEDI) = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpEDI++;
				TmpEDI -= 2;
				TmpCL--;
				if (TmpCL != 0) goto lkfr1;
			Lx22 : 
				//dec		dl
				//jnz		BeginLine2
				TmpDL--;
				if (TmpDL != 0) goto BeginLine2;
			NextLine2 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop2
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop2;
			ScanLineLoopEnd2 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else
			// BoonXRay 06.08.2017		
		//__asm
		{
			//push	esi
			//push	edi
			//mov		edi, ScrOfst
			//mov		esi, PicPtr
			//add		esi, addofs
			//xor		ecx, ecx
			//xor		ebx, ebx
			//cld
			unsigned int TmpEDI = ScrOfst;
			unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
            unsigned int TmpEBX = 0, TmpECX = 0;
            unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
			unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned int PushTmpEDI;
		ScanLineLoop :
			//cmp		PLY, 0
			//je		ScanLineLoopEnd
			//push	edi
			//mov		bl, [esi]
			//inc		esi
			//or bl, bl
			//jz		NextLine
			if (PLY == 0) goto ScanLineLoopEnd;
			PushTmpEDI = TmpEDI;
			TmpBL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			if (TmpBL == 0) goto NextLine;
		BeginLine : 
			//mov		cl, [esi]
			//sub		edi, ecx
			//mov		cl, [esi + 1]
			//add		esi, 2
			//jcxz	Lx2
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpEDI -= TmpECX;
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
			TmpESI += 2;			
			if (TmpCX == 0) goto Lx2;
		ghte : 
			//movsb
			//sub		edi, 2
			//dec		cl
			//jnz		ghte
			*reinterpret_cast<unsigned char *>(TmpEDI) = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			TmpEDI++;
			TmpEDI -= 2;
			TmpCL--;
			if (TmpCL != 0) goto ghte;
		Lx2 : 
			//dec		ebx
			//jnz		BeginLine
			TmpEBX--;
			if (TmpEBX != 0) goto BeginLine;
		NextLine : 
			//pop		edi
			//add		edi, ScrWidth
			//dec     PLY
			//jmp		ScanLineLoop
			TmpEDI = PushTmpEDI;
			TmpEDI += ScrWidth;
			PLY--;
			goto ScanLineLoop;
		ScanLineLoopEnd :
			//pop		edi
			//pop		esi
			;
		}
	}
}

//End of RLC with clipping & with palette
static void ShowRLCpal( int x, int y, void* PicPtr, uint8_t * pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x + PLX <= WindX ) || ( x > WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		// BoonXRay 06.08.2017
		//__asm
		{
			//mov		edx, PicPtr
			//add		edx, 4
			//xor eax, eax
			//mov		ecx, WindY
			//sub		ecx, y
			//xor		eax, eax
			//xor		ebx, ebx
			unsigned int TmpEDX = reinterpret_cast<unsigned int>(PicPtr) + 4;
			unsigned int TmpECX = WindY - y;
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned int TmpEAX = 0, TmpEBX = 0;
			unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
		Loop1xx1 : 
			//mov		al, [edx]
			//inc		edx
			//or eax, eax
			//jz		Loop1xx3
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEDX);
			TmpEDX++;
			if (TmpEAX == 0) goto Loop1xx3;
		Loop1xx2 : 
			//mov		bl, [edx + 1]
			//add		edx, ebx
			//add		edx, 2
			//dec		eax
			//jnz		Loop1xx2
			TmpBL = *reinterpret_cast<unsigned char *>(TmpEDX + 1);
			TmpEDX += TmpEBX;
			TmpEDX += 2;
			TmpEAX--;
			if (TmpEAX != 0) goto Loop1xx2;
		Loop1xx3 : 
			//dec		cx
			//jnz		Loop1xx1
			//sub		edx, PicPtr
			//sub		edx, 4
			//mov		addofs, edx
			TmpCX--;
			if (TmpCX != 0) goto Loop1xx1;
			TmpEDX -= reinterpret_cast<unsigned int>(PicPtr);
			TmpEDX -= 4;
			addofs = TmpEDX;
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
    uint8_t Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x < WindX)
		{
			int roff = WindX - x;
			// BoonXRay 06.08.2017			
			//__asm
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//mov		ebx, pal
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned short & TmpAX = *reinterpret_cast<unsigned short *>(&TmpEAX);
				//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned short & TmpDX = *reinterpret_cast<unsigned short *>(&TmpEDX);
				unsigned int PushTmpEDI;
				TmpEBX = reinterpret_cast<unsigned int>(pal);
			ScanLineLoop1 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd1
				//push	edi
				//mov		dl, [esi]
				//mov		Acm, dl
				//inc		esi
				//or dl, dl
				//jz		NextLine1
				//xor		edx, edx
				//xor     ecx, ecx
				//mov		dx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd1;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				Acm = TmpDL;
				TmpESI++;
				if (TmpDL == 0) goto NextLine1;
				TmpEDX = 0;
				TmpECX = 0;
				TmpDX = roff;
			BeginLine1 : 
				//mov		cl, [esi]
				//sub		dx, cx
				//add		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpDX -= TmpCX;
				TmpEDI += TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping left code
				//cmp		dx, 0
				//jle		ok1_1
				//cmp		dx, cx
				//jl		hdraw1
				if (static_cast<short>(TmpDX) <= 0) goto ok1_1;
				if (TmpDX < TmpCX) goto hdraw1;
				//nothing to draw
				//sub		dx, cx
				//add		esi, ecx
				//add		edi, ecx
				//dec		Acm
				//jnz		BeginLine1
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpDX -= TmpCX;
				TmpESI += TmpECX;
				TmpEDI += TmpECX;
				Acm--;
				if (Acm != 0) goto BeginLine1;
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			hdraw1 :			//draw only small part of line
				//sub		cx, dx
				//mov		ax, dx
				//xor		dx, dx
				//add		esi, eax
				//add		edi, eax
				TmpCX -= TmpDX;
				TmpAX = TmpDX;
				TmpDX = 0;
				TmpESI += TmpEAX;
				TmpEDI += TmpEAX;
			ok1_1 : 
				//jcxz    Lx21
				if (TmpCX == 0) goto Lx21;
			ok1 :
				//lodsb
				//mov		al, [ebx + eax]
				//stosb
				//dec		cl
				//jnz		ok1
				TmpAL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI++;
				TmpCL--;
				if (TmpCL != 0) goto ok1;
			Lx21 : 
				//dec		Acm
				//jnz		BeginLine1
				Acm--;
				if (Acm != 0) goto BeginLine1;
			NextLine1 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			ScanLineLoopEnd1 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else if (x + PLX >= WindX1)
		{
			int roff = WindX1 - x + 1;
			int part;
            // BoonXRay 06.08.2017
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//mov		ebx, pal
				//cld

				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
                unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned short & TmpDX = *reinterpret_cast<unsigned short *>(&TmpEDX);
				unsigned int PushTmpEDI;
				TmpEBX = reinterpret_cast<unsigned int>(pal);
			ScanLineLoop2 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd2
				//push	edi
				//mov		dl, [esi]
				//mov		Acm, dl
				//inc		esi
				//or dl, dl
				//jz		NextLine2
				//xor		edx, edx
				//xor     ecx, ecx
				//mov		dx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd2;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				Acm = TmpDL;
				TmpESI++;
				if (TmpDL == 0) goto NextLine2;
				TmpEDX = 0;
				TmpECX = 0;
				TmpDX = roff;
			BeginLine2 : 
				//mov		cl, [esi]
				//sub		dx, cx
				//add		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpDX -= TmpCX;
				TmpEDI += TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping right code
				//cmp		dx, cx
				//jge		ok2
				if (TmpDX >= TmpCX) goto ok2;
				//clipping
				//cmp		dx, 0
				//jle		ntd2
				if (static_cast<short>(TmpDX) <= 0) goto ntd2;
				//partial drawing
				//sub		cx, dx
				//mov		part, ecx
				//mov		cx, dx
				//jcxz	kkj1_2
				TmpCX -= TmpDX;
				part = TmpECX;
				TmpCX = TmpDX;
				if (TmpCX == 0) goto kkj1_2;
			kkj1 : 
				//lodsb
				//mov		al, [ebx + eax]
				//stosb
				//dec		cl
				//jnz		kkj1
				TmpAL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI++;
				TmpCL--;
				if (TmpCL != 0) goto kkj1;
			kkj1_2 : 
				//add		esi, part
				//jmp		ntd4
				TmpESI += part;
				goto ntd4;
			ntd2 :			//scanning to the next line
				//add		esi, ecx
				TmpESI += TmpECX;
			ntd4 : 
				//dec		Acm
				//jz		NextLine2
				Acm--;
				if (Acm != 0) goto NextLine2;
			ntd22 : 
				//mov		cl, [esi + 1]
				//add		esi, 2
				//add		esi, ecx
				//dec		Acm
				//jnz		ntd22
				//jmp		NextLine2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2 + TmpECX;
				Acm--;
				if (Acm != 0) goto ntd22;
				goto NextLine2;
			ok2 : 
				//sub		dx, cx
				//jcxz	Lx22
				TmpDX -= TmpCX;
				if (TmpCX == 0) goto Lx22;
			kkj2 : 
				//lodsb
				//mov		al, [ebx + eax]
				//stosb
				//dec		cl
				//jnz		kkj2
				TmpAL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI++;
				TmpCL--;
				if (TmpCL != 0) goto kkj2;
			Lx22 : 
				//dec		Acm
				//jnz		BeginLine2
				Acm--;
				if (Acm != 0) goto BeginLine2;
				
			NextLine2 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop2
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop2;
			ScanLineLoopEnd2 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else
            // BoonXRay 06.08.2017
		//__asm
		{
			//push	esi
			//push	edi
			//mov		edi, ScrOfst
			//mov		esi, PicPtr
			//add		esi, addofs
			//xor		ecx, ecx
			//xor		ebx, ebx
			//xor		eax, eax
			//mov		ebx, pal
			//cld
			unsigned int TmpEDI = ScrOfst;
			unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
			unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
            unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
			unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
            unsigned int PushTmpEDI;
			TmpEBX = reinterpret_cast<unsigned int>(pal);
		ScanLineLoop :
			//cmp		PLY, 0
			//je		ScanLineLoopEnd
			//push	edi
			//mov		dl, [esi]
			//inc		esi
			//or dl, dl
			//jz		NextLine
			if (PLY == 0) goto ScanLineLoopEnd;
			PushTmpEDI = TmpEDI;
			TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			if (TmpDL == 0) goto NextLine;
		BeginLine : 
			//mov		cl, [esi]
			//add		edi, ecx
			//mov		cl, [esi + 1]
			//add		esi, 2
			//jcxz	Lx2
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpEDI += TmpECX;
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
			TmpESI += 2;
			if (TmpCX == 0) goto Lx2;
		hgaw : 
			//lodsb
			//mov		al, [ebx + eax]
			//stosb
			//dec		cl
			//jnz		hgaw
			TmpAL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
			*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
			TmpEDI++;
			TmpCL--;
			if (TmpCL != 0) goto hgaw;
		Lx2 : 
			//dec		dl
			//jnz		BeginLine
			TmpDL--;
			if (TmpDL != 0) goto BeginLine;
		NextLine : 
			//pop		edi
			//add		edi, ScrWidth
			//dec     PLY
			//jmp		ScanLineLoop
			TmpEDI = PushTmpEDI;
			TmpEDI += ScrWidth;
			PLY--;
			goto ScanLineLoop;
		ScanLineLoopEnd :
			//pop		edi
			//pop		esi
			;
		}
	}
}

//End of RLC with clipping & encoding
//Showing inverse RLC image with clipping & encodint
static void ShowRLCipal( int x, int y, void* PicPtr, uint8_t * pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x < WindX ) || ( x - PLX + 1 >= WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
        // BoonXRay 09.08.2017
		//__asm
		{
			//mov		edx, PicPtr
			//add		edx, 4
			//xor eax, eax
			//mov		ecx, WindY
			//sub		ecx, y
			//xor		eax, eax
			//xor		ebx, ebx
			unsigned int TmpEDX = reinterpret_cast<unsigned int>(PicPtr) + 4;
			unsigned int TmpECX = WindY - y;
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned int TmpEAX = 0, TmpEBX = 0;
			unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
		Loop1xx1 : 
			//mov		al, [edx]
			//inc		edx
			//or eax, eax
			//jz		Loop1xx3
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEDX);
			TmpEDX++;
			if (TmpEAX == 0) goto Loop1xx3;
		Loop1xx2 : 
			//mov		bl, [edx + 1]
			//add		edx, ebx
			//add		edx, 2
			//dec		eax
			//jnz		Loop1xx2
			TmpBL = *reinterpret_cast<unsigned char *>(TmpEDX + 1);
			TmpEDX += TmpEBX;
			TmpEDX += 2;
			TmpEAX--;
			if (TmpEAX != 0) goto Loop1xx2;
		Loop1xx3 : 
			//dec		cx
			//jnz		Loop1xx1
			//sub		edx, PicPtr
			//sub		edx, 4
			//mov		addofs, edx
			TmpCX--;
			if (TmpCX != 0) goto Loop1xx1;
			TmpEDX -= reinterpret_cast<unsigned int>(PicPtr);
			TmpEDX -= 4;
			addofs = TmpEDX;
		}

		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
    uint8_t Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x > WindX1)
		{
			int roff = x - WindX1;
            // BoonXRay 09.08.2017
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//mov		ebx, pal
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned short & TmpAX = *reinterpret_cast<unsigned short *>(&TmpEAX);
				//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned short & TmpDX = *reinterpret_cast<unsigned short *>(&TmpEDX);
				unsigned int PushTmpEDI;
				TmpEBX = reinterpret_cast<unsigned int>(pal);
			ScanLineLoop1 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd1
				//push	edi
				//mov		dl, [esi]
				//mov		Acm, dl
				//inc		esi
				//or dl, dl
				//jz		NextLine1
				//xor     edx, edx
				//xor		ecx, ecx
				//mov		dx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd1;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				Acm = TmpDL;
				TmpESI++;
				if (TmpDL == 0) goto NextLine1;
				TmpEDX = 0;
				TmpECX = 0;
				TmpDX = roff;
			BeginLine1 : 
				//mov		cl, [esi]
				//sub		dx, cx
				//sub		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpDX -= TmpCX;
				TmpEDI -= TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping left code
				//cmp		dx, 0
				//jle		ok1_1
				//cmp		dx, cx
				//jl		hdraw1
				if (static_cast<short>(TmpDX) <= 0) goto ok1_1;
				if (TmpDX < TmpCX) goto hdraw1;
				//nothing to draw
				//sub		dx, cx
				//add		esi, ecx
				//sub		edi, ecx
				//dec		Acm
				//jnz		BeginLine1
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpDX -= TmpCX;
				TmpESI += TmpECX;
				TmpEDI -= TmpECX;
				Acm--;
				if (Acm != 0) goto BeginLine1;
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			hdraw1 :			//draw only small part of line
				//sub		cx, dx
				//mov		ax, dx
				//xor		edx, edx
				//add		esi, eax
				//sub		edi, eax
				TmpCX -= TmpDX;
				TmpAX = TmpDX;
				TmpEDX = 0;
				TmpESI += TmpEAX;
				TmpEDI -= TmpEAX;
			ok1_1 : 
				//jcxz	Lx21
				if (TmpCX == 0) goto Lx21;
			ok1 : 
				//lodsb
				//mov		al, [ebx + eax]
				//mov[edi], al
				//dec		edi
				//dec		cl
				//jnz		ok1
				TmpAL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI--;
				TmpCL--;
				if (TmpCL != 0) goto ok1;
			Lx21 : 
				//dec		Acm
				//jnz		BeginLine1
				Acm--;
				if (Acm != 0) goto BeginLine1;
			NextLine1 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			ScanLineLoopEnd1 :
				//pop		edi
				//pop		esi
				;
            }

		}
		else if (x - PLX + 1 < WindX)
		{
			int roff = x - WindX + 1;
			int part;
            // BoonXRay 09.08.2017
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//mov		ebx, pal
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
                unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned short & TmpDX = *reinterpret_cast<unsigned short *>(&TmpEDX);
				unsigned int PushTmpEDI;
				TmpEBX = reinterpret_cast<unsigned int>(pal);
			ScanLineLoop2 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd2
				//push	edi
				//mov		dl, [esi]
				//mov		Acm, dl
				//inc		esi
				//or dl, dl
				//jz		NextLine2
				//xor		edx, edx
				//xor		ecx, ecx
				//mov		dx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd2;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				Acm = TmpDL;
				TmpESI++;
				if (TmpDL == 0) goto NextLine2;
				TmpEDX = 0;
				TmpECX = 0;
				TmpDX = roff;
			BeginLine2 : 
				//mov		cl, [esi]
				//sub		dx, cx
				//sub		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpDX -= TmpCX;
				TmpEDI -= TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping right code
				//cmp		dx, cx
				//jge		ok2
				if (TmpDX >= TmpCX) goto ok2;
				//clipping
				//cmp		dx, 0
				//jle		ntd2
				if (static_cast<short>(TmpDX) <= 0) goto ntd2;
				//partial drawing
				//sub		cx, dx
				//mov		part, ecx
				//mov		cx, dx
				//jcxz    lxsd1_1
				TmpCX -= TmpDX;
				part = TmpECX;
				TmpCX = TmpDX;
				if (TmpCX == 0) goto lxsd1_1;
			lxsd1 : 
				//lodsb
				//mov		al, [ebx + eax]
				//mov[edi], al
				//dec		edi
				//dec		cx
				//jnz		lxsd1
				TmpAL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI--;
				TmpCL--;
				if (TmpCL != 0) goto lxsd1;
			lxsd1_1 : 
				//add		esi, part
				//jmp		ntd4
				TmpESI += part;
				goto ntd4;
			ntd2 :			//scanning to the next line
				//add		esi, ecx
				TmpESI += TmpECX;
			ntd4 : 
				//dec		Acm
				//jz		NextLine2
				Acm--;
				if (Acm != 0) goto NextLine2;
			ntd22 : 
				//mov		cl, [esi + 1]
				//add		esi, 2
				//add		esi, ecx
				//dec		Acm
				//jnz		ntd22
				//jmp		NextLine2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2 + TmpECX;
				Acm--;
				if (Acm != 0) goto ntd22;
				goto NextLine2;
			ok2 : 
				//sub		dx, cx
				//jcxz	Lx22
				TmpDX -= TmpCX;
				if (TmpCX == 0) goto Lx22;
			lkfr1 : 
				//lodsb
				//mov		al, [ebx + eax]
				//mov[edi], al
				//dec		edi
				//dec		cl
				//jnz		lkfr1
				TmpAL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI--;
				TmpCL--;
				if (TmpCL != 0) goto lkfr1;
			Lx22 : 
				//dec		Acm
				//jnz		BeginLine2
				Acm--;
				if (Acm != 0) goto BeginLine2;
			NextLine2 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop2
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop2;
			ScanLineLoopEnd2 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else
            // BoonXRay 09.08.2017
		//__asm
		{
			//push	esi
			//push	edi
			//mov		edi, ScrOfst
			//mov		esi, PicPtr
			//add		esi, addofs
			//xor		ecx, ecx
			//xor		ebx, ebx
			//xor		eax, eax
			//mov		ebx, pal
			//cld
			unsigned int TmpEDI = ScrOfst;
			unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
			unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
            unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
			unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
            unsigned int PushTmpEDI;
			TmpEBX = reinterpret_cast<unsigned int>(pal);
		ScanLineLoop :
			//cmp		PLY, 0
			//je		ScanLineLoopEnd
			//push	edi
			//mov		dl, [esi]
			//inc		esi
			//or dl, dl
			//jz		NextLine
			if (PLY == 0) goto ScanLineLoopEnd;
			PushTmpEDI = TmpEDI;
			TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			if (TmpDL == 0) goto NextLine;
		BeginLine : 
			//mov		cl, [esi]
			//sub		edi, ecx
			//mov		cl, [esi + 1]
			//add		esi, 2
			//jcxz	Lx2
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpEDI -= TmpECX;
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
			TmpESI += 2;
			if (TmpCX == 0) goto Lx2;
		ghte : 
			//lodsb
			//mov		al, [eax + ebx]
			//mov[edi], al
			//dec		edi
			//dec		cl
			//jnz		ghte
			TmpAL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
			*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
			TmpEDI--;
			TmpCL--;
			if (TmpCL != 0) goto ghte;
		Lx2 : 
			//dec		dl
			//jnz		BeginLine
			TmpDL--;
			if (TmpDL != 0) goto BeginLine;
		NextLine : 
			//pop		edi
			//add		edi, ScrWidth
			//dec     PLY
			//jmp		ScanLineLoop
			TmpEDI = PushTmpEDI;
			TmpEDI += ScrWidth;
			PLY--;
			goto ScanLineLoop;
		ScanLineLoopEnd :
			//pop		edi
			//pop		esi
			;
		}
	}
}
//End of inverted RLC with clipping & encoding

//End of RLC with clipping & with palette(half-transparent fog)
static void ShowRLChtpal( int x, int y, void* PicPtr, uint8_t * pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x + PLX <= WindX ) || ( x > WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
        // BoonXRay 10.08.2017
		//__asm
		{
			//mov		edx, PicPtr
			//add		edx, 4
			//xor eax, eax
			//mov		ecx, WindY
			//sub		ecx, y
			//xor		eax, eax
			//xor		ebx, ebx
			unsigned int TmpEDX = reinterpret_cast<unsigned int>(PicPtr) + 4;
			unsigned int TmpECX = WindY - y;
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned int TmpEAX = 0, TmpEBX = 0;
			unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
		Loop1xx1 : 
			//mov		al, [edx]
			//inc		edx
			//or eax, eax
			//jz		Loop1xx3
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEDX);
			TmpEDX++;
			if (TmpEAX == 0) goto Loop1xx3;
		Loop1xx2 : 
			//mov		bl, [edx + 1]
			//add		edx, ebx
			//add		edx, 2
			//dec		eax
			//jnz		Loop1xx2
			TmpBL = *reinterpret_cast<unsigned char *>(TmpEDX + 1);
			TmpEDX += TmpEBX;
			TmpEDX += 2;
			TmpEAX--;
			if (TmpEAX != 0) goto Loop1xx2;
		Loop1xx3 :
			//dec		cx
			//jnz		Loop1xx1
			//sub		edx, PicPtr
			//sub		edx, 4
			//mov		addofs, edx
			TmpCX--;
			if (TmpCX != 0) goto Loop1xx1;
			TmpEDX -= reinterpret_cast<unsigned int>(PicPtr);
			TmpEDX -= 4;
			addofs = TmpEDX;
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
    uint8_t Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x < WindX)
		{
			int roff = WindX - x;
            // BoonXRay 10.08.2017
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//mov		ebx, pal
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char & TmpAH = *(reinterpret_cast<unsigned char *>(&TmpEAX) + 1);
				unsigned short & TmpAX = *reinterpret_cast<unsigned short *>(&TmpEAX);
				//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned short & TmpDX = *reinterpret_cast<unsigned short *>(&TmpEDX);
				unsigned int PushTmpEDI;
				TmpEBX = reinterpret_cast<unsigned int>(pal);
			ScanLineLoop1 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd1
				//push	edi
				//mov		dl, [esi]
				//mov		Acm, dl
				//inc		esi
				//or dl, dl
				//jz		NextLine1
				//xor		edx, edx
				//xor		ecx, ecx
				//mov		dx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd1;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				Acm = TmpDL;
				TmpESI++;
				if (TmpDL == 0) goto NextLine1;
				TmpEDX = 0;
				TmpECX = 0;
				TmpDX = roff;
			BeginLine1 : 
				//mov		cl, [esi]
				//sub		dx, cx
				//add		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpDX -= TmpCX;
				TmpEDI += TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping left code
				//cmp		dx, 0
				//jle		ok1_1
				//cmp		dx, cx
				//jl		hdraw1
				if (static_cast<short>(TmpDX) <= 0) goto ok1_1;
				if (TmpDX < TmpCX) goto hdraw1;
				//nothing to draw
				//sub		dx, cx
				//add		esi, ecx
				//add		edi, ecx
				//dec		Acm
				//jnz		BeginLine1
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpDX -= TmpCX;
				TmpESI += TmpECX;
				TmpEDI += TmpECX;
				Acm--;
				if (Acm != 0) goto BeginLine1;
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			hdraw1 :			//draw only small part of line
				//sub		cx, dx
				//xor		eax, eax
				//mov		ax, dx
				//xor		edx, edx
				//add		esi, eax
				//add		edi, eax
				TmpCX -= TmpDX;
				TmpEAX = 0;
				TmpAX = TmpDX;
				TmpEDX = 0;
				TmpESI += TmpEAX;
				TmpEDI += TmpEAX;
			ok1_1 : 
				//jcxz	Lx21
				if (TmpCX == 0) goto Lx21;
			ok1 :
				//mov		ah, [esi]
				//inc		esi
				//mov		al, [edi]
				//mov		al, [ebx + eax]
				//stosb
				//dec		cl
				//jnz		ok1
				TmpAH = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEDI);
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI++;
				TmpCL--;
				if (TmpCL != 0) goto ok1;
			Lx21 :
				//dec		Acm
				//jnz		BeginLine1
				Acm--;
				if (Acm != 0) goto BeginLine1;
			NextLine1 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			ScanLineLoopEnd1 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else if (x + PLX >= WindX1)
		{
			int roff = WindX1 - x + 1;
			int part;
            // BoonXRay 10.08.2017
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//mov		ebx, pal
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
                unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char & TmpAH = *(reinterpret_cast<unsigned char *>(&TmpEAX) + 1);
				//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned short & TmpDX = *reinterpret_cast<unsigned short *>(&TmpEDX);
				unsigned int PushTmpEDI;
				TmpEBX = reinterpret_cast<unsigned int>(pal);
			ScanLineLoop2 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd2
				//push	edi
				//mov		dl, [esi]
				//mov		Acm, dl
				//inc		esi
				//or dl, dl
				//jz		NextLine2
				//xor		edx, edx
				//xor		ecx, ecx
				//mov		dx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd2;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				Acm = TmpDL;
				TmpESI++;
				if (TmpDL == 0) goto NextLine2;
				TmpEDX = 0;
				TmpECX = 0;
				TmpDX = roff;
			BeginLine2 : 
				//mov		cl, [esi]
				//sub		dx, cx
				//add		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpDX -= TmpCX;
				TmpEDI += TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping right code
				//cmp		dx, cx
				//jge		ok2
				if (TmpDX >= TmpCX) goto ok2;
				//clipping
				//cmp		dx, 0
				//jle		ntd2
				if (static_cast<short>(TmpDX) <= 0) goto ntd2;
				//partial drawing
				//sub		cx, dx
				//mov		part, ecx
				//mov		cx, dx
				//jcxz    kkj1_1
				TmpCX -= TmpDX;
				part = TmpECX;
				TmpCX = TmpDX;
				if (TmpCX == 0) goto kkj1_1;
			kkj1 : 
				//mov		ah, [esi]
				//inc		esi
				//mov		al, [edi]
				//mov		al, [ebx + eax]
				//stosb
				//dec		cl
				//jnz		kkj1
				TmpAH = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEDI);
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI++;
				TmpCL--;
				if (TmpCL != 0) goto kkj1;
			kkj1_1 : 
				//add		esi, part
				//jmp		ntd4
				TmpESI += part;
				goto ntd4;
			ntd2 :			//scanning to the next line
				//add		esi, ecx
				TmpESI += TmpECX;
			ntd4 : 
				//dec		Acm
				//jz		NextLine2
				Acm--;
				if (Acm != 0) goto NextLine2;
			ntd22 : 
				//mov		cl, [esi + 1]
				//add		esi, 2
				//add		esi, ecx
				//dec		Acm
				//jnz		ntd22
				//jmp		NextLine2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2 + TmpECX;
				Acm--;
				if (Acm != 0) goto ntd22;
				goto NextLine2;
			ok2 : 
				//jcxz	Lx22
				//sub		dx, cx
				if (TmpCX == 0) goto Lx22;
				TmpDX -= TmpCX;
			kkj4 : 
				//mov		ah, [esi]
				//inc		esi
				//mov		al, [edi]
				//mov		al, [ebx + eax]
				//stosb
				//dec		cl
				//jnz		kkj4
				TmpAH = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEDI);
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI++;
				TmpCL--;
				if (TmpCL != 0) goto kkj4;
			Lx22 :
				//dec		Acm
				//jnz		BeginLine2
				Acm--;
				if (Acm != 0) goto BeginLine2;
			NextLine2 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop2
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop2;
			ScanLineLoopEnd2 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else
            // BoonXRay 10.08.2017
		//__asm
		{
			//push	esi
			//push	edi
			//mov		edi, ScrOfst
			//mov		esi, PicPtr
			//add		esi, addofs
			//xor		ecx, ecx
			//xor		ebx, ebx
			//xor		eax, eax
			//mov		ebx, pal
			//cld
			unsigned int TmpEDI = ScrOfst;
			unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
			unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
            unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpAH = *(reinterpret_cast<unsigned char *>(&TmpEAX) + 1);
			//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
			unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
            unsigned int PushTmpEDI;
			TmpEBX = reinterpret_cast<unsigned int>(pal);
		ScanLineLoop :
			//cmp		PLY, 0
			//je		ScanLineLoopEnd
			//push	edi
			//mov		dl, [esi]
			//inc		esi
			//or dl, dl
			//jz		NextLine
			if (PLY == 0) goto ScanLineLoopEnd;
			PushTmpEDI = TmpEDI;
			TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			if (TmpDL == 0) goto NextLine;
		BeginLine : 
			//mov		cl, [esi]
			//add		edi, ecx
			//mov		cl, [esi + 1]
			//add		esi, 2
			//jcxz    Lx2
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpEDI += TmpECX;
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
			TmpESI += 2;
			if (TmpCX == 0) goto Lx2;
		hgaw : 
			//mov		ah, [esi]
			//inc		esi
			//mov		al, [edi]
			//mov		al, [ebx + eax]
			//stosb
			//dec		cl
			//jnz		hgaw
			TmpAH = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEDI);
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
			*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
			TmpEDI++;
			TmpCL--;
			if (TmpCL != 0) goto hgaw;
		Lx2 : 
			//dec		dl
			//jnz		BeginLine
			TmpDL--;
			if (TmpDL != 0) goto BeginLine;
		NextLine : 
			//pop		edi
			//add		edi, ScrWidth
			//dec     PLY
			//jmp		ScanLineLoop
			TmpEDI = PushTmpEDI;
			TmpEDI += ScrWidth;
			PLY--;
			goto ScanLineLoop;
		ScanLineLoopEnd :
			//pop		edi
			//pop		esi
			;
		}
	}
}
//End of RLC with clipping & encoding
//Showing inverse RLC image with clipping & encodint(half-transparent fog)
static void ShowRLCihtpal( int x, int y, void* PicPtr, uint8_t * pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x < WindX ) || ( x - PLX + 1 >= WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
        // BoonXRay 10.08.2017
		//__asm
		{
			//mov		edx, PicPtr
			//add		edx, 4
			//xor eax, eax
			//mov		ecx, WindY
			//sub		ecx, y
			//xor		eax, eax
			//xor		ebx, ebx
			unsigned int TmpEDX = reinterpret_cast<unsigned int>(PicPtr) + 4;
			unsigned int TmpECX = WindY - y;
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned int TmpEAX = 0, TmpEBX = 0;
			unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
		Loop1xx1 : 
			//mov		al, [edx]
			//inc		edx
			//or eax, eax
			//jz		Loop1xx3
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEDX);
			TmpEDX++;
			if (TmpEAX == 0) goto Loop1xx3;
		Loop1xx2 : 
			//mov		bl, [edx + 1]
			//add		edx, ebx
			//add		edx, 2
			//dec		eax
			//jnz		Loop1xx2
			TmpBL = *reinterpret_cast<unsigned char *>(TmpEDX + 1);
			TmpEDX += TmpEBX;
			TmpEDX += 2;
			TmpEAX--;
			if (TmpEAX != 0) goto Loop1xx2;
		Loop1xx3 : 
			//dec		cx
			//jnz		Loop1xx1
			//sub		edx, PicPtr
			//sub		edx, 4
			//mov		addofs, edx
			TmpCX--;
			if (TmpCX != 0) goto Loop1xx1;
			TmpEDX -= reinterpret_cast<unsigned int>(PicPtr);
			TmpEDX -= 4;
			addofs = TmpEDX;
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
    uint8_t Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x > WindX1)
		{
			int roff = x - WindX1;
            // BoonXRay 10.08.2017
			//__asm
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//mov		ebx, pal
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char & TmpAH = *(reinterpret_cast<unsigned char *>(&TmpEAX) + 1);
				unsigned short & TmpAX = *reinterpret_cast<unsigned short *>(&TmpEAX);
				//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned short & TmpDX = *reinterpret_cast<unsigned short *>(&TmpEDX);
				unsigned int PushTmpEDI;
				TmpEBX = reinterpret_cast<unsigned int>(pal);
			ScanLineLoop1 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd1
				//push	edi
				//mov		dl, [esi]
				//mov		Acm, dl
				//inc		esi
				//or dl, dl
				//jz		NextLine1
				//xor		edx, edx
				//xor		ecx, ecx
				//mov		dx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd1;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				Acm = TmpDL;
				TmpESI++;
				if (TmpDL == 0) goto NextLine1;
				TmpEDX = 0;
				TmpECX = 0;
				TmpDX = roff;
			BeginLine1 : 
				//mov		cl, [esi]
				//sub		dx, cx
				//sub		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpDX -= TmpCX;
				TmpEDI -= TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping left code
				//cmp		dx, 0
				//jle		ok1_1
				//cmp		dx, cx
				//jl		hdraw1
				if (static_cast<short>(TmpDX) <= 0) goto ok1_1;
				if (TmpDX < TmpCX) goto hdraw1;
				//nothing to draw
				//sub		dx, cx
				//add		esi, ecx
				//sub		edi, ecx
				//dec		Acm
				//jnz		BeginLine1
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpDX -= TmpCX;
				TmpESI += TmpECX;
				TmpEDI -= TmpECX;
				Acm--;
				if (Acm != 0) goto BeginLine1;
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			hdraw1 :			//draw only small part of line
				//xor		eax, eax
				//sub		cx, dx
				//mov		ax, dx
				//xor		edx, edx
				//add		esi, eax
				//sub		edi, eax
				TmpCX -= TmpDX;
				TmpEAX = 0;
				TmpAX = TmpDX;
				TmpEDX = 0;
				TmpESI += TmpEAX;
				TmpEDI -= TmpEAX;
			ok1_1 : 
				//jcxz	Lx21
				if (TmpCX == 0) goto Lx21;
			ok1 : 
				//mov		ah, [esi]
				//inc		esi
				//mov		al, [edi]
				//mov		al, [ebx + eax]
				//mov[edi], al
				//dec		edi
				//dec		cl
				//jnz		ok1
				TmpAH = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEDI);
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI--;
				TmpCL--;
				if (TmpCL != 0) goto ok1;
			Lx21 : 
				//dec		Acm
				//jnz		BeginLine1
				Acm--;
				if (Acm != 0) goto BeginLine1;
			NextLine1 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop1
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop1;
			ScanLineLoopEnd1 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else if (x - PLX + 1 < WindX)
		{
			int roff = x - WindX + 1;
			int part;
            // BoonXRay 10.08.2017
			//__asm 
			{
				//push	esi
				//push	edi
				//mov		edi, ScrOfst
				//mov		esi, PicPtr
				//add		esi, addofs
				//xor		ecx, ecx
				//xor		eax, eax
				//mov		ebx, pal
				//cld
				unsigned int TmpEDI = ScrOfst;
				unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
				unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
                unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char & TmpAH = *(reinterpret_cast<unsigned char *>(&TmpEAX) + 1);
				//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
				unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
				unsigned short & TmpDX = *reinterpret_cast<unsigned short *>(&TmpEDX);
				unsigned int PushTmpEDI;
				TmpEBX = reinterpret_cast<unsigned int>(pal);
			ScanLineLoop2 :
				//cmp		PLY, 0
				//je		ScanLineLoopEnd2
				//push	edi
				//mov		dl, [esi]
				//mov		Acm, dl
				//inc		esi
				//or dl, dl
				//jz		NextLine2
				//xor		ecx, ecx
				//xor		edx, edx
				//mov		dx, word ptr roff
				if (PLY == 0) goto ScanLineLoopEnd2;
				PushTmpEDI = TmpEDI;
				TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
				Acm = TmpDL;
				TmpESI++;
				if (TmpDL == 0) goto NextLine2;
				TmpEDX = 0;
				TmpECX = 0;
				TmpDX = roff;
			BeginLine2 : 
				//mov		cl, [esi]
				//sub		dx, cx
				//sub		edi, ecx
				//mov		cl, [esi + 1]
				//add		esi, 2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpDX -= TmpCX;
				TmpEDI -= TmpECX;
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2;
				//clipping right code
				//cmp		dx, cx
				//jge		ok2
				if (TmpDX >= TmpCX) goto ok2;
				//clipping
				//cmp		dx, 0
				//jle		ntd2
				if (static_cast<short>(TmpDX) <= 0) goto ntd2;
				//partial drawing
				//sub		cx, dx
				//mov		part, ecx
				//mov		cx, dx
				//jcxz	lxsd1_1
				TmpCX -= TmpDX;
				part = TmpECX;
				TmpCX = TmpDX;
				if (TmpCX == 0) goto lxsd1_1;
			lxsd1 : 
				//mov		ah, [esi]
				//inc		esi
				//mov		al, [edi]
				//mov		al, [ebx + eax]
				//mov[edi], al
				//dec		edi
				//dec		cl
				//jnz		lxsd1
				TmpAH = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEDI);
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI--;
				TmpCL--;
				if (TmpCL != 0) goto lxsd1;
			lxsd1_1 : 
				//add		esi, part
				//jmp		ntd4
				TmpESI += part;
				goto ntd4;
			ntd2 :			//scanning to the next line
				//add		esi, ecx
				TmpESI += TmpECX;
			ntd4 : 
				//dec		Acm
				//jz		NextLine2
				Acm--;
				if (Acm != 0) goto NextLine2;
			ntd22 : 
				//mov		cl, [esi + 1]
				//add		esi, 2
				//add		esi, ecx
				//dec		Acm
				//jnz		ntd22
				//jmp		NextLine2
				TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
				TmpESI += 2 + TmpECX;
				Acm--;
				if (Acm != 0) goto ntd22;
				goto NextLine2;
			ok2 : 
				//jcxz	Lx22
				//sub		dx, cx
				if (TmpCX == 0) goto Lx22;
				TmpDX -= TmpCX;
			lkfr1 : 
				//mov		ah, [esi]
				//inc		esi
				//mov		al, [edi]
				//mov		al, [ebx + eax]
				//mov[edi], al
				//dec		edi
				//dec		cl
				//jnz		lkfr1
				TmpAH = *reinterpret_cast<unsigned char *>(TmpESI);
				TmpESI++;
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEDI);
				TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
				*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
				TmpEDI--;
				TmpCL--;
				if (TmpCL != 0) goto lkfr1;
			Lx22 : 
				//dec		Acm
				//jnz		BeginLine2
				Acm--;
				if (Acm != 0) goto BeginLine2;
			NextLine2 : 
				//pop		edi
				//add		edi, ScrWidth
				//dec     PLY
				//jmp		ScanLineLoop2
				TmpEDI = PushTmpEDI;
				TmpEDI += ScrWidth;
				PLY--;
				goto ScanLineLoop2;
			ScanLineLoopEnd2 :
				//pop		edi
				//pop		esi
				;
            }
		}
		else
            // BoonXRay 10.08.2017
		//__asm
		{
			//push	esi
			//push	edi
			//mov		edi, ScrOfst
			//mov		esi, PicPtr
			//add		esi, addofs
			//xor		ecx, ecx
			//xor		ebx, ebx
			//xor		eax, eax
			//mov		ebx, pal
			//cld
			unsigned int TmpEDI = ScrOfst;
			unsigned int TmpESI = reinterpret_cast<unsigned int>(PicPtr) + addofs;
			unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
            unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpAH = *(reinterpret_cast<unsigned char *>(&TmpEAX) + 1);
			//unsigned short & TmpBX = *reinterpret_cast<unsigned short *>(&TmpEBX);
			unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
			unsigned short & TmpCX = *reinterpret_cast<unsigned short *>(&TmpECX);
			unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
            unsigned int PushTmpEDI;
			TmpEBX = reinterpret_cast<unsigned int>(pal);
		ScanLineLoop :
			//cmp		PLY, 0
			//je		ScanLineLoopEnd
			//push	edi
			//mov		dl, [esi]
			//inc		esi
			//or dl, dl
			//jz		NextLine
			if (PLY == 0) goto ScanLineLoopEnd;
			PushTmpEDI = TmpEDI;
			TmpDL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			if (TmpDL == 0) goto NextLine;
		BeginLine : 
			//mov		cl, [esi]
			//sub		edi, ecx
			//mov		cl, [esi + 1]
			//add		esi, 2
			//jcxz	Lx2
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpEDI -= TmpECX;
			TmpCL = *reinterpret_cast<unsigned char *>(TmpESI + 1);
			TmpESI += 2;
			if (TmpCX == 0) goto Lx2;
		ghte : 
			//mov		ah, [esi]
			//inc		esi
			//mov		al, [edi]
			//mov		al, [eax + ebx]
			//mov[edi], al
			//dec		edi
			//dec		cl
			//jnz		ghte
			TmpAH = *reinterpret_cast<unsigned char *>(TmpESI);
			TmpESI++;
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEDI);
			TmpAL = *reinterpret_cast<unsigned char *>(TmpEBX + TmpEAX);
			*reinterpret_cast<unsigned char *>(TmpEDI) = TmpAL;
			TmpEDI--;
			TmpCL--;
			if (TmpCL != 0) goto ghte;
		Lx2 : 
			//dec		dl
			//jnz		BeginLine
			TmpDL--;
			if (TmpDL != 0) goto BeginLine;
		NextLine : 
			//pop		edi
			//add		edi, ScrWidth
			//dec     PLY
			//jmp		ScanLineLoop
			TmpEDI = PushTmpEDI;
			TmpEDI += ScrWidth;
			PLY--;
			goto ScanLineLoop;
		ScanLineLoopEnd :
			//pop		edi
			//pop		esi
			;
		}
	}
}
//End of inverted RLC with clipping & encoding(half-transparent fog)

static void ShowRLCp1( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL1 );
}

static void ShowRLCp2( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL2 );
}

static void ShowRLCp3( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL3 );
}

static void ShowRLCp4( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL4 );
}

static void ShowRLCp5( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL5 );
}

static void ShowRLCp6( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL6 );
}

static void ShowRLCp7( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL7 );
}

static void ShowRLCip1( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL1 );
}

static void ShowRLCip2( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL2 );
}

static void ShowRLCip3( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL3 );
}

static void ShowRLCip4( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL4 );
}

static void ShowRLCip5( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL5 );
}

static void ShowRLCip6( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL6 );
}

static void ShowRLCip7( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL7 );
}

void ShowRLCItemPal(int x, int y, RLCTable *lprt, int n, uint8_t * Pal )
{
	if (n < 4096)
	{
		ShowRLCpal( x, y, (void*) ( ( *lprt )->OfsTable[n] ), Pal );
	}
	else
	{
		ShowRLCipal( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ), Pal );
    }
}

void ShowRLCItemGrad(int x, int y, RLCTable *lprt, int n, uint8_t* Pal )
{
	if (n < 4096)
	{
		ShowRLChtpal( x, y, (void*) ( ( *lprt )->OfsTable[n] ), Pal );
	}
	else
	{
		ShowRLCihtpal( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ), Pal );
    }
}

//Load rlc file, allocate and fill provided RLCTable
bool LoadRLC(const char *lpFileName, RLCTable *RLCtbl )
{
	ResFile f1 = RReset( lpFileName );
	if (INVALID_HANDLE_VALUE != f1)
	{
		DWORD fsz = RFileSize( f1 );
		LOADED += fsz;

		*RLCtbl = (RLCTable) malloc( fsz + 4 );
		( *RLCtbl )->Size = fsz + 4;

		RBlockRead( f1, &( ( *RLCtbl )->sign ), fsz );
		RClose( f1 );

		int shft = int( *RLCtbl ) + 4;
		int cnt = ( ( *RLCtbl )->SCount & 65535 );
		for (int i = 0; i < cnt; i++)
		{
			( *RLCtbl )->OfsTable[i] += shft;
		}

		return true;
	}

	char gg[128];
	sprintf( gg, "Could not load RLC file: %s", lpFileName );
	ErrM( gg );

	return false;
}

void ShowRLCItem(int x, int y, RLCTable *lprt, int n, uint8_t nt )
{
	cntr++;
	if (cntr > 64 && !InCycle)
	{
		InCycle = true;
		ProcessMessages();
		InCycle = false;
		cntr = 0;
	}

	int GPID = int( *lprt );

	if (GPID < 4096)
	{
		GPS.ShowGP( x, y, GPID, n, nt );
		return;
	}

	if (n < 4096)
	{
		switch (nt)
		{
		case 1:
			ShowRLCp1( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 2:
			ShowRLCp2( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 3:
			ShowRLCp3( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 4:
			ShowRLCp4( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 5:
			ShowRLCp5( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 6:
			ShowRLCp6( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 7:
			ShowRLCp7( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		default:
			ShowRLC( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
		}
	}
	else
	{
		switch (nt)
		{
		case 1:
			ShowRLCip1( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 2:
			ShowRLCip2( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 3:
			ShowRLCip3( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 4:
			ShowRLCip4( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 5:
			ShowRLCip5( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 6:
			ShowRLCip6( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 7:
			ShowRLCip7( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		default:
			ShowRLCi( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
		}
	}
}

int GetRLCWidth( RLCTable lpr, byte n )
{
	int GPID = int( lpr );
	if (GPID < 4096)
	{
		if (n == 32)
		{
			return GPS.GetGPWidth( GPID, 'c' );
		}
		return GPS.GetGPWidth( GPID, n );
    }
	if (n < lpr->SCount)
	{
		return ( *( (lpRLCHeader) ( (void*) ( lpr->OfsTable[n] ) ) ) ).SizeX;
	}
	else return 0;
}

static int GetCHEX( byte c )
{
	if (c >= '0'&&c <= '9')return c - '0';
	if (c >= 'a'&&c <= 'z')return c + 10 - 'a';
	if (c >= 'A'&&c <= 'Z')return c + 10 - 'A';
	return 0;
}

int GetRLCWidthUNICODE(RLCTable lpr, uint8_t *strptr, int* L )
{
	if (strptr[0] == SIGNBYTE)//FUNNY: Funny shit, Sherlock. DEL, srsly?!
	{
		if (L)*L = 5;
		int idx = ( GetCHEX( strptr[1] ) << 12 ) +
			( GetCHEX( strptr[2] ) << 8 ) +
			( GetCHEX( strptr[3] ) << 4 ) +
			GetCHEX( strptr[4] );
		int GPID = int( lpr );
		if (GPID < 4096)
		{
			UNICODETABLE* UT = GPS.UNITBL[GPID];
			if (!UT)return 0;
			for (int j = 0; j < UT->NTables; j++)
			{
				if (idx >= UT->USET[j].Start)
				{
					int v = idx - UT->USET[j].Start;
					if (v < UT->USET[j].NSymbols)
					{
						v += UT->USET[j].GP_Start;
						return GPS.GetGPWidth( UT->USET[j].GPID, v );
					}
				}
			}
			return 0;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (L)
		{
			*L = 1;
		}
		return GetRLCWidth( lpr, strptr[0] );
	}
}

__declspec( dllexport ) int GetRLCHeight( RLCTable lpr, uint8_t n )
{
	int GPID = int( lpr );
	if (GPID < 4096)
	{
		return GPS.GetGPHeight( GPID, n );
	}
	if (n < lpr->SCount)
	{
		return ( *( (lpRLCHeader) ( (void*) ( lpr->OfsTable[n] ) ) ) ).SizeY;
	}
	else
	{
		return 0;
	}
}

void RegisterRLCFont( lpRLCFont lrf, RLCTable lpr, int fir )
{
	lrf->FirstSymbol = fir;
	lrf->LastSymbol = lpr->SCount + fir - 1;
	lrf->RLC = lpr;
}

void CheckFontColor( lpRLCFont lpf )
{
	int GPID = int( lpf->RLC );
	if (GPID < 4096)GPS.ImageType[GPID] = ( GPS.ImageType[GPID] & 7 ) | lpf->Options;
}

static void ShowChar( int x, int y, char c, lpRLCFont lpf )
{
	int GPID = int( lpf->RLC );
	if (GPID < 4096)
	{
		GPS.ImageType[GPID] = ( GPS.ImageType[GPID] & 7 ) | lpf->Options;
		GPS.ShowGP( x, y, GPID, byte( c ), 0 );
		return;
    }
	ShowRLCItem( x, y, &( lpf->RLC ), c, 0 );
}

void ShowCharUNICODE(int x, int y, uint8_t *strptr, lpRLCFont lpr )
{
	if (strptr[0] == SIGNBYTE)
	{
		int idx = ( GetCHEX( strptr[1] ) << 12 ) +
			( GetCHEX( strptr[2] ) << 8 ) +
			( GetCHEX( strptr[3] ) << 4 ) +
			GetCHEX( strptr[4] );
		int GPID = int( lpr->RLC );
		if (GPID < 4096)
		{
			UNICODETABLE* UT = GPS.UNITBL[GPID];
			if (UT)
			{
				for (int j = 0; j < UT->NTables; j++)
				{
					if (idx >= UT->USET[j].Start)
					{
						int v = idx - UT->USET[j].Start;
						if (v < UT->USET[j].NSymbols)
						{
							v += UT->USET[j].GP_Start;
							GPID = UT->USET[j].GPID;
							if (lpr->ShadowGP != -1)
							{
								GPS.ImageType[lpr->ShadowGP] = 1;
								GPS.ShowGP( x + UT->USET[j].DX + lpr->ShadowDx, y + UT->USET[j].DY + lpr->ShadowDy, GPID, v, 0 );
							}
							if (UT->USET[j].UseColor)
							{
								GPS.ImageType[GPID] = ( GPS.ImageType[GPID] & 7 ) | lpr->Options;
							}
							else
							{
								GPS.ImageType[GPID] = 1;
							}
							GPS.ShowGP( x + UT->USET[j].DX, y + UT->USET[j].DY, GPID, v, 0 );
							return;
						}
					}
				}
			}
		}
	}
	else
	{
		if (lpr->ShadowGP != -1)
		{
			GPS.ImageType[lpr->ShadowGP] = 1;
			GPS.ShowGP( x + lpr->ShadowDx, y + lpr->ShadowDy, lpr->ShadowGP, strptr[0], 0 );
		}
		ShowChar( x, y, strptr[0], lpr );
	}
}

void ShowString(int x, int y, const char *lps, lpRLCFont lpf )
{
	if (nullptr == lps)
	{
		return;
	}

	int GPID = int( lpf->RLC );

	if (GPID < 4096)
	{
		GPS.ImageType[GPID] = ( GPS.ImageType[GPID] & 7 ) | lpf->Options;
		byte ch;
		int i = 0;
		do
		{
			ch = lps[i];
			if (ch != 0)
			{
				ShowCharUNICODE( x, y, (byte*) ( lps + i ), lpf );
				int L = 1;
				x += GetRLCWidthUNICODE( lpf->RLC, (byte*) ( lps + i ), &L );
				i += L - 1;
			}
			i++;
		} while (ch);

		return;
	}

	byte ch;
	int i = 0;
	do
	{
		ch = lps[i];
		if (ch != 0)
		{
			ShowRLCItem( x, y, &( lpf->RLC ), lpf->FirstSymbol + ch, 0 );
			x += GetRLCWidth( lpf->RLC, lpf->FirstSymbol + ch );
		}
		i++;
	} while (ch);
}

int GetRLCStrWidth( const char* str, lpRLCFont lpf )
{
	if (!str)return 0;
	int L = 0;
	int sl = strlen( str );
	for (int i = 0; i < sl; i++)
	{
		int LL = 1;
		L += GetRLCWidthUNICODE( lpf->RLC, (byte*) ( str + i ), &LL );
		i += LL - 1;
    }
	return L;
}

void LoadPalettes()//IMPORTANT: load color palettes for ???
{
	ResFile f1 = RReset( "pal1.dat" );
	RBlockRead( f1, PAL1, 256 );
	RClose( f1 );
	f1 = RReset( "pal2.dat" );
	RBlockRead( f1, PAL2, 256 );
	RClose( f1 );
	f1 = RReset( "pal3.dat" );
	RBlockRead( f1, PAL3, 256 );
	RClose( f1 );
	f1 = RReset( "pal4.dat" );
	RBlockRead( f1, PAL4, 256 );
	RClose( f1 );
	f1 = RReset( "pal5.dat" );
	RBlockRead( f1, PAL5, 256 );
	RClose( f1 );
	f1 = RReset( "pal6.dat" );
	RBlockRead( f1, PAL6, 256 );
	RClose( f1 );
	f1 = RReset( "pal7.dat" );
	RBlockRead( f1, PAL7, 256 );
	RClose( f1 );
}

/* End of Graphics routine */

/* Horisontal and vertical lines,rectangles*/
void Hline(int x, int y, int xend, uint8_t c)
{
    if(y<WindY||y>WindY1)return;
    int xr;
    int Lxr;
    if(xend>=x){
        if(x>WindX1)return;
        if(x<WindX)xr=WindX; else xr=x;
        if(xend>WindX1)Lxr=WindX1-xr+1;else Lxr=xend-xr+1;
    }else{
        if(xend>WindX1)return;
        if(xend<WindX)xr=WindX; else xr=xend;
        if(x>WindX1)Lxr=WindX1-xr+1;else Lxr=x-xr+1;
    }
    // BoonXRay 12.08.2017
    //int scr=int(ScreenPtr)+xr+y*ScrWidth;

    if(Lxr<=0)return;
    // BoonXRay 12.08.2017
    //__asm{
    //	mov		edi,scr
    //	mov		al,c
    //	mov		ecx,Lxr
    //	cld
    //	rep		stosb
    //}
    memset(reinterpret_cast<unsigned char *>(ScreenPtr) + xr + y*ScrWidth, c, Lxr);
}

void Vline(int x,int y,int yend, uint8_t c)
{
    if(x<WindX||x>WindX1)return;
    int yr;
    int Lyr;
    if(yend>=y){
        if(y>WindY1)return;
        if(y<WindY)yr=WindY; else yr=y;
        if(yend<WindY)return;
        if(yend>WindY1)Lyr=WindY1-yr+1;else Lyr=yend-yr+1;
    }else{
        if(yend>WindY1)return;
        if(yend<WindY)yr=WindY; else yr=yend;
        if(y>WindY1)Lyr=WindY1-yr+1;else Lyr=y-yr+1;
    }
    // BoonXRay 12.08.2017
    //int scr=int(ScreenPtr)+x+yr*ScrWidth;
    if(Lyr<=0)return;
    // BoonXRay 12.08.2017
//	__asm{
//		mov		edi,scr
//		mov		al,c
//		mov		ecx,Lyr
//		mov		edx,ScrWidth
//		dec		edx
//		cld
//uuuuu:	stosb
//		add		edi,edx
//		loop	uuuuu
//	}
    unsigned char * Ptr = reinterpret_cast<unsigned char *>(ScreenPtr)+x + yr*ScrWidth;
    for (int i = 0; i < Lyr; i++, Ptr += ScrWidth)
        *Ptr = c;
}

void Xbar(int x,int y,int lx,int ly, uint8_t c)
{
    Hline(x,y,x+lx-1,c);
    Hline(x,y+ly-1,x+lx-1,c);
    Vline(x,y,y+ly-1,c);
    Vline(x+lx-1,y,y+ly-1,c);
}

static int GetXKind(int x)
{
    if(x<WindX)return 8;
    else if(x>WindX1)return 4;
    else return 0;
}

static int GetYKind(int y)
{
    if(y<WindY)return 2;
    else if(y>WindY1)return 1;
    else return 0;
}

void DrawLine(int x, int y, int x1, int y1, uint8_t c)
{
    if(x==x1&&y==y1){
        Hline(x,y,x,c);
        return;
    }
    int pp=GetXKind(x)+GetYKind(y);
    int pp1=GetXKind(x1)+GetYKind(y1);
    if(pp&pp1)return;
    if(x>x1){
        x1=x+x1;
        y1=y+y1;
        x=x1-x;
        y=y1-y;
        x1=x1-x;
        y1=y1-y;
    }
    if(x<WindX){
        y+=div((y1-y)*(WindX-x),x1-x).quot;
        x=WindX;
    }
    if(x1>WindX1){
        y1=y+div((y1-y)*(WindX1-x),x1-x).quot;
        x1=WindX1;
    }
    if(GetYKind(y1)&GetYKind(y))return;
    if(y<WindY){
        x+=div((x1-x)*(WindY-y),y1-y).quot;
        y=WindY;
    }
    if(y1<WindY){
        x1=x+div((x1-x)*(WindY-y),y1-y).quot;
        y1=WindY;
    }
    if(y>WindY1){
        x+=div((x1-x)*(WindY1-y),y1-y).quot;
        y=WindY1;
    }
    if(y1>WindY1){
        x1=x+div((x1-x)*(WindY1-y),y1-y).quot;
        y1=WindY1;
    }
    if(GetXKind(x)|GetXKind(x1)|GetYKind(y)|GetYKind(y1))return;
    int Lx=x1-x;
    int Ly;
    int ofst=int(ScreenPtr)+x+y*ScrWidth;
    if(y<y1){
        Ly=y1-y;
        if(Lx<Ly){
            // BoonXRay 12.08.2017
//			__asm{
//				push	edi
//				mov		edi,ofst
//				mov		edx,Lx
//				mov		ebx,Ly
//				sar		ebx,1
//				mov		ecx,Ly
//				inc     ecx
//				mov		al,c
//lpp1:			mov		[edi],al
//				add		edi,ScrWidth
//				sub		ebx,edx
//				jg		lpp2
//				add		ebx,Ly
//				inc     edi
//lpp2:			dec		ecx
//				jnz		lpp1
//				pop		edi
//			}
            //__asm
            {
                //push	edi
                //mov		edi, ofst
                //mov		edx, Lx
                //mov		ebx, Ly
                //sar		ebx, 1
                //mov		ecx, Ly
                //inc     ecx
                //mov		al, c
                int TmpEDI = ofst;
                int TmpEDX = Lx, TmpEBX = Ly >> 1, TmpECX = Ly + 1;
            lpp1 :
                //mov[edi], al
                //add		edi, ScrWidth
                //sub		ebx, edx
                //jg		lpp2
                //add		ebx, Ly
                //inc     edi
                *reinterpret_cast<unsigned char *>(TmpEDI) = c;
                TmpEDI += ScrWidth;
                TmpEBX -= TmpEDX;
                if (TmpEBX > 0) goto lpp2;
                TmpEBX += Ly;
                TmpEDI++;
            lpp2 :
                //dec		ecx
                //jnz		lpp1
                //pop		edi
                TmpECX--;
                if (TmpECX != 0) goto lpp1;
            }
        }else{
            // BoonXRay 12.08.2017
//			__asm{
//				push	edi
//				mov		edi,ofst
//				mov		edx,Ly
//				mov		ebx,Lx
//				sar		ebx,1
//				mov		ecx,Lx
//				inc     ecx
//				mov		al,c
//lpp1_1:			mov		[edi],al
//				inc		edi
//				sub		ebx,edx
//				jg		lpp2_1
//				add		ebx,Lx
//				add		edi,ScrWidth
//lpp2_1:			dec		ecx
//				jnz		lpp1_1
//				pop		edi
//			}
        }
        //__asm
        {
            //push	edi
            //mov		edi, ofst
            //mov		edx, Ly
            //mov		ebx, Lx
            //sar		ebx, 1
            //mov		ecx, Lx
            //inc     ecx
            //mov		al, c
            int TmpEDI = ofst;
            int TmpEDX = Ly, TmpEBX = Lx >> 1, TmpECX = Lx + 1;
        lpp1_1 :
            //mov[edi], al
            //inc		edi
            //sub		ebx, edx
            //jg		lpp2_1
            //add		ebx, Lx
            //add		edi, ScrWidth
            *reinterpret_cast<unsigned char *>(TmpEDI) = c;
            TmpEDI++;
            TmpEBX -= TmpEDX;
            if (TmpEBX > 0) goto lpp2_1;
            TmpEBX += Lx;
            TmpEDI += ScrWidth;
        lpp2_1 :
            //dec		ecx
            //jnz		lpp1_1
            //pop		edi
            TmpECX--;
            if (TmpECX != 0) goto lpp1_1;
        }

    }else{
        Ly=y-y1;
        if(Lx<Ly){
            // BoonXRay 12.08.2017
//			__asm{
//				push	edi
//				mov		edi,ofst
//				mov		edx,Lx
//				mov		ebx,Ly
//				sar		ebx,1
//				mov		ecx,Ly
//				inc     ecx
//				mov		al,c
//lpp1_2:			mov		[edi],al
//				sub		edi,ScrWidth
//				sub		ebx,edx
//				jg		lpp2_2
//				add		ebx,Ly
//				inc     edi
//lpp2_2:			dec		ecx
//				jnz		lpp1_2
//				pop		edi
//			}
            //__asm
            {
                //push	edi
                //mov		edi, ofst
                //mov		edx, Lx
                //mov		ebx, Ly
                //sar		ebx, 1
                //mov		ecx, Ly
                //inc     ecx
                //mov		al, c
                int TmpEDI = ofst;
                int TmpEDX = Lx, TmpEBX = Ly >> 1, TmpECX = Ly + 1;
            lpp1_2 :
                //mov[edi], al
                //sub		edi, ScrWidth
                //sub		ebx, edx
                //jg		lpp2_2
                //add		ebx, Ly
                //inc     edi
                *reinterpret_cast<unsigned char *>(TmpEDI) = c;
                TmpEDI -= ScrWidth;
                TmpEBX -= TmpEDX;
                if (TmpEBX > 0) goto lpp2_2;
                TmpEBX += Ly;
                TmpEDI++;
            lpp2_2 :
                //dec		ecx
                //jnz		lpp1_2
                //pop		edi
                TmpECX--;
                if (TmpECX != 0) goto lpp1_2;
            }
        }else{
            // BoonXRay 12.08.2017
//			__asm{
//				push	edi
//				mov		edi,ofst
//				mov		edx,Ly
//				mov		ebx,Lx
//				sar		ebx,1
//				mov		ecx,Lx
//				inc     ecx
//				mov		al,c
//lpp1_3:			mov		[edi],al
//				inc		edi
//				sub		ebx,edx
//				jg		lpp2_3
//				add		ebx,Lx
//				sub		edi,ScrWidth
//lpp2_3:			dec		ecx
//				jnz		lpp1_3
//				pop		edi
//			}
            //__asm
            {
                //push	edi
                //mov		edi, ofst
                //mov		edx, Ly
                //mov		ebx, Lx
                //sar		ebx, 1
                //mov		ecx, Lx
                //inc     ecx
                //mov		al, c
                int TmpEDI = ofst;
                int TmpEDX = Ly, TmpEBX = Lx >> 1, TmpECX = Lx + 1;
            lpp1_3 :
                //mov[edi], al
                //inc		edi
                //sub		ebx, edx
                //jg		lpp2_3
                //add		ebx, Lx
                //sub		edi, ScrWidth
                *reinterpret_cast<unsigned char *>(TmpEDI) = c;
                TmpEDI++;
                TmpEBX -= TmpEDX;
                if (TmpEBX > 0) goto lpp2_3;
                TmpEBX += Lx;
                TmpEDI -= ScrWidth;
            lpp2_3 :
                //dec		ecx
                //jnz		lpp1_3
                //pop		edi
                TmpECX--;
                if (TmpECX != 0) goto lpp1_3;
            }

        }
    }
}
