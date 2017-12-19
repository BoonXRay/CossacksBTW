#ifndef BMPTOOL_H
#define BMPTOOL_H

#include <cstdint>

typedef unsigned char byte;

#pragma pack( push, 1 )
struct BMPformat  // bmfh
{
    uint16_t    bfType;
    uint32_t    bfSize;
    uint16_t    bfReserved1;
    uint16_t    bfReserved2;
    uint32_t    bfOffBits;
    uint32_t    biSize;
    int32_t     biWidth;
    int32_t     biHeight;
    uint16_t    biPlanes;
    uint16_t    biBitCount;
    uint32_t    biCompression;
    uint32_t    biSizeImage;
    int32_t     biXPelsPerMeter;
    int32_t     biYPelsPerMeter;
    uint32_t    biClrUsed;
    uint32_t    biClrImportant;
};

struct PAL3
{
    uint8_t r, g, b;
};

struct PAL4
{
    uint8_t b, g, r, reserved;
};
#pragma pack(pop)

void SaveToBMP24( const char * Name, int Lx, int Ly, uint8_t * data );
bool ReadBMP24( const char * Name, BMPformat * BM, uint8_t ** data );
bool ReadBMP8( const char * Name, BMPformat * BM, uint8_t ** data );
bool ReadBMP8TOBPX( const char * Name, uint8_t ** data );
void SaveToBMP8( char * Name, int Lx, int Ly, uint8_t * data, PAL3 * pal );

bool LoadBitmapLikeGrayscale( char* Name, int* Lx, int* Ly, uint8_t ** res );
int GetResVal( uint8_t * res, int LX, int LY, int RLX, int RLY, int x, int y );

#endif // BMPTOOL_H
