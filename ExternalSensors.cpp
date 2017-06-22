#include "FS.h"

#include"Arduino.h"  

#include"Wire.h"

#include"OneWire.h"

#include"ExternalSensors.h"

#include "ArduinoJson.h"

#include"ExternalSensor.h"
void ExternalSensors::begin() 
{
	for(int i=0;i< this->sensorsNumber ; i++)
	{
		if( (sensors[i].reference) == "NDIR_I2C" )
		{	

			ExternalSensor<NDIR_I2C> ndirCO2(77);

			sensors[i].exSensor=&ndirCO2;
			sensors[i].exSensor->begin();

		}
		if( (sensors[i].reference) == "DallasTemperature")
		{
			//OneWire oneWire(0);
			//ExternalSensor<DallasTemperature> dallasTemp;
			//sensors[i].exSensor=&dallasTemp;
			//sensors[i].exSensor->begin();
			
		}
		
		
	}
}

String ExternalSensors::read()
{

	String data;
	DynamicJsonBuffer  jsonBuffer(jsonSizeVar) ;
	JsonObject& root = jsonBuffer.createObject();
	if(!root.success() )
	{

	 return("00 ");
	}
	
	Serial.println("enter the for");	
	for(int i=0;i<sensorsNumber;i++)
	{
		if( (sensors[i].reference) == "NDIR_I2C" )
		{	

			ExternalSensor<NDIR_I2C> ndirCO2(77);

			sensors[i].exSensor=&ndirCO2;
			sensors[i].exSensor->begin();
			Serial.println("reading " );	 
			Serial.println(sensors[i].exSensor->read() );
			Serial.println("assigning " );	   	
			root[sensors[i].type]=sensors[i].exSensor->read();


		}

		
	
	
		
	 	
	}
	
	root.printTo(Serial);
	Serial.println(" ");
	root.printTo(data);
	
	return(data);

}

int ExternalSensors::jsonSize()
{
	return(this->jsonSizeVar );
}

bool ExternalSensors::config()
{
	//read config file
	//update data
	File externalSensorsConfig = SPIFFS.open("/externalSensorsConfig.json", "r");

	if (!externalSensorsConfig) 
	{
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
			  return(false);
		} 
		else
		{  	
			if(json["jsonSize"]!=NULL )
			{			
				this->jsonSizeVar=json["jsonSize"];
			}			

			
			if(json["sensorsNumber"]!=NULL)
			{
				this->sensorsNumber = json["sensorsNumber"];
				
				

				for(int i=0;i<sensorsNumber;i++)
				{	String name="sensor"+String(i);

					if(json.containsKey(name ) )
					{  
						JsonObject& sensorJson=json[name];

						if(sensorJson["reference"].success() )
						{  
							this->sensors[i].reference =sensorJson["reference"].as<String>();
							
													
						}
						else
						{
							Serial.println("Not Found Name " );
								
						}
					
						if(sensorJson["type"].success() )
						{					
							this->sensors[i].type=sensorJson["type"].as<String>();
						}
						else
						{
								Serial.println("Not Found Name " ) ;						
						}
					
					
						if(sensorJson["connection"].success() )
						{
							this->sensors[i].connection=sensorJson["connection"].as<String>();
					
						}
						else
						{
								Serial.println("Not Found Name " ) ;						
						}
					
						if(sensorJson["dataSize"]!=NULL )
						{					

							this->sensors[i].dataSize=sensorJson["dataSize"];

						}
						else
						{
								Serial.println("Not Found Name " ) ;						
						}
					
						if(sensorJson["address"]!=NULL )
						{					

							this->sensors[i].address=sensorJson["address"];

						}
						else
						{
								Serial.println("Not Found Name " ) ;						
						}
					
	
					}			        	
										

				}
 
			}
			
			
			return(true); 
		}
	}	
	



}
void ExternalSensors::printConf()
{
	Serial.println("External Sensors config ");
	Serial.println(sensorsNumber);
	Serial.println(jsonSizeVar);
	for(int i=0;i<sensorsNumber;i++)
	{
		Serial.println(this->sensors[i].reference);
		Serial.println(this->sensors[i].type);
		Serial.println(this->sensors[i].connection);
		Serial.println(this->sensors[i].dataSize);
		Serial.println(this->sensors[i].address);
	
	}
}

