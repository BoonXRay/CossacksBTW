#ifndef BOONUTILS_HPP
#define BOONUTILS_HPP

#include <cstdint>

// Convert Cossacks BIG char to uint16_t
constexpr inline uint16_t ConvCosChar( uint8_t a, uint8_t b ) noexcept
{
    return ( uint16_t( a ) << 8 ) | b;
}

// Convert Cossacks BIG char to uint32_t
constexpr inline uint32_t ConvCosChar( uint8_t a, uint8_t b, uint8_t c, uint8_t d ) noexcept
{
    return ( uint32_t( a ) << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
}

#endif // BOONUTILS_HPP
