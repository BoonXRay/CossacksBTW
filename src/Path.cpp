#include <cstring>

#include "LoadSave.h"
#include "NewMon.h"
#include "UnSyncro.h"

#include "Path.h"


bool AllowPathDelay;

//-------------------------------------------------------//
//                                                       //
//                    ОБХОД ПРЕПЯТСТВИЙ                  //
//                                                       //
//-------------------------------------------------------//

//Zero out MapV
void MotionField::ClearMaps()
{
	memset(MapV, 0, MAPSY*BMSX);//1024*128
}

void MotionField::BSetPt(int x, int y)
{
	int MAPV = int(MapV);
	if (x >= 0 && x < MAPSX&&y >= 0 && y < MAPSY) {
		switch (ADDSH) {
		case 1:
			// BoonXRay 13.08.2017
			//__asm 
			{				
				//Vertical
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 7//MAPSHF
				//add		ebx, eax
				//mov		al, 1
				//shl		al, cl
				//add		ebx, MAPV
				//or [ebx], al
				unsigned int TmpEAX = y;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned int TmpECX = TmpEAX & 7;
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				TmpEAX >>= 3;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 7;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = 1 << TmpCL;
				TmpEBX += MAPV;
				*reinterpret_cast<unsigned char *>(TmpEBX) |= TmpAL;
            }
			break;
		case 2:
			// BoonXRay 13.08.2017
			//__asm 
			{				
				//Vertical
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 8//MAPSHF
				//add		ebx, eax
				//mov		al, 1
				//shl		al, cl
				//add		ebx, MAPV
				//or [ebx], al
				unsigned int TmpEAX = y;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned int TmpECX = TmpEAX & 7;
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				TmpEAX >>= 3;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 8;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = 1 << TmpCL;
				TmpEBX += MAPV;
				*reinterpret_cast<unsigned char *>(TmpEBX) |= TmpAL;
            }
			break;
		case 3:
			// BoonXRay 13.08.2017
			//__asm 
			{				
				//Vertical
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 9//MAPSHF
				//add		ebx, eax
				//mov		al, 1
				//shl		al, cl
				//add		ebx, MAPV
				//or [ebx], al
				unsigned int TmpEAX = y;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned int TmpECX = TmpEAX & 7;
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				TmpEAX >>= 3;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 9;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = 1 << TmpCL;
				TmpEBX += MAPV;
				*reinterpret_cast<unsigned char *>(TmpEBX) |= TmpAL;
            }
			break;
        }
    }
}

void MotionField::BClrPt(int x, int y)
{
	int MAPV = int(MapV);
	if (x >= 0 && x < MAPSX&&y >= 0 && y < MAPSY) {
		switch (ADDSH) {
		case 1:
			// BoonXRay 13.08.2017
			//__asm 
			{
				//Vertical
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 7//MAPSHF
				//add		ebx, eax
				//mov		al, 254
				//rol		al, cl
				//add		ebx, MAPV
				//and[ebx], al
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char TmpCL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 7;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = ~(1 << TmpCL);	// Valid only for this case ( TmpCL = 0..7 )
				TmpEBX += MAPV;
				*reinterpret_cast<unsigned char *>(TmpEBX) &= TmpAL;
            }
			break;
		case 2:
			// BoonXRay 13.08.2017
			//__asm 
			{				
				//Vertical
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 8//MAPSHF
				//add		ebx, eax
				//mov		al, 254
				//rol		al, cl
				//add		ebx, MAPV
				//and[ebx], al
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char TmpCL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 8;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = ~(1 << TmpCL);	// Valid only for this case ( TmpCL = 0..7 )
				TmpEBX += MAPV;
				*reinterpret_cast<unsigned char *>(TmpEBX) &= TmpAL;
            }
			break;
		case 3:
			// BoonXRay 13.08.2017
			//__asm 
			{
				//Vertical
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 9//MAPSHF
				//add		ebx, eax
				//mov		al, 254
				//rol		al, cl
				//add		ebx, MAPV
				//and[ebx], al
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char TmpCL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 9;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = ~(1 << TmpCL);	// Valid only for this case ( TmpCL = 0..7 )
				TmpEBX += MAPV;
				*reinterpret_cast<unsigned char *>(TmpEBX) &= TmpAL;
            }
			break;
        }

    }
}

void MotionField::BSetBar(int x, int y, int Lx) {
	for (int ix = x; ix < x + Lx; ix++)
	{
		for (int iy = y; iy < y + Lx; iy++)
		{
			BSetPt(ix, iy);
		}
	}
}

void MotionField::BClrBar(int x, int y, int Lx) {
	for (int ix = x; ix < x + Lx; ix++)
	{
		for (int iy = y; iy < y + Lx; iy++)
		{
			BClrPt(ix, iy);
		}
	}
}

void MotionField::BSetSQ(int x, int y, int Lx, int Ly) {
	for (int ix = x; ix < x + Lx; ix++)
	{
		for (int iy = y; iy < y + Ly; iy++)
		{
			BSetPt(ix, iy);
		}
	}
}

void MotionField::BClrSQ(int x, int y, int Lx, int Ly) {
	for (int ix = x; ix < x + Lx; ix++)
	{
		for (int iy = y; iy < y + Ly; iy++)
		{
			BClrPt(ix, iy);
		}
	}
}

//Checks coordinates against MotionField::MapV
//Possible return values: 0, 1, 2, 4, 8, 16, 32, 64, 128
int MotionField::CheckPt(int x, int y)
{
	int retval = 0;
	int MAPV = int(MapV);
	if (x >= 0 && x < MAPSX&&y >= 0 && y < MAPSY) 
	{
		switch (ADDSH) 
		{
		case 1:
			// BoonXRay 13.08.2017
			//__asm
			{
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 7//MAPSHF
				//add		ebx, eax
				//mov		al, 1
				//shl		al, cl
				//add		ebx, MAPV
				//and		al, [ebx]
				//and eax, 0xFF
				//mov		retval, eax
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char TmpCL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 7;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = 1 << TmpCL;
				TmpEBX += MAPV;
				TmpAL &= *reinterpret_cast<unsigned char *>(TmpEBX);
				retval = TmpEAX & 0xFF;
			}
			break;

		case 2:
			// BoonXRay 13.08.2017
			//__asm
			{
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 8//MAPSHF
				//add		ebx, eax
				//mov		al, 1
				//shl		al, cl
				//add		ebx, MAPV
				//and		al, [ebx]
				//and eax, 0xFF
				//mov		retval, eax
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char TmpCL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 8;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = 1 << TmpCL;
				TmpEBX += MAPV;
				TmpAL &= *reinterpret_cast<unsigned char *>(TmpEBX);
				retval = TmpEAX & 0xFF;
			}
			break;

		case 3:
			// BoonXRay 13.08.2017
			//__asm
			{
				//mov		eax, y
				//mov		ecx, eax
				//and		ecx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 9//MAPSHF
				//add		ebx, eax
				//mov		al, 1
				//shl		al, cl
				//add		ebx, MAPV
				//and		al, [ebx]
				//and eax, 0xFF
				//mov		retval, eax
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
				unsigned char TmpCL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 9;	//MAPSHF
				TmpEBX += TmpEAX;
				TmpAL = 1 << TmpCL;
				TmpEBX += MAPV;
				TmpAL &= *reinterpret_cast<unsigned char *>(TmpEBX);
				retval = TmpEAX & 0xFF;
			}
			break;
		}
		return retval;
	}
	else
	{
		return 1;
	}
}

