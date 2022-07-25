// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include "Hardware.Focuser.h"
#include <QtSerialPort/QSerialPort>

class DIYFocuser : public QObject, public Hardware::Focuser {
public:
    static std::shared_ptr<DIYFocuser> Open();
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
    // Focuser (arduino)
    QSerialPort* serial = nullptr;
    int stepsToGo = 128;
    mutable int targetPos = INT_MIN;
    mutable int focuserPos = INT_MIN;
    bool isInsideMoveTo() const { return targetPos != INT_MIN; }
    void cancelMoveTo() const { targetPos = INT_MIN; }
    void writeToSerial( const QString& ) const;
    void readSerial() const;

    bool connect( const QString& portName );
};
