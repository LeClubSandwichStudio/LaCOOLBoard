/**
*	\file ExternalSensor.h
*	\brief ExternalSensor Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/
#ifndef BaseExternalSensor_H
#define BaseExternalSensor_H

#include<NDIR_I2C.h>
#include<DallasTemperature.h>
#include"Arduino.h"  

/**
*	\class BaseExternalSensor:
*	\brief This class is a generic external Sensor
*	it is a way to access real external sensor
*	methods through run Time polymorphism
*/
class BaseExternalSensor
{

public:
	/**
	*	BaseExternalSensor():
	*	Base class generic Constructor
	*/
	BaseExternalSensor()
	{
		Serial.println("BaseExternalSensor Constructor");
		Serial.println();
	}
	/**
	*	begin():
	*	Base class virtual 
	*	generic begin method
	*	
	*	\return generic value as it's not supposed
	*	to be used
	*/
	virtual uint8_t begin()
	{
		Serial.println("BaseExternalSensor.begin()");
		Serial.println();

		return(-2);
	}
	
	/**
	*	read():
	*	Base class virtual
	*	generic read method
	*
	*	\return generic value
	*	as it is not supposed 
	*	to be used	
	*/
	virtual float read()
	{
		Serial.println("BaseExternalSensor.read()");
		Serial.println();		
		
		return(-2);
	}
	
};



/**
*	\class ExternalSensor	
*	\brief template<class SensorClass> class External Sensor: 
*	Derived class from BaseExternalSensor.
*
*	This is the generic Template for an external sensor
*	This class works automatically with sensors that 
*	provide the following methods :
*		- constructor(void);
*		- uint8_t/bool begin(void);
*		- int read(void);
*
*	If your sensor doesn't provide these methods
*	or is not present in the specialized templates
*	feel free to implement your own specializiation,
*	following the provided generic template , 
*	or contact us and we will be glad to expand our
*	list of supported external sensors
*/
template<class T >
class ExternalSensor : public BaseExternalSensor
{
public :
	/**
	*	Generic Constructor
	*/ 
	ExternalSensor()
	{
		Serial.println("ExternalSensor <Generic> Constructor");
		Serial.println();

		sensor();
	}

	/**
	*	Generic begin method
	*
	*	
	*/
	virtual uint8_t begin()
	{
		Serial.println("ExternalSensor <Generic> begin()");
		Serial.println();

		return(sensor.begin() );	
	}
	
	/**
	*	Generic read method
	*/
	virtual float read()
	{

		Serial.println("ExternalSensor <Generic> read() ");
		Serial.println();
		
		return(sensor.read());
	}



private :
	
	T sensor; //the sensor itself

};

/**
*	\class ExternalSensor<NDIR_I2C>	
*	\brief NDIR_I2C Specialization Class
*	This is the template specialization
*	for the NDIR_I2C CO2 sensor
*/
template<>
class ExternalSensor<NDIR_I2C> :public BaseExternalSensor
{
public:

	/**
	*	ExternalSensor(I2C address):
	*	NDIR_I2C specific constructor
	*/
	ExternalSensor(uint8_t i2c_addr)
	{
		Serial.println("ExternalSensor <NDIR_I2C> constructor");
		Serial.println();

		sensor=NDIR_I2C(i2c_addr);
	}
	
	/**
	*	begin():
	*	NDIR_I2C specific begin method
	*
	*	\return true if successful,
	*	false otherwise
	*/
	virtual uint8_t begin()
	{
		Serial.println("ExternalSensor <NDIR_I2C> begin()");
		Serial.println();

		 if (sensor.begin()) 
		{
			Serial.println("NDIR_I2C init : wait 10 seconds");
			Serial.println();

			delay(10000);
			return(true);
    		}
		 else 
		{
			Serial.println("NDIR_I2C init : fail ");
			Serial.println();

			return(false);
		}	
	}
	
	/**
	*	read():
	*	NDIR_I2C specific read method
	*
	*	\return the ppm value if successful,
	*	else return -42
	*/
	virtual float read()
	{
		Serial.println("ExternalSensor <NDIR_I2C> read()");
		Serial.println();

		if (sensor.measure())
		{
			Serial.print("NDIR_I2C ppm :");
			Serial.println( (float) sensor.ppm);
			
			Serial.println();			

			return( (float) sensor.ppm);
			
		}
		
		else
		{
			Serial.println("NDIR_I2C read fail ");
			Serial.println();

			return(-42);
		}
	}

private:

	NDIR_I2C sensor=NULL;
};

/**
*	\class ExternalSensor<DallasTemperature>	
*	\brief DallasTemperature Specialization Class
*	This is the template specialization
*	for the Dallas Temperature sensor
*/
template<>
class ExternalSensor<DallasTemperature> :public BaseExternalSensor
{
public:
	/**
	*	ExternalSensor():
	*	DallasTemperature specific constructor
	*/
	ExternalSensor()
	{
		Serial.println("ExternalSensor <DallasTemperature> constructor");
		Serial.println();

		OneWire oneWire(0);
		
		sensor=DallasTemperature(&oneWire);
	}
	
	/**
	*	begin():
	*	DallasTemperature specific begin method
	*
	*	\return true if successful
	*/
	virtual uint8_t begin()
	{
		Serial.println("ExternalSensor <DallasTemperature> begin()");
		Serial.println();
		
		sensor.begin(); 
		delay(5);
		sensor.getAddress(this->dallasAddress, 1);	
		return(true);
	}
	/**
	*	read():
	*	DallasTemperature specific read method
	*
	*	\return the temperature in °C
	*/
	virtual float read()
	{
		Serial.println("ExternalSensor <DallasTemperature> read()");
		Serial.println();

		Serial.print("temperature : ");
		Serial.print( (float) sensor.getTempC(this->dallasAddress) );
		Serial.print("°C");
		Serial.println();
		
		return( (float) sensor.getTempC(this->dallasAddress) );
	}

private:

	DallasTemperature sensor=NULL;
	DeviceAddress dallasAddress;
};


#endif
