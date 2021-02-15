// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "renderer.h"

#include <QPainter>

Renderer::Renderer( const ushort* _raw, int _width, int _height ) :
    raw( _raw ), width( _width ), height( _height )
{

}

QPixmap Renderer::Render( TRenderingMethod method )
{
    // Initialize histogram
    const int hSize = 256;
    histR.resize( hSize );
    histG.resize( hSize );
    histB.resize( hSize );

    if( method == RM_HalfResolution ) {
        size_t w = width / 2;
        size_t h = height / 2;
        size_t byteWidth = 3 * w;
        std::vector<uchar> pixels( byteWidth * h );

        renderHalfResolutionWithHistogram( pixels.data(), byteWidth );

        QImage image( pixels.data(), w, h, QImage::Format_RGB888 );
        return QPixmap::fromImage( image );
    } else {
        assert( method = RM_FullResolution );

        size_t byteWidth = 3 * width;
        std::vector<uchar> pixels( byteWidth * height );
        uchar* rgb = pixels.data();

        renderHighQualityLinearWithHistogram( rgb, byteWidth );

        QImage image( rgb, width, height, QImage::Format_RGB888 );
        return QPixmap::fromImage( image );
    }
}

void Renderer::renderHalfResolutionWithHistogram( uchar* rgb, int byteWidth )
{
    uint* hr = histR.data();
    uint* hg = histG.data();
    uint* hb = histB.data();

    for( size_t y = 0; y < height / 2; y++ ) {
        const ushort* srcLine = raw + 2 * width * y;
        uchar* dstLine = rgb + byteWidth * y;
        for( size_t x = 0; x < width / 2; x++ ) {
            const ushort* src = srcLine + 2 * x;
            uchar r = addToStatistics( src[0] ) >> 4;
            uchar g1 = addToStatistics( src[1] ) >> 4;
            uchar g2 = addToStatistics( src[width] ) >> 4;
            uchar b = addToStatistics( src[width + 1] ) >> 4;

            uchar* dst = dstLine + 3 * x;
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

static QString collapseNumber( uint n )
{
    // Collapse uint to Kilo-s and Mega-s with at most 3 digits (1.24K, 24.5M)
    if( n < 1000 ) {
        return QString::number( n );
    } else if( n < 10000 ) {
        return QString::number( n / 1000.0, 'f', 2 ) + "K";
    } else if( n < 100000 ) {
        return QString::number( n / 1000.0, 'f', 1 ) + "K";
    } else if( n < 1000000 ) {
        return QString::number( n / 1000 ) + "K";
    } else if( n < 10000000 ) {
        return QString::number( n / 1000000.0, 'f', 2 ) + "M";
    } else if( n < 100000000 ) {
        return QString::number( n / 1000000.0, 'f', 1 ) + "M";
    } else {
        return QString::number( n / 1000000 ) + "M";
    }
}

QPixmap Renderer::RenderHistogram()
{
    const uint* r = histR.data();
    const uint* g = histG.data();
    const uint* b = histB.data();

    int size = histR.size();

    uint max = 0;
    for( int i = 0; i < size - 1; i++ ) {
        if( r[i] > max ) {
            max = r[i];
        }
        if( g[i] / 2 > max ) {
            max = g[i] / 2;
        }
        if( b[i] > max ) {
            max = b[i];
        }
    }
    if( max == 0 ) {
        max = std::max( std::max( r[size - 1], g[size - 1] ), b[size - 1 ] );
    }

    int h = 128;
    int biteWidth = 3 * size;
    std::vector<uchar> hist( biteWidth * h );
    std::fill( hist.begin(), hist.end(), 0 );
    uchar* p = hist.data();
    if( max > 0 ) {
        for( int i = 0; i < size; i++ ) {
            int R = 127 - ( 127 * r[i] ) / max;
            int G = 127 - ( 127 * g[i] / 2 ) / max;
            int B = 127 - ( 127 * b[i] ) / max;
            for( int j = 0; j < h; j++ ) {
                uchar* p0 = p + j * biteWidth + 3 * i;
                if( R < 127 ) {
                    if( j > R ) {
                        p0[0] = 0x50;
                    } else if( j == R ){
                        p0[0] = 0xFF;
                    }
                }
                if( G < 127 ) {
                    if( j > G ) {
                        p0[1] = 0x50;
                    } else if( j == G ){
                        p0[1] = 0xFF;
                    }
                }
                if( B < 127 ) {
                    if( j > B ) {
                        // Make blue on black brighter (otherwise it is too dim)
                        if( p0[0] == 0 && p0[1] == 0 ) {
                            p0[2] = 0x80;
                        } else {
                            p0[2] = 0x50;
                        }
                    } else if( j == B ){
                        // Make blue outline brighter
                        p0[0] = 0x80;
                        p0[1] = 0x80;
                        p0[2] = j < 127 ? 0xFF : 0;
                    }
                }
            }
        }
    }

    QImage image( p, size, h, QImage::Format_RGB888 );
    auto pixmap = QPixmap::fromImage( image );

    // Add basic statistics (min/max etc)
    QPainter painter( &pixmap );
    QPen pen( QColor::fromRgb( 0x40, 0x80, 0x40 ) );
    painter.setPen( pen );
    QFont font( "Consolas" );
    font.setPointSizeF( 6.5 );
    painter.setFont( font );
    painter.drawText( size - 140, 10,
        QString( "max:%1 (%2%,%3)" ).arg(
            QString::number( maxValue ),
            QString::number( ( 100.0 * maxCount ) / ( width * height ), 'f', 1 ),
            collapseNumber( maxCount ) )
    );
    painter.drawText( size - 140, 20,
        QString( "min:%1 (%2%,%3)" ).arg(
            QString::number( minValue ),
            QString::number( ( 100.0 * minCount ) / ( width * height ), 'f', 1 ),
            collapseNumber( minCount ) )
    );
    painter.end();

    return pixmap;
}

QPixmap Renderer::RenderRectHalfRes( int cx, int cy, int W, int H )
{
    size_t w = width / 2;
    size_t h = height / 2;
    size_t byteWidth = 3 * w;
    std::vector<uchar> pixels( byteWidth * h );
    uchar* rgb = pixels.data();
    for( size_t y = cy - H / 2; y <= cy + H / 2; y++ ) {
        const ushort* srcLine = raw + 2 * width * y;
        uchar* dstLine = rgb + byteWidth * y;
        for( size_t x = cx - W / 2; x <= cx + W / 2; x++ ) {
            const ushort* src = srcLine + 2 * x;
            uchar r = src[0] / 256;
            uchar g = src[1] / 256;
            uchar b = src[width + 1] / 256;

            uchar* dst = dstLine + 3 * x;
            dst[0] = r;
            dst[1] = g;
            dst[2] = b;
        }
    }

    QImage image( rgb, w, h, QImage::Format_RGB888 );
    return QPixmap::fromImage( image );
}

QPixmap Renderer::RenderGrayScale()
{
    size_t byteWidth = 3 * width;
    std::vector<uchar> pixels( byteWidth * height );
    uchar* rgb = pixels.data();
    for( size_t y = 0; y < height; y++ ) {
        const ushort* srcLine = raw +  width * y;
        uchar* dstLine = rgb + byteWidth * y;
        for( size_t x = 0; x < width; x++ ) {
            const ushort* src = srcLine +  x;
            uchar v = src[0] >> 4;

            uchar* dst = dstLine + 3 * x;
            dst[0] = v;
            dst[1] = v;
            dst[2] = v;
        }
    }

    QImage image( rgb, width, height, QImage::Format_RGB888 );
    return QPixmap::fromImage( image );
}
