// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Stack.h"
#include <QDebug>

template<typename T>
static std::tuple<size_t, size_t> patches_statistics( const T* pixels, size_t width, size_t height, int bitDepth, int numverOfPatches )
{
    size_t minM = INT_MAX;
    size_t maxM = 0;

    const size_t w = width / numverOfPatches;
    const size_t h = height / numverOfPatches;
    for( int i = 0; i < numverOfPatches; i++ ) {
        int y0 = i * h;
        for( int j = 0; j < numverOfPatches; j++ ) {
            int x0 = j * w;
            CHistogram hi = pixels_patch_histogram( pixels, width, height, x0, y0, w, h, bitDepth );
            size_t m = pixels_histogram_median( hi, 0 );
            if( m > maxM ) {
                maxM = m;
            }
            if( m < minM ) {
                minM = m;
            }
        }
    }

    return std::make_tuple( minM, maxM );
}

template<typename T>
static std::tuple<size_t, size_t> patches_statistics_float( const T* pixels, size_t width, size_t height, int bitDepth, int numberOfPatches )
{
    size_t minM = INT_MAX;
    size_t maxM = 0;

    const size_t w = width / numberOfPatches;
    const size_t h = height / numberOfPatches;
    for( int i = 0; i < numberOfPatches; i++ ) {
        int y0 = i * h;
        for( int j = 0; j < numberOfPatches; j++ ) {
            int x0 = j * w;
            CHistogram hi = pixels_patch_histogram_float( pixels, width, height, x0, y0, w, h, bitDepth );
            size_t m = pixels_histogram_median( hi, 0 );
            if( m > maxM ) {
                maxM = m;
            }
            if( m < minM ) {
                minM = m;
            }
        }
    }

    return std::make_tuple( minM, maxM );
}

template<typename T>
static std::shared_ptr<CRgbImage> correlationGraph( const T* pixels1, const T* pixels2, size_t count, int bitDepth, size_t size = 1024 )
{
    auto result = std::make_shared<CRgbImage>( size + 8, size + 8 );

    int maxValue = ( 2 << bitDepth ) - 1;
    double k = ( 1.0 * size ) /  maxValue;

    for( size_t i = 0; i < count; i++ ) {
        double v1 = pixels1[i] * k + 4;
        double v2 = pixels2[i] * k + 4;

        if( v1 < 0 ) v1 = 0;
        if( v2 < 0 ) v2 = 0;
        auto p = result->Ptr( v1 , v2 );
        /*if( p[1] < 255 ) {
            p[1]++;
        }*/
        p[1] = 255;
    }

    return result;
}

std::shared_ptr<const CPixelBuffer<double>> CStacker::calibrateImage( std::shared_ptr<const CRawU16Image> rawImage )
{
    auto buffer = std::make_shared<CPixelBuffer<double>>( rawImage->Width(), rawImage->Height() );
    pixels_set( buffer->Pixels(), rawImage->Pixels(), rawImage->Count() );
    return buffer;
}

void CStacker::prepareTwoStacks( const ImageSequence& images, CStacker::Callback* callback )
{
    int n = images.Count();

    std::shared_ptr<const CPixelBuffer<double>> prev;
    for( int i = 0; i < n; i++ ) {
        auto rawImage = images.LoadRawU16( i );
        if( count == 0 ) {
            count = rawImage->Count();
            bitDepth = rawImage->BitDepth();
        } else {
            assert( rawImage->Count() == count );
            assert( rawImage->BitDepth() == bitDepth );
        }
        auto image = calibrateImage( rawImage );
        if( i < n / 2 ) {
            if( stack1 == 0 ) {
                stack1 = std::make_shared<CPixelBuffer<double>>( rawImage->Width(), rawImage->Height() );
                pixels_set( stack1->Pixels(), image->Pixels(), count );
                count1 = 1;
            } else {
                pixels_add( stack1->Pixels(), image->Pixels(), count );
                count1++;
            }
        } else {
            if( stack2 == 0 ) {
                stack2 = std::make_shared<CPixelBuffer<double>>( image->Width(), image->Height() );
                pixels_set( stack2->Pixels(), image->Pixels(), count );
                count2 = 1;
            } else {
                pixels_add( stack2->Pixels(), image->Pixels(), count );
                count2++;
            }
        }
        if( prev && callback ) {
            callback->OnShowImage( correlationGraph( image->Pixels(), prev->Pixels(), count, bitDepth ) );
        }
        prev = image;
    }

    // Bring the values to the range of the orignial images (compatible with their bitDepth)
    pixels_divide_by_value( stack1->Pixels(), count1, count );
    pixels_divide_by_value( stack2->Pixels(), count2, count );

    if( callback ) {
        callback->OnShowImage( correlationGraph( stack1->Pixels(), stack2->Pixels(), count, bitDepth ) );
    }
}

