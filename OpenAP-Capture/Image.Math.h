// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_MATH_H
#define IMAGE_MATH_H

#include <Image.Image.h>
#include <Image.RawImage.h>

#include <map>
#include <cmath>
#include <limits>

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

struct DetectionRegion {
    int Vmax;
    int Xmax;
    int Ymax;
    int Cmax;
    int Background;
    int AreaAtMax;
    int AreaAt90PercentOfMax;
    int AreaAtHalfMax;
    int AreaAt10PercentOfMax;
    int AreaAtHalfDetectionThreshold;
    unsigned FluxAtHalfDetectionThreshold;
};

struct DetectionResults {
    std::vector<std::shared_ptr<DetectionRegion>> DetectionRegions;
    std::shared_ptr<CGrayU16Image> Image;
    std::shared_ptr<CGrayImage> Mask;
};

class CRawU16 {
public:
    CRawU16( const CRawU16Image* );
    CRawU16( const unsigned short* raw, int width, int height, int bitDepth );

    std::shared_ptr<CRgbU16Image> DebayerRect( int x, int y, int width, int height ) const;
    std::shared_ptr<CGrayU16Image> GrayU16( int x, int y, int width, int height ) const;

    CPixelStatistics CalculateStatistics( int x, int y, int width, int height ) const;

    std::shared_ptr<CRgbImage> Stretch( int x, int y, int w, int h ) const;
    std::shared_ptr<CRgbImage> StretchHalfRes( int x, int y, int w, int h ) const;
    std::shared_ptr<CRgbImage> StretchQuarterRes( int x, int y, int w, int h ) const;

    DetectionResults DetectStars( int x, int y, int w, int h ) const;

    static std::shared_ptr<CGrayU16Image> ToGrayU16( const CRgbU16Image* );
    static std::shared_ptr<CGrayImage> ToGray( const CGrayU16Image* );

    void GradientAscentToLocalMaximum( int& x, int& y, int size );
    static void GradientAscentToLocalMaximum( const CGrayU16Image*, int& x, int& y, int window );

    static CPixelStatistics CalculateStatistics( const CGrayU16Image* );

private:
    const unsigned short* raw;
    int width;
    int height;
    int bitDepth;
};

class CFocusingHelper {
public:
    CFocusingHelper() {}
    CFocusingHelper( int x, int y ) : cx( x ), cy( y ) {}
    void AddFrame( const CRawU16Image*, int imageSize, int focuserPos );

    int R = 0;
    int R_out = 0;
    int cx = 0;
    int cy = 0;
    double HFD = 0;
    double CX = 0;
    double CY = 0;
    std::shared_ptr<const CGrayImage> Mask;
    std::vector<std::pair<double,double>> Stars;

    std::shared_ptr<CPixelBuffer<double, 3>> Stack;
    int StackSize;
    std::shared_ptr<CRgbImage> GetStackedImage( bool stratch, int factor ) const;
    int MaxStackSize = 0;
    void SetStackSize( int stackSize ) { MaxStackSize = stackSize; };

    struct Data {
        std::vector<double> HFD;
        std::vector<double> FWHM;
        std::vector<double> Max;

        std::vector<double> CX;
        std::vector<double> CY;

        std::vector<DetectionResults> theDetectionResults;
    };

    std::shared_ptr<Data> currentSeries;
    std::map<int, std::shared_ptr<Data>> focuserStats;
    std::vector<int> focuserPositions;

    struct FocuserPosStats {
        int Pos;
        double HFD;

        FocuserPosStats( int pos, double hfd ) : Pos( pos ), HFD( hfd ) {}
    };

    FocuserPosStats getFocuserStats( int focuserPosition )
    {
        double sumV = 0;
        int count = 0;
        for( auto v : focuserStats[focuserPosition]->HFD ) {
            sumV += v;
            count++;
        }
        return FocuserPosStats( focuserPosition, sumV / count );
    }

    FocuserPosStats getFocuserStatsByIndex( size_t index )
    {
        return getFocuserStats( focuserPositions[index] );
    }

    std::vector<std::shared_ptr<CFocusingHelper>> extra;
    int countL = 0;
    double sumD = 0;
    double sumDD = 0;
    double L;
    double sigmaL;
    double dCX = 0;
    double dCY = 0;
    double sigmadCX = 0;
    double sigmadCY = 0;

    double PX = 0;
    double PY = 0;

    double PX1 = 0;
    double PY1 = 0;

    void addExtra( int x, int y )
    {
        extra.push_back( std::make_shared<CFocusingHelper>( x, y ) );
        L = 0;
        countL = 0;
        sumD = 0;
        sumDD = 0;
    }

    bool isGlobalPolarAlign = false;
    void toggleGlobalPolarAllign() { isGlobalPolarAlign = !isGlobalPolarAlign; }
};

#endif // IMAGE_MATH_H
