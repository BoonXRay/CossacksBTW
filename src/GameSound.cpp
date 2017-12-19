#include <cassert>

#include "3DGraph.h"
#include "CDirSnd.h"
#include "Fog.h"
#include "Interface.h"
#include "LoadSave.h"
#include "main.h"
#include "Mapa.h"
#include "Multi.h"
#include "stRecog.h"

#include "GameSound.h"


int FieldDelay = 0;
static bool SoundOK;
CDirSound* CDS;
char* SoundID[MaxSnd];
static uint16_t SndTable[MaxSnd][16];
static uint8_t SnDanger[MaxSnd];
static uint16_t NSnd[MaxSnd];
uint16_t NSounds;

static void Errs( const char * s)
{
	MessageBox(hwnd, s, "Sound loading failed...", MB_ICONWARNING | MB_OK);
	assert(false);
}

void LoadSounds(const char* fn) {
	char sss[128];
	char idn[128];
    int nsn, z;
	NSounds = 0;
	SoundOK = CDS->DirectSoundOK() != 0;
	GFILE* f1 = Gopen(fn, "rt");

	char ccpr[128];
	ccpr[0] = 0;
	if (f1 != INVALID_HANDLE_VALUE) {
		do {
		uuux:	int Extent = 0;
			z = Gscanf(f1, "%s", idn);
			if (idn[0] == '/') {
				NLine(f1);
				goto uuux;
            }
			if (idn[0] == '#')Extent = 1;
			z = Gscanf(f1, "%d", &nsn);
			NSnd[NSounds] = nsn;
			SoundID[NSounds] = new char[strlen(idn) + 1 - Extent];
			strcpy(SoundID[NSounds], idn + Extent);
			if (z) {
				NLine(f1);
				SnDanger[NSounds] = 0;
				for (int i = 0; i < nsn; i++) {
					int Vol = 0;
				uuu:			if (Extent) {
					z = Gscanf(f1, "%s%d", sss, &Vol);
				}
								else {
									z = Gscanf(f1, "%s", sss);
                                }
								if (sss[0] == '/') {
									NLine(f1);
									goto uuu;
                                }
								if (!strcmp(sss, "DANGER")) {
									SnDanger[NSounds] = 1;
									goto uuu;
                                }
								normstr(sss);
								if (SoundOK) {
									if (!strcmp(ccpr, sss)) {
										SndTable[NSounds][i] = CDS->DuplicateSoundBuffer(CDS->m_currentBufferNum);
										CDS->SetLastVolume(Vol);
									}
									else {
										CWave CW(sss);

										int sdid;
										if (CW.WaveOK()) {
											sdid = CDS->CreateSoundBuffer(&CW);
											CDS->SetLastVolume(Vol);
											if (CDS->DirectSoundOK()) {
												CDS->CopyWaveToBuffer(&CW, sdid);
											}
											else {
												sprintf(idn, "Could not create sound buffer : %s", sss);
												Errs(idn);
                                            }
											SndTable[NSounds][i] = sdid;
											strcpy(ccpr, sss);
										}
										else {
											//sprintf(idn,"Could not load sound : %s",sss);
											//Errs(idn);
											nsn--;
											NSnd[NSounds]--;
											i--;
                                        }
                                    }
                                }
                }
            }
			NSounds++;
		} while (z);
	}
	else {
		sprintf(sss, "Could not open sounds list : %s", fn);
		Errs(sss);
    }
	//NSounds=0;
	Gclose(f1);
	//if(!CDS->DirectSoundOK()){
	//	NSounds=0;
    //}
	//NoMineSound=GetSound("NOFREEMINES");
}

void PlayEffect(int n, int pan, int vol) {
	if (!SoundOK)return;
	vol -= (100 - WarSound) * 40;
	if (n < NSounds) {
		if (SnDanger[n])FieldDelay = 400;
		if (NSnd[n] > 0) {
			int maxsnd = NSnd[n];
			int u = maxsnd;
			int nnn = (srando()*maxsnd) >> 15;
			bool sndmade = true;
			do {
				int sid = SndTable[n][nnn];
				bool poss = CDS->IsPlaying(sid);
				if (!poss) {
					CDS->SetVolume(sid, vol/*+CDS->Volume[sid]*/);
					CDS->SetPan(sid, pan);
					CDS->PlaySound(sid);
					sndmade = false;
				}
				else {
					u--;
					nnn++;
					if (nnn >= maxsnd)nnn = 0;
                }
			} while (sndmade&&u);
			/*
			if(sndmade&&srando()<200){
				int nnn=(srando()*maxsnd)>>15;
				CDS->SetVolume(SndTable[n][nnn],vol);
				CDS->SetPan(SndTable[n][nnn],pan);
				CDS->PlaySound(SndTable[n][nnn]);
            }
			*/
        }
    }
}

static void PlayCoorEffect(int n, int x, int y, int pan, int vol) {
	if (!SoundOK)return;
	vol -= (100 - WarSound) * 40;
	if (n < NSounds) {
		if (SnDanger[n])FieldDelay = 400;
		if (NSnd[n] > 0) {
			int maxsnd = NSnd[n];
			int u = maxsnd;
			int nnn = (srando()*maxsnd) >> 15;
			bool sndmade = true;
			do {
				int sid = SndTable[n][nnn];
				bool poss = CDS->IsPlaying(sid);
				if (!poss) {
					CDS->SetVolume(sid, vol/*+CDS->Volume[sid]*/);
					CDS->SetPan(sid, pan);
					CDS->PlayCoorSound(sid, x, y);
					sndmade = false;
				}
				else {
					u--;
					nnn++;
					if (nnn >= maxsnd)nnn = 0;
                }
			} while (sndmade&&u);
			/*
			if(sndmade&&srando()<200){
				int nnn=(srando()*maxsnd)>>15;
				CDS->SetVolume(SndTable[n][nnn],vol);
				CDS->SetPan(SndTable[n][nnn],pan);
				CDS->PlaySound(SndTable[n][nnn]);
            }
			*/
        }
    }
}

