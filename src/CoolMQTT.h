/**
*	\file CoolMQTT.h
*	\brief CoolMQTT Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*	\copyright La Cool Co SAS 
*	\copyright MIT license
*	Copyright (c) 2017 La Cool Co SAS
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*
*/


#ifndef CoolMQTT_H
#define CoolMQTT_H

#include"Arduino.h"  
#include <ESP8266WiFi.h>
#include "internals/CoolPubSubClient.h"

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
