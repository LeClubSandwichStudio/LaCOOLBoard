/**
 *	SHT1x CoolBoardExample
 *
 *	This example shows typical use
 *	of the SHT1x with the CoolBoard.
 *
 *  WARNING:
 *	SHT1x uses the same pins as the JetPack,
 *  do not use both at the same time.
 *
 *  Data pin requires a 10k pull-up resistor
 *
 *	Wiring:
 *  Yellow -> Clock Pin = GPIO5
 *  Black  -> Data  Pin = GPIO4
 *  Blue   -> GND
 *  Red    -> 3.3V
 *
 *	Save this example to another location
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

void loop() { coolBoard.loop(); }
