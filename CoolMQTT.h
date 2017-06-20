#ifndef CoolMQTT_H
#define CoolMQTT_H


#include"Arduino.h"  
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


class CoolMQTT
{
public:

void begin();
int connect(uint16_t keepAlive);

bool publish(const char* data);

String read();

bool config();

void callback(char* topic, byte* payload, unsigned int length);

void printConf();

bool state();

bool mqttLoop();

private:
	
	char mqtt_server[50];
	String msg;
	char inTopic[50];
	char outTopic[50];
	char clientId[50];
	int bufferSize;	
	WiFiClient espClient;
	PubSubClient client;


		

};

#endif
