// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <Image.Image.h>

#include <map>
#include <cmath>
#include <limits>
#include <cassert>

constexpr int maxValueForBitDepth( int bitDepth )
{
    return ( 2 << bitDepth ) - 1;
}

template<typename T1, typename T2>
inline void pixels_set( T1* dst, const T2* src, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] = src[i];
    }
}

template<typename T1, typename T2>
inline void pixels_set_value( T1* dst, T2 value, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] = value;
    }
}

template<typename T1, typename T2>
inline void pixels_set_round( T1* dst, const T2* src, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] = std::round( src[i] );
    }
}

template<typename T1, typename T2>
inline void pixels_set_round_limit( T1* dst, const T2* src, size_t count, int bitDepth, size_t numberOfChannels = 1 )
{
    int maxValue = maxValueForBitDepth( bitDepth );
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        double value = std::round( src[i] );
        dst[i] = value < 0 ? 0 : ( value < maxValue ? value : maxValue );
    }
}

template<typename T1, typename T2>
inline void pixels_set_multiply_by_value( T1* dst, const T2* src, T2 value, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] = src[i] * value;
    }
}

template<typename T1, typename T2>
inline void pixels_add( T1* dst, const T2* src, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] += src[i];
    }
}

template<typename T1, typename T2>
inline void pixels_add_value( T1* dst, T2 value, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] += value;
    }
}

template<typename T1, typename T2>
inline void pixels_add_multiply_by_value( T1* dst, const T2* src, T2 value, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] += src[i] * value;
    }
}

template<typename T1, typename T2>
inline void pixels_subtract( T1* dst, const T2* src, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] -= src[i];
    }
}

template<typename T1, typename T2>
inline void pixels_subtract_value( T1* dst, T2 value, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] -= value;
    }
}

template<typename T1, typename T2>
inline void pixels_multiply( T1* dst, const T2* src, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] *= src[i];
    }
}

template<typename T1, typename T2>
inline void pixels_multiply_by_value( T1* dst, T2 value, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] *= value;
    }
}

template<typename T1, typename T2>
inline void pixels_divide( T1* dst, const T2* src, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] /= src[i];
    }
}

template<typename T1, typename T2>
inline void pixels_divide_by_value( T1* dst, T2 value, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] /= value;
    }
}

template<typename T1, typename T2, typename T3>
inline void pixels_divide_round( T1* dst, const T2* src, T3 value, size_t count, size_t numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] = std::round( src[i] / value );
    }
}

template<class T, int numberOfChannels = 1>
std::tuple<double, double, T, T> simple_pixel_statistics( const T* pixels, size_t count, size_t channel = 0 )
{
    auto minValue = std::numeric_limits<T>::max();
    auto maxValue = std::numeric_limits<T>::min();
    double sum_v = 0;
    double sum_vv = 0;
    const T* channelPixels = pixels + channel;
    for( size_t i = 0; i < count; i++ ) {
        T pixelValue = channelPixels[numberOfChannels * i];
        if( pixelValue > maxValue ) {
            maxValue = pixelValue;
        }
        if( pixelValue < minValue ) {
            minValue = pixelValue;
        }
        double v = pixelValue;
        sum_v += v;
        sum_vv += v * v;
    }
    return std::make_tuple( sum_v / count, sqrt( sum_vv / count - ( sum_v * sum_v ) / count / count ), minValue, maxValue );
}

class CHistogram {
public:
    CHistogram( size_t numberOfChannels, size_t bitsPerChannel );

    std::vector<unsigned int>& operator [] ( size_t index ) { return channels[index]; }
    const std::vector<unsigned int>& operator [] ( size_t index ) const { return channels[index]; }
    void SetPixelsCount( unsigned int value ) { count = value; }

    unsigned int ChannelSize() const { return channelSize; }
    unsigned int PixelsCount() const { return count; }

private:
    const unsigned int channelSize;
    std::vector<std::vector<unsigned int>> channels;
    unsigned int count = 0;
};

