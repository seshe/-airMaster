/**
 * Check brightness
 */
void checkBrightness() {
  if (LCD_BRIGHT == 11) {                         // если установлен автоматический режим для экрана (с)НР
    if (analogRead(PHOTO) < BRIGHT_THRESHOLD) {   // если темно
      analogWrite(BACKLIGHT, LCD_BRIGHT_MIN);
    } else {                                      // если светло
      analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);
    }
  } else {
    analogWrite(BACKLIGHT, LCD_BRIGHT * LCD_BRIGHT * 2.5);
  }

  if (LED_BRIGHT == 11) {                         // если установлен автоматический режим для индикатора (с)НР
    if (analogRead(PHOTO) < BRIGHT_THRESHOLD) {   // если темно
#if (LED_MODE == 0)
      LED_ON = (LED_BRIGHT_MIN);
#else
      LED_ON = (255 - LED_BRIGHT_MIN);
#endif
    } else {                                      // если светло
#if (LED_MODE == 0)
      LED_ON = (LED_BRIGHT_MAX);
#else
      LED_ON = (255 - LED_BRIGHT_MAX);
#endif
    }
  }
}

/*
  mode:
  0 - Главный экран
  1-8 - Графики: СО2 (час, день), Влажность (час, день), Температура (час, день), Осадки (час, день)
  252 - выбор режима индикатора (podMode от 0 до 3: индикация СО2, влажности, температуры, осадков)
  253 - настройка яркости экрана (podMode от 0 до 11: выбор от 0% до 100% или автоматическое регулирование)
  254 - настройка яркости индикатора (podMode от 0 до 11: выбор от 0% до 100% или автоматическое регулирование)
  255 - главное меню (podMode от 0 до 13: 1 - Сохранить, 2 - Выход, 3 - Ярк.индикатора, 4 - Ярк.экрана, 5 - Режим индикатора,
                                        6-13 вкл/выкл графики: СО2 (час, день), Влажность (час, день), Температура (час, день), Осадки (час, день))
*/
/**
 * Modes tick
 */
