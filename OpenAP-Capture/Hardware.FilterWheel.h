// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <memory>

namespace Hardware {

class FilterWheel {
public:
    static std::shared_ptr<FilterWheel> Open();

    virtual void Close() = 0;

    virtual int GetSlotsCount() const = 0;

    virtual int GetPosition() = 0;
    virtual void SetPosition( int ) = 0;

    virtual ~FilterWheel() {}
};

} // namespace Hardware
