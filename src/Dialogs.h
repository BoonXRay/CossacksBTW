#ifndef DIALOGS_H
#define DIALOGS_H

#include <cstdint>
#include <string>

#include "Fonts.h"

#ifndef DIALOGS_USER
#define DIALOGS_API __declspec(dllexport)
#else
#define DIALOGS_API __declspec(dllimport)
#endif

constexpr int MAXDLG = 1024;

extern bool LB_SORT_ITEM;
extern char LASTCLICKCHATNAME[128];
extern bool SafeLoad;

void SetCurPtr( int v );

DIALOGS_API int GetSound( const char* Name);

#pragma pack( push, 1 )

class DIALOGS_API SQPicture
{
public:
        unsigned short * PicPtr;
	SQPicture();
        SQPicture(const char* Name);
        void LoadPicture(const char* Name);
	void Draw(int x, int y);
	void DrawTransparent(int x, int y);
	~SQPicture();
	int GetLx()
	{
		if (!PicPtr)
		{
			return 0;
		}
		return PicPtr[0];
    }

	int GetLy()
	{
		if (!PicPtr)
		{
			return 0;
		}
		return PicPtr[1];
    }
};

class DIALOGS_API SimpleDialog;
typedef bool VCall(SimpleDialog* SD);
typedef bool VCallXY(SimpleDialog* SD, int x, int y, int ActiveID);
class DialogsSystem;
class DIALOGS_API SimpleDialog;
class DIALOGS_API VScrollBar;

class DIALOGS_API SimpleDialog 
{
public:
	bool Enabled : 1;
	bool Active : 1;
	bool PrevMouseOver : 1;
	bool MouseOver : 1;
	bool NeedToDraw : 1;
	bool MouseOverActive : 1;
	bool IsActive : 1;//drop-down panel is active
	bool Visible : 1;
        bool AllocHint : 1;
        bool NeedRedrawAll;
	short MouseSound;
	short ClickSound;
	int x, y, x1, y1;
	int UserParam;
        char* Hint;
	char* AllocPtr;
	SimpleDialog* Parent;
	SimpleDialog* Child;
	VCall* OnClick;
	VCall* OnDraw;
	VCall* OnActivate;
	VCall* OnLeave;
	VCall* OnKeyDown;
	VCall* OnMouseOver;
	VCall* Refresh;
	VCall* Destroy;
	VCall* OnUserClick;
	VCallXY* MouseOverActiveZone;
	VCall* OnDrawActive;
	VCallXY* OnNewClick;
	VScrollBar* ParentSB;
	DialogsSystem* ParentDS;
	SimpleDialog();
	void AssignSound(int ID, int USAGE);
	void AssignSound(char* Name, int USAGE);
};

#define CLICK_SOUND  0x1234
#define MOUSE_SOUND  0x4321
#define OK_SOUND     0x5421
#define CANCEL_SOUND 0x7513

class DIALOGS_API Picture :public SimpleDialog {
public:
	SQPicture* PassivePicture;
	SQPicture* ActivePicture;
	SQPicture* DisabledPicture;
	bool Transparent : 1;
        // BoonXRay 16.08.2017
        //Picture() { SimpleDialog; };
        Picture() : SimpleDialog() {}
};

class DIALOGS_API RLCPicture :public SimpleDialog {
public:
	RLCTable* PassivePicture;
        uint8_t ppic;
	RLCTable* ActivePicture;
        uint8_t apic;
	RLCTable* DisabledPicture;
        uint8_t dpic;
};

class DIALOGS_API GPPicture :public SimpleDialog {
public:
	int dx, dy;
	int FileID;
	int SpriteID;
	int Nation;
};

class DIALOGS_API TextMessage {
	char* Message;
	RLCFont* Font;
        uint8_t Align;
};

