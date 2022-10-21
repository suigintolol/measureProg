#include "DataTransmissionPC.h"

DataTransmissionPC::DataTransmissionPC(ISerial* _iSerial) : DataTransmission(_iSerial) {
    //iSerialPC =
}

DataTransmissionPC::~DataTransmissionPC() {
    //delete iSerial;
}

void DataTransmissionPC::inputVFullAngle() {
    size_t startVal = 5; //начало приёма значений из пакета n - хз какое. надо смотреть структуру пакета с флотами
    //size_t focusIter = startVal + 1;           // итератор с которого продолжается заполнение массива. он не нужен если мы всегда вставляем в конец
    commonType bufPacket;
    for(size_t i = startVal; i < sizeReceivedMes; i += 4) {
        for(size_t j = 0; j < 4; ++j) {
            bufPacket.byte[j] = receivedMes[i + startVal + j];
        }
        QVector<float> vAdjAngle(2);   // убрать костыль
        vAdjAngle.append(bufPacket.commonVar); // значение из масива значений которые мы приняли;
    }
    //sendComand(GET_FULL_ANGLE);
}

//ExitCode DataTransmissionPC::getMasFullAngle(float* mas, size_t & size) { //***дописать***   эта функция вызывается с компьютера. и запускает процесс в результате которого на компьютере должен быть массив данных полного угла
ExitCode DataTransmissionPC::getMasFullAngle() {
    formingDataPacket(GET_FULL_ANGLE);
    if (sendPacket() == END_CODE) {
        //if(ecConfirmGetMasFullAngle == END_CODE) {
        bool fFinal = false;
        byte ccRecMes ;///= NULL;
        //size_t focusIter = 0;
        while(!fFinal && fConnection) { //цикл пока не завершающий флаг/  или не флаг ошибки или не флаг дооолгого таймера.       // вроде можно вынести в отдельную функцию
            ExitCode ecInsideRecMes = insideRecMes(); //принимаем пакет.
            switch(ecInsideRecMes) {
            case END_CODE:
                ccRecMes = getComandRecMes();
                switch(ccRecMes) {         //   расбираем его.
                case MASS_VOUT:
                    inputVFullAngle();
                    //точно ли так?
                    formingDataPacket(CONFIRM);
                    sendPacket(false); //отправить подтверждение
                    break;
                case END_FULL_ANGLE:
                    fFinal = true; //масив получен. завершаем работу фуункции;
                    formingDataPacket(CONFIRM);
                    sendPacket(false); //отправить подтверждение
                    break;
                default:
                    iSerial->iPrintConsol("getComandRecMes default, wtf? \n");
                    return ecInsideRecMes;
                    break;
                }
                //***дописать***
                break;
            case ERR_CODE:
                //отправляем вместо подтверждения - ошибку. нам отправляют повторно. ловим ответку
                formingDataPacket(ERR);
                sendPacket(false);
                return ERR_CODE;
                break;
            case TIMER_CODE:
                //дисконект?
                formingDataPacket(ERR);
                sendPacket(false);
                return TIMER_CODE;
                break;
            default:
                iSerial->iPrintConsol("insideRecMes default, wtf? \n");
                formingDataPacket(ERR);
                sendPacket(false);
                return ecInsideRecMes;
                break;
            }

        }
    } else {
        return ERR_CODE; // если что то пошло не так то возвращаем ERR
    }
}

void DataTransmissionPC::testMas(char* maschar, size_t size) {
  //iSerial->iWrite(maschar, size);
  iSerial->iPrintConsol(maschar);
}

void DataTransmissionPC::testPrintedInput() {
    clearBufMes(receivedMes, sizeReceivedMes);
    timer.zeroingTimer();
    timer.setTimer(10000);
    while(timer.checkTimer()) {
        while (iSerial->iAvailable() == 0) {
            if(timer.checkTimer()) {
                iSerial->iPrintConsol("nope \n");
            }
        }
        while(iSerial->iAvailable() > 0 && sizeReceivedMes < sizeReceivedBuf) {
            receivedMes[sizeReceivedMes++] = iSerial->iRead();
        }
    }
    iSerial->iPrintConsol("YOUR Date \n");
    iSerial->iPrintConsol(receivedMes);
}


void DataTransmissionPC::testF() {
  formingDataPacket(CONNECT);
  byte tmp = '#';
  if(sendMes[0] == tmp) {
    iSerial->iPrintConsol("0 eap");
  }
  if(sendMes[1] == 5) {
    iSerial->iPrintConsol("1 eap");
  }
  if(sendMes[2] == idMes) {
    iSerial->iPrintConsol("2 eap");
  }
  if(sendMes[3] == CONNECT) {
    iSerial->iPrintConsol("3 eap");
  }
  tmp = getChecksum(sendMes, sizeSendMes);
  if(0 == tmp) {
    iSerial->iPrintConsol("4 eap");
  }

  iSerial->iPrintConsol("\nFULL");
  testMas(sendMes, 5);
  iSerial->iPrintConsol("\nnotFULL");
  testMas(&sendMes[1], 4);
  iSerial->iPrintConsol("\nENDTEST");
  sendPacket(false); //отправить подтверждение
}

