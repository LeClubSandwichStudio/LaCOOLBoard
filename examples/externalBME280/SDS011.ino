/**
 *	CoolBoardExample
 *
 *	This example shows how to configurate a second BME280 to the coolboard
 *
 */

#include <CoolBoard.h>

CoolBoard coolBoard;

void setup() {
  Serial.begin(115200);
  coolBoard.begin();
}

void loop() {
  coolBoard.loop();
}
