/**
*	\file	ExternalSensor.h
*  	\brief	ExternalSensor Header file
*	\version 1.0  
*	\author	Mehdi Zemzem
*	\version 0.0 
*	\author  Simon Juif
*  	\date	27/06/2017
*	\copyright La Cool Co SAS 
*	\copyright MIT license
*	Copyright (c) 2017 La Cool Co SAS
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*
*/

#ifndef BaseExternalSensor_H
#define BaseExternalSensor_H

#include "internals/CoolNDIR_I2C.h"
#include <DallasTemperature.h>
#include "internals/CoolAdafruit_TCS34725.h"
#include "Arduino.h" 

 
#define DEBUGExternal 0


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

	#if DEBUGExternal == 1 

		Serial.println( "BaseExternalSensor Constructor" );
		Serial.println();
	
	#endif

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
	
	#if DEBUGExternal == 1 
	
		Serial.println( "BaseExternalSensor.begin()" );
		Serial.println();
	
	#endif

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
	
	#if DEBUGExternal == 1 

		Serial.println( "BaseExternalSensor.read()" );
		Serial.println();
	
	#endif		
		
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
	virtual float read(uint16_t *r,uint16_t *g,uint16_t *b,uint16_t *c,uint16_t *colorTemp,uint16_t *lux)
	{
		return(-42.42);	
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
*		- float read(void);
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
	
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <Generic> Constructor" );
		Serial.println();
	
	#endif

		sensor();
	}

	/**
	*	Generic begin method
	*
	*	
	*/
	virtual uint8_t begin()
	{
	
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <Generic> begin()" );
		Serial.println();
	
	#endif

		return(sensor.begin() );	
	}
	
	/**
	*	Generic read method
	*/
	virtual float read()
	{
	
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <Generic> read() " );
		Serial.println();
		
	#endif

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
	
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <NDIR_I2C> constructor");
		Serial.println();
	
	#endif

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
	
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <NDIR_I2C> begin()" );
		Serial.println();
	
	#endif 

		if (sensor.begin()) 
		{
		
		#if DEBUGExternal == 1 
			
			Serial.println( "NDIR_I2C init : wait 10 seconds" );
			Serial.println();
		
		#endif

			delay(10000);
			return(true);

    		}
		else 
		{
		
		#if DEBUGExternal == 1 

			Serial.println( "NDIR_I2C init : fail " );
			Serial.println();
		
		#endif

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
		
	#if DEBUGExternal == 1 
		
		Serial.println( "ExternalSensor <NDIR_I2C> read()" );
		Serial.println();

	#endif

		if (sensor.measure())
		{
		
		#if DEBUGExternal == 1 

			Serial.print( "NDIR_I2C ppm :" );
			Serial.println( (float) sensor.ppm);
			
			Serial.println();			

		#endif

			return( (float) sensor.ppm);
			
		}
		
		else
		{
		
		#if DEBUGExternal == 1 

			Serial.println( "NDIR_I2C read fail " );
			Serial.println();
		
		#endif

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
	ExternalSensor(OneWire* oneWire)
	{
		
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <DallasTemperature> constructor" );
		Serial.println();
	
	#endif
		sensor=DallasTemperature(oneWire);
	}
	
	/**
	*	begin():
	*	DallasTemperature specific begin method
	*
	*	\return true if successful
	*/
	virtual uint8_t begin()
	{
	
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <DallasTemperature> begin()" );
		Serial.println();
	
	#endif

		sensor.begin(); 
		delay(5);
		sensor.getAddress(this->dallasAddress, 0);	
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

		sensor.requestTemperatures(); // Send the command to get temperatures
		float result=(float) sensor.getTempCByIndex(0);
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <DallasTemperature> read()" );
		Serial.println();

		Serial.print("Requesting temperature...");

		Serial.print( "temperature : ");
		Serial.print( result );
		Serial.print( "°C" );
		Serial.println();
	
	#endif
		
		return( result );
	}

private:


	DallasTemperature sensor;
	DeviceAddress dallasAddress;
};


/**
*	\class ExternalSensor<Adafruit_TCS34725>	
*	\brief Adafruit_TCS34725 Specialization Class
*	This is the template specialization
*	for the Adafruit RGB Sensor
*/

template<>
class ExternalSensor<Adafruit_TCS34725> :public BaseExternalSensor
{
public:
	/**
	*	ExternalSensor():
	*	Adafruit_TCS34725 specific constructor
	*/
	ExternalSensor()
	{
		
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <Adafruit_TCS34725> constructor" );
		Serial.println();
	
	#endif
		/* Initialise with default values (int time = 2.4ms, gain = 1x) */
		sensor=Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS,TCS34725_GAIN_1X);

	}
	
	/**
	*	begin():
	*	Adafruit_TCS34725 specific begin method
	*
	*	\return true if successful
	*/
	virtual uint8_t begin()
	{
	
	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <Adafruit_TCS34725> begin()" );
		Serial.println();
	
	#endif

		if (sensor.begin()) 
		{

			Serial.println("Found sensor");

			return(true);
		} 
		else 
		{
			Serial.println("No TCS34725 found ... check your connections");

			return(false);
		}	

	}

	/**
	*	read(uint16_t *r,uint16_t *g,uint16_t *b,uint16_t *c,uint16_t *colorTemp,uint16_t *lux):
	*	Adafruit_TCS34725 specific read method
	*
	*	modifies the R,G,B,C,lux,ColorTemp input variables
	*/
	virtual float read(uint16_t *r,uint16_t *g,uint16_t *b,uint16_t *c,uint16_t *colorTemp,uint16_t *lux)
	{
		uint16_t internR,internG,internB,internC,internColorTemp,internLux;

		sensor.getRawData(&internR, &internG, &internB, &internC);
		
		internColorTemp=sensor.calculateColorTemperature(internR,internG,internB);
	
		internLux =sensor.calculateLux(internR,internG,internB);


	#if DEBUGExternal == 1 

		Serial.println( "ExternalSensor <Adafruit_TCS34725> read()" );
		Serial.println();

		Serial.print("Color Temp: "); Serial.print(internColorTemp, DEC); Serial.print(" K - ");
		Serial.print("Lux: "); Serial.print(internLux, DEC); Serial.print(" - ");
		Serial.print("R: "); Serial.print(internR, DEC); Serial.print(" ");
		Serial.print("G: "); Serial.print(internG, DEC); Serial.print(" ");
		Serial.print("B: "); Serial.print(internB, DEC); Serial.print(" ");
		Serial.print("C: "); Serial.print(internC, DEC); Serial.print(" ");
		Serial.println(" ");
	
	#endif
		*r=internR;
		*g=internG;
		*b=internB;
		*c=internC;
		*colorTemp=internColorTemp;
		*lux=internLux;

		return( 0.0 );
	}

private:


	Adafruit_TCS34725 sensor;

};



#endif
