// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef FOCUSER_ZWO_H
#define FOCUSER_ZWO_H

#include <memory>
#include <limits.h>

#include "Focuser.h"

class ZWOFocuser : public Focuser {
public:
    static std::shared_ptr<Focuser> Open();
    virtual void Close() override;

    virtual void Forward() override;
    virtual void Backward() override;

    virtual void StepUp() override { if( stepsToGo < 512 ) stepsToGo *= 2; }
    virtual void StepDown() override { if( stepsToGo > 1 ) stepsToGo /= 2; }
    virtual int StepsPerMove() const override { return stepsToGo; }

    virtual void MarkZero() override;
    virtual void GoToPos( int ) override;
    virtual int GetPos() const override { return focuserPos; }

private:
    int id;
    int stepsToGo = 128;
    mutable int targetPos = INT_MIN;
    mutable int focuserPos = INT_MIN;
    void syncMoveTo( int pos );
    bool isInsideMoveTo() const;
    void cancelMoveTo() const;

    bool connect( int id );
};

#endif // FOCUSER_ZWO_H