void modesTick() {
  button.tick();
  boolean changeFlag = false;
  if (button.isSingle()) {    // одинарное нажатие на кнопку

    if (mode >= 240) {
      podMode++;
      switch (mode) {
        case 252:             // Перебираем все варианты режимов LED индикатора (с)НР
          //         podMode++;
          if (podMode > 4) {
            podMode = 0;
          }
          LEDType = podMode;
          changeFlag = true;
          break;

        case 253:             // Перебираем все варианты яркости LCD экрана (с)НР
          //         podMode++;
          if (podMode > 11) {
            podMode = 0;
          }
          LCD_BRIGHT = podMode;
          checkBrightness();
          changeFlag = true;
          break;

        case 254:             // Перебираем все варианты яркости LED индикатора (с)НР
          //         podMode++;
          if (podMode > 11) {
            podMode = 0;
          }
          LED_BRIGHT = podMode;
          changeFlag = true;
          break;

        case 255:             // Перебираем все варианты основных настроек (с)НР
          //         podMode++;
          if (podMode > 15) {
            podMode = 1;
          }
          changeFlag = true;
          break;
      }
    } else {
      do {
        mode++;
        if (mode > 10) {
          mode = 0;
        }
#if (CO2_SENSOR == 0 && mode == 1)
        mode = 3;
#endif
      } while (((VIS_ONDATA & (1 << (mode - 1))) == 0) && (mode > 0));   // проверка на отображение графиков (с)НР
      changeFlag = true;
    }
  }
  if (button.isDouble()) {                    // двойное нажатие (с)НР ----------------------------
    if (mode > 0 && mode < 11) {              // Меняет пределы графика на установленные/фактические максимумы (с)НР
      MAX_ONDATA = (int)MAX_ONDATA ^ (1 << (mode - 1));
    } else if (mode == 0)  {
      mode0scr++;
      if (CO2_SENSOR == 0 && mode0scr == 1) {
        mode0scr++;
      }
      if (mode0scr > 4) {
        mode0scr = 0;         // Переключение рехима работы главного экрана (с)НР
      }
    } else if (mode > 240) {
      podMode = 1;       // Переключение на меню сохранения (с)НР
    }
    changeFlag = true;
  }

  if ((button.isTriple()) && (mode == 0)) {  // тройное нажатие в режиме главного экрана - переход в меню (с)НР
    mode = 255;
    podMode = 3;
    changeFlag = true;
  }

  if (button.isHolded()) {    // удержание кнопки (с)НР
    //    if ((mode >=252) && (mode <= 254)) {
    //      mode = 255;
    //      podMode = 1;
    //    }
    switch (mode) {
      case 0:
        bigDig = !bigDig;
        break;
      case 252:       // реж. индикатора
        mode = 255;
        podMode = 1;
        break;
      case 253:       // ярк. экрана
        mode = 255;
        podMode = 1;
        break;
      case 254:       // ярк. индикатора
        mode = 255;
        podMode = 1;
        break;
      case 255:       // главное меню
        if (podMode == 2 || podMode == 1) {
          mode = 0;                   // если Выход или Сохранить
        }
        if (podMode >= 3 && podMode <= 5) {
          mode = 255 - podMode + 2;   // если настройки яркостей, то переключаемся в настройки пункта меню
        }
        if (podMode >= 6 && podMode <= 17) {
          VIS_ONDATA = VIS_ONDATA ^ (1 << (podMode - 6));  // вкл/выкл отображения графиков
        }
        if (podMode == 1) {                                           // если Сохранить
          if (EEPROM.read(2) != (MAX_ONDATA & 255)) {
            EEPROM.write(2, (MAX_ONDATA & 255));
          }
          if (EEPROM.read(3) != (MAX_ONDATA >> 8)) {
            EEPROM.write(3, (MAX_ONDATA >> 8));
          }
          if (EEPROM.read(4) != (VIS_ONDATA & 255)) {
            EEPROM.write(4, (VIS_ONDATA & 255));
          }
          if (EEPROM.read(5) != (VIS_ONDATA >> 8)) {
            EEPROM.write(5, (VIS_ONDATA >> 8));
          }
          if (EEPROM.read(6) != mode0scr) {
            EEPROM.write(6, mode0scr);
          }
          if (EEPROM.read(7) != bigDig) {
            EEPROM.write(7, bigDig);
          }
          if (EEPROM.read(8) != LED_BRIGHT) {
            EEPROM.write(8, LED_BRIGHT);
          }
          if (EEPROM.read(9) != LCD_BRIGHT) {
            EEPROM.write(9, LCD_BRIGHT);
          }
          if (EEPROM.read(10) != LEDType) {
            EEPROM.write(10, LEDType);
          }
          if (EEPROM.read(0) != 122) {
            EEPROM.write(0, 122);
          }
        }
        if (podMode < 6) {
          podMode = 1;
        }
        if (mode == 252) {      // если выбран режим LED - устанавливаем текущее значение (с)НР
          podMode = LEDType;
        }
        if (mode == 254) {      // если выбрана яркость LED - устанавливаем текущее показание (с)НР
          podMode = LED_BRIGHT;
        }
        if (mode == 253) {      // если выбрана яркость LCD - устанавливаем текущее показание (с)НР
          podMode = LCD_BRIGHT;
        }
        break;
      default:
        mode = 0;
    }
    changeFlag = true;
  }

  if (changeFlag) {
    if (mode >= 240) {
      lcd.clear();
#if (LANG == 1)
      lcd.createChar(1, BM);  //Ь
      lcd.createChar(2, IY);  //Й
      lcd.createChar(3, DD);  //Д
      lcd.createChar(4, II);  //И
      lcd.createChar(5, IA);  //Я
      lcd.createChar(6, YY);  //Ы
      lcd.createChar(7, AA);  //Э
      lcd.createChar(0, ZZ);  //Ж
#endif
      lcd.setCursor(0, 0);
    }
    if (mode == 255) {          // Перебираем варианты в главном меню (с)НР
#if (LANG == 1)
      lcd.print("HACTPO\2K\4:");              // ---Настройки
#else
      lcd.print("Setup:");
#endif
      lcd.setCursor(0, 1);
      switch (podMode) {
        case 1:
#if (LANG == 1)
          lcd.print("COXPAH\4T\1");     // ---Сохранить
#else
          lcd.print("Save");
#endif
          break;
        case 2:
#if (LANG == 1)
          lcd.print("B\6XO\3");         // --- Выход
#else
          lcd.print("Exit");
#endif
          break;
        case 5:
#if (LANG == 1)
          lcd.print("PE\10.\4H\3\4KATOPA");  // ---Реж.индик.
#else
          lcd.print("indicator mode");
#endif
          break;
        case 3:
#if (LANG == 1)
          lcd.print("\5PK.\4H\3\4KATOPA");  // ---Ярк.индик.
#else
          lcd.print("indicator brt.");
#endif
          break;
        case 4:
#if (LANG == 1)
          lcd.print("\5PK.\7KPAHA");    // ---Ярк.экрана
#else
          lcd.print("Bright LCD");
#endif
          break;
      }
      if (podMode >= 6 && podMode <= 17) {
        lcd.setCursor(10, 0);
#if (LANG == 1)
        lcd.createChar(8, FF);  //ф
        lcd.createChar(7, GG);  //Г
        lcd.createChar(5, LL);  //Л
        lcd.print("\7PA\10\4KOB");            // ---графиков
#else
        lcd.print("Charts  ");
#endif
        lcd.setCursor(0, 1);
        if ((3 & (1 << (podMode - 6))) != 0) {
           lcd.print("CO2 ");
        }
        if ((12 & (1 << (podMode - 6))) != 0) {
#if (LANG == 1)
          lcd.print("B\5,% ");
#else
          lcd.print("Hum,%");
#endif
        }
        if ((48 & (1 << (podMode - 6))) != 0) {
          lcd.print("t\337 ");
        }
        if ((192 & (1 << (podMode - 6))) != 0) {
#if (PRESSURE == 1)
          lcd.print("p,rain ");
#else
          lcd.print("p,mmPT ");
#endif
        }
        if ((768 & (1 << (podMode - 6))) != 0) {
#if (LANG == 1)
          lcd.print("B\6C,m  ");
#else
          lcd.print("hgt,m  ");
#endif
        }

        if ((1365 & (1 << (podMode - 6))) != 0) {
          lcd.setCursor(8, 1);
#if (LANG == 1)
          lcd.createChar(3, CH);  //Ч
          lcd.print("\3AC:");
#else
          lcd.print("Hour:");
#endif
        } else {
          lcd.setCursor(7, 1);
#if (LANG == 1)
          lcd.print("\3EH\1:");
#else
          lcd.print("Day: ");
#endif
        }
        if ((VIS_ONDATA & (1 << (podMode - 6))) != 0) {
#if (LANG == 1)
          lcd.print("BK\5 ");
#else
          lcd.print("On  ");
#endif
        } else {
#if (LANG == 1)
          lcd.print("B\6K\5");
#else
          lcd.print("Off ");
#endif
        }
      }
    }
    if (mode == 252) {                        // --------------------- показать  "Реж.индикатора"
      LEDType = podMode;
      lcd.setCursor(0, 0);
#if (LANG == 1)
      lcd.createChar(6, LL);  //Л
      lcd.createChar(3, DD);  //Д
      lcd.createChar(5, II);  //И
      lcd.createChar(8, ZZ);  //Ж
      lcd.print("PE\10.\4H\3\4KATOPA:");
#else
      lcd.print("indicator mode:");
#endif
      lcd.setCursor(0, 1);
      switch (podMode) {
        case 0:
          lcd.print("CO2   ");
          break;
        case 1:
#if (LANG == 1)
          lcd.print("B\6A\10H.");          // влажн.
#else
          lcd.print("Humid.");
#endif
          break;
        case 2:
          lcd.print("t\337     ");
          break;
        case 3:
#if (LANG == 1)
          lcd.print("OCA\3K\5");          // осадки
#else
          lcd.print("rain  ");
#endif
          break;
        case 4:
#if (LANG == 1)
          lcd.print("\3AB\6EH\5E");       // давление
#else
          lcd.print("pressure");
#endif
          break;
      }

    }
    if (mode == 253) {                        // --------------------- показать  "Ярк.экрана"
#if (LANG == 1)
      lcd.print("\5PK.\7KPAHA:");// + String(LCD_BRIGHT * 10) + "%  ");
#else
      lcd.print("Bright LCD:");
#endif
      //lcd.setCursor(11, 0);
      if (LCD_BRIGHT == 11) {
#if (LANG == 1)
        lcd.print("ABTO ");
#else
        lcd.print("Auto ");
#endif
      } else lcd.print(String(LCD_BRIGHT * 10) + "%");
    }
    if (mode == 254) {                        // --------------------- показать  "Ярк.индикатора"
#if (LANG == 1)
      lcd.print("\5PK.\4H\3\4K.:");// + String(LED_BRIGHT * 10) + "%  ");
#else
      lcd.print("indic.brt.:");
#endif
      //lcd.setCursor(15, 0);
      if (LED_BRIGHT == 11) {
#if (LANG == 1)
        lcd.print("ABTO ");
#else
        lcd.print("Auto ");
#endif
      } else lcd.print(String(LED_BRIGHT * 10) + "%");
    }

    if (mode == 0) {
      lcd.clear();
      loadClock();
      drawSensors();
      if (DISPLAY_TYPE == 1) drawData();
    } else if (mode <= 10) {
      //lcd.clear();
      loadPlot();
      redrawPlot();
    }
  }
}

