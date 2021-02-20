// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_MATH_H
#define IMAGE_MATH_H

#include <QImage>
#include <QPixmap>
#include <Image.h>

struct CChannelStat {
    int Median;
    int Sigma;
};

class CPixelStatistics {
public:
    CPixelStatistics( int numberOfChannels, int bitsPerChannel );

    std::vector<uint>& operator [] ( size_t index ) { return channels[index]; }
    const std::vector<uint>& operator [] ( size_t index ) const { return channels[index]; }

    CChannelStat stat( int channel, int n = 1 ) const;

    size_t p( int channel, int target ) const;
    size_t maxP( int channel, int start, int end ) const;

    void setCount( int _count ) { count = _count; }

private:
    const size_t channelSize;
    std::vector<std::vector<uint>> channels;
    int count = 0;
};

QPixmap FocusingHelper( const Raw16Image* image, int x0, int y0, int W, int H );

template<typename T, int numOfChannels>
class CPixelBuffer {
public:
    CPixelBuffer( int width, int height );

    int Width() const { return width; }
    int Height() const { return height; }

    int ByteWidth() const { return byteWidth; }

protected:
    int width;
    int height;
    int byteWidth;
    mutable std::vector<T> buf;
};

template<typename T, int numOfChannels>
inline CPixelBuffer<T, numOfChannels>::CPixelBuffer( int _width, int _height ) :
    width( _width ), height( _height )
{
    byteWidth = numOfChannels * sizeof( ushort) * width;
    buf.resize( byteWidth * height );
}

class CRgbU16Image : public CPixelBuffer<unsigned short, 3> {
public:
    using CPixelBuffer::CPixelBuffer;

    const unsigned short* RgbPixels() const { return buf.data(); };
    unsigned short* RgbPixels() { return buf.data(); };
};

class CRgbImage : public CPixelBuffer<unsigned char, 3> {
public:
    using CPixelBuffer::CPixelBuffer;

    const unsigned char* RgbPixels() const { return buf.data(); };
    unsigned char* RgbPixels() { return buf.data(); };
};

class CRawU16PixelMath {
public:
    CRawU16PixelMath( const ushort* raw, int width, int height );

    std::shared_ptr<CRgbU16Image> DebayerRect( int x, int y, int width, int height );

    CPixelStatistics CalculateStatistics( int x, int y, int width, int height );

    std::shared_ptr<CRgbImage> Stretch( int x, int y, int w, int h );
    std::shared_ptr<CRgbImage> StretchHalfRes( int x, int y, int w, int h );

private:
    const ushort* raw;
    int width;
    int height;
};

#endif // IMAGE_MATH_H
