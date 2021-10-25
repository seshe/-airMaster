// ****************************************************************************************
// * Читает параметры из EEPROM и переопределяет значения по умолчанию                    *
// ****************************************************************************************
void readParam() {
  // байты:
  // 0 - заглушка, всегда 128
  // 1-2 - мин яркость дисплея LCD
  // 3-4 - макс яркость дисплея LCD
  // 5 - display mode
  // 6 - led bright
  // 7 - lcd bright
  // 8 - привязка индикатора к датчикам
  // 9 - CRC

  byte param[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (byte i = 0; i < 10; i++) EEPROM.get(i, param[i]);
  if ( param[0] == 128 && param[9] == CRC8(param, 9) ) { // данные верные, распакуем
    MAX_ONDATA = param[1] + (long)(param[2] << 8);

    VIS_ONDATA = param[3] + (long)(param[4] << 8);

    mode0scr = param[5];

    LED_BRIGHT = param[6];

    LCD_BRIGHT = param[7];

    LEDType = param[8];
  }
}

// ****************************************************************************************
// * Записывает параметры в EEPROM                                                        *
// ****************************************************************************************
void writeParam() {
  byte param[10] = {
    128,
    byte(MAX_ONDATA & 255),
    byte(MAX_ONDATA >> 8),
    byte(VIS_ONDATA & 255),
    byte(VIS_ONDATA >> 8),
    mode0scr,
    LED_BRIGHT,
    LCD_BRIGHT,
    LEDType,
    0
  };
  param[9] = CRC8(param, 9);
  DEBUGLN("CRC8");
  DEBUGLN(param[9]);
  for (byte i = 0; i < 10; i++) EEPROM.update(i, param[i]);
}

// ****************************************************************************************
// * Вычисляет контрольную сумму CRC8 Dallas/Maxim (для подписи данных в EEPROM)          *
// ****************************************************************************************
byte CRC8(const byte *data, byte len) {
  byte crc = 0x00;
  byte extract = 0;
  byte sum = 0;
  while (len--) {
    extract = *data++;
    for (byte tempI = 8; tempI; tempI--) {
      sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}
