#include <RefractorBeketov.h>
#include <DataTransmissionBeketov.h>

using namespace std;
//#include <QTimer>
//#include <QTime>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4); 
// в версии ардуины испольуем char . без typedef тк в ардуине byte уже существует потому его не переопределить.



int led = 13;
void blink() {  
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(1000);
}

void blink(int time) {  
  digitalWrite(led, HIGH);
  delay(time);
  digitalWrite(led, LOW);
  delay(time);
}

void blink(bool check) {  
  if(check) {
    digitalWrite(led, HIGH);    
  } else {
    digitalWrite(led, LOW);
  }
}


//#define sizeArrVout 10
//#include <iostream>

class ISerialARD : public ISerial {
public:
  ISerialARD() {
    //Serial.begin(115200, SERIAL_8E1);
  }

  char iRead() {
    return Serial.read();
  }

  void iWrite(char val) {
    Serial.write(val);
  }

  void iWrite(char* ol_buffer, unsigned int ol_cnt) {  
    //this->iPrintConsol(ol_buffer, ol_cnt);  
    Serial.write(ol_buffer, ol_cnt);
  }

  size_t iAvailable() {
    return Serial.available();
  }

  /*void iPrintConsol(char* ol_buffer, unsigned int ol_cnt) {
    for(unsigned int i = 0; i < ol_cnt; i++) {
      Serial.print(ol_buffer[i]);      
    }    
  }*/

  /*void iPrintConsol(char* ol_buffer, unsigned int ol_cnt) {
    String tmp = "";
    for(unsigned int i = 0; i < ol_cnt; i++) {
       tmp[i] = ol_buffer[i];
    }
    Serial.print(tmp);
  }*/

  /*void iPrintConsol(char* ol_buffer, const unsigned int ol_cnt = 2) {
    Serial.print(ol_buffer, 2);
  }*/

  void iPrintConsol(String ol_buffer) {
    Serial.print(ol_buffer);
  }
  
  void iFlush() {
    Serial.flush();
  }

  ~ISerialARD() {
    Serial.end();
  }
};


class DataTransmissionARD : public DataTransmission {
  Interface* interface;
  
  public:
  
  DataTransmissionARD(Interface* _interface, ISerialARD* _ISerialARD) : interface(_interface), DataTransmission(_ISerialARD) {}

  ExitCode sendMasFullAngle() { // функция по которой мы запускаем процесс передачи массива полного угла. (те вместе с границами контактных площадок) //должна выполняться SLAVE
  //тут мы должны ещё встать на границу
    bool fEnd = false;
    sizeSendMes = 4;    //  начало данных.
    while((!fEnd) && (fConnection)) { // пока не найдём полный угол (потом передадим его)
      if(interface->meter->insideArrVout()) { 
        ExitCode ec = AddBuf(interface->meter->getAverageVout());
        if(ec == END_CODE) {
          //Всё ок
        } else {
          fConnection = false;
        }
      } else {  //дошли до конца
        //сделать КР и тд
        sendMes[0] = '#';
        sendMes[1] = sizeSendMes;
        sendMes[2] = idMes;
        sendMes[3] = GET_FULL_ANGLE; //какой команд?
        //sendMes[sizeSendMes] = '$';
        sendMes[sizeSendMes - 1] = getChecksum(sendMes, sizeSendMes - 1);

        //sendMes[4] = getChecksum(sendMes, sizeSendMes); //считаем чексумму
        ExitCode ecFloatPacket = sendPacket(); 
        if(ecFloatPacket == END_CODE) {
          fEnd = true;
          //Подготовить буфер Отправки для работы с следующим итератором();
          sizeSendMes = 4;    //  начало данных.
          idMes++;
        } else {
          fConnection = false;
          return ecFloatPacket;
        }
      }
    }
    formingDataPacket(END_FULL_ANGLE);
    ExitCode ecEndFloatPacket = sendPacket();
    if(ecEndFloatPacket == END_CODE) {
      return END_CODE;
    } else {
      return ecEndFloatPacket;
    }
  }

