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

#include <CoolBoardSensors.h>

CoolBoardSensors sensors;

void setup() {
  Serial.begin(115200);
  sensors.allActive();
  sensors.begin();
}

void loop() {
  Serial.println("Sensor data dump:");
  Serial.println(sensors.read());
  delay(1000);

  Serial.println("Individual readings:");
  Serial.print("Battery voltage: ");
  Serial.println(sensors.readVBat());
  Serial.print("Earth moisture: ");
  Serial.println(sensors.readMoisture());
  Serial.print("Visible light: ");
  Serial.println(sensors.lightSensor.ReadVisible());
  Serial.print("InfraRed light: ");
  Serial.println(sensors.lightSensor.ReadIR());
  Serial.print("Ultraviolet light: ");
  Serial.println(sensors.lightSensor.ReadUV() / 100);
  Serial.print("Pressure: ");
  Serial.println(sensors.envSensor.readFloatPressure());
  Serial.print("Air humidity: ");
  Serial.println(sensors.envSensor.readFloatHumidity());
  Serial.print("Temperature:");
  Serial.print(sensors.envSensor.readTempC());
  Serial.println("°C");
  delay(1000);
}
