/**
 *	CoolBoardExample
 *
 *	This example shows typical use
 *	of the CoolBoard.
 *
 *	Save this example in another location
 *	in order to safely modify the configuration files
 *	in the data folder.
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
