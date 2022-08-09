// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Debayer.CFA.h"

void CDebayer_RawU16_CFA::ToRgbU8( std::uint8_t* rgb, int stride, int x0, int y0, int w, int h, unsigned int* hr, unsigned int* hg, unsigned int* hb )
{
    for( int y = 0; y < h; y++ ) {
        int Y = y0 + y;
        if( Y < 0 || Y >= height ) {
            continue;
        }
        const auto* srcLine = raw + width * Y;
        auto* dstLine = rgb + stride * y;
        for( int x = 0; x < w; x++ ) {
            int X = x0 + x;
            if( X < 0 || X >= width ) {
                continue;
            }
            const auto* src = srcLine + X;
            auto v = addToStatistics( src[0] ) >> scaleTo8bits;
            // Actual raw image data sometimes contain pixel values exceeding expected camera bitDepth
            v = v > UINT8_MAX ? UINT8_MAX : v;
            auto* dst = dstLine + 3 * x;
            switch( CFA_CHANNEL_AT( X, Y ) ) {
                case 0: dst[0] = v; hr[v]++; continue;
                case 1:
                case 2: dst[1] = v; hg[v]++; continue;
                case 3: dst[2] = v; hb[v]++; continue;
            }
        }
    }
}
