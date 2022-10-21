using namespace std;


#include "RefractorBeketov.h"


//#define TestMode

  Meter::Meter(long* pMeterV, long* pMeterRTotal, float vin, int bit)
    : pMeterV(pMeterV), pMeterRTotal(pMeterRTotal), vin(vin), ADCsignals(pow(2, bit)), priceBit(vin / ADCsignals) {
      for (long i = 0; i < maxSection; i++) {
        pinMode(pMeterV[i], INPUT);
        pinMode(pMeterRTotal[i], INPUT);
      }
      
      chooseReference(0);
      float temtR = 0;      
      for (; countSections < maxSection; countSections++) {
        setFocusSections(countSections);
        temtR = measureRTotal();
        if((temtR != 0)  && getfValidation()) {
          RTotal[countSections] = temtR;
		  #ifdef TestMode
			  Serial.print("\tRtotal_");
			  Serial.print(countSections);
			  Serial.print(" \t");
			  Serial.println(temtR);
		  #endif
        } else {
          break;
        }
      }
	  if (countSections == 0) {
		  fPot = false;
	  }	  	 
      setFocusSections(0);
	  #ifdef TestMode
		  Serial.print("\tcountSections\t");
		  Serial.println(countSections);
	  #endif
	  //delay(10);
    }
	
  Meter::~Meter() {
	offVin();
    delete[] RTotal;
	delete[] arrVout;
	delete[] RChooseReference;
	delete[] DChooseReference;
	delete[] U1;
	delete[] U3;
  }
  
  void Meter::setFocusSections(size_t _focusSections) { focusSections = _focusSections; }

  float Meter::getRTotal() { return RTotal[focusSections]; }
  
  float Meter::getU1() { return U1[focusSections]; }
  
  float Meter::getU3() { return U3[focusSections]; }
  
  void Meter::setU1(float UVout) {
	  U1[focusSections] = UVout; 
  }
  
  void Meter::setU3(float UVout) {
	  U3[focusSections] = UVout; 
  }
  
  void Meter::offVin() {
	for (long i = 0; i < sizeRChooseReference; i++) {     
	  pinMode(DChooseReference[i], OUTPUT);
	  delay(10);
      digitalWrite(DChooseReference[i], LOW);    
    }
  }
  
  float Meter::getQuadraticAverageVout() { return quadraticAverageVout; }

  float Meter::getVin() { return vin; }

  long Meter::getADCsignals() { return ADCsignals; }

  bool Meter::getfValidation() { return fValidation; }

  float Meter::getPriceBit() { return priceBit; }

  float Meter::getAverageVout() { return averageVout; }
  
  float Meter::getMedianVout() { return medianVout; }

  float Meter::MeasureVout(pinMeasure pinMes) {
    return MeasureBitADC(pinMes) * getPriceBit();
  }

  size_t Meter::getCountSections() { return countSections; }
  
  bool Meter::getfPot() { return fPot; }

  long Meter::MeasureBitADC(pinMeasure pinMes) {
    switch(pinMes) {
      case V_MODE:
      return analogRead(pMeterV[focusSections]);
      break;
    case R_MODE:      
      return analogRead(pMeterRTotal[focusSections]);
      break;
    default:
      return 0;
      break;
    }    
  }

  void Meter::chooseReference(long selectedR) {
    for (long i = 0; i < sizeRChooseReference; i++) {
      delay(10);
      digitalWrite(DChooseReference[i], LOW); 
      pinMode(DChooseReference[i], INPUT);         
    }
    delay(10);
    pinMode(DChooseReference[selectedR], OUTPUT);
    digitalWrite(DChooseReference[selectedR], HIGH);
    delay(10);
  }

  float Meter::measureRTotal() {   
    float raw = 0; 
    float Ru = 0;
    long Rn = 0;
    float min = ADCsignals * 0.3;
	float max = ADCsignals * 0.8;
    for (long i = 1 ; i < sizeRChooseReference && (raw < min || raw > max); i++) {
      delay(100);
      chooseReference(i);
      insideArrVout(R_MODE);
      raw = getAverageVout();
      Rn = RChooseReference[i];
	  #ifdef TestMode
      Serial.print(raw);
      Serial.print("\tref \t");
      Serial.println(Rn);
	  #endif
    }
    if(raw > max) {
      Ru = 0;
      //Serial.println("No contact");
    } else {
      float Vout = vin  * (raw / ADCsignals);
      Ru = (Rn * Vout) / (vin - Vout);
      /*Serial.print("Ru ");
      Serial.println(Ru);*/
    }
    chooseReference(0);
    return Ru;
  }

	float Meter::getCalcilateAVR(float* mas, int size) {
		float sumVout = 0;
		for (long i = 0; i < size; i++) {
		  sumVout += mas[i];
		}
		float rezAverageVout = sumVout / size;
		return rezAverageVout;
	}

	float Meter::getQuadraticAverageVout(float avrVout, float* mas, int size) {	
		float sumDeviantVout = 0;
		for (long i = 0; i < size; i++) {
		  sumDeviantVout += (mas[i] - avrVout) * (mas[i] - avrVout);
		}
		float rezQuadraticAverageVout = sqrt(sumDeviantVout / size);
		return rezQuadraticAverageVout;
	}

	float* Meter::getArrVoutFiltrThreeSigma(float avrVout, float quadrAvrVout, float* mas, int size, int& newSize) {//фильтрация всплесков и просадок
		newSize = 0;
		float* rez = new float[size]{};	
		for (long i = 0; i < size; i++) {
			if((abs(mas[i] - avrVout)) <= (quadrAvrVout * 3)) {
				rez[newSize] = mas[i];
				newSize++;
			}
		}
		return rez;
	}

  bool Meter::insideArrVout(pinMeasure pin) { // фильтра задают количество отрбасываемых элементов с верху и снизу. (те они не участвуют в расчётах среднего, среднего квадратического, и среднего медианного)    
	float averageVoutFiltr = 0;
    float sumDeviantVout = 0;
	float sumDeviantVoutFiltr = 0;
	float quadraticAverageVoutFiltr = 0;	
	int sizeArrVoutFiltr = 0;
	float* arrVoutFiltr = NULL;
	averageVout = 0;
	float needDelay = 20000 / sizeArrVout;
	for (long i = 0; i < sizeArrVout; i++) {
		arrVout[i] = MeasureBitADC(pin);
		delayMicroseconds(needDelay);
		#ifdef TestMode
		Serial.print(i);      
        Serial.print(" ");
        Serial.print(arrVout[i]);
        Serial.print(" !! ");
		#endif
	}

	averageVout = getCalcilateAVR(arrVout, sizeArrVout);
	quadraticAverageVout = getQuadraticAverageVout(averageVout, arrVout, sizeArrVout);

	arrVoutFiltr = getArrVoutFiltrThreeSigma(averageVout, quadraticAverageVout, arrVout, sizeArrVout, sizeArrVoutFiltr);
	averageVoutFiltr = getCalcilateAVR(arrVoutFiltr, sizeArrVoutFiltr);
	quadraticAverageVoutFiltr = getQuadraticAverageVout(averageVoutFiltr, arrVoutFiltr, sizeArrVoutFiltr);
	
	#ifdef TestMode
	Serial.print(" FILTR ");
	for (long i = 0; i < sizeArrVoutFiltr; i++) {		
		Serial.print(i);      
        Serial.print(" ");
        Serial.print(arrVoutFiltr[i]);
        Serial.print(" !! ");
	}
	#endif
	
        #ifdef TestMode
		Serial.print(" !! ");
        Serial.print(" aV ");
        Serial.print(getAverageVout(), 2);
        Serial.print(" !! qAV ");
        Serial.print(quadraticAverageVout, 2);		
		Serial.print(" !! aVf ");
        Serial.print(averageVoutFiltr, 2);
		Serial.print(" !! qAVf ");
        Serial.print(quadraticAverageVoutFiltr, 2);
        Serial.println("\t");
		#endif
		averageVout = averageVoutFiltr;
		quadraticAverageVout = quadraticAverageVoutFiltr;
	delete arrVoutFiltr;
    if (quadraticAverageVout > (ADCsignals * 0.04)) {  //  0.02- занчение порога для vout и 10 для битов. было изменено на 1 % от максимума бит
      fValidation = false;
    } else {
      fValidation = true;
    }
    return fValidation;
  }


  Stepper::Stepper(long pStep, long pDir, long pPower, float reduction, float countOnFull, long timeStop)
    : pStep(pStep), pDir(pDir), pPower(pPower), reduction(reduction), countOnFull(countOnFull), timeStop(timeStop) {
    pinMode(pStep, OUTPUT);
    pinMode(pDir, OUTPUT);
    pinMode(pPower, OUTPUT);
    digitalWrite(pStep, 0);
    digitalWrite(pDir, 0);
    digitalWrite(pPower, 0);
	setPower(false);
  }
  
  Stepper::~Stepper() {
	  setPower(true);
  }

  void Stepper::ZeroingCounterSteps() { counterSteps = 0; }

  float Stepper::getReduction() { return reduction; }

  long Stepper::getPStep() { return pStep; }

  long Stepper::getCounterSteps() { return counterSteps; }
  
  float Stepper::getCounterAngle() { return counterSteps * reduction; }

  bool Stepper::getPower() { return Power; }

  bool Stepper::getDir() { return !Dir; }

  long Stepper::getPDir() { return pDir; }

  long Stepper::getCountStepsOnFull() { return countStepsOnFull; }

  void Stepper::setReduction(long reduction) { this->reduction = reduction; }

  void Stepper::setPower(bool Power) {
    digitalWrite(pPower, Power);
  }

  void Stepper::setTimeStop(long timeStop) {
    this->timeStop = timeStop;
  }

  void Stepper::invDir() {
    Dir = !Dir;
    setDir(Dir);
    //digitalWrite (pDir, !Dir);
  }

  void Stepper::setDir(bool needDir) {  // true по возрастанию
    Dir = needDir;
    digitalWrite(pDir, Dir);       // если подать высокий сигнал то он крутится к vin 
  }

  void Stepper::moveStep() {
    if (!Dir) {
      --counterSteps;
    } else {
      ++counterSteps;
    }
    /*if (counterSteps % 10 == 0) {        
        Serial.print(" CouSteps ");
        Serial.println(counterSteps);
      }*/

    digitalWrite(pStep, 1);
    delayMicroseconds(timeStop);
    digitalWrite(pStep, 0);
    delayMicroseconds(timeStop);
  }

  void Stepper::moveToPosition(const long needPosition) {
    long needsSteps = needPosition - counterSteps;
    if (needsSteps > 0) {
      setDir(true);
      moveSteps(abs(needsSteps));
      /*for (long i = 0; i < needsSteps; i++) {
        moveStep();
      }*/
    } else {
      setDir(false);
	  moveSteps(abs(needsSteps));
      /*for (long i = 0; i > needsSteps; i--) {  
        moveStep();
      }*/
    }
  }

  void Stepper::movePercentagesAngle(long percentagesAngle) {
    /*Serial.print(" moveSteps ");
        Serial.println(numberOfSteps);*/
	long numberOfSteps = percentagesAngle / getReduction();
    //long numberOfSteps = percentagesAngle * 0.01 * getCountStepsOnFull();
    for (long i = 0; i < numberOfSteps; i++) {
      moveStep();
    }
  }

  void Stepper::moveSteps(long numberOfSteps) {
    /*Serial.print(" moveSteps ");
        Serial.println(numberOfSteps);*/
    for (long i = 0; i < numberOfSteps; i++) {
      moveStep();
    }
  }


  Interface::Interface(Stepper* stepper, Meter* meter)
    : stepper(stepper), meter(meter) {
		fPotAvailability = meter->getfPot();
	}
	
  Interface::~Interface() {
  }

  void Interface::test() {
	Serial.println("START TEST");
	stepper->moveToPosition(100);
	Serial.print("st\t");
	Serial.println(stepper->getCounterSteps());
	Serial.println("\t100");
	for (long iterMes = 0; iterMes < 10; iterMes++) {
		Serial.print("st\t");
		Serial.print(stepper->getCounterSteps());
		Serial.print("\t");
		meter->insideArrVout();
		stepper->moveSteps(10);	
	}
	Serial.println("ZERO");
	for (long iterMes = 0; iterMes < 10; iterMes++) {
		Serial.print("st\t");
		Serial.print(stepper->getCounterSteps());
		Serial.print("\t");
		meter->insideArrVout();
		stepper->moveSteps(10);		
	}
	Serial.println("END TEST");
	stepper->moveToPosition(1000);
	delay(1000000);
  }
  
    void Interface::test2() {
	Serial.println("START TEST2");
	stepper->moveToPosition(coordContctPlatformMaxEnd-100);
	Serial.print("st\t");
	Serial.println(stepper->getCounterSteps());
	Serial.println("\t100");
	for (long iterMes = 0; iterMes < 10; iterMes++) {
		Serial.print("st\t");
		Serial.print(stepper->getCounterSteps());
		Serial.print("\t");
		meter->insideArrVout();
		stepper->moveSteps(10);	
	}
	Serial.println("MAX");
	for (long iterMes = 0; iterMes < 10; iterMes++) {
		Serial.print("st\t");
		Serial.print(stepper->getCounterSteps());
		Serial.print("\t");
		meter->insideArrVout();
		stepper->moveSteps(10);		
	}
	Serial.println("END TEST2");
	stepper->moveToPosition(1000);
	delay(1000000);
  }

  long Interface::getCoordContctPlatformMinStrt() { return coordContctPlatformMinStrt; }

  long Interface::getCoordContctPlatformMinEnd() { return coordContctPlatformMinEnd; }

  long Interface::getCoordContctPlatformMaxStrt() { return coordContctPlatformMaxStrt; }

  long Interface::getCoordContctPlatformMaxEnd() { return coordContctPlatformMaxEnd; }

  float Interface::getRTotal() { return meter->getRTotal(); }

  bool Interface::getfPotAvailability() { return fPotAvailability; }

  float Interface::mesU(long sizeSteps, long countMes) {	  // тут что то неладное
	  float sumVout = 0;
	  float tmpVout = 0;
	  float avrVout = 0;	 

	  //Serial.print("\nUVout\n");
	  for (long iterMes = 0; iterMes < countMes; iterMes++) {
		  meter->insideArrVout();
		  tmpVout = meter->getAverageVout();
		  sumVout += tmpVout;
		  stepper->moveSteps(sizeSteps);
		  /*
			#ifdef TestMode
				Serial.print(iterMes);
				Serial.print(tmpVout);
				Serial.print("\t!!\t");
				Serial.print(tmpVout);
			#endif*/
			
	  }
	  avrVout = sumVout / countMes;
	  stepper->invDir();
	  stepper->moveSteps(sizeSteps*countMes);
	  stepper->invDir();
	  
	  //Serial.print("\nENDUVout\n");
      return avrVout;
  }

  long Interface::getCoordinatesBorderAdjAngl(long startVout, long sizeSteps, bool fFast, bool test, float border) {
    meter->insideArrVout();
    //long startVout = meter->getAverageVout();
	//long startVout = meter->getU3();
    float oldVout = startVout;
    float difVout = 0;
    float difVoutStrt = 0;
    long tmp = 0;
    if(fFast) {
      for (long j = 0; sizeSteps > 4; j++) {
        for (long i = 0; startVout == meter->getAverageVout(); i++) {
          stepper->moveSteps(sizeSteps);
          meter->insideArrVout();
        }
        stepper->invDir();
        stepper->moveSteps(sizeSteps * 2);
        stepper->invDir();
        sizeSteps /= 4;
        meter->insideArrVout();
        delay(10);
      }
    } else {
      //for (long i = 0; startVout == meter->getAverageVout(); i++) {
      for (long i = 0; abs(difVoutStrt) <= border; i++) { 
      //for (long i = 0; startVout == meter->getAverageVout() || (test && (abs(difVout) < 1)); i++) {
        stepper->moveSteps(sizeSteps);
        meter->insideArrVout();
        difVout = oldVout - meter->getAverageVout();
        oldVout = meter->getAverageVout();
        difVoutStrt = startVout - meter->getAverageVout();
        if(test) {
          #ifdef TestMode
		  Serial.print("\tstartVout\t");
          Serial.print(startVout);
          Serial.print("\tvout\t");
          Serial.print(meter->getAverageVout());       
          Serial.print("\tdifVoutStrt\t");
          Serial.print(difVoutStrt);      
          Serial.print("\tdifVout\t");
          Serial.print(difVout);          
          Serial.print("\tstep\t");
          Serial.print(stepper->getCounterSteps());
          Serial.print("\tangle\t");
          Serial.println(stepper->getCounterAngle());
		  #endif
        }
      }
    }
    return stepper->getCounterSteps();
  }

  long Interface::getCoordinatesBorderFulAngl(bool fStartValidation, long sizeSteps, bool fFast) {  // будет двигать пока не получит в точке искомую валидацию: true - в зоне. false - не в зоне    
    if(fFast) {
      for (long j = 0; sizeSteps > 4; j++) {
        for (long i = 0; fStartValidation != meter->getfValidation(); i++) {
          stepper->moveSteps(sizeSteps);
          meter->insideArrVout();
        }
        stepper->invDir();
        stepper->moveSteps(sizeSteps * 2);
        stepper->invDir();
        sizeSteps /= 4;
        meter->insideArrVout();
        delay(10);
      }
    } else {
      for (long i = 0; fStartValidation != meter->getfValidation(); i++) {
        stepper->moveSteps(sizeSteps);
        meter->insideArrVout();
		//Serial.println("");
      }
    }
	stepper->invDir();
    stepper->moveSteps(sizeSteps);
	stepper->invDir();
    return stepper->getCounterSteps();
  }

  void Interface::insideCoordContctPlatform() {
    stepper->setDir(false);
    //stepper->setDir(true);
    meter->insideArrVout();
    if (!meter->getfValidation()) {
      Serial.println("f1");
      stepper->movePercentagesAngle(10);
      meter->insideArrVout();
      if (!meter->getfValidation()) {
        Serial.println("f2");
        stepper->invDir();
        stepper->movePercentagesAngle(20);
        stepper->invDir();
        meter->insideArrVout();
        if (!meter->getfValidation()) {
          Serial.println("NO IN ZONE");   // не в зоне.
        }
      }
    } else {
      #ifdef TestMode
	    Serial.println("\ncoordConPlatMinStrt getCoordinatesBorderFulAngl");
	  #endif
      getCoordinatesBorderFulAngl(false);
      //Serial.println("\neap_");
      stepper->ZeroingCounterSteps();
      //Serial.println("\neap__");
      delay(1000);
      stepper->invDir();
      #ifdef TestMode
		Serial.println("\ncoordConPlatmMinEnd getCoordinatesBorderAdjAngl");
	  #endif
      stepper->movePercentagesAngle(1);
	  Serial.println("\nU1");
      meter->setU1(mesU());
      delay(1000);
	  
	  float bordValU1 = meter->getADCsignals() * 0.001;
	  float bordValU3 = meter->getADCsignals() * 0.011;
      coordContctPlatformMinEnd = getCoordinatesBorderAdjAngl(meter->getU1(), 10, false, true, bordValU1);
      #ifdef TestMode
		Serial.println("\ncoordConPlatMaxEnd getCoordinatesBorderFulAngl");
	  #endif
      delay(1000);
      coordContctPlatformMaxEnd = getCoordinatesBorderFulAngl(false);
      #ifdef TestMode
		Serial.println("\ncoordConPlatMaxStrt getCoordinatesBorderAdjAngl");
	  #endif
      delay(1000);
      stepper->invDir();
	  
	  stepper->movePercentagesAngle(1);	
	  
      /*stepper->movePercentagesAngle(9.7);	  
	  Serial.println("\nOP OP OP");
	  meter->insideArrVout();
	  meter->insideArrVout();
	  meter->insideArrVout();
	  meter->insideArrVout();
	  meter->insideArrVout();
	  stepper->invDir();
	  stepper->movePercentagesAngle(9);
	  stepper->invDir(); 
	  Serial.println("\nOP OP OP");
	  */
	  Serial.println("\nU3");
	  meter->setU3(mesU());	  
	  delay(1000);
      coordContctPlatformMaxStrt = getCoordinatesBorderAdjAngl(meter->getU3(), 10, false, true, bordValU3);
      fCoordContctPlatform = true;
    }
	#ifdef TestMode
		Serial.print("U1\t");
        Serial.print(meter->getU1());
        Serial.print("\tU3\t");
        Serial.println(meter->getU3());
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinStrt * stepper->getReduction());
        Serial.print("CCPMiE\t");
        Serial.print(coordContctPlatformMinEnd);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinEnd * stepper->getReduction());
        Serial.print("CCPMaS\t");
        Serial.print(coordContctPlatformMaxStrt);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxStrt * stepper->getReduction());
        Serial.print("CCPMaE\t");
        Serial.print(coordContctPlatformMaxEnd);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxEnd * stepper->getReduction());
        Serial.print("needSTbordMaxStrt ");
        Serial.println(coordContctPlatformMaxEnd - 711); //вычисленное количество шагов для 10 градусов (контактная площадка)		
		Serial.print(" PPcont 1.1 ");
		Serial.println((getCoordContctPlatformMinEnd() - getCoordContctPlatformMinStrt()) * stepper->getReduction() - 0.7);  
		Serial.print(" PPcont 1.2 ");
		Serial.println((getCoordContctPlatformMaxEnd() - getCoordContctPlatformMaxStrt()) * stepper->getReduction() - 0.7);		
	#endif
	
	// correction
	coordContctPlatformMaxStrtV2 = coordContctPlatformMaxStrt;
	coordContctPlatformMinEndV2 = coordContctPlatformMinEnd;
	long needCorection = (coordContctPlatformMaxEnd - 711 * 2) * 0.01;
	coordContctPlatformMaxStrt += needCorection;
	coordContctPlatformMinEnd -= needCorection;
	
	#ifdef TestMode
		Serial.print("\n CORRECT \n");
		Serial.print("U1\t");
        Serial.print(meter->getU1());
        Serial.print("\tU3\t");
        Serial.println(meter->getU3());
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinStrt * stepper->getReduction());
        Serial.print("CCPMiE\t");
        Serial.print(coordContctPlatformMinEnd);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinEnd * stepper->getReduction());
        Serial.print("CCPMaS\t");
        Serial.print(coordContctPlatformMaxStrt);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxStrt * stepper->getReduction());
        Serial.print("CCPMaE\t");
        Serial.print(coordContctPlatformMaxEnd);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxEnd * stepper->getReduction());
        Serial.print("needSTbordMaxStrt ");
        Serial.println(coordContctPlatformMaxEnd - 711); //вычисленное количество шагов для 10 градусов (контактная площадка)
		Serial.print("CCPMiE_V2\t");
        Serial.print(coordContctPlatformMinEndV2);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinEndV2 * stepper->getReduction());
        Serial.print("CCPMaS_V2\t");
        Serial.print(coordContctPlatformMaxStrtV2);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxStrtV2 * stepper->getReduction());
	#endif
	
	
    delay(1000);
  }


  void Interface::insideCoordContctPlatformV2() {
	  int pRele1 = 22;
	  int pRele2 = 24;
	  pinMode(pRele1, OUTPUT);
	  pinMode(pRele2, OUTPUT);
    stepper->setDir(false);
    //stepper->setDir(true);
    meter->insideArrVout();
    if (!meter->getfValidation()) {
      Serial.println("f1");
      stepper->movePercentagesAngle(10);
      meter->insideArrVout();
      if (!meter->getfValidation()) {
        Serial.println("f2");
        stepper->invDir();
        stepper->movePercentagesAngle(20);
        stepper->invDir();
        meter->insideArrVout();
        if (!meter->getfValidation()) {
          Serial.println("NO IN ZONE");   // не в зоне.
        }
      }
    } else {
      #ifdef TestMode
	    Serial.println("\nV2coordConPlatMinStrt getCoordinatesBorderFulAngl");
	  #endif
      getCoordinatesBorderFulAngl(false);
      //Serial.println("\neap_");
      stepper->ZeroingCounterSteps();
      //Serial.println("\neap__");
      delay(1000);
      stepper->invDir();
      #ifdef TestMode
		Serial.println("\nV2coordConPlatmMinEnd getCoordinatesBorderAdjAngl");
	  #endif
      stepper->movePercentagesAngle(1);
	  Serial.println("\nU1");
      meter->setU1(mesU());
      delay(1000);
      coordContctPlatformMinEnd = getCoordinatesBorderAdjAngl(meter->getU1(), 10, false, true, 50);
	  
	  
	  // тут надо переключить полярность   
	  delay(1000);	
	  digitalWrite(pRele1, LOW);
	  digitalWrite(pRele2, HIGH);
	  delay(1000);	
      #ifdef TestMode
		Serial.println("\nV2coordConPlatMaxEnd getCoordinatesBorderFulAngl");
	  #endif
	  	  
      delay(1000);
      coordContctPlatformMaxEnd = getCoordinatesBorderFulAngl(false);
	  	  
      #ifdef TestMode
		Serial.println("\nV2coordConPlatMaxStrt getCoordinatesBorderAdjAngl");
	  #endif
	  
      delay(1000);	  
	  stepper->invDir();
	  stepper->movePercentagesAngle(1);	 	  
	  meter->setU3(mesU());	  
	  delay(1000);	  

      coordContctPlatformMaxStrt = getCoordinatesBorderAdjAngl(meter->getU3(), 10, false, true, 50);
	  // тут надо переключить полярность обратно
	  delay(1000);	
	  digitalWrite(pRele1, HIGH);
	  digitalWrite(pRele2, LOW);
	  delay(1000);	
	  
	  stepper->moveToPosition(coordContctPlatformMaxStrt);
	  stepper->invDir();
	  stepper->movePercentagesAngle(1);	 	  
	  Serial.println("\nU3");
	  meter->setU3(mesU());	  
	  delay(1000);
	  
      fCoordContctPlatform = true;
    }
	#ifdef TestMode
		Serial.print("U1\t");
        Serial.print(meter->getU1());
        Serial.print("\tU3\t");
        Serial.println(meter->getU3());
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinStrt * stepper->getReduction());
        Serial.print("CCPMiE\t");
        Serial.print(coordContctPlatformMinEnd);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinEnd * stepper->getReduction());
        Serial.print("CCPMaS\t");
        Serial.print(coordContctPlatformMaxStrt);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxStrt * stepper->getReduction());
        Serial.print("CCPMaE\t");
        Serial.print(coordContctPlatformMaxEnd);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxEnd * stepper->getReduction());
        Serial.print("needSTbordMaxStrt ");
        Serial.println(coordContctPlatformMaxEnd - 711); //вычисленное количество шагов для 10 градусов (контактная площадка)
		Serial.print(" PPcont 1.1 ");
		Serial.println((getCoordContctPlatformMinEnd() - getCoordContctPlatformMinStrt()) * stepper->getReduction() - 0.7);  
		Serial.print(" PPcont 1.2 ");
		Serial.println((getCoordContctPlatformMaxEnd() - getCoordContctPlatformMaxStrt()) * stepper->getReduction() - 0.7);
	#endif
	
	// correction 
	long needCorection = (coordContctPlatformMaxEnd - 711 * 2) * 0.01;
	coordContctPlatformMaxStrt += needCorection;
	coordContctPlatformMinEnd -= needCorection;
	
	#ifdef TestMode
		Serial.print("\n CORRECT \n");
		Serial.print("U1\t");
        Serial.print(meter->getU1());
        Serial.print("\tU3\t");
        Serial.println(meter->getU3());
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinStrt * stepper->getReduction());
        Serial.print("CCPMiE\t");
        Serial.print(coordContctPlatformMinEnd);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMinEnd * stepper->getReduction());
        Serial.print("CCPMaS\t");
        Serial.print(coordContctPlatformMaxStrt);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxStrt * stepper->getReduction());
        Serial.print("CCPMaE\t");
        Serial.print(coordContctPlatformMaxEnd);
        Serial.print("\tangle\t");
        Serial.println(coordContctPlatformMaxEnd * stepper->getReduction());
        Serial.print("needSTbordMaxStrt ");
        Serial.println(coordContctPlatformMaxEnd - 711); //вычисленное количество шагов для 10 градусов (контактная площадка)
	#endif

    delay(1000);
  }

  float Interface::getAdjAngl() {
    float rez = (coordContctPlatformMaxStrt - coordContctPlatformMinEnd) * stepper->getReduction();
    /*Serial.print("AA ");
    Serial.println(rez);*/
    return rez;
  }

  float Interface::getFullAdjAngl() {
    float rez = coordContctPlatformMaxEnd * stepper->getReduction();
    /*Serial.print(" FAA ");
    Serial.println(rez);*/
    return rez;
  }

  float Interface::getFChV2(const long numbPoints) {
    if(!fCoordContctPlatform) {
      insideCoordContctPlatform();
    }
    long stepAdjAngl = coordContctPlatformMaxStrtV2 - coordContctPlatformMinEndV2; //надо тут поменять координаты для Изменение нахождения ФХ
    /*Serial.print(" stepAdjAngl "); 			//coordContctPlatformMaxStrt надо поменять
    Serial.println(stepAdjAngl);*/
    float FCh = 0;
    float averVout = 0;
    float teorVout = 0;
    float minDif = 0;
    float maxDif = 0;
    float* mDiferenceVout = new float[numbPoints]{};
    float sizeStep = float(meter->getU3() - meter->getU1()) / stepAdjAngl;     // теоритически это - вольт на 1 шаг
    long stepp = stepAdjAngl / (numbPoints - 1);  // количество шагов  между измерениями . (нет проверки на деление на ноль)

    /*Serial.print(" stepp ");
    Serial.println(stepp);
    Serial.print(" sizeStep ");
    Serial.println(sizeStep);*/

    stepper->moveToPosition(coordContctPlatformMinEndV2); //надо тут поменять координаты для Изменение нахождения ФХ
    delay(10);											//надо поменять coordContctPlatformMinEnd
    stepper->setDir(true);

    for (long i = 0; i < numbPoints; i++) {
      meter->insideArrVout();
      averVout = meter->getAverageVout();
      teorVout = ((float(stepper->getCounterSteps()) - float(coordContctPlatformMinEndV2)) * sizeStep) - meter->getU1();
      mDiferenceVout[i] = averVout - teorVout;
     #ifdef TestMode
		Serial.print(" iter\t");
        Serial.print(i);
        Serial.print("\tav\t");
        Serial.print(averVout);
        Serial.print("\tst\t");
        Serial.print(stepper->getCounterSteps() - coordContctPlatformMinEndV2);
        Serial.print("\ttv\t");
        Serial.print(' ');
        Serial.print(teorVout);
        Serial.print("\tmDif\t");
        Serial.println(mDiferenceVout[i]);
	#endif
      stepper->moveSteps(stepp);
    }

    delay(10);
    // найти максимум и минимум
    minDif = mDiferenceVout[0];
    //maxDif = mDiferenceVout[0];

    for (long i = 0; i < numbPoints; i++) {
      if (mDiferenceVout[i] > maxDif) {
        maxDif = mDiferenceVout[i];
      } else if (mDiferenceVout[i] < minDif) {
        minDif = mDiferenceVout[i];
      }
    }

    Serial.print("\tMaxDif\t");
    Serial.print(maxDif);
    Serial.print("\tMinDif\t");
    Serial.println(minDif);

    FCh = (maxDif - minDif) * 100.0 / (2.0 * (float(meter->getU3()) - float(meter->getU1())));

    /*Serial.print("\tFCh\t");
    Serial.println(FCh);*/
    delete[] mDiferenceVout;
    delay(10);
    return FCh;
  }

  float Interface::getFCh(const long numbPoints) {
    if(!fCoordContctPlatform) {
      insideCoordContctPlatform();
    }
    long stepAdjAngl = coordContctPlatformMaxStrt - coordContctPlatformMinEnd; 
    /*Serial.print(" stepAdjAngl "); 			
    Serial.println(stepAdjAngl);*/
    float FCh = 0;
    float averVout = 0;
    float teorVout = 0;
    float minDif = 0;
    float maxDif = 0;
    float* mDiferenceVout = new float[numbPoints]{};
    float sizeStep = float(meter->getU3() - meter->getU1()) / stepAdjAngl;     // теоритически это - вольт на 1 шаг
    long stepp = stepAdjAngl / (numbPoints - 1);  // количество шагов  между измерениями . (нет проверки на деление на ноль)

    Serial.print(" stepp ");
    Serial.println(stepp);
    Serial.print(" sizeStep ");
    Serial.println(sizeStep);

    stepper->moveToPosition(coordContctPlatformMinEnd);
    delay(10);											
    stepper->setDir(true);

    for (long i = 0; i < numbPoints; i++) {
      meter->insideArrVout();
      averVout = meter->getAverageVout();
      teorVout = ((float(stepper->getCounterSteps()) - float(coordContctPlatformMinEnd)) * sizeStep) - meter->getU1();
      mDiferenceVout[i] = averVout - teorVout;
     #ifdef TestMode
		Serial.print(" iter\t");
        Serial.print(i);
        Serial.print("\tav\t");      
        Serial.print(averVout);
        Serial.print("\tst\t");
        Serial.print(stepper->getCounterSteps() - coordContctPlatformMinEnd);
        Serial.print("\ttv\t");
        Serial.print(' ');
        Serial.print(teorVout);
        Serial.print("\tmDif\t");
        Serial.println(mDiferenceVout[i]);
	#endif
	  delay(100);
      stepper->moveSteps(stepp);
    }

    delay(10);
    // найти максимум и минимум
    minDif = mDiferenceVout[0];
    //maxDif = mDiferenceVout[0];

    for (long i = 0; i < numbPoints; i++) {
      if (mDiferenceVout[i] > maxDif) {
        maxDif = mDiferenceVout[i];
      } else if (mDiferenceVout[i] < minDif) {
        minDif = mDiferenceVout[i];
      }
    }

    Serial.print("\tMaxDif\t");
    Serial.print(maxDif);
    Serial.print("\tMinDif\t");
    Serial.println(minDif);

    FCh = (maxDif - minDif) * 100.0 / (2.0 * (float(meter->getU3()) - float(meter->getU1())));

    /*Serial.print("\tFCh\t");
    Serial.println(FCh);*/
    delete[] mDiferenceVout;
    delay(10);
    return FCh;
  }
  
  

  long Interface::moveToVout(const float needVout, bool fAllowance) {	
	float needSt = needVout / meter->getADCsignals() * getCoordContctPlatformMaxEnd();
    //stepper->moveToPosition(getCoordContctPlatformMaxEnd() / 2);
	
	Serial.print(" needSt\t");
    Serial.println(needSt);
	
	stepper->moveToPosition(needSt);
	
    meter->insideArrVout();

    float averVout = meter->getAverageVout();
    Serial.print(" need search vout ");
    Serial.println(needVout);

    Serial.print(" before search vout ");
    Serial.println(averVout);

    if (needVout > averVout) {
      stepper->setDir(true);
    } else {
      stepper->setDir(false);
    }

    if (fAllowance) {
      float allowance = needVout * 2 * 0.001;
      while ((averVout < (needVout - allowance) || (averVout > (needVout + allowance))) && meter->insideArrVout()) {
        averVout = meter->getAverageVout();
        stepper->moveStep();
      }
      stepper->invDir();	  
	  stepper->moveSteps(10);
	  stepper->invDir();
	  delay(100);
	  averVout = meter->getAverageVout();
	  while ((averVout != needVout) && meter->insideArrVout()) {
        averVout = meter->getAverageVout();
        stepper->moveStep();
      }
	  
	  
    } else {
      while ((averVout != needVout) && meter->insideArrVout()) {
        averVout = meter->getAverageVout();
        stepper->moveSteps(10);
      }
	  stepper->invDir();	  
	  stepper->moveSteps(10);
	  stepper->invDir();
	  delay(100);	  
	  averVout = meter->getAverageVout();
	  while ((averVout != needVout) && meter->insideArrVout()) {
        averVout = meter->getAverageVout();
        stepper->moveStep();
      }
	  
    }

    Serial.print(" vout ");
    Serial.println(averVout);
    return stepper->getCounterSteps();
  }

  float Interface::getNonlinearity() {  // нелинейность . надо дописасать
    stepper->moveToPosition(coordContctPlatformMinEnd);
  }

  bool Interface::sendMeasureVout(long stepFrequency) {  // двигает шаговик, отправляет данные на компьютер. всего данных будет столькоже, сколько шагов у двигателя по модулю
    float mv = -1;
    for (long i = abs(stepper->getCounterSteps()); abs(stepper->getCounterSteps()) < i + stepFrequency;) {
      mv = meter->MeasureVout();
      String Mes = "$" + String(abs(stepper->getCounterSteps())) + "/" + String(mv, 4) + "#";
      Serial.println(Mes);  //отправляем элемент [iter]  на компьютер
      stepper->moveStep();
    }
    return true;
  }
  
  void Timer::zeroingTimer() {
    timeStart = millis();
  }

  void Timer::setTimer(unsigned long val) {
    needTime = val;
  }

  bool Timer::checkTimer() {
    if((millis() - timeStart) > needTime) {
      return true;
    } else {
      return false;
    }
  }    

