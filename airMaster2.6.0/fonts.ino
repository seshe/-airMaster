// символы для отображния графиков
byte row7[8] = {0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
byte row6[8] = {0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
byte row5[8] = {0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
byte row4[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111};
byte row3[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};
byte row2[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};

byte rowS[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b10001,  0b01010,  0b00100,  0b00000};   // стрелка вниз (с)НР

// символы для больших цифр
byte LT[8] = {0b00111,  0b01111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
byte UB[8] = {0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};
byte RT[8] = {0b11100,  0b11110,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
byte LL[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b01111,  0b00111};
byte LR[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11110,  0b11100};
byte UMB[8] = {0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};

byte DRP[8] = {0b00100,  0b01110,  0b01110,  0b11111,  0b10111,  0b10011,  0b01110,  0b00000}; // символ "капля" - прогноз дождя

/*
  byte customChar[8] = {
  0b11110,
  0b01010,
  0b00100,
  0b11101,
  0b10111,
  0b01000,
  0b10111,
  0b11101
  };

  byte customChar2[8] = {
  0b11110,
  0b01010,
  0b00101,
  0b11111,
  0b00000,
  0b11111,
  0b10001,
  0b11111
  };
*/
/**
   Load clock
*/
void loadClock() {
  lcd.createChar(0, LT);
  lcd.createChar(1, UB);
  lcd.createChar(2, RT);
  lcd.createChar(3, LL);
  lcd.createChar(4, row2);
  lcd.createChar(5, LR);
  lcd.createChar(6, UMB);
  lcd.createChar(7, DRP);
}

/**
   Load plot
*/
void loadPlot() {
  // символы для отображния графиков
  byte row1[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111};

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
   Draw digit
*/
void drawDig(byte dig, byte x, byte y) {        // рисуем цифры (с)НР ---------------------------------------

  char s1[4] = { 32, 32, 32, 32 };
  char s2[4] = { 32, 32, 32, 32 };
  byte i;
  byte cnt = 3;

  //if (clean) cnt = 4;

  switch (dig) {
    case 0:
      s1[0] = 0;
      s1[1] = 1;
      s1[2] = 2;
      s2[0] = 3;
      s2[1] = 4;
      s2[2] = 5;
      break;
    case 1:
      s1[1] = 0;
      s2[1] = 255;
      break;
    case 2:
      s1[0] = 1;
      s1[1] = 1;
      s1[2] = 2;
      s2[0] = 0;
      s2[1] = 6;
      s2[2] = 6;
      break;
    case 3:
      s1[0] = 1;
      s1[1] = 6;
      s1[2] = 5;
      s2[0] = 4;
      s2[1] = 4;
      s2[2] = 5;
      break;
    case 4:
      s1[0] = 0;
      s1[2] = 255;
      s2[0] = 1;
      s2[1] = 1;
      s2[2] = 255;
      break;
    case 5:
      s1[0] = 255;
      s1[1] = 6;
      s1[2] = 6;
      s2[0] = 4;
      s2[1] = 4;
      s2[2] = 5;
      break;
    case 6:
      s1[0] = 0;
      s1[1] = 6;
      s1[2] = 6;
      s2[0] = 3;
      s2[1] = 4;
      s2[2] = 5;
      break;
    case 7:
      s1[0] = 1;
      s1[1] = 1;
      s1[2] = 5;
      s2[1] = 0;
      break;
    case 8:
      s1[0] = 3;
      s1[1] = 6;
      s1[2] = 5;
      s2[0] = 3;
      s2[1] = 4;
      s2[2] = 5;
      break;
    case 9:
      s1[0] = 0;
      s1[1] = 1;
      s1[2] = 2;
      s2[0] = 6;
      s2[1] = 6;
      s2[2] = 5;
      break;
    case 10:
      break;
  }

  lcd.setCursor(x, y);
  for (i = 0; i < cnt; i++) lcd.write(s1[i]);
  lcd.setCursor(x, y + 1);
  for (i = 0; i < cnt; i++) lcd.write(s2[i]);
}
