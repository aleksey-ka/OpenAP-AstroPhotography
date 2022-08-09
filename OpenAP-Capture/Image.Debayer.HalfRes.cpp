// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Debayer.HalfRes.h"

void CDebayer_RawU16_HalfRes::ToRgbU8( std::uint8_t* rgb, int stride, int x0, int y0, int w, int h, unsigned int* hr, unsigned int* hg, unsigned int* hb )
{
    for( int y = 0; y < h; y++ ) {
        int Y = 2 * y + y0;
        if( Y < 0 || Y >= height ) {
            continue;
        }
        const auto* srcLine = raw + width * Y;
        auto* dstLine = rgb + stride * y;
        for( int x = 0; x < w; x++ ) {
            int X = 2 * x + x0;
            if( X < 0 || X >= width ) {
                continue;
            }
            const auto* src = srcLine + X;
            auto r = addToStatistics( src[0] ) >> scaleTo8bits;
            auto g1 = addToStatistics( src[1] ) >> scaleTo8bits;
            auto g2 = addToStatistics( src[width] ) >> scaleTo8bits;
            auto b = addToStatistics( src[width + 1] ) >> scaleTo8bits;

            auto* dst = dstLine + 3 * x;
            // Actual raw image data sometimes contain pixel values exceeding expected camera bitDepth
            dst[0] = r > UINT8_MAX ? UINT8_MAX : r;
            dst[1] = g1 > UINT8_MAX ? UINT8_MAX : g1;
            dst[2] = b > UINT8_MAX ? UINT8_MAX : b;

            hr[r]++;
            hg[g1]++;
            hg[g2]++;
            hb[b]++;
        }
    }
}
