/**
*	ExternalSensors.h
*	This class handles the external sensors
*	run time defintion , configuartion and actions
*
*/

#ifndef ExternalSensors_H
#define ExternalSensors_H


#include"Arduino.h"  
#include"Wire.h"
#include"OneWire.h"

#include"ExternalSensor.h"

class ExternalSensors 
{
public:

	void begin(); 


	String read ();

	bool config();

	int getJsonSize();
	void printConf();

private:
	struct sensor
	{		
		String reference;
		String type;
		String connection;
		int dataSize;
		uint8_t address;	
		BaseExternalSensor *exSensor=NULL;
	}sensors[50];

int sensorsNumber;
int jsonSize;

};

#endif
