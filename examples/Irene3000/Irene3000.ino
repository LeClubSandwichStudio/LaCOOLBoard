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

#include <Irene3000.h>

Irene3000 irene;
double temperature;

void setup() {
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  Serial.begin(115200);
  irene.begin();
}

void loop() {
  temperature = irene.readTemp();
  Serial.println(irene.readPh(irene.gainConvert(temperature)));
  Serial.println(temperature);
  delay(2000);
}
