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
#include <PubSubClient.h>

/**
*	\class CoolMQTT
*	\brief This class handles the mqtt client
*/
class CoolMQTT
{

public:

	void begin();
	int connect(uint16_t keepAlive);

	bool publish(const char* data);

	bool publish(const char* data,int logInterval);

	String read();

	void config(const char mqttServer[],const char inTopic[],const char outTopic[],const char user[],int bufferSize);
	bool config();

	void callback(char* topic, byte* payload, unsigned int length);

	void printConf();

	int state();

	bool mqttLoop();

	String getUser();

private:
	
	char mqttServer[50];
	String msg;
	char inTopic[50];
	char outTopic[50];
	char user[50];
	int bufferSize;	
	WiFiClient espClient;
	PubSubClient client;
	bool newMsg;
	unsigned long previousLogTime=0;


};

#endif
