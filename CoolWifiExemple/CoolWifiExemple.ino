#include<CoolWifi.h>

CoolWifi wifiManager;

String ssid[] = {"lapaillasse","ssid"};
String pass[]={"biohacking","pass"};


void setup()
{
	wifiManager.config(ssid,pass,2,180);
	wifiManager.begin();
	wifiManager.printConf();
	
	wifiManager.connect();
}


void loop()
{
}
