// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "image.formats.h"

#include <QImage>

std::shared_ptr<const Raw16Image> Png16BitGrayscale::Load( const char* filePath, const ImageInfo& imageInfo ) const
{
    auto result = std::make_shared<Raw16Image>( imageInfo );

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

void Png16BitGrayscale::Save( const char* filePath, const Raw16Image* image ) const
{
    auto imageInfo = image->Info();
    QImage greyScaleImage( image->Buffer(), imageInfo.Width, imageInfo.Height, QImage::Format_Grayscale16 );

    // Saving PNG files is extremely slow especially with high zlib compression rates. High compression rates are not
    // practical as the effect is very slight on astronomical images and the speed differs greatly
    // - (default) ~ 6 sec
    // - (0 - highest compression) ~ 8 sec
    // - (80..89 - fastest) ~ 0.7 sec
    // - (90..100 - no compression) ~ 0.4 sec
    // NB: writing raw pixels takes only 0.02 sec
    greyScaleImage.save( filePath, 0, 80 );
}
