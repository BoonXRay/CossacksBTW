#ifndef STRATEGYRESEARCH_H
#define STRATEGYRESEARCH_H

#include <cstdint>

class CellInfo{
public:
    uint16_t NSword;
    uint16_t NStrel;
    uint16_t NCaval;
    uint16_t NPush;
    uint16_t NMort;
    uint16_t NTow;
    uint16_t Total;
    uint8_t x;
    uint8_t y;
    uint16_t ArmyID;
    uint8_t Changed;
};

class ArmyInfo{
public:
    uint16_t NSword;
    uint16_t NStrel;
    uint16_t NCaval;
    uint16_t NPush;
    uint16_t NMort;
    uint16_t NTow;
    //uint16_t NPeon;
    uint8_t MinX;
    uint8_t MinY;
    uint8_t MaxX;
    uint8_t MaxY;
    uint16_t N;
};

class GlobalArmyInfo{
public:
    uint16_t * ArmDistr;
	CellInfo* CIN;
	int NCIN;
	int MaxCIN;
	ArmyInfo* AINF;
	int NArmy;
	int MaxArmy;
	GlobalArmyInfo();
	~GlobalArmyInfo();
	void Clear();
    void ResearchArmyDistribution( uint8_t NI );
	void Show(int x,int y);
};

#endif // STRATEGYRESEARCH_H
