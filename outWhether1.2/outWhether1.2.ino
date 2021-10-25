#include <GyverPower.h>
#define millis() (millis() << (CLKPR & 0xF))
#define micros() (micros() << (CLKPR & 0xF))
#define delay(x) delay((x) >> (CLKPR & 0xf))
#define delayMicroseconds(x) delayMicroseconds((x) >> (CLKPR & 0xf))

// адрес BME280 жёстко задан в файле библиотеки Adafruit_BME280.h
// стоковый адрес был 0x77, у китайского модуля адрес 0x76.
// Так что если юзаете не библиотеку из архива - не забудьте поменять
#include <GyverBME280.h>
GyverBME280 bme;

//#define DEBUG_ENABLE 1

// библиотеки
#include <EEPROM.h>
#include <microWire.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10


//#define CALIBRATE_VCC 1 /* Need to save the correct voltage constant to the EEPROM, to get it later we need to call restoreConstant(adr)**/
#include <GyverHacks.h>

#define VCC_CONST_ADR 100

//------- Debuger --------
#if (DEBUG_ENABLE)
#define DEBUGLN(x) Serial.println(x)
#define DEBUG(x) Serial.print(x)
#else
#define DEBUGLN(x)
#define DEBUG(x)
#endif


/*
  Характеристики датчика BME:
  Температура: от-40 до + 85 °C
  Влажность: 0-100%
  Давление: 300-1100 hPa (225-825 ммРтСт)
  Разрешение:
  Температура: 0,01 °C
  Влажность: 0.008%
  Давление: 0,18 Pa
  Точность:
  Температура: +-1 °C
  Влажность: +-3%
  Давление: +-1 Па
*/
#define SLEEP_CORRECTION 10
#define GET_TEMP_DELAY   20     //milliseconds
#define LONG_SLEEP       600000 - SLEEP_CORRECTION //milliseconds
#define SHORT_SLEEP      30000  //milliseconds

//#define VCC_PERIOD       (long)30*60*1000

#define TEMP_FIX_VALUE   38

byte shortSleep = 10;
int transmit_data[6]; // массив, хранящий передаваемые данные
byte address[][6] = {"1Node", "2Node"}; //возможные номера труб

byte delayCorection = 0;
boolean resistorsFlag = false;
boolean chargingFlag = false;
int vcc;

/**
   Setup
*/
void setup() {
  power.setSystemPrescaler(PRESCALER_2);   // замедляем в 2 разa

  //enable bme power
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

  //enable nRF24L01 power
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);

  //pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(7, OUTPUT);
  //digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(7, LOW);


#if (DEBUG_ENABLE || CALIBRATE_VCC)
  Serial.begin(9600 * 2L);
#endif


#ifdef CALIBRATE_VCC
  constantWizard();
#endif

#ifdef VCC_CONST_ADR
  restoreConstant(VCC_CONST_ADR);
#endif

  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_HIGH); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  //radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик

  delay(20);
  vcc = voltageRunningAverage(getVCC(), true);

  bme.setStandbyTime(STANDBY_1000MS);
  bme.setMode(FORCED_MODE);
  bme.begin();

  power.hardwareDisable(PWR_USB | PWR_TIMER1 | PWR_TIMER2 | PWR_TIMER3 | PWR_TIMER4 | PWR_TIMER5 | PWR_UART0 | PWR_UART1 | PWR_UART2 | PWR_UART3);
  //power.hardwareDisable(PWR_USB | PWR_TIMER1 | PWR_TIMER2 | PWR_TIMER3 | PWR_TIMER4 | PWR_TIMER5);
  power.setSleepMode(POWERDOWN_SLEEP);

  power.calibrate(power.getMaxTimeout());
}

/**
   Loop
*/
void loop() {
  readSensors();
}