void CStacker::analyzePixels( const CPixelBuffer<double>& buffer )
{
    double mean, sigma, minv, maxv;
    std::tie( mean, sigma, minv, maxv ) = simple_pixel_statistics( buffer.Pixels(), count );
    qDebug() << "Mean" << mean << "Sigma" << sigma << "Min" << minv << "Max" << maxv;
    CHistogram h1 = pixels_histogram_float( buffer.Pixels(), buffer.Count(), bitDepth );
    qDebug() << "Median" << pixels_histogram_median( h1, 0 );
    size_t minM, maxM;
    std::tie( minM, maxM ) = patches_statistics_float( buffer.Pixels(), buffer.Width(), buffer.Height(), bitDepth, 3 );
    qDebug() << "Median3x3" << minM << maxM << ( 1.0 * ( maxM - minM ) ) / ( maxM + minM );
    std::tie( minM, maxM ) = patches_statistics_float( buffer.Pixels(), buffer.Width(), buffer.Height(), bitDepth, 32 );
    qDebug() << "Median32x32" << minM << maxM << ( 1.0 * ( maxM - minM ) ) / ( maxM + minM );
    std::tie( minM, maxM ) = patches_statistics_float( buffer.Pixels(), buffer.Width(), buffer.Height(), bitDepth, 256 );
    qDebug() << "Median256x256" << minM << maxM << ( 1.0 * ( maxM - minM ) ) / ( maxM + minM );
}

std::shared_ptr<CPixelBuffer<double>> CDarksStacker::Process( const ImageSequence& images, Callback* callback )
{
    qDebug() << "=== Processing darks ===";

    prepareTwoStacks( images, callback );

    // Test dark image
    CPixelBuffer<double> testDark( stack1->Width(), stack1->Height() );
    pixels_set( testDark.Pixels(), stack1->Pixels(), count );

    // Normalizing by median
    CHistogram h0 = pixels_histogram_float( testDark.Pixels(), count, bitDepth );
    auto median = pixels_histogram_median( h0, 0 );
    pixels_subtract_value( testDark.Pixels(), median, count );

    // Test light image
    CPixelBuffer<double> testLight( stack2->Width(), stack2->Height() );
    pixels_set( testLight.Pixels(), stack2->Pixels(), count );

    pixels_subtract( testLight.Pixels(), testDark.Pixels(), count );

    // Analyzing result
    analyzePixels( testLight );

    // Final dark frame
    auto final = std::make_shared<CPixelBuffer<double>>( stack1->Width(), stack1->Height() );
    pixels_set_multiply_by_value( final->Pixels(), stack1->Pixels(), ( 1.0 * count1 ) / ( count1 + count2 ), count );
    pixels_add_multiply_by_value( final->Pixels(), stack2->Pixels(), ( 1.0 * count2 ) / ( count1 + count2 ), count );

    analyzePixels( *final );

    return final;
}

std::shared_ptr<const CPixelBuffer<double>> CFlatsStacker::calibrateImage( std::shared_ptr<const CRawU16Image> rawImage )
{
    if( darkFrame ) {
        auto buffer = std::make_shared<CPixelBuffer<double>>( rawImage->Width(), rawImage->Height() );
        pixels_set( buffer->Pixels(), rawImage->Pixels(), rawImage->Count() );
        pixels_subtract( buffer->Pixels(), darkFrame->Pixels(), rawImage->Count() );
        return buffer;
    }
    return CStacker::calibrateImage( rawImage );
}