  ExitCode AddBuf(float val) { // добавляет элемент в буфер. или если буфер полный то отправляет его и возвращает результат, успешно или не успешно //должна выполняться SLAVE
    union commonType {
      float commonVar;
      char mychar[4];
    };
    commonType bufPacket;
    bufPacket.commonVar = val;
    for (size_t i = 0; i < 4; i++) {
      sendMes[sizeSendMes++] = bufPacket.mychar[i]; //buf это буфер на отправку
    }
    if(sizeSendMes > sizeSendBuf - 4) {
      // места нет!, Надо отправить
      sizeSendMes++; //для вставки доллара
      sendMes[0] = '#';
      sendMes[1] = sizeSendMes;
      sendMes[2] = idMes;
      sendMes[3] = MASS_VOUT; //какой команд?
      //sendMes[sizeSendMes] = '$';             // может он не нужен????
      //sendMes[4] = getChecksum(sendMes, sizeSendMes); //считаем чексумму. вопрос??? считать чексумму до доллара или после????

      sendMes[sizeSendMes - 1] = getChecksum(sendMes, sizeSendMes - 1);

      ExitCode ecSendPacket = sendPacket(); // отправляем, получая код
      if(ecSendPacket != END_CODE) {
        return ecSendPacket;
      } else {
        //Подготовить буфер Отправки для работы с следующим итератором();      
        sizeSendMes = 4;    //  начало данных.
        idMes++;
      }      
    } else {
      //Есть место для следующей итерации
    }
    return END_CODE;
  }

  void inputModeARD() { // тут будет бесконечный цикл в котором мы будем принимать данные от компухтера. //должна выполняться SLAVE
    char ccRecMes;
    ExitCode ecInsRecMes;
    while(fConnection) {
      if(iSerial->iAvailable() != 0) {
        //раскидать данные на пакет и проверить правильность ( эти функции уже имеются в insideRecMes)        
        ecInsRecMes = insideRecMes();
        lcd.setCursor(17,1);
        lcd.print("9");  
        lcd.setCursor(1,0);
        for(size_t i = 0; i < sizeReceivedMes; i++) {
          lcd.print(receivedMes[i]);
        }
        if(ecInsRecMes == END_CODE) {
          formingDataPacket(CONFIRM);
          sendPacket(false); //отправить подтверждение  
          ccRecMes = getComandRecMes();
          lcd.setCursor(18,1);
          lcd.print("8");  
          switch(ccRecMes) {         //   расбираем его.
                //case CUST_COMAND:
                //  break;
                case GET_VOUT:
                  break;
                case GET_FULL_ANGLE: {
                  // надо отправить подтверждение по хорошему .
                  ExitCode ecMasFA = sendMasFullAngle();           // может стоит добавить условие всё ли прошло гладко. но вопрос. а делать то что если пошло не так. наверное стоит отправить код ошибки
                  if(ecMasFA == ERR_CODE) {
                    formingDataPacket(ERR);
                    sendPacket(false);
                  }
                  break;
                }
                //case END_FULL_ANGLE:              // его отправит ардуинка после отправки последнего пакета с массивом данных
                //  break;
                case GET_IND:
                  break;
                case TAKE_STEPS:
                  break;
                case MOVE_POSITION:
                  break;
                //case CONFIRM: // не здесь
                //  break;
                case CONNECT:                 // чисто для отладки что бы не ресетать каждый раз дуинку
                  formingDataPacket(CONFIRM);
                  sendPacket(false);
                  lcd.setCursor(10,2);
                  lcd.print("DCon");  
                  break;
                case ERR:
                  lcd.setCursor(19,2);
                  lcd.print("E");  
                  break;                
                default:
                                
                  break;
          }
          ccRecMes = CUST_COMAND; // типа обнуление
        } else {
          formingDataPacket(ERR);
          sendPacket(false);
        }
      }
    }
    lcd.setCursor(10,1);
    lcd.print("inpModEnd");    
  }

