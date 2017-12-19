#ifndef DATAXCHANGE_H
#define DATAXCHANGE_H

#include "IntExplorer.h"

extern bool InReparse;
extern bool NOXCFILE;

void InitDevs();
void EraseTempFiles();

void ProcessDataXchange();

void StartDownloadInternetFile( char* Name, char* Server, char* DestName );
void ProcessDownloadInternetFiles();
void SendRecBuffer( byte* Data, int size, bool Final );
DWORD SendGlobalRequest( sicExplorer* SXP, const char* data, bool allow );

#endif // DATAXCHANGE_H
