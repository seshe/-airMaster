/**
   Read sensors
*/
void readSensors() {
  dispTemp = ((bme.readTemperature() - 1.2) + 0.05) * 10; //round and convert to int // -1.2 because to hot in the box
  dispHum = bme.readHumidity() + 0.5; //round
  if (transmit_data[2]) {
    dispPres = transmit_data[2];
  } else {
    dispPres = pressureToMmHg(bme.readPressure()) + 0.5; //round
  }
#if (CO2_SENSOR == 1)
  dispCO2 = mhz19.getPPM();
#else
  dispCO2 = 0;
#endif
}

/**
   Check sensors on setup
*/
void checkSensors() {
#ifndef DEBUG_ENABLE

#if (CO2_SENSOR == 1)
  while ((bme.readTemperature() + bme.readHumidity() + mhz19.getPPM()) < 1 || mhz19.getPPM() == 410) {
    lcd.clear();
    lcd.print(F("BME and CO2 sensors preparing"));
    myDelayMicroseconds(5000);
  }
#else
  while ((bme.readTemperature() + bme.readHumidity()) < 1) {
    lcd.clear();
    lcd.print(F("BME sensor preparing"));
    myDelayMicroseconds(5000);
  }
#endif

#endif
}

/**
   Delay method
*/
void myDelayMicroseconds(unsigned long us) {
  unsigned long tmr = micros();
  while (micros() - tmr < us);
}
