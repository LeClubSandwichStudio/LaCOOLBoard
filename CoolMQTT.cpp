#include "FS.h"
#include "Arduino.h"  
#include <ESP8266WiFi.h>
#include <PubSubClient.h>                             
#include "CoolMQTT.h"
#include "ArduinoJson.h"

void CoolMQTT::begin()
{ 
  client.setClient(espClient);
  client.setServer(mqtt_server, 1883);	
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
	delay(50);
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
void CoolMQTT::callback(char* topic, byte* payload, unsigned int length) {
   //some leak problems
    char temp[length+1];
    for (int i = 0; i < length; i++) {
    temp[i]=(char)payload[i]; 
    

  }
  
  temp[length+1]='\0';
  
  msg=String(temp);
  msg.remove(length,1);
  Serial.println("received");
  Serial.println(msg);

}

String CoolMQTT::read()
{
	return(this->msg);
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
				if(json["mqtt_server"].success() )
				{			
					const char* temp_mqttServer = json["mqtt_server"]; // "inTopic"
					for(int i =0;i< 50 ;i++)
					{
						mqtt_server[i]=temp_mqttServer[i];
					}
				}
				
				if(json["inTopic"].success() )
				{
					const char* temp_inTopic = json["inTopic"]; // "inTopic"
					for(int i =0;i< 50;i++)
					{
						inTopic[i]=temp_inTopic[i];
					}
				}
				
				if(json["outTopic"].success() )
				{
					const char* temp_outTopic = json["outTopic"]; // "outTopic"
					for(int i =0;i<50;i++)
					{
						outTopic[i]=temp_outTopic[i];
					}
				}
			
				
				if(json["clientId"].success() )
				{				
					const char* temp_clientId = json["clientId"]; // "espAshiroji"
					for(int i =0;i<50;i++)
					{
						clientId[i]=temp_clientId[i];
					}
				}
				
				if(json["bufferSize"].success() )
				{
					int temp_bufferSize = json["bufferSize"]; // 512
					bufferSize=temp_bufferSize;
				}
				
			  return(true); 
		}
	}	
	

}
void CoolMQTT::printConf()
{
Serial.println("MQTT conf ");
Serial.println(mqtt_server);
Serial.println(inTopic);
Serial.println(outTopic);
Serial.println(clientId);
Serial.println(bufferSize);
Serial.println(" ");


}
