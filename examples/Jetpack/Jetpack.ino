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

#include <Jetpack.h>

Jetpack jetpack;

void setup() {
  jetpack.begin();
}

void loop() {
  jetpack.write(0xFF); // writes to the Jetpack
  delay(1000);
  jetpack.writeBit(3, 0); // writes to a single pin of the Jetpack
  delay(1000);
}
