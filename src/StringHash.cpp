#include <cstdlib>
#include <cstdint>
#include <cstring>

#include "StringHash.h"

static inline uint8_t CalcHash( const char* s)
{
	// BoonXRay 14.08.2017
	//__asm 
	{
		//mov esi, s
		//cld
		//xor bl, bl
		//xor eax, eax
        const unsigned char * TmpESI = reinterpret_cast< const unsigned char *>( s );
		unsigned char TmpBL = 0, TmpAL = 0;
	strlp1 : 
		//lodsb
		//add bl, al
		//or al, al
		//jnz strlp1
		//mov al, bl
		TmpAL = *TmpESI;
		TmpESI++;
		TmpBL += TmpAL;
		if (TmpAL != 0) goto strlp1;
		return TmpBL;
    }

}

StrHash::StrHash()
{
    memset(this,0,sizeof( StrHash));
}

void StrHash::Clear()
{
	for(int i=0;i<256;i++){
		StrHashItem* lpSHI=SHI+i;
		for(int j=0;j<lpSHI->NStr;j++)free(lpSHI->Str[j]);
		if(lpSHI->Str){
			free(lpSHI->Str);
			free(lpSHI->Value);
        }
    }
    memset(this,0,sizeof( StrHash));
}

StrHash::~StrHash()
{
	Clear();
}

void StrHash::AddString( const char* s)
{
    uint8_t hv=CalcHash(s);
	StrHashItem* lpSHI=SHI+hv;
	int nv=lpSHI->NStr;
	if(lpSHI->NStr>=lpSHI->MaxStr){
		lpSHI->MaxStr+=16;
		lpSHI->Str=(char**)realloc(lpSHI->Str,lpSHI->MaxStr<<2);
		lpSHI->Value=(int*)realloc(lpSHI->Value,lpSHI->MaxStr<<2);
    }
	lpSHI->Str[nv]=(char*)malloc(strlen(s)+1);
	strcpy(lpSHI->Str[nv],s);
	lpSHI->Value[nv]=LastIndex;
	LastIndex++;
	lpSHI->NStr++;
}

int StrHash::SearchString( const char* s)
{
    uint8_t hv=CalcHash(s);
	StrHashItem* lpSHI=SHI+hv;
	int nv=lpSHI->NStr;
	for(int i=0;i<nv;i++)if(!strcmp(lpSHI->Str[i],s)){
		return lpSHI->Value[i];
    }
	return -1;
}
