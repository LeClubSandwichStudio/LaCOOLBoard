#include<ExternalSensor.h>
#include<Wire.h>

ExternalSensor sensor<NDIR_I2C>(0x4D);

void setup()
{
	Wire.begin(2,14);
	Serial.begin(115200);
	sensor.begin();
}

void loop()
{
	Serial.println(sensor.read());
	delay(1000);
}
