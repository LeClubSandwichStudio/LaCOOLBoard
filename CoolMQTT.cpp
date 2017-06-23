#include "FS.h"
#include "Arduino.h"  
#include <ESP8266WiFi.h>
#include <PubSubClient.h>                             
#include "CoolMQTT.h"
#include "ArduinoJson.h"

void CoolMQTT::begin()
{ 
	client.setClient(espClient);
	client.setServer(mqttServer, 1883);	
	client.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
	client.setBufferSize((unsigned short)bufferSize);

}

bool CoolMQTT::state()
{
	return(client.state());
}

int CoolMQTT::connect(uint16_t keepAlive)
{       
	int i=0;
	Serial.println("MQTT connecting...");
	while ((!client.connected())&&(i<100)) 
	{
		// Attempt to connect
		if (client.connect(clientId,keepAlive)) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			client.publish(outTopic, "hello world by Ash");
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


bool CoolMQTT::publish(const char* data)
{

	//data is in JSON, publish it directly
	Serial.println("data to publish");
	Serial.println(data);
	bool pub=client.publish( outTopic, data,sizeof(data) );
	return( pub);

}

bool CoolMQTT::mqttLoop()
{
	this->client.loop();
	return(client.loop());
}

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

String CoolMQTT::read()
{	
	if(this->newMsg==true)
	{
		return(this->msg);
		this->newMsg=false;
	}
	return(" ");

}

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
					const char* tempmqttServer = json["mqttServer"]; // "inTopic"
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

				
				if(json["inTopic"].success() )
				{
					const char* tempInTopic = json["inTopic"]; // "inTopic"
					for(int i =0;i< 50;i++)
					{
						inTopic[i]=tempInTopic[i];
					}
				}
				else
				{
					for(int i=0;i<50;i++)
					{
						this->inTopic[i]=this->inTopic[i];
					}				
				}
				
				if(json["outTopic"].success() )
				{
					const char* tempOutTopic = json["outTopic"]; // "outTopic"
					for(int i =0;i<50;i++)
					{
						outTopic[i]=tempOutTopic[i];
					}
				}
				else
				{
					for(int i=0;i<50;i++)
					{
						this->outTopic[i]=this->outTopic[i];
					}
				}			
				
				if(json["clientId"].success() )
				{				
					const char* tempClientId = json["clientId"]; // "espAshiroji"
					for(int i =0;i<50;i++)
					{
						clientId[i]=tempClientId[i];
					}
				}
				else
				{
					for(int i=0;i<50;i++)
					{
						this->clientId[i]=this->clientId[i];
					}				
				}
				
				if(json["bufferSize"].success() )
				{
					int tempBufferSize = json["bufferSize"]; // 512
					bufferSize=tempBufferSize;
				}
				else
				{
					this->bufferSize=this->bufferSize;
				}
			  
			  return(true); 
		}
	}	
	

}

void CoolMQTT::config(const char mqttServer[],const char inTopic[],const char outTopic[],const char clientId[],int bufferSize)
{
	for(int i =0;i< 50 ;i++)
	{
		this->mqttServer[i]=mqttServer[i];
		this->inTopic[i]=inTopic[i];
		this->outTopic[i]=outTopic[i];
		this->clientId[i]=clientId[i];
	}
	this->bufferSize=bufferSize;

}

void CoolMQTT::printConf()
{
	Serial.println("MQTT conf ");
	Serial.println(mqttServer);
	Serial.println(inTopic);
	Serial.println(outTopic);
	Serial.println(clientId);
	Serial.println(bufferSize);
	Serial.println(" ");


}
