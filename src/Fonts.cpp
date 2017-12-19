#include <cstdlib>

#include "FastDraw.h"
#include "GP_Draw.h"

#include "Fonts.h"

RLCFont  BigBlackFont;
RLCFont  BigWhiteFont;
RLCFont  BigYellowFont;
RLCFont  BigRedFont;
RLCFont  BlackFont;
RLCFont  YellowFont;
RLCFont  WhiteFont;
RLCFont  RedFont;
RLCFont  SmallBlackFont;
RLCFont  SmallWhiteFont;
RLCFont  SmallYellowFont;
RLCFont  SmallRedFont;
RLCFont  SmallBlackFont1;
RLCFont  SmallWhiteFont1;
RLCFont  SmallYellowFont1;
RLCFont  SmallRedFont1;
RLCFont  SpecialWhiteFont;
RLCFont  SpecialYellowFont;
RLCFont  SpecialRedFont;
RLCFont  SpecialBlackFont;
RLCFont  fn10;
RLCFont  fn8;

RLCFont::RLCFont( int GP_Index )
{
    RLC = (RLCTable) GP_Index;
    FirstSymbol = 0;
    LastSymbol = 0;
    Options = 0;
    ShadowGP = -1;
}

RLCFont::RLCFont( const char* Name )
{
    int LOLD = LOADED;
    LoadRLC( Name, &RLC );
    RegisterRLCFont( this, RLC, 0 );
    LOADED = LOLD;
    Options = 0;
    ShadowGP = -1;
}

void RLCFont::SetGPIndex( int n )
{
    RLC = (RLCTable) n;
    FirstSymbol = 0;
    LastSymbol = 0;
    Options = 0;
}
RLCFont::RLCFont()
{
    RLC = nullptr;
    FirstSymbol = 0;
    LastSymbol = 0;
    Options = 0;
    ShadowGP = -1;
}
RLCFont::~RLCFont()
{
    if (RLC&&int( RLC ) > 4096)free( RLC );
    RLC = nullptr;
}

void RLCFont::SetRedColor()
{
    Options = 32;
    //SetStdShadow();
}

void RLCFont::SetWhiteColor()
{
    Options = 16;
    //SetStdShadow();
}

void RLCFont::SetBlackColor()
{
    Options = 0;
}

void RLCFont::SetColorTable( int n )
{
    Options = n << 4;
    //SetStdShadow();
}

void InitFonts()
{
	int GPID = GPS.PreLoadGPImage( "MainFont" );
	WhiteFont.SetGPIndex( GPID );
	WhiteFont.SetWhiteColor();
	YellowFont.SetGPIndex( GPID );
	YellowFont.SetColorTable( 3 );
	RedFont.SetGPIndex( GPID );
	RedFont.SetRedColor();
	BlackFont.SetGPIndex( GPID );
	GPID = GPS.PreLoadGPImage( "romBig" );
	BigWhiteFont.SetGPIndex( GPID );
	BigWhiteFont.SetWhiteColor();
	BigYellowFont.SetGPIndex( GPID );
	BigYellowFont.SetColorTable( 3 );
	BigRedFont.SetGPIndex( GPID );
	BigRedFont.SetRedColor();
	BigBlackFont.SetGPIndex( GPID );
	GPID = GPS.PreLoadGPImage( "rom10" );
	SmallWhiteFont.SetGPIndex( GPID );
	SmallWhiteFont.SetWhiteColor();
	SmallRedFont.SetGPIndex( GPID );
	SmallRedFont.SetRedColor();
	SmallBlackFont.SetGPIndex( GPID );
	SmallYellowFont.SetGPIndex( GPID );
	SmallYellowFont.SetColorTable( 3 );

	GPID = GPS.PreLoadGPImage( "serif" );
	SmallWhiteFont1.SetGPIndex( GPID );
	SmallWhiteFont1.SetWhiteColor();
	SmallRedFont1.SetGPIndex( GPID );
	SmallRedFont1.SetRedColor();
	SmallBlackFont1.SetGPIndex( GPID );
	SmallYellowFont1.SetGPIndex( GPID );
	SmallYellowFont1.SetColorTable( 3 );

	GPID = GPS.PreLoadGPImage( "Special" );
	SpecialWhiteFont.SetGPIndex( GPID );
	SpecialWhiteFont.SetWhiteColor();
	SpecialYellowFont.SetGPIndex( GPID );
	SpecialYellowFont.SetColorTable( 3 );
	SpecialRedFont.SetGPIndex( GPID );
	SpecialRedFont.SetRedColor();
	SpecialBlackFont.SetGPIndex( GPID );

	int s1 = GPS.PreLoadGPImage( "fn8" );
	int s2 = GPS.PreLoadGPImage( "fn10" );
	fn8.SetGPIndex( s1 );
	fn8.SetWhiteColor();
	fn10.SetGPIndex( s2 );
	fn10.SetWhiteColor();
}
