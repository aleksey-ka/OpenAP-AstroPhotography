// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "mockcamera.h"

#include <QThread>

#include <string.h>
#include <iostream>
#include <fstream>

static std::shared_ptr<ASI_CAMERA_INFO> createCameraInfo()
{
    auto cameraInfo = std::make_shared<ASI_CAMERA_INFO>();
    strcpy( cameraInfo->Name, "MOCK" );
    cameraInfo->CameraID = -1;
    return cameraInfo;
}

int MockCamera::GetCount()
{
    std::ifstream file( "image.cfa" );
    return file ? 1 : 0;
}

std::shared_ptr<ASI_CAMERA_INFO> MockCamera::GetInfo( int index )
{
    return createCameraInfo();
}

MockCamera::MockCamera()
{
    image = Raw16Image::LoadFromFile( "image.cfa" );
    info = image->Info();
}

std::shared_ptr<MockCamera> MockCamera::Open( int id )
{
    return std::make_shared<MockCamera>();
}

std::shared_ptr<ASI_CAMERA_INFO> MockCamera::GetInfo() const
{
    return createCameraInfo();
}

std::shared_ptr<const Raw16Image> MockCamera::DoExposure() const
{
    QThread::usleep( info.Exposure );
    return Raw16Image::LoadFromFile( "image.cfa" );
}
