// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Math.h"

#include <QDebug>

CPixelStatistics::CPixelStatistics( int numberOfChannels, int bitsPerChannel ) :
    channelSize( 2 << bitsPerChannel )
{
    channels.resize( numberOfChannels );
    for( int i = 0; i < numberOfChannels; i++ ) {
        channels[i].resize( channelSize );
    }
}

CChannelStat CPixelStatistics::stat( int channel, int count ) const
{
    CChannelStat stat;
    stat.Median = p( channel, count / 2 );
    stat.Sigma = stat.Median - p( channel, count / 2 - count / 3 );
    int dR2 = p( channel, count / 2 + count / 3 ) - stat.Median;
    int mR = maxP( channel, stat.Median - stat.Sigma, stat.Median + dR2 );
    qDebug() << "Hist" << channel << stat.Median  << stat.Sigma << "/" << dR2 << mR;

    return stat;
}

size_t CPixelStatistics::p( int channel, int target ) const
{
    auto& hist = channels[channel];
    uint sum = 0;
    for( int i = 0; i < channelSize; i++ ) {
        uint delta = target - sum;
        uint v = hist[i];
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

size_t CPixelStatistics::maxP( int channel, int start, int end ) const
{
    auto& hist = channels[channel];

    int max = INT_MIN;
    int maxPos = INT_MIN;
    for( int i = start; i <= end; i++ ) {
        int v = hist[i];
        if( v > max ) {
            max = v;
            maxPos = i;
        }
    }
    return maxPos;
}

QPixmap FocusingHelper( const Raw16Image* image, int x0, int y0, int W, int H )
{
    x0 = ( x0 + W / 2 ) - W;
    y0 = ( y0 + H / 2 ) - H;
    x0 += x0 % 2;
    y0 += y0 % 2;

    CPixelStatistics stats( 3, 12 );

    auto& histR = stats[0];
    auto& histG = stats[1];
    auto& histB = stats[2];

    const auto& size = histR.size();

    const ushort* raw = image->RawPixels();
    int width = image->Width();
    int height = image->Height();

    int count = 0;

    for( size_t y = 0; y < H; y++ ) {
        int Y = 2 * y + y0;
        if( Y < 0 || Y >= height ) {
            continue;
        }
        const ushort* srcLine = raw + width * Y;
        for( size_t x = 0; x < W; x++ ) {
            int X = 2 * x + x0;
            if( X < 0 || X >= width ) {
                continue;
            }
            const ushort* src = srcLine + X;
            ushort r = src[0];
            ushort g1 = src[1];
            ushort g2 = src[width];
            ushort b = src[width + 1];

            histR[r]++;
            histB[b]++;
            histG[g1]++;
            histG[g2]++;

            count++;
        }
    }

    const CChannelStat sR = stats.stat( 0, count );
    const CChannelStat sG = stats.stat( 1, count * 2);
    const CChannelStat sB = stats.stat( 2, count);

    size_t byteWidth = 3 * W;
    std::vector<uchar> pixels( byteWidth * H );
    uchar* rgb = pixels.data();
    for( size_t y = 0; y < H; y++ ) {
        int Y = 2 * y + y0;
        if( Y < 0 || Y >= height ) {
            continue;
        }
        const ushort* srcLine = raw + width * Y;
        uchar* dstLine = rgb + byteWidth * y;
        for( size_t x = 0; x < W; x++ ) {
            int X = 2 * x + x0;
            if( X < 0 || X >= width ) {
                continue;
            }
            const ushort* src = srcLine + X;
            uchar* dst = dstLine + 3 * x;

            uint r = src[0];
            uint g1 = src[1];
            uint g2 = src[width];
            uint b = src[width + 1];
            uint g = ( g1 + g2 ) / 2;

            if( r >= 4094 || g1 >= 4094 || g2 >= 4094 || b >= 4094 ) {
                dst[0] = 0xFF;
                dst[1] = 0x00;
                dst[2] = 0x80;
            } else {
                const int k = 12;
                dst[0] = r <= ( sR.Median + k * sR.Sigma ) ? ( r < sR.Median ? 0 : ( 255 * ( r - sR.Median ) / k / sR.Sigma ) ) : 255;
                dst[1] = g <= ( sG.Median + k * sG.Sigma ) ? ( g < sG.Median ? 0 : ( 255 * ( g - sG.Median ) / k / sG.Sigma ) ) : 255;
                dst[2] = b <= ( sB.Median + k * sB.Sigma ) ? ( b < sB.Median ? 0 : ( 255 * ( b - sB.Median ) / k / sB.Sigma ) ) : 255;
            }

            /*uint r = 0;
            uint g1 = 0;
            uint g2 = 0;
            uint b = 0;
            int count = 0;
            const int n = 5;
            for( int i = -n; i <= n; i++ )
            for( int j = -n; j <= n; j++ ) {
                int i0 = 2 * ( i * width + j );
                r += src[i0];
                g1 += src[i0 + 1];
                g2 += src[i0 + width];
                b += src[i0 + width + 1];
                count++;
            }
            r /= count;
            g1 /= count;
            g2 /= count;
            b /= count;
            uint g = ( g1 + g2 ) / 2;

            const int k = 4;
            const int m = 2 * n + 1;
            dst[0] = r < ( sR.Median + ( k * sR.Sigma ) / m ) ? 0 : 255;
            dst[1] = g < ( sG.Median + ( k * sG.Sigma ) / m ) ? 0 : 255;
            dst[2] = b < ( sB.Median + ( k * sB.Sigma ) / m ) ? 0 : 255;*/
        }
    }
    QImage im( rgb, W, H, byteWidth, QImage::Format_RGB888 );
    return QPixmap::fromImage( im );
}