class DIALOGS_API TextButton :public SimpleDialog {
public:
	char* Message;
	RLCFont* ActiveFont;
	RLCFont* PassiveFont;
	RLCFont* DisabledFont;
	int	xc;
	int yc;
	int ActiveDX;
	int ActiveDY;
        uint8_t Align;
};
class DIALOGS_API GP_TextButton :public TextButton {
public:
	char* Message;
	RLCFont* ActiveFont;
	RLCFont* PassiveFont;
	int FontDy;
	int FontDx;
	bool Center;
	int GP_File;
	int Sprite;
	int Sprite1;
	int Nx;
	int OneLx;
};
class DIALOGS_API UniversalButton :public TextButton {
public:
	char* Message;
	RLCFont* ActiveFont;
	RLCFont* PassiveFont;
	RLCFont* SelectedFont;
	int  FontDy;
	int  FontDx;
	bool Center;
	int  GP_File;
	int  SpritesSet[30];
	int  Group;
	int  State;
	bool Tiling;
};

class DIALOGS_API VideoButton :public SimpleDialog {
public:
	int GP_on;
	int GP_off;
	int N_on;
	int N_off;
	int CurSprite;
	int Stage;
	int LastTime;
};
class DIALOGS_API VScrollBar :public SimpleDialog {
public:
	int SMaxPos, SPos;
	int sbx, sby, sblx, sbly;
	int LastTime;
	int mkly, btnly;
	bool Zaxvat;
	SQPicture* btn_up0;
	SQPicture* btn_up1;
	SQPicture* btn_up1p;
	SQPicture* btn_dn0;
	SQPicture* btn_dn1;
	SQPicture* btn_dn1p;
	SQPicture* sbar0;
	SQPicture* sbar1;
	SQPicture* marker;
	int OnesDy;
    void SetMaxPos(int i) { SMaxPos = i; }
    int GetMaxPos() { return SMaxPos; }
	int GetPos() { return SPos; }
	//----------GP_ScrollBar------------//
	int GP_File;
	int ScrollIndex;
	int LineIndex;
	int ScrDx, ScrDy;
	int ScrLx, ScrLy;
	int LineLx;
	int LineLy;
	//----------
	int StartGP_Spr;
};
class DIALOGS_API BpxTextButton :public TextButton {
public:
	SQPicture* PassivePicture;
	SQPicture* ActivePicture;
	SQPicture* DisabledPicture;
};

typedef void procDrawBoxElement(int x, int y, int Lx, int Ly, int Index, uint8_t Active, int param);
#define MaxColumn 16
class DIALOGS_API ComplexBox :public SimpleDialog {
public:
	procDrawBoxElement* DRAWPROC;
	int N;
	int CurrentItem;
	int TopStart;
	int NOnScr;
	int OneLy;
	int GP_Index;
	int StSprite;
	int M_OvrItem;
	int FirstVisItem;
	int param;
	VScrollBar* VS;
};
class DIALOGS_API Canvas :public SimpleDialog {
public:
	int BottomY;
	int L;
	int MaxL;
	VScrollBar* VS;
        uint8_t* DrawData;
        void AddLine(int x, int y, int x1, int y1, uint8_t c);
        void AddBar(int x, int y, int Lx, int Ly, uint8_t c);
        void AddRect(int x, int y, int Lx, int Ly, uint8_t c);
    void CheckSize(int sz);
};
class DIALOGS_API CustomBox :public SimpleDialog {
public:
	procDrawBoxElement* DRAWPROC;
	int param;
};
struct ListBoxItem {
	char* Message;
	int Param1;
        uint8_t Flags;
	ListBoxItem* NextItem;
};
class DIALOGS_API ListBox :public SimpleDialog {
public:
	ListBoxItem* FirstItem;
	ListBoxItem* LastItem;
	SQPicture* ItemPic;
	RLCFont* AFont;
	RLCFont* PFont;
	RLCFont* DFont;
        uint8_t ny;
        uint8_t oneLy;
	int	 oneLx;
	int NItems;
	int CurItem;
	int FLItem;
	int GP_File;
	int Sprite;
	int FontDy;
	int FontDx;
	int M_Over;
	int CurMouseOver;
	VScrollBar* VS;
	ListBoxItem* GetItem(int i);
        void AddItem(const char *str, int info);
        void AddStaticItem(const char *str, int info);
	void ClearItems();
    void SetCurrentItem(int i);
};
class DIALOGS_API RLCListBox :public SimpleDialog {
public:
	//RLCTable Items;
	int GPIndex;
        uint8_t* Choosed;
	int NItems;
        uint8_t BackColor;
        uint8_t SelColor;
	int  XPos;
	int  MaxXpos;
	bool Done;
};
class DIALOGS_API InputBox :public SimpleDialog {
public:
        // BoonXRay 20.08.2017
        //char* Str;
        std::string String;

