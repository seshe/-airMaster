#include "PowerTool.h"
#include "Arduino.h"
#include "EEPROM.h"

// ***************************** VCC *****************************
int vcc_const = 1100;

int getVCC() {
  //reads internal 1V1 reference against VCC
  #if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44__)
    ADMUX = _BV(MUX5) | _BV(MUX0); // For ATtiny84
  #elif defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__)
    ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85/45
  #elif defined(__AVR_ATmega1284P__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega1284
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
  #endif
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  byte low  = ADCL; // must read ADCL first - it then locks ADCH
  byte high = ADCH; // unlocks both
  int result = (high << 8) | low;
  result = round((float)vcc_const * 1023 / result); // расчёт реального VCC
  return result; // возвращает VCC в милливольтах
}

int getVoltage(uint8_t pin) {
  long vcc_aver = 0;
  for (byte i = 0; i < 20; i++) {   
    vcc_aver += getVCC();
    delay(3);
  }
  vcc_aver /= 20;
  analogRead(pin);//somehow, the first analog read is incorrect in my case
  return round((float)vcc_aver * analogRead(pin) / 1024);
}

void constantWizard() {
	vcc_const = 1100;     // начальаня константа калибровки
	
	// ищем среднее за 100 измерений
  long vcc_aver = 0;
  for (byte i = 0; i < 100; i++) {    
    vcc_aver += getVCC();
    delay(5);
  }
  vcc_aver /= 100;
  
  Serial.print(F("Actual VCC: "));
  Serial.print(vcc_aver);
  Serial.println(F(" mV"));
  Serial.println(F("Send your VCC in millivolts"));
  
  while (!Serial.available());  // ждём ответа
  int Vcc = Serial.parseInt();  // напряжение от пользователя
  Serial.print(F("You insert: "));
  Serial.println(Vcc);
  delay(50);
  vcc_const = round((float)vcc_const * Vcc / vcc_aver);                // расчёт константы
  Serial.print(F("New voltage constant: "));
  Serial.println(vcc_const);
  Serial.println(F("Save in EEPROM? (Y / N)"));
  while(1) {
    while (!Serial.available());  // ждём ответа
    char answer = Serial.read();
    if (answer == 'N') {
      Serial.println(F("Bye"));
      return;
    }
    if (answer == 'Y') {
      break;
    }
  }
  Serial.println(F("Send address(for example 1000)"));  
  unsigned long now = millis ();
  while (millis () - now < 500) Serial.read ();  // read and discard any input
  while (!Serial.available());  // ждём ответа
  int adr = Serial.parseInt();  // адрес ячейки
  EEPROM.put(adr, vcc_const);
  Serial.print(F("Write in "));
  Serial.print(adr);
  Serial.println(F(" OK"));
}

void restoreConstant(int adr) {
	EEPROM.get(adr, vcc_const);
}

void setConst(int new_const) {
	vcc_const = new_const;
}

int getConst() {
	return vcc_const;
}

// 5 шагов по 20%
// Литий 1 шт 4200, 3950, 3850, 3750, 3700, 2800
// Алкалин 3шт 4650, 4050, 3870, 3690, 3570, 3300
// Никель 3шт 4200, 3810, 3750, 3690, 3600, 3000
// Никель 4шт 5600, 5080, 5000, 4920, 4800, 4000

byte lithiumPercent(int volts) {
	return mVtoPercent(volts, 4200, 3950, 3850, 3750, 3700, 2800);
}

byte alkaline3Percent(int volts) {
	return mVtoPercent(volts, 4650, 4050, 3870, 3690, 3570, 3300);
}

byte nickel3Percent(int volts) {
	return mVtoPercent(volts, 4200, 3810, 3750, 3690, 3600, 3000);
}

byte nickel4Percent(int volts) {
	return mVtoPercent(volts, 5600, 5080, 5000, 4920, 4800, 4000);
}

byte mVtoPercent(int volts, int volt100, int volt80, int volt60, int volt40, int volt20, int volt0) {
	int capacity;
	if (volts > volt80) capacity = map(volts, volt100, volt80, 100, 80);
	else if ((volts <= volt80) && (volts > volt60) ) capacity = map(volts, volt80, volt60, 80, 60);
	else if ((volts <= volt60) && (volts > volt40) ) capacity = map(volts, volt60, volt40, 60, 40);
	else if ((volts <= volt40) && (volts > volt20) ) capacity = map(volts, volt40, volt20, 40, 20);
	else if (volts <= volt20) capacity = map(volts, volt20, volt0, 20, 0);
	capacity = constrain(capacity, 0, 100);
	return capacity;
}

