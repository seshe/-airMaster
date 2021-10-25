/*
  Скетч к проекту "Домашняя метеостанция"
  Страница проекта (схемы, описания): https://alexgyver.ru/meteoclock/
  Исходники на GitHub: https://github.com/AlexGyver/MeteoClock https://github.com/Norovl/meteoClock
  Автор: AlexGyver Technologies, 2018 /  Роман Новиков (с)НР
  http://AlexGyver.ru/
*/

/*
  Время и дата устанавливаются атвоматически при загрузке прошивки (такие как на компьютере)
  График всех величин за час и за сутки (усреднённые за каждый час)
  В модуле реального времени стоит батарейка, которая продолжает отсчёт времени после выключения/сброса питания
  Как настроить время на часах. У нас есть возможность автоматически установить время на время загрузки прошивки, поэтому:
  - Ставим настройку RESET_CLOCK на 1
  - Прошиваемся
  - Сразу ставим RESET_CLOCK 0
  - И прошиваемся ещё раз
  - Всё
*/

//#define DEBUG_ENABLE 1

// библиотеки
#include <GyverTimers.h>  // Либа прерываний по таймеру
#include "FastIO.h"       // Либа быстрого ввода/вывода
volatile byte dutyA0 = 0; // Переменные для хранения заполнения ШИМ

#define PWM_DEPTH 15      // Предел счета таймера, определяет разрешение ШИМ
// Можно выбрать от 2 до 254 (больше нельзя)
// В данном случае ШИМ имеет пределы 0 - 15 (4 бита, 16 градаций)

#include <EEPROM.h>
#include <microWire.h>

//#define CALIBRATE_VCC 1 /* Need to save the correct voltage constant to the EEPROM, to get it later we need to call restoreConstant(adr)**/
#include <GyverHacks.h>

#define VCC_CONST_ADR 100

// если дисплей не заводится - поменяйте адрес (строка 54)
#include <microLiquidCrystal_I2C.h>
#define DISPLAY_ADDR 0x27 // адрес платы дисплея: 0x27 или 0x3f. Если дисплей не работает - смени адрес! На самом дисплее адрес не указан
LiquidCrystal_I2C lcd(DISPLAY_ADDR, 20, 4);

//
#include <microDS3231.h>
MicroDS3231 rtc;
//#include <buildTime.h>

// адрес BME280 жёстко задан в файле библиотеки Adafruit_BME280.h
// стоковый адрес был 0x77, у китайского модуля адрес 0x76.
// Так что если юзаете не библиотеку из архива - не забудьте поменять
#include <GyverBME280.h>
GyverBME280 bme;

//BH1750
#define BH1750_SENSOR 1      // включить или выключить поддержку/вывод с датчика освещения BH1750 (1 вкл, 0 выкл) 
#if (BH1750_SENSOR == 1)
#include <BH1750.h>
BH1750 lightMeter;

boolean luxFlag = false; // show or hide Lux value on the main screen (could be changed on the first plot by triple click)
#endif

//MHZ19
#define CO2_SENSOR 1      // включить или выключить поддержку/вывод с датчика СО2 (1 вкл, 0 выкл)
#if (CO2_SENSOR == 1)
#define MHZ_RX 2
#define MHZ_TX 3

#include <MHZ19_uart.h>
MHZ19_uart mhz19;
#endif

#include <GyverButton.h>
//Button pin
#define BTN_PIN 4

GButton button(BTN_PIN, LOW_PULL, NORM_OPEN);

//Timer
#include <timer2Minim.h>

#define RADIO 1
#if (RADIO == 1)
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(10, 8);

int transmit_data[6]; // массив, хранящий передаваемые данные
byte address[][6] = {"1Node", "2Node"}; //возможные номера труб
#endif

//------- Debuger --------
#if (DEBUG_ENABLE)
#define DEBUGLN(x) Serial.println(x)
#define DEBUG(x) Serial.print(x)
#else
#define DEBUGLN(x)
#define DEBUG(x)
#endif

// ------------------------- НАСТРОЙКИ --------------------
#define RESET_CLOCK 0     // сброс часов на время загрузки прошивки (для модуля с несъёмной батарейкой). Не забудь поставить 0 и прошить ещё раз!
#define SENS_TIME 30000   // время обновления показаний сенсоров на экране, миллисекунд

byte powerStatus = 1;         // индикатор вида питания: 255 - не показывать, остальное автоматически (0 - от сети, 1-5 уровень заряда батареи) (с)НР

#define PRESSURE 0        // 0 - график давления, 1 - график прогноза дождя (вместо давления). Не забудь поправить пределы графика

// пределы для индикатора (с)НР
#define normCO2 900       // и выше - желтый
#define maxCO2 1200       // и выше - красный
#define blinkLEDCO2 1500  // Значение СО2, при превышении которого будет мигать индикатор

//temp /10 as converted to int
#define minTemp 210        // и ниже для синего индикатора
#define normTemp 260       // и выше - желтый
#define maxTemp 280        // и выше - красный
#define blinkLEDTemp 350   // Значение температуры, при превышении которой будет мигать индикатор