int MotionField::CheckHLine(int x, int y, int Lx)
{
	for (int i = 0; i < Lx; i++)
	{
		if (CheckPt(x + i, y))
		{
			return 1;
		}
	}
	return 0;
}

int MotionField::CheckVLine(int x, int y, int Lx)
{
	int retval = 0;
	int MAPV = int(MapV);
	if (x > 0 && y > 0 && y + Lx - 1 < MAPSY && x < MAPSX && Lx <= 24)
	{
		switch (ADDSH)
		{
		case 1:
			// BoonXRay 13.08.2017
			//__asm
			{
				//mov		eax, y
				//mov		edx, eax
				//and		edx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 7//MAPSHF
				//add		ebx, eax
				//mov		ecx, Lx
				//mov		eax, 1
				//shl		eax, cl
				//dec		eax
				//mov		cl, dl
				//rol		eax, cl
				//add		ebx, MAPV
				//and		eax, dword ptr[ebx]
				//mov		retval, eax
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char TmpDL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 7;	//MAPSHF
				TmpEBX += TmpEAX;
				unsigned int TmpECX = static_cast<unsigned int>(Lx);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				TmpEAX = (1 << TmpCL) - 1;
				// Valid only for this case ( TmpDL = 0..7 )
				unsigned char TmpUChar = TmpEAX >> (32 - TmpDL);
				TmpEAX = (TmpEAX << TmpDL) | TmpUChar;

				TmpEBX += MAPV;
				retval = TmpEAX & *reinterpret_cast<unsigned int *>(TmpEBX);
			}
			break;

		case 2:
			// BoonXRay 13.08.2017
			//__asm
			{
				//mov		eax, y
				//mov		edx, eax
				//and		edx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 8//MAPSHF
				//add		ebx, eax
				//mov		ecx, Lx
				//mov		eax, 1
				//shl		eax, cl
				//dec		eax
				//mov		cl, dl
				//rol		eax, cl
				//add		ebx, MAPV
				//and		eax, dword ptr[ebx]
				//mov		retval, eax
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char TmpDL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 8;	//MAPSHF
				TmpEBX += TmpEAX;
				unsigned int TmpECX = static_cast<unsigned int>(Lx);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				TmpEAX = (1 << TmpCL) - 1;
				// Valid only for this case ( TmpDL = 0..7 )
				unsigned char TmpUChar = TmpEAX >> (32 - TmpDL);
				TmpEAX = (TmpEAX << TmpDL) | TmpUChar;

				TmpEBX += MAPV;
				retval = TmpEAX & *reinterpret_cast<unsigned int *>(TmpEBX);
			}
			break;

		case 3:
			// BoonXRay 13.08.2017
			//__asm
			{
				//mov		eax, y
				//mov		edx, eax
				//and		edx, 7
				//shr		eax, 3
				//mov		ebx, x
				//shl		ebx, 9//MAPSHF
				//add		ebx, eax
				//mov		ecx, Lx
				//mov		eax, 1
				//shl		eax, cl
				//dec		eax
				//mov		cl, dl
				//rol		eax, cl
				//add		ebx, MAPV
				//and		eax, dword ptr[ebx]
				//mov		retval, eax
				unsigned int TmpEAX = static_cast<unsigned int>(y) >> 3;
				unsigned char TmpDL = static_cast<unsigned int>(y) & 7;
				unsigned int TmpEBX = static_cast<unsigned int>(x) << 9;	//MAPSHF
				TmpEBX += TmpEAX;
				unsigned int TmpECX = static_cast<unsigned int>(Lx);
				unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
				TmpEAX = (1 << TmpCL) - 1;
				// Valid only for this case ( TmpDL = 0..7 )
				unsigned char TmpUChar = TmpEAX >> (32 - TmpDL);
				TmpEAX = (TmpEAX << TmpDL) | TmpUChar;

				TmpEBX += MAPV;
				retval = TmpEAX & *reinterpret_cast<unsigned int *>(TmpEBX);
			}
			break;
		}
		return retval;
	}
	else
	{
		return 1;
	}
}

bool MotionField::CheckBar(int x, int y, int Lx, int Ly)
{
	for (int ix = 0; ix < Lx; ix++)
	{
		if (CheckVLine(x + ix, y, Ly))
		{
			return true;
		}
	}
	return false;
}

static uint8_t NewCirc[16];

void MotionField::FillNewCirc(int x, int y, int Lx, int Ly)
{
	int* xx = (int*)NewCirc;
	xx[0] = 0;
	xx[1] = 0;

	if (CheckHLine(x, y - 1, Lx))
	{
		NewCirc[0] = 1;
	}
	if (CheckHLine(x, y + Ly, Lx))
	{
		NewCirc[4] = 1;
	}
	if (CheckVLine(x + Lx, y, Ly))
	{
		NewCirc[2] = 1;
	}
	if (CheckVLine(x - 1, y, Ly))
	{
		NewCirc[6] = 1;
	}

	if (Ly < 2)
	{
		if (CheckPt(x - 1, y - 1))
			NewCirc[7] = 1;

		if (CheckPt(x + Lx, y - 1))
			NewCirc[1] = 1;

		if (CheckPt(x - 1, y + Ly))
			NewCirc[5] = 1;

		if (CheckPt(x + Lx, y + Ly))
			NewCirc[3] = 1;
	}
	else
	{
		if (Ly == 2)
		{
			if (CheckHLine(x + 1, y - 1, Lx) && CheckPt(x + Lx, y))
				NewCirc[1] = 1;

			if (CheckHLine(x + 1, y + Ly, Lx) && CheckPt(x + Lx, y + 1))
				NewCirc[3] = 1;

			if (CheckHLine(x - 1, y + Ly, Lx) && CheckPt(x - 1, y + 1))
				NewCirc[5] = 1;

			if (CheckHLine(x - 1, y - 1, Lx) && CheckPt(x - 1, y))
				NewCirc[7] = 1;
		}
		else
		{
			if (CheckHLine(x + 1, y - 1, Lx) || CheckVLine(x + Lx, y, Ly - 1))
				NewCirc[1] = 1;

			if (CheckHLine(x + 1, y + Ly, Lx) || CheckVLine(x + Lx, y + 1, Ly - 1))
				NewCirc[3] = 1;

			if (CheckHLine(x - 1, y + Ly, Lx) || CheckVLine(x - 1, y + 1, Ly - 1))
				NewCirc[5] = 1;

			if (CheckHLine(x - 1, y - 1, Lx) || CheckVLine(x - 1, y, Ly - 1))
				NewCirc[7] = 1;
		}
	}
	xx[2] = xx[0];
	xx[3] = xx[1];
}

