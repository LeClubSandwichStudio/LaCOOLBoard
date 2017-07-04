/**
*	\file CoolMQTT.cpp
*	\brief CoolMQTT Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/


#include "FS.h"
#include "Arduino.h"  
#include <ESP8266WiFi.h>
#include <PubSubClient.h>                             
#include "CoolMQTT.h"
#include "ArduinoJson.h"

/**
*	CoolMQTT::begin():
*	This method is provided to set the mqtt
*	client's parameters:	-client
*				-server
*				-callback method
*				-buffer size
*/
void CoolMQTT::begin()
{ 
	client.setClient(espClient);
	client.setServer(mqttServer, 1883);	
	client.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
	client.setBufferSize((unsigned short)bufferSize);

}

/**
*	CoolMQTT::state():
*	This method is provided to return the 
*	mqtt client's state.
*	\return mqtt client state:	
*		-4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
*		-3 : MQTT_CONNECTION_LOST - the network connection was broken
*		-2 : MQTT_CONNECT_FAILED - the network connection failed
*		-1 : MQTT_DISCONNECTED - the client is disconnected cleanly
*		0 : MQTT_CONNECTED - the cient is connected
*		1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
*		2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
*		3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
*		4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
*		5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
*/
int CoolMQTT::state()
{
	return(client.state());
}

/**
*	CoolMQTT::connect( time to keep the connection alive ):
*	This method is provided to connect the client to the server,
*	publish to the out topic , subscribe to the in topic and set
*	the keepAlive time.
*	
*	\return mqtt client state
*/
int CoolMQTT::connect(uint16_t keepAlive)
{       
	int i=0;
	Serial.println("MQTT connecting...");
	while ((!client.connected())&&(i<100)) 
	{
		// Attempt to connect
		if (client.connect(user,keepAlive)) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			//client.publish(outTopic, "hello world by Ash");
			// ... and resubscribe
			client.subscribe(inTopic);
			Serial.println("published and subscribed , leavin ") ;
			return(client.state());
		}
		else
		{
			Serial.println("not connected , leaving");
			return(client.state());
			
		}
	delay(5);
	i++;
	}
	
	return(1);

}

/**
*	CoolMQTT::publish(data):
*	This method is provided to publish data
*	to the out topic
*
*	\return true if publish successful,
*	false otherwise
*/
bool CoolMQTT::publish(const char* data)
{

	//data is in JSON, publish it directly

	Serial.println("data to publish");
	Serial.println(data);
	Serial.print("data size ");Serial.println(strlen(data));
	bool pub=client.publish( outTopic, data,strlen(data) );


	return( pub);

}

/**
*	CoolMQTT::publish(data):
*	This method is provided to publish data
*	to the out topic every logInterval ms
*
*	\return true if publish successful,
*	false otherwise
*/
bool CoolMQTT::publish(const char* data,int logInterval)
{
	if( (millis()-this->previousLogTime) >=( logInterval ) )
	{
		this->publish(data);

		this->previousLogTime=millis();

		return(true);
	}
	
	return(false);
}

/**
*	CoolMQTT::mqttLoop():
*	This method is provided to allow the
*	client to process the data
*
*	\return true if successful,false
*	otherwise
*/	
bool CoolMQTT::mqttLoop()
{
	this->client.loop();
	return(client.loop());
}

/**
*	CoolMQTT::callback(in topic, incoming message , message length):
*	This method is provided to handle incoming messages from the
*	subscribed inTopic.
*	
*	Arguments are automatically assigned in client.setCallback()
*/
void CoolMQTT::callback(char* topic, byte* payload, unsigned int length) 
{
	char temp[length+1];

	for (int i = 0; i < length; i++) 
	{
		temp[i]=(char)payload[i]; 

	}

	this->newMsg=true;

	temp[length+1]='\0';

	msg=String(temp);
	msg.remove(length,1);
	Serial.println("received");
	Serial.println(msg);

}

