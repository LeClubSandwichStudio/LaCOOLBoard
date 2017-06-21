#include <CoolFileSystem.h>
#include <CoolBoardLed.h>
#include <CoolBoardSensors.h>
#include <CoolMQTT.h>
#include <CoolTime.h>

#include <Jetpack.h>

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

CoolFileSystem coolFS;
CoolMQTT mqtt;
CoolTime coolRTC;
Jetpack jetpack;
int yearC,monthC,dayC,hourC,minuteC,secondC;

char data[290];
String answer;
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
Serial.print("Fs begin : ");
Serial.println(coolFS.begin());

WiFiManager wifiManager;

Serial.println(mqtt.config());
Serial.print("mqtt data ") ;
mqtt.printData();

delay(500);

mqtt.begin();
delay(500);

wifiManager.autoConnect("CoolBoard"); 
Serial.print("Wifi connected to ");
Serial.println(WiFi.SSID());
delay(3000);  

coolRTC.config();
coolRTC.begin();
coolRTC.update();
coolRTC.getTimeDate(yearC,monthC,dayC,hourC,minuteC,secondC);
Serial.print(yearC);Serial.print(":");Serial.print(monthC);Serial.print(":");Serial.print(dayC);Serial.print("  ");Serial.print(hourC);Serial.print(":");Serial.print(minuteC);Serial.print(":");Serial.print(secondC);

Serial.println("Jetpack test :");
jetpack.config();
jetpack.begin();

jetpack.write(0xff);
delay(500);
jetpack.write(0x00);

}

void loop() {
  // put your main code here, to run repeatedly:
if(mqtt.connect()==1)
{Serial.println("connected to mqtt ");
  }
  else
  { Serial.println("not connected to mqtt");
      }
delay(1000);


mqtt.publish("hello by Ash");

Serial.println("loop");
delay(1000);
answer=mqtt.read();
Serial.print("answer : ");
Serial.println(answer);
 delay(1000);
}
