#include <cmath>
#include <cstdio>

#include "3DGraph.h"
#include "Ddini.h"
#include "FastDraw.h"
#include "GP_Draw.h"
#include "LoadSave.h"
#include "main.h"
#include "Mapa.h"
#include "Mouse_X.h"
#include "NewMon.h"
#include "ResFile.h"
#include "Weapon.h"

#include "ShipTrace.h"


class Blob
{
public:
	short Lx;
	short Ly;
	short N;
	short H;
	byte* Data;
	Blob();
	~Blob();
    void Load( const char * Name );
	void Show(int x, int y, int N);
};

Blob::Blob()
{
	Data = NULL;
	Lx = 0;
	Ly = 0;
}

Blob::~Blob()
{
	if (Data)free(Data);
}

void Blob::Load( const char * Name ) {
	ResFile f = RReset(Name);
	if (f == INVALID_HANDLE_VALUE) {
		char ccc[128];
		sprintf(ccc, "Could not load: %s", Name);
		ErrM(ccc);
		return;
    }
	RBlockRead(f, &Lx, 2);
	RBlockRead(f, &Ly, 2);
	RBlockRead(f, &N, 2);
	RBlockRead(f, &H, 2);
	Data = new byte[Lx*Ly*N];
	RBlockRead(f, Data, Lx*Ly*N);
	RClose(f);
}

static uint8_t TraceGrd[4096];

