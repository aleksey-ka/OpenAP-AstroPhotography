// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <vector>
#include <limits>

const auto U8_MAX = std::numeric_limits<unsigned char>::max();
const auto U16_MAX = std::numeric_limits<unsigned short>::max();

class CDebayer_RawU16 {
public:
    CDebayer_RawU16( const unsigned short* _raw, int _width, int _height, int bitDepth ) :
        raw( _raw ), width( _width ), height( _height ), scaleTo8bits( bitDepth - 8 )
    {
    }

    unsigned short MaxValue = 0;
    unsigned MaxCount = 0;
    unsigned short MinValue = U16_MAX;
    unsigned MinCount = 0;

protected:
    const unsigned short* raw;
    const int width;
    const int height;
    const int scaleTo8bits;

    // Fast statistics (calculated for each pixel on each frame)
    unsigned short addToStatistics( unsigned short value );
};

inline unsigned short CDebayer_RawU16::addToStatistics( unsigned short value )
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
