#ifndef GFILE_H
#define GFILE_H

#include <stdio.h>

#include "ResFile.h"

class GFILE{
    unsigned char Buf[16384];
	int NBytesRead;
	int BufPos;
	int GlobalPos;
	int Size;
	ResFile F;
public:
	FILE* rf;
	bool RealText;
	GFILE();
	~GFILE();
    bool Open(const char *Name);
	void Close();
	int ReadByte();
	int CheckByte();
	//standart functions
    int Gscanf(const char *Mask, va_list args);
	int Ggetch();
};

__declspec(dllexport) GFILE* Gopen(const char *Name, const char *Mode);
__declspec(dllexport) int Gscanf(GFILE* F, const char* mask,...);
__declspec(dllexport) int Ggetch(GFILE* F);
__declspec(dllexport) void Gprintf(GFILE* F,const char *format,...);
__declspec(dllexport) void Gclose(GFILE* F);

inline void NLine(GFILE* f)
{
    char tt[8];
    int zz;
    do {
        zz = Gscanf(f, "%lc", tt);
    } while (tt[0] != 10 && zz == 1);
}

#endif // GFILE_H
