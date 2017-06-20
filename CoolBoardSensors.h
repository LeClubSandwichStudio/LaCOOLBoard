/*
* This class handles the On-Board Sensors.
* 
* It's just a wrapper
* 
*
*/

#ifndef CoolBoardSensors_H
#define CoolBoardSensors_H

#include "Arduino.h"
#include "SI114X.h"        // Light sensor Support
#include "SparkFunBME280.h"// Environmental sensor Support
#include "GenericSensor.h"

class CoolBoardSensors: public GenericSensor
{

public:

//Constructor
	CoolBoardSensors();


// inherited methods redifintion:

virtual void begin();



//data is in json
String read();

int jsonSize();
void setJsonSize(int jsonSize);
void allActive();

void initReadI2C();

void stopReadI2C();

//additional method
void end();

bool config();
void printConf();



//environment sensor methods

//set the enviornment sensor settings , if argument is ommitted , default value will be assigned
        void setEnvSensorSettings( uint8_t commInterface=I2C_MODE, uint8_t I2CAddress=0x76,    uint8_t runMode = 3,
					   
				   uint8_t tStandby=0	, uint8_t filter=0,    uint8_t tempOverSample=1,                          					   
				   uint8_t pressOverSample= 1,    uint8_t humidOverSample= 1);



//VBat
	float readVBat();

//Moisture

	int readMoisture();	
	
private:
	//sensor objects :
	
	SI114X lightSensor = SI114X();			// light sensor
	struct lightActive
	{
		byte visible;
		byte ir;
		byte uv;	

	}lightDataActive;

	BME280 envSensor;			        // environment sensor
	struct airActive
	{
		byte temperature;
		byte humidity;
		byte pressure;



	}airDataActive;

	const int EnMoisture = 13;                      // Moisture Enable Pin
	const int AnMplex = 12;                         // Analog Multiplexer  LOW=Vbat , HIGH=Moisture
	const int EnI2C = 5;                            // double usage for I2C and shift register latch , HIGH=I2C , LOW=shift register latch
							// All I2C is over pins (2,14)
        byte vbatActive;
	byte earthMoistureActive;

	int SENSOR_JSON_SIZE;//for the json output
};

#endif
