/**
*	\file CoolBoardActor.cpp
*	\brief CoolBoardActor Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/

#include "FS.h"
#include "Arduino.h"
#include "ArduinoJson.h"

#include "CoolBoardActor.h"


#define DEBUG 0


/**
*	CoolBoardActor::begin():
*	This method is provided to
*	initialise the CoolBoardActor pin
*/
void CoolBoardActor::begin()
{

#if DEBUG == 1 
 
	Serial.println( F("Entering CoolBoardActor.begin() ") );
	Serial.println();

#endif

	pinMode(this->pin,OUTPUT);
	
	

}


/**
*	CoolBoardActor::write(action):
*	This method is provided to write
*	the given action to the CoolBoardActor.
*
*/
void CoolBoardActor::write(bool action)
{

#if DEBUG == 1

	Serial.println( F("Entering CoolBoardActor.write()") );
	Serial.println();

	Serial.println( F("writing this action : ") );
	Serial.println(action,BIN);
	Serial.println();

#endif 
	
	digitalWrite(this->pin,action);
	

}
	
/**
*	CoolBoardActor::doAction(sensor data ):
*	This method is provided to automate the CoolBoardActor.
*	exemple:
*	initial state:
*		current Temperature = 23 °C
*		actor.actif=1
*		actor.rangeLow=25 °C
*		actor.rangeHigh=30 °C
*		actor.primaryType="Temperature"
*		
*	condition verified:		
*		root["Temperature"] < actor.rangeLow
*
*	action : activate the actor 
*
*
*	initial state:
*		actor.actif=1
*		actor.rangeLow=2°C
*		actor.rangeHigh=12°C
*		actor.inverted=1
*		actor.primaryType="Temperature"
*		
*	condition verified:		
*		root["Temperature"] > actor.rangeHigh
*
*	action: activate the actor
*
*
*	initial state:
*		actor.actif=1
*		actor.timeLow=2500ms
*		actor.timeHigh=3000ms
*		actor.temporal=1
*		
*	condition verified:		
*		millis()-actor.actifTime >=actor.timeHigh
*
*	action: deactivate the actor 
*
*
*	initial state:
*		actor.actif=1
*		actor.hourLow=10
*		actor.hourHigh=8
*		actor.temporal=1
*		actor.secondaryType="hour"( or "minute" or "hourMinute")
*		
*	condition verified:		
*		root["hour"]>=actorhourHigh
*
*	action: activate the actor 
*
*
*/
void CoolBoardActor::doAction( const char* data )
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolBoardActor.doAction()") );
	Serial.println();

	Serial.println( F("input data is :") );
	Serial.println(data);
	Serial.println();

#endif 

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(data);
	
	if (!root.success()) 
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to parse json object ") );
		Serial.println();
	
	#endif 

	}
	else
	{
	
	#if DEBUG == 1 

		Serial.println( F("created Json object :") );
		root.printTo(Serial);
		Serial.println();

		Serial.print(F("jsonBuffer size: "));
		Serial.println(jsonBuffer.size());
		Serial.println();

	
	#endif 

		//invert the current action state for the actor
		//if the value is outside the limits
		
		//check if actor is actif
		if(this->actor.actif==1)
		{
			//normal actor
			if(this->actor.temporal == 0)
			{
				//not inverted actor
				if(this->actor.inverted==0)
				{
					this->normalAction(root[this->actor.primaryType]);				
				}
				//inverted actor
				else if(this->actor.inverted==1)
				{
					this->invertedAction(root[this->actor.primaryType]);				
				}
			}
			//temporal actor
			else if(this->actor.temporal == 1 )
			{
				//hour actor
				if(this->actor.secondaryType=="hour")
				{
					//mixed hour actor
					if(root[this->actor.primaryType].success() )
					{
						this->mixedHourAction(root[this->actor.secondaryType],root[this->actor.primaryType]);
					}
					//normal hour actor
					else
					{
						this->hourAction(root[this->actor.secondaryType]);
					}
				
				}
				//minute actor
				else if(this->actor.secondaryType=="minute")
				{
					//mixed minute actor
					if(root[this->actor.primaryType].success() )
					{
						this->mixedMinuteAction(root[this->actor.secondaryType],root[this->actor.primaryType]);
					}
					//normal minute actor
					else
					{
						this->minuteAction(root[this->actor.secondaryType]);
					}
				}
				//hourMinute actor
				else if(this->actor.secondaryType=="hourMinute")
				{
					//mixed hourMinute actor
					if(root[this->actor.primaryType].success() )
					{
						this->mixedHourMinuteAction(root["hour"],root["minute"],root[this->actor.primaryType]);
					}
					//normal hourMinute actor
					else
					{
						this->hourMinuteAction(root["hour"],root["minute"]);
					}
				}
				//normal temporal actor
				else if(this->actor.secondaryType=="")
				{
					//mixed temporal actor
					if(root[this->actor.primaryType].success() )
					{
						this->mixedTemporalActionOff(root[this->actor.primaryType]);
					}
					//normal temporal actor
					else
					{
						this->temporalActionOff();
					}
										
				}

			}
		}
		//inactif actor
		else if(this->actor.actif == 0 )
		{
			//temporal actor
			if(this->actor.temporal==1)
			{
				//mixed temporal actor
				if(root[this->actor.primaryType].success() )
				{
					this->mixedTemporalActionOn(root[this->actor.primaryType]);
				}
				//normal temporal actor
				else
				{
					this->temporalActionOn();
				}
			}			
		}

	} 
}