static void PlaySingleEffect(int n, int pan, int vol) {
	if (!SoundOK)return;
	vol -= (100 - WarSound) * 40;
	if (n < NSounds) {
		if (SnDanger[n])FieldDelay = 400;
		if (NSnd[n] > 0) {
			int maxsnd = NSnd[n];
			for (int i = 0; i < maxsnd; i++) {
				int sid = SndTable[n][i];
				if (CDS->IsPlaying(sid))return;
            }

			int u = maxsnd;
			int nnn = (srando()*maxsnd) >> 15;
			bool sndmade = true;
			do {
				int sid = SndTable[n][nnn];
				bool poss = CDS->IsPlaying(sid);
				if (!poss) {
					CDS->SetVolume(sid, vol);
					CDS->SetPan(sid, pan);
					CDS->PlaySound(sid);
					sndmade = false;
				}
				else {
					u--;
					nnn++;
					if (nnn >= maxsnd)nnn = 0;
                }
			} while (sndmade&&u);
			/*
			if(sndmade&&srando()<200){
				int nnn=(srando()*maxsnd)>>15;
				CDS->SetVolume(SndTable[n][nnn],vol);
				CDS->SetPan(SndTable[n][nnn],pan);
				CDS->PlaySound(SndTable[n][nnn]);
            }
			*/
        }
    }
}

static int SMinX, SMaxX, LoMinX, LoMaxX, SMinY, SMaxY, LoMinY, LoMaxY;
int CenterX;

void PrepareSound() {
	if (FieldDelay)FieldDelay--;
	SMinX = mapx << 5;
	SMaxX = (mapx + smaplx) << 5;
	CenterX = (SMinX + SMaxX) >> 1;
	LoMinX = SMinX - 1024;
	LoMaxX = SMaxX + 1024;

	SMinY = mapy << 4;
	SMaxY = (mapy + smaply) << 4;
    LoMinY = SMinY - 512;
	LoMaxY = SMaxY + 512;
}

void AddEffectV(int x, int y, int vx, int id) {
	if (!SoundOK)return;
	if (id < 0)return;
	if (x<LoMinX || x>LoMaxX || y<LoMinY || y>LoMaxY)return;
	//fog checking;
	if (FogMode && (!NATIONS[MyNation].Vision) && GetFog(x, y << 1) < 900)return;

	//int maxp=div(8000,smaplx).quot;
	int pan = (x - CenterX) << 1;
	if (pan < -4000)pan = -4000;
	if (pan > 4000)pan = 4000;
	//int pan=-9999;
	if (x >= SMinX&&y >= SMinY&&x < SMaxX&&y < SMaxY) {
		PlayCoorEffect(id, x, vx, pan, 0);
	}
	else {
		PlayCoorEffect(id, x, vx, pan, -800);
    }
}

void AddEffect(int x, int y, int id) {
	AddEffectV(x, y, 0, id);
}

void AddUnlimitedEffect(int x, int y, int id) {
	if (!SoundOK)return;
	if (id < 0)return;
	//fog checking;
	if (GetFog(x, y << 1) < 900 && FogMode)return;

	//int maxp=div(8000,smaplx).quot;
	int pan = (x - CenterX) << 1;
	if (pan < -4000)pan = -4000;
	if (pan > 4000)pan = 4000;
	//int pan=-9999;
	PlayEffect(id, pan, 0);
}

void AddSingleEffect(int x, int y, int id) {
	if (!SoundOK)return;
	if (id < 0)return;
	if (x<LoMinX || x>LoMaxX || y<LoMinY || y>LoMaxY)return;
	//fog checking;
	if (GetFog(x, y << 1) < 900 && FogMode)return;

	//int maxp=div(8000,smaplx).quot;
	int pan = (x - CenterX) << 1;
	if (pan < -4000)pan = -4000;
	if (pan > 4000)pan = 4000;
	//int pan=-9999;
	if (x >= SMinX&&y >= SMinY&&x < SMaxX&&y < SMaxY) {
		PlaySingleEffect(id, pan, 0);
	}
	else {
		PlaySingleEffect(id, pan, -800);
    }
}

void AddOrderEffect(int x, int y, int id) 
{
	if (!SoundOK)
	{
		return;
	}
	if (GetV_fmap(x >> 2, y >> 2) < 2000)
	{
		return;
	}

	if (id < 0)
	{
		return;
	}

	int mx0 = mapx - 8;
	int my0 = mapy - 8;
	int mx1 = mapx + smaplx + 16;
	int my1 = mapy + smaply + 16;
	int maxp = div(8000, smaplx).quot;
	int pan = (x - (smaplx / 2) - mapx)*maxp;

	if (pan < -4000)
	{
		pan = -4000;
	}

	if (pan > 4000)
	{
		pan = 4000;
	}

	if (x >= mapx && y >= mapy && x < mapx + smaplx && y < mapy + smaply) {
		PlayEffect(id, pan, OrderSound);
	}
	else 
	{
		if (x > mx0 && y > my0 && x < mx1 && y < my1) 
		{
			PlayEffect(id, pan, OrderSound - 400);
		}
	}
}