/**
*	CoolMQTT::read():
*	This method is provided to return the last
*	read message.
*/
String CoolMQTT::read()
{	
	if(this->newMsg==true)
	{
		return(this->msg);
		this->newMsg=false;
	}
	return(" ");

}

/**
*	CoolMQTT::config():
*	This method is provided to configure
*	the mqttClient :	-server
*				-inTopic
*				-outTopic
*				-client Id
*				-buffer size	
*
*	\return true if successful,false otherwise
*/
bool CoolMQTT::config()
{
	//read config file
	//update data
	File configFile = SPIFFS.open("/mqttConfig.json", "r");

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
				if(json["mqttServer"].success() )
				{			
					const char* tempmqttServer = json["mqttServer"]; 
					for(int i =0;i< 50 ;i++)
					{
						mqttServer[i]=tempmqttServer[i];
					}
				}
				else
				{
					for(int i =0;i< 50 ;i++)
					{
						this->mqttServer[i]=this->mqttServer[i];
					}

				}
				json["mqttServer"]=this->mqttServer;

				
				if(json["inTopic"].success() )
				{
					const char* tempInTopic = json["inTopic"]; 
					for(int i =0;i< 50;i++)
					{
						inTopic[i]=tempInTopic[i];
					}
				}
				else
				{
					String tempMAC = WiFi.macAddress();
					tempMAC.replace(":","");
					snprintf(inTopic, 50, "$aws/things/%s/shadow/update/delta", tempMAC.c_str());	
					Serial.print("Set Incomming MQTT Channel to : ");
					Serial.println(inTopic);	
				}
				json["inTopic"]=this->inTopic;
				
				
				if(json["outTopic"].success() )
				{
					const char* tempOutTopic = json["outTopic"]; 
					for(int i =0;i<50;i++)
					{
						outTopic[i]=tempOutTopic[i];
					}
				}
				else
				{
					String tempMAC = WiFi.macAddress();
					tempMAC.replace(":","");
					snprintf(outTopic, 50, "$aws/things/%s/shadow/update", tempMAC.c_str());
					Serial.print("Set Outgoing MQTT Channel to : ");
					Serial.println(outTopic);
				}
				json["outTopic"]=this->outTopic;
			
				
				if(json["user"].success() )
				{				
					const char* tempUser = json["user"]; 
					for(int i =0;i<50;i++)
					{
						user[i]=tempUser[i];
					}
				}
				else
				{
					for(int i=0;i<50;i++)
					{
						this->user[i]=this->user[i];
					}				
				}
				json["user"]=this->user;
				
				if(json["bufferSize"].success() )
				{
					int tempBufferSize = json["bufferSize"]; 
					bufferSize=tempBufferSize;
				}
				else
				{
					this->bufferSize=this->bufferSize;
				}
				json["bufferSize"]=this->bufferSize;

				configFile.close();
				configFile = SPIFFS.open("/mqttConfig.json", "w");
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
*	CoolMQTT::config(server,in topic, out topic , user Id, buffer size):
*	This method is provided to manually configure the mqtt client	
*
*/
void CoolMQTT::config(const char mqttServer[],const char inTopic[],const char outTopic[],const char user[],int bufferSize)
{
	for(int i =0;i< 50 ;i++)
	{
		this->mqttServer[i]=mqttServer[i];
		this->inTopic[i]=inTopic[i];
		this->outTopic[i]=outTopic[i];
		this->user[i]=user[i];
	}
	this->bufferSize=bufferSize;

}

/**
*	CoolMQTT::printConf():
*	This method is provided to print the
*	configuration to the Serial Monitor
*/
void CoolMQTT::printConf()
{
	Serial.println("MQTT conf ");
	Serial.println(mqttServer);
	Serial.println(inTopic);
	Serial.println(outTopic);
	Serial.println(user);
	Serial.println(bufferSize);
	Serial.println(" ");


}

/**
*	CoolMQTT::getUser():
*	This method is provided to get the user name
*/
String CoolMQTT::getUser()
{
	return String(user);
}