/**
 * Redraw plot
 */
void redrawPlot() {
  lcd.clear();
#if (DISPLAY_TYPE == 1)       // для дисплея 2004
  switch (mode) {             // добавлена переменная для "растягивания" графика до фактических максимальных и(или) минимальных значений(с)НР
    case 1: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Hour, "c ", "hr", mode);
      break;
    case 2: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Day, "c ", "da", mode);
      break;
    case 3: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humHour, "h%", "hr", mode);
      break;
    case 4: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humDay, "h%", "da", mode);
      break;
    case 5: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempHour, "t\337", "hr", mode);
      break;
    case 6: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempDay, "t\337", "da", mode);
      break;
    //    case 7: drawPlot(0, 3, 15, 4, RAIN_MIN, RAIN_MAX, (int*)rainHour, "r ", "hr", mode);
    //      break;
    //    case 8: drawPlot(0, 3, 15, 4, RAIN_MIN, RAIN_MAX, (int*)rainDay, "r ", "da", mode);
    //      break;
    case 7: drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressHour, "p ", "hr", mode);
      break;
    case 8: drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressDay, "p ", "da", mode);
      break;
  }
#else                         // для дисплея 1602
  switch (mode) {
    case 1: drawPlot(0, 1, 12, 2, CO2_MIN, CO2_MAX, (int*)co2Hour, "c", "h", mode);
      break;
    case 2: drawPlot(0, 1, 12, 2, CO2_MIN, CO2_MAX, (int*)co2Day, "c", "d", mode);
      break;
    case 3: drawPlot(0, 1, 12, 2, HUM_MIN, HUM_MAX, (int*)humHour, "h", "h", mode);
      break;
    case 4: drawPlot(0, 1, 12, 2, HUM_MIN, HUM_MAX, (int*)humDay, "h", "d", mode);
      break;
    case 5: drawPlot(0, 1, 12, 2, TEMP_MIN, TEMP_MAX, (int*)tempHour, "t", "h", mode);
      break;
    case 6: drawPlot(0, 1, 12, 2, TEMP_MIN, TEMP_MAX, (int*)tempDay, "t", "d", mode);
      break;
    //    case 7: drawPlot(0, 1, 12, 2, RAIN_MIN, RAIN_MAX, (int*)rainHour, "r", "h", mode);
    //      break;
    //    case 8: drawPlot(0, 1, 12, 2, RAIN_MIN, RAIN_MAX, (int*)rainDay, "r", "d", mode);
    //      break;
    case 7: drawPlot(0, 1, 12, 2, PRESS_MIN, PRESS_MAX, (int*)pressHour, "p", "h", mode);
      break;
    case 8: drawPlot(0, 1, 12, 2, PRESS_MIN, PRESS_MAX, (int*)pressDay, "p", "d", mode);
      break;
  }
#endif
}

/**
 * Read sensors
 */
