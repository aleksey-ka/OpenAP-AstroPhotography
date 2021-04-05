// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Camera.h"

#include "Hardware.Camera.ZWO.ASICamera.h"
#include "Hardware.Camera.MockCamera.h"

using namespace Hardware;

int Camera::GetCount()
{
    return ASICamera::GetCount() + MockCamera::GetCount();
}

std::shared_ptr<Hardware::CAMERA_INFO> Camera::GetInfo( int index )
{
    int count = ASICamera::GetCount();
    if( index < count ) {
        return ASICamera::GetInfo( index );
    }
    index -= count;
    return MockCamera::GetInfo( index );
}

std::shared_ptr<Camera> Camera::Open( int id )
{
    if( id >= 0 ) {
        return ASICamera::Open( id );
    }
    return MockCamera::Open( id );
}