/**
   Read sensors
*/
void readSensors() {
  unsigned long scriptStartTime = millis();

  /*
    static unsigned long tmr1;
    if (scriptStartTime - tmr1 >= VCC_PERIOD) {
      vcc = voltageRunningAverage(getVCC(), false);
      do {
        tmr1 += VCC_PERIOD;
        if (tmr1 < VCC_PERIOD) break;  // переполнение uint32_t
      } while (tmr1 < scriptStartTime - VCC_PERIOD); // защита от пропуска шага
    }
  */


  int temp = ((GetTemp() + 0.05) * 10) + TEMP_FIX_VALUE; //round and convert to int

  if (transmit_data[4]) {
    int tempDifference = temp - transmit_data[4];
    if (tempDifference < 0)
    {
      tempDifference = -tempDifference;
    }
    if (tempDifference <= 5) {
      transmit_data[4] = temp;
    } else {
      transmit_data[4] = (float)(( transmit_data[4] * 2 + temp) / 3);
    }
  } else {
    transmit_data[4] = temp;
  }

  //transmit_data[4] = 200;

  vcc = voltageRunningAverage(getVCC(), false);
  transmit_data[3] = vcc;

  transmit_data[5] = ((float)vcc * analogRead(A2) / 1023);

  //transmit_data[5] = getVoltage(A2);

  //transmit_data[3] = getVoltage(A1);


  if (transmit_data[5] > 900) {
    if (transmit_data[4] <= 100) {
      pinMode(5, OUTPUT);
      digitalWrite(5, HIGH);

      resistorsFlag = true;

      if (chargingFlag) {
        digitalWrite(7, LOW);
        pinMode(7, INPUT);
        chargingFlag = false;
      }
    } else {
      pinMode(7, OUTPUT);
      digitalWrite(7, HIGH);

      chargingFlag = true;

      if (resistorsFlag) {
        digitalWrite(5, LOW);
        pinMode(5, INPUT);
        resistorsFlag = false;
      }

    }
  }

  if (shortSleep > 9) {

    //enable bme power
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);

    bme.oneMeasurement();


    int count = 500;
    byte tCount = 0;
    while (bme.isMeasuring() && tCount < 10)
    {
      count--;
      if (count <= 0) {
        bme.oneMeasurement();
        count = 500;
        tCount++;
      }
    }

    if (tCount < 10) {
      radio.powerUp(); // включить передатчик
      myDelayMicroseconds(2);

      transmit_data[0] = (bme.readTemperature() + 0.05) * 10; //round and convert to int
      transmit_data[1] = bme.readHumidity() + 0.5; //round
      transmit_data[2] = pressureToMmHg(bme.readPressure()) + 0.5; //round

      radio.write(&transmit_data, sizeof(transmit_data));

      radio.powerDown(); // выключить передатчик

      if (radio.failureDetected) {
        radio.begin();                // Attempt to re-configure the radio with defaults
        radio.failureDetected = 0;    // Reset the detection value

        radio.setAutoAck(1);
        radio.setRetries(0, 15);
        radio.enableAckPayload();
        radio.setPayloadSize(32);

        radio.openWritingPipe(address[0]);
        radio.setChannel(0x60);

        radio.setPALevel (RF24_PA_HIGH);
        radio.setDataRate (RF24_1MBPS);

        radio.stopListening();
      }
    }

    digitalWrite(8, LOW);
    pinMode(8, INPUT);
  }


  int sunVcc = ((float)vcc * analogRead(A2) / 1023);
  if (sunVcc <= 900) {
    if (resistorsFlag) {
      digitalWrite(5, LOW);
      pinMode(5, INPUT);
      resistorsFlag = false;
    }

    if (chargingFlag) {
      digitalWrite(7, LOW);
      pinMode(7, INPUT);
      chargingFlag = false;
    }

    long shortSleepTime;
    byte shortSleepMultiplier = 10 - shortSleep;
    if (shortSleepMultiplier > 0) {
      shortSleepTime = (long)shortSleepMultiplier * SHORT_SLEEP;
    } else {
      shortSleepTime = 0;
    }

    delayCorection = power.sleepDelay(((LONG_SLEEP + delayCorection) - shortSleepTime) - (millis() - scriptStartTime));
    shortSleep = 10;
    //power.sleepDelay(60000);
  } else {
    shortSleep--;
    if (shortSleep < 1) {
      shortSleep = 10;
    }
    delayCorection = power.sleepDelay((SHORT_SLEEP + delayCorection) - (millis() - scriptStartTime));
  }



#if (DEBUG_ENABLE)

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'B')
    {
      DEBUGLN("*** CHARGING BATTARY \n\r");


      digitalWrite(7, HIGH);
    }
    else if ( c == 'R')
    {
      DEBUGLN("*** HEAT RESISTORS\n\r");



      digitalWrite(5, HIGH);
    }

    else if ( c == '1')
    {
      DEBUGLN("*** FULL CHARGE\n\r");


      digitalWrite(4, HIGH);
    }
    else if ( c == '0')
    {
      DEBUGLN("*** SLOW CHARGE\n\r");

      digitalWrite(4, LOW);
    }
    else if ( c == 'S')
    {
      DEBUGLN("*** STOP ALL\n\r");

      digitalWrite(5, LOW);
      digitalWrite(7, LOW);
    }

    else if ( c == 'Z')
    {
      digitalWrite(8, LOW);
      pinMode(8, INPUT);
      digitalWrite(6, LOW);
      pinMode(6, INPUT);

      //digitalWrite(4, LOW);
      digitalWrite(5, LOW);
      digitalWrite(7, LOW);
      //pinMode(4, INPUT);
      pinMode(5, INPUT);
      pinMode(7, INPUT);
      power.sleepDelay(60000);
      //enable bme power
      pinMode(8, OUTPUT);
      digitalWrite(8, HIGH);

      //enable nRF24L01 power
      pinMode(6, OUTPUT);
      digitalWrite(6, HIGH);

      //pinMode(4, OUTPUT);
      pinMode(5, OUTPUT);
      pinMode(7, OUTPUT);
    }
    else if ( c == 'I')
    {

      DEBUG(transmit_data[0]); DEBUGLN(" T");
      DEBUG(transmit_data[1]); DEBUGLN(" H");
      DEBUG(transmit_data[2]); DEBUGLN(" P");
      DEBUG(transmit_data[3]); DEBUGLN(" mV");
      DEBUG(transmit_data[4]); DEBUGLN(" t");
      DEBUG(transmit_data[5]); DEBUGLN(" mV");
      DEBUGLN("--------------");
    }
  }
#endif
}

double GetTemp(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(GET_TEMP_DELAY);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  int tCount = 0;
  // Detect end-of-conversion
  while (bit_is_set(ADCSRA, ADSC) && tCount < 1000) {
    tCount++;
  }


  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);
}

/**
   Delay method
*/
void myDelayMicroseconds(unsigned long us) {
  unsigned long tmr = micros();
  while (micros() - tmr < us);
}

int voltageRunningAverage(int newVal, boolean firstCall) {
  float k = 0.1;  // коэффициент фильтрации, 0.0-1.0

  static int filVal;

  if (firstCall) {
    filVal = newVal;
    return filVal;
  }
  filVal += (newVal - filVal) * k;
  return filVal;
}
