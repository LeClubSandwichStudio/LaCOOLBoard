/**
*	\file ExternalSensors.cpp
*	\brief ExternalSensors Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/


#include "FS.h"

#include"Arduino.h"  

#include"Wire.h"

#include"OneWire.h"

#include"ExternalSensors.h"

#include "ArduinoJson.h"

#include"ExternalSensor.h"

#define DEBUG 0

OneWire oneWire(0);


/**
*	ExternalSensors::begin():
*	This method is provided to initialise
*	the external sensors. 
*/
void ExternalSensors::begin() 
{

#if DEBUG == 1

	Serial.println( F("Enter ExternalSensors.begin()") );
	Serial.println();

#endif 

	for(int i=0;i< this->sensorsNumber ; i++)
	{
		if( (sensors[i].reference) == "NDIR_I2C" )
		{	
			std::unique_ptr< ExternalSensor<NDIR_I2C> > sensorCO2(new ExternalSensor<NDIR_I2C>( this->sensors[i].address) );


			sensors[i].exSensor= sensorCO2.release();
			sensors[i].exSensor->begin();
			sensors[i].exSensor->read();

		}
		if( (sensors[i].reference) == "DallasTemperature")
		{

			std::unique_ptr< ExternalSensor<DallasTemperature> > dallasTemp(new ExternalSensor<DallasTemperature> (&oneWire));
			 
			sensors[i].exSensor=dallasTemp.release();
			sensors[i].exSensor->begin();
			sensors[i].exSensor->read();
		}
		
		
	}
}

/**
*	ExternalSensors::read():	
*	This method is provided to
*	read the data from the external sensors
*
*	\return json string that contains the
*	sensors data
*/
String ExternalSensors::read()
{

#if DEBUG == 1

	Serial.println( F("Entering ExternalSensors.read()") );
	Serial.println();

#endif 

	String data;
	DynamicJsonBuffer  jsonBuffer ;
	JsonObject& root = jsonBuffer.createObject();

	if(!root.success() )
	{
 
	#if DEBUG == 1

		Serial.println( F("failed to create json ") );
	
	#endif 

		return("00");
	}
	else
	{
		if(sensorsNumber>0)
		{
			for(int i=0;i<sensorsNumber;i++)
			{
				if(sensors[i].exSensor != NULL )
				{
					root[sensors[i].type]=sensors[i].exSensor->read();	 	
				}
			
			#if DEBUG == 1
				else
				{
					Serial.println(F("null pointer "));
				}
			#endif	
			}
		}	
		
		root.printTo(data);
	
	#if DEBUG == 1

		Serial.println( F("sensors data :") );
		Serial.println(data);
		Serial.println();

		Serial.print(F("jsonBuffer size: "));
		Serial.println(jsonBuffer.size());
		Serial.println();

	
	#endif
	
		return(data);
	}

}

