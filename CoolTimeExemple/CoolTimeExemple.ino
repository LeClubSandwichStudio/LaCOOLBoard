#include<CoolTime.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include<Wire.h>

WiFiManager wifiManager;
CoolTime clock;


IPAddress timeServer(132,163,4,101);
unsigned int localPort = 8888;

void setup()
{
  Wire.begin(2,14);
	Serial.begin(115200);

	if(WiFi.status() != WL_CONNECTED)
	{
		Serial.println("Entering WiFi Connect");
		wifiManager.autoConnect("CoolBoardAP");
		Serial.println("Wifi Set" );
	}
	
	clock.config(timeServer,localPort);
	clock.begin();

	

}

void loop()
{  
  Serial.println(clock.getESDate());
  delay(3000);
}
