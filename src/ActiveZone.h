#ifndef ACTIVEZONE_H
#define ACTIVEZONE_H

#include <cstdint>

#pragma pack( push, 1 )

class ActiveZone{
public:
	int x,y,R;
	char* Name;
    uint8_t Dir;
//--------------------//
	void Draw();
};

class ActiveGroup{
public:
    uint16_t * Units;
    uint16_t * Serials;
	int N;
	char* Name;
	int MinX,MinY,MaxX,MaxY;
	int AminX,AminY,AmaxX,AmaxY;
	bool Visible;
    void Draw(uint8_t c);
};

#pragma pack(pop)

extern ActiveGroup* AGroups;
extern int NAGroups;
extern int MaxAGroups;

extern ActiveZone* AZones;
extern int NAZones;
extern int MaxAZones;
extern int ZoneCmd;

void DrawAZones();
void ClearAllZones();
bool DelCurrentAZone();
void ControlAZones( int x, int y );
void DrawMinAZones( int x, int y );
void DrawMinAZonesVis( int x, int y );
void RefreshAGroups();

#endif // ACTIVEZONE_H
