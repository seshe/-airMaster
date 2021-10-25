
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
   Modes tick
*/
void modesTick() {
  button.tick();
  boolean changeFlag = false;
  //Display navigation

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
        if (mode > 8) {
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

  if ((mode == 0) && (button.isTriple())) {  // тройное нажатие в режиме главного экрана - переход в меню (с)НР
    mode = 255;
    podMode = 3;
    changeFlag = true;
  }


  if (button.isHolded()) {    // удержание кнопки (с)НР
    switch (mode) {
      case 0:
        //bigDig = !bigDig;
#if (BH1750_SENSOR == 1)
        luxFlag = !luxFlag;
#endif
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

        if (podMode == 1) {     // если Сохранить
          writeParam();
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

  //Show menu on display
  if (changeFlag) {
    if (mode >= 240) {
      lcd.clear();
      lcd.setCursor(0, 0);
    }
    if (mode == 255) {          // Перебираем варианты в главном меню (с)НР
      lcd.print(F("Setup:"));
      lcd.setCursor(0, 1);
      switch (podMode) {
        case 1:
          lcd.print(F("Save"));
          break;
        case 2:
          lcd.print(F("Exit"));
          break;
        case 5:
          lcd.print(F("indicator mode"));
          break;
        case 3:
          lcd.print(F("indicator brt."));
          break;
        case 4:
          lcd.print(F("Bright LCD"));
          break;
      }
      if (podMode >= 6 && podMode <= 17) {
        lcd.setCursor(10, 0);
        lcd.print(F("Charts  "));
        lcd.setCursor(0, 1);
        if ((3 & (1 << (podMode - 6))) != 0) {
          lcd.print(F("CO2 "));
        }
        if ((12 & (1 << (podMode - 6))) != 0) {
          lcd.print(F("Hum,%"));
        }
        if ((48 & (1 << (podMode - 6))) != 0) {
          lcd.print(F("t\337 "));
        }
        if ((192 & (1 << (podMode - 6))) != 0) {
#if (PRESSURE == 1)
          lcd.print(F("p,rain "));
#else
          lcd.print(F("p,mmPT "));
#endif
        }
        if ((768 & (1 << (podMode - 6))) != 0) {
          lcd.print(F("hgt,m  "));
        }

        if ((1365 & (1 << (podMode - 6))) != 0) {
          lcd.setCursor(8, 1);
          lcd.print(F("Hour:"));
        } else {
          lcd.setCursor(7, 1);
          lcd.print(F("Day: "));
        }
        if ((VIS_ONDATA & (1 << (podMode - 6))) != 0) {
          lcd.print(F("On  "));
        } else {
          lcd.print(F("Off "));
        }
      }
    }
    if (mode == 252) {                        // --------------------- показать  "Реж.индикатора"
      LEDType = podMode;
      lcd.setCursor(0, 0);
      lcd.print(F("indicator mode:"));
      lcd.setCursor(0, 1);
      switch (podMode) {
        case 0:
          lcd.print(F("CO2   "));
          break;
        case 1:
          lcd.print(F("Humid."));
          break;
        case 2:
          lcd.print(F("t\337     "));
          break;
        case 3:
          lcd.print(F("rain  "));
          break;
        case 4:
          lcd.print(F("pressure"));
          break;
      }

    }
    if (mode == 253) {                        // --------------------- показать  "Ярк.экрана"
      lcd.print(F("Bright LCD:"));
      if (LCD_BRIGHT == 11) {
        lcd.print(F("Auto "));
      } else {
        lcd.print((LCD_BRIGHT * 10));
        lcd.print(F("%"));
      }
    }
    if (mode == 254) {                        // --------------------- показать  "Ярк.индикатора"
      lcd.print(F("indic.brt.:"));
      if (LED_BRIGHT == 11) {
        lcd.print(F("Auto "));
      } else {
        lcd.print((LED_BRIGHT * 10));
        lcd.print(F("%"));
      }
    }

    if (mode == 0) {
      lcd.clear();
      loadClock();
      drawSensors();
      drawInfoFromRadio();
      drawData();
      drawClock(hrs, mins);
      dispPowerStatus(true);
    } else if (mode <= 8) {
      loadPlot();
      redrawPlot();
    }
  }
}
