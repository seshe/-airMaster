#define IDLE_TIME 600 //sec
#define SENSOR_DELAY 2.5 //sec

unsigned int moveCount = IDLE_TIME;

// ****************************************************************************************
// * Читает показания с датчика движения, отключает экран если движения нет               *
// ****************************************************************************************
void checkMotion() {
  if (MOTION_CONTROL) {
    byte wasMove = fastRead(PIN_PIR);
    if (wasMove) {
      if (moveCount < IDLE_TIME) {
        if (moveCount > SENSOR_DELAY && moveCount < 10) {
          moveCount += 10;
        }
        moveCount++;
      }
    } else if (moveCount != 0 && !wasMove) {
      moveCount--;
    }

    if (!scrOn && moveCount > SENSOR_DELAY) {
      scrOn = true;
      drawSensors();
      drawInfoFromRadio();
      drawClock(hrs, mins);
      drawData();
      checkBrightness();
      lcd.backlight();
      dispPowerStatus(true);
    } else if (scrOn && moveCount == 0) {
      lcd.clear();
      lcd.noBacklight();
      fastWrite(BACKLIGHT, LOW);
      scrOn = false;
    }
  }
}
