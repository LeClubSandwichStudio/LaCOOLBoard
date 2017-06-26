/**
*
* This class handles the led in the Sensor Board
*
*
*
*/

#include "FS.h"
#include "Arduino.h"

#include <NeoPixelBus.h>
#include "CoolBoardLed.h"
#include "ArduinoJson.h"


/**
*	CoolBoardLed::colorFade ( Red , Green , Blue, Time in seconds ):
*	colorFade animation:	Fade In over T(seconds)
*				Fade Out over T(seconds)
*/
void CoolBoardLed::colorFade(int R, int G, int B, int T) 
{
	for (int k = 0; k < 1000; k++) 
	{
		neoPixelLed->SetPixelColor(0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
		neoPixelLed->Show();
		delay(T);
	}
	for (int k = 1000; k >= 0; k--) 
	{
		neoPixelLed->SetPixelColor(0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
		neoPixelLed->Show();
		delay(T);
	}
}

/**
*	CoolBoardLed::blink( Red , Green , Blue , Time in seconds ):
*	Blink animation:	Led On for T seconds
				Led off
*/
void CoolBoardLed::blink(int R, int G, int B, int T) 
{
	neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
	neoPixelLed->Show();
	delay(T);
	neoPixelLed->SetPixelColor(0, RgbColor(0, 0, 0));
	neoPixelLed->Show();
}

/**
*	CoolBoardLed::fadeIn(Red , Green , Blue , Time in seconds)
*	Fade In animation:	gradual increase over T(seconds)
*/
void CoolBoardLed::fadeIn(int R, int G, int B, int T) 
{
	for (int k = 0; k < 1000; k++) 
	{
		neoPixelLed->SetPixelColor(0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
		neoPixelLed->Show();
		delay(T);
	}
}

/**
*	CoolBoardLed::fadeOut( Red , Green , Blue , Time in seconds)
*	Fade Out animation:	gradual decrease over T(seconds)
*/
void CoolBoardLed::fadeOut(int R, int G, int B, int T) 
{
	for (int k = 1000; k >= 0; k--) 
	{
		neoPixelLed->SetPixelColor(0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
		neoPixelLed->Show();
		delay(T);
	}
}

/**
*	CoolBoardLed::strobe(Red , Green , Blue , Time in seconds)
*	Strobe animation:	blinks over T(seconds)	
*/
void CoolBoardLed::strobe(int R, int G, int B, int T) 
{
	for (int k = 1000; k >= 0; k--) 
	{
		neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
		neoPixelLed->Show();
		delay(T);
		neoPixelLed->SetPixelColor(0, RgbColor(0, 0, 0));
		neoPixelLed->Show();
		delay(T);
	}
}

/**
* 	CoolBoardLed::end() :
*	this method is provided to delete the dynamically created neoPixelLed
*/
void CoolBoardLed::end()
{
	delete neoPixelLed;
}


/**
*	CoolBoardLed::begin():
*	This method is provided to start the Led Object 
*	by setting the correct pin and creating a dynamic
*	neoPixelBus  
*/
void CoolBoardLed::begin( )
{
	pinMode(5,OUTPUT);
	digitalWrite(5,HIGH);
	neoPixelLed = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(1,2); 
	neoPixelLed->Begin();
	neoPixelLed->Show();
} 

/**
*	CoolBoardLed::write(Red,Green,Blue):
*	This method is provided to set the 
*	Color of the Led
*/
void CoolBoardLed::write(int R, int G, int B)
{
	neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
	neoPixelLed->Show();
}

/**
*	CoolBoardLed::config():
*	This method is provided to configure
*	the Led Object :	-ledActive=0 : deactivated
*				-ledActive=1 : activated			
*/
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
			else
			{
				this->ledActive=this->ledActive;			
			}
			
			json["ledActive"]=this->ledActive;
			coolBoardLedConfig.close();
			
			coolBoardLedConfig = SPIFFS.open("/coolBoardLedConfig.json", "w");
			if(!coolBoardLedConfig)
			{
				return(false);			
			}

			json.printTo(coolBoardLedConfig);
			coolBoardLedConfig.close();

			  return(true); 
		}
	}	

}				

/**
*	CoolBoardLed::printConf():
*	This method is provided to print the
*	Led Object Configuration to the Serial
*	Monitor
*/
void CoolBoardLed::printConf()
{
	Serial.println("Led Conf");
	Serial.println(ledActive);
	Serial.println(" ");	
}
