// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Math.h"

#include <Image.Debayer.HQLinear.h>

#include <QDebug>

CPixelStatistics::CPixelStatistics( int numberOfChannels, int bitsPerChannel ) :
    channelSize( 2 << bitsPerChannel )
{
    channels.resize( numberOfChannels );
    for( int i = 0; i < numberOfChannels; i++ ) {
        channels[i].resize( channelSize );
    }
}

CChannelStat CPixelStatistics::stat( int channel, int n ) const
{
    CChannelStat stat;
    int count = n * this->count;
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

QPixmap FocusingHelper( const Raw16Image* image, int x0, int y0, int w, int h )
{
    CRawU16PixelMath pixelMath( image->RawPixels(), image->Width(), image->Height() );
    auto result = pixelMath.Stretch( x0, y0, w, h );
    QImage im( result->RgbPixels(), w, h, result->ByteWidth(), QImage::Format_RGB888 );
    return QPixmap::fromImage( im );
}

CRawU16PixelMath::CRawU16PixelMath( const ushort* _raw, int _width, int _height ) :
    raw( _raw ), width( _width ), height( _height )
{

}

std::shared_ptr<CRgbU16Image> CRawU16PixelMath::DebayerRect( int x, int y, int w, int h )
{
    auto result = std::make_shared<CRgbU16Image>( w, h );
    CDebayer_RawU16_HQLiner debayer( raw, width, height );
    debayer.ToRgbU16( result->RgbPixels(), result->ByteWidth(), x, y, w, h );
    return result;
}

CPixelStatistics CRawU16PixelMath::CalculateStatistics( int x0, int y0, int W, int H )
{
    CPixelStatistics stats( 3, 12 );

    auto& histR = stats[0];
    auto& histG = stats[1];
    auto& histB = stats[2];

    const auto& size = histR.size();

    int count = 0;

    int cx = x0 + W / 2;
    int cy = y0 + H / 2;
    x0 = cx - W;
    y0 = cy - H;
    x0 += x0 % 2;
    y0 += y0 % 2;

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

    stats.setCount( count );

    return stats;
}

std::shared_ptr<CRgbImage> CRawU16PixelMath::Stretch( int x0, int y0, int W, int H )
{
    CPixelStatistics stats = CalculateStatistics( x0, y0, W, H );

    const CChannelStat sR = stats.stat( 0 );
    const CChannelStat sG = stats.stat( 1, 2);
    const CChannelStat sB = stats.stat( 2 );

    auto rgb16 = DebayerRect( x0, y0, W, H );

    auto result = std::make_shared<CRgbImage>( W, H );
    for( size_t y = 0; y < H; y++ ) {
        const ushort* srcLine = rgb16->RgbPixels() + rgb16->ByteWidth() * y;
        uchar* dstLine = result->RgbPixels() + result->ByteWidth() * y;
        for( size_t x = 0; x < W; x++ ) {
            const ushort* src = srcLine + 3 * x;
            uchar* dst = dstLine + 3 * x;

            uint r = src[0];
            uint g = src[1];
            uint b = src[2];

            if( r >= 4094 || g >= 4094 || b >= 4094 ) {
                dst[0] = 0xFF;
                dst[1] = 0x00;
                dst[2] = 0x80;
            } else {
                const int k = 16;
                dst[0] = r <= ( sR.Median + k * sR.Sigma ) ? ( r < sR.Median ? 0 : ( 255 * ( r - sR.Median ) / k / sR.Sigma ) ) : 255;
                dst[1] = g <= ( sG.Median + k * sG.Sigma ) ? ( g < sG.Median ? 0 : ( 255 * ( g - sG.Median ) / k / sG.Sigma ) ) : 255;
                dst[2] = b <= ( sB.Median + k * sB.Sigma ) ? ( b < sB.Median ? 0 : ( 255 * ( b - sB.Median ) / k / sB.Sigma ) ) : 255;
            }
        }
    }

    return result;
}

std::shared_ptr<CRgbImage> CRawU16PixelMath::StretchHalfRes( int x0, int y0, int W, int H )
{
    CPixelStatistics stats = CalculateStatistics( x0, y0, W, H );

    const CChannelStat sR = stats.stat( 0 );
    const CChannelStat sG = stats.stat( 1, 2);
    const CChannelStat sB = stats.stat( 2 );

    int cx = x0 + W / 2;
    int cy = y0 + H / 2;
    x0 = cx - W;
    y0 = cy - H;
    x0 += x0 % 2;
    y0 += y0 % 2;

    auto result = std::make_shared<CRgbImage>( W, H );
    for( size_t y = 0; y < H; y++ ) {
            int Y = 2 * y + y0;
            if( Y < 0 || Y >= height ) {
                continue;
            }
            const ushort* srcLine = raw + width * Y;
            uchar* dstLine = result->RgbPixels() + result->ByteWidth() * y;
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
            }
        }

    return result;
}
