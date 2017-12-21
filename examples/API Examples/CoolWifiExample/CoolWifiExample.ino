/**
*	CoolWifiExample	
*
*	This example shows typical use
*	of the CoolWifi class.
*		
*/
#include<CoolWifi.h>

CoolWifi wifiManager;

String ssid[] = {"ssid0","ssid1"};
String pass[]={"pass0","pass1"};


void setup()
{
  Serial.begin(115200);
	//config(ssid array, pass array, number of wifis, AP timeout,nomad flag );
	wifiManager.config(ssid,pass,2,180,0);
	wifiManager.begin();
	wifiManager.printConf();
	
	wifiManager.connect();
}


void loop()
{
}
