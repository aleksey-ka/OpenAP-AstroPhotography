// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_MATH_H
#define IMAGE_MATH_H

#include <Image.Image.h>
#include <Image.RawImage.h>

struct CChannelStat {
    int Median;
    int Sigma;
};

class CPixelStatistics {
public:
    CPixelStatistics( int numberOfChannels, int bitsPerChannel );

    std::vector<unsigned int>& operator [] ( size_t index ) { return channels[index]; }
    const std::vector<unsigned int>& operator [] ( size_t index ) const { return channels[index]; }

    CChannelStat stat( int channel, int n = 1 ) const;

    size_t p( int channel, int target ) const;
    size_t maxP( int channel, int start, int end ) const;

    void setCount( int _count ) { count = _count; }

private:
    const size_t channelSize;
    std::vector<std::vector<unsigned int>> channels;
    int count = 0;
};

class CRawU16 {
public:
    CRawU16( const CRawU16Image* );
    CRawU16( const unsigned short* raw, int width, int height );

    std::shared_ptr<CRgbU16Image> DebayerRect( int x, int y, int width, int height );

    CPixelStatistics CalculateStatistics( int x, int y, int width, int height );

    std::shared_ptr<CRgbImage> Stretch( int x, int y, int w, int h );
    std::shared_ptr<CRgbImage> StretchHalfRes( int x, int y, int w, int h );

    static std::shared_ptr<CGrayU16Image> ToGrayU16( const CRgbU16Image* );
    static std::shared_ptr<CGrayImage> ToGray( const CGrayU16Image* );

private:
    const unsigned short* raw;
    int width;
    int height;
};

#endif // IMAGE_MATH_H