#define maxHum 90         // и выше - синий
#define normHum 30        // и ниже - желтый
#define minHum 20         // и ниже - красный
#define blinkLEDHum 15    // Значение влажности, при показаниях ниже порога которого будет мигать индикатор

#define normPress 733     // и ниже - желтый
#define minPress 728      // и ниже - красный   // может, переделать на синий?

#define minRain -50       // и ниже - красный   
#define normRain -20      // и ниже - желтый
#define maxRain 50        // и выше - синий

// яркость
#define LED_BRIGHT_MAX 200    // макс яркость светодиода СО2 (0 - 255)
#define LED_BRIGHT_MIN 10     // мин яркость светодиода СО2 (0 - 255)
//#define LCD_BRIGHT_MAX 200    // макс яркость подсветки дисплея (0 - 255)
//#define LCD_BRIGHT_MIN 10     // мин яркость подсветки дисплея (0 - 255)

#define MOTION_CONTROL 0       // 0/1 - запретить/разрешить выключение и включение по датчику движения ( при запрете датчик движения не используется )

#define BATTERY_PIN A1
#define POWER_PIN A2

// управление яркостью
byte LED_BRIGHT = 5;         // при отсутствии сохранения в EEPROM: яркость светодиода СО2 (0 - 10) (коэффициент настраиваемой яркости индикатора по умолчанию, если нет сохранения и не автоматическая регулировка (с)НР)
byte LCD_BRIGHT = 5;         // при отсутствии сохранения в EEPROM: яркость экрана (0 - 10) (коэффициент настраиваемой яркости экрана по умолчанию, если нет сохранения и не автоматическая регулировка (с)НР)

byte LEDType = 0;         //  при отсутствии сохранения в EEPROM: привязка индикатора к датчикам: 0 - СО2, 1 - Влажность, 2 - Температура, 3 - Осадки

// яркость
byte LIGHT;
#define LIGHT_MIN 0
#define LIGHT_MAX 255

int MAX_ONDATA = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024 + 2048; // при отсутствии сохранения в EEPROM: максимальные показания графиков исходя из накопленных фактических (но в пределах лимитов) данных вместо указанных пределов, 0 - использовать фиксированные пределы (с)НР
int VIS_ONDATA = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512 + 1024 + 2048; // при отсутствии сохранения в EEPROM: отображение показания графиков, 0 - Не отображать (с)НР

/* 1 - для графика СО2 часового, 2 - для графика СО2 суточного (с)НР
   4 - для графика влажности часовой, 8 - для графика влажности суточной (с)НР
   16 - для графика температуры часовой, 32 - для графика температуры суточной (с)НР
   64 - для прогноза дождя часового, 128 - для прогноза дождя суточного (с)НР
   256 - для графика давления часового, 512 - для графика давления суточного (с)НР
   1024 - для графика высоты часового, 2048 - для графика высоты суточного (с)НР
   для выборочных графиков значения нужно сложить (с)НР
   например: для изменения пределов у графиков суточной температуры и суточного СО2 складываем 2 + 32 и устанавливаем значение 34 (можно ставить сумму) (с)НР
*/

// пределы отображения для графиков
//temp /10 as converted to int
#define TEMP_MIN 150
#define TEMP_MAX 350

#define HUM_MIN 0
#define HUM_MAX 100
#define PRESS_MIN 720
#define PRESS_MAX 760
#define CO2_MIN 400
#define CO2_MAX 2000

// пины
#define BACKLIGHT A0
#define PIN_PIR   7  // ифракрасный датчик движния
#define LED_R 9
#define LED_G 6
#define LED_B 5

//Timers
timerMinim sensorsTimer(SENS_TIME);
timerMinim drawSensorsTimer(SENS_TIME);
timerMinim clockTimer(500);

// 4 минуты
timerMinim hourPlotTimer((long)4 * 60 * 1000);
timerMinim plotTimer((long)4 * 60 * 1000);
timerMinim dayPlotTimer((long)1.6 * 60 * 60 * 1000);    // 1.6 часа

timerMinim predictTimer((long)10 * 60 * 1000);
timerMinim checkVoltageTimer((long)5 * 60 * 1000);

int8_t hrs, mins, secs;
byte mode = 0;
/*
  0 часы и данные
  1 график углекислого за час
  2 график углекислого за сутки
  3 график влажности за час
  4 график влажности за сутки
  5 график температуры за час
  6 график температуры за сутки
  7 график дождя/давления за час
  8 график дождя/давления за сутки
*/

byte podMode = 1; // подрежим меню(с)НР
byte mode0scr = 0;
boolean scrOn = true;

// переменные для вывода
int dispTemp;
byte dispHum;
int dispPres;
int dispCO2 = 410;
int dispRain;


// массивы графиков
int tempHour[15], tempDay[15];
int humHour[15], humDay[15];
int pressHour[15], pressDay[15];
int co2Hour[15], co2Day[15];

