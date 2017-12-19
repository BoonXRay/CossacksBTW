#ifndef MASKS_H
#define MASKS_H

#include <cstdint>

constexpr uint8_t MaskLx = 64;
extern uint8_t ResultMask[MaskLx*256];

struct xRLCTable;

// 1(x0,y0)
//  |\          '
//  |  \ s1
//s3|    \ 2
//  |    /
//  |  / s2
//  |/
// 3
//
// bm1,bm2,bm3-numbers of bitmaps in BitmapArray
// if bm1<bm2 then bm2 is over bm1
// s1 (0..2) - section of 1-2 
// s2 (0..2) - section of 2-3 
// s3 (0..2) - section of 3-1 
void PrepareIntersection1(int bm1,int bm2,int bm3,
						  int x0,int y0,
						  int s1,int s2,int s3,
                          xRLCTable * Masks, uint8_t * BitmapsArray);
//
//                1
//         s1   /|
//            /  |
//        2 /    | s3
// (x01,y0) \    |
//            \  |
//         s2   \|
//                3
void PrepareIntersection2(int bm1,int bm2,int bm3,
						  int x0,int y01,
						  int s1,int s2,int s3,
                          xRLCTable * Masks, uint8_t * BitmapsArray);

inline int GetBmOfst(int i)
{
    return ((i&3)<<6)+((i>>2)<<(8+6));
}

#endif // MASKS_H