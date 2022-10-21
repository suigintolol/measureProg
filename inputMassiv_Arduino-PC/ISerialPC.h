#ifndef ISERIALPC_H
#define ISERIALPC_H
#include "DataTransmissionPC.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QByteArray>
//#include <QString>

typedef char byte;

class ISerialPC : public ISerial {
    QSerialPort* initializationArduinoDevice();
public:

    //QSerialPort* arduino = new QSerialPort;
    QSerialPort* arduino = initializationArduinoDevice();

    //ISerialPC();

    byte iRead() override;

    void iWrite(const byte* val) override;

    //void iWrite(const byte val) override;

    //void iWrite(byte val);

    void iWrite(byte* ol_buffer, unsigned int ol_cnt) override;

    size_t iAvailable() override;

    void iPrintConsol(QString ol_buffer) override;

    ~ISerialPC() override;

    QByteArray inputArdPc(QSerialPort *arduinoi);
};



#endif // ISERIALPC_H
