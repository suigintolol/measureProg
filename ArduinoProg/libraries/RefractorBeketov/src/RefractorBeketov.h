#pragma once
#include <Arduino.h>
#include <SPI.h>
//#include <SD.h>
using namespace std;

enum OutputMode {
  CONSOL,
  SD_CARD,
};

typedef char kek;

//typedef char byte;
//typedef unsigned char byte;

/*
OutputMode outputMode = SD_CARD;

const int chipSelect = 53;
SD.begin(chipSelect);
File dataFile = SD.open("test.txt", FILE_WRITE);
*/



class Meter {
protected:
  const size_t sizeRChooseReference = 6;
private:
  const size_t maxSection = 4;
  float vin;
  size_t focusSections = 0;
  size_t countSections = 0;
  bool fPot = true;			//если false, то потенциометр не подключён.
  long* pMeterV;
  long* pMeterRTotal;
  float* RTotal  = new float[maxSection] { 0 };
  
  float averageVout;            //нужное значение. можно считать его "правильным" и свободным от шумов 50Гц
  float medianVout;            //среднее медианное значение
  const int sizeArrVout = 20;  // сколько измерений берём на оодин период синусоиды 50Гц
  float priceBit;              //цена 1 бита АЦП в вольтах
  long ADCsignals;				// всего бит
  float* arrVout = new float[sizeArrVout] { 0 };
  float quadraticAverageVout = 0;
  bool fValidation = true;
    
  long* RChooseReference = new long[sizeRChooseReference] { 0, 1000000, 100000, 10040, 1003, 100 };
  int* DChooseReference = new int[sizeRChooseReference] { 2, 7, 6, 5, 4, 3 };

  float* U1  = new float[maxSection] { 0 };
  float* U3  = new float[maxSection] { 0 };
  
public:
  Meter(long* pMeterV, long* pMeterRTotal, float vin, int bit = 10);
  ~Meter();

  enum pinMeasure {
    V_MODE,
    R_MODE,
  };

  void setFocusSections(size_t _focusSections);

  float getRTotal();
  
  float getU1();
  
  float getU3();
  
  void setU1(float UVout);
  
  void setU3(float UVout);

  void offVin();
  
  float getQuadraticAverageVout();

  float getVin();

  long getADCsignals();

  bool getfValidation();

  float getPriceBit();

  float getAverageVout();
  
  float getMedianVout();
  
  size_t getCountSections();
  
  bool getfPot();

  float MeasureVout(pinMeasure pinMes = V_MODE);

  long MeasureBitADC(pinMeasure pinMes = V_MODE);

  void chooseReference(long selectedR);

  float measureRTotal();

  float getCalcilateAVR(float* mas, int size);

  float getQuadraticAverageVout(float avrVout, float* mas, int size);

  float* getArrVoutFiltrThreeSigma(float avrVout, float quadrAvrVout, float* mas, int size, int& newSize);

  bool insideArrVout(pinMeasure pin = V_MODE);
};

class Stepper {
  long pStep;
  long pDir;
  long pPower;

  float reduction;    // это сколько единиц измерения (углы или миллиметры) приходится на 1 шаг ШД 
  float countOnFull;  //количество единиц измерения (углы или миллиметры) на полное перемещение от начала до конца
  long timeStop;      //задержка по идее надо связать их с количеством шагов на полное движение, но не факт, ибо муторное дело

  long countStepsOnFull = countOnFull / reduction;  //количество шагов на полное перемещение от начала до конца
  bool Dir = 0;                                     // 0 - по часовой 1 - против часовой.   все функции написаны с учётом и  //дописать для линейных
  bool Power = 0;
  long counterSteps = 0;

public:
  Stepper(long pStep, long pDir, long pPower, float reduction, float countOnFull, long timeStop);
  
  ~Stepper();
  
  void ZeroingCounterSteps();

  float getReduction();

  long getPStep();

  long getCounterSteps();
  
  float getCounterAngle();

  bool getPower();

  bool getDir();

  long getPDir();
    
  long getCountStepsOnFull();

  void setReduction(long reduction);

  void invDir();

  void setDir(bool needDir);  // true по возрастанию

  void setPower(bool Power);

  void setTimeStop(long timeStop);

  void moveStep();

  void moveToPosition(const long needPosition);

  void movePercentagesAngle(long percentagesAngle);

  void moveSteps(long numberOfSteps);
  
};

class Interface {
  //float bufMeasure[10] = { 0 };
  //float* fullElectricAngle = new float[stepper->getCountStepsOnFull()];
  const long coordContctPlatformMinStrt = 0;  // 0 v
  long coordContctPlatformMinEnd = 0;
  long coordContctPlatformMaxStrt = 0;
  long coordContctPlatformMaxEnd = 0;
  
  long coordContctPlatformMinEndV2 = 0;
  long coordContctPlatformMaxStrtV2 = 0;
  
  bool fCoordContctPlatform = false;   //становится true после того как мы получим координаты
  bool fPotAvailability = false;
public:
  Stepper* stepper;
  Meter* meter;

  Interface(Stepper* stepper, Meter* meter);

  ~Interface();

  void test();
  
  void test2();

  long getCoordContctPlatformMinStrt();

  long getCoordContctPlatformMinEnd();

  long getCoordContctPlatformMaxStrt();

  long getCoordContctPlatformMaxEnd();

  float getRTotal();
  
  bool getfPotAvailability();

  float mesU(long sizeSteps = 10, long countMes = 10);
  
  long getCoordinatesBorderAdjAngl(long startVout, long sizeSteps = 10, bool fFast = false, bool test = false, float border = 20);

  long getCoordinatesBorderFulAngl(bool fStartValidation, long sizeSteps = 10, bool fFast = false);   // будет двигать пока не получит в точке искомую валидацию: true - в зоне. false - не в зоне    

  void insideCoordContctPlatform();
  
  void insideCoordContctPlatformV2();

  float getAdjAngl();

  float getFullAdjAngl();

  float getFCh(const long numbPoints = 70);
  
  float getFChV2(const long numbPoints = 70);  

  long moveToVout(const float needVout, bool fAllowance = true);

  float getNonlinearity();   // нелинейность . надо дописасать

  bool sendMeasureVout(long stepFrequency);  // двигает шаговик, отправляет данные на компьютер. всего данных будет столькоже, сколько шагов у двигателя по модулю

};



class Timer {
  unsigned long timeStart = millis();
  unsigned long needTime = 1000;
  //QTimer *timer;

  public:
    void zeroingTimer();

    void setTimer(unsigned long val);

    bool checkTimer();
};



