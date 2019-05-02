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

#include "CoolBoardActor.h"

CoolBoardActor actor;

void setup() {
  Serial.begin(115200);
  actor.begin();
}

void loop() {
  actor.write(1);
  delay(500);
  actor.write(0);
  delay(500);
}
