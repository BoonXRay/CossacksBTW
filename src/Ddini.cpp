/***********************************************************************
 *Direct Draw initialisation module
 *
 * This module creates the Direct Draw object with the primary surface
 * and a backbuffer and sets 800x600x8 display mode.
 *
 ***********************************************************************/

#include <cassert>
#include <cstdio>

#include "FastDraw.h"
#include "main.h"
#include "ResFile.h"
#include "VirtScreen.h"

#include "Ddini.h"

//Dimensions of possible screen resolutions
int ModeLX[32];
int ModeLY[32];

//Number of possible screen resolutions
int NModes = 0;

int RealLx;
int RealLy;
int SCRSizeX;
int SCRSizeY;
int RSCRSizeX;
int RSCRSizeY;
int COPYSizeX;

static LPDIRECTDRAW            lpDD = NULL;      // DirectDraw object
LPDIRECTDRAWSURFACE     lpDDSPrimary;   // DirectDraw primary surface
BOOL                    bActive;        // is application active (not minimized / has focus)?

BOOL                    DDError;        //=FALSE if Direct Draw works normally 
static DDSURFACEDESC           ddsd;
static PALETTEENTRY            GPal[256];
static LPDIRECTDRAWPALETTE     lpDDPal;

struct zzz
{
	BITMAPINFO bmp;
	PALETTEENTRY XPal[255];
};

uint8_t GetPaletteColor( int r, int g, int b )
{
	int dmax = 10000;
	int bestc = 0;
	for (int i = 0; i < 256; i++)
	{
		int d = abs( r - GPal[i].peRed ) + abs( g - GPal[i].peGreen ) + abs( b - GPal[i].peBlue );
		if (d < dmax)
		{
			dmax = d;
			bestc = i;
		}
	}
	return bestc;
}

zzz xxt;
//typedef byte barr[ScreenSizeX*ScreenSizeY];
void* offScreenPtr;
/*
 * Flipping Pages
 */

//Copies secundary screen buffer into primary buffer
//Call rate: menu ~545 Hz, ingame ~38 Hz
__declspec( dllexport ) void FlipPages( void )
{
	if (!bActive)
	{
		return;
	}

	if (window_mode)
	{
		HDC WH = GetDC( hwnd );

        for (int i = 0; i < 256; i++)
        {
			xxt.bmp.bmiColors[i].rgbRed = GPal[i].peRed;
			xxt.bmp.bmiColors[i].rgbBlue = GPal[i].peBlue;
			xxt.bmp.bmiColors[i].rgbGreen = GPal[i].peGreen;
        }

        xxt.bmp.bmiHeader.biSize = sizeof( BITMAPINFOHEADER);
		xxt.bmp.bmiHeader.biWidth = SCRSizeX;
		xxt.bmp.bmiHeader.biHeight = -SCRSizeY;
		xxt.bmp.bmiHeader.biPlanes = 1;
		xxt.bmp.bmiHeader.biBitCount = 8;
		xxt.bmp.bmiHeader.biCompression = BI_RGB;
		xxt.bmp.bmiHeader.biSizeImage = 0;

        /*int z =*/ StretchDIBits( WH,
			0, 0, //X|YDest
			COPYSizeX, RSCRSizeY, //nDestWidth|Height
			0, MaxSizeY - RSCRSizeY, //X|YSrc
			COPYSizeX, RSCRSizeY, //nSrcWidth|Height
			RealScreenPtr, //*lpBits
			&xxt.bmp, //*lpBitsInfo
			DIB_RGB_COLORS, SRCCOPY ); //iUsage, dwRop

		ReleaseDC( hwnd, WH );

		return;
	}


	//TODO: replace asm
	int ofs = 0;
	int	lx = COPYSizeX >> 2;
	int	ly = RealLy;
	int	addOf = SCRSizeX - ( lx << 2 );
	int RaddOf = RSCRSizeX - ( lx << 2 );
	// BoonXRay 06.08.2017
	//__asm 
	//{
	//	push	esi
	//	push	edi
	//	mov		esi, ScreenPtr
	//	mov		edi, RealScreenPtr
	//	add		esi, ofs
	//	add		edi, ofs
	//	cld
	//	mov		eax, ly
	//	xxx :
	//	mov		ecx, lx
	//		rep		movsd
	//		add		esi, addOf
	//		add		edi, RaddOf
	//		dec		eax
	//		jnz		xxx
	//}
	//__asm
	{
		//push	esi
		//push	edi
		//mov		esi, ScreenPtr
		unsigned int TmpESI = reinterpret_cast<unsigned int>(ScreenPtr);
		//mov		edi, RealScreenPtr
		unsigned int TmpEDI = reinterpret_cast<unsigned int>(RealScreenPtr);
		//add		esi, ofs
		TmpESI += ofs;
		//add		edi, ofs
		TmpEDI += ofs;
		//cld
		//mov		eax, ly
		unsigned int TmpEAX = ly, TmpECX = 0;
	xxx :
		//mov		ecx, lx
		TmpECX = lx;
		//rep		movsd
		for (unsigned int i = 0; i < TmpECX; i++, TmpESI += 4 /*sizeof(int)*/, TmpEDI += 4 /*sizeof(int)*/)
			*reinterpret_cast<int *>(TmpEDI) = *reinterpret_cast<int *>(TmpESI);
		//add		esi, addOf
		TmpESI += addOf;
		//add		edi, RaddOf
		TmpEDI += RaddOf;
		//dec		eax
		TmpEAX--;
		//jnz		xxx
		if (TmpEAX != 0) goto xxx;
	}

	/*
	//Refactored asm... doesn't work for 'Connecting to master server' message when in fullscreen?

	int	addOf = SCRSizeX - COPYSizeX;
	int RaddOf = RSCRSizeX - COPYSizeX;

	int src = (int) ScreenPtr;
	int dest = (int) RealScreenPtr;

	for (int i = 0; i < RealLy; i++)
	{
		memcpy( (void*) dest, (void*) src, COPYSizeX );
		src += addOf;
		dest += RaddOf;
	}
	*/
}

