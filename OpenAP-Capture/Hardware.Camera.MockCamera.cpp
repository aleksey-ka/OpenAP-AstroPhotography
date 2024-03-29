// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Camera.MockCamera.h"

#include <QThread>
#include <QDir>
#include <QTextStream>
#include <QFileInfo>

#include <iostream>
#include <fstream>

static QStringList rootFileEntries;
static QStringList rootDescription;

static std::shared_ptr<const CRawU16Image> loadImage( QString filePath )
{
    return CRawU16Image::LoadFromFile( filePath.toLocal8Bit().constData()  );
}

static std::shared_ptr<const CRawU16Image> loadImage( QString filePath, QStringList& frames )
{
    QFileInfo fileInfo( filePath );
    if( fileInfo.isDir() ) {
        frames = QDir( filePath, "*.u16.pixels" ).entryList( QDir::Files );
        return loadImage( filePath + QDir::separator() + frames[0] );
    } else {
        return loadImage( filePath.left( filePath.lastIndexOf( '.' ) ) + ".pixels" );
    }
}

static std::shared_ptr<const CRawU16Image> loadImage( int index, QStringList& frames )
{
    return loadImage( rootFileEntries[index], frames );
}

static std::shared_ptr<Hardware::CAMERA_INFO> createCameraInfo( const CRawU16Image* image, std::string name, int id )
{
    auto cameraInfo = std::make_shared<Hardware::CAMERA_INFO>();
    strcpy( cameraInfo->Name, name.c_str() );
    cameraInfo->IsColorCamera = image->Info().CFA.empty();
    cameraInfo->Id = id;
    return cameraInfo;
}

static std::shared_ptr<Hardware::CAMERA_INFO> createCameraInfo( int index )
{
    auto cameraInfo = std::make_shared<Hardware::CAMERA_INFO>();
    QStringList frames;
    std::shared_ptr<const CRawU16Image> image = loadImage( index, frames );
    std::string name( image->Info().Camera );
    name += rootDescription[index].toUtf8().constData();
    return createCameraInfo( image.get(), name, -( index + 1 ) );
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
              if( desc == "*" ) {
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
    path = rootFileEntries[index].toUtf8().constData();
    currentSettings = loadImage( index, frames )->Info();
    cameraInfo = createCameraInfo( index );
}

MockCamera::MockCamera( const char* _path )
{
    index = INT_MAX;
    path = QString::fromUtf8( _path );
    std::shared_ptr<const CRawU16Image> image = loadImage( _path, frames );
    currentSettings = image->Info();
    cameraInfo = createCameraInfo( image.get(),
        QFileInfo( path ).fileName().toStdString(), index );
}


std::shared_ptr<MockCamera> MockCamera::Open( const Hardware::CAMERA_INFO& cameraInfo )
{
    return std::make_shared<MockCamera>( cameraInfo.Id );
}

std::shared_ptr<MockCamera> MockCamera::Open( const char* path )
{
    return std::make_shared<MockCamera>( path );
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

void MockCamera::GetROIFormat( int& width, int& height, int& bin, Hardware::IMAGE_TYPE& imgType ) const
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
        image = CRawU16Image::LoadFromFile( ( path + QDir::separator() + frames[nextFrame] ).toLocal8Bit().constData() );
        if( nextFrame == 0 ) {
            const_cast<ImageInfo&>( image->Info() ).Flags |= IF_SERIES_START;
        }
        nextFrame += forwardPass ? 1 : -1;
        if( nextFrame == frames.size() ) {
            forwardPass = false;
            nextFrame = frames.size() - 2;
            if( index == INT_MAX ) {
                const_cast<ImageInfo&>( image->Info() ).Flags |= IF_SERIES_END;
                forwardPass = true;
                nextFrame = 0;
            }
        } else if ( nextFrame == -1 ) {
            forwardPass = true;
            nextFrame = 1;
        }
    } else {
        QStringList empty;
        image = loadImage( index, empty );
    }

    if( index != INT_MAX ) {
        auto& imageInfo = const_cast<ImageInfo&>( image->Info() );
        std::string filePath = imageInfo.FilePath;
        imageInfo = currentSettings;
        imageInfo.FilePath = filePath;
        imageInfo.SeriesId = templateImageInfo.SeriesId;
        imageInfo.Channel = templateImageInfo.Channel;
        imageInfo.FilterDescription = templateImageInfo.FilterDescription;

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::system_clock::to_time_t( now );
        imageInfo.Timestamp = timestamp;
    }

    return image;
}