  void connectionMode() { //  здесь мы болтаемся пока не получим запрос на connect .
    ExitCode ecInsRecMes;
    char ccRecMes;
    size_t iterMes = 0;
    //Serial.println("con1");
    lcd.setCursor(0,2);
    lcd.print("p1");
    lcd.setCursor(17,2);
    lcd.print("1");
    while(true) {
      ecInsRecMes = insideRecMes();      
      if(ecInsRecMes == END_CODE) {
        //Serial.println("con2");
        lcd.setCursor(0,2);
        lcd.print("p2");
        lcd.setCursor(18,2);
        lcd.print("2");
        ccRecMes = getComandRecMes();

        lcd.setCursor(0,3);
        lcd.print("ccRM ");
        lcd.print(int(ccRecMes));
        
        lcd.setCursor(8,3);
        lcd.print("stRM ");
        lcd.print(int(startRecMes));

        lcd.setCursor(8,1);
        lcd.print("siRM ");
        lcd.print(sizeReceivedMes);
        
                      
        if(ccRecMes == CONNECT) {
          //Serial.println("con3");
          lcd.setCursor(0,2);
          lcd.print("p3");
          lcd.setCursor(19,2);
          lcd.print("3"); 

          lcd.setCursor(7,2);
          for(size_t i = 0; i < 10; i++) { // тут пустота по идее идёт.
            lcd.print(sendMes[i]);
          }
          
          formingDataPacket(CONFIRM);

          lcd.setCursor(7,0);
          for(size_t i = 0; i < 10; i++) {// тут пакетик. значит он фформируется
            lcd.print(sendMes[i]);
          }
// надо зафиксировать пакет нормальный который приходит от пеки. что бы он красиво смотрелся там (хз зачем) пакет конфёрма и конекта разные

          for (size_t i = 0; i < 10; ++i) {////////////////////////////////////////// идёт тестирование . 
            //sendPacket(false);
            Serial.write(sendMes);
          }
          sendPacket(false); //отправить подтверждение/ вот тут пека не получает пакет или долбится в порты
          fConnection = true;
          inputModeARD();
        }
        lcd.setCursor(1,1);
        lcd.print(iterMes);
        lcd.setCursor(1,0);
        for(size_t i = 0; i < sizeReceivedMes; i++) {
          lcd.print(receivedMes[i]);
        }
        iterMes++;
      }
      //blink();
      //Serial.flush();
      lcd.setCursor(1,2);
      lcd.print("              ");            
    }
  }

  void testMas(char* maschar, size_t size) {
    Serial.write(maschar, size);
  }

  void testF() {
    formingDataPacket(CONNECT);
    char tmp = '#';
    if(sendMes[0] == tmp) {
      Serial.println("0 eap");
    }
    if(sendMes[1] == 5) {
      Serial.println("1 eap");
    }
    if(sendMes[2] == idMes) {
      Serial.println("2 eap");
    }
    if(sendMes[3] == CONNECT) {
      Serial.println("3 eap");
    }
    tmp = getChecksum(sendMes, sizeSendMes);
    Serial.println(int(tmp));
    if(tmp == 0) {
      Serial.println("4 eap");
    }
    
    Serial.println("\nFULL");
    testMas(sendMes, 5);
    Serial.println("\nnotFULL");
    testMas(&sendMes[1], 4);
    Serial.println("\nENDTEST");
    sendPacket(false); //отправить подтверждение  
  }

};

void setup() {
  Serial.begin(115200);
  //Serial.begin(9600);
  lcd.init(); 

  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("M");
  lcd.setCursor(0,1);
  lcd.print("P");
  lcd.setCursor(0,2);
  lcd.print("beb");
  lcd.setCursor(0,3);
  lcd.print(" ");

  //Serial.begin(9600, SERIAL_8E1);
  //SerialUSB.begin(9600);
  pinMode(led, OUTPUT);
  //Serial.print(" Start ");
}

