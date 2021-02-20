// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_RGBIMAGE_H
#define IMAGE_RGBIMAGE_H

#include <vector>

template<typename T, int numOfChannels>
class CPixelBuffer {
public:
    CPixelBuffer( int width, int height );

    int Width() const { return width; }
    int Height() const { return height; }

    const T* ScanLine( int y ) const { return buffer.data() + y * stride; }
    T* ScanLine( int y ) { return buffer.data() + y * stride; }

protected:
    int width;
    int height;
    int stride;
    std::vector<T> buffer;
};

template<typename T, int numOfChannels>
inline CPixelBuffer<T, numOfChannels>::CPixelBuffer( int _width, int _height ) :
    width( _width ), height( _height )
{
    stride = numOfChannels * width;
    buffer.resize( sizeof( T ) * stride * height );
}

class CRgbU16Image : public CPixelBuffer<unsigned short, 3> {
public:
    using CPixelBuffer::CPixelBuffer;

    const unsigned short* RgbPixels() const { return buffer.data(); };
    unsigned short* RgbPixels() { return buffer.data(); };

    int Stride() const { return stride; }
};

class CRgbImage : public CPixelBuffer<unsigned char, 3> {
public:
    using CPixelBuffer::CPixelBuffer;

    const unsigned char* RgbPixels() const { return buffer.data(); };
    unsigned char* RgbPixels() { return buffer.data(); };

    int ByteWidth() const { return stride; }
};

#endif // IMAGE_RGBIMAGE_H
