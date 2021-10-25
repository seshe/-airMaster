/**
   Plot recalculation timers
*/
void plotSensorsTick() {
  // 4 или 5 минутный таймер
  if (hourPlotTimer.isReady()) {
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
  if (dayPlotTimer.isReady()) {
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
  if (predictTimer.isReady()) {
    // тут делаем линейную аппроксимацию для предсказания погоды
    unsigned long averPress = 0;
    for (byte i = 0; i < 10; i++) {
      averPress += bme.readPressure();
      myDelayMicroseconds(1000);
    }
    averPress *= 0.1; //the same as /10 but faster

    for (byte i = 0; i < 5; i++) {                   // счётчик от 0 до 5 (да, до 5. Так как 4 меньше 5)
      pressure_array[i] = pressure_array[i + 1];     // сдвинуть массив давлений КРОМЕ ПОСЛЕДНЕЙ ЯЧЕЙКИ на шаг назад
    }
    pressure_array[5] = averPress;                   // последний элемент массива теперь - новое давление
    unsigned long sumX = 0;
    unsigned long sumY = 0;
    unsigned long sumX2 = 0;
    unsigned long sumXY = 0;
    for (int i = 0; i < 6; i++) {                    // для всех элементов массива
      //sumX += time_array[i];
      sumX += i;
      sumY += (long)pressure_array[i];
      //sumX2 += time_array[i] * time_array[i];
      sumX2 += i * i;
      //sumXY += (long)time_array[i] * pressure_array[i];
      sumXY += (long)i * pressure_array[i];
    }
    float a = 0;
    a = (long)6 * sumXY;                            // расчёт коэффициента наклона приямой
    a = a - (long)sumX * sumY;
    a = (float)a / (6 * sumX2 - sumX * sumX);
    int delta = a * 6;      // расчёт изменения давления
    dispRain = map(delta, -250, 250, 100, -100);    // пересчитать в проценты
    DEBUGLN(String(pressure_array[5]) + " " + String(delta) + " " + String(dispRain));   // дебаг
  }
}

/**
   Draw plot
*/
void drawPlot(byte pos, byte row, byte width, byte height, int min_val, int max_val, int *plot_array, String label1, String label2, int stretch, bool isTemp) {  // график ---------------------------------
  int max_value = -32000;
  int min_value = 32000;
  for (byte i = 0; i < 15; i++) {
    max_value = max(plot_array[i] , max_value);
    min_value = min(plot_array[i] , min_value);
  }

  // меняем пределы графиков на предельные/фактические значения, одновременно рисуем указатель пределов (стрелочки вверх-вниз) (с)НР
  lcd.setCursor(15, 0);
  if ((MAX_ONDATA & (1 << (stretch - 1))) > 0) {    // побитовое сравнение 1 - растягиваем, 0 - не растягиваем (по указанным пределам) (с)НР
    max_val = max_value;
    min_val = min_value;

    lcd.write(0b01011110);
    lcd.setCursor(15, 3);
    lcd.write(0);

  } else {
    lcd.write(0);
    lcd.setCursor(15, 3);
    lcd.write(0b01011110);
  }

  if (min_val >= max_val) max_val = min_val + 1;
  lcd.setCursor(15, 1); lcd.write(0b01111100);
  lcd.setCursor(15, 2); lcd.write(0b01111100);

  DEBUGLN(max_val); DEBUGLN(min_val); // отладка (с)НР

  if (isTemp) {
    lcd.setCursor(16, 0); lcd.print(max_value / 10.0);
    lcd.setCursor(16, 3); lcd.print(min_value / 10.0);
    lcd.setCursor(16, 2); lcd.print(plot_array[14] / 10.0);
  } else {
    lcd.setCursor(16, 0); lcd.print(max_value);
    lcd.setCursor(16, 3); lcd.print(min_value);
    lcd.setCursor(16, 2); lcd.print(plot_array[14]);
  }
  lcd.setCursor(16, 1); lcd.print(label1); lcd.print(label2);
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
   Redraw plot
*/
void redrawPlot() {
  lcd.clear();
  switch (mode) {             // добавлена переменная для "растягивания" графика до фактических максимальных и(или) минимальных значений(с)НР
    case 1: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Hour, "c ", "hr", mode, false);
      break;
    case 2: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Day, "c ", "da", mode, false);
      break;
    case 3: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humHour, "h%", "hr", mode, false);
      break;
    case 4: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humDay, "h%", "da", mode, false);
      break;
    case 5: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempHour, "t\337", "hr", mode, true);
      break;
    case 6: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempDay, "t\337", "da", mode, true);
      break;
    case 7: drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressHour, "p ", "hr", mode, false);
      break;
    case 8: drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressDay, "p ", "da", mode, false);
      break;
  }
}
