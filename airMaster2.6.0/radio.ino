void readRadio() {
#if (RADIO == 1)
  //byte pipeNo, gotByte;

  //while( radio.available(&pipeNo)){    // слушаем эфир со всех труб
  //  radio.read( &gotByte, 1 );         // чиатем входящий сигнал
  //  radio.writeAckPayload(pipeNo,&gotByte, 1 );  // отправляем обратно то что приняли
  //  Serial.print("Recieved: "); Serial.println(gotByte);
  //}
  //if (radio.available(&pipeNo)) {  // слушаем эфир со всех труб
  if (radio.available()) {  // слушаем эфир со всех труб
    radio.read( &transmit_data, sizeof(transmit_data) );         // чиатем входящий сигнал


    DEBUGLN("Recieved: ");

    DEBUG(transmit_data[0] / 10); DEBUGLN(" C");

    DEBUG(transmit_data[1]); DEBUGLN(" Hum");
    DEBUG(transmit_data[2]); DEBUGLN(" Press");
    DEBUG(transmit_data[3]); DEBUGLN(" V");
    DEBUG(transmit_data[4]); DEBUGLN(" C");
    DEBUG(transmit_data[5]); DEBUGLN(" V");
    DEBUGLN("end");

    drawInfoFromRadio();
  }

#endif

}

void drawInfoFromRadio() {
#if (RADIO == 1)
  if (scrOn && mode == 0 && mode0scr == 0) {
    if (transmit_data[2] > 600 && transmit_data[2] < 900)
    {
      lcd.setCursor(0, 3);
      lcd.print(String(transmit_data[0] / 10.0, 1));
      lcd.write(223);

      lcd.setCursor(5, 3);
      lcd.print(F(" "));
      lcd.print(transmit_data[1]);
      lcd.print(F("%"));
      /*
            lcd.setCursor(10, 3);
            lcd.print(transmit_data[2]);
            lcd.print(F("mm"));
      */
      lcd.setCursor(10, 3);
      lcd.print(String(transmit_data[4] / 10.0, 1));
      lcd.write(223);

      lcd.setCursor(16, 3);
      lcd.print(transmit_data[3]);


      lcd.setCursor(15, 1);
      lcd.print(F("     "));
      lcd.setCursor(15, 1);
      lcd.print(transmit_data[5]);
    }
  }
#endif
}
