boolean dotFlag;

/**
   Draw clock
*/
void drawClock(byte hours, byte minutes) {
  if (!scrOn) {
    return;
  }
  if (mode0scr != 0) {                      // время (с)НР ----------------------------
    lcd.setCursor(15, 3);
    if (hours / 10 == 0) lcd.print(F(" "));
    lcd.print(hours);
    lcd.print(F(":"));
    if (minutes / 10 == 0) lcd.print(F("0"));
    lcd.print(minutes);
  } else {    // рисуем время крупными цифрами -------------------------------------------
    if (hours > 23 || minutes > 59) {
      return;
    }
    if (hours / 10 == 0) {
      drawDig(10, 0, 0);
    } else {
      drawDig(hours / 10, 0, 0);
    }
    drawDig(hours % 10, 4, 0);

    drawDig(minutes / 10, 8, 0);
    drawDig(minutes % 10, 12, 0);
  }
}

/*
   Draw dots
*/
void drawDots() {
  if (mode == 0 && scrOn) {                                     // Точки (с)НР ---------------------------------------------------
    byte code;
    if (dotFlag) {
      code = 165;
    } else {
      code = 32;
    }
    if (mode0scr == 0) {          // мигание большими точками только в нулевом режиме главного экрана (с)НР
      lcd.setCursor(7, 0);
      lcd.write(code);
      lcd.setCursor(7, 1);
      lcd.write(code);
    } else {
      if (code == 165) {
        code = 58;
      }
      lcd.setCursor(17, 3);
      lcd.write(code);
    }
  }
}

/**
   Print week name
*/
void lcdPrintWeekName( byte dayOfWeek ) {

  static const char *dayNames[]  = {
    "Mo",
    "Tu",
    "We",
    "Th",
    "Fr",
    "Sa",
    "Su",
  };

  lcd.print(dayNames[dayOfWeek - 1]);
}

/**
   Draw data
*/
void drawData() {                     // выводим дату -------------------------------------------------------------
  if (mode0scr == 0 && scrOn) {
    lcd.setCursor(18, 0);
    lcdPrintWeekName(rtc.getDay());
  }
}


/**
   Clock
*/
void clockTick() {
  dotFlag = !dotFlag;
  if (dotFlag) {            // каждую секунду пересчёт времени
    secs++;
    if (secs > 59) {        // каждую минуту
      secs = 0;
      mins++;
      if (mins <= 59 && mode == 0) {
        drawClock(hrs, mins);
      }
    }
    if (mins > 59) {        // каждый час
      secs = rtc.getSeconds();
      mins = rtc.getMinutes();
      hrs = rtc.getHours();

      if (mode == 0) {
        drawClock(hrs, mins);
      }

      if (hrs > 23) {
        hrs = 0;
      }
      if (mode == 0) {
        drawData();
      }
    }
    if (scrOn && mode == 0 && mode0scr == 0) {   // показывать секунды (с)НР
      lcd.setCursor(15, 0);
      if (secs < 10) {
        lcd.print(F(" "));
      }
      lcd.print(secs);

#if (BH1750_SENSOR == 1)
      drawLux();
#endif
    }

    checkMotion();

    //отладка (с)НР
    //DEBUG("Значение: " + String(analogRead(A0)));
    //DEBUG(" Напряжение0: " + String(analogRead(A0) * 5.2 / 1023.0));
    //DEBUG(" Напряжение1: " + String(analogRead(BATTERY_PIN) * 5.2 / 1023.0));
    //DEBUG(" Статус: " + String(powerStatus));
    //DEBUGLN(" Статус2: " + String((constrain((int)analogRead(A0) * 5.0 / 1023.0, 3.0, 4.2) - 3.0) / ((4.2 - 3.0) / 6.0) + 1));
  }
  drawDots();
  dispPowerStatus(false);

  if ((powerStatus == 0 || dispCO2 >= blinkLEDCO2 && LEDType == 0 || dispHum <= blinkLEDHum && LEDType == 1 || dispTemp >= blinkLEDTemp && LEDType == 2) && !dotFlag) {
    setLED(1);     // мигание индикатора в зависимости от значения и привязанного сенсора (с)НР
  } else {
    if ((dispCO2 >= maxCO2) && LEDType == 0 || (dispHum <= minHum) && LEDType == 1 || (dispTemp >= maxTemp) && LEDType == 2 || (dispRain <= minRain) && LEDType == 3 || (dispPres <= minPress) && LEDType == 4) {
      setLED(1);   // красный
    } else if ((dispCO2 >= normCO2) && LEDType == 0 || (dispHum <= normHum) && LEDType == 1 || (dispTemp >= normTemp) && LEDType == 2 || (dispRain <= normRain) && LEDType == 3 || (dispPres <= normPress) && LEDType == 4) {
      setLED(3);   // желтый
    } else if (LEDType == 0 || (dispHum <= maxHum) && LEDType == 1 || (dispTemp >= minTemp) && LEDType == 2 || (dispRain <= maxRain) && LEDType == 3 || LEDType == 4) {
      setLED(2);    // зеленый
    } else {
      setLED(4);   // синий (если влажность превышает заданный максимум, температура ниже минимума, вероятность осадков выше maxRain)
    }
  }

}