void loop() {
  delay(1000);
  size_t maxSec = 4;
  long* pMeterV = new long[maxSec]{ A0,   A1,   A2,   A3 };
  long* pMeterR = new long[maxSec]{ A8,   A9,   A10,  A11 };
  float fullAngle = 360;
  float reductions = fullAngle / 25600;
  Stepper* spet = new Stepper(48, 50, 52, reductions, fullAngle, 350);  //    Stepper(long pStep, long pDir, long pPower, float reduction, float countOnFull, long timeStop)
  Meter* metr = new Meter(pMeterV, pMeterR, 5, 10);                     //    Meter(long* pMeterV, long* pMeterR, float vin, int bit = 10)
  Interface* interface = new Interface(spet, metr);
  ISerialARD* ser = new ISerialARD();
  DataTransmissionARD* dtrArd = new DataTransmissionARD(interface, ser);

  //dtrArd->testF();
  dtrArd->connectionMode();

    
    //byte testByte = 0;
    //dtrArd->formingDataPacket(GET_VOUT);
    //while(true) {
       //dtrArd->sendPacket(false);
       //delay(1);      
       //SerialUSB.write(testByte++);
       //SerialUSB.write(255);
       //Serial.write(testByte++);
       
    //}


  char kek = 3;
  // put your main code here, to run repeatedly:
  lcd.setCursor(0,0);
    
  size_t iterMes = 0;
  char beb = 0;
  delay(1000);
  while(true) {
    lcd.setCursor(0,0);    
    if (Serial.available() > 0) { 
      
      while(Serial.available() > 0) {
        beb = Serial.read();
        lcd.write(beb);
        delay(1);
      }
      lcd.setCursor(0,1); 
      lcd.print(iterMes);
      iterMes++;
    }
  }
  
  Serial.print(" ONOOOOOO f/n ");


  /*delay(10);
  metr->setFocusSections(0);
  Serial.print(" R1 ");
  Serial.println(interface->getRTotal());
  interface->insideCoordContctPlatform();
  Serial.print(" AA1 ");
  Serial.println(interface->getAdjAngl());
  Serial.print(" FAA1 ");
  Serial.println(interface->getFullAdjAngl());
  Serial.print(" FCh1 ");
  Serial.println(interface->getFCh());

  Serial.print(" cont 1.1 ");
  Serial.println((interface->getCoordContctPlatformMinEnd() - interface->getCoordContctPlatformMinStrt()) * interface->stepper->getReduction());
  Serial.print(" cont 1.2 ");
  Serial.println((interface->getCoordContctPlatformMaxEnd() - interface->getCoordContctPlatformMaxStrt()) * interface->stepper->getReduction());
*/

  /*

  delay(10);

  metr->setFocusSections(1);
  Serial.print(" R2 ");
  Serial.println(interface->getRTotal());
  interface->insideCoordContctPlatform(); 
  Serial.print(" AA2 ");
  Serial.println(interface->getAdjAngl());
  Serial.print(" FAA2 ");
  Serial.println(interface->getFullAdjAngl());
  
  Serial.print(" FCh2 ");
  Serial.println(interface->getFCh());

  //вычисление рассогласования

  double ind1;
  double ind2;
  double nonLineary;
  long stepInd;
  delay(1000);

  metr->setFocusSections(0);
  stepInd = interface->moveToVout(1023 / 2, false);
  interface->meter->insideArrVout();
  ind1 = interface->meter->getAverageVout();
  Serial.print(" vout ind 1 ");
  Serial.println(ind1);
  delay(10);
  metr->setFocusSections(1);


  interface->meter->insideArrVout();
  ind2 = interface->meter->getAverageVout();
  Serial.print(" vout2Pin ");
  Serial.println(ind2);
  nonLineary = (ind2 - ind1) / 1023.0 * 100.0;
  Serial.print(" nonLineary2 ");
  Serial.print(nonLineary);
  Serial.println(" % ");
*/

  spet->setPower(true);
  delete dtrArd;  
  delete ser;
  delete interface;
  delete metr;
  delete spet;
  delete pMeterR;
  delete pMeterV;
  
  Serial.println(" ************************************************** ");
  delay(100000);
}
