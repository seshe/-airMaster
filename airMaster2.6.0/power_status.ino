// батарея
//byte DC[8] = {0b01110,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};    // уровень батареи - изменяется в коде скетча (с)НР
//byte AC[8] = {0b01010,  0b01010,  0b11111,  0b11111,  0b01110,  0b00100,  0b00100,  0b00011};
byte percent = 0;
/*
   Display power status
*/
void dispPowerStatus(boolean forceDisplay) {
  if (powerStatus != 255 && scrOn && mode == 0 && mode0scr == 0) {          // отображаем статус питания (с)НР
    if (luxFlag) {
      return;
    }
    //powerStatus = (constrain((int)expRunningAverage(getVoltage(BATTERY_PIN)), 3000, 4200) - 3000) / ((4200 - 3000) / 6000) + 1000;//TODO

    //powerStatus = (mVtoPercent(voltageRunningAverage(getVoltage(BATTERY_PIN), false), 4200, 3972, 3808, 3641, 3528, 3400) + 5) / 10;
    //powerStatus = (mVtoPercent(voltageRunningAverage(getVoltage(BATTERY_PIN), false), 4200, 3972, 3808, 3640, 3520, 3000) + 5) / 10;
    if (forceDisplay || checkVoltageTimer.isReady()) {
      if (analogRead(POWER_PIN) > 900 || analogRead(BATTERY_PIN) < 300 || (analogRead(POWER_PIN) < 300 && analogRead(BATTERY_PIN) < 300)) {
        powerStatus = 10;
      } else {
        //lcd.setCursor(5, 3);
        int vcc = voltageRunningAverage(getVoltage(BATTERY_PIN), false);
        //lcd.print(vcc);
        percent = mVtoPercent(vcc, 4140, 3972, 3808, 3640, 3520, 3000);
        powerStatus = (percent + 5) / 20;
        if (powerStatus < 1) {
          powerStatus == 1;
        }
        //pinMode(POWER_PIN, OUTPUT);
        //pinMode(BATTERY_PIN, OUTPUT);
      }
    }
    /*
        if (powerStatus) {
          for (byte i = 2; i <= 6; i++) {         // рисуем уровень заряда батареи (с)НР
            if ((7 - powerStatus) < i) {
              DC[i] = 0b11111;
            } else {
              DC[i] = 0b10001;
            }
          }

          lcd.createChar(7, DC);
        } else if (powerStatus == 10) {
          // сеть
          lcd.createChar(7, AC);
        }
    */
    /*
        if (mode0scr != 1) {
          lcd.setCursor(19, 2);
        } else {
          lcd.setCursor(19, 0);
        }
    */
    /*
      lcd.setCursor(19, 2);
      if (!dotFlag && powerStatus < 1) {
      lcd.write(32);
      } else {
      lcd.write(7);
      }
    */

    lcd.setCursor(19, 2);
    if (powerStatus == 10) {
      lcd.print(F("+"));
    } else {
      lcd.print(powerStatus);
    }
    /*
        if (percent < 10) {
          if (powerStatus != 10) {
            powerStatus = 0;
          }
          lcd.setCursor(18, 3);
        } else {
          if (powerStatus == 0) {
            powerStatus = 1;
          }
          lcd.setCursor(17, 3);
        }
        if (!dotFlag && percent && percent < 10) {
          lcd.print(F("   "));
        } else {
          if (percent < 100) {
            lcd.print(percent);
            lcd.print(F("%"));
          } else {
            lcd.print(F("   "));
          }
        }
    */
  }
}

// бегущее среднее
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
