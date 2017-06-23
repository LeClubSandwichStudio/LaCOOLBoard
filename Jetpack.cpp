/*
*
* This class manages the external Actors
*
*
*
*/
#include "FS.h"
#include "Arduino.h"
#include "ArduinoJson.h"

#include "Jetpack.h"

void Jetpack::begin()
 { 
	//tests all the actors
	pinMode(EnI2C,OUTPUT);
	pinMode(dataPin,OUTPUT);
	pinMode(clockPin,OUTPUT);

	this->action=0xFF;

	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);
	
	delay(100);
	
	this->action=0x00;
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);

 }



void Jetpack::write(byte action)
{
	this->action=action;

	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);
}	


void Jetpack::writeBit(byte pin,bool state) 
{

	bitWrite(this->action, pin, state);
	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);

}

void Jetpack::doAction(const char* data,int JSON_SIZE)
{
	DynamicJsonBuffer jsonBuffer(JSON_SIZE);
	JsonObject& root = jsonBuffer.parseObject(data);
	
	//invert the current action state for each actor
	//if the value is outside the limits
	for(int i=0;i<8;i++)
	{
		if(this->actors[i].actif==1)
		{
			if( ((root[this->actors[i].type])>(this->actors[i].high)) || ((root[ this->actors[i].type ])<(this->actors[i].low)) )	
			{	
			bitWrite(this->action , i , !(bitRead(this->action, i ) ) );	
			}
		}
	}
	this->write(this->action);
}

 
bool Jetpack::config()
{

	File jetPackConfig = SPIFFS.open("/jetPackConfig.json", "r");

	if (!jetPackConfig) 
	{
		return(false);
	}
	else
	{
		size_t size = jetPackConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		jetPackConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
			  return(false);
		} 
		else
		{  	  
			if(json["ActorsNumber"].success() )
			{
				this->actorsNumber = json["ActorsNumber"]; 
			
				for(int i=0;i<this->actorsNumber;i++)
				{	if(json[String("Act")+String(i)].success())
					{
						if(json[String("Act")+String(i)]["actif"].success() )
						{
							this->actors[i].actif=json[String("Act")+String(i)]["actif"];
						}
						else
						{
							this->actors[i].actif=this->actors[i].actif;
						}

						if(json[String("Act")+String(i)]["low"].success() )
						{					
							this->actors[i].low=json[String("Act")+String(i)]["low"];
						}
						else
						{
							this->actors[i].low=this->actors[i].low;					
						}	
					
						if(json[String("Act")+String(i)]["high"].success() )
						{				
							this->actors[i].high=json[String("Act")+String(i)]["high"];
						}
						else
						{
							this->actors[i].high=this->actors[i].high;
						}
					
						if(json[String("Act")+String(i)]["type"].success() )
						{				
							this->actors[i].type=json[String("Act")+String(i)]["type"]; 
						}
						else
						{
							this->actors[i].type=this->actors[i].type;
						}	
					}
					else
					{
						this->actors[i]=this->actors[i];
					}
				}
			}
			else
			{
				this->actorsNumber=this->actorsNumber;
			}
			
			return(true); 
		}
	}	
	

}

void Jetpack::printConf()
{
	Serial.println("Jetpack Config ");
	Serial.println(this->actorsNumber); 
        for(int i=0;i<this->actorsNumber;i++)
	{
	Serial.println(this->actors[0].actif);
	Serial.println(this->actors[0].low);
	Serial.println(this->actors[0].high);
	Serial.println(this->actors[0].type); 
	}
}
 