/**
*	ExternalSensors::config():
*	This method is provided to configure
*	the externalSensors through a configuration
*	file
*
*	\return true if successful,false otherwise
*/
bool ExternalSensors::config()
{
	//read config file
	//update data
	File externalSensorsConfig = SPIFFS.open("/externalSensorsConfig.json", "r");

	if (!externalSensorsConfig) 
	{
	
	#if DEBUG == 1
		
		Serial.println( F("failed to read /externalSensorsConfig.json") );
		Serial.println();
	
	#endif
		
		return(false);
	}
	else
	{
		size_t size = externalSensorsConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		externalSensorsConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());

		if (!json.success()) 
		{
		
		#if DEBUG == 1 

			Serial.println( F("failed to parse json") );
			Serial.println();
		
		#endif

			return(false);
		} 
		else
		{
		
		#if DEBUG == 1 
	
			Serial.println( F("configuration json is : ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();

		
		#endif			
			if(json["sensorsNumber"]!=NULL)
			{
				this->sensorsNumber = json["sensorsNumber"];
				
				

				for(int i=0;i<sensorsNumber;i++)
				{	String name="sensor"+String(i);
					
					if(json[name].success())
					{  
						JsonObject& sensorJson=json[name];
						
						if(sensorJson["reference"].success() )
						{  
							this->sensors[i].reference =sensorJson["reference"].as<String>();
						}
						else
						{
							this->sensors[i].reference=this->sensors[i].reference;							
									
						}
						sensorJson["reference"]=this->sensors[i].reference;

					
						if(sensorJson["type"].success() )
						{					
							this->sensors[i].type=sensorJson["type"].as<String>();
						}
						else
						{
							this->sensors[i].type=this->sensors[i].type;

						}
						sensorJson["type"]=this->sensors[i].type;
					
						if(sensorJson["address"].success() )
						{					
							this->sensors[i].address=sensorJson["address"];
						}
						else
						{	
							this->sensors[i].address=this->sensors[i].address;

						}
						sensorJson["address"]=this->sensors[i].address;
					
	
					}
					else
					{
						this->sensors[i]=this->sensors[i];					
					}
								        	
					json[name]["reference"]=this->sensors[i].reference;
					json[name]["type"]=this->sensors[i].type;
					json[name]["address"]=this->sensors[i].address;
				}
 
			}
			else
			{
				this->sensorsNumber=this->sensorsNumber;
			}
			json["sensorsNumber"]=this->sensorsNumber;

			externalSensorsConfig.close();
			externalSensorsConfig = SPIFFS.open("/externalSensorsConfig.json", "w");

			if(!externalSensorsConfig)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /externalSensorsConfig.json") );
				Serial.println();
			
			#endif

				return(false);
			}
			
			json.printTo(externalSensorsConfig);
			externalSensorsConfig.close();
			
		#if DEBUG == 1 

			Serial.println( F("saved configuration is : ") );
			json.printTo(Serial);
			Serial.println();
		
		#endif

			return(true); 
		}
	}	
	



}

/**
*	ExternalSensors::config(String reference[],String type[],uint8_t address[],int sensorsNumber):
*	This method is provided to configure
*	the externalSensors without a configuration
*	file
*
*	\return true if successful,false otherwise
*/
bool ExternalSensors::config(String reference[],String type[],uint8_t address[],int sensorsNumber)
{

#if DEBUG == 1

	Serial.println( F("Entering ExternalSensors.conf(reference[], type[], address[], sensorsNumber)") );
	Serial.println();

#endif 	
	if(sensorsNumber>50)
	{
	
	#if DEBUG == 1
	
		Serial.println(F("you can't add more than 50 sensors"));	
	
	#endif	
	
		return(false);
	}

	this->sensorsNumber=sensorsNumber;
	
	for(int i=0;i<sensorsNumber;i++)
	{
	
		this->sensors[i].reference=reference[i];
		
		this->sensors[i].type=type[i];

		this->sensors[i].address=address[i];
	
	}
	
	return(true);

}

/**
*	ExternalSensors::printConf():
*	This method is provided to print the
*	configuration to the Serial Monitor
*/
void ExternalSensors::printConf()
{

#if DEBUG == 1

	Serial.println( F("Entering ExternalSensors.printConf()") );
	Serial.println();

#endif 

	Serial.println("External Sensors configuration ");

	Serial.print("sensorsNumber : ");
	Serial.println(sensorsNumber);

	for(int i=0;i<sensorsNumber;i++)
	{
		Serial.print("sensor ");
		Serial.print(i);
		Serial.print(" reference : ");
		Serial.println(this->sensors[i].reference);

		Serial.print("sensor ");
		Serial.print(i);
		Serial.print(" type : ");
		Serial.println(this->sensors[i].type);
				
		Serial.print("sensor ");
		Serial.print(i);
		Serial.print(" address : ");
		Serial.println(this->sensors[i].address);
	
	}
}

