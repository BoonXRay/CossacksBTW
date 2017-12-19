#ifndef UPGRADE_H
#define UPGRADE_H

#include <cstdint>

class OneObject;
class SimpleUTP{
public:
	char* Message;
    uint8_t NLinks;
    uint8_t Kind;
	int* Links;
    uint8_t NAuto;
	int* AutoPerf;
	bool Enabled:1;
	bool OneTime:1;
	bool Done:1;
	bool Finished:1;
    uint16_t IFileID;
    uint16_t IFIndex;
    uint16_t Cost;
    uint16_t Wood;
    uint16_t Oil;
    uint16_t MonsterID;
    uint16_t Time;
    uint16_t Stage;
};

class UTP1:public SimpleUTP{
public:
    uint8_t ResType;
    uint8_t AddVal;
};
class UTP2:public SimpleUTP{
public:
    uint16_t MID;
    uint8_t CharID;
    uint8_t AddVal;
};
class UTP3:public SimpleUTP{
public:
    uint16_t FinalMID;
};
class Upgrade{
public:
	union{
		UTP1* utp1[1024];
		UTP2* utp2[1024];
		UTP3* utp3[1024];
		SimpleUTP* utp[1024];
	};
	int NUpgrades;
	Upgrade();
};

void PerformUpgradeLink( OneObject* OB );

#endif // UPGRADE_H