template<typename T>
inline CHistogram pixels_histogram( const T* pixels, size_t count, size_t bitsPerChannel, size_t numberOfChannels = 1 )
{
    if( numberOfChannels == 1 ) {
        // Monochrome
        CHistogram result( 1, bitsPerChannel );
        auto& ch0 = result[0];
        for( size_t i = 0; i < count; i++ ) {
            ch0[pixels[i]]++;
        }
        result.SetPixelsCount( count );
        return result;
    } else if( numberOfChannels == 3 ) {
        // Color
        CHistogram result( 3, bitsPerChannel );
        auto& ch0 = result[0];
        auto& ch1 = result[1];
        auto& ch2 = result[2];
        for( size_t i = 0; i < count; i++ ) {
            const T* p = pixels + 3 * i;
            ch0[p[0]]++;
            ch1[p[1]]++;
            ch2[p[2]]++;
        }
        result.SetPixelsCount( count );
        return result;
    } else {
        assert( false );
        CHistogram result( 0, 0 );
        return result;
    }
}

template<typename T>
inline CHistogram pixels_histogram_float( const T* pixels, size_t count, size_t bitsPerChannel, size_t numberOfChannels = 1 )
{
    if( numberOfChannels == 1 ) {
        // Monochrome
        CHistogram result( 1, bitsPerChannel );
        auto& ch0 = result[0];
        for( size_t i = 0; i < count; i++ ) {
            ch0[std::round( pixels[i] )]++;
        }
        result.SetPixelsCount( count );
        return result;
    } else if( numberOfChannels == 3 ) {
        // Color
        CHistogram result( 3, bitsPerChannel );
        auto& ch0 = result[0];
        auto& ch1 = result[1];
        auto& ch2 = result[2];
        for( size_t i = 0; i < count; i++ ) {
            const T* p = pixels + 3 * i;
            ch0[std::round( p[0] )]++;
            ch1[std::round( p[1] )]++;
            ch2[std::round( p[2] )]++;
        }
        result.SetPixelsCount( count );
        return result;
    } else {
        assert( false );
        CHistogram result( 0, 0 );
        return result;
    }
}

template<typename T>
inline CHistogram pixels_patch_histogram( const T* pixels, size_t width, size_t height, size_t x0, size_t y0, size_t w, size_t h, int bitsPerChannel, int numberOfChannels = 1 )
{
    if( numberOfChannels == 1 ) {
        // Monochrome
        CHistogram result( 1, bitsPerChannel );
        auto& ch0 = result[0];
        int count = 0;
        for( size_t i = 0, y = y0; i < h && y < height; i++, y++ ) {
            const T* p = pixels + y * width + x0;
            for( size_t j = 0, x = x0; j < w && x < width; j++, x++ ) {
                ch0[p[j]]++;
                count++;
            }
        }
        result.SetPixelsCount( count );
        return result;
    } else {
        assert( false );
        CHistogram result( 0, 0 );
        return result;
    }
}

template<typename T>
inline CHistogram pixels_patch_histogram_float( const T* pixels, size_t width, size_t height, size_t x0, size_t y0, size_t w, size_t h, int bitsPerChannel, int numberOfChannels = 1 )
{
    if( numberOfChannels == 1 ) {
        // Monochrome
        CHistogram result( 1, bitsPerChannel );
        auto& ch0 = result[0];
        int count = 0;
        for( size_t i = 0, y = y0; i < h && y < height; i++, y++ ) {
            const T* p = pixels + y * width + x0;
            for( size_t j = 0, x = x0; j < w && x < width; j++, x++ ) {
                ch0[std::round( p[j] )]++;
                count++;
            }
        }
        result.SetPixelsCount( count );
        return result;
    } else {
        assert( false );
        CHistogram result( 0, 0 );
        return result;
    }
}

size_t pixels_histogram_p( const CHistogram& h, size_t channel, size_t target );

inline size_t pixels_histogram_median( const CHistogram& h, size_t channel )
{
    return pixels_histogram_p( h, channel, h.PixelsCount() / 2 );
}
