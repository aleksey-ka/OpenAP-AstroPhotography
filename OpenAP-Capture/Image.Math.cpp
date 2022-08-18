// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Math.h"

#include <climits>

CHistogram::CHistogram( size_t numberOfChannels, size_t bitsPerChannel ) :
    channelSize( maxValueForBitDepth( bitsPerChannel ) )
{
    channels.resize( numberOfChannels );
    for( size_t i = 0; i < numberOfChannels; i++ ) {
        channels[i].resize( channelSize );
    }
}

size_t pixels_histogram_p( const CHistogram& h, size_t channel, size_t target )
{
    auto& ch = h[channel];
    size_t channelSize = h.ChannelSize();
    size_t sum = 0;
    for( size_t i = 0; i < channelSize; i++ ) {
        size_t delta = target - sum;
        size_t v = ch[i];
        if( delta <= v ) {
            if( i > 0 && delta <= v / 2 ) {
                return i - 1;
            } else {
                return i;
            }
        }
        sum += v;
    }
    return INT_MAX;
}