static void ShowBlob(int x, int y, byte* Blob, int Lx, int Ly)
{
	if (x > WindX1 || y > WindY1)return;
	byte* bof = Blob;
	int BLX = Lx;
	int BLY = Ly;
	if (x < WindX) {
		if (x + Lx <= WindX)return;
		bof -= x - WindX;
		BLX += x - WindX;
		x = 0;
    }
	if (y < WindY) {
		if (y + Ly <= WindY)return;
		bof -= (y - WindY)*Lx;
		BLY += y - WindY;
		y = 0;
    }
	if (x + Lx > WindX1 + 1) {
		BLX -= x + Lx - WindX1 - 1;
    }
	if (y + Ly > WindY1 + 1) {
		BLY -= y + Ly - WindY1 - 1;
    }
	if (BLX <= 0 || BLY <= 0) {
		//assert(BLX>0&&BLY>0);
		return;
    }
    /*int sof = int(ScreenPtr) + x + y*ScrWidth;
    int bufo = int(bof);*/
	int scadd = ScrWidth - BLX;
	int badd = Lx - BLX;
	if (BLX & 3) {
		// BoonXRay 14.08.2017
		//__asm 
		{
			//push	esi
			//push	edi
			//mov		esi, bufo
			//mov		edi, sof
			//mov		ch, byte ptr BLY
			//xor		eax, eax
			unsigned char * TmpESI = bof;
			unsigned char * TmpEDI = reinterpret_cast<unsigned char *>(ScreenPtr) + x + y*ScrWidth;
			unsigned int TmpEAX = 0, TmpECX = 0;
			unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpAH = *(reinterpret_cast<unsigned char *>(&TmpEAX) + 1);
			unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
			unsigned char & TmpCH = *(reinterpret_cast<unsigned char *>(&TmpECX) + 1);
			TmpCH = BLY;
		lab1 :
			//mov		cl, byte ptr BLX
			TmpCL = BLX;
		lab2 :
			//mov		ah, [esi]
			//mov		al, [edi]
			//mov		al, [TraceGrd + eax]
			//mov[edi], al
			//inc		esi
			//inc		edi
			TmpAH = *TmpESI;
			TmpAL = *TmpEDI;
			TmpAL = TraceGrd[TmpEAX];
			*TmpEDI = TmpAL;
			TmpESI++;
			TmpEDI++;
			//dec		cl
			//jnz		lab2
			//add		esi, badd
			//add		edi, scadd
			//dec		ch
			//jnz		lab1
			TmpCL--;
			if (TmpCL != 0) goto lab2;
			TmpESI += badd;
			TmpEDI += scadd;
			TmpCH--;
			if (TmpCH != 0) goto lab1;
			//pop		edi
			//pop		esi
        }
	}
	else {
		BLX >>= 2;
		// BoonXRay 14.08.2017
		//__asm 
		{
			//push	esi
			//push	edi
			//mov		esi, bufo
			//mov		edi, sof
			//mov		ch, byte ptr BLY
			//xor		eax, eax
			unsigned char * TmpESI = bof;
			unsigned char * TmpEDI = reinterpret_cast<unsigned char *>(ScreenPtr) + x + y*ScrWidth;
			unsigned int TmpEAX = 0, TmpEBX = 0, TmpECX = 0, TmpEDX = 0;
			unsigned char & TmpAL = *reinterpret_cast<unsigned char *>(&TmpEAX);
			unsigned char & TmpAH = *(reinterpret_cast<unsigned char *>(&TmpEAX) + 1);
			unsigned char & TmpBL = *reinterpret_cast<unsigned char *>(&TmpEBX);
			unsigned char & TmpCL = *reinterpret_cast<unsigned char *>(&TmpECX);
			unsigned char & TmpCH = *(reinterpret_cast<unsigned char *>(&TmpECX) + 1);
			unsigned char & TmpDL = *reinterpret_cast<unsigned char *>(&TmpEDX);
			TmpCH = BLY;
		lab1o :
			//mov		cl, byte ptr BLX
			TmpCL = BLX;
		lab2o :
			//mov		ebx, [esi]
			//mov		edx, [edi]
			//mov		ah, bl
			//mov		al, dl
			//shr		ebx, 8
			//mov		al, [TraceGrd + eax]
			//mov		dl, al
			//mov		ah, bl
			//ror		edx, 8
			//mov		al, dl
			TmpEBX = *reinterpret_cast<unsigned int *>(TmpESI);
			TmpEDX = *reinterpret_cast<unsigned int *>(TmpEDI);
			TmpAH = TmpBL;
			TmpAL = TmpDL;
			TmpEBX >>= 8;
			TmpAL = TraceGrd[TmpEAX];
			TmpDL = TmpAL;
			TmpAH = TmpBL;
			unsigned int TmpUInt = TmpEDX << (32 - 8);
			TmpEDX = (TmpEDX >> 8) | TmpUInt;
			TmpAL = TmpDL;

			//shr		ebx, 8
			//mov		al, [TraceGrd + eax]
			//mov		dl, al
			//mov		ah, bl
			//ror		edx, 8
			//mov		al, dl
			TmpEBX >>= 8;
			TmpAL = TraceGrd[TmpEAX];
			TmpDL = TmpAL;
			TmpAH = TmpBL;
			TmpUInt = TmpEDX << (32 - 8);
			TmpEDX = (TmpEDX >> 8) | TmpUInt;
			TmpAL = TmpDL;

			//add		edi, 4
			TmpEDI += 4;

			//shr		ebx, 8
			//mov		al, [TraceGrd + eax]
			//mov		dl, al
			//mov		ah, bl
			//ror		edx, 8
			//mov		al, dl
			TmpEBX >>= 8;
			TmpAL = TraceGrd[TmpEAX];
			TmpDL = TmpAL;
			TmpAH = TmpBL;
			TmpUInt = TmpEDX << (32 - 8);
			TmpEDX = (TmpEDX >> 8) | TmpUInt;
			TmpAL = TmpDL;

			//add		esi, 4
			TmpESI += 4;

			//mov		al, [TraceGrd + eax]
			//mov		dl, al
			//ror		edx, 8
			TmpAL = TraceGrd[TmpEAX];
			TmpDL = TmpAL;
			TmpUInt = TmpEDX << (32 - 8);
			TmpEDX = (TmpEDX >> 8) | TmpUInt;
			//dec		cl

			//mov[edi - 4], edx

			//jnz		lab2o
			//add		esi, badd
			//add		edi, scadd
			//dec		ch
			//jnz		lab1o
			*reinterpret_cast<unsigned int *>(TmpEDI - 4) = TmpEDX;
			TmpCL--;
			if (TmpCL != 0) goto lab2o;
			TmpESI += badd;
			TmpEDI += scadd;
			TmpCH--;
			if (TmpCH != 0) goto lab1o;
			//pop		edi
			//pop		esi
        }
    }
}

