using namespace std;


#include "RefractorBeketov.h"
#include <DataTransmissionBeketov.h>

//#define TestMode


// ISerial ---


// DataTransmission 
  //    # sizePac id kod krSum                        структура пакета команды
  ///   # sizePac id kod param krSum                  структура пакета команды с параметром
  ////  # sizePac id kod krSum 4b 4b ... 4b 4b $      структура пакета с значениями
 
  DataTransmission::DataTransmission(ISerial* _iSerial) : iSerial(_iSerial) {
	clearBufMes(sendMes, sizeSendMes);
    clearBufMes(receivedMes, sizeReceivedMes);	  
  }

  size_t DataTransmission::getStartRecMes() { return startRecMes; } 
  
  char DataTransmission::getSizeRecPac() { return receivedMes[startRecMes+1]; }
  
  char DataTransmission::getIdRecMes() { return receivedMes[startRecMes+2]; }

  char DataTransmission::getComandRecMes() { return receivedMes[startRecMes+3]; }

  /*char DataTransmission::getChecksum(char* packet, const size_t sizeMes) { // возвращает свормированную контрольную сумму.
    //char Checksum = 0;
    size_t countEvenchars = 0;
    size_t numberUnits = 0;
    for (size_t i = 0; i < sizeMes-1; ++i) {
      numberUnits = 0;
      for (; packet[i]; packet[i] <<= 1) { // вроде как тут подсчитывается сумма чётных байтов в пакете на отправку.
          numberUnits += packet[i] & 1;
      }
      if (!numberUnits%2) {
          ++countEvenchars;
      }
    }
    return (char)countEvenchars;
  }*/

  char DataTransmission::getChecksum(char* packet, char sizeMes) { // возвращает свормированную контрольную сумму.
	char crc = 0;
	for (int i = 0; i < sizeMes; i++) {
		char data = packet[i];
		for (int j = 8; j > 0; j--) {
		  crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
		  data >>= 1;
		}
	  }
	 return crc;
  }

  /* char* formingDataPacket (float date) {
       union commonType {// нельзя сразу задать размер массива переменной. поэтому придётся городить switch
           float commonVar;
           unsigned char char[4];
       };

       commonType bufPacket;
       size_t dataTypeSize = sizeof(date);
       size_t sizePacket = 5 + dataTypeSize;
       char* packet = new chaRTotal[sizePacket];

       packet[0] = (char)68; // 68 - Date
       packet[1] = (char)countSendMes;
       packet[2] = (char)countReceivedMes;
       packet[3] = (char)dataTypeSize;

       bufPacket.commonVar = date;

       for(size_t i = 0; i < dataTypeSize; ++i) {
          //packet[4+i] = bufPacket.commonVaRTotal[i];
          packet[4+i] = bufPacket.char[i];
       }

       packet[sizePacket-1] = getChecksum(packet, sizePacket);
       return packet;
   }*/

  void DataTransmission::clearBufMes(char* mes, size_t& sizeMes) {
    for (size_t i = 0; i < sizeMes; ++i) {
      mes[i] = 0;
    }
    sizeMes = 0;
  }

  void DataTransmission::clearBufMes(char* mes, char& sizeMes) {
    for (size_t i = 0; i < sizeMes; ++i) {
      mes[i] = 0;
    }
    sizeMes = 0;
  }
  
  ExitCode DataTransmission::formingDataPacket(const Comand comand, long param, bool fClear) {//это новое оно не юзабельно для формирования пакета с флотами. (что плохо)    
    // можно дописать условие на то масив ли это. и тогда он набьёт пакет данными из буфера.  тогда надо почистить буфер. потом набить данными но с отступом.
    // и тогда не надо будет делаеть переносы с одного буера в другой
    if(fClear) {
      clearBufMes(sendMes, sizeSendMes);
    }
    
    if (param == NULL) {
    sizeSendMes = 5;
    sendMes[0] = '#';
    sendMes[1] = sizeSendMes;
    sendMes[2] = idMes;
    sendMes[3] = comand;
    sendMes[4] = getChecksum(sendMes, sizeSendMes-1);
    } else {
      union commonType {
        long commonVar;
        char mychar[4];
      };

      sizeSendMes = 9;
      sendMes[0] = '#';
      sendMes[1] = sizeSendMes;
      sendMes[2] = idMes;
      sendMes[3] = comand;
      commonType bufPacket;
      bufPacket.commonVar = param;
      sendMes[4] = bufPacket.mychar[0];
      sendMes[5] = bufPacket.mychar[1];
      sendMes[6] = bufPacket.mychar[2];
      sendMes[7] = bufPacket.mychar[3];
      sendMes[8] = getChecksum(sendMes, sizeSendMes-1);
    }
    return END_CODE;
  }

  ExitCode DataTransmission::insideBuf() {     // наполняет буфер данных приёмки для того что бы с ними можно было работать. возращает норм если данные есть.
    clearBufMes(receivedMes, sizeReceivedMes);
    timer.zeroingTimer();
    while (iSerial->iAvailable() == 0) {
      if(timer.checkTimer()) { // надо сделать норм таймер  и условие для выхода. и возврата кода таймера.
        return TIMER_CODE;
      }
    }
	delay(500);
    while(iSerial->iAvailable() > 0 && sizeReceivedMes < sizeReceivedBuf) {	  
      receivedMes[sizeReceivedMes++] = iSerial->iRead();
	  iSerial->iFlush();
    }
    return END_CODE;/**********************************************/    
  }