void MotionField::Allocate()
{
    //MapH=new byte[MAPSY*BMSX];
    //memset(MapH,0,MAPSY*BMSX);
    MapV = new byte[MAPSY*BMSX];
    memset( MapV, 0, MAPSY*BMSX );
}

void MotionField::FreeAlloc()
{
    //free(MapH);
    free( MapV );
}

//------------------------------------------------//
//          KERNEL OF THE MOTION ENGINE           //
//            Search for the best way             //
//------------------------------------------------//
static constexpr uint16_t MaxP = 4096;

static uint16_t pxx[MaxP + 160];
static uint16_t pyy[MaxP + 160];
static uint16_t RVIS[MaxP + 160];
static uint16_t LVIS[MaxP + 160];

static const int ndrr[9] = { 7,6,5,0,0,4,1,2,3 };
static const int nidrx[8] = { 0,1,1,1,0,-1,-1,-1 };
static const int nidry[8] = { -1,-1,0,1,1,1,0,-1 };

MotionField MFIELDS[2];//0-Land,1-Water

bool FindBestPosition(OneObject* OB, int* xd, int* yd, int R0)
{
	MotionField* MFI = &MFIELDS[OB->LockType];

	if (!OB)
		return false;

	if (!MFI->CheckBar((*xd) - 1, (*yd) - 1, OB->Lx + 2, OB->Lx + 2))
		return true;

	int Lx = OB->Lx + 2;
	int R1 = R0;
	int bx = *xd;
	int by = *yd;
	int bdist = 100000;
	int xxx = bx - 1;
	int yyy = by - 1;
	int LL = 2;
	int bx1, by1, dstn;
	int x = OB->x;
	int y = OB->y;
	while (R1)
	{
		for (int i = 0; i <= LL; i++)if (!MFI->CheckBar(xxx + i - 1, yyy - 1, Lx, Lx)) {
			bx1 = xxx + i;
			by1 = yyy;
			dstn = Norma(bx1 - x, by1 - y);
			if (dstn < bdist) {
				bx = bx1;
				by = by1;
				bdist = dstn;
            }
        }
		for (int i = 0; i <= LL; i++)if (!MFI->CheckBar(xxx + i - 1, yyy + LL - 1, Lx, Lx)) {
			bx1 = xxx + i;
			by1 = yyy + LL;
			dstn = Norma(bx1 - x, by1 - y);
			if (dstn < bdist) {
				bx = bx1;
				by = by1;
				bdist = dstn;
            }
        }
		for (int i = 0; i < LL - 1; i++)if (!MFI->CheckBar(xxx - 1, yyy + i, Lx, Lx)) {
			bx1 = xxx;
			by1 = yyy + i + 1;
			dstn = Norma(bx1 - x, by1 - y);
			if (dstn < bdist) {
				bx = bx1;
				by = by1;
				bdist = dstn;
            }
        }
		for (int i = 0; i < LL - 1; i++)if (!MFI->CheckBar(xxx + LL - 1, yyy + i, Lx, Lx)) {
			bx1 = xxx + LL;
			by1 = yyy + i + 1;
			dstn = Norma(bx1 - x, by1 - y);
			if (dstn < bdist) {
				bx = bx1;
				by = by1;
				bdist = dstn;
            }
        }
		if (bdist < 100000) {
			*xd = bx;
			*yd = by;
			return true;
        }
		R1--;
		LL += 2;
		xxx--;
		yyy--;
    }
	return false;
}

void CreateFullPath(int x1, int y1, OneObject* OB)
{
	OB->DeletePath();
	OB->NIPoints = 0;
	int xx1 = x1;
	int yy1 = y1;

	if (!FindBestPosition(OB, &xx1, &yy1, 40))
		return;

	bool pc;
	int NN = 0;
	AllowPathDelay = false;
	do
	{
		int t0 = GetTickCount();

		pc = OB->CreatePrePath(xx1, yy1);
		t0 = GetTickCount() - t0;

		if (pc) {
			xx1 = OB->PathX[OB->NIPoints - 1];
			yy1 = OB->PathY[OB->NIPoints - 1];
        }
		NN++;
		rando();
	} while (pc&&NN < 4);

	if (AllowPathDelay)
	{
		OB->PathDelay = (rando() & 7);
	}
}