/*
 * Getting Screen Pointer
 *
 * You will get the pointer to the invisible area of the screen
 * i.e, if primary surface is visible, then you will obtain the
 * pointer to the backbuffer.
 * You must call UnlockSurface() to allow Windows draw on the screen
 */
int SCRSZY = 0;

void LockSurface( void )
{
	long dderr = 0;
	if (window_mode)
	{
		ScreenPtr = (void*) ( int( offScreenPtr ) + MaxSizeX * 32 );
		ddsd.lpSurface = ScreenPtr;
		RealScreenPtr = ScreenPtr;
		return;
	}

	if (DDError)
	{
		return;
	}

	if (( dderr = lpDDSPrimary->Lock( NULL, &ddsd,
		DDLOCK_SURFACEMEMORYPTR |
		DDLOCK_WAIT, NULL ) ) != DD_OK)
	{
		DDError = true;
	}

	RSCRSizeX = ddsd.lPitch;

	ScreenPtr = (void*) ( int( offScreenPtr ) + MaxSizeX * 32 );
	RealScreenPtr = ScreenPtr;
	RealScreenPtr = ddsd.lpSurface;
	SCRSZY = ddsd.dwHeight;
	ClearScreen();
}

/*
 *  Unlocking the surface
 *
 *  You must unlock the Video memory for Windows to work properly
 */
void UnlockSurface( void )
{
	if (window_mode)
	{
		return;
	}

	if (DDError)
	{
		return;
	}

	if (lpDDSPrimary->Unlock( NULL ) != DD_OK)
	{
		DDError = true;
	}
}

//Save results of DirectX display mode enumeration
HRESULT CALLBACK ModeCallback( LPDDSURFACEDESC pdds, LPVOID /*lParam*/ )
{
	if (1024 > pdds->dwWidth || 768 > pdds->dwHeight)
	{//Don't allow for resolutions less than 1024 x 768 ot bigger than 1920x[...]
		return S_FALSE;
	}

	if (1920 < pdds->dwWidth)
	{//Also disable all resolutions above ~1920 px wide for fairness reasons
		return S_FALSE;
	}

	if (32 == pdds->ddpfPixelFormat.dwRGBBitCount)
	{
		ModeLX[NModes] = pdds->dwWidth;
		ModeLY[NModes] = pdds->dwHeight;
		NModes++;
	}

	//return S_TRUE to stop enuming modes, S_FALSE to continue
	return S_FALSE;
}

//Init DirectDraw and find possible resolutions
bool EnumModesOnly()
{
	HRESULT ddrval = DirectDrawCreate_wrapper( NULL, &lpDD, NULL );
	if (ddrval == DD_OK)
	{
		lpDD->EnumDisplayModes( 0, NULL, NULL, ModeCallback );
		lpDD->Release();
		lpDD = NULL;

		return true;
	}
	else
	{
		MessageBox( hwnd, "Unable to initialise Direct Draw. Cossacks should not run.", "Loading error", MB_ICONSTOP );
		exit( 0 );
	}
}

