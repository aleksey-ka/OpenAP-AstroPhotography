// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Focuser.h"

#include "Hardware.Focuser.ZWO.EAFocuser.h"
#include "Hardware.Focuser.DIYFocuser.h"

using namespace Hardware;

std::shared_ptr<Focuser> Focuser::Open()
{
    std::shared_ptr<Focuser> focuser = ZWOFocuser::Open();
    if( focuser == 0 ) {
        focuser = DIYFocuser::Open();
    }
    return focuser;
}
