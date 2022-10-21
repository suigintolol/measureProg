#include <QCoreApplication>
#include <iostream>

//qt mingv5.12 32x
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <fstream>
#include "ISerialPC.h"

using namespace std;

// в версии ПК используем typedef char, называем их byte. радуемся
// везде char - радость и лад в семье





int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    ISerialPC *iArduino = new ISerialPC;
    DataTransmissionPC* dateTrans = new DataTransmissionPC(iArduino);

    QTime time;
    time.start();
    for(;time.elapsed() < 3000;){}

    QByteArray QBAi = QByteArray::number(-125);
    //QByteArray QBAi2 = 1;
    byte* messOutputMas = "-125";
    //messOutputMas[0] = 50;
    byte messOutput = -125;
    //cout << messOutputMas;
    cout << "\nYour comand\n";

    //dateTrans->iSerial->iWrite(messOutput);
    //iArduino->arduino->waitForReadyRead(500);
    //dateTrans->testF();
    //iArduino->arduino->open();

    dateTrans->formingDataPacket(CONNECT);
    while(messOutput != 111) {
        //dateTrans->iSerial->iWrite(messOutputMas);
        //dateTrans->iSerial->iWrite(messOutput);

        if(iArduino->arduino->isWritable()){
            dateTrans->sendPacket(true);

            //iArduino->arduino->write(QBAi);
            //iArduino->iWrite("kek");
            //iArduino->iWrite(messOutputMas);

            /*iArduino->arduino->flush();
            iArduino->arduino->waitForBytesWritten(50);
            iArduino->arduino->waitForReadyRead(50);*/
            cout << "\n print \n";

            time.start();
            for(;time.elapsed() < 1000;){}

            //if(dateTrans)
        }

        messOutput = 111;
        //iArduino->arduino->writeData(messOutputMas, 3);

    }

    cout << "\n print end \n";

    dateTrans->testPrintedInput();


    delete dateTrans;
    delete iArduino;

    //arduino->close();
    //delete arduino;
    return a.exec();
}




