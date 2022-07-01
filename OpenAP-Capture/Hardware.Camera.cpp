// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Camera.h"

#include "Hardware.Camera.ZWO.ASICamera.h"
#include "Hardware.Camera.QHY.QHYCamera.h"
#include "Hardware.Camera.MockCamera.h"

using namespace Hardware;

int Camera::GetCount()
{
    return ASICamera::GetCount() + QHYCamera::GetCount() + MockCamera::GetCount();
}

std::shared_ptr<Hardware::CAMERA_INFO> Camera::GetInfo( int index )
{
    int count = ASICamera::GetCount();
    if( index < count ) {
        return ASICamera::GetInfo( index );
    }
    index -= count;
    count = QHYCamera::GetCount();
    if( index < count ) {
        return QHYCamera::GetInfo( index );
    }
    index -= count;
    return MockCamera::GetInfo( index );
}

std::shared_ptr<Camera> Camera::Open( const Hardware::CAMERA_INFO& cameraInfo )
{
    if( cameraInfo.Id >= 0 ) {
        return ASICamera::Open( cameraInfo );
    } else if( cameraInfo.GUID[0] != 0 ) {
        return QHYCamera::Open( cameraInfo );
    }
    return MockCamera::Open( cameraInfo );
}
