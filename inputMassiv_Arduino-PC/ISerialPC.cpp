#include "ISerialPC.h"

/*ISerialPC::ISerialPC() {
    //arduino = initializationArduinoDevice();
}*/

byte ISerialPC::iRead() {
    QByteArray QBAtest = arduino->read(1);
    return QBAtest[1];
}

void ISerialPC::iWrite(const byte* val) {
    //Serial.write(val);
    //arduino->write(22);

    if(arduino->isWritable()) {
        arduino->write(val);
        arduino->flush();
        arduino->waitForBytesWritten(50);
        arduino->waitForReadyRead(50);
        //QByteArray();
    }
}
/*
void ISerialPC::iWrite(const byte val) {
    //Serial.write(val);
    //arduino->write(22);
    if(arduino->isWritable()) {
        arduino->write(&val);
        arduino->flush();
        arduino->waitForBytesWritten(50);
        arduino->waitForReadyRead(5);
        //arduino->wr
        //QByteArray();
    }
}
*/
/*
void ISerialPC::iWrite(byte val) {
    //Serial.write(val);
    //arduino->write(22);
    if(arduino->isWritable()) {
        arduino->write(&val);
        //arduino->wr
        //QByteArray();
    }
}*/

void ISerialPC::iWrite(byte* ol_buffer, unsigned int ol_cnt) {
    //Serial.write(ol_buffer, ol_cnt);
    bool checkWriteble = arduino->isWritable();
    if(checkWriteble) {
        //QByteArray QBAi = QByteArray(ol_buffer, 1);
        //arduino->write(QBAi, ol_cnt);
        //arduino->waitForReadyRead(10);

        int chekSum = arduino->write(ol_buffer, ol_cnt);
        arduino->flush();
        arduino->waitForBytesWritten(50);
        arduino->waitForReadyRead(50);
        if(chekSum == -1) {
            qDebug() << " ERR iWrite\n";
        }
    }
}

size_t ISerialPC::iAvailable() {
    return arduino->bytesAvailable();
}

/*void iPrintConsol(byte* ol_buffer, const unsigned int ol_cnt = 2) {
  Serial.print(ol_buffer, 2);
}*/

void ISerialPC::iPrintConsol(QString ol_buffer) {
    //Serial.print(ol_buffer);
    qDebug() << ol_buffer;
}

ISerialPC::~ISerialPC() {
    //Serial.end();
    arduino->close();
    delete arduino;
}

QByteArray ISerialPC::inputArdPc(QSerialPort *arduinoi) {
    QByteArray QBAtest = "";
    if(arduinoi->isReadable()) {
        arduino->flush();
        arduino->waitForBytesWritten(50);
        arduino->waitForReadyRead(50);
        int bufSize = 0;
        //while(arduinoi->bytesAvailable() || arduinoi->waitForReadyRead(5)) { // old version
        while(arduinoi->bytesAvailable()) {
            qint64 intege = arduinoi->bytesAvailable();
            bufSize += static_cast<int>(intege);
            // outputFile << "Buf i size: " <<bufSize << endl;
            QBAtest += arduinoi->readAll();
            arduino->flush();
            arduino->waitForBytesWritten(50);
            arduino->waitForReadyRead(50);
            //outputFile << QBAtest.toInt() << endl;
            //outputFile << QBAtest.toStdString() << endl;
        }
        qDebug() << "Buf i size: " << bufSize;
        //outputFile << "final Buf i size: " <<bufSize << endl;
        qDebug() << "Set i " << QBAtest ;
        // outputFile << "final " << QBAtest.toInt() << endl;
        //arduinoi->flush();
        return QBAtest;
        //cout << QBAtest << endl;
    }
    qDebug() << " ------------ ";
    return QBAtest;
}

QSerialPort* ISerialPC::initializationArduinoDevice() {
    QSerialPort *arduino = new QSerialPort;
    static const quint16 arduino_due_vendor_id = 9025;
    static const quint16 ttl_vendor_id = 1659;
    static const quint16 ttl_product_id = 8963;
    static const quint16 arduino_due_product_idProgram = 61;
    static const quint16 arduino_due_product_idNative = 62;

    QString arduino_port_name = "";
    bool arduino_is_available = false;

    //**************************   Conect   ****************************
    qDebug() << "Number of available ports: " << QSerialPortInfo::availablePorts().length();
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        qDebug()<< endl << "Has vendor ID: " << serialPortInfo.hasVendorIdentifier();
        if(serialPortInfo.hasVendorIdentifier()) {
            qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier();
        }
        qDebug() << "Has Product ID: " << serialPortInfo.hasProductIdentifier();
        if(serialPortInfo.hasProductIdentifier()) {
            qDebug() << "Product ID: " << serialPortInfo.productIdentifier();
        }
    }

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()) {
            if((serialPortInfo.vendorIdentifier() == arduino_due_vendor_id) || (serialPortInfo.vendorIdentifier() == ttl_vendor_id)) {
                if((serialPortInfo.productIdentifier() == arduino_due_product_idProgram) ||
                        (serialPortInfo.productIdentifier() == arduino_due_product_idNative) ||
                        (serialPortInfo.productIdentifier() == ttl_product_id)) {
                    arduino_port_name = serialPortInfo.portName();
                    arduino_is_available = true;
                    qDebug() << "Conect: yes" ;
                }
            }
        }
    }

    if(arduino_is_available) {
        // open and configure the serialport
        arduino->setPortName(arduino_port_name);
        arduino->open(QSerialPort::ReadWrite);
        //arduino->setBaudRate(QSerialPort::Baud9600);
        arduino->setBaudRate(QSerialPort::Baud115200);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        qDebug() << "Inicialization: yes" ;
        return arduino;
    } else {
        // give error Mes if not available
        //хорошо бы потом обрабатывать это
        qDebug() << "Port error, Couldn't find the Arduino!";
        return NULL;
    }
    //**************************   end Conect   ****************************
}
