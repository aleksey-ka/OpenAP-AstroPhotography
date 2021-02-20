// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_QT_H
#define IMAGE_QT_H

#include <QImage>
#include <QPixmap>

#include "Image.Formats.h"
#include "Image.Math.h"

namespace Qt {

inline QImage CreateImage( const CRgbImage* image )
{
    return QImage( image->RgbPixels(), image->Width(), image->Height(), image->ByteWidth(), QImage::Format_RGB888 );
}

inline QPixmap CreatePixmap( const CRgbImage* image )
{
    return QPixmap::fromImage( CreateImage( image ) );
}

inline QPixmap CreatePixmap( std::shared_ptr<const CRgbImage> image )
{
    return CreatePixmap( image.get() );
}

inline QImage CreateImage( const CGrayImage* image )
{
    return QImage( image->Pixels(), image->Width(), image->Height(), image->ByteWidth(), QImage::Format_Grayscale8 );
}

inline QPixmap CreatePixmap( const CGrayImage* image )
{
    return QPixmap::fromImage( CreateImage( image ) );
}

inline QPixmap CreatePixmap( std::shared_ptr<const CGrayImage> image )
{
    return CreatePixmap( image.get() );
}

inline QImage CreateImage( const CRawU16Image* image )
{
    return QImage( image->Buffer(), image->Width(), image->Height(), sizeof( ushort ) * image->Width(), QImage::Format_Grayscale16 );
}

} // namespace Qt

#endif // IMAGE_QT_H