	size_t CursPos;
	int totdx;
	int StrMaxLen;
	RLCFont* Font;
	RLCFont* AFont;
	SQPicture* Pict;
	bool Centering;
	bool Anonim;
};
class DIALOGS_API DialogsSystem;
class DIALOGS_API CheckBox :public SimpleDialog {
public:
	DialogsSystem* DS;
	SQPicture* OnPic;
	SQPicture* OffPic;
	char* Message;
	unsigned int State : 2;
	bool Transparent : 1;
	bool Central : 1;
	int GroupIndex;
	RLCFont* Font;
	RLCFont* AFont;
	//GP
	short GP_File;
	short Sprite0;
	short Sprite1;
	short Sprite2;
	short Sprite3;
};

class DIALOGS_API ColoredBar :public SimpleDialog
{
public:
        uint8_t color;
        uint8_t Style;
};

class DIALOGS_API ChatViewer :public SimpleDialog {
public:
	char*** Mess;
	char*** Names;
	int* NChats;
	int ChatDY;
	int MaxLx;
	int OneLy;
	int ScrNy;
};
struct LineInfo {
	bool NeedFormat;
        uint16_t LineSize;
        uint16_t NSpaces;
	int  Offset;
	int  NextOffset;
        uint16_t LineWidth;
};
class DIALOGS_API TextViewer :public SimpleDialog {
public:
	char* TextPtr;
	int TextSize;
	int NLines;
	int Line;
	int PageSize;
	int Lx;
        uint16_t SymSize;
	RLCFont* Font;
	void GetNextLine(LineInfo*);
	void CreateLinesList();
	char** LinePtrs;
        uint16_t*  LineSize;
        uint16_t*  LineWidth;
	bool*  NeedFormat;
        uint16_t*  NSpaces;
	VScrollBar* VS;
	void AssignScroll(VScrollBar* SB);
        void LoadFile(const char *Name);
};
class DIALOGS_API BPXView :public SimpleDialog {
public:
        uint8_t* Ptr;
	int OneLx;
	int OneLy;
	int Nx, Ny;
	int RealNy;
	int PosY;
	int ChoosedPos;
	int DECLX;
        uint8_t* Choosed;
	bool Done;
	bool EnableSelection;
	VScrollBar* VSC;
};
class DIALOGS_API WinComboBox;
class DIALOGS_API WinComboBox :public SimpleDialog {
public:
	RLCFont* ActiveFont;
	RLCFont* PassiveFont;
	int ListX0;
	int ListY0;
	int ListX1;
	int ListY1;
	char** Lines;
	int CurLine;
	int ActiveLine;
	int NLines;
	char* Message;
	WinComboBox** Group;
	int LastChooseTime;
	int NBox;
	int CurBox;
	void AddLine(char* Text);
	void Clear();
};

