// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Math.h"

#include <Image.Debayer.HQLinear.h>

#include <QDebug>

#include <math.h>
#include <stack>

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

CRawU16::CRawU16( const CRawU16Image* image ) :
    CRawU16( image->RawPixels(), image->Width(), image->Height(), image->BitDepth() )
{
}

CRawU16::CRawU16( const ushort* _raw, int _width, int _height, int _bitDepth ) :
    raw( _raw ), width( _width ), height( _height ), bitDepth( _bitDepth )
{

}

std::shared_ptr<CRgbU16Image> CRawU16::DebayerRect( int x, int y, int w, int h ) const
{
    auto result = std::make_shared<CRgbU16Image>( w, h );
    CDebayer_RawU16_HQLinear debayer( raw, width, height, bitDepth );
    debayer.ToRgbU16( result->RgbPixels(), result->Stride(), x, y, w, h );
    return result;
}

std::shared_ptr<CGrayU16Image> CRawU16::GrayU16( int x, int y, int width, int height ) const
{
    return ToGrayU16( DebayerRect( x, y, width, height ).get() );
}

CPixelStatistics CRawU16::CalculateStatistics( int x0, int y0, int W, int H ) const
{
    CPixelStatistics stats( 3, bitDepth );

    auto& histR = stats[0];
    auto& histG = stats[1];
    auto& histB = stats[2];

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

std::shared_ptr<CRgbImage> CRawU16::Stretch( int x0, int y0, int W, int H ) const
{
    CPixelStatistics stats = CalculateStatistics( x0, y0, W, H );

    const int maxValue = ~(~0u << bitDepth) - 1;

    const CChannelStat sR = stats.stat( 0 );
    const CChannelStat sG = stats.stat( 1, 2);
    const CChannelStat sB = stats.stat( 2 );

    auto rgb16 = DebayerRect( x0, y0, W, H );

    auto result = std::make_shared<CRgbImage>( W, H );
    for( size_t y = 0; y < H; y++ ) {
        const ushort* srcLine = rgb16->ScanLine( y );
        uchar* dstLine = result->ScanLine( y );
        for( size_t x = 0; x < W; x++ ) {
            const ushort* src = srcLine + 3 * x;
            uchar* dst = dstLine + 3 * x;

            uint r = src[0];
            uint g = src[1];
            uint b = src[2];

            if( r >= maxValue || g >= maxValue || b >= maxValue ) {
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

std::shared_ptr<CRgbImage> CRawU16::StretchHalfRes( int x0, int y0, int W, int H ) const
{
    CPixelStatistics stats = CalculateStatistics( x0, y0, W, H );

    const int maxValue = ~(~0u << bitDepth) - 1;

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

                if( r >= maxValue || g1 >= maxValue || g2 >= maxValue || b >= maxValue ) {
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

std::shared_ptr<CGrayU16Image> CRawU16::ToGrayU16( const CRgbU16Image* rgb16 )
{
    int width = rgb16->Width();
    int height = rgb16->Height();
    auto result = std::make_shared<CGrayU16Image>( width, height );
    for( int y = 0; y < height; y++ ) {
        const ushort* srcLine = rgb16->ScanLine( y );
        ushort* dstLine = result->ScanLine( y );
        for( int x = 0; x < width; x++ ) {
            const ushort* src = srcLine + 3 * x;
            uint r = src[0];
            uint g = src[1];
            uint b = src[2];

            dstLine[x] = r + g + b;
        }
    }

    return result;
}

std::shared_ptr<CGrayImage> CRawU16::ToGray( const CGrayU16Image* rgb16 )
{
    int width = rgb16->Width();
    int height = rgb16->Height();
    auto result = std::make_shared<CGrayImage>( width, height );
    for( int y = 0; y < height; y++ ) {
        const ushort* srcLine = rgb16->ScanLine( y );
        uchar* dstLine = result->ScanLine( y );
        for( int x = 0; x < width; x++ ) {
            const ushort v = srcLine[x] >> 3;
            dstLine[x] = v < 255 ? v : 255;
        }
    }

    return result;
}

void CRawU16::GradientAscentToLocalMaximum( int& x, int& y, int size )
{
    auto gray = GrayU16( x - size / 2, y - size / 2, size, size );
    int _x = size / 2;
    int _y = size / 2;
    CRawU16::GradientAscentToLocalMaximum( gray.get(), _x, _y, 25 );
    CRawU16::GradientAscentToLocalMaximum( gray.get(), _x, _y, 3 );
    x += _x - size / 2;
    y += _y - size / 2;
}

void CRawU16::GradientAscentToLocalMaximum( const CGrayU16Image* image, int& x, int& y, int window )
{
    int halfW = window / 2;
    int stride = image->Stride();
    while( true ) {
        const ushort* p = image->ScanLine( y ) + x;
        int maxv = 0;
        int maxPos = 0;
        int pos = 0;
        // For each pixel around the current pixel
        for( int i = -halfW; i <= halfW; i+= halfW ) {
            for( int j = -halfW; j <= halfW; j+= halfW ) {
                const ushort* s = p + i * stride + j;
                int v = 0;
                // Sum values of pixels in 3x3 area
                for( int n = -halfW; n <= halfW; n++ ) {
                    for( int m = -halfW; m <= halfW; m++ ) {
                        v += s[n * stride + m];
                    }
                }
                if( v > maxv ) {
                    maxv = v;
                    maxPos = pos;
                }
                pos++;
            }
        }
        // Move to the found maximum
        switch( maxPos ) {
            case 0: x--; y--; break;
            case 1: y--; break;
            case 2: x++; y--; break;
            case 3: x--; break;
            case 4: return;
            case 5: x++; break;
            case 6: x--; y++; break;
            case 7: y++; break;
            case 8: x++; y++; break;
        }
    }
}

CPixelStatistics CRawU16::CalculateStatistics( const CGrayU16Image* image )
{
    CPixelStatistics stats( 1, 16 );
    auto& hist = stats[0];

    int width = image->Width();
    int height = image->Height();

    int count = 0;
    for( int y = 0; y < height; y++ ) {
        const ushort* srcLine = image->ScanLine( y );
        for( int x = 0; x < width; x++ ) {
            const ushort* src = srcLine + x;
            ushort v = src[0];
            hist[v]++;
            count++;
        }
    }
    stats.setCount( count );

    return stats;
}

static std::shared_ptr<CGrayImage> starMask( const CGrayU16Image* image, int x, int y, int t, int width, int height, std::shared_ptr<CGrayImage> result, int value )
{
    std::stack<std::tuple<int, int>> s;
    s.emplace( x, y );
    while( not s.empty() ) {
        int x, y;
        std::tie( x, y ) = s.top();
        s.pop();
        if( x >= 0 && x < width && y > 0 && y < height ) {
            auto dst = result->At( x, y );
            if( dst[0] < value && image->At( x, y )[0] >= t ) {
                dst[0] = value;
                s.emplace( x - 1, y );
                s.emplace( x + 1, y );
                s.emplace( x - 1, y - 1 );
                s.emplace( x, y - 1);
                s.emplace( x + 1, y - 1);
                s.emplace( x - 1, y + 1 );
                s.emplace( x, y + 1);
                s.emplace( x + 1, y + 1);
            }
        }
    }
    return result;
}

static std::shared_ptr<CGrayImage> starMask( const CGrayU16Image* image, int x, int y, int t, int value = 128 )
{
    int width = image->Width();
    int height = image->Height();
    std::shared_ptr<CGrayImage> result = std::make_shared<CGrayImage>( width, height );
    return starMask( image, x, y, t, width, height, result, value );
}

static std::shared_ptr<CGrayImage> starMask( const CGrayU16Image* image, int x, int y, int t, int value, std::shared_ptr<CGrayImage>& result )
{
    int width = image->Width();
    int height = image->Height();
    return starMask( image, x, y, t, width, height, result, value );
}

void CFocusingHelper::AddFrame( const CRawU16Image* currentImage, int _cx, int _cy, int imageSize, int focuserPos )
{
    double prevCX = CX;
    double prevCY = CY;

    cx = _cx;
    cy = _cy;

    // Lock on the star (center on local maximum)
    CRawU16 rawU16( currentImage );
    rawU16.GradientAscentToLocalMaximum( cx, cy, imageSize );
    auto image = rawU16.GrayU16( cx - imageSize / 2, cy - imageSize / 2, imageSize, imageSize );

    int height = image->Height();
    int width = image->Width();

    const auto s = CRawU16::CalculateStatistics( image.get() ).stat( 0 );
    qDebug() << "Median:" << s.Median << "Sigma:" << s.Sigma;

    int cVal = image->At( imageSize / 2, imageSize / 2 )[0];

    int maxVal = 0;
    int halfMax = s.Median + ( cVal - s.Median ) / 2;
    int count = 0;
    for( int y = imageSize / 2 - 10; y <= imageSize / 2 + 10; y++ ) {
        const ushort* srcLine = image->ScanLine( y );
        for( int x = imageSize / 2 - 10; x < imageSize / 2 + 10; x++ ) {
            const ushort* src = srcLine + x;
            ushort v = src[0];
            if( v >= halfMax ) {
                count++;
            }
            if( v > maxVal ) {
                maxVal = v;
            }
        }
    }

    qDebug() << "Value at center:" << cVal;
    qDebug() << "Max val:" << maxVal;
    qDebug() << "Count at half max:" << count;
    qDebug() << "FWHM:" << 2 * sqrt ( count / M_PI );


    auto mask = starMask( image.get(), imageSize / 2, imageSize / 2, s.Median + s.Sigma, 16 );
    starMask( image.get(), imageSize / 2, imageSize / 2, s.Median + ( cVal - s.Median ) / 10, 32, mask );
    starMask( image.get(), imageSize / 2, imageSize / 2, s.Median + ( cVal - s.Median ) / 2, 128, mask );
    starMask( image.get(), imageSize / 2, imageSize / 2, s.Median + ( 9 * ( cVal - s.Median ) ) / 10, 192, mask );
    starMask( image.get(), imageSize / 2, imageSize / 2, s.Median + ( 19 * ( cVal - s.Median ) ) / 20, 255, mask );

    double sumVX = 0;
    double sumVY = 0;
    double sumV = 0;
    for( int y = 0; y < height; y++ ) {
        const ushort* src = image->ScanLine( y );
        const uchar* msk = mask->ScanLine( y );
        for( int x = 0; x < width; x++ ) {
            if( msk[x] > 0 ) {
                double value = src[x] - halfMax;
                if( value > 0 ) {
                    sumVX += value * ( x - imageSize / 2 );
                    sumVY += value * ( y - imageSize / 2 );
                    sumV += value;
                }
            }
        }
    }
    double dX = sumVX / sumV;
    double dY = sumVY / sumV;
    qDebug() << "dX:" << QString::number( dX, 'f', 2 ) << "dY:" << QString::number( dY, 'f', 2 );

    double meanSky = INT_MAX;
    if( R == 0 ) {
        const int dR = 10;
        int bestR = 0;
        int samples = 0;
        while( true ) {
            double sumV = 0;
            int count = 0;
            for( int y = 0; y < height; y++ ) {
                const ushort* src = image->ScanLine( y );
                for( int x = 0; x < width; x++ ) {
                    int dx = x - imageSize / 2;
                    int dy = y - imageSize / 2;
                    int rr = dx * dx + dy * dy;
                    if( rr >= R * R && rr < ( R + dR ) * ( R + dR ) ) {
                        sumV += src[x];
                        count++;
                    }
                }
            }
            double current = sumV / count;
            //qDebug() << "R:" << R << "Mean Sky:" << current;
            bool foundBoundary = false;
            if( current < meanSky || R == 0 ) {
                meanSky = current;
                bestR = R;
            } else {
                foundBoundary = true;
                //qDebug() << "----------";
            }
            if( foundBoundary || samples > 0 ) {
                if( ++samples == 15 ) {
                    break;
                }
            }
            R += 3;
        }
        R = bestR;
        R_out = R + dR;
    } else {
        double sumV = 0;
        int count = 0;
        for( int y = 0; y < height; y++ ) {
            const ushort* src = image->ScanLine( y );
            for( int x = 0; x < width; x++ ) {
                int dx = x - imageSize / 2;
                int dy = y - imageSize / 2;
                int rr = dx * dx + dy * dy;
                if( rr >= R * R && rr < ( R + 10 ) * ( R + 10 ) ) {
                    sumV += src[x];
                    count++;
                }
            }
        }
        meanSky = sumV / count;
    }
    qDebug() << "R:" << R << "Mean Sky:" << meanSky;

    sumV = 0;
    double sumVR = 0;
    for( int y = 0; y < height; y++ ) {
        const ushort* src = image->ScanLine( y );
        const uchar* msk = mask->ScanLine( y );
        for( int x = 0; x < width; x++ ) {
            if( msk[x] >= 32 ) {
                double value = src[x] - meanSky; // (?) ( s.Median + ( cVal - s.Median ) / 10 );
                for( int n = 0; n < 3; n++ ) {
                    for( int m = 0; m < 3; m++ ) {
                        double dx = x - ( imageSize / 2 + dX ) + n / 3.0;
                        double dy = y - ( imageSize / 2 + dY ) + m / 3.0;
                        double r = sqrt( dx * dx + dy * dy );
                        if( r < R ) {
                            sumVR += value * r;
                            sumV += value;
                        }
                    }
                }
            }
        }
    }
    HFD = 2 * sumVR / sumV;
    qDebug() << "HFD:" << QString::number( HFD, 'f', 2 );

    double halfMax2 = meanSky + ( maxVal - meanSky ) / 2;
    count = 0;
    for( int y = imageSize / 2 - 10; y <= imageSize / 2 + 10; y++ ) {
        const ushort* src = image->ScanLine( y );
        for( int x = imageSize / 2 - 10; x < imageSize / 2 + 10; x++ ) {
            if( src[x] - 0.5 >= halfMax2 ) {
                count++;
            }
        }
    }

    qDebug() << "(2) Count at half max:" << count;
    qDebug() << "(2) FWHM:" << 2 * sqrt ( count / M_PI );
    qDebug() << "(2) Max:" << maxVal;

    Mask = mask;

    CX = cx + dX;
    CY = cy + dY;

    auto it = focuserStats.find( focuserPos );
    if( it == focuserStats.end() ) {
        currentSeries = std::make_shared<Data>();
        focuserStats[focuserPos] = currentSeries;
        focuserPositions.push_back( focuserPos );
        std::sort( focuserPositions.begin(), focuserPositions.end() );
    } else {
        currentSeries = it->second;
    }

    currentSeries->HFD.push_back( HFD );
    currentSeries->FWHM.push_back( 2 * sqrt ( count / M_PI ) );
    currentSeries->Max.push_back( maxVal );

    double sumdCX = 0;
    double sumdCY = 0;
    double sumdCXdCX = 0;
    double sumdCYdCY = 0;
    dCX = CX - prevCX;
    dCY = CY - prevCY;

    if( extra.size() > 0 ) {
        sumdCX += dCX;
        sumdCXdCX += dCX * dCX;
        sumdCY += dCY;
        sumdCYdCY += dCY * dCY;
        int countC = 1;

        CFocusingHelper* prev = this;
        double d = 0; 
        for( auto helper : extra ) {
            double prevCX = helper->CX;
            double prevCY = helper->CY;
            helper->AddFrame( currentImage, helper->cx, helper->cy, imageSize, focuserPos );

            double dCX = helper->CX - prevCX;
            double dCY = helper->CY - prevCY;
            sumdCX += dCX;
            sumdCXdCX += dCX * dCX;
            sumdCY += dCY;
            sumdCYdCY += dCY * dCY;
            countC++;

            double dx = prev->CX - helper->CX;
            double dy = prev->CY - helper->CY;
            d += sqrt( dx * dx + dy * dy );
            prev = helper.get();
        }      
        sumD += d;
        sumDD += d * d;
        countL++;
        L = sumD / countL;
        sigmaL = sqrt( sumDD / countL - ( sumD * sumD ) / countL / countL );

        sigmadCX = sqrt( sumdCXdCX / countC - ( sumdCX * sumdCX ) / countC / countC );
        sigmadCY = sqrt( sumdCYdCY / countC - ( sumdCY * sumdCY ) / countC / countC );

        dCX = sumdCX / countC;
        dCY = sumdCY / countC;
    }
}
