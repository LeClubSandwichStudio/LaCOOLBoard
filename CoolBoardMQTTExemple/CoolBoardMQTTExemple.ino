#include<CoolMQTT.h>
#include<CoolWifi.h>

CoolWifi wifiManager;
CoolMQTT mqtt;

String ssid[] = {"lapaillasse","ssid"};
String pass[]={"biohacking","pass"};


const char mqttServer[]="broker.mqtt-dashboard.com";
const char inTopic[]="inTopic";
const char outTopic[]="outTopic";
const char clientId[]="espAshiroji";
int bufferSize = 128;//bytes
int keepAlive=15; //seconds


void setup()
{
	Serial.begin(115200);

	wifiManager.config(ssid,pass,2,180);
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
mqtt.publish("hello world by CoolBoard");
delay(1000);
Serial.println(mqtt.read());
delay(1000);
mqtt.mqttLoop();
delay(1000);
}