void readSensors() {
  bme.takeForcedMeasurement();
  dispTemp = bme.readTemperature();
  dispHum = bme.readHumidity();
  dispPres = (float)bme.readPressure() * 0.00750062;
#if (CO2_SENSOR == 1)
  dispCO2 = mhz19.getPPM();
#else
  dispCO2 = 0;
#endif
}

/**
 * Drow sensors
 */
void drawSensors() {
#if (DISPLAY_TYPE == 1)
  // дисплей 2004 ----------------------------------
  if (mode0scr != 2) {                        // Температура (с)НР ----------------------------
    lcd.setCursor(0, 2);
    if (bigDig) {
      if (mode0scr == 1) lcd.setCursor(15, 2);
      if (mode0scr != 1) lcd.setCursor(15, 0);
    }
    lcd.print(String(dispTemp, 1));
    lcd.write(223);
  } else {
    drawTemp(dispTemp, 0, 0);
  }

  if (mode0scr != 4) {                        // Влажность (с)НР ----------------------------
    lcd.setCursor(5, 2);
    if (bigDig) lcd.setCursor(15, 1);
    lcd.print(" " + String(dispHum) + "% ");
  } else {
    drawHum(dispHum, 0, 0);
  }

#if (CO2_SENSOR == 1)
  if (mode0scr != 1) {                       // СО2 (с)НР ----------------------------

    if (bigDig) {
      lcd.setCursor(15, 2);
      lcd.print(String(dispCO2) + "p");
    } else {
      lcd.setCursor(11, 2);
      lcd.print(String(dispCO2) + "ppm ");
    }
  } else {
    drawPPM(dispCO2, 0, 0);
  }
#endif

  if (mode0scr != 3) {                      // Давление (с)НР ---------------------------
    lcd.setCursor(0, 3);
    if (bigDig && mode0scr == 0) lcd.setCursor(15, 3);
    if (bigDig && (mode0scr == 1 || mode0scr == 2)) lcd.setCursor(15, 0);
    if (bigDig && mode0scr == 4) lcd.setCursor(15, 1);
    if (!(bigDig && mode0scr == 1)) lcd.print(String(dispPres) + "mm");
  } else {
    drawPres(dispPres, 0, 0);
  }

  if (!bigDig) {                            // дождь (с)НР -----------------------------
    lcd.setCursor(5, 3);
    lcd.print(" rain     ");
    lcd.setCursor(11, 3);
    if (dispRain < 0) lcd.setCursor(10, 3);
    lcd.print(String(dispRain) + "%");
    //  lcd.setCursor(14, 3);
  }

  if (mode0scr != 0) {                      // время (с)НР ----------------------------
    lcd.setCursor(15, 3);
    if (hrs / 10 == 0) lcd.print(" ");
    lcd.print(hrs);
    lcd.print(":");
    if (mins / 10 == 0) lcd.print("0");
    lcd.print(mins);
  } else {
    drawClock(hrs, mins, 0, 0); //, 1);
  }
#else
  // дисплей 1602 ----------------------------------
  if (!bigDig) {              // если только мелкими цифрами (с)НР
    lcd.setCursor(0, 0);
    lcd.print(String(dispTemp, 1));
    lcd.write(223);
    lcd.setCursor(6, 0);
    lcd.print(String(dispHum) + "% ");

#if (CO2_SENSOR == 1)
    lcd.print(String(dispCO2) + "ppm");
    if (dispCO2 < 1000) lcd.print(" ");
#endif

    lcd.setCursor(0, 1);
    lcd.print(String(dispPres) + " mm  rain ");
    lcd.print(String(dispRain) + "% ");
  } else {                    // для крупных цифр (с)НР
    switch (mode0scr) {
      case 0:
        drawClock(hrs, mins, 0, 0);
        break;
      case 1:
#if (CO2_SENSOR == 1)
        drawPPM(dispCO2, 0, 0);
#endif
        break;
      case 2:
        drawTemp(dispTemp, 2, 0);
        break;
      case 3:
        drawPres(dispPres, 2, 0);
        break;
      case 4:
        drawHum(dispHum, 0, 0);
        break;
    }
  }
#endif
}

/**
 * Plot recalculation timers 
 */
