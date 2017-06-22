#include<CoolMQTT.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

WiFiManager wifiManager;
CoolMQTT mqtt;

const char mqttServer[]="broker.mqtt-dashboard.com";
const char inTopic[]="inTopic";
const char outTopic[]="outTopic";
const char clientId[]="espAshiroji";
int bufferSize = 128;//bytes
int keepAlive=15; //seconds


void setup()
{
	Serial.begin(115200);

	if(WiFi.status() != WL_CONNECTED)
	{
		Serial.println("Entering WiFi Connect");
		wifiManager.autoConnect("CoolBoardAP");
		Serial.println("Wifi Set" );
	}

	mqtt.config(mqttServer,inTopic, outTopic,clientId,bufferSize);
	mqtt.begin();
	mqtt.connect(keepAlive);
}

void loop()
{
mqtt.publish("hello world by CoolBoard");
delay(1000);
Serial.println(mqtt.read());
delay(1000);
mqtt.mqttLoop();
delay(1000);
}
