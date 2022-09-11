// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Formats.h"
#include "Image.Qt.h"

#include <QFile>
#include <QTextStream>

std::shared_ptr<CRawU16Image> Png16BitGrayscale::Load( const char* filePath, const ImageInfo& imageInfo ) const
{
    auto result = std::make_shared<CRawU16Image>( imageInfo );

    // Reading png is relatively slow: ~ 0.8 sec for compressed and ~0.4 for uncompressed data
    QImage greyScaleImage;
    greyScaleImage.load( filePath );

    uchar* buf = result->Buffer();
    int byteWidth = imageInfo.Width * 2;
    for( int i = 0; i < imageInfo.Height; i++ ) {
        const uchar* src = greyScaleImage.constScanLine( i );
        uchar* dst = buf + i * byteWidth;
        memcpy( dst, src, byteWidth );
    }

    return result;
}

void Png16BitGrayscale::Save( const char* filePath, const CRawU16Image* image ) const
{
    // Saving PNG files is extremely slow especially with high zlib compression rates. High compression rates are not
    // practical as the effect is very slight on astronomical images and the speed differs greatly
    // - (default) ~ 6 sec
    // - (0 - highest compression) ~ 8 sec
    // - (80..89 - fastest) ~ 0.7 sec
    // - (90..100 - no compression) ~ 0.4 sec
    // NB: writing raw pixels takes only 0.02 sec
    auto qtImage = Qt::CreateImage( image );
    if( !qtImage.save( QString::fromLocal8Bit( filePath ), 0, 80 ) ) {
        assert( false );
    }
}

std::shared_ptr<CRawU16Image> FitsU16::Load( const char*, const ImageInfo& ) const
{
    assert( false );
    return 0;
}

void FitsU16::Save( const char* filePath, const CRawU16Image* image ) const
{
    // 2880 = 36 lines * 80 chars
    QString card;
    int linesCount = 0;
    card += "SIMPLE  =                    T                                                  ";linesCount++;
    card += "BITPIX  =                   16                                                  ";linesCount++;
    card += "NAXIS   =                    2                                                  ";linesCount++;
    card += QString( "NAXIS1  =%1                                                  " )
            .arg( image->Width(), 21 );linesCount++;
    card += QString( "NAXIS2  =%1                                                  " )
            .arg( image->Height(), 21 );linesCount++;
    card += "BZERO   =              32768.0                                                  ";linesCount++;
    card += "BSCALE  =                  1.0                                                  ";linesCount++;
    if( image->Info().CFA.length() > 0 ) {
        assert( image->Info().CFA.length() == 4 );
        card += QString( "CFAIMAGE=           '%1    '                                                  " )
            .arg( image->Info().CFA.c_str() );linesCount++;
    }
    card += QString( "EXPTIME =%1                                                  " )
            .arg( image->Info().Exposure / 1000000.0, 21, 'f', -1 );linesCount++;
    card += QString( "GAIN    =%1                                                  " )
            .arg( image->Info().Gain, 21 );linesCount++;
    card += "END                                                                             ";linesCount++;

    for( int i = linesCount; i < 36; i++ ) {
        card += QString( 80, ' ' );
    }

    QFile file( QString::fromLocal8Bit( filePath ) );
    assert( file.open( QIODevice::WriteOnly ) );
    {
        QTextStream stream( &file );
        stream << card;
    }
    {
        QDataStream stream( &file );
        const unsigned short* pixels = image->Pixels();
        for( size_t i = 0; i < image->Count(); i++ ) {
            int v = pixels[i];
            v -= 32768;
            stream << (unsigned char)( v >> 8 );
            stream << (unsigned char)( v );
        }
    }
}
