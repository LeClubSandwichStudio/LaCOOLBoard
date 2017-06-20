#include "FS.h"

#include"Arduino.h"  

#include"Wire.h"

#include"OneWire.h"

#include"ExternalSensors.h"

#include "ArduinoJson.h"

void ExternalSensors::begin() // inherited
{
	for(int i=0;i< this->sensorsNumber ; i++)
	{
	
	}
}

String ExternalSensors::read()
{
return("  ");
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