/**
*	CoolBoardActor::config():
*	This method is provided to configure the
*	CoolBoardActor with a configuration file
*
*	\return true if successful,false otherwise
*/ 
bool CoolBoardActor::config()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolBoardActor.config() ") );
	Serial.println();

#endif

	File coolBoardActorConfig = SPIFFS.open("/coolBoardActorConfig.json", "r");

	if (!coolBoardActorConfig) 
	{

	#if DEBUG == 1 

		Serial.println( F("failed to read /coolBoardActorConfig.json ") );
		Serial.println();

	#endif

		return(false);
	}
	else
	{
		size_t size = coolBoardActorConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		coolBoardActorConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
		
		#if DEBUG == 1 

			Serial.println( F("failed to parse coolBoardActor Config  json from file ") );
			Serial.println();

		#endif

			return(false);
		} 
		else
		{ 
 		
		#if DEBUG == 1 

			Serial.println( F("read configuration file : ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();

		
		#endif
  
			//parsing actif key
			if(json["actif"].success() )
			{
				this->actor.actif=json["actif"];
			}
			else
			{
				this->actor.actif=this->actor.actif;
			}
			json["actif"]=this->actor.actif;
			
			//parsing temporal key
			if(json["temporal"].success() )
			{
				this->actor.temporal=json["temporal"];
			}
			else
			{
				this->actor.temporal=this->actor.temporal;
			}
			json["temporal"]=this->actor.temporal;
			
			//parsing inverted key
			if(json["inverted"].success() )
			{
				this->actor.inverted=json["inverted"];
			}
			else
			{
				this->actor.inverted=this->actor.inverted;
			}
			json["inverted"]=this->actor.inverted;

			//parsing inverted key
			if(json["inverted"].success() )
			{
				this->actor.inverted=json["inverted"];
			}
			else
			{
				this->actor.inverted=this->actor.inverted;
			}
			json["inverted"]=this->actor.inverted;
			
			//parsing low key
			if(json["low"].success() )
			{
				this->actor.rangeLow=json["low"][0];
				this->actor.timeLow=json["low"][1];
				this->actor.hourLow=json["low"][2];						
				this->actor.minuteLow=json["low"][3];						
			}
			else
			{
				this->actor.rangeLow=this->actor.rangeLow;
				this->actor.timeLow=this->actor.timeLow;
				this->actor.hourLow=this->actor.hourLow;
				this->actor.minuteLow=this->actor.minuteLow;						
			}
			json["low"][0]=this->actor.rangeLow;
			json["low"][1]=this->actor.timeLow;
			json["low"][2]=this->actor.hourLow;
			json["low"][3]=this->actor.minuteLow;

			//parsing high key
			if(json["high"].success() )
			{
				this->actor.rangeHigh=json["high"][0];
				this->actor.timeHigh=json["high"][1];
				this->actor.hourHigh=json["high"][2];						
				this->actor.minuteHigh=json["high"][3];						
			}
			else
			{
				this->actor.rangeHigh=this->actor.rangeHigh;
				this->actor.timeHigh=this->actor.timeHigh;
				this->actor.hourHigh=this->actor.hourHigh;
				this->actor.minuteHigh=this->actor.minuteHigh;
			}
			json["high"][0]=this->actor.rangeHigh;
			json["high"][1]=this->actor.timeHigh;
			json["high"][2]=this->actor.hourHigh;
			json["high"][3]=this->actor.minuteHigh;

			//parsing type key
			if(json["type"].success() )
			{
				this->actor.primaryType=json["type"][0].as<String>();
				this->actor.secondaryType=json["type"][1].as<String>();						
				
			}
			else
			{
				this->actor.primaryType=this->actor.primaryType;
				this->actor.secondaryType=this->actor.secondaryType;
			}
			json["type"][0]=this->actor.primaryType;
			json["type"][1]=this->actor.secondaryType;
			

			coolBoardActorConfig.close();			
			coolBoardActorConfig = SPIFFS.open("/coolBoardActorConfig.json", "w");			
			if(!coolBoardActorConfig)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /coolBoardActorConfig.json ") );
				Serial.println();
			
			#endif
				
				return(false);			
			}  

			json.printTo(coolBoardActorConfig);
			coolBoardActorConfig.close();

		#if DEBUG == 1 
			
			Serial.println(F("saved configuration : "));
			json.printTo(Serial );
			Serial.println();		
		
		#endif

			return(true); 
		}
	}	
	

}

