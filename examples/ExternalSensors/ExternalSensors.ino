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

#include "ExternalSensors.h"

ExternalSensors externalSensors;
String reference[] = {"NDIR_I2C"};
String type[] = {"CO2"};
uint8_t address[] = {77};
int sensorsNumber = 1;

void setup() {
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH); // HIGH= I2C Enable
  Serial.begin(115200);
  externalSensors.config(reference, type, address, sensorsNumber);
  externalSensors.begin();
  externalSensors.printConf();
}

void loop() {
  Serial.println(externalSensors.read());
  delay(1000);
}
