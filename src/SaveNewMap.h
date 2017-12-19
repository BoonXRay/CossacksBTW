#ifndef SAVENEWMAP_H
#define SAVENEWMAP_H

#include <cstdint>

extern bool Loadingmap;

void Load3DMap( char* Map );
void Save3DMap( char* Map );
void CreateNewUnitAt( uint8_t NI, int x, int y, uint16_t Type, uint16_t Life, uint16_t Stage );
void DeleteAllUnits();

extern int LASTADDID;

void CreateNationalMaskForMap( const char* Name );

bool GetPreview( char* Name, uint8_t * Data );

void Load3DMapLandOnly( char* Map );
void ProcessMapAutosave();

#endif // SAVENEWMAP_H