std::shared_ptr<CPixelBuffer<double>> CFlatsStacker::Process( const ImageSequence& images, Callback* callback )
{
    qDebug() << "=== Processing flats ===";

    prepareTwoStacks( images, callback );

    // Test flat image
    CPixelBuffer<double> testFlat( stack1->Width(), stack1->Height() );
    pixels_set( testFlat.Pixels(), stack1->Pixels(), count );

    // Normalizing by median
    CHistogram h0 = pixels_histogram_float( testFlat.Pixels(), count, bitDepth );
    auto median = pixels_histogram_median( h0, 0 );
    pixels_divide_by_value( testFlat.Pixels(), median, count );

    // Test light image
    CPixelBuffer<double> testLight( stack2->Width(), stack2->Height() );
    pixels_set( testLight.Pixels(), stack2->Pixels(), count );
    pixels_divide( testLight.Pixels(), testFlat.Pixels(), count );

    CPixelBuffer<unsigned short> result( testLight.Width(), testLight.Height() );
    pixels_set_round( result.Pixels(), testLight.Pixels(), count );

    // Analyzing result
    analyzePixels( testLight );

    // Final flat frame
    auto final = std::make_shared<CPixelBuffer<double>>( stack1->Width(), stack1->Height() );
    pixels_set_multiply_by_value( final->Pixels(), stack1->Pixels(), ( 1.0 * count1 ) / ( count1 + count2 ), count );
    pixels_add_multiply_by_value( final->Pixels(), stack2->Pixels(), ( 1.0 * count2 ) / ( count1 + count2 ), count );

    analyzePixels( *final );

    // Normalize by median
    CHistogram h2 = pixels_histogram_float( final->Pixels(), final->Count(), bitDepth );
    auto m = pixels_histogram_median( h2, 0 );
    pixels_divide_by_value( final->Pixels(), m, count );

    return final;
}

std::shared_ptr<const CPixelBuffer<double>> CLightsStacker::calibrateImage( std::shared_ptr<const CRawU16Image> rawImage )
{
    if( darkFrame || flatFrame ) {
        auto buffer = std::make_shared<CPixelBuffer<double>>( rawImage->Width(), rawImage->Height() );
        pixels_set( buffer->Pixels(), rawImage->Pixels(), rawImage->Count() );
        if( darkFrame ) {
            pixels_subtract( buffer->Pixels(), darkFrame->Pixels(), rawImage->Count() );
        } else {
            pixels_subtract_value( buffer->Pixels(), offset, rawImage->Count() );
        }
        if( flatFrame ) {
            pixels_divide( buffer->Pixels(), flatFrame->Pixels(), rawImage->Count() );
        }
        if( darkFrame ) {
            if( offset == 0 ) {
                CHistogram h = pixels_histogram_float( darkFrame->Pixels(), darkFrame->Count(), bitDepth );
                offset = pixels_histogram_median( h, 0 );
            }
        }
        pixels_add_value( buffer->Pixels(), offset, rawImage->Count() );

        return buffer;
    }
    return CStacker::calibrateImage( rawImage );
}

std::shared_ptr<CPixelBuffer<double>> CLightsStacker::Process( const ImageSequence& images, Callback* callback )
{
    qDebug() << "=== Processing lights ===";

    if( darkFrame ) {
        assert( offset == 0.0 );
    }

    prepareTwoStacks( images, callback );

    // Final lights frame
    auto final = std::make_shared<CPixelBuffer<double>>( stack1->Width(), stack1->Height() );
    pixels_set_multiply_by_value( final->Pixels(), stack1->Pixels(), ( 1.0 * count1 ) / ( count1 + count2 ), count );
    pixels_add_multiply_by_value( final->Pixels(), stack2->Pixels(), ( 1.0 * count2 ) / ( count1 + count2 ), count );
    // TO_DO: This helps fighting pasterization in low signal frames (Ha). But better remove the real cause
    //pixels_multiply_by_value( final->Pixels(), 16.0, count );

    return final;
}
