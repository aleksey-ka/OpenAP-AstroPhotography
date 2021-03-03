// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef FOCUSER_H
#define FOCUSER_H

#include <QtSerialPort/QSerialPort>

class Focuser : public QObject {
public:
    bool Open();
    void Close();

    void Forward();
    void Backward();

    void StepUp() { if( stepsToGo < 512 ) stepsToGo *= 2; }
    void StepDown() { if( stepsToGo > 1 ) stepsToGo /= 2; }
    int StepsPerMove() const { return stepsToGo; }

    void MarkZero();
    void GoToPos( int );
    int GetPos() const { return focuserPos; }

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
};

#endif // FOCUSER_H
