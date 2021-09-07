// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Camera.MockCamera.h"

#include <QThread>
#include <QDir>
#include <QTextStream>

#include <string.h>
#include <iostream>
#include <fstream>

static QStringList rootFileEntries;
static QStringList rootDescription;

static std::shared_ptr<const CRawU16Image> loadImage( QString filePath )
{
    return CRawU16Image::LoadFromFile( filePath.toLocal8Bit().constData()  );
}

static std::shared_ptr<const CRawU16Image> loadImage( int index, QStringList& frames )
{
    auto filePath = rootFileEntries[index];
    QFileInfo fileInfo( filePath );
    if( fileInfo.isDir() ) {
        frames = QDir( filePath, "*.u16.pixels" ).entryList( QDir::Files );
        return loadImage( filePath + QDir::separator() + frames[0] );
    } else {
        return loadImage( filePath.left( filePath.lastIndexOf( '.' ) ) + ".pixels" );
    }
}

static std::shared_ptr<Hardware::CAMERA_INFO> createCameraInfo( int index )
{
    auto cameraInfo = std::make_shared<Hardware::CAMERA_INFO>();
    QStringList frames;
    std::shared_ptr<const CRawU16Image> image = loadImage( index, frames );
    std::string name( image->Info().Camera );
    name += rootDescription[index].toLocal8Bit().constData();
    strcpy( cameraInfo->Name, name.c_str() );
    cameraInfo->IsColorCamera = image->Info().CFA.empty();
    cameraInfo->Id = -( index + 1 );
    return cameraInfo;
}

int MockCamera::GetCount()
{
    if( QFileInfo::exists( "Debug.MockCamera.txt" ) ) {
        QFile inputFile( "Debug.MockCamera.txt" );
        if( inputFile.open( QIODevice::ReadOnly ) ) {
           QTextStream in( &inputFile );
           while( !in.atEnd() ) {
              auto desc = in.readLine();
              auto path = in.readLine();
              if( desc == L"*" ) {
                  auto entries = QDir( path, "*.u16.info" ).entryList( QDir::AllEntries );
                  for( int i = 0; i < entries.size(); i++ ) {
                      rootFileEntries.append( path + QDir::separator() + entries[i] );
                      rootDescription.append( "" );
                  }
              } else if( QFileInfo::exists( path ) ) {
                  rootDescription.append( desc );
                  rootFileEntries.append( path );
              }
           }
           inputFile.close();
        }
    }
    return rootFileEntries.size();
}

std::shared_ptr<Hardware::CAMERA_INFO> MockCamera::GetInfo( int index )
{
    return createCameraInfo( index );
}

MockCamera::MockCamera( int id )
{
    index = -id - 1;
    currentSettings = loadImage( index, frames )->Info();
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

std::shared_ptr<Hardware::CAMERA_INFO> MockCamera::GetInfo() const
{
    return createCameraInfo( index );
}

void MockCamera::GetROIFormat( int& width, int& height, int& bin, Hardware::IMG_TYPE& imgType ) const
{
    width = currentSettings.Width;
    height = currentSettings.Height;
    bin = 1;
    imgType = Hardware::IT_RAW16;
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

    std::shared_ptr<const CRawU16Image> image;
    if( frames.size() > 0 ) {
        image = CRawU16Image::LoadFromFile( ( rootFileEntries[index] + QDir::separator() + frames[nextFrame] ).toLocal8Bit().constData() );
        nextFrame += forwardPass ? 1 : -1;
        if( nextFrame == frames.size() ) {
            forwardPass = false;
            nextFrame = frames.size() - 2;
        } else if ( nextFrame == -1 ) {
            forwardPass = true;
            nextFrame = 1;
        }
    } else {
        QStringList empty;
        image = loadImage( index, empty );
    }

    auto& imageInfo = const_cast<ImageInfo&>( image->Info() );
    imageInfo = currentSettings;
	imageInfo.SeriesId = templateImageInfo.SeriesId;
    imageInfo.Channel = templateImageInfo.Channel;
    imageInfo.FilterDescription = templateImageInfo.FilterDescription;

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t( now );
    imageInfo.Timestamp = timestamp;

    return image;
}
