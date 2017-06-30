/**
*	\file Jetpack.cpp
*	\brief Jetpack Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/

#include "FS.h"
#include "Arduino.h"
#include "ArduinoJson.h"

#include "Jetpack.h"

/**
*	Jetpack::begin():
*	This method is provided to
*	initialise the pin that control
*	the Jetpack shield
*/
void Jetpack::begin()
 { 

	pinMode(EnI2C,OUTPUT);
	pinMode(dataPin,OUTPUT);
	pinMode(clockPin,OUTPUT);
	
	for(int i=0;i<8;i++)
	{
		if(this->actors[i].actif==1)
		{
			if(this->actors[i].temporal==1)
			{
				tickerSetHigh[i].attach( (this->actors[i].high), this->setBit ,i );


				tickerSetLow[i].attach( (this->actors[i].low), this->resetBit ,i );
			
			}
			else
			{
				tickerSetHigh[i].detach();


				tickerSetLow[i].detach();

			}		
		}
	}



 }


/**
*	Jetpack::write(action):
*	This method is provided to write
*	the given action to the entire Jetpack
*	action is a Byte (8 bits ), each bit goes 
*	to an output. 
*	MSBFirst 
*/
void Jetpack::write(byte action)
{
	this->action=action;

	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);
}	

/**
*	Jetpack::writeBit(pin,state):
*	This method is provided to write
*	the given state to the given pin
*/
void Jetpack::writeBit(byte pin,bool state) 
{

	bitWrite(this->action, pin, state);
	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);

}

/**
*	Jetpack::setBit(pin):
*	This method is provided to
*	directly put to HIGH the pin
*	passed as argument
*/
void Jetpack::setBit(byte pin)
{
	this->writeBit(pin,1); 
}

/**
*	Jetpack::setBit(pin):
*	This method is provided to
*	directly put to LOW the pin
*	passed as argument
*/	
void Jetpack::resetBit(byte pin)
{
	this->writeBit(pin,0); 
}

/**
*	Jetpack::doAction(sensor data, sensor data size):
*	This method is provided to automate the Jetpack.
*	exemple:
*	initial state:
*		current Temperature = 23 °C
*		actors[0].actif=1
*		actors[0].low=25 °C
*		actors[0].high=30 °C
*		actors[0].type="Temperature"
*		
*	condition verified:		
*		root["Temperature"]<actors[0].low
*
*	action: invert the state of actors[0]:
*		bitWrite( action,0,!( bitRead ( action,0 ) ) )
*		write(action)
*	
*/
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
			if( this->actors[i].temporal==0 ) 
			{
				if( ( ( root[this->actors[i].type] ) > ( this->actors[i].high ) ) || ( ( root[ this->actors[i].type ] ) < ( this->actors[i].low ) ) )	
				{	
					bitWrite( this->action , i , !( bitRead(this->action, i ) ) );	
				}
			}
		}
	}

	this->write(this->action);
}

/**
*	Jetpack::config():
*	This method is provided to configure the
*	Jetpack with a configuration file
*
*	\return true if successful,false otherwise
*/ 
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
						json[String("Act")+String(i)]["actif"]=this->actors[i].actif;


						if(json[String("Act")+String(i)]["low"].success() )
						{					
							this->actors[i].low=json[String("Act")+String(i)]["low"];
						}
						else
						{
							this->actors[i].low=this->actors[i].low;					
						}
						json[String("Act")+String(i)]["low"]=this->actors[i].low;
	
					
						if(json[String("Act")+String(i)]["high"].success() )
						{				
							this->actors[i].high=json[String("Act")+String(i)]["high"];
						}
						else
						{
							this->actors[i].high=this->actors[i].high;
						}
						json[String("Act")+String(i)]["high"]=this->actors[i].high;

					
						if(json[String("Act")+String(i)]["type"].success() )
						{				
							this->actors[i].type=json[String("Act")+String(i)]["type"]; 
						}
						else
						{
							this->actors[i].type=this->actors[i].type;
						}
						json[String("Act")+String(i)]["type"]=this->actors[i].type;

						if(json[String("Act")+String(i)["temporal"].success() )
						{
							this->actors[i].temporal=json[String("Act")+String(i)]["temporal"]; 													
						}
						else
						{
							this->actors[i].temporal=json[String("Act")+String(i)]["temporal"]; 
						}	
						json[String("Act")+String(i)]["temporal"]=this->actors[i].temporal; 
					}
					else
					{
						this->actors[i]=this->actors[i];
					}
					
					//json[String("Act")+String(i)]=this->actors[i];
				}
			}
			else
			{
				this->actorsNumber=this->actorsNumber;
			}
			json["actorsNumber"]=this->actorsNumber;
			
			return(true); 
		}
	}	
	

}

/**
*	Jetpack::printConf():
*	This method is provided to
*	print the configuration to the 
*	Serial Monitor
*/
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
 


