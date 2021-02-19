// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Renderer.h"

void Renderer::renderHighQualityLinearWithHistogram( uchar* rgb, int byteWidth, int x0, int y0, int w, int h )
{
    uint* hr = histR.data();
    uint* hg = histG.data();
    uint* hb = histB.data();

    for( size_t y = 0; y < h; y++ ) {
        int Y = y + y0;
        if( Y < 0 || Y >= height ) {
            continue;
        }
        const ushort* srcLine = raw +  width * Y;
        uchar* dstLine = rgb + byteWidth * y;
        for( size_t x = 0; x < w; x++ ) {
            int X = x + x0;
            if( X < 0 || X >= width ) {
                continue;
            }

            const ushort* src = srcLine + X ;
            int R = 0;
            int G = 0;
            int B = 0;

            int channel = Y % 2 << 1 | X % 2;
            if( Y < 2 || Y >= height - 2 || X < 2 || X >= width - 2 ) {
                switch( channel ) {
                    case 0: R = addToStatistics( src[0] ); break;
                    case 1: G = addToStatistics( src[0] ); break;
                    case 2: G = addToStatistics( src[0] ); break;
                    case 3: B = addToStatistics( src[0] ); break;
                }
            } else {
                switch( channel ) {
                    case 0: // R
                    {
                        R = addToStatistics( src[0] );
                        hr[src[0] >> 4]++;

                        // (0) Green at red location
                        G -= src[-width-width];
                        G += 2 * src[-width];
                        G -= src[-2];
                        G += 2 * src[-1] + 4 * src[0] + 2 * src[1];
                        G -= src[2];
                        G += 2 * src[width];
                        G -= src[width+width];
                        if( G < 0 ) G = 0;
                        G >>= 3;

                        // (1) Blue at red location
                        int x = 0;
                        x += src[-width-width];
                        B += 2 * ( src[-width-1] + src[-width+1] );
                        x += src[-2];
                        B += 6 * src[0];
                        x += src[2];
                        B += 2 * ( src[width-1] + src[width+1] );
                        x += src[width+width];
                        x *= 3;
                        x >>= 1;
                        B -= x;
                        if( B < 0 ) B = 0;
                        B >>= 3;
                        break;
                    }
                    case 1: // G1
                    {
                        // (3) Red at G1 location
                        R += src[-width-width] >> 1;
                        R -= src[-width-1] + src[-width+1];
                        R -= src[-2];
                        R += 4 * src[-1] + 5 * src[0] + 4 * src[1];
                        R -= src[2];
                        R -= src[width-1] + src[width+1];
                        R += src[width+width] >> 1;
                        if( R < 0 ) R = 0;
                        R >>= 3;

                        G = addToStatistics( src[0] );
                        hg[src[0] >> 4]++;

                        // (4) Blue at G1 location
                        B -= src[-width-width];
                        B -= src[-width-1];
                        B += 4 * src[-width];
                        B -= src[-width+1];
                        B += ( src[-2] >> 1 ) + 5 * src[0] + ( src[2] >> 1 );
                        B -= src[width+1];
                        B += 4 * src[width];
                        B -= src[width-1];
                        B -= src[width+width];
                        if( B < 0 ) B = 0;
                        B >>= 3;
                        break;
                    }
                    case 2: // G2
                    {
                        // (4) Red at G2 location
                        R -= src[-width-width];
                        R -= src[-width-1];
                        R += 4 * src[-width];
                        R -= src[-width+1];
                        R += ( src[-2] >> 1 ) + 5 * src[0] + ( src[2] >> 1 );
                        R -= src[width+1];
                        R += 4 * src[width];
                        R -= src[width-1];
                        R -= src[width+width];
                        if( R < 0 ) R = 0;
                        R >>= 3;

                        G = addToStatistics( src[0] );
                        hg[src[0] >> 4]++;

                        // (3) Blue at G2 location
                        B += src[-width-width] >> 1;
                        B -= src[-width-1] + src[-width+1];
                        B -= src[-2];
                        B += 4 * src[-1] + 5 * src[0] + 4 * src[1];
                        B -= src[2];
                        B -= src[width-1] + src[width+1];
                        B += src[width+width] >> 1;
                        if( B < 0 ) B = 0;
                        B >>= 3;
                        break;
                    }
                    case 3: // B
                    {
                        // (1) Red at blue location
                        int x = 0;
                        x += src[-width-width];
                        R += 2 * ( src[-width-1] + src[-width+1] );
                        x += src[-2];
                        R += 6 * src[0];
                        x += src[2];
                        R += 2 * ( src[width-1] + src[width+1] );
                        x += src[width+width];
                        x *= 3;
                        x >>= 1;
                        R -= x;
                        if( R < 0 ) R = 0;
                        R >>= 3;

                        // (0) Green at blue location
                        G -= src[-width-width];
                        G += 2 * src[-width];
                        G -= src[-2];
                        G += 2 * src[-1] + 4 * src[0] + 2 * src[1];
                        G -= src[2];
                        G += 2 * src[width];
                        G -= src[width+width];
                        if( G < 0 ) G = 0;
                        G >>= 3;

                        B = addToStatistics( src[0] );
                        hb[src[0] >> 4]++;
                        break;
                    }
                }
            }

            uchar* dst = dstLine + 3 * x;
            R >>= 4;
            G >>= 4;
            B >>= 4;

            dst[0] = R > 255 ? 255 : R;
            dst[1] = G > 255 ? 255 : G;
            dst[2] = B > 255 ? 255 : B;
        }
    }
}
