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

    void MultiplyByTwo() { if( stepsToGo < 512 ) stepsToGo *= 2; }
    void DevideByTwo() { if( stepsToGo > 1 ) stepsToGo /= 2; }

private:
    // Focuser (arduino)
    QSerialPort* serial = nullptr;
    int stepsToGo = 128;
    void writeToSerial( const QString& );
    void readSerial();
};

#endif // FOCUSER_H
