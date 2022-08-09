// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <cstdint>

class CDebayer_RawU16 {
public:
    CDebayer_RawU16( const std::uint16_t* _raw, int _width, int _height, int bitDepth ) :
        raw( _raw ), width( _width ), height( _height ), scaleTo8bits( bitDepth - 8 )
    {
    }

    std::uint16_t MaxValue = 0;
    unsigned int MaxCount = 0;
    std::uint16_t MinValue = UINT16_MAX;
    unsigned int MinCount = 0;

protected:
    const std::uint16_t* raw;
    const int width;
    const int height;
    const int scaleTo8bits;

    // Fast statistics (calculated for each pixel on each frame)
    std::uint16_t addToStatistics( std::uint16_t value );
};

inline std::uint16_t CDebayer_RawU16::addToStatistics( std::uint16_t value )
{
    if( value >= MaxValue ) {
        if( value == MaxValue ) {
            MaxCount++;
        } else {
            MaxValue = value;
            MaxCount = 1;
        }
    } else if( value <= MinValue ) {
        if( value == MinValue ) {
            MinCount++;
        } else {
            MinValue = value;
            MinCount = 1;
        }
    }
    return value;
}

#define CFA_CHANNEL_AT( x, y ) ( x % 2 | y % 2 << 1 )
