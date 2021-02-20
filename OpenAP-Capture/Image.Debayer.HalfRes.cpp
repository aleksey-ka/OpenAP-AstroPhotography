// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Debayer.HalfRes.h"

void CDebayer_RawU16_HalfRes::ToRgbU8( unsigned char* rgb, int stride, int x0, int y0, int w, int h, unsigned int* hr, unsigned int* hg, unsigned int* hb )
{
    for( int y = 0; y < h; y++ ) {
        const auto* srcLine = raw + 2 * width * ( y0 / 2 + y );
        auto* dstLine = rgb + stride * y;
        for( int x = 0; x < w; x++ ) {
            const auto* src = srcLine + 2 * ( x0 / 2 + x );
            unsigned char r = addToStatistics( src[0] ) >> 4;
            unsigned char g1 = addToStatistics( src[1] ) >> 4;
            unsigned char g2 = addToStatistics( src[width] ) >> 4;
            unsigned char b = addToStatistics( src[width + 1] ) >> 4;

            unsigned char* dst = dstLine + 3 * x;
            dst[0] = r;
            dst[1] = g1;
            dst[2] = b;

            hr[r]++;
            hg[g1]++;
            hg[g2]++;
            hb[b]++;
        }
    }
}
