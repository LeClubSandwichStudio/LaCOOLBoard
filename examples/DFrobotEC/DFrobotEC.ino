/**
 *	CoolBoardExample
 *
 *	This example shows typical use
 *	of the CoolBoard with IRN3000 Ph/Temp interface 
 *  and the DFrobot EC kit. connect the EC kit to 
 *  ADC2 on IRN3000 and you should be ready to go.
 *  Don't forget to power the device from IRN3000.
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
