#ifndef DATATRANSMISSIONPC_H
#define DATATRANSMISSIONPC_H
#include "DataTransmissionBeketov/src/DataTransmissionBeketov.h"
#include <QVector>
#include "ISerialPC.h"


class DataTransmissionPC : public DataTransmission {
    //Interface* interface;
    //QVector<float> vAdjAngle(); // вернуть

public:
    //

    DataTransmissionPC(ISerial* _iSerial);

    void inputVFullAngle();

    //ExitCode getMasFullAngle(float* mas, size_t & size);
    ExitCode getMasFullAngle();

    void testMas(char* maschar, size_t size);
    void testF();
    void testPrintedInput();

    ~DataTransmissionPC();
};



#endif // DATATRANSMISSIONPC_H
