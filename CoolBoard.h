/**
*	\file	CoolBoard.h
*  	\brief	CoolBoard Header file
*	\author	Mehdi Zemzem
*	\version 1.0  
*  	\date	27/06/2017
*  
*  
*/

#ifndef CoolBoard_H
#define CoolBoard_H


#include"CoolFileSystem.h" 	//CoolBoard File System Manager
#include"CoolBoardSensors.h"	//CoolBoard Sensor Board Manager
#include"CoolBoardLed.h"	//CoolBoard Led Manager
#include"CoolTime.h"		//CoolBoard Real Time Clock Manager
#include"CoolMQTT.h"		//CoolBoard MQTT Manager
#include"Jetpack.h"		//CoolBoard Jetpack Manager
#include"Irene3000.h"		//CoolBoard Irene3000 Manager
#include"ExternalSensors.h"	//CoolBoard External Sensors Manager

#include <ESP8266WiFi.h>	//ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>		//Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>	//Local WebServer used to serve the configuration portal
#include <WiFiManager.h>	//https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include"Arduino.h"		//Arduino Defs

/**
*	\class	CoolBoard
*	
*	\brief This class manages the CoolBoard and all of
*	Its functions
*/
class CoolBoard
{

public:

	void begin(); 
	
	bool config();

	void update(const char*answer );

	void offLineMode();

	void onLineMode();

	int connect();

	uint16_t getLogInterval();
	


	void printConf();

	void sleep(int interval);

	String readSensors();
	
	String userData();


private:

	CoolFileSystem fileSystem; 

	CoolBoardSensors coolBoardSensors;

	CoolBoardLed coolBoardLed;

	CoolTime rtc;

	WiFiManager wifiManager;

	CoolMQTT mqtt;

	Jetpack jetPack;

	Irene3000 irene3000;

	ExternalSensors externalSensors;

	bool userActive=0;

	bool ireneActive=0;

	bool jetpackActive=0;

	bool externalSensorsActive=0;		

	bool sleepActive=0;	

	uint16_t logInterval=1000;//ms

	int answerJsonSize=3000;
	
	int sensorJsonSize=1000;

	int serverTimeOut=180;

	String data="";

	String answer="";


};

#endif
