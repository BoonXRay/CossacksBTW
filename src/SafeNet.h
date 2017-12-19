#ifndef SAFENET_H
#define SAFENET_H

#include <cstdint>

uint16_t GetNextGreCell( uint8_t NI, int F, int Start );
uint16_t GetNextDivCell( uint8_t NI, int F, int Start );
uint16_t GetNextSafeCell( uint8_t NI, int F, int start, int Fin );

#endif // SAFENET_H