/**
*	CoolBoardActor::printConf():
*	This method is provided to
*	print the configuration to the 
*	Serial Monitor
*/
void CoolBoardActor::printConf()
{

#if DEBUG == 1 

	Serial.println( F("Enter CoolBoardActor.printConf() ") );
	Serial.println();

#endif 
	Serial.println(F( "CoolBoardActor configuration " ) ) ;
 
	Serial.print(F(" actif :"));
	Serial.println(this->actor.actif);
	

	Serial.print(F(" temporal :"));
	Serial.println(this->actor.temporal);


	Serial.print(F(" inverted :"));
	Serial.println(this->actor.inverted);



	Serial.print(F(" primary Type :"));
	Serial.println(this->actor.primaryType);

	Serial.print(F(" secondary Type :"));		
	Serial.println(this->actor.secondaryType);


	Serial.print(F(" range Low :"));
	Serial.println(this->actor.rangeLow);


	Serial.print(F(" time Low :"));
	Serial.println(this->actor.timeLow);


	Serial.print(F(" hour low:"));
	Serial.println(this->actor.hourLow);


	Serial.print(F(" minute low:"));
	Serial.println(this->actor.minuteLow);


	Serial.print(F(" range High:"));
	Serial.println(this->actor.rangeHigh);


	Serial.print(F(" time High:"));
	Serial.println(this->actor.timeHigh);


	Serial.print(F(" hour high:"));
	Serial.println(this->actor.hourHigh);


	Serial.print(F(" minute high:"));
	Serial.println(this->actor.minuteHigh);

	Serial.println(); 

}
 
void CoolBoardActor::temporalActionOff()
{
	if( ( millis()- this->actor.actifTime  ) >= (  this->actor.timeHigh  ) )
		{
			//stop the actor
			this->write( 0) ;

			//make the actor inactif:
			this->actor.actif=0;

			//start the low timer
			this->actor.inactifTime=millis();				
		}
}


void CoolBoardActor::temporalActionOn()
{	
	 if( ( millis() - this->actor.inactifTime ) >= (  this->actor.timeLow  ) )
		{
			//start the actor
			this->write(  1) ;

			//make the actor actif:
			this->actor.actif=1;

			//start the low timer
			this->actor.actifTime=millis();
		}
}

void CoolBoardActor::hourAction( int hour)
{
	//starting the actor
	if(hour >= this->actor.hourHigh)
	{
		this->write(1) ;
	}
	//stop the actor	
	else if(hour >= this->actor.hourLow)
	{
		this->write( 0) ;
	}
}

void CoolBoardActor::minuteAction(int minute)
{
	//starting the actor
	if(minute >= this->actor.minuteHigh)
	{
		this->write(1) ;
	}
	//stop the actor	
	else if(minute >= this->actor.minuteLow)
	{
		this->write( 0) ;
	}

} 

void CoolBoardActor::hourMinuteAction(int hour,int minute)
{
	//start the actor
	if(hour>=this->actor.hourHigh)
	{
		if(minute>= this->actor.minuteHigh)
		{
			this->write( 1) ;
		}
	}
	//stop the actor
	else if(hour>=this->actor.hourLow)
	{
		if(minute>= this->actor.minuteLow)
		{
			this->write( 0) ;
		}
	}
	
}