//супер новая версия
/*ExitCode DataTransmission::checkingCheckSumReceivedMes() {
    byte sizeTempMasRes = receivedMes[startRecMes+1];
    byte calcChecksum = getChecksum(&receivedMes[startRecMes], sizeTempMasRes);
    if(calcChecksum == 0) {
        return END_CODE; // чексумма прошла.
    } else {
        return ERR_CODE; // чексумма не прошла.
    }
}*/

  ExitCode DataTransmission::checkingCheckSumReceivedMes() {
    char sizeTempMasRes = receivedMes[startRecMes+1];
    char* tempMasRes = new char[sizeTempMasRes];

    size_t j = startRecMes;
    for(size_t i = 0; i < sizeTempMasRes; ++i) {
      tempMasRes[i] = receivedMes[j];
      j++;
    }

    char checksumMasRes = receivedMes[startRecMes + sizeTempMasRes - 1];
    char calcChecksum = getChecksum(tempMasRes, sizeTempMasRes);
    delete[] tempMasRes;

    if(checksumMasRes == calcChecksum) {
      return END_CODE; // чексумма прошла. 
    } else {
      return ERR_CODE; // чексумма не прошла. 
    }
  }
  
  ExitCode DataTransmission::checkingIdReceivedMes() {
    if(getIdRecMes() == idMes) {
      return END_CODE;  // id совпадает 
     }else {
      return ERR_CODE;  // id не совпадает 
    }
  }

  void DataTransmission::insIterStartMes() {
      char iterStart = 0;
      while(iterStart < sizeReceivedMes && receivedMes[iterStart] != '#') {
        iterStart++;
      }
      startRecMes = iterStart;
  }

  ExitCode DataTransmission::insideRecMes() {    //  функция которая запускается другой функцией (см ниже getConfirm() )эта функция запускает процесс принятия пакета
    ExitCode ecInsBufDataPac = insideBuf();
    if(ecInsBufDataPac == END_CODE) {
      //данные есть  можно обрабатывать
      insIterStartMes();
      if(/*startRecMes < sizeReceivedMes                        // есть ли символ старта пакета в буфере?
        &&*/ startRecMes + getSizeRecPac() < sizeReceivedMes // если есть, то проверяем влез ли пакет в буфер полностью?
        && checkingCheckSumReceivedMes()                   // чек сумма 
        && checkingIdReceivedMes()) {                       // id
        return END_CODE;                                      // успех 
      } else {
        return ERR_CODE;                                      // пакет повреждён
      }
    } else {
      return TIMER_CODE;                                      //если прошёл таймер, а данных всё нет
    }
  }

  ExitCode DataTransmission::getConfirm() {                     // запускаем проц получения подтверждения, после того как отправили что либо . 
    ExitCode ecInsideRecMes = insideRecMes();
    switch(ecInsideRecMes) {
      case END_CODE:
        if(getComandRecMes() == CONFIRM) {             // в команде пакета, который мы приняли, подтверждение?
          return END_CODE;
        } else {
          return ERR_CODE;                                      // символа старта нету в том что мы приняли
        }
        break;
      case ERR_CODE:
        return ERR_CODE;
        break;
      case TIMER_CODE:
        return TIMER_CODE; 
        break;
      default:        
        iSerial->iPrintConsol("getConfirm default, wtf? \n");
        return ecInsideRecMes;
        break;
    }
  }

    ExitCode DataTransmission::sendPacket(bool fConfirm) {
      // отправляем уже сформированный пакет и получаем ответку на него. результат ответки возвращаем
      iSerial->iWrite(sendMes, sizeSendMes);
      // получаем ответку. 
      if(fConfirm) {
        ExitCode ecGetConfirm = getConfirm();
        switch(ecGetConfirm) {
          case END_CODE:
            // если  ответка пришла и всё норм. возвращаем ендКод
            return END_CODE;
            break;
          case TIMER_CODE:
			for (int var = 0; (var < 3) && (ecGetConfirm == TIMER_CODE); ++var) {
                iSerial->iWrite(sendMes, sizeSendMes);
                ecGetConfirm = getConfirm();
            }
            //  если таймер  пробуем ещё раз
            //  отправляем, ждём ответ. и так n попыток. если не получается то пишем дисконект.
            //  дописать/
			return ecGetConfirm;
            //return TIMER_CODE;
            break;
          case ERR_CODE:
            // если ерр то возвращаем ерр
            // обработка ошибки. что то пошло не так пакет нажо отправить заново.
            iSerial->iPrintConsol("ERROR getConfirm(comand)\n");
            return ERR_CODE;
            break;
          default:
          iSerial->iPrintConsol("sendComand default, wtf? \n");
            return ecGetConfirm;
            break;
        }
      } else {
        return END_CODE;
      }
    }  

  DataTransmission::~DataTransmission() {
    delete[] receivedMes;
    delete[] sendMes;
  }





