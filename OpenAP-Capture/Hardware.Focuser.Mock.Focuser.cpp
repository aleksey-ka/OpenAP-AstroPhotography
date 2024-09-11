// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Focuser.Mock.Focuser.h"

#include <thread>

std::shared_ptr<MockFocuser> MockFocuser::Open()
{
    return std::make_shared<MockFocuser>();
}

void MockFocuser::Forward()
{
    prevPos = focuserPos;
    focuserPos += stepsToGo;
}

void MockFocuser::Backward()
{
    prevPos = focuserPos;
    focuserPos -= stepsToGo;
}

void MockFocuser::MarkZero()
{
    focuserPos = 0;
    prevPos = 0;
}

void MockFocuser::MoveZero( int steps )
{
    focuserPos -= steps;
    prevPos -= steps;
}

void MockFocuser::GoToPos( int pos )
{
    prevPos = focuserPos;
    focuserPos = pos;
}
