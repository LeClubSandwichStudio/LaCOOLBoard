/*
*  CoolBoard.cpp
*  
*  This class is a wrapper.
*  It starts and calls everything
*  
*  
*  
*  
*  
*/


#include "FS.h"

#include "Arduino.h"
#include "CoolBoard.h"
#include "ArduinoJson.h"


uint16_t CoolBoard::getDelay()
{
	return(this->delay);
}

void CoolBoard::begin()
{       
	fileSystem.begin(); 
	
	coolBoardSensors.config(); 
	coolBoardSensors.begin();
	//coolBoardSensors.printConf();
	
	rtc.config();
	rtc.begin();
	//rtc.printConf();
	
	coolBoardLed.config();	
	coolBoardLed.begin();
	//coolBoardLed.printConf();

	mqtt.config();
	mqtt.begin();
	//mqtt.printConf();

	if(jetpackActive)	
	{	
		jetPack.config();
		jetPack.begin();
		//jetPack.printConf();
	}

	if(ireneActive)
	{
		irene3000.config();
		irene3000.begin();
		//irene3000.printConf();
	}

	if(externalSensorsActive)
	{
		externalSensors.config();
		externalSensors.begin();	
		externalSensors.printConf();
	}

}

bool CoolBoard::connect()
{	if(WiFi.status() != WL_CONNECTED)
	{
		//Serial.println("Entering WiFi Connect");
		wifiManager.setConfigPortalTimeout(this->serverTimeOut);
		wifiManager.autoConnect("CoolBoard");
		//Serial.println("Wifi Set" );
	}
	if(mqtt.state()!=0)
	{	
		//Serial.println("setting mqtt connection");
		mqtt.connect(this->getDelay()) ;
		//Serial.println(mqtt.state());
	}
	
	return(mqtt.state()); 
		
}

void CoolBoard::onLineMode()
{	
	
	rtc.update();
	

	this->update(answer.c_str());				

	data=coolBoardSensors.read(); 
		
	if(externalSensorsActive)
	{
		data+=externalSensors.read();	
	}		
	if(ireneActive)
	{
		data+=irene3000.read(); 
	}
	
	if(jetpackActive)
	{
		jetPack.doAction(data.c_str(), sensorJsonSize); 

	}
	
	//Serial.println("On Line Data : ");
	//Serial.println(data);
	 
	//Serial.println("success ??");	
	//Serial.println(mqtt.publish(data.c_str()));
	mqtt.publish(data.c_str());
	mqtt.mqttLoop();
	answer=mqtt.read();	
	//Serial.println("answer is ");
	//Serial.println(answer);
			
}

void CoolBoard::offLineMode()
{
	//offLineMode
	data=coolBoardSensors.read();
	
	if(externalSensorsActive)
	{
		data+=externalSensors.read();	
	}		
	if(ireneActive)
	{
		data+=irene3000.read(); 
	}
	
	if(jetpackActive)
	{
		jetPack.doAction(data.c_str(),sensorJsonSize); 
	}	
	//Serial.print("data :" );		
        //Serial.println(data);
	fileSystem.saveSensorData(data.c_str(), sensorJsonSize ); 
}

bool CoolBoard::config()
{
	fileSystem.begin(); 
	//read config file
	//update data
	File configFile = SPIFFS.open("/coolBoardConfig.json", "r");

	if (!configFile) 
	{
		return(false);
	}
	else
	{
		size_t size = configFile.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		configFile.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
			  return(false);
		} 
		else
		{  	  
			if( json["delay"].success() )
			{
				this->delay = json["delay"]; 
			}

			if(json["sensorJsonSize"].success())
			{
				this->sensorJsonSize = json["sensorJsonSize"];
			}

			if(json["answerJsonSize"].success())
			{
				this->answerJsonSize = json["answerJsonSize"];
			}
			
			if(json["ireneActive"].success() )
			{
				this->ireneActive=json["ireneActive"];
			}	

			if(json["jetpackActive"].success() )
			{		
				this->jetpackActive=json["jetpackActive"];
			}
			
			if(json["externalSensorsActive"].success() )
			{			
			
				this->externalSensorsActive=json["externalSensorsActive"];
			}
			
			if(json["serverTimeOut"].success() )
			{			
				this->serverTimeOut=json["serverTimeOut"];
			}
				
			return(true); 
		}
	}	
	

}

void CoolBoard::printConf()
{
Serial.println("Cool Board Conf");
Serial.println(delay);
Serial.println(sensorJsonSize);
Serial.println(answerJsonSize);
Serial.println(ireneActive);
Serial.println(jetpackActive);
Serial.println(externalSensorsActive);
Serial.println(serverTimeOut);
Serial.println(" ");
}

void CoolBoard::update(const char*answer )
{	
	DynamicJsonBuffer  jsonBuffer(answerJsonSize) ;
	JsonObject& root = jsonBuffer.parseObject(answer);
	
	if(root["update"]==1)
		{
			fileSystem.updateConfigFiles(answer,answerJsonSize); 
	
			this->config();	
		
			coolBoardSensors.config();

			rtc.config(); 

			coolBoardLed.config();
			
			mqtt.config();			
						
			if(jetpackActive)
			{
				jetPack.config(); 
			}

			if(ireneActive)
			{
				irene3000.config();	
			}
			
			if(externalSensorsActive)
			{
				externalSensors.config();			
			}

			
			
			

		}

}

