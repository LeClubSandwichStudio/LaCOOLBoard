#include<CoolTime.h>
#include<CoolWifi.h>
#include<Wire.h>

CoolWifi wifiManager;
CoolTime clock;

String ssid[] = {"ssid0","ssid1"};
String pass[]={"pass0","pass1"};

IPAddress timeServer(132,163,4,101);
unsigned int localPort = 8888;

void setup()
{
	Wire.begin(2,14);

	Serial.begin(115200);
	
	wifiManager.config(ssid,pass,2,180,0);
	
	wifiManager.begin();

	wifiManager.printConf();
	
	wifiManager.connect();
	

	clock.config(timeServer,localPort);

	clock.begin();

	

}

void loop()
{  
	Serial.println(clock.getESDate());
	delay(3000);
}
