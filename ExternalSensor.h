/**
*	ExternalSensor.h
*	This class handles the actual	
*	usage and creation of the external
*	sensors
*/

#ifndef BaseExternalSensor_H
#define BaseExternalSensor_H

#include<NDIR_I2C.h>
#include<DallasTemperature.h>
#include"Arduino.h"  

/**
*	BaseExternalSensor:
*	This class is a generic external Sensor
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
	
	}
	/**
	*	begin():
	*	Base class virtual 
	*	generic begin method
	*/
	virtual uint8_t begin()
	{

		return(-2);
	}
	
	/**
	*	read():
	*	Base class virtual
	*	generic read method
	*/
	virtual int read()
	{

		return(-1);
	}
	
};



/**
*	template<class SensorClass> class External Sensor: 
*	Derived class from BaseExternalSensor.
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
		sensor();
	}

	/**
	*	Generic begin method
	*/
	virtual uint8_t begin()
	{

		return(sensor.begin() );	
	}
	
	/**
	*	Generic read method
	*/
	virtual int read()
	{

		return(0);
	}



private :
	
	T sensor; //the sensor itself

};

/**
*	NDIR_I2C Specialization Class
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
		sensor=NDIR_I2C(i2c_addr);
	}
	
	/**
	*	begin():
	*	NDIR_I2C specific begin method
	*/
	virtual uint8_t begin()
	{

		 if (sensor.begin()) 
		{

			delay(10000);
			return(true);
    		}
		 else 
		{
			return(false);
		}	
	}
	
	/**
	*	read():
	*	NDIR_I2C specific read method
	*/
	virtual int read()
	{

		if (sensor.measure())
		{


			return(sensor.ppm);
			
		}
		
		else
		{

			return(-42);
		}
	}

private:

	NDIR_I2C sensor=NULL;
};

/**
*	DallasTemperature Specialization Class
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
		OneWire oneWire(0);
		
		sensor=DallasTemperature(&oneWire);
	}
	
	/**
	*	begin():
	*	DallasTemperature specific begin method
	*/
	virtual uint8_t begin()
	{
		
		sensor.begin(); 
		delay(5);
		sensor.getAddress(this->dallasAddress, 1);	
		return(true);
	}
	/**
	*	read():
	*	DallasTemperature specific read method
	*/
	virtual int read()
	{
		 return( (int) sensor.getTempC(this->dallasAddress) );
	}

private:

	DallasTemperature sensor=NULL;
	DeviceAddress dallasAddress;
};


#endif