void Blob::Show(int x, int y, int m)
{
	if (m >= N || m < 0)return;
	ShowBlob(x - (Lx >> 1), y - (Ly >> 1), Data + Lx*Ly*m, Lx, Ly);
}

//-------------------Traces system--------------------
static Blob Blob1;

static constexpr uint16_t MaxBlob = 1024;
static int BlobX[MaxBlob];
static int BlobY[MaxBlob];
static int BlobVx[MaxBlob];
static int BlobVy[MaxBlob];
static uint8_t BlobTime[MaxBlob];
static uint8_t BlobVisible[MaxBlob];
static uint8_t BlobOpt[MaxBlob];
static int NBlobs;
static int CurBlob;

static void ProcessBlobs()
{
    if (!NBlobs)return;
	int MinX = (mapx << 9) - 32 * 16;
	int MinY = (mapy << 9) - 32 * 16;
	int MaxX = ((mapx + smaplx) << 9) + 32 * 16;
	int MaxY = ((mapy + smaply) << 9) + 32 * 16;
	memset(BlobVisible, 0, MaxBlob);
    // BoonXRay 14.08.2017
	//__asm 
	{
		//push	esi
		//push	edi
		//mov		esi, NBlobs;
		//xor		ecx, ecx
		//mov		ebx, MinX
		//mov		edx, MaxX
		//mov		edi, MinY
		int TmpESI = NBlobs;
		int TmpEAX = 0, TmpEBX = MinX, TmpECX = 0, TmpEDX = MaxX, TmpEDI = MinY;
		char & TmpAL = *reinterpret_cast<char *>(&TmpEAX);
	lpp1 : 
		//mov		al, [BlobTime + ecx]
		//or al, al
		//jnz		lpp2
		//inc		ecx
		//dec		esi
		//jnz		lpp1
		//jmp     final_label
		TmpAL = BlobTime[TmpECX];
		if (TmpAL != 0) goto lpp2;
		TmpECX++;
		TmpESI--;
		if (TmpESI != 0) goto lpp1;
		goto final_label;
	lpp2:	
		//mov		eax, [BlobVx + ecx * 4]
		//add[BlobX + ecx * 4], eax
		//mov		eax, [BlobVy + ecx * 4]
		//add[BlobY + ecx * 4], eax
		//dec		byte ptr[BlobTime + ecx]
		//mov		eax, [BlobX + ecx * 4]
		BlobX[TmpECX] += BlobVx[TmpECX];
		BlobY[TmpECX] += BlobVy[TmpECX];
		BlobTime[TmpECX]--;
		TmpEAX = BlobX[TmpECX];
		//cmp		eax, ebx
		//jl		NoShow
		//cmp		eax, edx
		//jg		NoShow
		//mov		eax, [BlobY + ecx * 4]
		//cmp		eax, edi
		//jl		NoShow
		//cmp		eax, MaxY
		//jg		NoShow
		//mov		byte ptr[BlobVisible + ecx], 1
		if (TmpEAX < TmpEBX) goto NoShow;
		if (TmpEAX > TmpEDX) goto NoShow;
		TmpEAX = BlobY[TmpECX];
		if (TmpEAX < TmpEDI) goto NoShow;
		if (TmpEAX > MaxY) goto NoShow;
		BlobVisible[TmpECX] = 1;
	NoShow: 
		//inc		ecx
		//dec		esi
		//jnz		lpp1
		TmpECX++;
		TmpESI--;
		if (TmpESI != 0) goto lpp1;
	final_label:
		//pop		edi
		//pop		esi
		;
	}

	MinX = mapx * 32;
	MinY = mapy * 16;

	for (int i = 0; i < NBlobs; i++)
	{
		if (BlobVisible[i])
		{
			int p = BlobTime[i];
			byte op = BlobOpt[i];
			if (p)
			{
				int n = Blob1.N;

				if (p < n)
				{
					p = n - p;
				}
				else
				{
					p -= n;
				}

				if (p < 0)
					p = 0;

				if (p >= n)
					p = n - 1;

				if (op)
				{
					p = 20 + (i % 18);
				}

				Blob1.Show((BlobX[i] >> 4) - MinX, (BlobY[i] >> 5) - MinY, p);
			}
		}
		else
		{
			BlobTime[i] = 0;
		}
	}
}

