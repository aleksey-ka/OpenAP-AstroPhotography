// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "focuser.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>

bool Focuser::Open()
{
    QString portName;
    foreach( const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts() ) {
        if( serialPortInfo.description() == "USB-SERIAL CH340" ) {
            portName = serialPortInfo.portName();
            break;
        }
    }
    if( portName.length() > 0 ) {
        serial = new QSerialPort( this );
        serial->setPortName( portName );
        serial->setBaudRate( QSerialPort::Baud9600 );
        serial->setDataBits( QSerialPort::Data8 );
        serial->setParity( QSerialPort::NoParity );
        serial->setStopBits( QSerialPort::OneStop );
        serial->setFlowControl( QSerialPort::NoFlowControl) ;
        serial->open( QIODevice::ReadWrite );
        assert( serial->isOpen() );

        QObject::connect( serial, &QSerialPort::readyRead, this, &Focuser::readSerial ) ;

        qDebug() << "Connected to " << portName;
        return true;
    }
    return false;
}

void Focuser::Close()
{
    if( isOn ) {
        ToggleMotorPower();
    }
    if( serial != nullptr && serial->isOpen() ) {
        serial->close();
        delete serial;
    }
}

void Focuser::ToggleMotorPower()
{
    assert( serial->isWritable() );
    serial->write( isOn ? "OFF\n" : "ON\n" );
    isOn = !isOn;
    serial->waitForBytesWritten( 5000) ;
}

void Focuser::readSerial()
{
    QByteArray data = serial->readAll();
    qDebug() << QString::fromStdString( data.toStdString() );
}
