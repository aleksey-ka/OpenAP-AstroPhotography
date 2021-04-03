// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.FilterWheel.h"

#include "Hardware.FilterWheel.ZWO.EFWheel.h"

using namespace Hardware;

std::shared_ptr<FilterWheel> FilterWheel::Open()
{
    return EFWheel::Open();
}
