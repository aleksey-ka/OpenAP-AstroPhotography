// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Debayer.HQLinear.h"

void CDebayer_RawU16_HQLiner::ToRgbU16( unsigned short* rgb, int stride, int x0, int y0, int w, int h )
{
    for( int y = 0; y < h; y++ ) {
        int Y = y + y0;
        if( Y < 0 || Y >= height ) {
            continue;
        }
        const unsigned short* srcLine = raw +  width * Y;
        unsigned short* dstLine = rgb + stride * y;
        for( int x = 0; x < w; x++ ) {
            int X = x + x0;
            if( X < 0 || X >= width ) {
                continue;
            }

            const unsigned short* src = srcLine + X ;
            int R = 0;
            int G = 0;
            int B = 0;

            int channel = CFA_CHANNEL_AT( X, Y );
            if( Y < 2 || Y >= height - 2 || X < 2 || X >= width - 2 ) {
                switch( channel ) {
                    case 0: R = src[0]; break;
                    case 1: G = src[0]; break;
                    case 2: G = src[0]; break;
                    case 3: B = src[0]; break;
                }
            } else {
                switch( channel ) {
                    case 0: // R
                    {
                        R = src[0];

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

                        G = src[0];

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

                        G = src[0];

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

                        B = src[0];
                    }
                }
            }

            unsigned short* dst = dstLine + 3 * x;

            dst[0] = R > USHRT_MAX ? USHRT_MAX : R;
            dst[1] = G > USHRT_MAX ? USHRT_MAX : G;
            dst[2] = B > USHRT_MAX ? USHRT_MAX : B;
        }
    }
}

void CDebayer_RawU16_HQLiner::ToRgbU8( unsigned char* rgb, int stride, int x0, int y0, int w, int h, unsigned int* hr, unsigned int* hg, unsigned int* hb )
{
    for( int y = 0; y < h; y++ ) {
        int Y = y + y0;
        if( Y < 0 || Y >= height ) {
            continue;
        }
        const auto* srcLine = raw +  width * Y;
        auto* dstLine = rgb + stride * y;
        for( int x = 0; x < w; x++ ) {
            int X = x + x0;
            if( X < 0 || X >= width ) {
                continue;
            }

            const auto* src = srcLine + X ;
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

            auto* dst = dstLine + 3 * x;
            R >>= 4;
            G >>= 4;
            B >>= 4;

            dst[0] = R > UCHAR_MAX ? UCHAR_MAX : R;
            dst[1] = G > UCHAR_MAX ? UCHAR_MAX : G;
            dst[2] = B > UCHAR_MAX ? UCHAR_MAX : B;
        }
    }
}
