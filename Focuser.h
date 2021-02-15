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

    void MarkZero();
    void GoToPos( int );

private:
    // Focuser (arduino)
    QSerialPort* serial = nullptr;
    int stepsToGo = 128;
    int targetPos = INT_MIN;
    bool isInsideMoveTo() const { return targetPos != INT_MIN; }
    void cancelMoveTo() { targetPos = INT_MIN; }
    void writeToSerial( const QString& );
    void readSerial();
};

#endif // FOCUSER_H
