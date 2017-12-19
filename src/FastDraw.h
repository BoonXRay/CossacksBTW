/*               Fast raster graphics routine
 *
 *  This module presents several useful procedures for fast output,
 * using RLC graphics format (see the discription below). Before the
 * wor you must call SetRlcWindow to set properly the screen
 * capabilities.After this you mast call SetScreenPtr to set the
 * output distanation. Uou can use LoadRLC to load the set of RLC
 * pictures into the RLCTable structure. Then you can draw the
 * picture on the screen wit the procedure ShowRLCItem
 */

#ifndef FASTDRAW_H
#define FASTDRAW_H

#include <cstdint>

#include "Fonts.h"

#ifndef FASTDRAW_USER
// BoonXRay 19.08.2017
//#define FASTDRAW_API __declspec(dllexport)
#define FASTDRAW_API
#else
#define FASTDRAW_API __declspec(dllimport)
#endif

//Signbyte for recognizing unicode strings inside char*
//0x7F = DEL
#define SIGNBYTE ''//FUNNY: Yep, that's a DEL non-printable char right there

extern FASTDRAW_API int WindX;
extern FASTDRAW_API int WindY;
extern FASTDRAW_API int WindLx;
extern FASTDRAW_API int WindLy;
extern FASTDRAW_API int WindX1;
extern FASTDRAW_API int WindY1;
extern FASTDRAW_API int ScrWidth;
extern FASTDRAW_API int ScrHeight;
extern FASTDRAW_API int BytesPerPixel;
extern FASTDRAW_API void* ScreenPtr;
extern FASTDRAW_API void* RealScreenPtr;

struct RLCHeader
{
	short	SizeX;
	short	SizeY;
};

typedef RLCHeader* lpRLCHeader;

void InitRLCWindows();
int GetRLCWidth(RLCTable lpr, unsigned char n);

__declspec(dllexport) void SetRLCWindow(int x, int y, int lx, int ly, int slx);

__declspec(dllexport) int GetRLCHeight(RLCTable lpr, unsigned char n);

//Loading RLC file
FASTDRAW_API bool LoadRLC( const char * lpFileName, RLCTable *RLCtbl);

FASTDRAW_API void ShowRLCItem(int x, int y, RLCTable * lprt, int n, uint8_t nt);
void ShowRLCItemPal( int x, int y, RLCTable * lprt, int n, uint8_t * Pal );
void ShowRLCItemGrad( int x, int y, RLCTable * lprt, int n, uint8_t* Pal );
FASTDRAW_API void ClearScreen();
FASTDRAW_API void ShowString(int x, int y, const char * lps, lpRLCFont lpf);
FASTDRAW_API void RegisterRLCFont(lpRLCFont lrf, RLCTable lpr, int fir);
FASTDRAW_API void LoadPalettes();
FASTDRAW_API int GetRLCStrWidth(const char* str, lpRLCFont lpf);
FASTDRAW_API void CheckFontColor(lpRLCFont lpf);
FASTDRAW_API int GetRLCWidthUNICODE(RLCTable lpr, uint8_t * strptr, int* L);
FASTDRAW_API void ShowCharUNICODE( int x, int y, uint8_t * strptr, lpRLCFont lpr );

/*
 *                   Graphics routine
 *                 RLC-format interface
 *
 *RLC - description
 *
 *  unsigned int    Horisontal size of the picture
 *  unsigned short  Vertical size of the sub-picture
 *
 *    byte          Number of RLE substrings
 *
 *      byte        Length of the substring
 *
 *  byte[...]       Colors table
 */


/* Horisontal and vertical lines,rectangles*/
void Hline(int x,int y,int xend, uint8_t c);
void Vline(int x,int y,int yend, uint8_t c);
void Xbar(int x,int y,int lx,int ly, uint8_t c);
void DrawLine(int x,int y,int x1,int y1, uint8_t c);

#endif // FASTDRAW_H
