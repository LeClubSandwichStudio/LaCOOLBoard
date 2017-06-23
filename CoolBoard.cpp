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


void CoolBoard::begin()
{       
	fileSystem.begin(); 
	
	coolBoardSensors.config(); 
	coolBoardSensors.begin();

	
	rtc.config();
	rtc.begin();

	
	coolBoardLed.config();	
	coolBoardLed.begin();


	mqtt.config();
	mqtt.begin();


	if(jetpackActive)	
	{	
		jetPack.config();
		jetPack.begin();

	}

	if(ireneActive)
	{
		irene3000.config();
		irene3000.begin();

	}

	if(externalSensorsActive)
	{
		externalSensors.config();
		externalSensors.begin();	

	}

}

bool CoolBoard::connect()
{	if(WiFi.status() != WL_CONNECTED)
	{

		wifiManager.setConfigPortalTimeout(this->serverTimeOut);
		wifiManager.autoConnect("CoolBoard");

	}
	if(mqtt.state()!=0)
	{	

		mqtt.connect(this->getDelay()) ;

	}
	
	return(mqtt.state()); 
		
}

void CoolBoard::onLineMode()
{	
	
	
	

	this->update(answer.c_str());				
	
	rtc.update();	

	data=coolBoardSensors.read(); //{..,..,..}


	if(externalSensorsActive)
	{	

		data+=externalSensors.read();//{..,..,..}{..,..}	
		data.setCharAt(data.lastIndexOf('}'),',');//{..,..,..}{..,..,
		data.setCharAt(data.lastIndexOf('{'),',');//{..,..,..},..,..,
		data.remove(data.lastIndexOf('}'),1);//{..,..,..,..,..,
		data.setCharAt(data.lastIndexOf(','),'}');//{..,..,..,..,..}		
	}		
	if(ireneActive)
	{
		data+=irene3000.read();//{..,..,..,..,..}{..,..,..}
		data.setCharAt(data.lastIndexOf('}'),',');//{..,..,..,..,..{..,..,.., 
		data.setCharAt(data.lastIndexOf('{'),',');//{..,..,..,..,..},..,..,..,
		data.remove(data.lastIndexOf('}'),1);//{..,..,..,..,..,..,..,..,	
		data.setCharAt(data.lastIndexOf(','),'}');//{..,..,..,..,..,..,..,..}
	}

	
	if(jetpackActive)
	{
		jetPack.doAction(data.c_str(), sensorJsonSize);
		

	}
	
	String jsonData="{\"state\":{\"reported\":";	
	jsonData+=data;//{"state":{"reported":{..,..,..,..,..,..,..,..}
	jsonData+=",\"desired\":null} }";//{"state":{"reported":{..,..,..,..,..,..,..,..},"desired" :null }  }
	
	mqtt.publish(jsonData.c_str());
	mqtt.mqttLoop();
	answer=mqtt.read();	
				
}

void CoolBoard::offLineMode()
{

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
	JsonObject& stateDesired = root["state"]["desired"];
	//or ( root["state"]["desired"].success() )	
	if(stateDesired["update"]==1) 
		{	
			String answerDesired;
			stateDesired.printTo(answerDesired);
			Serial.println("begin config " );
			Serial.println("printing the config files ");
			fileSystem.updateConfigFiles(answerDesired.c_str(),answerJsonSize); 
	
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

uint16_t CoolBoard::getDelay()
{
	return(this->delay);
}


