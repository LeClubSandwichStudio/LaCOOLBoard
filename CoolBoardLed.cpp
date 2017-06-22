#include "FS.h"
#include "Arduino.h"

#include <NeoPixelBus.h>
#include "CoolBoardLed.h"
#include "ArduinoJson.h"

void CoolBoardLed::colorFade(int R, int G, int B, int T) {
  for (int k = 0; k < 100; k++) {
    neoPixelLed->SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    neoPixelLed->Show();
    delay(T);
  }
  // Fade OUT
  for (int k = 100; k >= 0; k--) {
    neoPixelLed->SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    neoPixelLed->Show();
    delay(T);
  }
}

void CoolBoardLed::blink(int R, int G, int B, int T) {
  neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
  neoPixelLed->Show();
  delay(T);
  neoPixelLed->SetPixelColor(0, RgbColor(0, 0, 0));
  neoPixelLed->Show();
}

void CoolBoardLed::fadeIn(int R, int G, int B, int T) {
  for (int k = 0; k < 100; k++) {
    neoPixelLed->SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    neoPixelLed->Show();
    delay(T);
  }
}

void CoolBoardLed::fadeOut(int R, int G, int B, int T) {
  for (int k = 100; k >= 0; k--) {
    neoPixelLed->SetPixelColor(0, RgbColor(k * R / 100, k * G / 100, k * B / 100));
    neoPixelLed->Show();
    delay(T);
  }
}

void CoolBoardLed::strobe(int R, int G, int B, int T) {
  for (int k = 5; k >= 0; k--) {
    neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
    neoPixelLed->Show();
    delay(T);
    neoPixelLed->SetPixelColor(0, RgbColor(0, 0, 0));
    neoPixelLed->Show();
    delay(T);
  }
}

void CoolBoardLed::end()
{
 delete neoPixelLed;
}

void CoolBoardLed::neoPixelLedBegin()
{   pinMode(5,OUTPUT);
    digitalWrite(5,HIGH);
    neoPixelLed = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(1,2); //create the led
    neoPixelLed->Begin();
    neoPixelLed->Show();

}

void CoolBoardLed::begin( )
{
//starts the actor
  this->neoPixelLedBegin();
} 


void CoolBoardLed::write(int R, int G, int B)
{
    neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
    neoPixelLed->Show();
}

bool CoolBoardLed::config()
{
	File coolBoardLedConfig = SPIFFS.open("/coolBoardLedConfig.json", "r");

	if (!coolBoardLedConfig) 
	{
		return(false);
	}
	else
	{
		size_t size = coolBoardLedConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		coolBoardLedConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
			  return(false);
		} 
		else
		{  	  
			if(json["ledActive"].success() )
			{
			this->ledActive = json["ledActive"]; 
			}
			  return(true); 
		}
	}	

}				

void CoolBoardLed::printConf()
{
	Serial.println("Led Conf");
	Serial.println(ledActive);
	Serial.println(" ");	
}
