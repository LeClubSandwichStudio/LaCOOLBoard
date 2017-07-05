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
	Serial.println("Entering Jetpack.begin() ");
	Serial.println();

	pinMode(EnI2C,OUTPUT);
	pinMode(dataPin,OUTPUT);
	pinMode(clockPin,OUTPUT);
	
	

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
	Serial.println("Entering Jetpack.write()");
	Serial.println();

	Serial.println("writing this action : ");
	Serial.println(action,HEX);
	Serial.println();

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
	Serial.println("Entering Jetpack.writeBit() ");
	Serial.print("Writing ");Serial.print(state);Serial.print("to pin N°");Serial.print(pin);
	Serial.println();

	bitWrite(this->action, pin, state);
	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);

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
	Serial.println("Entering Jetpack.doAction()");
	Serial.println();

	Serial.println("input data is :");
	Serial.println(data);
	Serial.println();

	Serial.println("input size is :");	
	Serial.println(JSON_SIZE);
	Serial.println();

	DynamicJsonBuffer jsonBuffer(JSON_SIZE);
	JsonObject& root = jsonBuffer.parseObject(data);
	
	if (!root.success()) 
	{
		Serial.println("failed to parse json object ");
		Serial.println();
	}
	else
	{
		Serial.println("created Json object :");
		root.printTo(Serial);
		Serial.println();

		//invert the current action state for each actor
		//if the value is outside the limits
		for(int i=0;i<8;i++)
		{
			//check if the actor is actif 
			if(this->actors[i].actif==1)
			{	
				//if the actor is not temporal
				if( this->actors[i].temporal==0 ) 
				{
					if( ( ( root[this->actors[i].type] ) > ( this->actors[i].high ) ) || ( ( root[ this->actors[i].type ] ) < ( this->actors[i].low ) ) )	
					{	
						bitWrite( this->action , i , !( bitRead(this->action, i ) ) );	
					}
				}
				//if the actor is temporal
				else
				{	//if the actor was actif for highTime or more :
					if( ( millis()- this->actors[i].actifTime  ) >= ( this->actors[i].high  ) )
					{
						//stop the actor
						bitWrite( this->action , i , 0) ;

						//make the actor inactif:
						this->actors[i].actif=0;

						//start the low timer
						this->actors[i].inactifTime=millis();				
					}			
							
				}
			}
			//check if actor is inactif
			else
			{	//check if actor is temporal
				if(this->actors[i].temporal==1)
				{
					//if the actor was inactif for lowTime or more :
					if( ( millis() - this->actors[i].inactifTime ) >= ( this->actors[i].low  ) )
					{
						//start the actor
						bitWrite( this->action , i , 1) ;

						//make the actor actif:
						this->actors[i].actif=1;

						//start the low timer
						this->actors[i].actifTime=millis();				
					}			
			
				}
			}
		}

		Serial.println("new action is : ");
		Serial.println(this->action);
		Serial.println();

		this->write(this->action);

	} 
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
	Serial.println("Entering Jetpack.config() ");
	Serial.println();

	File jetPackConfig = SPIFFS.open("/jetPackConfig.json", "r");

	if (!jetPackConfig) 
	{
		Serial.println("failed to read /jetPackConfig.json ");
		Serial.println();

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
			Serial.println("failed to parse jetpack config json from file ");
			Serial.println();

			return(false);
		} 
		else
		{  	
			Serial.println("read configuration file : ");
			json.printTo(Serial);
			Serial.println();
  
			if(json["ActorsNumber"].success() )
			{
				this->actorsNumber = json["ActorsNumber"]; 
			
				for(int i=0;i<8;i++)
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

						if(json[String("Act")+String(i)]["temporal"].success() )
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
					
					json[String("Act")+String(i)]["actif"]=this->actors[i].actif;
					json[String("Act")+String(i)]["low"]=this->actors[i].low;
					json[String("Act")+String(i)]["high"]=this->actors[i].high;
					json[String("Act")+String(i)]["type"]=this->actors[i].type;
					json[String("Act")+String(i)]["temporal"]=this->actors[i].temporal; 
				}
			}
			else
			{
				this->actorsNumber=this->actorsNumber;
			}
			json["actorsNumber"]=this->actorsNumber;

			jetPackConfig.close();			
			jetPackConfig = SPIFFS.open("/jetPackConfig.json", "w");			
			if(!jetPackConfig)
			{
				Serial.println("failed to write to /jetPackConfig.json ");
				Serial.println();
				
				return(false);			
			}  

			json.printTo(jetPackConfig);
			jetPackConfig.close();
			
			Serial.println("saved configuration : ");
			json.printTo(Serial );
			Serial.println();		
			
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
	Serial.println("Enter Jetpack.printConf() ");
	Serial.println();
	
	Serial.print("actorsNumber : ");
	Serial.println(this->actorsNumber);
 
        for(int i=0;i<this->actorsNumber;i++)
	{	
		Serial.print("actor N°");
		Serial.print(i);
		Serial.print(" actif :");
		Serial.println(this->actors[0].actif);

		Serial.print("actor N°");
		Serial.print(i);
		Serial.print(" low :");
		Serial.println(this->actors[0].low);

		Serial.print("actor N°");
		Serial.print(i);
		Serial.print(" high :");
		Serial.println(this->actors[0].high);

		Serial.print("actor N°");
		Serial.print(i);
		Serial.print(" type :");
		Serial.println(this->actors[0].type);
		
		Serial.print("actor N°");
		Serial.print(i);
		Serial.print(" temporal :");
		Serial.println(this->actors[0].temporal);
 

	}
}
 