//------------------------LOADING&TEST-----------------
void LoadBlobs()
{
    ResFile f = RReset("wave.grd");
    RBlockRead(f, TraceGrd, 4096);
    RClose(f);
    Blob1.Load("Wave1.pix");
}

void TestBlob() {
    ProcessBlobs();
}



static short BDVX[256];
static short BDVY[256];

void InitBlobs()
{
	for (int i = 0; i < 256; i++)
	{
		BDVX[i] = (TCos[i]) >> 4;
		BDVY[i] = (TSin[i]) >> 4;
	}
	NBlobs = 0;
	CurBlob = 0;
}

static void AddBlob(int x, int y, byte Dir, bool dir2)
{
	int Cur;
	if (NBlobs < MaxBlob)
	{
		Cur = NBlobs;
		NBlobs++;
		CurBlob++;
		if (NBlobs == MaxBlob)
		{
			CurBlob = 0;
		}
	}
	else
	{
		// BoonXRay 14.08.2017
		//__asm
		{
			//mov		eax, CurBlob
			//mov		ecx, MaxBlob
			//sub		ecx, eax
			int TmpEAX = CurBlob, TmpECX = MaxBlob - CurBlob;
		lpp1:
			//cmp		byte ptr[BlobTime + eax], 0
			//je		lpp2
			//inc		eax
			//dec		ecx
			//jnz		lpp1
			//mov		Cur, -1
			//jmp		lpp3
			if (BlobTime[TmpEAX] == 0) goto lpp2;
			TmpEAX++;
			TmpECX--;
			if (TmpECX != 0) goto lpp1;
			Cur = -1;
			goto lpp3;
		lpp2:
			//mov		Cur, eax
			//inc		eax
			Cur = TmpEAX;
			TmpEAX++;
		lpp3:
			//mov		CurBlob, eax
			CurBlob = TmpEAX;
		}
	}	

	if (CurBlob >= MaxBlob)
	{
		CurBlob = 0;
	}

	if (Cur == -1)
	{
		return;
	}

	BlobX[Cur] = x;
	BlobY[Cur] = y;
	BlobVx[Cur] = BDVX[Dir];
	BlobVy[Cur] = BDVY[Dir];

	if (dir2)
	{
		BlobTime[Cur] = (Blob1.N) << 1;
	}
	else
	{
		BlobTime[Cur] = (Blob1.N);
	}

	BlobOpt[Cur] = dir2;
}

static void AddBlob(int x, int y, byte Dir)
{
	AddBlob(x, y, Dir, 0);
}

static int GetRiverDir(int x, int y);

void AddRandomBlobs()
{
	int x0 = (mapx << 5);
	int y0 = (mapy << 5);
	for (int i = 0; i < 80; i++)
	{
		int x = x0 + ((srando()*RealLx) >> 15);
		int y = y0 + ((srando()*RealLy) >> 14);
		int dir = GetRiverDir(x, y);
		if (dir != -1)
		{
            AddBlob(x << 4, y << 4, dir, 1);
		}
	}
}