void plotSensorsTick() {
  // 4 или 5 минутный таймер
  if (testTimer(hourPlotTimerD, hourPlotTimer)) {
    for (byte i = 0; i < 14; i++) {
      tempHour[i] = tempHour[i + 1];
      humHour[i] = humHour[i + 1];
      pressHour[i] = pressHour[i + 1];
      //      rainHour[i] = rainHour[i + 1];
      co2Hour[i] = co2Hour[i + 1];
    }
    tempHour[14] = dispTemp;
    humHour[14] = dispHum;
    co2Hour[14] = dispCO2;

#if (PRESSURE == 1)
    pressHour[14] = dispRain;
#else
    pressHour[14] = dispPres;
#endif
  }

  // 1.5 или 2 часовой таймер
  if (testTimer(dayPlotTimerD, dayPlotTimer)) {
    long averTemp = 0, averHum = 0, averPress = 0, averCO2 = 0; //, averRain = 0

    for (byte i = 0; i < 15; i++) {
      averTemp += tempHour[i];
      averHum += humHour[i];
      averPress += pressHour[i];
      //      averRain += rainHour[i];
      averCO2 += co2Hour[i];
    }
    averTemp /= 15;
    averHum /= 15;
    averPress /= 15;
    //    averRain /= 15;
    averCO2 /= 15;

    for (byte i = 0; i < 14; i++) {
      tempDay[i] = tempDay[i + 1];
      humDay[i] = humDay[i + 1];
      pressDay[i] = pressDay[i + 1];
      //      rainDay[i] = rainDay[i + 1];
      co2Day[i] = co2Day[i + 1];
    }
    tempDay[14] = averTemp;
    humDay[14] = averHum;
    pressDay[14] = averPress;
    //    rainDay[14] = averRain;
    co2Day[14] = averCO2;
  }

  // 10 минутный таймер
  if (testTimer(predictTimerD, predictTimer)) {
    // тут делаем линейную аппроксимацию для предсказания погоды
    long averPress = 0;
    for (byte i = 0; i < 10; i++) {
      bme.takeForcedMeasurement();
      averPress += bme.readPressure();
      delay(1);
    }
    averPress /= 10;

    for (byte i = 0; i < 5; i++) {                   // счётчик от 0 до 5 (да, до 5. Так как 4 меньше 5)
      pressure_array[i] = pressure_array[i + 1];     // сдвинуть массив давлений КРОМЕ ПОСЛЕДНЕЙ ЯЧЕЙКИ на шаг назад
    }
    pressure_array[5] = averPress;                   // последний элемент массива теперь - новое давление
    sumX = 0;
    sumY = 0;
    sumX2 = 0;
    sumXY = 0;
    for (int i = 0; i < 6; i++) {                    // для всех элементов массива
      //sumX += time_array[i];
      sumX += i;
      sumY += (long)pressure_array[i];
      //sumX2 += time_array[i] * time_array[i];
      sumX2 += i * i;
      //sumXY += (long)time_array[i] * pressure_array[i];
      sumXY += (long)i * pressure_array[i];
    }
    a = 0;
    a = (long)6 * sumXY;                            // расчёт коэффициента наклона приямой
    a = a - (long)sumX * sumY;
    a = (float)a / (6 * sumX2 - sumX * sumX);
    delta = a * 6;      // расчёт изменения давления
    dispRain = map(delta, -250, 250, 100, -100);    // пересчитать в проценты
    DEBUGLN(String(pressure_array[5]) + " " + String(delta) + " " + String(dispRain));   // дебаг
  }
}

/**
 * Clock 
 */
void clockTick() {
  dotFlag = !dotFlag;
  if (dotFlag) {            // каждую секунду пересчёт времени
    secs++;
    if (secs > 59) {        // каждую минуту
      secs = 0;
      mins++;
      if (mins <= 59 && mode == 0) {
        drawSensors();      // (с)НР
      }
    }
    if (mins > 59) {        // каждый час
      // loadClock();        // Обновляем знаки, чтобы русские буквы в днях недели тоже обновились. (с)НР
      now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      if (mode == 0) {
        drawSensors();
      }
      if (hrs > 23) {
        hrs = 0;
      }
      if (mode == 0 && DISPLAY_TYPE) {
        drawData();
      }
    }
    if ((DISP_MODE != 0 && mode == 0) && DISPLAY_TYPE == 1 && !bigDig) {   // Если режим секунд или дни недели по-русски, и 2-х строчные цифры то показывать секунды (с)НР
      lcd.setCursor(15, 1);
      if (secs < 10) {
         lcd.print(" ");
      }
      lcd.print(secs);
    }
  }

  if (mode == 0) {                                     // Точки и статус питания (с)НР ---------------------------------------------------

    if (!bigDig && powerStatus != 255 && DISPLAY_TYPE == 1) {          // отображаем статус питания (с)НР

      if (analogRead(A1) > 900 || analogRead(A0) < 300 || (analogRead(A1) < 300 && analogRead(A0) < 300)) {
         powerStatus = 0;
      } else {
        powerStatus = (constrain((int)analogRead(A0) * 5.2 / 1023.0, 3.0, 4.2) - 3.0) / ((4.2 - 3.0) / 6.0) + 1;
      }

      if (powerStatus) {
        for (byte i = 2; i <= 6; i++) {         // рисуем уровень заряда батареи (с)НР
          if ((7 - powerStatus) < i) {
            DC[i] = 0b11111;
          } else {
            DC[i] = 0b10001;
          }
        }
        lcd.createChar(6, DC);
      } else {
        lcd.createChar(6, AC);
      }

      if (mode0scr != 1) {
        lcd.setCursor(19, 2);
      } else {
        lcd.setCursor(19, 0);
      }
      if (!dotFlag && powerStatus == 1) {
        lcd.write(32);
      } else {
        lcd.write(6);
      }
    }
    
    //отладка (с)НР
    //Serial.print("Значение: " + String(analogRead(A0))); Serial.print(" Напряжение0: " + String(analogRead(A0) * 5.2 / 1023.0)); Serial.print(" Напряжение1: " + String(analogRead(A1) * 5.2 / 1023.0)); Serial.print(" Статус: " + String(powerStatus));  Serial.println(" Статус2: " + String((constrain((int)analogRead(A0) * 5.0 / 1023.0, 3.0, 4.2) - 3.0) / ((4.2 - 3.0) / 6.0) + 1)); 
    byte code;
    if (dotFlag) {
      code = 165;
    } else {
      code = 32;
    }
    if (mode0scr == 0 && (bigDig && DISPLAY_TYPE == 0 || DISPLAY_TYPE == 1)) {          // мигание большими точками только в нулевом режиме главного экрана (с)НР
      if (bigDig && DISPLAY_TYPE == 1) {
        lcd.setCursor(7, 2);
      } else {
        lcd.setCursor(7, 0);
      }
      lcd.write(code);
      lcd.setCursor(7, 1);
      lcd.write(code);
    } else {
#if (DISPLAY_TYPE == 1)
      if (code == 165) {
        code = 58;
      }
      lcd.setCursor(17, 3);
      lcd.write(code);
#endif
    }
  }

  if ((dispCO2 >= blinkLEDCO2 && LEDType == 0 || dispHum <= blinkLEDHum && LEDType == 1 || dispTemp >= blinkLEDTemp && LEDType == 2) && !dotFlag) {
     setLEDcolor(0);     // мигание индикатора в зависимости от значения и привязанного сенсора (с)НР
  } else {
    setLED();
  }
}

