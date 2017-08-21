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
		
		//check if the actor is actif 
		if(this->actor.actif==1)
		{						
			//if the actor is not temporal
			if( this->actor.temporal==0 ) 
			{	
				//regular actor
				if( (this->actor.inverted) == 0 )
				{
					//measure >= high limit : stop actor
					if( ( root[this->actor.primaryType] ) >= ( this->actor.rangeHigh ) ) 	
					{	
						this->write( 0 ) ;	

					#if DEBUG == 1
						
						Serial.println(F("not inverted Actor "));

						Serial.print(F("measured value : "));
						Serial.println(root[this->actor.primaryType].as<float>());

						Serial.print(F("high range : "));
						Serial.println(this->actor.rangeHigh);
					
					#endif
					
					}
					//measure <= low limit : start actor
					else if( ( root[ this->actor.primaryType ] ) <= ( this->actor.rangeLow ) )
					{
						this->write( 1 ) ;

					#if DEBUG == 1

						Serial.println(F("not inverted Actor "));

						Serial.print(F("measured value : "));
						Serial.println(root[this->actor.primaryType].as<float>());

						Serial.print(F("low range : "));
						Serial.println(this->actor.rangeLow);
					
					#endif
										
					}
					else 
					{
						this->write( 0 ) ;						
					}
				}
				//inverted actor
				else if( (this->actor.inverted) == 1 )
				{
					//measure >= high limit : start actor
					if( ( root[this->actor.primaryType] ) >= ( this->actor.rangeHigh ) ) 	
					{	
						this->write( 1 ) ;

					#if DEBUG == 1

						Serial.println(F(" inverted Actor  "));
						
						Serial.print(F("measured value : "));
						Serial.println(root[this->actor.primaryType].as<float>());

						Serial.print(F("high range : "));
						Serial.println(this->actor.rangeHigh);
					
					#endif
						
					}
					//measure <= low limit : stop actor
					else if( ( root[ this->actor.primaryType ] ) <= ( this->actor.rangeLow ) )
					{
						this->write( 0 ) ;

					#if DEBUG == 1
						
						Serial.print(F("inverted Actor "));
						Serial.println();

						Serial.print(F("measured value : "));
						Serial.println(root[this->actor.primaryType].as<float>());

						Serial.print(F("low range : "));
						Serial.println(this->actor.rangeLow);
					
					#endif
										
					}
					else 
					{
						this->write( 0 ) ;						
					}

				
				}
			}

			//if the actor is temporal
			else
			{
				//actor has a secondary type (either hour,minute or hourMinute)
				if( ( this->actor.secondaryType ) !="" ) 	
				{
				
				#if DEBUG == 1
					
					Serial.print(this->actor.secondaryType);
					Serial.print(" actor ");
					Serial.println();
				#endif
					//secondary type is hour 	
					if( ( this->actor.secondaryType=="hour" ) )
					{
						//time >= hourLow : stop actor
						if( ( root[this->actor.secondaryType] ) >= ( this->actor.hourLow ) ) 	
						{
					
						#if DEBUG == 1 
						
							Serial.print("deactive actor ");
							Serial.println();
					
						#endif	
							this->write( 0 ) ;	
						}
						//time >= hourHigh : start actor
						else if( ( root[ this->actor.secondaryType ] ) >= ( this->actor.hourHigh ) )
						{
					
						#if DEBUG == 1 
					
							Serial.print("active actor ");
							Serial.println();
					
						#endif
							this->write( 1 ) ;					
						}
					}

					//secondary type is minute 	
					if( ( this->actor.secondaryType=="minute" ) )
					{
						//time >= minuteLow : stop actor
						if( ( root[this->actor.secondaryType] ) >= ( this->actor.minuteLow ) ) 	
						{
					
						#if DEBUG == 1 
						
							Serial.print("deactive actor ");
							Serial.println( );
					
						#endif	
							this->write( 0 ) ;	
						}
						//time >= minuteHigh : start actor
						else if( ( root[ this->actor.secondaryType ] ) >= ( this->actor.minuteHigh ) )
						{
					
						#if DEBUG == 1 
					
							Serial.print("active actor  ");
							Serial.println();
					
						#endif
							this->write( 1 ) ;					
						}
					}

					//secondary type is hourMinute 	
					if( ( this->actor.secondaryType=="hourMinute" ) )
					{
						//time == hourLow :
						if( ( root["hour"] ) == ( this->actor.hourLow ) ) 	
						{
							//time > minuteLow : stop actor
							if( (root["minute"])>=(this->actor.minuteLow) )						
							{
							#if DEBUG == 1 
					
								Serial.print(" time.hour == hourLow, time.minute>=minuteLow : deactive actor ");
								Serial.println();
				
							#endif	
								this->write( 0 ) ;
							}	
						}
						//time > hourLow: stop actor
						else if( ( root["hour" ] ) > ( this->actor.hourLow ) )
						{

						#if DEBUG == 1 
					
							Serial.print("time.hour>hourLow : deactive actor ");
							Serial.println();
			
						#endif		
							this->write( 0 ) ;
												
						}
						//time == hourHigh:
						else if( ( root["hour" ] ) == ( this->actor.hourHigh ) )
						{
							//time > minuteHigh: start actor
							if( (root["minute"])>=(this->actor.minuteHigh) )
							{
					
							#if DEBUG == 1 
					
								Serial.print("time.hour==hourHigh, time.mintue>=minuteHigh : active actor ");
								Serial.println();
					
							#endif
								this->write( 1 ) ;
							}					
						}
						//time > hourHigh : start actor
						else if( ( root["hour" ] ) > ( this->actor.hourHigh ) )
						{
							
						#if DEBUG == 1 
					
							Serial.print("time.hour>hourHigh : active actor ");
							Serial.println();
			
						#endif		

							this->write( 1 ) ;
												
						}

					}


				}
				//actor not of type hour
				else if( ( this->actor.secondaryType ) == ( "" ) ) 	 
				{
				
				#if DEBUG == 1 
					
					Serial.println("not hour temporal actor");
					Serial.println();
					Serial.println(this->actor.secondaryType);
					Serial.println("actifTime : ");
					Serial.println(this->actor.actifTime);
					Serial.println("millis : ");
					Serial.println(millis() );
					Serial.println(" high : ");
					Serial.println(this->actor.timeHigh );
					Serial.println();
				
				#endif
					//if the actor was actif for highTime or more :
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
						
			}
		}
		//check if actor is inactif
		else if(this->actor.actif==0)
		{	//check if actor is temporal
			if(this->actor.temporal==1)
			{
				//if the actor was inactif for lowTime or more :
				if( ( millis() - this->actor.inactifTime ) >= (  this->actor.timeLow  ) )
				{
					//start the actor
					this->write( 1 ) ;

					//make the actor actif:
					this->actor.actif=1;

					//start the low timer
					this->actor.actifTime=millis();

				#if DEBUG == 1 
					
					Serial.println("inactif temporal actor");
					Serial.println(this->actor.primaryType);
					Serial.print("temporal : ");
					Serial.println(this->actor.temporal);
					Serial.println(i);
					Serial.println("inactifTime : ");
					Serial.println(this->actor.inactifTime);
					Serial.println("millis : ");
					Serial.println(millis() );
					Serial.println(" low : ");
					Serial.println(this->actor.timeLow );
					Serial.println();

					Serial.println();
				
				#endif
			
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
 


