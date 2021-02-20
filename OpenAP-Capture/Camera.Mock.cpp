// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Camera.Mock.h"

#include <QThread>
#include <QDir>

#include <string.h>
#include <iostream>
#include <fstream>

static QStringList files;

static std::shared_ptr<const CRawU16Image> loadImage( int index )
{
    return CRawU16Image::LoadFromFile( files[index].toStdString().c_str() );
}

static std::shared_ptr<ASI_CAMERA_INFO> createCameraInfo( int index )
{
    auto cameraInfo = std::make_shared<ASI_CAMERA_INFO>();
    auto image = loadImage( index );
    strcpy( cameraInfo->Name, image->Info().Camera.c_str() );
    cameraInfo->IsColorCam = image->Info().CFA.empty() ? ASI_FALSE : ASI_TRUE;
    cameraInfo->CameraID = -( index + 1 );
    return cameraInfo;
}

int MockCamera::GetCount()
{
    files = QDir( "", "*.u16.pixels" ).entryList( QDir::Files );
    return files.size();
}

std::shared_ptr<ASI_CAMERA_INFO> MockCamera::GetInfo( int index )
{
    return createCameraInfo( index );
}

MockCamera::MockCamera( int id )
{
    index = -id - 1;
    currentSettings = loadImage( index )->Info();
}

std::shared_ptr<MockCamera> MockCamera::Open( int id )
{
    return std::make_shared<MockCamera>( id );
}

void MockCamera::Close()
{
    isClosing = true;
    while( isExposure );
}

std::shared_ptr<ASI_CAMERA_INFO> MockCamera::GetInfo() const
{
    return createCameraInfo( index );
}

void MockCamera::GetROIFormat( int& width, int& height, int& bin, ASI_IMG_TYPE& imgType ) const
{
    width = currentSettings.Width;
    height = currentSettings.Height;
    bin = 1;
    imgType = ASI_IMG_RAW16;
}

std::shared_ptr<const CRawU16Image> MockCamera::DoExposure() const
{
    isExposure = true;
    if( isClosing ) {
        isExposure = false;
        return 0;
    }

    int chunks = currentSettings.Exposure / 100000;
    int delta = currentSettings.Exposure % 100000;
    for( int i = 0; i < chunks; i++ ) {
        QThread::usleep( 100000 );
        if( isClosing ) {
            isExposure = false;
            return 0;
        }
    }
    QThread::usleep( delta );
    isExposure = false;
    auto image = loadImage( index );

    auto& imageInfo = const_cast<ImageInfo&>( image->Info() );
    imageInfo = currentSettings;
	imageInfo.SeriesId = templateImageInfo.SeriesId;
    imageInfo.Channel = templateImageInfo.Channel;
    imageInfo.FilterDescription = templateImageInfo.FilterDescription;
    return image;
}
