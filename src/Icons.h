#ifndef ICONS_H
#define ICONS_H

#include <cstdint>

class WIcon{
public:
	char* Message;
    uint16_t FileID;
    uint16_t Spr;
    uint8_t Kind;
    uint16_t MagReq;
	//==0 - directory entry
	//==1 - uniq command, no parameters
	//==2 - uniq command, (x,y) requrired
	//==3 - upgrade
	//==4 - producind
    uint16_t Param1;
    uint16_t Param2;
    uint16_t * SubList;
};

#endif // ICONS_H
