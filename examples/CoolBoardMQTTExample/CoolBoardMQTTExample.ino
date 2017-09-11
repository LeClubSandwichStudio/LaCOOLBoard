/**
*	CoolBOardMQTTExample
*
*	This example shows basic usage of
*	the MQTT protocol
*
*/
#include<CoolMQTT.h>
#include<CoolWifi.h>

CoolWifi wifiManager;
CoolMQTT mqtt;

String ssid[] = {"ssid1","ssid2"};//put up to 50 WiFi ssids
String pass[]={"pass1","pass2"};//put up to 50 WiFi passwords


const char mqttServer[]="----------";//the mqtt server you're going to use
const char inTopic[]="--------";//the topic you're going to subscribe to (receive from)
const char outTopic[]="-------";//the topic you're going to publish to (send to )
const char clientId[]="------";//your user name
int bufferSize = 128;//bytes
int keepAlive=15; //seconds


void setup()
{
	Serial.begin(115200);

	wifiManager.config(ssid,pass,2,180,0);
	wifiManager.begin();
	wifiManager.printConf();

	mqtt.config(mqttServer,inTopic, outTopic,clientId,bufferSize);
	mqtt.begin();
	mqtt.printConf();
	
	wifiManager.connect();

	mqtt.connect(keepAlive);
}

void loop()
{
mqtt.publish("hello world by CoolBoard");//publish this message to the outTopic
delay(1000);
Serial.println(mqtt.read());//print answer if answer is received
delay(1000);
mqtt.mqttLoop();
delay(1000);
}