/**
 * Test timer
 */
boolean testTimer(unsigned long & dataTimer, unsigned long setTimer) {   // Проверка таймеров (с)НР
  if (millis() - dataTimer >= setTimer) {
    dataTimer = millis();
    return true;
  } else {
    return false;
  }
}

/**
 * Set LED color
 */
void setLEDcolor(byte color) {                    // цвет индикатора задается двумя битами на каждый цвет (с)НР
  analogWrite(LED_R, LED_ON + LED_ON * ((LED_MODE << 1) - 1) * (3 - (color & 3)) / 3);
  analogWrite(LED_G, LED_ON + LED_ON * ((LED_MODE << 1) - 1) * (3 - ((color & 12) >> 2)) / 3);
  analogWrite(LED_B, LED_ON + LED_ON * ((LED_MODE << 1) - 1) * (3 - ((color & 48) >> 4)) / 3);
}

/**
 * Set LED
 */
void setLED() {
  if (LED_BRIGHT < 11) {                                     // если ручные установки яркости
    LED_ON = 255 / 100 * LED_BRIGHT * LED_BRIGHT;
  } else {
    checkBrightness();
  }
  if (LED_MODE != 0) {
    LED_ON = 255 - LED_ON;
  }

  // ниже задается цвет индикатора в зависимости от назначенного сенсора: красный, желтый, зеленый, синий (с)НР

  if ((dispCO2 >= maxCO2) && LEDType == 0 || (dispHum <= minHum) && LEDType == 1 || (dispTemp >= maxTemp) && LEDType == 2 || (dispRain <= minRain) && LEDType == 3 || (dispPres <= minPress) && LEDType == 4) {
    setLEDcolor(3);   // красный
  } else if ((dispCO2 >= normCO2) && LEDType == 0 || (dispHum <= normHum) && LEDType == 1 || (dispTemp >= normTemp) && LEDType == 2 || (dispRain <= normRain) && LEDType == 3 || (dispPres <= normPress) && LEDType == 4) {
    setLEDcolor(3 + 8);   // желтый
  } else if (LEDType == 0 || (dispHum <= maxHum) && LEDType == 1 || (dispTemp >= minTemp) && LEDType == 2 || (dispRain <= maxRain) && LEDType == 3 || LEDType == 4) {
    setLEDcolor(12);    // зеленый
  } else {
    setLEDcolor(48);   // синий (если влажность превышает заданный максимум, температура ниже минимума, вероятность осадков выше maxRain)
  }
}

/**
 * Load plot
 */
void loadPlot() {
  lcd.createChar(0, rowS);      // Стрелка вниз для индикатора пределов (с)НР
  lcd.createChar(1, row1);
  lcd.createChar(2, row2);
  lcd.createChar(3, row3);
  lcd.createChar(4, row4);
  lcd.createChar(5, row5);
  lcd.createChar(6, row6);
  lcd.createChar(7, row7);
}

/**
 * Display digit
 */
void digSeg(byte x, byte y, byte z1, byte z2, byte z3, byte z4, byte z5, byte z6) {   // отображение двух строк по три символа с указанием кодов символов (с)НР
  lcd.setCursor(x, y);
  lcd.write(z1); lcd.write(z2); lcd.write(z3);
  if (x <= 11) {
    lcd.print(" ");
  }
  lcd.setCursor(x, y + 1);
  lcd.write(z4); lcd.write(z5); lcd.write(z6);
  if (x <= 11) {
    lcd.print(" ");
  }
}

/**
 * Draw digit
 */