class DIALOGS_API ComboBox :public SimpleDialog 
{
public:
	RLCFont* ActiveFont;
	RLCFont* PassiveFont;
	int HdrLy;
	int FontDy;
	int FontDx;
	int OneLy;
	int OneLx;
	int OneDx;
	int OneDy;
	int HiLy;
	int LoLy;
	int NLines;
	int DropX;
	int DropY;
	int LightIndex;
	int GP_File;
	int UpPart;
	int Center;
	int DownPart;
	int UpLy;
        uint8_t BackColor;
        uint8_t BorderColor;
	char** Lines;
	int CurLine;
	//--------new--------
	VScrollBar* VS;
	int  MaxLY;
	int  YPos;
	int  DLX;
	//--------ruler(new!!)------
	bool rulermode;
	int MinDeal;

	void AssignScroll(DialogsSystem* DSS, VScrollBar** SCR, int GPFile, int Sprite, int MaxLy);
        void AddLine(const char* Text);
	void AddComplexLine(char* Text);
	void CreateRuler(int MinDeal, int NDeals);
	void Clear();
};

class DIALOGS_API GP_Button :public SimpleDialog {
public:
	int GP_File;
	int ActiveFrame;
	int PassiveFrame;
};
struct OnePage {
	int x, y, x1, y1;
	int Index;
};
class DIALOGS_API GP_PageControl :public SimpleDialog {
public:
	int GP_File;
	int CurPage;
	OnePage* Pages;
	int NPages;
	void AddPage(int x0, int y0, int x1, int y1, int Index);
};
class DIALOGS_API BorderEx :public SimpleDialog {
public:
	int ymid;
        uint8_t Style;
};
class DIALOGS_API CustomBorder :public SimpleDialog {
public:
	int GP;
	int BOR_N[8];
	int BOR_A[8];
	int FILL_N;
	int FILL_A;
};

class DIALOGS_API DialogsSystem 
{
public:
	short OkSound;
	short CancelSound;
	short UserClickSound;
	int HintX, HintY;
        char* Hint;
	char DefaultHint[1024];
	int	BaseX, BaseY;

	//Pointer array to all dialogs
	SimpleDialog* DSS[MAXDLG];

	int NDial;
	DialogsSystem(int x, int y);
	DialogsSystem();
	~DialogsSystem();
	RLCFont* Active;
	RLCFont* Passive;
	RLCFont* Disabled;
	RLCFont* Message;
	RLCFont* HintFont;
	int ActiveX, ActiveY, ActiveX1, ActiveY1, ActiveID;
	SimpleDialog* ActiveParent;
	void ProcessDialogs();
	void MarkToDraw();
	void RefreshView();
	void CloseDialogs();
	Picture* addPicture(SimpleDialog* Parent, int x, int y,
		SQPicture* Active,
		SQPicture* Passive,
		SQPicture* Disabled);
	GPPicture* addGPPicture(SimpleDialog* Parent,
		int dx, int dy, int FileID, int SpriteID);
	RLCPicture* addRLCPicture(SimpleDialog* Parent, int x, int y,
                RLCTable* Active, uint8_t apic,
                RLCTable* Passive, uint8_t ppic,
                RLCTable* Disabled, uint8_t dpic);
        TextMessage* addTextMessage(SimpleDialog* Parent, int x, int y, char* str, RLCFont* Font, uint8_t Align);
	TextMessage* addsTextMessage(SimpleDialog* Parent, int x, int y, char* str);
        TextButton* addTextButton(SimpleDialog* Parent, int x, int y, const char* str,
		RLCFont* Active,
		RLCFont* Passive,
		RLCFont* Disabled,
                uint8_t Align);//==0-left, ==1-center,  ==2-right
	TextButton* addsTextButton(SimpleDialog* Parent, int x, int y, char* str);
        GP_TextButton* addGP_TextButton(SimpleDialog* Parent, int x, int y, const char* str,
		int GP_File, int Sprite, RLCFont* Active, RLCFont* Passive);
        GP_TextButton* addGP_TextButtonLimited(SimpleDialog* Parent, int x, int y, const char *str,
                int GP_File, int SpriteActive, int SpritePassive, int Lx, RLCFont* Active, RLCFont* Passive);
	GP_Button* addGP_Button(SimpleDialog* Parent, int x, int y, int GP_File, int Active, int Passsive);

