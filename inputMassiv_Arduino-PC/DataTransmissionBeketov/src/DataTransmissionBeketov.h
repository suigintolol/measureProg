#pragma once
//#include <Arduino.h>
//#include <RefractorBeketov/src/RefractorBeketov.h>
#include <cstddef>
#include <QTime>
#include <QString>
#include <QDebug>
#include <iostream>


using namespace std;

typedef char byte;

enum Comand {
    CUST_COMAND,
    GET_VOUT,
    GET_FULL_ANGLE,
    END_FULL_ANGLE,              // его отправит ардуинка после отправки последнего пакета с массивом данных
    MASS_VOUT,
    FOCUS_SECTION,
    GET_IND,
    GET_R,
    TAKE_STEPS,
    MOVE_POSITION,
    CONFIRM,
    CONNECT,
    ERR,
};

enum ExitCode {
    CUST_EXIT_CODE,
    END_CODE,
    ERR_CODE,
    TIMER_CODE,
    ID_CODE,
};

class ISerial { 
public:

    virtual byte iRead() = 0;

    virtual void iWrite(const byte* val) = 0;

    //virtual void iWrite(const byte val) = 0;

    virtual void iWrite(byte* ol_buffer, unsigned int sizeSendMes) = 0;

    virtual size_t iAvailable() = 0;

    virtual void iPrintConsol(QString ol_buffer) = 0;

    virtual ~ISerial();
};

class Timer {
    QTime *timer = new QTime;

    unsigned long timeStart = timer->msec();
    unsigned long needTime = 50;

public:
    void zeroingTimer();

    void setTimer(unsigned long val);

    bool checkTimer();

    ~Timer();
};

class DataTransmission {
    //    # sizePac id kod krSum                        структура пакета команды
    ///   # sizePac id kod param krSum                  структура пакета команды с параметром
    ////  # sizePac id kod krSum 4b 4b ... 4b 4b $      структура пакета с значениями

    //QSerialPort* arduino = initializationArduinoDevice();

protected:
    const size_t sizeSendBuf = 256;
    const size_t sizeReceivedBuf = 512;

    byte* sendMes = new byte[sizeSendBuf];
    byte* receivedMes = new byte[sizeReceivedBuf];

    bool fConnection = false;

    byte startRecMes = 0;

    byte sizeSendMes = 0;     //размер пакета на отправку
    size_t sizeReceivedMes = 0; // принятые

    size_t countSendMes = 0; // не может быть больше чем 255
    size_t countReceivedMes = 0; // не может быть больше чем 255

    byte idMes = 1;
    Timer timer;

    union commonType {
        float commonVar;
        unsigned char byte[4];
    };

public:
    ISerial* iSerial;

    DataTransmission(ISerial* _iSerial);

    size_t getStartRecMes();

    byte getSizeRecPac();

    byte getIdRecMes();

    byte getComandRecMes();

    //byte getChecksum(byte* packet, const size_t sizeMes); // возвращает свормированную контрольную сумму.

    byte getChecksum(byte* packet, byte sizeMes); // возвращает свормированную контрольную сумму.

    void clearBufMes(byte* mes, size_t& sizeMes);

    void clearBufMes(byte* mes, byte& sizeMes);

    ExitCode formingDataPacket(const Comand comand, long param = -1, bool fClear = true);//это новое оно не юзабельно для формирования пакета с флотами. (что плохо)
    // можно дописать условие на то масив ли это. и тогда он набьёт пакет данными из буфера.  тогда надо почистить буфер. потом набить данными но с отступом.
    // и тогда не надо будет делаеть переносы с одного буера в другой


    ExitCode insideBuf();     // наполняет буфер данных приёмки для того что бы с ними можно было работать. возращает норм если данные есть.

    ExitCode checkingCheckSumReceivedMes();

    ExitCode checkingIdReceivedMes();

    void insIterStartMes();

    ExitCode insideRecMes();                               //  функция которая запускается другой функцией (см ниже getConfirm() )эта функция запускает процесс принятия пакета

    ExitCode getConfirm();                     // запускаем проц получения подтверждения, после того как отправили что либо .

    ExitCode sendPacket(bool fConfirm = true); // отправляем уже сформированный пакет и получаем ответку на него. результат ответки возвращаем

    ~DataTransmission();

};