void drawDig(byte dig, byte x, byte y) {        // рисуем цифры (с)НР ---------------------------------------
  if (bigDig && DISPLAY_TYPE == 1) {
    switch (dig) {            // четырехстрочные цифры (с)НР
      case 0:
        digSeg(x, y, 255, 0, 255, 255, 32, 255);
        digSeg(x, y + 2, 255, 32, 255, 255, 3, 255);
        break;
      case 1:
        digSeg(x, y, 32, 255, 32, 32, 255, 32);
        digSeg(x, y + 2, 32, 255, 32, 32, 255, 32);
        break;
      case 2:
        digSeg(x, y, 0, 0, 255, 1, 1, 255);
        digSeg(x, y + 2, 255, 2, 2, 255, 3, 3);
        break;
      case 3:
        digSeg(x, y, 0, 0, 255, 1, 1, 255);
        digSeg(x, y + 2, 2, 2, 255, 3, 3, 255);
        break;
      case 4:
        digSeg(x, y, 255, 32, 255, 255, 1, 255);
        digSeg(x, y + 2, 2, 2, 255, 32, 32, 255);
        break;
      case 5:
        digSeg(x, y, 255, 0, 0, 255, 1, 1);
        digSeg(x, y + 2, 2, 2, 255, 3, 3, 255);
        break;
      case 6:
        digSeg(x, y, 255, 0, 0, 255, 1, 1);
        digSeg(x, y + 2, 255, 2, 255, 255, 3, 255);
        break;
      case 7:
        digSeg(x, y, 0, 0, 255, 32, 32, 255);
        digSeg(x, y + 2, 32, 255, 32, 32, 255, 32);
        break;
      case 8:
        digSeg(x, y, 255, 0, 255, 255, 1, 255);
        digSeg(x, y + 2, 255, 2, 255, 255, 3, 255);
        break;
      case 9:
        digSeg(x, y, 255, 0, 255, 255, 1, 255);
        digSeg(x, y + 2, 2, 2, 255, 3, 3, 255);
        break;
      case 10:
        digSeg(x, y, 32, 32, 32, 32, 32, 32);
        digSeg(x, y + 2, 32, 32, 32, 32, 32, 32);
        break;
    }
  } else {
    switch (dig) {            // двухстрочные цифры
      case 0:
        digSeg(x, y, 255, 1, 255, 255, 2, 255);
        break;
      case 1:
        digSeg(x, y, 32, 255, 32, 32, 255, 32);
        break;
      case 2:
        digSeg(x, y, 3, 3, 255, 255, 4, 4);
        break;
      case 3:
        digSeg(x, y, 3, 3, 255, 4, 4, 255);
        break;
      case 4:
        digSeg(x, y, 255, 0, 255, 5, 5, 255);
        break;
      case 5:
        digSeg(x, y, 255, 3, 3, 4, 4, 255);
        break;
      case 6:
        digSeg(x, y, 255, 3, 3, 255, 4, 255);
        break;
      case 7:
        digSeg(x, y, 1, 1, 255, 32, 255, 32);
        break;
      case 8:
        digSeg(x, y, 255, 3, 255, 255, 4, 255);
        break;
      case 9:
        digSeg(x, y, 255, 3, 255, 4, 4, 255);
        break;
      case 10:
        digSeg(x, y, 32, 32, 32, 32, 32, 32);
        break;
    }
  }
}

/**
 * Draw CO2
 */
void drawPPM(int dispCO2, byte x, byte y) {     // Уровень СО2 крупно на главном экране (с)НР ----------------------------
  if (dispCO2 / 1000 == 0) {
    drawDig(10, x, y);
  } else {
    drawDig(dispCO2 / 1000, x, y);
  }
  drawDig((dispCO2 % 1000) / 100, x + 4, y);
  drawDig((dispCO2 % 100) / 10, x + 8, y);
  drawDig(dispCO2 % 10 , x + 12, y);
  lcd.setCursor(15, 0);
#if (DISPLAY_TYPE == 1)
  lcd.print("ppm");
#else
  lcd.print("p");
#endif
}

/**
 * Draw pressure
 */
void drawPres(int dispPres, byte x, byte y) {   // Давление крупно на главном экране (с)НР ----------------------------
  drawDig((dispPres % 1000) / 100, x , y);
  drawDig((dispPres % 100) / 10, x + 4, y);
  drawDig(dispPres % 10 , x + 8, y);
  lcd.setCursor(x + 11, 1);
  if (bigDig) {
    lcd.setCursor(x + 11, 3);
  }
  lcd.print("mm");
}

/**
 * Draw temperature
 */
void drawTemp(float dispTemp, byte x, byte y) { // Температура крупно на главном экране (с)НР ----------------------------
  if (dispTemp / 10 == 0) {
    drawDig(10, x, y);
  } else {
    drawDig(dispTemp / 10, x, y);
  }
  drawDig(int(dispTemp) % 10, x + 4, y);
  drawDig(int(dispTemp * 10.0) % 10, x + 9, y);

  if (bigDig && DISPLAY_TYPE == 1) {
    lcd.setCursor(x + 7, y + 3);
    lcd.write(1);             // десятичная точка
  } else {
    lcd.setCursor(x + 7, y + 1);
    lcd.write(0B10100001);    // десятичная точка
  }
  lcd.setCursor(x + 13, y);
  lcd.write(223);             // градусы
}

/**
 * Draw humidity
 */
void drawHum(int dispHum, byte x, byte y) {   // Влажность крупно на главном экране (с)НР ----------------------------
  if (dispHum / 100 == 0) {
    drawDig(10, x, y);
  } else {
    drawDig(dispHum / 100, x, y);
  }
  if ((dispHum % 100) / 10 == 0) {
    drawDig(0, x + 4, y);
  } else {
    drawDig(dispHum / 10, x + 4, y);
  }
  drawDig(int(dispHum) % 10, x + 8, y);
  if (bigDig && DISPLAY_TYPE == 1) {
    lcd.setCursor(x + 12, y + 1);
    lcd.print("\245\4");
    lcd.setCursor(x + 12, y + 2);
    lcd.print("\5\245");
  } else {
    lcd.setCursor(x + 12, y + 1);
    lcd.print("%");
  }
}

/**
 * Draw clock
 */
void drawClock(byte hours, byte minutes, byte x, byte y) {    // рисуем время крупными цифрами -------------------------------------------
  if (hours > 23 || minutes > 59) {
    return;
  }
  if (hours / 10 == 0) {
    drawDig(10, x, y);
  } else {
    drawDig(hours / 10, x, y);
  }
  drawDig(hours % 10, x + 4, y);
  // тут должны быть точки. Отдельной функцией
  drawDig(minutes / 10, x + 8, y);
  drawDig(minutes % 10, x + 12, y);
}

/**
 * Draw data
 */
