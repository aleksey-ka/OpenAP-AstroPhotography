// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef HARDWARE_FILTERWHEEL_EFWHEEL_H
#define HARDWARE_FILTERWHEEL_EFWHEEL_H

#include "Hardware.FilterWheel.h"

class EFWheel : public FilterWheel {
public:
    static std::shared_ptr<EFWheel> Open();
    virtual void Close() override;

    virtual int GetSlotsCount() const override { return slotsCount; };

    virtual int GetPosition() override;
    virtual void SetPosition( int ) override;

private:
    int id;
    int slotsCount = 0;
    bool open();
};

#endif // HARDWARE_FILTERWHEEL_EFWHEEL_H