static void AddTrace(int x, int y, uint8_t Dir)
{
	AddBlob(x, y, Dir + 64);
	AddBlob(x, y, Dir - 64);
}

void CreateTrace(OneObject* OB)
{
	if (tmtmt & 3)
	{
		return;
	}

	NewMonster* NM = OB->newMons;
	byte ANGL = OB->RealDir - 64;

	int dz = NM->WaveDZ;
	int SIN = TSin[ANGL];
	int COS = TCos[ANGL];

	if (NM->NWaves)
	{
		int N = NM->NWaves;
		for (int i = 0; i < N; i++)
		{
			int x = NM->WavePoints[i + i];
			int y = NM->WavePoints[i + i + 1];
			int xx = (x*COS - y*SIN) >> 4;
			int yy = ((y*COS + x*SIN) >> 4) + (dz << 5);
			xx += srando() >> 6;
			yy += srando() >> 7;
			AddTrace(OB->RealX + xx, OB->RealY + yy, OB->RealDir);
		}
	}
}

uint8_t * RivDir = nullptr;
uint8_t * RivVol = nullptr;

void InitRiv()
{
    memset(RivDir, 0, RivNX*RivNX);
    memset(RivVol, 0, RivNX*RivNX);
}

bool RiverEditMode = 0;

void ProcessRivEditor()
{
    if (!EditMapMode)
    {
        return;
    }

    if (RiverEditMode)
    {
        int x0 = -((mapx % 4) << 5) - 128;
        int y0 = -((mapy % 4) << 4) - 64;
        int vx0 = (mapx >> 2) - 1;
        int vy0 = (mapy >> 2) - 1;
        int Nx = (smaplx >> 2) + 2;
        int Ny = (smaply >> 2) + 2;

        for (int ix = 0; ix < Nx; ix++)
        {
            Vline(x0 + (ix << 7), 0, RealLy, 0xFD);
        }

        for (int iy = 0; iy < Ny; iy++)
        {
            Hline(0, y0 + (iy << 6), RealLx, 0xFD);
        }

        for (int ix = 0; ix < Nx; ix++)
        {
            for (int iy = 0; iy < Ny; iy++)
            {
                int ofs = ix + vx0 + ((iy + vy0) << RivSH);
                if (RivVol[ofs])
                {
                    byte dir = RivDir[ofs];
                    int vol = RivVol[ofs];
                    int dx = (int(TCos[dir])*vol) >> 8;
                    int dy = (int(TSin[dir])*vol) >> 9;
                    int xx = ((ix + vx0) << 7) - (mapx << 5) + 64;
                    int yy = ((iy + vy0) << 6) - (mapy << 4) + 32;
                    CBar(xx - 3, yy - 3, 6, 6, 0xFE);
                    xLine(xx, yy, xx + dx, yy + dy, 0xFE);
                }
            }
        }

        if (Lpressed || Rpressed)
        {
            int px = mouseX + (mapx << 5);
            int py = (mouseY << 1) + (mapy << 5);
            int nx = (px >> 7);
            int ny = (py >> 7);
            if (nx >= 0 && ny >= 0)
            {
                px -= (nx << 7) + 64;
                py -= (ny << 7) + 64;
                byte dir = GetDir(px, py);
                int vol;

                if (Rpressed)
                {
                    vol = 0;
                }
                else
                {
                    vol = int(sqrt(px*px + py*py));
                }

                int ofs = nx + (ny << RivSH);
                RivVol[ofs] = vol;
                RivDir[ofs] = dir;
            }
        }
    }
}

static int GetRiverDir(int x, int y)
{
    x >>= 7;
    y >>= 7;
    if (x >= 0 && y >= 0)
    {
        int ofs = x + (y << RivSH);
        if (RivVol[ofs])
        {
            return RivDir[ofs];
        }
    }
    return -1;
}
