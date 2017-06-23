/*
*  CoolFileSystem.h
*  
*  This class manages the file system and on board server
*  
*  
*  
*  
*  
*  
*/
#include "FS.h"                // File Storage System >>> http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html
#include"CoolFileSystem.h"
#include "ArduinoJson.h"      // Arduino JSON File controller  https://github.com/bblanchon/ArduinoJson


bool CoolFileSystem::begin()
{
	return( SPIFFS.begin() );                                   //Initialize Filesystem

}

bool CoolFileSystem::saveSensorData(const char* data,int Sensor_JSON_SIZE)
{
	File sensorsData=SPIFFS.open("/sensorsData.json","a");
	if(!sensorsData)
	{
		return false;	
	}	

	DynamicJsonBuffer jsonBuffer(Sensor_JSON_SIZE);
	JsonObject& root = jsonBuffer.parseObject(data);
	
	root.printTo(sensorsData);
	sensorsData.close();
	
	return true;		
}

bool CoolFileSystem::updateConfigFiles(const char* answer,int JSON_SIZE)
{
	//total json object	
	DynamicJsonBuffer jsonBuffer(JSON_SIZE);
	JsonObject& root = jsonBuffer.parseObject(answer);
	
	//temp string
	String temp;

	//CoolBoard Configuration File
	DynamicJsonBuffer jsonCBoard;
    	JsonObject& jsonCoolBoard = jsonCBoard.createObject();

	jsonCoolBoard["CoolBoard"]=root["CoolBoard"];
	if(jsonCoolBoard.success())
	{
		File coolBoardConfig = SPIFFS.open("/coolBoardConfig.json", "w+");	
		if(!coolBoardConfig)
		{
			return(false);
		}
		Serial.println("CoolBoard Config");
		jsonCoolBoard.printTo(Serial);
		
		jsonCoolBoard.printTo(coolBoardConfig);
		
		coolBoardConfig.close();
	}		

	
	//Cool Board Sensors Configuration File
	DynamicJsonBuffer jsonSBoard;
    	JsonObject& jsonSensorsBoard = jsonSBoard.createObject();
	jsonSensorsBoard["CoolSensorsBoard"]=root["CoolSensorsBoard"];	
	
	if(jsonSensorsBoard.success())
	{	
		File coolBoardSensorsConfig = SPIFFS.open("/coolBoardSensorsConfig.json", "w+");	
		if(!coolBoardSensorsConfig)
		{
			return(false);
		}
		
		Serial.println("CoolBoardSensors Config");
		jsonSensorsBoard.printTo(coolBoardSensorsConfig);
		jsonSensorsBoard.printTo(Serial);
		coolBoardSensorsConfig.close();
	}
	
	
	
	//rtc configuration file
	DynamicJsonBuffer jsonR;
    	JsonObject& jsonRTC = jsonR.createObject();
	
	jsonRTC["rtc"]=root["rtc"];
	if(jsonRTC.success() )
	{
		File rtcConfig = SPIFFS.open("/rtcConfig.json", "w+");	
		if(!rtcConfig)
		{
			return(false);
		}
		Serial.println("RTC Config");
		jsonRTC.printTo(rtcConfig);
		jsonRTC.printTo(Serial);
		rtcConfig.close();
	
	}

	
	
	
	
        //cool board led configuration
	DynamicJsonBuffer jsonLBoard;
    	JsonObject& jsonLedBoard = jsonLBoard.createObject();
	jsonLedBoard["Led"]=root["Led"];
	if(jsonLedBoard.success())
	{	
		File coolBoardLedConfig = SPIFFS.open("/coolBoardLedConfig.json", "w+");	
		if(!coolBoardLedConfig)
		{
			return(false);
		}
		Serial.println("CoolBoardLed Config");
		jsonLedBoard.printTo(coolBoardLedConfig);
		jsonLedBoard.printTo(Serial);
		coolBoardLedConfig.close();
	
	}
		

	

	//jetpack configuration
	DynamicJsonBuffer jsonJBoard;
    	JsonObject& jsonJetpack = jsonJBoard.createObject();
	jsonJetpack["jetPack"]=root["jetPack"];
	
	if(jsonJetpack.success())
	{	
		File jetPackConfig = SPIFFS.open("/jetPackConfig.json", "w+");	
		if(!jetPackConfig)
		{
			return(false);
		}
		Serial.println("jetpack Config");	
		jsonJetpack.printTo(jetPackConfig);
		jsonJetpack.printTo(Serial);
		jetPackConfig.close();
	}
	
	//irene configuration	
	DynamicJsonBuffer jsonIBoard;
    	JsonObject& jsonIrene = jsonIBoard.createObject();
	jsonIrene["irene3000"]=root["irene3000"];
	
	if(jsonIrene.success())
	{
		File irene3000Config = SPIFFS.open("/irene3000Config.json", "w+");	
		if(!irene3000Config)
		{
			return(false);
		}
		Serial.println("irene3000 Config");
		jsonIrene.printTo(irene3000Config);
		jsonIrene.printTo(Serial);
		irene3000Config.close();
	
	}
	
	//external sensors
	DynamicJsonBuffer jsonESBoard;
    	JsonObject& jsonExternalSensors = jsonESBoard.createObject();
	
			
	jsonExternalSensors["externalSensors"]=root["externalSensors"];
	
	if(jsonExternalSensors.success())
	{
		File externalSensorsConfig = SPIFFS.open("/externalSensorsConfig.json", "w+");	
		if(!externalSensorsConfig)
		{
			return(false);
		}
		Serial.println("externalSensors Config");
		jsonExternalSensors.printTo(externalSensorsConfig);
		jsonExternalSensors.printTo(Serial);
		for(int i=0;i<root["externalSensors"]["sensorsNumber"];i++)
		{	
			String path="/"+String(i)+".json"; 
         		File temp=SPIFFS.open(path,"w+");
			Serial.print(" external sensor ");Serial.print(i);Serial.println(" Config");
			jsonExternalSensors[String(i)].printTo(temp);
			jsonExternalSensors[String(i)].printTo(Serial);
			temp.close();
		}
			
		externalSensorsConfig.close();

	}

	
	//mqtt config
	DynamicJsonBuffer jsonMQ;
    	JsonObject& jsonMQTT = jsonMQ.createObject();
		
	jsonMQTT["mqtt"]=root["mqtt"];
	if(jsonMQTT.success())
	{
		File mqttConfig = SPIFFS.open("/mqttConfig.json", "w+");	
		if(!mqttConfig)
		{
			return(false);
		}
		Serial.println("mqtt config");
		jsonMQTT.printTo(mqttConfig);
		jsonMQTT.printTo(Serial);
		mqttConfig.close();
	}	
		
	return true;

}	



