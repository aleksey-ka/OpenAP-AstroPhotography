// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_MATH_H
#define IMAGE_MATH_H

#include <Image.Image.h>

#include <map>
#include <cmath>
#include <limits>

template<class T, int numberOfChannels = 1>
std::tuple<double, double, T, T> simple_pixel_statistics( const T* pixels, size_t count, int channel = 0 )
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

template<typename T1, typename T2>
void pixels_set_value( T1* dst, T2 value, size_t count, int numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] = value;
    }
}

template<typename T1, typename T2>
void pixels_copy( T1* dst, const T2* src, size_t count, int numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] = src[i];
    }
}

template<typename T1, typename T2>
void pixels_add( T1* dst, const T2* src, size_t count, int numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] += src[i];
    }
}

template<typename T1, typename T2>
void pixels_add_value( T1* dst, T2 value, size_t count, int numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] += value;
    }
}

template<typename T1, typename T2>
void pixels_subtract( T1* dst, const T2* src, size_t count, int numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] -= src[i];
    }
}

template<typename T1, typename T2>
void pixels_subtract_value( T1* dst, T2 value, size_t count, int numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] -= value;
    }
}

template<typename T1, typename T2>
void pixels_divide( T1* dst, T2 value, size_t count, int numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] /= value;
    }
}

template<typename T1, typename T2, typename T3>
void pixels_divide( T1* dst, const T2* src, T3 value, size_t count, int numberOfChannels = 1 )
{
    for( size_t i = 0; i < count * numberOfChannels; i++ ) {
        dst[i] = src[i] / value;
    }
}

#endif // IMAGE_MATH_H
