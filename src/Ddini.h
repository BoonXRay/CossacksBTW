/***********************************************************************
 *Direct Draw initialisation module
 *
 * This module creates the Direct Draw object with the primary surface
 * and a backbuffer and sets 800x600x8 display mode.
 *
 ***********************************************************************/
#ifndef __DDINI_H_
#define __DDINI_H_

#include <cstdint>

#include <ddraw.h>
#include <windows.h>


//Dimensions of possible screen resolutions
extern __declspec( dllexport ) int ModeLX[32];
extern __declspec( dllexport ) int ModeLY[32];

//Number of possible screen resolutions
extern __declspec( dllexport ) int NModes;

extern int RealLx;
extern int RealLy;

extern __declspec( dllexport ) int SCRSizeX;
extern __declspec( dllexport ) int SCRSizeY;
extern __declspec( dllexport ) int RSCRSizeX;
extern __declspec( dllexport ) int RSCRSizeY;
extern __declspec( dllexport ) int COPYSizeX;

#define MAKE_PTC

extern LPDIRECTDRAWSURFACE     lpDDSPrimary;   // DirectDraw primary surface
extern BOOL                    bActive;        // is application active?

extern BOOL                    DDError;        //=FALSE if Direct Draw works normally 
extern bool window_mode;
extern int SCRSZY;

uint8_t GetPaletteColor( int r, int g, int b );

/*  Create Direct Draw object
 *
 * This procedure creadtes Direct Draw object with primary surface and
 * one backbuffer and sets 800x600x8 display mode.
 * This procedure enables usage of all other procedure in thes module.
 */
bool CreateDDObjects(HWND hwnd);
/*     Closing all Direct Draw objects
 *
 * This procedure must be called before the program terminates,
 * otherwise Windows can occur some problems.
 */
void FreeDDObjects(void);
/*
 * Flipping Pages
 */
__declspec(dllexport)
void FlipPages(void);
/*
 * Getting Screen Pointer
 *
 * You will get the pointer to the invisible area of the screen
 * i.e, if primary surface is visible, then you will obtain the
 * pointer to the backbuffer.
 * You must call UnlockSurface() to allow Windows draw on the screen
 */
void LockSurface(void);
/*
 *  Unlocking the surface
 *
 *  You must unlock the Video memory for Windows to work properly
 */
void UnlockSurface(void);

bool EnumModesOnly();

/*
 * Getting the DC of the active (invisible) area of the screen
 */
 //HDC GetSDC(void);//OBSOLETE
 //Sttting the palette
void LoadPalette(LPCSTR lpFileName);
void SetDarkPalette();

__declspec( dllexport ) void SlowLoadPalette( LPCSTR lpFileName );
__declspec( dllexport ) void SlowUnLoadPalette( LPCSTR /*lpFileName*/ );

__declspec( dllexport ) void GetPalColor( uint8_t idx, uint8_t * r, uint8_t * g, uint8_t * b );

void SetDebugMode();
void NoDebugMode();

extern __declspec( dllexport ) int CurPalette;
void ErrM( const char * s );
void ErrM( const char * s, char * s1 );

// Define own DirectDrawCreate helper function which loads mdraw.dll at runtime
HRESULT DirectDrawCreate_wrapper(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);

//#endif
#endif //__DDINI_H_
