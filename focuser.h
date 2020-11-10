// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef FOCUSER_H
#define FOCUSER_H

#include <QtSerialPort/QSerialPort>

class Focuser : public QObject {
public:
    bool Open();
    void Close();

    void ToggleMotorPower();

private:
    // Focuser (arduino)
    QSerialPort* serial = nullptr;
    bool isOn = false;
    void readSerial();
};

#endif // FOCUSER_H
