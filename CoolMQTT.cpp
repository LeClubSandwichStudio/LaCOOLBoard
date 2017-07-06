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
	Serial.println("Entering CoolMQTT.begin()");
	Serial.println();

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
	Serial.println("Entering CoolMQTT.state()");
	Serial.println();
	
	Serial.print("state : ");
	Serial.println( this->client.state() );
	
	return( this->client.state() );
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
	Serial.println("Entering CoolMQTT.connect()");

	int i=0;
	Serial.println("MQTT connecting...");
	while( ( !this->client.connected() ) && ( i<100 ) ) 
	{
		// Attempt to connect
		if( this->client.connect( this-> user, keepAlive ) )
		{
			Serial.println("MQTT connected");
			// Once connected, publish an announcement...
			//client.publish(outTopic, "hello world by Ash");
			// ... and resubscribe
			client.subscribe( this->inTopic );
			Serial.println(" subscribed , leavin ") ;
			return( this->state() );
		}
		else
		{
			Serial.println("not connected , retrying");
			
		}
	delay(5);
	i++;
	}
	
	return( this->state() );

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

	Serial.println("Entering CoolMQTT.publish()");
	Serial.println();
	//data is in JSON, publish it directly

	Serial.println("data to publish");
	Serial.println(data);
	Serial.print("data size ");Serial.println(strlen(data));
	Serial.println();
	
	bool pub=client.publish( this->outTopic, data,strlen(data));
	
	Serial.print("success : ");Serial.println(pub);	

	return(pub);

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
	Serial.println("Entering CoolMQTT.publish() every logInterval ");
	Serial.println();
	
	if( ( millis() - ( this->previousLogTime)  ) >=( logInterval ) )
	{
		Serial.println("log Interval has passed ");
		Serial.println();

		this->publish(data);

		this->previousLogTime=millis();

		Serial.print("last log time : ");
		Serial.println(this->previousLogTime);

		return(true);
	}

	Serial.println("log Interval still didn't pass ");	
	Serial.println();

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
	unsigned long lastTime=millis();

	Serial.println("Entering CoolMQTT.mqttLoop()");
	Serial.println();

	

	while( ( millis() - lastTime ) < 5000)
	{
		this->client.loop();	
	}
	
	Serial.print("loop result : ");
	Serial.println( this->client.loop() );
	Serial.println();

	return( this->client.loop() );
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
	Serial.println("Entering CoolMQTT.callback() ");
	Serial.println();

	if(this->newMsg==false)
	{
		char temp[length+1];
		Serial.println("received temp msg : ");
		
		for (int i = 0; i < length; i++) 
		{
			temp[i]=(char)payload[i];
			Serial.print( (char)payload[i] );

		}

		Serial.println();
	
		Serial.println("storing new message : ");
		Serial.println();

		this->newMsg=true;

		temp[length+1]='\0';

		this->msg=String(temp);
		this->msg.remove(length,1);
		Serial.println("stored message : ");
		Serial.println(this->msg);
	}
	else
	{
		Serial.println("did not read last message");
		
	}

}

/**
*	CoolMQTT::read():
*	This method is provided to return the last
*	read message.
*/
String CoolMQTT::read()
{	
	Serial.println("Entering CoolMQTT.read()");
	Serial.println();
	if(this->newMsg==true)
	{
		
		this->newMsg=false;

		Serial.println("received new message");
		Serial.println("message : ");
		Serial.println(this->msg);
		Serial.println();
				
		return(this->msg);
		
	}
	return("");

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
	Serial.println("Entering CoolMQTT.config()");
	Serial.println();

	//read config file
	//update data
	File configFile = SPIFFS.open("/mqttConfig.json", "r");

	if (!configFile) 
	{
		Serial.println("failed to read /mqttConfig.json");
		Serial.println();
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
			Serial.println("failed to parse json ");
			Serial.println();
			
			return(false);
		} 
		else
		{		
			Serial.println("configuration json is ");
			json.printTo(Serial);
			Serial.println();
	
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
				Serial.println("failed to write to /mqttConfig.json");
				return(false);				
			}
			
			json.printTo(configFile);

			Serial.println("saved configuration is :");
			json.printTo(Serial);
			Serial.println();

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
	Serial.println("Entering CoolMQTT.config() , no SPIFFS variant");
	Serial.println();

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
	Serial.println("Entering CoolMQTT.printConf()");
	Serial.println();	
	
	Serial.println("MQTT configuration ");

	Serial.print("mqttServer : ");
	Serial.println(this->mqttServer);

	Serial.print("inTopic : ");
	Serial.println(this->inTopic);

	Serial.print("outTopic : ");
	Serial.println(this->outTopic);

	Serial.print("user : ");
	Serial.println(this->user);

	Serial.print("bufferSize : ");
	Serial.println(this->bufferSize);

	Serial.println();


}

/**
*	CoolMQTT::getUser():
*	This method is provided to get the user name
*/
String CoolMQTT::getUser()
{
	Serial.println("Entering CoolMQTT.getUser()");
	Serial.println();
	
	Serial.print("user : ");
	Serial.println(this->user);

	return String(this->user);
}
