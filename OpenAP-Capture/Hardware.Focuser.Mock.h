// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include "Hardware.Focuser.h"

#include <climits>

class MockFocuser : public Hardware::Focuser {
public:
    static std::shared_ptr<MockFocuser> Open();
    virtual void Close() override {}

    virtual void Forward() override;
    virtual void Backward() override;

    virtual void StepUp() override { if( stepsToGo < 512 ) stepsToGo *= 2; }
    virtual void StepDown() override { if( stepsToGo > 1 ) stepsToGo /= 2; }
    virtual int StepsPerMove() const override { return stepsToGo; }

    virtual void MarkZero() override;
    virtual void MoveZero( int ) override;
    virtual void GoToPos( int ) override;
    virtual int GetPos() const override { return focuserPos; }

    virtual int PrevPos() const override { return prevPos; }

private:
    int stepsToGo = 128;
    mutable int focuserPos = 0;
    mutable int prevPos = 0;
};
