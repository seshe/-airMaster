/* (с)НР
  0 - Крупно время
  1 - Крупно содержание СО2
  2 - Крупно температура
  3 - Крупно давление
  4 - Крупно влажность
*/

/**
   Drow sensors
*/
void drawSensors() {
  if (mode0scr != 2) {                        // Температура (с)НР ----------------------------
    lcd.setCursor(0, 2);
    lcd.print(String(dispTemp / 10.0, 1));
    lcd.write(223);
  } else {
    drawTemp(dispTemp / 10.0, 0, 0);
  }

  if (mode0scr != 4) {                        // Влажность (с)НР ----------------------------
    lcd.setCursor(5, 2);
    lcd.print(F(" "));
    lcd.print(dispHum);
    lcd.print(F("%"));
  } else {
    drawHum(dispHum, 0, 0);
  }

#if (CO2_SENSOR == 1)
  if (mode0scr != 1) {                       // СО2 (с)НР ----------------------------

    if (dispCO2 >= 1000) {
      lcd.setCursor(9, 2);
    } else {
      lcd.setCursor(10, 2);
    }

    lcd.print(F(" "));
    lcd.print(dispCO2);
    lcd.print(F("ppm "));
  } else {
    drawPPM(dispCO2, 0, 0);
  }
#endif

  if (mode0scr != 3) {                      // Давление (с)НР ---------------------------
    //lcd.setCursor(15, 1);
    //lcd.print(dispPres);
    //lcd.print(F("mm"));
  } else {
    drawPres(dispPres, 0, 0);
  }

  // дождь (с)НР -----------------------------
  /*
  lcd.setCursor(0, 3);
  lcd.print(F("     "));
  lcd.setCursor(0, 3);

  lcd.write(7);
  lcd.setCursor(2, 3);
  if (dispRain < 0) {
    lcd.setCursor(1, 3);
  }
  lcd.print(dispRain);
  lcd.print(F("%"));
  */
}

/**
   Draw CO2
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
  lcd.print(F("ppm"));
}

/**
   Draw pressure
*/
void drawPres(int dispPres, byte x, byte y) {   // Давление крупно на главном экране (с)НР ----------------------------
  drawDig(dispPres / 100, x , y);
  drawDig((dispPres % 100) / 10, x + 4, y);
  drawDig(dispPres % 10 , x + 8, y);
  lcd.setCursor(x + 11, 1);
  lcd.print(F("mm"));
}

/**
   Draw temperature
*/
void drawTemp(float temp, byte x, byte y) { // Температура крупно на главном экране (с)НР ----------------------------
  if (temp / 10 == 0) {
    drawDig(10, x, y);
  } else {
    drawDig(temp / 10, x, y);
  }
  drawDig(int(temp) % 10, x + 4, y);
  drawDig(int(temp * 10.0) % 10, x + 9, y);
  lcd.setCursor(x + 7, y + 1);
  lcd.write(0B10100001);    // десятичная точка
  lcd.setCursor(x + 13, y);
  lcd.write(223);             // градусы
}

/**
   Draw humidity
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
  lcd.setCursor(x + 12, y + 1);
  lcd.print(F("%"));
}

/*
   Draw lux
*/
#if (BH1750_SENSOR == 1)
void drawLux() {
  if (luxFlag) {
    lcd.setCursor(15, 3);
    lcd.print(predareLux(lightMeter.readLightLevel()));
  }
}
#endif
