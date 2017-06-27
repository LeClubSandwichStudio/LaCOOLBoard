/**
*	\file	CoolBoard.cpp
*  	\breif	CoolBoard Source file
*	\author	Mehdi Zemzem
*	\version 1.0  
*  	\date	27/06/2017
*  
*  
*/


#include "FS.h"
#include "Arduino.h"
#include "CoolBoard.h"
#include "ArduinoJson.h"

/**
*	CoolBoard::begin():
*	This method is provided to configure, 
*	begin the used CoolKit Parts.
*	If Serial is enabled,it prints the configuration
*	of the used parts.
*/
void CoolBoard::begin()
{       
	fileSystem.begin(); 
	
	coolBoardSensors.config(); 
	coolBoardSensors.begin();
	coolBoardSensors.printConf();
	
	rtc.config();
	rtc.begin();
	rtc.printConf();
	
	coolBoardLed.config();	
	coolBoardLed.begin();
	coolBoardLed.printConf();

	mqtt.config();
	mqtt.begin();
	mqtt.printConf();

	if(jetpackActive)	
	{	
		jetPack.config();
		jetPack.begin();
		jetPack.printConf();

	}

	if(ireneActive)
	{
		irene3000.config();
		irene3000.begin();
		irene3000.printConf();

	}

	if(externalSensorsActive)
	{
		externalSensors.config();
		externalSensors.begin();	
		externalSensors.printConf();
	}

}

/**
*	CoolBoard::connect():
*	This method is provided to manage the network
*	connection and the mqtt connection.
*	
*	 \return mqtt client state		
*/
int CoolBoard::connect()
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

/**
*	CoolBoard::onLineMode():
*	This method is provided to manage the online
*	mode:	-update clock
*		-read sensors
*		-do actions
*		-publish data
*		-read answer
*		-update config
*/
void CoolBoard::onLineMode()
{	
	
	
	

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
	this->update(answer.c_str());	
				
}

/**
*	CoolBoard::offlineMode():
*	This method is provided to manage the offLine
*	mode:	-read sensors
*		-do actions
*		-save data in the file system
*/
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


/**
*	CoolBoard::config():
*	This method is provided to configure
*	the CoolBoard :	-log interval
*			-Size of the data to write
*			-Size of the data to read
*			-irene3000 activated/deactivated
*			-jetpack activated/deactivated
*			-external Sensors activated/deactivated
*			-mqtt server timeout
*
*	\return true if configuration is done, 
*	false otherwise
*/
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
			else
			{
				this->delay=this->delay;
				
			}
			json["delay"]=this->delay;

			if(json["sensorJsonSize"].success())
			{
				this->sensorJsonSize = json["sensorJsonSize"];
			}
			else
			{
				this->sensorJsonSize=this->sensorJsonSize;
			}
			json["sensorJsonSize"]=this->sensorJsonSize;

			if(json["answerJsonSize"].success())
			{
				this->answerJsonSize = json["answerJsonSize"];
			}
			else
			{
				this->answerJsonSize=this->answerJsonSize;
			}
			json["answerJsonSize"]=this->answerJsonSize;
			
			if(json["ireneActive"].success() )
			{
				this->ireneActive=json["ireneActive"];
			}
			else
			{
				this->ireneActive=this->ireneActive;
			}
			json["ireneActive"]=this->ireneActive;	

			if(json["jetpackActive"].success() )
			{		
				this->jetpackActive=json["jetpackActive"];
			}
			else
			{
				this->jetpackActive=this->jetpackActive;
			}
			json["jetpackActive"]=this->jetpackActive;
			
			if(json["externalSensorsActive"].success() )
			{			
			
				this->externalSensorsActive=json["externalSensorsActive"];
			}
			else
			{
				this->externalSensorsActive=this->externalSensorsActive;
			}
			json["externalSensorsActive"]=this->externalSensorsActive;
			
			if(json["serverTimeOut"].success() )
			{			
				this->serverTimeOut=json["serverTimeOut"];
			}
			else
			{
				this->serverTimeOut=this->serverTimeOut;
			}
			json["serverTimeOut"]=this->serverTimeOut;
			
			configFile.close();
			configFile = SPIFFS.open("/coolBoardConfig.json", "w");
		
			if(!configFile)
			{
				return(false);
			}

			json.printTo(configFile);
			configFile.close();
	
			return(true); 
		}
	}	
	

}

/**
*
*	CoolBoard::printConf():
*	This method is provided to print
*	the configuration to the Serial
*	Monitor.
*/
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

/**
*	CoolBoard::update(mqtt answer):
*	This method is provided to handle the
*	configuration update of the different parts 
*/
void CoolBoard::update(const char*answer )
{	
	DynamicJsonBuffer  jsonBuffer(answerJsonSize) ;
	JsonObject& root = jsonBuffer.parseObject(answer);
	JsonObject& stateDesired = root["state"]["desired"];
	if(stateDesired.success() )
	{
		if(stateDesired["update"]==1) 
			{	
				String answerDesired;
				stateDesired.printTo(answerDesired);
				Serial.println(answerDesired);
				
				bool result=fileSystem.updateConfigFiles(answerDesired,answerJsonSize); 
				Serial.print("update : ");Serial.println(result);
				
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
}

/**
*	CoolBoard::getDelay():
*	This method is provided to get
*	the log interval
*	\return interval value in ms
*/
uint16_t CoolBoard::getDelay()
{
	return(this->delay);
}