unsigned long pressure_array[6];


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

/**
   Setup
*/
void setup() {
  pinMode(BACKLIGHT, OUTPUT); // Все каналы ШИМ устанавливаются как выходы
  fastWrite(BACKLIGHT, HIGH);
#if (DEBUG_ENABLE || CALIBRATE_VCC)
  Serial.begin(9600);
#endif

#ifdef CALIBRATE_VCC
  constantWizard();
#endif

#ifndef CALIBRATE_VCC
#ifdef VCC_CONST_ADR
  restoreConstant(VCC_CONST_ADR);
#endif

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  setLED(0);

  pinMode(PIN_PIR, INPUT);

  //analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);

  readParam();

  lcd.init();
  lcd.backlight();
  lcd.clear();

#if (DEBUG_ENABLE)// вывод на дисплей лог инициализации датчиков при запуске.
  boolean status = true;

  setLED(3);

#if (CO2_SENSOR == 1)
  lcd.setCursor(0, 0);
  lcd.print(F("MHZ-19... "));
  DEBUG(F("MHZ-19... "));
  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);
  mhz19.getStatus();    // первый запрос, в любом случае возвращает -1
  delay(500);
  if (mhz19.getStatus() == 0) {
    lcd.print(F("OK"));
    DEBUGLN(F("OK"));
  } else {
    lcd.print(F("ERROR"));
    DEBUGLN(F("ERROR"));
    status = false;
  }
#endif

  setLED(1);

  delay(500);

  setLED(0);
  lcd.setCursor(0, 3);
  if (status) {
    lcd.print(F("All good"));
    DEBUGLN(F("All good"));
  } else {
    lcd.print(F("Check wires!"));
    DEBUGLN(F("Check wires!"));
  }

#else

#if (CO2_SENSOR == 1)
  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);
#endif

  bme.begin();

#if (BH1750_SENSOR == 1)
  lightMeter.begin();
#endif

#endif

  if (RESET_CLOCK || rtc.lostPower()) {
    rtc.setTime(BUILD_SEC + 15, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR); //~15 sec to upload
  }

  lcd.clear();

  secs = rtc.getSeconds();
  mins = rtc.getMinutes();
  hrs = rtc.getHours();

  checkSensors();

  lcd.clear();
  readSensors();

  unsigned long pressure = bme.readPressure();

  for (byte i = 0; i < 6; i++) {          // счётчик от 0 до 5
    pressure_array[i] = pressure; // забить весь массив текущим давлением
  }

  // заполняем графики текущим значением
  for (byte i = 0; i < 15; i++) {   // счётчик от 0 до 14
    tempHour[i] = dispTemp;
    tempDay[i] = dispTemp;
    humHour[i] = dispHum;
    humDay[i] = dispHum;
#if (PRESSURE == 1)
    pressHour[i] = 0;
    pressDay[i] = 0;
#else
    pressHour[i] = dispPres;
    pressDay[i] = dispPres;
#endif
#if (CO2_SENSOR == 1)
    co2Hour[i] = dispCO2;
    co2Day[i] = dispCO2;
#endif
  }

  drawData();
  loadClock();

  drawSensors();
  drawClock(hrs, mins);
  if (powerStatus != 255) {
    voltageRunningAverage(getVoltage(BATTERY_PIN), true);
    dispPowerStatus(true);
  }

#if (RADIO == 1)
  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openReadingPipe(1, address[0]);     //хотим слушать трубу 0
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_HIGH); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.startListening();  //не слушаем радиоэфир, мы передатчик

#endif


  if (LCD_BRIGHT == 11) {                         // если установлен автоматический режим для экрана (с)НР
#if (BH1750_SENSOR == 1)
    Timer2.setFrequency(6000);     // Заводим прерывания таймера 2 на 6кгц
    Timer2.enableISR();             // Вкл. прерывания таймера
#endif
  }
#endif
}

/**
   Loop
*/
void loop() {

  //delay(1);
  //setConst(1109);//best const
  //Serial.println(getVoltage(A0));
  //delay(10);;
#ifndef CALIBRATE_VCC
  if (sensorsTimer.isReady()) {
    readSensors();    // читаем показания датчиков с периодом SENS_TIME
  }

  readRadio();
  if (clockTimer.isReady()) {
    //checkBrightness();  // яркость

    clockTick();          // два раза в секунду пересчитываем время и мигаем точками
  }
  plotSensorsTick();                                // тут внутри несколько таймеров для пересчёта графиков (за час, за день и прогноз)

  if (scrOn) {
    modesTick();                                      // тут ловим нажатия на кнопку и переключаем режимы

    if (mode == 0) {                                  // в режиме "главного экрана"
      if (drawSensorsTimer.isReady()) {
        drawSensors();  // обновляем показания датчиков на дисплее с периодом SENS_TIME
        drawInfoFromRadio();
      }
    } else {                                          // в любом из графиков
      if (plotTimer.isReady()) {
        redrawPlot();  // перерисовываем график
      }
    }
  }


  checkBrightness();
#endif
}
