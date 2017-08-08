#include"ExternalSensors.h"
#include<Wire.h>

ExternalSensors externalSensors;

String reference[]={"NDIR_I2C"};

String type[]={"CO2"};

uint8_t address[]={77};

int sensorsNumber=1;	

void setup()
{
	Wire.begin(2,14);

	digitalWrite(5,HIGH);//HIGH= I2C Enable

	Serial.begin(115200);

	externalSensors.config(reference,type,address,sensorsNumber);
	externalSensors.begin();
	externalSensors.printConf();
}

void loop()
{
	Serial.println(externalSensors.read());
	delay(1000);
}