#define sssh 1
bool OneObject::CreatePrePath(int x1, int y1)
{
	AllowPathDelay = false;
	if (LockType == 1) {
		return CreatePrePath2(x1, y1);
    }
	if (PathDelay)return false;

	AllowPathDelay = true;
	if (abs(x - x1) < 2 && abs(y - y1) < 2)return false;
	MotionField* MFI = &MFIELDS[LockType];
	if (GLock)MFI->BClrBar(x, y, Lx);
	bool InLocked = MFI->CheckBar(x, y, Lx, Lx);
	if (InLocked&&PathX) {
		if (GLock)MFI->BSetBar(x, y, Lx);
		return false;
    }
	int sdx = x1 - x;
	int	sdy = y1 - y;
    int Pps = 0;
	int sx = (sdx > 0) ? 1 : -1;
	int sy = (sdy > 0) ? 1 : -1;
	int dx = abs(sdx);
	int dy = abs(sdy);
	int	Mdx = dx;
	int	Mdy = dy;
	int	Mx = x;
	int	My = y;
	int Angle0 = GetLAngle(x1 - x, y1 - y, 0);
	int Angle;
	int ddx, ddy;
    int Rvp = 0;


	CurIPoint = 0;
	NeedPath = false;

	//соединяем линией начальную и конечную точки. 
	//Оптимизация только по скорости
	// BoonXRay 13.08.2017
	//__asm
	{
		//mov		ax, word ptr Mdx
		//mov		bx, word ptr Mdy
		//xor		edx, edx  //Pps
		//xor		ecx, ecx  //Cum
		//mov		si, word ptr Mx
		//mov		di, word ptr My
		//cmp		bx, ax
		//jae		Lp5xx
		short TmpAX = Mdx, TmpBX = Mdy, TmpCX = 0/* Cum */;
		unsigned short TmpSI = Mx, TmpDI = My;
		unsigned int TmpEDX = 0;		//Pps
		if (TmpBX >= TmpAX) goto Lp5xx;
		//dx>dy
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//or ax, ax
		//jz		LoopsEnd
		//cmp		sy, 0
		//jl		Lp3xx
		//cmp		sx, 0
		//jl		Lp2begin
		pxx[ TmpEDX ] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		if (TmpAX == 0) goto LoopsEnd;
		if (sy < 0) goto Lp3xx;
		if (sx < 0) goto Lp2begin;
		//dx>dy,sx>0,sy>0
	Lp1begin :
		//inc		si	//x++
		//add		cx, bx
		//cmp		cx, word ptr Mdx
		//jb		Lp1_1
		//sub		cx, word ptr Mdx
		//inc		di  //y++
		TmpSI++;	//x++
		TmpCX += TmpBX;
		if (TmpCX < Mdx) goto Lp1_1;
		TmpCX -= Mdx;
		TmpDI++;	//y++
	Lp1_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		ax
		//jnz		Lp1begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpAX--;
		if (TmpAX != 0) goto Lp1begin;
		goto LoopsEnd;
	Lp2begin : //dx>dy,sx<0,sy>0
		//dec		si	//x--
		//add		cx, bx
		//cmp		cx, word ptr Mdx
		//jb		Lp2_1
		//sub		cx, word ptr Mdx
		//inc		di //y++
		TmpSI--;	//x--
		TmpCX += TmpBX;
		if (TmpCX < Mdx) goto Lp2_1;
		TmpCX -= Mdx;
		TmpDI++;	//y++
	Lp2_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		ax
		//jnz		Lp2begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpAX--;
		if (TmpAX != 0) goto Lp2begin;
		goto LoopsEnd;
	Lp3xx :	//dy<0
		//cmp		sx, 0
		//jl		Lp4begin
		if (sx < 0) goto Lp4begin;
	Lp3begin : //dx>dy,sx>0,sy<0
		//inc		si	//x++
		//add		cx, bx
		//cmp		cx, word ptr Mdx
		//jb		Lp3_1
		//sub		cx, word ptr Mdx
		//dec		di //y--
		TmpSI++;	//x++
		TmpCX += TmpBX;
		if (TmpCX < Mdx) goto Lp3_1;
		TmpCX -= Mdx;
		TmpDI--;	//y--
	Lp3_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		ax
		//jnz		Lp3begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpAX--;
		if (TmpAX != 0) goto Lp3begin;
		goto LoopsEnd;
	Lp4begin : //dx>dy,sx<0,sy<0
		//dec		si	//x--
		//add		cx, bx
		//cmp		cx, word ptr Mdx
		//jb		Lp4_1
		//sub		cx, word ptr Mdx
		//dec		di //y--
		TmpSI--;	//x--
		TmpCX += TmpBX;
		if (TmpCX < Mdx) goto Lp4_1;
		TmpCX -= Mdx;
		TmpDI--;	//y--
	Lp4_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		ax
		//jnz		Lp4begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpAX--;
		if (TmpAX != 0) goto Lp4begin;
		goto LoopsEnd;
	Lp5xx :	//dx<dy
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//or bx, bx
		//jz		LoopsEnd
		//cmp		sx, 0
		//jl		Lp7xx
		//cmp		sy, 0
		//jl		Lp6begin
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		if (TmpBX == 0) goto LoopsEnd;
		if (sx < 0) goto Lp7xx;
		if (sy < 0) goto Lp6Begin;
	Lp5Begin :
		//inc		di	//y++
		//add		cx, ax
		//cmp		cx, word ptr dy
		//jb		Lp5_1
		//sub		cx, word ptr dy
		//inc		si	//x++
		TmpDI++;	//y++
		TmpCX += TmpAX;
		if (TmpCX < dy) goto Lp5_1;
		TmpCX -= dy;
		TmpSI++;	//x++
	Lp5_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		bx
		//jnz		Lp5begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpBX--;
		if (TmpBX != 0) goto Lp5Begin;
		goto LoopsEnd;
	Lp6Begin ://sx>0,sy<0
		//dec		di	//y++
		//add		cx, ax
		//cmp		cx, word ptr dy
		//jb		Lp6_1
		//sub		cx, word ptr dy
		//inc		si	//x++
		TmpDI--;	//y--
		TmpCX += TmpAX;
		if (TmpCX < dy) goto Lp6_1;
		TmpCX -= dy;
		TmpSI++;	//x++
	Lp6_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		bx
		//jnz		Lp6begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpBX--;
		if (TmpBX != 0) goto Lp6Begin;
		goto LoopsEnd;
	Lp7xx :	//dx<0
		//cmp		sy, 0
		//jl		Lp8begin
		if (sy < 0) goto Lp8Begin;
	Lp7Begin ://dx<0,dy>0
		//inc		di	//y++
		//add		cx, ax
		//cmp		cx, word ptr dy
		//jb		Lp7_1
		//sub		cx, word ptr dy
		//dec		si	//x--
		TmpDI++;	//y++
		TmpCX += TmpAX;
		if (TmpCX < dy) goto Lp7_1;
		TmpCX -= dy;
		TmpSI--;	//x--
	Lp7_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		bx
		//jnz		Lp7begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpBX--;
		if (TmpBX != 0) goto Lp7Begin;
		goto LoopsEnd;
	Lp8Begin ://dx<0,dy<0
		//dec		di	//y--
		//add		cx, ax
		//cmp		cx, word ptr dy
		//jb		Lp8_1
		//sub		cx, word ptr dy
		//dec		si	//x--
		TmpDI--;	//y--
		TmpCX += TmpAX;
		if (TmpCX < dy) goto Lp8_1;
		TmpCX -= dy;
		TmpSI--;	//x--
	Lp8_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		bx
		//jnz		Lp8begin
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpBX--;
		if (TmpBX != 0) goto Lp8Begin;
	LoopsEnd :
		//shr		edx, 1
		//mov		Pps, edx
		Pps = TmpEDX >> 1;
	}

	Pps--;

	if (InLocked)
	{
		int uu;
		for (uu = 0; uu < Pps&&MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
		if (uu >= Pps)
		{
			return false;
		}

		PathX = new short;
		PathY = new short;
		PathX[0] = pxx[uu];
		PathY[0] = pyy[uu];
		NIPoints = 1;
		return true;
	}

	int RVisPos = 0;
	int LVisPos = 0;
	bool RightPrefer = true;
	int Rtx;//current point 
	int Rty;
	int Ltx;
	int Lty;
	word Rpx[MaxP];//right path
	word Rpy[MaxP];
	word Lpx[MaxP];//left path
	word Lpy[MaxP];
	int Rpp = 1;//index of current point
	bool LDoing;//=true if last point reached
	bool RDoing;
	byte Rdirc;//currend direction
	byte Ldirc;
	int Rmaxalt;//maximum alteration,right path
	int Lmaxalt;//maximum alteration,left path
	int Rcum = 0;
	int Rcum1 = 0;
	int Lcum = 0;
	int Lcum1 = 0;
    // BoonXRay 10.09.2017
    //bool Rvis;
	//Проверяем прямую проходимость
	int uu;
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
	if (uu == Pps)return false;
	//Идем, пока не упремся в стенку
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu] - 1, pyy[uu] - 1, Lx + 2, Lx + 2); uu++);
	Rpp = uu;
	Rtx = pxx[Rpp];
	Rty = pyy[Rpp];
	int Rppm = uu;
	int Lppm = uu;
	// Если dx>dy,то на каждом шагу dx изменяетя строго на 1
	if (Rtx != x1 || Rty != y1) {
		//LLock[y][x]=false;
		if (Rpp > 0)Rpp -= 1;
		Rtx = pxx[Rpp];
		Rty = pyy[Rpp];
		Ltx = pxx[Rpp];
		Lty = pyy[Rpp];
		int Ppi = Rpp + 1;
		LDoing = true;
		RDoing = true;
		//Ищем, пока находимся в занятой зоне
		while (MFI->CheckBar(pxx[Ppi] - 1, pyy[Ppi] - 1, Lx + 2, Lx + 2) && Ppi < Pps)Ppi++;
		if (Ppi > Pps)LDoing = false;//Кон.точка недостижима
		int Xls = pxx[Ppi - 1];
		int Yls = pyy[Ppi - 1];
		//Уперлись...Вычисляем направление движения
		Rdirc = ndrr[(pxx[Rpp + 1] - pxx[Rpp] + 1) * 3 + pyy[Rpp + 1] - pyy[Rpp] + 1];
		Ldirc = Rdirc;
		//Выбираем начальное направление-right
		MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
		int dirc1 = (Rdirc + 1) & 7;
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1++, z++);
		if (!NewCirc[6 + (dirc1 & 7)])dirc1++;
		Rdirc = dirc1 & 7;
		//-left
		dirc1 = 8 + ((Ldirc + 7) & 7);
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1--, z++);
		if (!NewCirc[(dirc1 & 7) + 2])dirc1--;
		Ldirc = dirc1 & 7;
		//Идем по правому краю до тех пор пока вновь не пересе-
		//чемся с прямой линией, соединяющей начальную и конечную
		//точки
		Rmaxalt = 0;
		Lmaxalt = 0;
		while (Rpp < MaxP - 8 && LDoing&&RDoing) {
			//пытаемся повернуть направо
			MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
			int dirc1 = (Rdirc + 6) & 7;
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1++, z++);
			Rdirc = dirc1 & 7;
			Rpp++;
			int Tdx = nidrx[Rdirc];
			int Tdy = nidry[Rdirc];
			Rcum -= sdy*Tdx;
			Rcum += sdx*Tdy;
			Rtx += Tdx;
			Rty += Tdy;
			Rpx[Rpp] = Rtx; Rpy[Rpp] = Rty;
			Angle = GetLAngle(Rtx - x, Rty - y, Angle0);
			if (Angle > Rmaxalt) {
				Rmaxalt = Angle;
				Rppm = Rpp;
            }
			//проверяем условие прямой видимости
			ddx = x1 - Rtx;
			ddy = y1 - Rty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			int dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			int dirmo = (ndrr[Tdx + Tdx + Tdx + Tdy + 4] - dirvi) & 7;
			if ((dirmo == 1 || dirmo == 7 || dirmo == 0) && !MFI->CheckBar(Rtx + ddx + ddx - 1, Rty + ddy + ddy - 1, Lx + 2, Lx + 2)) {
				//if(!NewCirc[dirvi]){
					//direct vision
				RVIS[RVisPos] = Rpp;
				RVisPos++;
            }
			//the same, but left
			MFI->FillNewCirc(Ltx - 1, Lty - 1, Lx + 2, Lx + 2);
			dirc1 = 8 + ((Ldirc + 2) & 7);
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1--, z++);
			Ldirc = dirc1 & 7;
			Tdx = nidrx[Ldirc];
			Tdy = nidry[Ldirc];
			Lcum += sdy*Tdx;
			Lcum -= sdx*Tdy;
			Ltx += Tdx;
			Lty += Tdy;
			Lpx[Rpp] = Ltx; Lpy[Rpp] = Lty;
			Angle = GetRAngle(Ltx - x, Lty - y, Angle0);
			if (Angle > Lmaxalt) {
				Lmaxalt = Angle;
				Lppm = Rpp;
            }
			//проверяем условие прямой видимости
			ddx = x1 - Ltx;
			ddy = y1 - Lty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			dirmo = (ndrr[Tdx + Tdx + Tdx + Tdy + 4] - dirvi) & 7;
			if ((dirmo == 1 || dirmo == 7 || dirmo == 0) && !MFI->CheckBar(Ltx + ddx + ddx - 1, Lty + ddy + ddy - 1, Lx + 2, Lx + 2)) {
				//if(!NewCirc[dirvi]){
				LVIS[LVisPos] = Rpp;
				LVisPos++;
            }
			//Вычисляем значение y на прямой линии, соотв. 
			//данному x
			if (Rcum <= 0 && Rcum1 >= 0) {
                // BoonXRay 10.09.2017
                //if (!Rvis)Rvp = Rpp;
                Rvp = Rpp;

				if (dx > dy) {
					if (sx > 0) {
						if (Rtx >= Xls)RDoing = false;
					}
					else if (Rtx <= Xls)RDoing = false;
				}
				else {
					if (sy > 0) {
						if (Rty >= Yls)RDoing = false;
					}
					else if (Rty <= Yls)RDoing = false;
                }
				if (!RDoing)RightPrefer = true;
            }
			Rcum1 = Rcum;
			//the same for left
            if (Lcum <= 0 && Lcum1 >= 0) {
                if (dx > dy) {
					if (sx > 0) {
						if (Ltx >= Xls)LDoing = false;
					}
					else if (Ltx <= Xls)LDoing = false;
				}
				else {
					if (sy > 0) {
						if (Lty >= Yls)LDoing = false;
					}
					else if (Lty <= Yls)LDoing = false;
                }
				if (!LDoing)RightPrefer = false;
            }
			Lcum1 = Lcum;
        }
		//LLock[y][x]=true;
		if (Rpp < MaxP - 9)
		{
			if (RightPrefer)
			{
				Pps = Rppm;
				//search for direct vision point
				int i;
				for (i = 0; i < RVisPos&&RVIS[i] < Pps; i++);

				if (i < RVisPos)
				{
					Rvp = RVIS[i];
				}
				else
				{
					Rvp = Rpp - 1;
				}
				if (Rvp < 0)
				{
					Rvp = 0;
				}
			}
			else
			{
				Pps = Lppm;
				//Rvp=Lvp;
				//search for direct vision point
				int i;
				for (i = 0; i < LVisPos&&LVIS[i] < Pps; i++);

				if (i < LVisPos)
				{
					Rvp = LVIS[i];
				}
				else
				{
					Rvp = Rpp - 1;
				}
				if (Rvp < 0)
				{
					Rvp = 0;
				}
			}
			if (Rvp < Rpp)Rvp++;
			if (Rvp < Pps)Rvp = Pps;
			int addNIPoints = Rvp - Pps + 1;
			int maxp = addNIPoints;
			CurIPoint = 0;
			if (addNIPoints > 2) {
				addNIPoints = 2 + ((addNIPoints - 2) >> sssh);
            }
			short* OldPathX = PathX;
			short* OldPathY = PathY;
			PathX = new short[NIPoints + addNIPoints];
			PathY = new short[NIPoints + addNIPoints];
			if (OldPathX) {
				memcpy(PathX, OldPathX, NIPoints << 1);
				memcpy(PathY, OldPathY, NIPoints << 1);
				free(OldPathX);
				free(OldPathY);
            }
			if (RightPrefer) {
				if (maxp <= 2) {
					PathX[NIPoints] = Rpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Rpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Rpx[Pps];
						PathY[NIPoints + 1] = Rpy[Pps];
                    }
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Rpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Rpy[Pps + (nn << sssh)];
                    }
					PathX[NIPoints] = Rpx[Pps + maxp - 1];
					PathY[NIPoints] = Rpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
                }
			}
			else {
				if (maxp <= 2) {
					PathX[NIPoints] = Lpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Lpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Lpx[Pps];
						PathY[NIPoints + 1] = Lpy[Pps];
                    }
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Lpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Lpy[Pps + (nn << sssh)];
                    }
					PathX[NIPoints] = Lpx[Pps + maxp - 1];
					PathY[NIPoints] = Lpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
                }
            }
			NeedPath = true;
			if (GLock)MFI->BSetBar(x, y, Lx);
			return true;
		}
		else {
			if (GLock)MFI->BSetBar(x, y, Lx);
			PathDelay = 200 + (rando() & 15);
			return false;
        }
    }
	if (GLock)MFI->BSetBar(x, y, Lx);
	return false;
}

