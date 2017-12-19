#ifndef SORT_H
#define SORT_H

#include <cstdint>

class SortClass
{
    public:
        uint16_t * Uids;
        //word* Usn;
        int * Parms;
        int NUids;
        int MaxUID;
        SortClass();
        ~SortClass();
        void Sort();
        void CheckSize( int Size );
        uint16_t CreateByX( uint16_t * UIDS, int NUIDS, bool Direction );
        uint16_t CreateByY( uint16_t * UIDS, int NUIDS, bool Direction );
        uint16_t CreateByR( uint16_t * UIDS, int NUIDS, bool Direction, int x, int y );
        uint16_t CreateByLine( uint16_t * UIDS, int NUIDS, int dx, int dy );
        uint16_t CreateByLineForShips( uint16_t * UIDS, int NUIDS, int dx, int dy );
        void CopyToDst( uint16_t * Dst, uint16_t * Sns );
        void Copy( uint16_t * Dst );
};

extern SortClass UNISORT;
uint16_t SortUnitsByR( uint16_t * urefs, uint16_t * usn, int nu, int x, int y );

#endif // SORT_H
