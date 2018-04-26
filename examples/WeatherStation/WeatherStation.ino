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
  coolBoard.config();
  coolBoard.begin();
}

void loop() {
  if (coolBoard.isConnected() == 0) {
    coolBoard.onLineMode();
  } else {
    coolBoard.offLineMode();
  }
}
