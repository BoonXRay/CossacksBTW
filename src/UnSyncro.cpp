#include <cstdio>
#include <cstring>

#include "3DGraph.h"
#include "Weapon.h"

#include "UnSyncro.h"


void Syncro::Copy( Syncro* Syn )
{
    memcpy( Syn->RSL, RSL, NSlots * sizeof( RandSlot) );
	Syn->NSlots = NSlots;
	NSlots = 0;
}

Syncro SYN;
Syncro SYN1;
static const char* LastFile;

int RandNew( const char* File, int Line )
{
	LastFile = File;
    int r = randoma[rpos];
	rpos = ( rpos + 1 ) & 8191;
	if ( SYN.NSlots < maxUline )
	{
		RandSlot* RS = SYN.RSL + SYN.NSlots;
		RS->FileName = File;
		RS->Line = Line;
		RS->Param = 0;
		RS->rpos = rpos;
		RS->Type = -1;
		SYN.NSlots++;
	}

	return r;
}
