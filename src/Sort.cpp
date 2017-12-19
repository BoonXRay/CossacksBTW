#include <cstring>
#include <cstdlib>

#include "Mapa.h"
#include "MapDescr.h"
#include "NewMon.h"

#include "Sort.h"


SortClass::SortClass()
{
    NUids = 0;
    MaxUID = 0;
    Uids = NULL;
    //Usn=NULL;
    Parms = NULL;
}

SortClass::~SortClass()
{
    if (MaxUID) {
        free(Uids);
        free(Parms);
        //free(Usn);
    }
}

void SortClass::Sort()
{
    if (NUids < 2)return;
    int uids = int(Uids);
    //int sns=int(Usn);
    //int snsn;
    int parms = int(Parms);
    int nuids = NUids - 1;
    uint8_t fault;

    // BoonXRay 12.08.2017
    //__asm
    {
        //push    esi
        //push    edi
        //mov     esi, uids
        //mov     edi, parms
        //mov     ecx, nuids
        int TmpESI = uids;
        int TmpEDI = parms;
        int TmpECX = nuids;
        int TmpEBX = 0, TmpEDX = 0;
        short & TmpBX = *reinterpret_cast<short *>(&TmpEBX);
        short TmpUShort = 0;
    lpp1 :
        //xor     eax, eax
        //mov     esi, uids
        //mov     edi, parms
        //mov     ecx, nuids
        //mov     ecx, nuids
        //mov     fault, 0
        TmpESI = uids;
        TmpEDI = parms;
        TmpECX = nuids;
        fault = 0;
    lpp3:
        //mov     ebx, [edi]
        //mov     edx, [edi + 4]
        //cmp     ebx, edx
        //jle     lpp2
        TmpEBX = *reinterpret_cast<int *>(TmpEDI);
        TmpEDX = *reinterpret_cast<int *>(TmpEDI + 4);
        if (TmpEBX <= TmpEDX) goto lpp2;
        //mov[edi], edx
        //mov[edi + 4], ebx
        //mov     bx, [esi]
        *reinterpret_cast<int *>(TmpEDI) = TmpEDX;
        *reinterpret_cast<int *>(TmpEDI + 4) = TmpEBX;
        TmpBX = *reinterpret_cast<short *>(TmpESI);
        //xchg    bx, [esi + 2]
        //mov[esi], bx
        //mov     fault, 1
        TmpUShort = *reinterpret_cast<short *>(TmpESI + 2);
        *reinterpret_cast<short *>(TmpESI + 2) = TmpBX;
        TmpBX = TmpUShort;
        *reinterpret_cast<short *>(TmpESI) = TmpBX;
        fault = 1;
    lpp2:
        //add     esi, 2
        //add     edi, 4
        //dec     ecx
        //jnz     lpp3
        //cmp     fault, 0
        //jnz      lpp1
        TmpESI += 2;
        TmpEDI += 4;
        TmpECX--;
        if (TmpECX != 0) goto lpp3;
        if (fault != 0) goto lpp1;
        //pop     esi
        //pop     edi
    }
}

void SortClass::CheckSize(int Size)
{
    if (!Size)return;
    if (Size > MaxUID) {
        if (MaxUID) {
            free(Uids);
            free(Parms);
            //free(Usn);
        }
        Uids = new uint16_t[Size];
        Parms = new int[Size];
        //Usn=new word[Size];
        MaxUID = Size;
    }
}

uint16_t SortClass::CreateByX(uint16_t *  UIDS, int NUIDS, bool Direction)
{
    CheckSize(NUIDS);
    uint16_t realn = 0;
    if (Direction) {
        for (int i = 0; i < NUIDS; i++) {
            uint16_t MID = UIDS[i];
            if (MID != 0xFFFF) {
                OneObject* OB = Group[MID];
                if (OB && !OB->Sdoxlo) {
                    Uids[realn] = MID;
                    //Usn[realn]=USN[i];
                    UIDS[realn] = MID;
                    Parms[realn] = OB->RealX;
                    realn++;
                }
            }
        }
    }
    else {
        for (int i = 0; i < NUIDS; i++) {
            uint16_t MID = UIDS[i];
            if (MID != 0xFFFF) {
                OneObject* OB = Group[MID];
                if (OB && !OB->Sdoxlo) {
                    Uids[realn] = MID;
                    //Usn[realn]=USN[i];
                    UIDS[realn] = MID;
                    Parms[realn] = -OB->RealX;
                    realn++;
                }
            }
        }
    }
    NUids = realn;
    return realn;
}