#undef sssh
#define sssh 2
bool OneObject::CreatePrePath2(int x1, int y1) {
	if (PathDelay)return false;
	if (abs(x - x1) < 2 && abs(y - y1) < 2)return false;
	MotionField* MFI = &MFIELDS[LockType];
	if (GLock)MFI->BClrBar(x, y, Lx);
	bool InLocked = MFI->CheckBar(x, y, Lx, Lx);
	if (InLocked&&PathX) {
		if (GLock)MFI->BClrBar(x, y, Lx);
		return false;
    }
	int sdx = x1 - x;
	int	sdy = y1 - y;
    int Pps = 0;
	int sx = (sdx > 0) ? 1 : -1;
	int sy = (sdy > 0) ? 1 : -1;
	int dx = abs(sdx);
	int dy = abs(sdy);
	int	Mdx = dx;
	int	Mdy = dy;
	int	Mx = x;
	int	My = y;
	int Angle0 = GetLAngle(x1 - x, y1 - y, 0);
	int Angle;
	int ddx, ddy;
	int Lvp = 0;
	int Rvp = 0;
	bool LvpLast = false;
	bool RvpLast = false;
	//if(PathX)free(PathX);
	//if(PathY)free(PathY);
	//PathX=NULL;
	//PathY=NULL;
	//NIPoints=0;
	CurIPoint = 0;
	NeedPath = false;
	//соединяем линией начальную и конечную точки. 
	//Оптимизация только по скорости
	// BoonXRay 13.08.2017
	//__asm 
	{
		//mov		ax, word ptr Mdx
		//mov		bx, word ptr Mdy
		//xor		edx, edx  //Pps
		//xor		ecx, ecx  //Cum
		//mov		si, word ptr Mx
		//mov		di, word ptr My
		//cmp		bx, ax
		//jae		Lp5xx
		short TmpAX = Mdx, TmpBX = Mdy, TmpCX = 0/* Cum */;
		unsigned short TmpSI = Mx, TmpDI = My;
		unsigned int TmpEDX = 0;		//Pps
		if (TmpBX >= TmpAX) goto Lp5xx;
		//dx>dy
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//or ax, ax
		//jz		LoopsEnd
		//cmp		sy, 0
		//jl		Lp3xx
		//cmp		sx, 0
		//jl		Lp2begin
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		if (TmpAX == 0) goto LoopsEnd;
		if (sy < 0) goto Lp3xx;
		if (sx < 0) goto Lp2begin;
		//dx>dy,sx>0,sy>0
	Lp1begin :
		//inc		si	//x++
		//add		cx, bx
		//cmp		cx, word ptr Mdx
		//jb		Lp1_1
		//sub		cx, word ptr Mdx
		//inc		di  //y++
		TmpSI++;	//x++
		TmpCX += TmpBX;
		if (TmpCX < Mdx) goto Lp1_1;
		TmpCX -= Mdx;
		TmpDI++;	//y++
	Lp1_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		ax
		//jnz		Lp1begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpAX--;
		if (TmpAX != 0) goto Lp1begin;
		goto LoopsEnd;
	Lp2begin : //dx>dy,sx<0,sy>0
		//dec		si	//x--
		//add		cx, bx
		//cmp		cx, word ptr Mdx
		//jb		Lp2_1
		//sub		cx, word ptr Mdx
		//inc		di //y++
		TmpSI--;	//x--
		TmpCX += TmpBX;
		if (TmpCX < Mdx) goto Lp2_1;
		TmpCX -= Mdx;
		TmpDI++;	//y++
	Lp2_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		ax
		//jnz		Lp2begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpAX--;
		if (TmpAX != 0) goto Lp2begin;
		goto LoopsEnd;
	Lp3xx :	//dy<0
		//cmp		sx, 0
		//jl		Lp4begin
		if (sx < 0) goto Lp4begin;
	Lp3begin : //dx>dy,sx>0,sy<0
		//inc		si	//x++
		//add		cx, bx
		//cmp		cx, word ptr Mdx
		//jb		Lp3_1
		//sub		cx, word ptr Mdx
		//dec		di //y--
		TmpSI++;	//x++
		TmpCX += TmpBX;
		if (TmpCX < Mdx) goto Lp3_1;
		TmpCX -= Mdx;
		TmpDI--;	//y--
	Lp3_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		ax
		//jnz		Lp3begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpAX--;
		if (TmpAX != 0) goto Lp3begin;
		goto LoopsEnd;
	Lp4begin : //dx>dy,sx<0,sy<0
		//dec		si	//x--
		//add		cx, bx
		//cmp		cx, word ptr Mdx
		//jb		Lp4_1
		//sub		cx, word ptr Mdx
		//dec		di //y--
		TmpSI--;	//x--
		TmpCX += TmpBX;
		if (TmpCX < Mdx) goto Lp4_1;
		TmpCX -= Mdx;
		TmpDI--;	//y--
	Lp4_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		ax
		//jnz		Lp4begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpAX--;
		if (TmpAX != 0) goto Lp4begin;
		goto LoopsEnd;
	Lp5xx :	//dx<dy
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//or bx, bx
		//jz		LoopsEnd
		//cmp		sx, 0
		//jl		Lp7xx
		//cmp		sy, 0
		//jl		Lp6begin
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		if (TmpBX == 0) goto LoopsEnd;
		if (sx < 0) goto Lp7xx;
		if (sy < 0) goto Lp6Begin;
	Lp5Begin :
		//inc		di	//y++
		//add		cx, ax
		//cmp		cx, word ptr dy
		//jb		Lp5_1
		//sub		cx, word ptr dy
		//inc		si	//x++
		TmpDI++;	//y++
		TmpCX += TmpAX;
		if (TmpCX < dy) goto Lp5_1;
		TmpCX -= dy;
		TmpSI++;	//x++
	Lp5_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		bx
		//jnz		Lp5begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpBX--;
		if (TmpBX != 0) goto Lp5Begin;
		goto LoopsEnd;
	Lp6Begin ://sx>0,sy<0
		//dec		di	//y++
		//add		cx, ax
		//cmp		cx, word ptr dy
		//jb		Lp6_1
		//sub		cx, word ptr dy
		//inc		si	//x++
		TmpDI--;	//y--
		TmpCX += TmpAX;
		if (TmpCX < dy) goto Lp6_1;
		TmpCX -= dy;
		TmpSI++;	//x++
	Lp6_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		bx
		//jnz		Lp6begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpBX--;
		if (TmpBX != 0) goto Lp6Begin;
		goto LoopsEnd;
	Lp7xx :	//dx<0
		//cmp		sy, 0
		//jl		Lp8begin
		if (sy < 0) goto Lp8Begin;
	Lp7Begin ://dx<0,dy>0
		//inc		di	//y++
		//add		cx, ax
		//cmp		cx, word ptr dy
		//jb		Lp7_1
		//sub		cx, word ptr dy
		//dec		si	//x--
		TmpDI++;	//y++
		TmpCX += TmpAX;
		if (TmpCX < dy) goto Lp7_1;
		TmpCX -= dy;
		TmpSI--;	//x--
	Lp7_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		bx
		//jnz		Lp7begin
		//jmp		LoopsEnd
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpBX--;
		if (TmpBX != 0) goto Lp7Begin;
		goto LoopsEnd;
	Lp8Begin ://dx<0,dy<0
		//dec		di	//y--
		//add		cx, ax
		//cmp		cx, word ptr dy
		//jb		Lp8_1
		//sub		cx, word ptr dy
		//dec		si	//x--
		TmpDI--;	//y--
		TmpCX += TmpAX;
		if (TmpCX < dy) goto Lp8_1;
		TmpCX -= dy;
		TmpSI--;	//x--
	Lp8_1 :
		//mov		word ptr[pxx + edx], si
		//mov		word ptr[pyy + edx], di
		//add		edx, 2
		//dec		bx
		//jnz		Lp8begin
		pxx[TmpEDX] = TmpSI;
		pyy[TmpEDX] = TmpDI;
		TmpEDX += 2;
		TmpBX--;
		if (TmpBX != 0) goto Lp8Begin;
	LoopsEnd :
		//shr		edx, 1
		//mov		Pps, edx
		Pps = TmpEDX >> 1;
    }
	Pps--;
	//assert(Pps<MaxP);
	if (InLocked) {
		int uu;
		for (uu = 0; uu < Pps&&MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
		if (uu >= Pps)return false;
		PathX = new short;
		PathY = new short;
		PathX[0] = pxx[uu];
		PathY[0] = pyy[uu];
		NIPoints = 1;
		return true;
    }
	bool RightPrefer = true;
	int Rtx;//current point 
	int Rty;
	int Ltx;
	int Lty;
	word Rpx[MaxP];//right path
	word Rpy[MaxP];
	word Lpx[MaxP];//left path
	word Lpy[MaxP];
	int Rpp = 1;//index of current point
	bool LDoing;//=true if last point reached
	bool RDoing;
	byte Rdirc;//currend direction
	byte Ldirc;
	int Rmaxalt;//maximum alteration,right path
	int Lmaxalt;//maximum alteration,left path
	int Rcum = 0;
	int Rcum1 = 0;
	int Lcum = 0;
	int Lcum1 = 0;
	bool Rvis, Lvis;
	//Проверяем прямую проходимость
	int uu;
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
	if (uu == Pps)return false;
	//Идем, пока не упремся в стенку
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu] - 1, pyy[uu] - 1, Lx + 2, Lx + 2); uu++);
	Rpp = uu;
	Rtx = pxx[Rpp];
	Rty = pyy[Rpp];
	int Rppm = uu;
	int Lppm = uu;
	// Если dx>dy,то на каждом шагу dx изменяетя строго на 1
	if (Rtx != x1 || Rty != y1) {
		//LLock[y][x]=false;
		if (Rpp > 0)Rpp -= 1;
		Rtx = pxx[Rpp];
		Rty = pyy[Rpp];
		Ltx = pxx[Rpp];
		Lty = pyy[Rpp];
		int Ppi = Rpp + 1;
		LDoing = true;
		RDoing = true;
		//Ищем, пока находимся в занятой зоне
		while (MFI->CheckBar(pxx[Ppi] - 1, pyy[Ppi] - 1, Lx + 2, Lx + 2) && Ppi < Pps)Ppi++;
		if (Ppi > Pps)LDoing = false;//Кон.точка недостижима
		int Xls = pxx[Ppi - 1];
		int Yls = pyy[Ppi - 1];
		//Уперлись...Вычисляем направление движения
		Rdirc = ndrr[(pxx[Rpp + 1] - pxx[Rpp] + 1) * 3 + pyy[Rpp + 1] - pyy[Rpp] + 1];
		Ldirc = Rdirc;
		//Выбираем начальное направление-right
		MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
		int dirc1 = (Rdirc + 1) & 7;
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1++, z++);
		if (!NewCirc[6 + (dirc1 & 7)])dirc1++;
		Rdirc = dirc1 & 7;
		//-left
		dirc1 = 8 + ((Ldirc + 7) & 7);
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1--, z++);
		if (!NewCirc[(dirc1 & 7) + 2])dirc1--;
		Ldirc = dirc1 & 7;
		//Идем по правому краю до тех пор пока вновь не пересе-
		//чемся с прямой линией, соединяющей начальную и конечную
		//точки
		Rmaxalt = 0;
		Lmaxalt = 0;
		while (Rpp < MaxP - 8 && LDoing&&RDoing) {
			//пытаемся повернуть направо
			MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
			int dirc1 = (Rdirc + 6) & 7;
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1++, z++);
			Rdirc = dirc1 & 7;
			Rpp++;
			int Tdx = nidrx[Rdirc];
			int Tdy = nidry[Rdirc];
			Rcum -= sdy*Tdx;
			Rcum += sdx*Tdy;
			Rtx += Tdx;
			Rty += Tdy;
			Rpx[Rpp] = Rtx; Rpy[Rpp] = Rty;
			Angle = GetLAngle(Rtx - x, Rty - y, Angle0);
			if (Angle > Rmaxalt) {
				Rmaxalt = Angle;
				Rppm = Rpp;
            }
			//проверяем условие прямой видимости
			ddx = x1 - Rtx;
			ddy = y1 - Rty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			int dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			if (!NewCirc[dirvi]) {
				Rvis = true;
				if (!RvpLast) {
					//int distant=Norma(x1-Rtx,y1-Rty);
					//if(RvpDist>distant){
					Rvp = Rpp;
					RvpLast = true;
					//	RvpDist=distant;
					//}else{
					//	RvpLast=true;
                    //}
                }
			}
			else {
				RvpLast = false;
				Rvis = false;
            }
			//the same, but left
			MFI->FillNewCirc(Ltx - 1, Lty - 1, Lx + 2, Lx + 2);
			dirc1 = 8 + ((Ldirc + 2) & 7);
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1--, z++);
			Ldirc = dirc1 & 7;
			Tdx = nidrx[Ldirc];
			Tdy = nidry[Ldirc];
			Lcum += sdy*Tdx;
			Lcum -= sdx*Tdy;
			Ltx += Tdx;
			Lty += Tdy;
			Lpx[Rpp] = Ltx; Lpy[Rpp] = Lty;
			Angle = GetRAngle(Ltx - x, Lty - y, Angle0);
			if (Angle > Lmaxalt) {
				Lmaxalt = Angle;
				Lppm = Rpp;
            }
			//проверяем условие прямой видимости
			ddx = x1 - Ltx;
			ddy = y1 - Lty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			if (!NewCirc[dirvi]) {
				Lvis = true;
				if (!LvpLast) {
					//int distant=Norma(x1-Ltx,y1-Lty);
					//if(LvpDist>distant){
					Lvp = Rpp;
					LvpLast = true;
					//	LvpDist=distant;
					//}else LvpLast=true;
                }
			}
			else {
				LvpLast = false;
				Lvis = false;
            }
			//Вычисляем значение y на прямой линии, соотв. 
			//данному x
			if (Rcum <= 0 && Rcum1 >= 0) {
				if (!Rvis)Rvp = Rpp;
				if (dx > dy) {
					if (sx > 0) {
						if (Rtx >= Xls)RDoing = false;
					}
					else if (Rtx <= Xls)RDoing = false;
				}
				else {
					if (sy > 0) {
						if (Rty >= Yls)RDoing = false;
					}
					else if (Rty <= Yls)RDoing = false;
                }
				if (!RDoing)RightPrefer = true;
            }
			Rcum1 = Rcum;
			//the same for left
			if (Lcum <= 0 && Lcum1 >= 0) {
				if (!Lvis)Lvp = Rpp;
				if (dx > dy) {
					if (sx > 0) {
						if (Ltx >= Xls)LDoing = false;
					}
					else if (Ltx <= Xls)LDoing = false;
				}
				else {
					if (sy > 0) {
						if (Lty >= Yls)LDoing = false;
					}
					else if (Lty <= Yls)LDoing = false;
                }
				if (!LDoing)RightPrefer = false;
            }
			Lcum1 = Lcum;
        }
		//LLock[y][x]=true;
		if (Rpp < MaxP - 9) {
			if (RightPrefer) {
				//if(Rppm+1<Rpp)Rppm+=1;else Rppm=Rpp;
				//memcpy(xx,Rpx,(Rppm+1));
				//memcpy(yy,Rpy,(Rppm+1));
				Pps = Rppm;
			}
			else {
				//if(Lppm+1<Rpp)Lppm+=1;else Lppm=Rpp;
				//memcpy(xx,Lpx,(Lppm+1));
				//memcpy(yy,Lpy,(Lppm+1));
				Pps = Lppm;
				Rvp = Lvp;
				//for(int G=Pps;G<=Rpp;G++)assert(Lpx[G]<400&&Lpx[G]>0);
            }
			if (Rvp < Rpp)Rvp++;
			if (Rvp < Pps)Rvp = Pps;
			int addNIPoints = Rvp - Pps + 1;
			int maxp = addNIPoints;
			CurIPoint = 0;
			if (addNIPoints > 2) {
				addNIPoints = 2 + ((addNIPoints - 2) >> sssh);
            }
			short* OldPathX = PathX;
			short* OldPathY = PathY;
			PathX = new short[NIPoints + addNIPoints];
			PathY = new short[NIPoints + addNIPoints];
			if (OldPathX) {
				memcpy(PathX, OldPathX, NIPoints << 1);
				memcpy(PathY, OldPathY, NIPoints << 1);
				free(OldPathX);
				free(OldPathY);
            }
			if (RightPrefer) {
				if (maxp <= 2) {
					PathX[NIPoints] = Rpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Rpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Rpx[Pps];
						PathY[NIPoints + 1] = Rpy[Pps];
                    }
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Rpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Rpy[Pps + (nn << sssh)];
                    }
					PathX[NIPoints] = Rpx[Pps + maxp - 1];
					PathY[NIPoints] = Rpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
                }
			}
			else {
				if (maxp <= 2) {
					PathX[NIPoints] = Lpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Lpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Lpx[Pps];
						PathY[NIPoints + 1] = Lpy[Pps];
                    }
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Lpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Lpy[Pps + (nn << sssh)];
                    }
					PathX[NIPoints] = Lpx[Pps + maxp - 1];
					PathY[NIPoints] = Lpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
                }
            }
			NeedPath = true;
			if (GLock)MFI->BSetBar(x, y, Lx);
			return true;
		}
		else {
			if (GLock)MFI->BSetBar(x, y, Lx);
			PathDelay = 64 + (rando() & 15);
			return false;
        }
    }
	if (GLock)MFI->BSetBar(x, y, Lx);
	return false;
}
#undef sssh
#define sssh 3

