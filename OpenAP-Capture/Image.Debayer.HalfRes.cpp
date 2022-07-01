// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Debayer.HalfRes.h"

void CDebayer_RawU16_HalfRes::ToRgbU8( unsigned char* rgb, int stride, int x0, int y0, int w, int h, unsigned int* hr, unsigned int* hg, unsigned int* hb )
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
            unsigned char r = addToStatistics( src[width] ) >> scaleTo8bits;
            unsigned char g1 = addToStatistics( src[0] ) >> scaleTo8bits;
            unsigned char g2 = addToStatistics( src[width + 1] ) >> scaleTo8bits;
            unsigned char b = addToStatistics( src[1] ) >> scaleTo8bits;

            unsigned char* dst = dstLine + 3 * x;
            dst[0] = (255 * r) / 300;
            dst[1] = (200 * g1) / 300;
            dst[2] = b;

            hr[r]++;
            hg[g1]++;
            hg[g2]++;
            hb[b]++;
        }
    }
}