uint16_t SortClass::CreateByY(uint16_t *  UIDS, int NUIDS, bool Direction)
{
    CheckSize(NUIDS);
    uint16_t realn = 0;
    if (Direction) {
        for (int i = 0; i < NUIDS; i++) {
            uint16_t MID = UIDS[i];
            if (MID != 0xFFFF) {
                OneObject* OB = Group[MID];
                if (OB && !OB->Sdoxlo) {
                    Uids[realn] = MID;
                    //Usn[realn]=USN[i];
                    UIDS[realn] = MID;
                    Parms[realn] = OB->RealY;
                    realn++;
                }
            }
        }
    }
    else {
        for (int i = 0; i < NUIDS; i++) {
            uint16_t MID = UIDS[i];
            if (MID != 0xFFFF) {
                OneObject* OB = Group[MID];
                if (OB && !OB->Sdoxlo) {
                    Uids[realn] = MID;
                    //Usn[realn]=USN[i];
                    UIDS[realn] = MID;
                    Parms[realn] = -OB->RealY;
                    realn++;
                }
            }
        }
    }
    NUids = realn;
    return realn;
}

uint16_t SortClass::CreateByR(uint16_t *  UIDS, int NUIDS, bool Direction, int x, int y)
{
    CheckSize(NUIDS);
    uint16_t realn = 0;
    if (Direction) {
        for (int i = 0; i < NUIDS; i++) {
            uint16_t MID = UIDS[i];
            if (MID != 0xFFFF) {
                OneObject* OB = Group[MID];
                if (OB && !OB->Sdoxlo) {
                    Uids[realn] = MID;
                    //Usn[realn]=USN[i];
                    UIDS[realn] = MID;
                    Parms[realn] = Norma(OB->RealX - x, OB->RealY - y);
                    realn++;
                }
            }
        }
    }
    else {
        for (int i = 0; i < NUIDS; i++) {
            uint16_t MID = UIDS[i];
            if (MID != 0xFFFF) {
                OneObject* OB = Group[MID];
                if (OB && !OB->Sdoxlo) {
                    Uids[realn] = MID;
                    //Usn[realn]=USN[i];
                    UIDS[realn] = MID;
                    Parms[realn] = -Norma(OB->RealX - x, OB->RealY - y);
                    realn++;
                }
            }
        }
    }
    NUids = realn;
    return realn;
}

uint16_t SortClass::CreateByLine(uint16_t *  UIDS, int NUIDS, int dx, int dy)
{
    CheckSize(NUIDS);
    uint16_t realn = 0;
    for (int i = 0; i < NUIDS; i++) {
        uint16_t MID = UIDS[i];
        if (MID != 0xFFFF) {
            OneObject* OB = Group[MID];
            if (OB && !OB->Sdoxlo) {
                Uids[realn] = MID;
                //Usn[realn]=USN[i];
                UIDS[realn] = MID;
                Parms[realn] = (OB->RealX >> 5)*dx + (OB->RealY >> 5)*dy;
                realn++;
            }
        }
    }
    NUids = realn;
    return realn;
}

uint16_t SortClass::CreateByLineForShips(uint16_t *  UIDS, int NUIDS, int dx, int dy)
{
    CheckSize(NUIDS);
    uint16_t realn = 0;
    for (int i = 0; i < NUIDS; i++) {
        uint16_t MID = UIDS[i];
        if (MID != 0xFFFF) {
            OneObject* OB = Group[MID];
            if (OB && !OB->Sdoxlo) {
                Uids[realn] = MID;
                //Usn[realn]=USN[i];
                UIDS[realn] = MID;
                Parms[realn] = (OB->RealX >> 5)*dx + (OB->RealY >> 5)*dy;
                if (OB->newMons->Usage == GaleraID)Parms[realn] -= 100000;
                realn++;
            }
        }
    }
    NUids = realn;
    return realn;
}

void SortClass::CopyToDst(uint16_t *  Dst, uint16_t *  Sns)
{
    memcpy(Dst, Uids, NUids << 1);
    for (int i = 0; i < NUids; i++) {
        OneObject* OB = Group[Uids[i]];
        if (OB)Sns[i] = OB->Serial;
    }
}

void SortClass::Copy(uint16_t *  Dst)
{
    memcpy(Dst, Uids, NUids << 1);
}

//-----------------------Usage of the SortClass----------------------
SortClass UNISORT;

uint16_t SortUnitsByR(uint16_t *  urefs, uint16_t *  snrefs, int nu, int x, int y)
{
    int n = UNISORT.CreateByR(urefs, nu, true, x, y);
    UNISORT.Sort();
    UNISORT.CopyToDst(urefs, snrefs);
    return n;
}
