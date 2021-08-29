// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Image.Formats.h"
#include "Image.Qt.h"

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
    Qt::CreateImage( image ).save( filePath, 0, 80 );
}
