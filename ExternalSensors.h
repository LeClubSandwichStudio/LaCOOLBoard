#ifndef ExternalSensors_H
#define ExternalSensors_H


#include"Arduino.h"  
#include"Wire.h"
#include"OneWire.h"
#include"GenericSensor.h"
#include"ExternalSensor.h"

class ExternalSensors : public GenericSensor
{
public:

virtual void begin(); // inherited


String read ();

bool config();

int jsonSize();
void printConf();

private:
struct sensor{		
	String reference;
	String type;
	String connection;
	int dataSize;
	uint8_t address;	
	BaseExternalSensor *exSensor=NULL;
}sensors[50];

int sensorsNumber;
int jsonSizeVar;

};

#endif
