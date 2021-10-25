
/**
   Check brightness
*/
void checkBrightness() {
  if (!scrOn) {
    lcd.noBacklight();
    fastWrite(BACKLIGHT, LOW);
    return;
  }

  if (LCD_BRIGHT == 11) {
#if (BH1750_SENSOR == 1)
    byte l_light = max ( min ( lightMeter.readLightLevel(), LIGHT_MAX), LIGHT_MIN ); // яркость освещения

    if ((byte)(l_light - LIGHT) < 5) {
      LIGHT = l_light;
    } else {
      LIGHT = (float)(( LIGHT * 2 + l_light) / 3); // резко увеличилась освещённость - плавно подтянем к нужному значению - исключим мигание при переходе вперед-назад на 1 пункт
    }
    dutyA0 = map(LIGHT, LIGHT_MIN, LIGHT_MAX, 1, PWM_DEPTH);
#endif
  } else {
    dutyA0 = map((LCD_BRIGHT * 10 * 2.2), LIGHT_MIN, LIGHT_MAX, 1, PWM_DEPTH);
  }

#if (BH1750_SENSOR == 1)
  /*
    byte l_light = max ( min ( lightMeter.readLightLevel(), LIGHT_MAX), LIGHT_MIN ); // яркость освещения
    if ((byte)(l_light - LIGHT) < 10) {
      LIGHT = l_light;
    } else {
      LIGHT = (float)(( LIGHT * 2 + l_light) / 3); // резко увеличилась освещённость - плавно подтянем к нужному значению - исключим мигание при переходе вперед-назад на 1 пункт
    }*/
#endif
  /*
    if (LCD_BRIGHT == 11) {                         // если установлен автоматический режим для экрана (с)НР
    #if (BH1750_SENSOR == 1)
      if (LIGHT >= LCD_BRIGHT_MAX) {
        analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);
      } else if (LIGHT <= LCD_BRIGHT_MIN) {
        analogWrite(BACKLIGHT, LCD_BRIGHT_MIN);
      } else {
        analogWrite(BACKLIGHT, LIGHT);
      }
    #endif
    } else {
      analogWrite(BACKLIGHT, LCD_BRIGHT * LCD_BRIGHT * 2.5);
    }
  */
}

// ****************************************************************************************
// * Устанавливает цвет и яркость свтодиода СО2                                           *
// ****************************************************************************************
void setLED(byte color) {

  byte L_R = 0;
  byte L_G = 0;
  byte L_B = 0;
  byte led_bright = 255 / 100 * LED_BRIGHT * LED_BRIGHT;

#if (BH1750_SENSOR == 1)
  if (LED_BRIGHT == 11) {
    if (LIGHT >= LED_BRIGHT_MAX) {
      led_bright = LED_BRIGHT_MAX;
    } else if (LIGHT <= LED_BRIGHT_MIN) {
      led_bright = LED_BRIGHT_MIN;
    } else {
      led_bright = LIGHT;
    }
  }
#endif

  switch (color) {    // 0 выкл, 1 красный, 2 зелёный, 3 жёлтый, 4 синий
    case 0:
      break;
    case 1:
      L_R = led_bright;
      break;
    case 2:
      L_G = led_bright;
      break;
    case 3:
      L_R = led_bright / 2;
      L_G = led_bright / 4;
      break;
    case 4:
      L_B = led_bright;
      break;
  }

  analogWrite(LED_R, L_R);
  analogWrite(LED_G, L_G);
  analogWrite(LED_B, L_B);
}

#if (BH1750_SENSOR == 1)
/*
   Show lux
*/
String predareLux(unsigned int value) {           //MH convert LUX to string
  String luxvalue = String(value);
  switch (luxvalue.length()) {
    case 1:
      luxvalue = " " + luxvalue + "Lux";
      break;
    case 2:
      luxvalue = luxvalue + "Lux";
      break;
    case 3:
      luxvalue = luxvalue + "Lx";
      break;
    case 4:
      luxvalue = luxvalue + "L";
      break;
  }
  return (luxvalue);
}
#endif