bool CreateDDObjects( HWND hwnd )
{
	HRESULT ddrval;
	char buf[256];
	DDError = false;

	if (window_mode)
	{
		SVSC.SetSize( RealLx, RealLy );
		DDError = false;
		SCRSizeX = MaxSizeX;
		SCRSizeY = MaxSizeY;
		COPYSizeX = RealLx;
		RSCRSizeX = RealLx;
		RSCRSizeY = RealLy;
		ScrHeight = SCRSizeY;
		ScrWidth = SCRSizeX;

		InitRLCWindows();

		WindX = 0;
		WindY = 0;
		WindLx = RealLx;
		WindLy = RealLy;
		WindX1 = WindLx - 1;
		WindY1 = WindLy - 1;
		BytesPerPixel = 1;
		offScreenPtr = ( malloc( SCRSizeX*( SCRSizeY + 32 * 4 ) ) );

		const int screen_width = GetSystemMetrics( SM_CXSCREEN );
		const int screen_height = GetSystemMetrics( SM_CYSCREEN );

		const int ModeLX_candidates[] = { 1024, 1152, 1280, 1280, 1366, 1600, 1920 };
		const int ModeLY_candidates[] = { 768,  864,  720, 1024,  768,  900, 1080 };

		NModes = 0;
        for (int i = 0; i < 7; i++)
		{
			//Only show resolutions up to current screen resolution
			if (ModeLX_candidates[i] <= screen_width
				&& ModeLY_candidates[i] <= screen_height)
			{
				ModeLX[i] = ModeLX_candidates[i];
				ModeLY[i] = ModeLY_candidates[i];
				NModes++;
			}
		}

		return true;
	}

	SVSC.SetSize( RealLx, RealLy );
    offScreenPtr = ( malloc( MaxSizeX*( MaxSizeY + 32 * 4 ) ) );

	if (lpDD)
	{
		lpDDSPrimary->Release();
		goto SDMOD;
	}

	lpDD = NULL;

	ddrval = DirectDrawCreate_wrapper( nullptr, &lpDD, nullptr );

	if (ddrval == DD_OK)
	{
SDMOD:;
		ddrval = lpDD->SetCooperativeLevel( hwnd,
			DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
		if (ddrval == DD_OK)
		{
			ddrval = lpDD->SetDisplayMode( RealLx, RealLy, 8 );
			if (ddrval == DD_OK)
			{
				ddsd.dwSize = sizeof( ddsd );
				ddsd.dwFlags = DDSD_CAPS;
				ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
				ddrval = lpDD->CreateSurface( &ddsd, &lpDDSPrimary, nullptr );
				if (ddrval == DD_OK)
				{
					DDError = false;
					SCRSizeX = MaxSizeX;
					SCRSizeY = MaxSizeY;
					RSCRSizeX = RealLx;
                    COPYSizeX = RealLx;
					RSCRSizeY = RealLy;
					ScrHeight = SCRSizeY;
					ScrWidth = SCRSizeX;
					WindX = 0;
					WindY = 0;
					WindLx = RealLx;
					WindLy = RealLy;
					WindX1 = WindLx - 1;
					WindY1 = WindLy - 1;
					BytesPerPixel = 1;

					return true;
				}
			}
		}
	}
	wsprintf( buf, "Direct Draw Init Failed (%08lx)\n", ddrval );
	MessageBox( hwnd, buf, "ERROR", MB_OK );
	return false;
}

/*   Direct Draw palette loading*/
void LoadPalette( LPCSTR lpFileName )
{
	if (!lpDDPal)
	{
		return;
	}

	if (window_mode)
	{
		return;
	}

	if (DDError)
	{
		return;
	}

	ResFile pf = RReset( lpFileName );
	memset( &GPal, 0, 1024 );

	if (pf != INVALID_HANDLE_VALUE)
	{
		for (int i = 0; i < 256; i++)
		{
			RBlockRead( pf, &GPal[i], 3 );
		}
		RClose( pf );

		if (!strcmp( lpFileName, "agew_1.pal" ))
		{
            int C0 = 65;//128-DCL*4;
			for (int i = 0; i < 12; i++)
			{
				int gray = 0;
				if (i > 2)gray = ( i - 2 ) * 2;
				if (i > 7)gray += ( i - 7 ) * 8;
				if (i > 9)gray += ( i - 10 ) * 10;
				if (i > 10)gray += 50;
				gray = gray * 6 / 3;
				int rr = 0 * C0 / 150 + gray * 8 / 2;
				int gg = 80 * C0 / 150 + gray * 6 / 2;//80
				int bb = 132 * C0 / 150 + gray * 4 / 2;
				if (rr > 255)rr = 255;
				if (gg > 255)gg = 255;
				if (bb > 255)bb = 255;
				if (i < 5)
				{
					rr = rr - ( ( rr*( 5 - i ) ) / 6 );
					gg = gg - ( ( rr*( 5 - i ) ) / 6 );
					bb = bb - ( ( rr*( 5 - i ) ) / 6 );
				}
				if (i < 3)
				{
					rr = rr - ( ( rr*( 3 - i ) ) / 4 );
					gg = gg - ( ( rr*( 3 - i ) ) / 4 );
					bb = bb - ( ( rr*( 3 - i ) ) / 4 );
				}
				if (i < 2)
				{
					rr = rr - ( ( rr*( 2 - i ) ) / 3 );
					gg = gg - ( ( rr*( 2 - i ) ) / 3 );
					bb = bb - ( ( rr*( 2 - i ) ) / 3 );
				}

				GPal[0xB0 + i].peBlue = bb;
				GPal[0xB0 + i].peRed = rr;
				GPal[0xB0 + i].peGreen = gg;
				C0 += 5;
			}
			ResFile pf = RRewrite( lpFileName );
			for (int i = 0; i < 256; i++)RBlockWrite( pf, &GPal[i], 3 );
			RClose( pf );
		}

		if (!window_mode)
		{
			if (!PalDone)
			{
				lpDD->CreatePalette( DDPCAPS_8BIT, &GPal[0], &lpDDPal, NULL );
				PalDone = true;
				lpDDSPrimary->SetPalette( lpDDPal );
			}
			else
			{
				lpDDPal->SetEntries( 0, 0, 256, GPal );
			}
		}
	}
}

void SetDarkPalette()
{
	if (DDError)
	{
		return;
	}

	memset( &GPal, 0, 1024 );

	if (!window_mode)
	{
		if (!PalDone)
		{
			lpDD->CreatePalette( DDPCAPS_8BIT, &GPal[0], &lpDDPal, nullptr );
			PalDone = true;
			lpDDSPrimary->SetPalette( lpDDPal );
		}
		else
		{
			lpDDPal->SetEntries( 0, 0, 256, GPal );
		}
	}
}

void SlowLoadPalette( LPCSTR lpFileName )
{
	PALETTEENTRY NPal[256];

	if (DDError)
	{
		return;
	}

	SetDarkPalette();
	ResFile pf = RReset( lpFileName );
	memset( &GPal, 0, 1024 );

	if (pf != INVALID_HANDLE_VALUE)
	{
		for (int i = 0; i < 256; i++)
		{
			RBlockRead( pf, &GPal[i], 3 );
		}

		RClose( pf );

		if (!strcmp( lpFileName, "agew_1.pal" ))
		{
            int C0 = 65;//128-DCL*4;
			for (int i = 0; i < 12; i++)
			{
				int gray = 0;
				if (i > 2)gray = ( i - 2 ) * 2;
				if (i > 7)gray += ( i - 7 ) * 8;
				if (i > 9)gray += ( i - 10 ) * 10;
				if (i > 10)gray += 50;
				gray = gray * 6 / 3;
				//gray=(i+5)*6;
				int rr = 0 * C0 / 150 + gray * 8 / 2;
				int gg = 80 * C0 / 150 + gray * 6 / 2;//80
				int bb = 132 * C0 / 150 + gray * 4 / 2;
				if (rr > 255)rr = 255;
				if (gg > 255)gg = 255;
				if (bb > 255)bb = 255;
				if (i < 5)
				{
					rr = rr - ( ( rr*( 5 - i ) ) / 6 );
					gg = gg - ( ( rr*( 5 - i ) ) / 6 );
					bb = bb - ( ( rr*( 5 - i ) ) / 6 );
				};
				if (i < 3)
				{
					rr = rr - ( ( rr*( 3 - i ) ) / 4 );
					gg = gg - ( ( rr*( 3 - i ) ) / 4 );
					bb = bb - ( ( rr*( 3 - i ) ) / 4 );
				};
				if (i < 2)
				{
					rr = rr - ( ( rr*( 2 - i ) ) / 3 );
					gg = gg - ( ( rr*( 2 - i ) ) / 3 );
					bb = bb - ( ( rr*( 2 - i ) ) / 3 );
				};
				//if(!i){
				//	rr=rr*10/11;
				//	gg=gg*10/11;
				//	bb=bb*10/11;
				//};
				GPal[0xB0 + i].peBlue = bb;
				GPal[0xB0 + i].peRed = rr;
				GPal[0xB0 + i].peGreen = gg;
				C0 += 5;
			}
			ResFile pf = RRewrite( lpFileName );

			for (int i = 0; i < 256; i++)
			{
				RBlockWrite( pf, &GPal[i], 3 );
			}

			RClose( pf );
		}

		if (!window_mode)
		{
			byte* pal = (byte*) NPal;
			byte* pal0 = (byte*) GPal;
			int mul = 0;
			int t0 = GetTickCount();
			int mul0 = 0;
			do
			{
				mul = ( GetTickCount() - t0 ) * 2;
				if (mul > 255)
				{
					mul = 255;
				}

				if (mul != mul0)
				{
					for (int j = 0; j < 1024; j++)
					{
						pal[j] = byte( ( int( pal0[j] )*mul ) >> 8 );
					}
					pal[1023] = 0;
					pal[1022] = 0;
					pal[1021] = 0;
					pal[1020] = 0;
					lpDDPal->SetEntries( 0, 0, 255, NPal );
				}
				mul0 = mul;
			} while (mul != 255);
		}
	}
}

void SlowUnLoadPalette( LPCSTR /*lpFileName*/ )
{
	PALETTEENTRY NPal[256];
	if (DDError)
	{
        return;
	}

	if (!window_mode)
	{
		byte* pal = (byte*) NPal;
		byte* pal0 = (byte*) GPal;
		int mul = 0;
		int t0 = GetTickCount();
		int mul0 = 0;
		do
		{
			mul = ( GetTickCount() - t0 ) * 2;
			if (mul > 255)
			{
				mul = 255;
			}

			if (mul != mul0)
			{
				for (int j = 0; j < 1024; j++)
				{
					pal[j] = byte( ( int( pal0[j] )*( 255 - mul ) ) >> 8 );
				}

				pal[1023] = 0;
				pal[1022] = 0;
				pal[1021] = 0;
				pal[1020] = 0;
				lpDDPal->SetEntries( 0, 0, 255, NPal );
			};
			mul0 = mul;
		} while (mul != 255);
	}
}

/*     Closing all Direct Draw objects
 *
 * This procedure must be called before the program terminates,
 * otherwise Windows can occur some problems.
 */
void FreeDDObjects( void )
{
	free( offScreenPtr );

	offScreenPtr = nullptr;

	if (window_mode)
	{
		return;
	}

	if (lpDD != nullptr)
	{
		//ClearScreen();
		if (lpDDSPrimary != nullptr)
		{
			lpDDSPrimary->Release();
			lpDDSPrimary = nullptr;
		};
		lpDD->Release();
		lpDD = nullptr;
	}
}

void GetPalColor( uint8_t idx, uint8_t * r, uint8_t * g, uint8_t * b )
{
	*r = GPal[idx].peRed;
	*g = GPal[idx].peGreen;
	*b = GPal[idx].peBlue;
}

int CurPalette = 0;

void ErrM( const char * s )
{
    if ( PalDone )
    {
        if ( CurPalette == 2 )
        {
            LoadPalette( "2\\agew_1.pal" );
        }
        else
        {
            LoadPalette( "0\\agew_1.pal" );
        }
    }
    MessageBox( hwnd, s, "LOADING FAILED...", MB_ICONWARNING | MB_OK );
}

void ErrM( const char* s, char* s1 )
{
    if ( CurPalette == 2 )LoadPalette( "2\\agew_1.pal" );
    else LoadPalette( "0\\agew_1.pal" );
    char cc2[200];
    sprintf( cc2, s, s1 );
    MessageBox( hwnd, cc2, "LOADING FAILED...", MB_ICONWARNING | MB_OK );
    assert( false );
}



/*
	DirectDraw substitute.
	Uses mdraw.dll instead of the original, ddraw.lib exported DirectDrawCreate().
	Prevents the color palette corruption bug in modern Windows systems.
	No idea what the mdraw.dll funtion does, but you end up with a working
	IDirectDraw interface and no legacy bugs.
*/
HRESULT DirectDrawCreate_wrapper( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter )
{
	HMODULE mdrawHandle = LoadLibrary( "mdraw.dll" );
	if (nullptr != mdrawHandle)
	{
		typedef HRESULT( __stdcall *mdrawProcType )( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
		mdrawProcType mdrawProc = (mdrawProcType) GetProcAddress( mdrawHandle, "DirectDrawCreate" );
		if (nullptr != mdrawProc)
		{
			HRESULT mdrawResult = mdrawProc( lpGUID, lplpDD, pUnkOuter );
			return mdrawResult;
		}
		FreeLibrary( mdrawHandle );
	}
	return DDERR_GENERIC;
}
