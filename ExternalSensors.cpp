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

/**
*	ExternalSensors::begin():
*	This method is provided to initialise
*	the external sensors. 
*/
void ExternalSensors::begin() 
{
	Serial.println("Enter ExternalSensors.begin()");
	Serial.println();

	for(int i=0;i< this->sensorsNumber ; i++)
	{
		if( (sensors[i].reference) == "NDIR_I2C" )
		{	
			std::unique_ptr< ExternalSensor<NDIR_I2C> > sensorCO2(new ExternalSensor<NDIR_I2C> (77));


			sensors[i].exSensor= sensorCO2.release();                       // using std::move;
			sensors[i].exSensor->begin();

		}
		if( (sensors[i].reference) == "DallasTemperature")
		{
			OneWire oneWire(0);
			std::unique_ptr< ExternalSensor<DallasTemperature> > dallasTemp(new ExternalSensor<DallasTemperature> ());
			 ;
			sensors[i].exSensor=dallasTemp.release();
			sensors[i].exSensor->begin();
			
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

	Serial.println("Entering ExternalSensors.read()");
	Serial.println();

	String data;
	DynamicJsonBuffer  jsonBuffer(jsonSize) ;
	JsonObject& root = jsonBuffer.createObject();

	if(!root.success() )
	{
		Serial.println("failed to create json ");
		return("00");
	}
	else
	{
		if(sensorsNumber>0)
		{
			for(int i=0;i<sensorsNumber;i++)
			{
			
				root[sensors[i].type]=sensors[i].exSensor->read();	 	
			}
		}	
		
		root.printTo(data);

		Serial.println("sensors data :");
		Serial.println(data);
		Serial.println();
	
		return(data);
	}

}

/**
*	ExternalSensors::getJsonSize():
*	This method is provided to return
*	the size of the json data as a way
*	to control memory usage
*
*	\return the json data size
*/
int ExternalSensors::getJsonSize()
{
	Serial.println("Enter ExternalSensors.getJsonSize");
	Serial.println();
	
	Serial.print("jsonSize : ");
	Serial.println(this->jsonSize);
	Serial.println();

	return(this->jsonSize );
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
		Serial.println("failed to read /externalSensorsConfig.json");
		Serial.println();
		
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
			Serial.println("failed to parse json");
			Serial.println();

			return(false);
		} 
		else
		{  	
			Serial.println("read configuration json : ");
			json.printTo(Serial);
			Serial.println();

			if(json["jsonSize"]!=NULL )
			{			
				this->jsonSize=json["jsonSize"];
			}
			else
			{
				this->jsonSize=this->jsonSize;
			}
			json["jsonSize"]=this->jsonSize;			

			
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
							Serial.println("Not Found Name " );		
						}
						sensorJson["reference"]=this->sensors[i].reference;

					
						if(sensorJson["type"].success() )
						{					
							this->sensors[i].type=sensorJson["type"].as<String>();
						}
						else
						{
							this->sensors[i].type=this->sensors[i].type;
							Serial.println("Not Found Name " ) ;						
						}
						sensorJson["type"]=this->sensors[i].type;
					
					
						if(sensorJson["connection"].success() )
						{
							this->sensors[i].connection=sensorJson["connection"].as<String>();
						}
						else
						{
							this->sensors[i].connection=this->sensors[i].connection;
							Serial.println("Not Found Name " ) ;						
						}
						sensorJson["connection"]=this->sensors[i].connection;

					
						if(sensorJson["dataSize"].success() )
						{				
							this->sensors[i].dataSize=sensorJson["dataSize"];
						}
						else
						{
							this->sensors[i].dataSize=this->sensors[i].dataSize;
							Serial.println("Not Found Name " ) ;						
						}
						sensorJson["dataSize"]=this->sensors[i].dataSize;

					
						if(sensorJson["address"].success() )
						{					
							this->sensors[i].address=sensorJson["address"];
						}
						else
						{	
							this->sensors[i].address=this->sensors[i].address;
							Serial.println("Not Found Name " ) ;						
						}
						sensorJson["address"]=this->sensors[i].address;
					
	
					}
					else
					{
						this->sensors[i]=this->sensors[i];					
					}
								        	
					json[name]["reference"]=this->sensors[i].reference;
					json[name]["type"]=this->sensors[i].type;
					json[name]["connection"]=this->sensors[i].connection;
					json[name]["dataSize"]=this->sensors[i].dataSize;
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
				Serial.println("failed to write to /externalSensorsConfig.json");
				Serial.println();

				return(false);
			}
			
			json.printTo(externalSensorsConfig);
			externalSensorsConfig.close();
			
			Serial.println("saved configuration is : ");
			json.printTo(Serial);
			Serial.println();

			return(true); 
		}
	}	
	



}

/**
*	ExternalSensors::printConf():
*	This method is provided to print the
*	configuration to the Serial Monitor
*/
void ExternalSensors::printConf()
{
	Serial.println("Entering ExternalSensors.printConf()");
	Serial.println();

	Serial.println("External Sensors configuration ");

	Serial.print("sensorsNumber : ");
	Serial.println(sensorsNumber);

	Serial.println("jsonSize : ");
	Serial.println(jsonSize);

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
		Serial.print(" connection : ");
		Serial.println(this->sensors[i].connection);
		
		Serial.print("sensor ");
		Serial.print(i);
		Serial.print(" dataSize : ");
		Serial.println(this->sensors[i].dataSize);
		
		Serial.print("sensor ");
		Serial.print(i);
		Serial.print(" address : ");
		Serial.println(this->sensors[i].address);
	
	}
}

