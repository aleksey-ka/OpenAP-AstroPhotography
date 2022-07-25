// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <memory>

namespace Hardware {

class Focuser {
public:
    static std::shared_ptr<Focuser> Open();

    virtual void Close() = 0;

    virtual void Forward() = 0;
    virtual void Backward() = 0;

    virtual void StepUp() = 0;
    virtual void StepDown() = 0;
    virtual int StepsPerMove() const = 0;

    virtual void MarkZero() = 0;
    virtual void GoToPos( int ) = 0;
    virtual int GetPos() const = 0;

    virtual ~Focuser() {}
};

} // namespace Hardware
