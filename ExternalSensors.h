/**
*	\file ExternalSensors.h
*	\brief ExternalSensors Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/

#ifndef ExternalSensors_H
#define ExternalSensors_H


#include"Arduino.h"  
#include"Wire.h"
#include"OneWire.h"

#include"ExternalSensor.h"

/**
*	\class ExternalSensors
*	\brief This class handles the external sensors
*	run time defintion , configuartion and actions
*
*/
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
		String reference="";
		String type="";
		String connection="";
		int dataSize=0;
		uint8_t address=0;	
		BaseExternalSensor *exSensor=NULL;
	}sensors[50];

int sensorsNumber=0;
int jsonSize=100;

};

#endif