#undef CreatePath
void OneObject::CreatePath(int x1, int y1) {


	if (UnlimitedMotion) {
		DestX = (x1 << 8) + ((Lx) << 7);
		DestY = (y1 << 8) + ((Lx) << 7);
		CPdestX = (word)-1;
		CPdestY = (word)-1;
		return;
    }
	MotionField* MFI = &MFIELDS[LockType];
	if (CPdestX == x1&&CPdestY == y1) {
		if (PathX) {
			if (NIPoints) {
				//if(MFI->CheckBar(x,y,Lx,Lx)){
					//BClrBar(x,y,Lx);         //???CHECK???
				//	CorrectLockPosition(this);
                //}
				int cx = PathX[NIPoints - 1];
				int cy = PathY[NIPoints - 1];
				int dis = DistTo(cx, cy);
				//if(dis<2&&NIPoints>1)NIPoints--;
				//else if((!dis))NIPoints--;
				if (dis < 2)NIPoints--;
				if (NIPoints) {
					cx = PathX[NIPoints - 1];
					cy = PathY[NIPoints - 1];
					if (MFI->CheckBar(cx, cy, Lx, Lx)) {
						NIPoints = 0;
						free(PathX);
						free(PathY);
						PathX = NULL;
						PathY = NULL;

						CreateFullPath(x1, y1, this);
					}
					else {
						DestX = (cx << 8) + ((Lx) << 7);
						DestY = (cy << 8) + ((Lx) << 7);
                    }
				}
				else {
					free(PathX);
					free(PathY);
					PathX = NULL;
					PathY = NULL;

					CreateFullPath(x1, y1, this);
					if (NIPoints == 1) {
						int cx1 = PathX[0];
						int cy1 = PathY[0];
						if (Norma(cx1 - x, cy1 - y) < 3) {
							free(PathX);
							free(PathY);
							PathX = NULL;
							PathY = NULL;
							NIPoints = 0;
                        }
                    }
                }
			}
			else {

				CreateFullPath(x1, y1, this);
            }
		}
		else {
			DestX = (x1 << 8) + ((Lx) << 7);
			DestY = (y1 << 8) + ((Lx) << 7);
        }
	}
	else {
		if (PathX) {
			free(PathX);
			free(PathY);
			NIPoints = 0;
			PathX = NULL;
			PathY = NULL;
        }

		CreateFullPath(x1, y1, this);
    }
	CPdestX = x1;
	CPdestY = y1;
}
