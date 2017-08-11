/**
*	\file CoolMQTT.h
*	\brief CoolMQTT Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/


#ifndef CoolMQTT_H
#define CoolMQTT_H

#include"Arduino.h"  
#include <ESP8266WiFi.h>
#include "CoolPubSubClient.h"

/**
*	\class CoolMQTT
*	\brief This class handles the mqtt client
*/
class CoolMQTT
{

public:

	void begin();

	int connect(unsigned long keepAlive); //keepAlive in seconds

	bool publish(const char* data);

	bool publish(const char* data,unsigned long logInterval);

	String read();

	void config(const char mqttServer[],const char inTopic[],const char outTopic[],const char user[],int bufferSize);

	bool config();

	void callback(char* topic, byte* payload, unsigned int length);

	void printConf();

	int state();

	bool mqttLoop();

	String getUser();

private:
	
	char mqttServer[50]={'0'};

	String msg="";

	char inTopic[50]={'0'};

	char outTopic[50]={'0'};

	char user[50]={'0'};

	int bufferSize=3000;	

	WiFiClient espClient;

	CoolPubSubClient client;

	bool newMsg=0;

	unsigned long previousLogTime=0;


};

#endif
