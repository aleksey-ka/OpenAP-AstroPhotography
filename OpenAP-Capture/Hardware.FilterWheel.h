// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef HARDWARE_FILTERWHEEL_H
#define HARDWARE_FILTERWHEEL_H

#include <memory>

class FilterWheel {
public:
    virtual void Close() = 0;

    virtual int GetSlotsCount() const = 0;

    virtual int GetPosition() = 0;
    virtual void SetPosition( int ) = 0;

    virtual ~FilterWheel() {}
};

#endif // HARDWARE_FILTERWHEEL_H
