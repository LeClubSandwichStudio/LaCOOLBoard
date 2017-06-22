#ifndef BaseExternalSensor_H
#define BaseExternalSensor_H

#include<NDIR_I2C.h>
#include<DallasTemperature.h>
#include"Arduino.h"  

//Base Classe
class BaseExternalSensor
{
public:
BaseExternalSensor()
{
}

virtual uint8_t begin()
{
Serial.println("base");
return(-2);
}

virtual int read()
{
	Serial.println("base reading");	
	return(-1);
}

private:
	
};



//Template Derived Classes 
template<class T >
class ExternalSensor : public BaseExternalSensor
{
public :
	//General Functions
	ExternalSensor()
	{
		sensor();
	}
	
	virtual uint8_t begin()
	{
		Serial.println("generic type ");
		return(sensor.begin() );	
	}

	virtual int read()
	{
		Serial.println("generic read ");
		return(0);
	}



private :
	
	T sensor; //the sensor itself
	


};

//NDIR Specialization 

template<>
class ExternalSensor<NDIR_I2C> :public BaseExternalSensor
{
public:
	ExternalSensor(uint8_t i2c_addr)
	{
		sensor=NDIR_I2C(i2c_addr);
	}
	
	virtual uint8_t begin()
	{
		Serial.println("NDIR shit ");
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

	virtual int read()
	{
	Serial.println("reading co2");
		if (sensor.measure())
		{
			Serial.println("reading co2");
			Serial.println(sensor.ppm);
			return(sensor.ppm);
			
		}
		
		else
		{
			Serial.println("not reading co2");
			return(-42);
		}
	}

private:

	NDIR_I2C sensor=NULL;
};

//Dallas Temperature Specialization 
template<>
class ExternalSensor<DallasTemperature> :public BaseExternalSensor
{
public:
	ExternalSensor()
	{
		OneWire oneWire(0);
		
		sensor=DallasTemperature(&oneWire);
	}
	
	virtual uint8_t begin()
	{
		
		sensor.begin(); 
		delay(5);
		sensor.getAddress(this->dallasAddress, 1);	
		return(true);
	}

	virtual int read()
	{
		 return( (int) sensor.getTempC(this->dallasAddress) );
	}

private:

	DallasTemperature sensor=NULL;
	DeviceAddress dallasAddress;
};


#endif
