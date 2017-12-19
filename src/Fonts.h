#ifndef FONTS_H
#define FONTS_H

struct xRLCTable
{
    int Size;
    int sign;
    int	SCount;
    int OfsTable[32768000];
};

using RLCTable = xRLCTable *;

class RLCFont
{
public:
    int	FirstSymbol;
    int LastSymbol;
    unsigned char Options;
    int ShadowGP;
    int ShadowDx;
    int ShadowDy;

    RLCTable RLC;

    RLCFont();
    RLCFont(const char *Name);
    RLCFont(int GP_Index);

    void SetGPIndex(int n);
    void SetRedColor();
    void SetWhiteColor();
    void SetBlackColor();
    void SetColorTable(int n);

    void SetStdShadow()
    {
        ShadowGP = int(RLC);
        ShadowDx = 1;
        ShadowDy = 1;
    }

    void SetShadowGP(int n, int dx, int dy)
    {
        ShadowGP = n;
        ShadowDx = dx;
        ShadowDy = dy;
    }

    ~RLCFont();
};

typedef RLCFont* lpRLCFont;

extern RLCFont  BigBlackFont;
extern RLCFont  BigWhiteFont;
extern RLCFont  BigYellowFont;
extern RLCFont  BigRedFont;
extern RLCFont  BlackFont;
extern RLCFont  YellowFont;
extern RLCFont  WhiteFont;
extern RLCFont  RedFont;
extern RLCFont  SmallBlackFont;
extern RLCFont  SmallWhiteFont;
extern RLCFont  SmallYellowFont;
extern RLCFont  SmallRedFont;
extern RLCFont  SmallBlackFont1;
extern RLCFont  SmallWhiteFont1;
extern RLCFont  SmallYellowFont1;
extern RLCFont  SmallRedFont1;
extern RLCFont  SpecialWhiteFont;
extern RLCFont  SpecialYellowFont;
extern RLCFont  SpecialRedFont;
extern RLCFont  SpecialBlackFont;
extern RLCFont  fn10;
extern RLCFont  fn8;

void InitFonts();

#endif // FONTS_H
