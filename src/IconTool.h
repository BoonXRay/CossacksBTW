#ifndef ICONTOOL_H
#define ICONTOOL_H

#include <cstdint>

typedef void HandlePro( int );

class OneIcon{
public:
    uint16_t FileID;
    uint16_t SpriteID;
	//Type:
	//0  -simple icon
	//1  -icon with colored rectangle
	//2  -icon with Health bar
	//4  -icon with stage bar
	//8  -icon with integer value
	//16 -icon with hint
	//32 -right button active
	//64 -left  button active
    //128-memory for hint was allocated
    uint8_t  Color;
	int   Health;
	int   MaxHealth;
	int   Stage;
	int   MaxStage;
	int   IntVal;
    uint8_t  Level;
	bool  Disabled:1;
	bool  Selected:1;
    uint16_t MoreSprite;
    uint16_t Type;
	bool Visible:1;
	bool NoPress:1;
	char* Hint;
	char* HintLo;
    HandlePro* LPro;
    HandlePro* RPro;
	HandlePro* MoveOver;
    int   LParam;
    int   RParam;
	int   MoveParam;
	int   IntParam;

    void InitIcon();
    void AssignIcon(uint16_t FileID, uint16_t SpriteID);
    void AssignHint(char* str);
    void CreateHint( const char * str );
	void CreateHintLo(char* str);
    void AssignColor( uint8_t c);
    void AssignHealth(int Health,int MaxHeath);
    void AssignStage(int Stage,int MaxStage);
    void AssignIntVal(int ival);
    void AssignLeft(HandlePro* Lpro,int param);
    void AssignRight(HandlePro* Hpro,int param);
	void AssignMoveOver(HandlePro* Hpro,int param);
    void AssignLevel( uint8_t Level);
	void AssignIntParam(int i);
	void AssignRedPulse();
	void SelectIcon();
	void Disable();
    void Draw(int x,int y);
};

class IconSet{
public:
    int Space;
    OneIcon* Icons;
    int NIcons;
    IconSet();
    ~IconSet();
    void ClearIconSet();
    OneIcon* AddIconFixed( uint16_t FileID, uint16_t SpriteID,int Index);
    OneIcon* AddIcon( uint16_t FileID, uint16_t SpriteID);
    OneIcon* AddIcon( uint16_t FileID, uint16_t SpriteID, const char* Hint);
    OneIcon* AddIcon( uint16_t FileID, uint16_t SpriteID, const char* Hint, uint8_t Color);
    OneIcon* AddIcon( uint16_t FileID, uint16_t SpriteID, uint8_t Color, const char *Hint);
	int GetMaxX(int Lx);
    void DrawIconSet(int x,int y,int Nx,int Ny,int NyStart);
};

extern int AddIconLx;
extern int AddIconLy;

#endif // ICONTOOL_H