	VScrollBar* addVScrollBar(SimpleDialog* Parent, int x, int y, int MaxPos, int Pos,
		SQPicture* btn_up0,
		SQPicture* btn_up1,
		SQPicture* btn_up1p,
		SQPicture* btn_dn0,
		SQPicture* btn_dn1,
		SQPicture* btn_dn1p,
		SQPicture* sbar0,
		SQPicture* sbar1,
		SQPicture* marker);
	VScrollBar* addHScrollBar(SimpleDialog* Parent, int x, int y, int MaxPos, int Pos,
		SQPicture* btn_up0,
		SQPicture* btn_up1,
		SQPicture* btn_up1p,
		SQPicture* btn_dn0,
		SQPicture* btn_dn1,
		SQPicture* btn_dn1p,
		SQPicture* sbar0,
		SQPicture* sbar1,
		SQPicture* marker);
	ListBox* addListBox(SimpleDialog* Parent, int x, int y, int Ny,
		SQPicture* ItemPic,
		RLCFont* AFont,
		RLCFont* PFont,
		VScrollBar* VS);
	ListBox* addListBox(SimpleDialog* Parent,
		int x, int y, int Ny, int Lx, int Ly,
		RLCFont* AFont,
		RLCFont* PFont,
		VScrollBar* VS);
	ListBox* addGP_ListBox(SimpleDialog* Parent, int x, int y, int Ny,
		int GP_File, int Sprite, int Ly,
		RLCFont* AFont,
		RLCFont* PFont,
		VScrollBar* VS);
	ComplexBox* addComplexBox(int x, int y, int Ny, int OneLy,
		procDrawBoxElement* PDRAW, int GP_File, int Spr);
	CustomBox* addCustomBox(int x, int y, int Lx, int Ly, procDrawBoxElement* PDRAW);
	InputBox* addInputBox(SimpleDialog* Parent, int x, int y, char* str, int Len, SQPicture* Panel, RLCFont* RFont, RLCFont* AFont);
	InputBox* addInputBox(SimpleDialog* Parent, int x, int y, char* str, int Len, int Lx, int Ly, RLCFont* RFont, RLCFont* AFont, bool Centering);
        InputBox* addInputBox(SimpleDialog* Parent,
        int x, int y, const char *str,
        int Len,
        int Lx, int Ly,
        RLCFont* RFont,
        RLCFont* AFont);
        CheckBox* addGP_CheckBox(SimpleDialog* Parent,
                int x, int y, const char *message, RLCFont* a_font, RLCFont* p_font,
                int group, bool State,
                int GP, int active, int passive, int mouseover);
	SimpleDialog* addViewPort(int x, int y, int Nx, int Ny);
        ColoredBar* addViewPort2(int x, int y, int Nx, int Ny, uint8_t Color);
        ColoredBar* addColoredBar(int x, int y, int Nx, int Ny, uint8_t c);
        TextViewer* addTextViewer(SimpleDialog* Parent, int x, int y, int Lx, int Ly, const char *TextFile, RLCFont* TFont);
        BPXView* addBPXView(SimpleDialog* Parent, int x, int y, int OneLx, int OneLy, int Nx, int Ny, int RealNy, uint8_t* Ptr, VScrollBar* VSC);
        RLCListBox* addRLCListBox(SimpleDialog* Parent, int x, int y, int Lx, int Ly, int GPIndex, uint8_t BGColor, uint8_t SelColor);
	ComboBox* addComboBox(SimpleDialog* Parent, int x, int y, int Lx, int Ly, int LineLy,
                uint8_t BackColor, uint8_t BorderColor,
		RLCFont* ActiveFont, RLCFont* PassiveFont,
		char* Contence);
	ComboBox* addGP_ComboBox(SimpleDialog* Parent, int x, int y, int GP_File,
		int UpPart, int Center, int DownPart,
		RLCFont* ActiveFont, RLCFont* PassiveFont,
		char* Contence);
	ComboBox* addGP_ComboBoxDLX(SimpleDialog* Parent, int x, int y, int LX, int GP_File,
		int UpPart, int Center, int DownPart,
		RLCFont* ActiveFont, RLCFont* PassiveFont,
		char* Contence);
	GP_PageControl* addPageControl(SimpleDialog* Parent, int x, int y, int GF_File, int FirstIndex);
	VScrollBar* addGP_ScrollBarL(SimpleDialog* Parent, int x, int y,
		int MaxPos, int Pos, int GP_File,
		int ScrIndex, int LineLx, int LineLy, int ScrDx, int ScrDy);
	VScrollBar* addNewGP_VScrollBar(SimpleDialog* Parent, int x, int y, int Ly,
		int MaxPos, int Pos, int GP_File, int Sprite);
	VideoButton* addVideoButton(SimpleDialog* Parent, int x, int y, int GP1, int GP2);
	CustomBorder* addCustomBorder(int x, int y, int x1, int y1, int gp, int* bn, int* ba, int fill_n, int fill_a);
	GP_TextButton* addStdGP_TextButton(int x, int y, int Lx, char* str,
		int GP_File, int Sprite, RLCFont* Active, RLCFont* Passive);
	ChatViewer* addChatViewer(SimpleDialog* Parent, int x, int y, int Ny, int OneLy, int OneLx
		, char*** Mess, char*** Name, int* Nchats);
	SimpleDialog* addClipper(int x0, int y0, int x1, int y1);
	Canvas* AddCanvas(int x, int y, int Lx, int Ly);
        BorderEx* addBorder(int x, int y, int x1, int y1, int Ymid, uint8_t Style);
	//--------------------New style elements---------------//
        UniversalButton* addUniversalButton(int x, int y, int Lx, const char *str, int GP_File,
                int* SprSet, int Group, int NowSelected, bool tiling,
                RLCFont* Active, RLCFont* Passive, RLCFont* Selected);
};