void drawData() {                     // выводим дату -------------------------------------------------------------
  int Y = 0;
  if (DISPLAY_TYPE == 1 && mode0scr == 1) {
    Y = 2;
  }
  if (!bigDig) {              // если 4-х строчные цифры, то дату, день недели (и секунды) не пишем - некуда (с)НР
    lcd.setCursor(15, 0 + Y);
    if (now.day() < 10) {
      lcd.print(0);
    }
    lcd.print(now.day());
    lcd.print(".");
    if (now.month() < 10) {
      lcd.print(0);
    }
    lcd.print(now.month());

    if (DISP_MODE == 0) {
      lcd.setCursor(16, 1);
      lcd.print(now.year());
    } else {
      loadClock();              // принудительно обновляем знаки, т.к. есть жалобы на необновление знаков в днях недели (с)НР
      lcd.setCursor(18, 1);
      int dayofweek = now.dayOfTheWeek();
      lcd.print(dayNames[dayofweek]);
      // if (hrs == 0 && mins == 0 && secs <= 1) loadClock();   // Обновляем знаки, чтобы русские буквы в днях недели тоже обновились. (с)НР
    }
  }
}

/**
 * Draw plot
 */
void drawPlot(byte pos, byte row, byte width, byte height, int min_val, int max_val, int *plot_array, String label1, String label2, int stretch) {  // график ---------------------------------
  int max_value = -32000;
  int min_value = 32000;

  for (byte i = 0; i < 15; i++) {
    max_value = max(plot_array[i] , max_value);
    min_value = min(plot_array[i] , min_value);
  }

  // меняем пределы графиков на предельные/фактические значения, одновременно рисуем указатель пределов (стрелочки вверх-вниз) (с)НР
  lcd.setCursor(15, 0);
  if ((MAX_ONDATA & (1 << (stretch - 1))) > 0) {    // побитовое сравнение 1 - растягиваем, 0 - не растягиваем (по указанным пределам) (с)НР
    //    max_val = min(max_value, max_val);
    //    min_val = max(min_value, min_val);
    max_val = max_value;
    min_val = min_value;
#if (DISPLAY_TYPE == 1)
    lcd.write(0b01011110);
    lcd.setCursor(15, 3);
    lcd.write(0);
#endif
  } else {
#if (DISPLAY_TYPE == 1)
    lcd.write(0);
    lcd.setCursor(15, 3);
    lcd.write(0b01011110);
#endif
  }

  if (min_val >= max_val) max_val = min_val + 1;
#if (DISPLAY_TYPE == 1)
  lcd.setCursor(15, 1); lcd.write(0b01111100);
  lcd.setCursor(15, 2); lcd.write(0b01111100);

  DEBUGLN(max_val);DEBUGLN(min_val);  // отладка (с)НР

  lcd.setCursor(16, 0); lcd.print(max_value);
  lcd.setCursor(16, 1); lcd.print(label1); lcd.print(label2);
  lcd.setCursor(16, 2); lcd.print(plot_array[14]);
  lcd.setCursor(16, 3); lcd.print(min_value);
#else
  lcd.setCursor(12, 0); lcd.print(label1);
  lcd.setCursor(13, 0); lcd.print(max_value);
  lcd.setCursor(12, 1); lcd.print(label2);
  lcd.setCursor(13, 1); lcd.print(min_value);
#endif
  for (byte i = 0; i < width; i++) {                  // каждый столбец параметров
    int fill_val = plot_array[i];
    fill_val = constrain(fill_val, min_val, max_val);
    byte infill, fract;
    // найти количество целых блоков с учётом минимума и максимума для отображения на графике
    if ((plot_array[i]) > min_val) {
      infill = floor((float)(plot_array[i] - min_val) / (max_val - min_val) * height * 10);
    } else {
      infill = 0;
    }
    fract = (float)(infill % 10) * 8 / 10;            // найти количество оставшихся полосок
    infill = infill / 10;

    for (byte n = 0; n < height; n++) {     // для всех строк графика
      if (n < infill && infill > 0) {       // пока мы ниже уровня
        lcd.setCursor(i, (row - n));        // заполняем полными ячейками
        lcd.write(255);
      }
      if (n >= infill) {                    // если достигли уровня
        lcd.setCursor(i, (row - n));
        if (n == 0 && fract == 0) {         // если нижний перел графика имеет минимальное значение, то рисуем одну полоску, чтобы не было пропусков (с)НР
          fract++;
        }
        if (fract > 0) {                   // заполняем дробные ячейки
          lcd.write(fract);
        } else {                           // если дробные == 0, заливаем пустой
          lcd.write(16);
        }
        for (byte k = n + 1; k < height; k++) { // всё что сверху заливаем пустыми
          lcd.setCursor(i, (row - k));
          lcd.write(16);
        }
        break;
      }
    }
  }
}

/**
 * Load clock 
 */
void loadClock() {
  if (bigDig && (DISPLAY_TYPE == 1)) {              // для четырехстрочных цифр (с)НР
    lcd.createChar(0, UT);
    lcd.createChar(1, row3);
    lcd.createChar(2, UB);
    lcd.createChar(3, row5);
    lcd.createChar(4, KU);
    lcd.createChar(5, KD);
  } else {                                            // для двустрочных цифр (с)НР
    lcd.createChar(0, row2);
    lcd.createChar(1, UB);
    lcd.createChar(2, row3);
    lcd.createChar(3, UMB);
    lcd.createChar(4, LMB);
    lcd.createChar(5, LM2);
  }

#if (LANG == 1)
  if (now.dayOfTheWeek() == 4) {          // Для четверга в ячейку запоминаем "Ч", для субботы "Б", иначе "П" (с)НР
    lcd.createChar(7, CH);  // Ч (с)НР
  } else if (now.dayOfTheWeek() == 6) {
    lcd.createChar(7, BB);  // Б (с)НР
  } else {
    lcd.createChar(7, PP);  // П (с)НР
  }
#endif
}
