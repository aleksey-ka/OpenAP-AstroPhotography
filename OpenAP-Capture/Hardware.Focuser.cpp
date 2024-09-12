// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Focuser.h"

#include "Hardware.Focuser.ZWO.EAF.h"
#include "Hardware.Focuser.DIY.ArduinoFocuser.h"
#include "Hardware.Focuser.Mock.h"

using namespace Hardware;

std::shared_ptr<Focuser> Focuser::Open()
{
    std::shared_ptr<Focuser> focuser = ZWOFocuser::Open();
    if( focuser == 0 ) {
        focuser = DIYFocuser::Open();
#ifdef QT_DEBUG
        if( focuser == 0 ) {
            focuser = MockFocuser::Open();
        }
#endif
    }
    return focuser;
}