#pragma pack(pop)


void CopyToScreen( int zx, int zy, int zLx, int zLy );

//Current dialog IDs?
constexpr uint8_t mcmExit       =   0xFF;
constexpr uint8_t mcmSingle     =   0xF1;
constexpr uint8_t mcmMulti      =   0xF2;
constexpr uint8_t mcmVideo      =   0xF3;
constexpr uint8_t mcmResume     =   0xF4;
constexpr uint8_t mcmOk         =   0xF5;
constexpr uint8_t mcmCancel     =   0xF6;
constexpr uint8_t mcmLoad		=   0xF7;
constexpr uint8_t mcmSave		=   0xF8;
constexpr uint8_t mcmHost		=   0xF9;
constexpr uint8_t mcmJoin		=   0xFA;
constexpr uint8_t mcmRefresh    =   0xFB;
constexpr uint8_t mcmEdit		=   0xFC;
constexpr uint8_t mcmAll		=   0xFD;
constexpr uint8_t mcmGraph      =   0xE0;
constexpr uint8_t mcmSound      =   0xE1;
constexpr uint8_t mcmSpeed      =   0xE2;
constexpr uint8_t mcmSnWeapon   =   0xE3;
constexpr uint8_t mcmSnBirth	=   0xE4;
constexpr uint8_t mcmSnOrder	=   0xE5;
constexpr uint8_t mcmSnAttack   =   0xE6;
constexpr uint8_t mcmOptions	=   0xE7;
constexpr uint8_t mcmHelp		=   0xE8;

#endif // DIALOGS_H
