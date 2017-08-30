/**
*	\file	CoolBoardActor.cpp
*  	\brief	CoolBoardActor Source file
*	\version 1.0  
*	\author	Mehdi Zemzem
*	\version 0.0  
*	\author Simon Juif
*  	\date	27/06/2017
*	\copyright La Cool Co SAS 
*	\copyright MIT license
*	Copyright (c) 2017 La Cool Co SAS
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
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
*
*	The result action is the result of
*	checking the different flags of the actor
*	(actif , temporal ,inverted, primaryType
*	and secondaryType ) and the corresponding
*	call to the appropriate helping method
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
					this->normalAction(root[this->actor.primaryType].as<float>());				
				}
				//inverted actor
				else if(this->actor.inverted==1)
				{
					this->invertedAction(root[this->actor.primaryType].as<float>());				
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
						this->mixedHourAction(root[this->actor.secondaryType].as<int>(),root[this->actor.primaryType].as<float>());
					}
					//normal hour actor
					else
					{
						this->hourAction(root[this->actor.secondaryType].as<int>());
					}
				
				}
				//minute actor
				else if(this->actor.secondaryType=="minute")
				{
					//mixed minute actor
					if(root[this->actor.primaryType].success() )
					{
						this->mixedMinuteAction(root[this->actor.secondaryType].as<int>(),root[this->actor.primaryType].as<float>());
					}
					//normal minute actor
					else
					{
						this->minuteAction(root[this->actor.secondaryType].as<int>());
					}
				}
				//hourMinute actor
				else if(this->actor.secondaryType=="hourMinute")
				{
					//mixed hourMinute actor
					if(root[this->actor.primaryType].success() )
					{
						this->mixedHourMinuteAction(root["hour"].as<int>(),root["minute"].as<int>(),root[this->actor.primaryType].as<float>());
					}
					//normal hourMinute actor
					else
					{
						this->hourMinuteAction(root["hour"].as<int>(),root["minute"].as<int>());
					}
				}
				//normal temporal actor
				else if(this->actor.secondaryType=="")
				{
					//mixed temporal actor
					if(root[this->actor.primaryType].success() )
					{
						this->mixedTemporalActionOn(root[this->actor.primaryType].as<float>());
					}
					//normal temporal actor
					else
					{
						this->temporalActionOn();
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
					this->mixedTemporalActionOff(root[this->actor.primaryType].as<float>());
				}
				//normal temporal actor
				else
				{
					this->temporalActionOff();
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
 
/**
*	CoolBoardActor::normalAction( measured value):
*	This method is provided to
*	handle normal actors.
*	it changes the action according to wether the
*	measured value is: > rangeHigh ( deactivate actor)
*	or < rangeLow (activate actor )
*/
void CoolBoardActor::normalAction(float measurment)
{

#if DEBUG == 1
	
	Serial.print(F("none inverted Actor"));
	Serial.println();

	Serial.print(F("measured value : "));
	Serial.println(measurment);

	Serial.print(F("high range : "));
	Serial.println(this->actor.rangeHigh);

	Serial.print(F("low range : "));
	Serial.println(this->actor.rangeLow);

#endif

	//measured value lower than minimum range : activate actor
	if(measurment < this->actor.rangeLow)
	{
		this->write( 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));s
	
	#endif
				
	}
	//measured value higher than maximum range : deactivate actor
	else if(measurment > this->actor.rangeHigh)
	{
		this->write( 0) ;

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));
	
	#endif
	
	}


}


/**
*	CoolBoardActor::invertedAction( measured value):
*	This method is provided to
*	handle inverted actors.
*	it changes the action according to wether the
*	measured value is:
*	> rangeHigh (activate actor)
*	< rangeLow ( deactivate actor )
*/
void CoolBoardActor::invertedAction(float measurment)
{
#if DEBUG == 1
	
	Serial.print("inverted Actor ");
	Serial.println();

	Serial.print("measured value : ");
	Serial.println(measurment);

	Serial.print("high range : ");
	Serial.println(this->actor.rangeHigh);

	Serial.print("low range : ");
	Serial.println(this->actor.rangeLow);

#endif

	//measured value lower than minimum range : deactivate actor
	if(measurment < this->actor.rangeLow)
	{
		this->write( 0) ;

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));
	
	#endif

	}
	//measured value higher than maximum range : activate actor
	else if(measurment > this->actor.rangeHigh)
	{
		this->write( 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));
	
	#endif

	}


}

/**
*	CoolBoardActor::temporalActionOff( ):
*	This method is provided to
*	handle temporal actors.
*	it changes the action according to:
*	
*	currentTime - startTime > timeHigh : deactivate actor 
*
*/
void CoolBoardActor::temporalActionOff( )
{

#if DEBUG == 1
	
	Serial.print(F("temporal Actor  "));
	Serial.println();

	Serial.print(F("millis : "));
	Serial.println(millis());

	Serial.print(F("actif Time : "));
	Serial.println(this->actor.actifTime);

	Serial.print(F("high time : "));
	Serial.println(this->actor.timeHigh);


#endif
	
	if( ( millis()- this->actor.actifTime  ) >= (  this->actor.timeHigh  ) )
	{
		//stop the actor
		this->write( 0) ;

		//make the actor inactif:
		this->actor.actif=0;

		//start the low timer
		this->actor.inactifTime=millis();

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));
	
	#endif
				
	}	
}


/**
*	CoolBoardActor::mixedTemporalActionOff( measured value ):
*	This method is provided to
*	handle mixed temporal actors.
*	it changes the action according to:
*	
*	currentTime - startTime >= timeHigh :  
*		measured value >= rangeHigh : deactivate actor
*		measured value < rangeHigh : activate actor
*/
void CoolBoardActor::mixedTemporalActionOff( float measurment)
{

#if DEBUG == 1
	
	Serial.print("mixed Temporal Actor N° : ");
	Serial.println();

	Serial.print("measured value : ");
	Serial.println(measurment);

	Serial.print("high range : ");
	Serial.println(this->actor.rangeHigh);

	Serial.print("time high : ");
	Serial.println(this->actor.timeHigh);

	Serial.print("actif Time : ");
	Serial.println(this->actor.actifTime);

	Serial.print(F("millis : "));
	Serial.println(millis());

#endif
	if( ( millis()- this->actor.actifTime  ) >= (  this->actor.timeHigh  ) )
	{	
		if( measurment >= this->actor.rangeHigh )
		{
			//stop the actor
			this->write( 0) ;

			//make the actor inactif:
			this->actor.actif=0;

			//start the low timer
			this->actor.inactifTime=millis();

		#if DEBUG == 1 

			Serial.print(F("actor was on for at least "));
			Serial.print(this->actor.timeHigh);
			Serial.println(F(" ms "));

			Serial.print(measurment);
			Serial.print(F(" > " ));
			Serial.println(this->actor.rangeHigh);

			
			Serial.println(F("actor OFF "));

		#endif

		}
		else 
		{
			this->write( 1) ;

		#if DEBUG == 1 
			
			Serial.print(F("actor was on for at least "));
			Serial.print(this->actor.timeHigh);
			Serial.println(F(" ms "));

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actor.rangeHigh);

			Serial.println(F("actor ON "));

		#endif				

		}			
	}

}


/**
*	CoolBoardActor::temporalActionOn( ):
*	This method is provided to
*	handle temporal actors.
*	it changes the action according to :
*
*	currentTime - stopTime > timeLow : activate actor 
*
*/
void CoolBoardActor::temporalActionOn( )
{

#if DEBUG == 1
	
	Serial.print(F("temporal Actor "));
	Serial.println();

	Serial.print(F("millis : "));
	Serial.println(millis());

	Serial.print(F("inactif Time : "));
	Serial.println(this->actor.inactifTime);

	Serial.print(F("low time : "));
	Serial.println(this->actor.timeLow);


#endif
	
	 if( ( millis() - this->actor.inactifTime ) >= (  this->actor.timeLow  ) )
	{
		//start the actor
		this->write( 1) ;

		//make the actor actif:
		this->actor.actif=1;

		//start the low timer
		this->actor.actifTime=millis();

	#if DEBUG == 1 

		Serial.println(F("actor ON "));

	#endif				

	}

}

/**
*	CoolBoardActor::mixedTemporalActionOn( measured value ):
*	This method is provided to
*	handle mixed temporal actors.
*	it changes the action according to :
*
*	currentTime - stopTime > timeLow :   
*		measured value >= rangeLow : deactivate actor
*		measured value < rangeLow : activate actor
*
*/
void CoolBoardActor::mixedTemporalActionOn( float measurment)
{

#if DEBUG == 1
	
	Serial.print("mixed Temporal Actor  ");
	Serial.println();

	Serial.print("measured value : ");
	Serial.println(measurment);

	Serial.print("low range : ");
	Serial.println(this->actor.rangeLow);

	Serial.print("time low : ");
	Serial.println(this->actor.timeLow);

	Serial.print("inactif Time : ");
	Serial.println(this->actor.inactifTime);

	Serial.print(F("millis : "));
	Serial.println(millis());

#endif

	if( ( millis() - this->actor.inactifTime ) >= (  this->actor.timeLow  ) )
	{
		if( measurment < this->actor.rangeLow )
		{
			//start the actor
			this->write( 1) ;

			//make the actor actif:
			this->actor.actif=1;

			//start the low timer
			this->actor.actifTime=millis();

		#if DEBUG == 1 

			Serial.print(F("actor was off for at least "));
			Serial.print(this->actor.timeLow);
			Serial.println(F(" ms "));

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actor.rangeLow);
	
			Serial.println(F("actor ON "));
	
		#endif	

		}
		else 
		{
			this->write( 0) ;	

		#if DEBUG == 1 

			Serial.print(F("actor was off for at least "));
			Serial.print(this->actor.timeLow);
			Serial.println(F(" ms "));

			Serial.print(measurment);
			Serial.print(F(" > " ));
			Serial.println(this->actor.rangeLow);

			Serial.println(F("actor OFF "));
	
		#endif				

		}

	}

	
}


/**
*	CoolBoardActor::hourAction( current hour ):
*	This method is provided to
*	handle hour actors.
*	it changes the action according to:
*	
*	hour >= hourLow : deactivate the actor
*	hour >= hourHigh : activate the actor 
*
*/
void CoolBoardActor::hourAction( int hour)
{

#if DEBUG == 1
	
	Serial.print(F("hour Actor "));
	Serial.println();

	Serial.print(F(" hour : "));
	Serial.println(hour);

	Serial.print(F("high hour : "));
	Serial.println(this->actor.hourHigh);

	Serial.print(F("low hour : "));
	Serial.println(this->actor.hourLow);

#endif

	//stop the actor	
	if(hour >= this->actor.hourLow)
	{
		this->write( 0) ;

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));

	#endif	

	}
	//starting the actor
	else if(hour >= this->actor.hourHigh)
	{
		this->write( 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));

	#endif	
	
	}

}


/**
*	CoolBoardActor::mixedHourAction( current hour, measured value ):
*	This method is provided to
*	handle mixed hour actors.
*	it changes the action according to :
*
*	hour >= hourLow :
*		-measuredValue >= rangeHigh : deactivate actor
*		-measured < rangeHigh : activate actor
*
*	hour >= hourHigh :
*		-measuredValue < rangeLow : activate actor
*		-measuredValue >=rangeLow : activate actor
*/
void CoolBoardActor::mixedHourAction(int hour, float measurment)
{

#if DEBUG == 1
	
	Serial.print("mixed hour Actor N° : ");
	Serial.println();

	Serial.print(" hour : ");
	Serial.println(hour);

	Serial.print("high hour : ");
	Serial.println(this->actor.hourHigh);

	Serial.print("low hour : ");
	Serial.println(this->actor.hourLow);

	Serial.print("measured value : ");
	Serial.println(measurment);

	Serial.print("high range : ");
	Serial.println(this->actor.rangeHigh);

	Serial.print("low range : ");
	Serial.println(this->actor.rangeLow);

#endif
	//stop the actor	
	if(hour >= this->actor.hourLow)
	{
			if( measurment >= this->actor.rangeHigh )
			{
				this->write( 0) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" > " ));
				Serial.println(this->actor.rangeHigh);

				Serial.println(F("actor OFF "));

			#endif	

			}
			else 
			{
				this->write( 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.print(this->actor.rangeHigh);

				Serial.println(F("actor ON "));

			#endif	
				
			}
	}
	//starting the actor
	else if(hour >= this->actor.hourHigh)
	{
			if( measurment < this->actor.rangeLow )
			{
				this->write( 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.println(this->actor.rangeLow);

				Serial.println(F("actor ON "));

			#endif	
			}
			else 
			{
				this->write( 0) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" > " ));
				Serial.println(this->actor.rangeLow);

				Serial.println(F("actor OFF "));

			#endif					
			}

	}

}

/**
*	CoolBoardActor::minteAction( current minute ):
*	This method is provided to
*	handle minute actors.
*	it changes the action according to:
*	
*	minute >= minuteLow : deactivate the actor
*	minute >= minuteHigh : activate the actor 
*
*/
void CoolBoardActor::minuteAction(int minute)
{

#if DEBUG == 1
	
	Serial.print(F("minute Actor "));
	Serial.println();

	Serial.print(F(" minute : "));
	Serial.println(minute);

	Serial.print(F("high minute : "));
	Serial.println(this->actor.minuteHigh);

	Serial.print(F("low minute : "));
	Serial.println(this->actor.minuteLow);

#endif

	//stop the actor	
	if(minute >= this->actor.minuteLow)
	{
		this->write( 0) ;

	#if DEBUG == 1 

		Serial.println(F("actor OFF "));

	#endif	

	}	
	//starting the actor
	else if(minute >= this->actor.minuteHigh)
	{
		this->write( 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));

	#endif	

	}

} 

/**
*	CoolBoardActor::mixedMinuteAction( current minute, measured value ):
*	This method is provided to
*	handle mixed minute actors.
*	it changes the action according to :
*
*	minute >= minuteLow :
*		-measuredValue >= rangeHigh : deactivate actor
*		-measured < rangeHigh : activate actor
*
*	minute >= minuteHigh :
*		-measuredValue < rangeLow : activate actor
*		-measuredValue >=rangeLow : activate actor
*/
void CoolBoardActor::mixedMinuteAction(int minute,float measurment)
{

#if DEBUG == 1
	
	Serial.print("mixed minute Actor ");
	Serial.println();

	Serial.print(" minute : ");
	Serial.println(minute);

	Serial.print("high minute : ");
	Serial.println(this->actor.minuteHigh);

	Serial.print("low minute : ");
	Serial.println(this->actor.minuteLow);

	Serial.print("measured value : ");
	Serial.println(measurment);

	Serial.print("high range : ");
	Serial.println(this->actor.rangeHigh);

	Serial.print("low range : ");
	Serial.println(this->actor.rangeLow);

#endif
	//stop the actor	
	if(minute >= this->actor.minuteLow)
	{
			if( measurment > this->actor.rangeHigh )
			{
				this->write( 0) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" > " ));
				Serial.println(this->actor.rangeHigh);

				Serial.println(F("actor OFF "));

			#endif
	
			}
			else 
			{
				this->write( 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.println(this->actor.rangeHigh);

				Serial.println(F("actor ON "));

			#endif	
				
			}
	}	
	//starting the actor
	else if(minute >= this->actor.minuteHigh)
	{
			if( measurment < this->actor.rangeLow )
			{
				this->write( 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.println(this->actor.rangeLow);

				Serial.println(F("actor ON "));

			#endif	

			}
			else 
			{
				this->write( 0) ;
			
			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" > " ));
				Serial.println(this->actor.rangeLow);

				Serial.println(F("actor OFF "));

			#endif	
				
			}

	}

}


/**
*	CoolBoardActor::minteAction( current hour,current minute ):
*	This method is provided to
*	handle hour minute actors.
*	it changes the action according to:
*	
*	hour == hourLow : 
*		minute >= minuteLow : deactivate the actor
*
*	hour >  hourLow : deactivate the actor
*
*	hour == hourHigh : 
*		minute >= minteHigh : activate the actor 
*
*	hour >  hourHigh : activate the actor
*/
void CoolBoardActor::hourMinuteAction(int hour,int minute)
{

#if DEBUG == 1

	Serial.print(F("hourMinute Actor "));
	Serial.println();

	Serial.print(F(" hour : "));
	Serial.println(hour);
	Serial.print(F(" minute : "));
	Serial.println(minute);

	Serial.print(F("high hour : "));
	Serial.println(this->actor.hourHigh);

	Serial.print(F("high minute : "));
	Serial.println(this->actor.minuteHigh);

	Serial.print(F("low hour : "));
	Serial.println(this->actor.hourLow);

	Serial.print(F("low minute : "));
	Serial.println(this->actor.minuteLow);

#endif
	//stop the actor
	if(hour==this->actor.hourLow)
	{
		if(minute>= this->actor.minuteLow)
		{
			this->write( 0) ;
		#if DEBUG == 1 

			Serial.println(F("actor OFF "));

		#endif	
		}
	}
	else if(hour > this->actor.hourLow)
	{

		this->write( 0) ;
	#if DEBUG == 1 

		Serial.println(F("actor OFF "));

	#endif	
	
	}
	//start the actor
	else if(hour==this->actor.hourHigh)
	{
		if(minute>= this->actor.minuteHigh)
		{
			this->write( 1) ;

		#if DEBUG == 1 

			Serial.println(F("actor ON "));

		#endif	
		}
	}
	else if(hour > this->actor.hourHigh)
	{

		this->write( 1) ;

	#if DEBUG == 1 

		Serial.println(F("actor ON "));

	#endif		

	}

	
}

/**
*	CoolBoardActor::minteAction( current hour,current minute , measured Value ):
*	This method is provided to
*	handle hour minute actors.
*	it changes the action according to:
*	
*	hour == hourLow : 
*		minute >= minuteLow : 
*			measuredValue >= rangeHigh : deactivate actor
*			measuredValue < rangeHigh : activate actor
*	
*	hour >  hourLow : 
*		measuredValue >= rangeHigh : deactivate actor
*		measuredValue < rangeHigh : activate actor
*
*	hour == hourHigh : 
*		minute >= minteHigh : 
*			measuredValue >= rangeLow : deactivate actor
*			measuredValue < rangeLow : activate actor 
*
*	hour >  hourHigh :
*		measuredValue >= rangeLow : deactivate actor
*		measuredValue < rangeLow : activate actor 
*
*/
void CoolBoardActor::mixedHourMinuteAction(int hour,int minute ,float measurment)
{

#if DEBUG == 1
	
	Serial.print("hourMinute Actor  ");
	Serial.println();

	Serial.print(" hour : ");
	Serial.println(hour);
	Serial.print(" minute : ");
	Serial.println(minute);

	Serial.print("high hour : ");
	Serial.println(this->actor.hourHigh);

	Serial.print("high minute : ");
	Serial.println(this->actor.minuteHigh);

	Serial.print("low hour : ");
	Serial.println(this->actor.hourLow);

	Serial.print("low minute : ");
	Serial.println(this->actor.minuteLow);

	Serial.print("measured value : ");
	Serial.println(measurment);

	Serial.print("high range : ");
	Serial.println(this->actor.rangeHigh);

	Serial.print("low range : ");
	Serial.println(this->actor.rangeLow);

#endif
	//stop the actor
	if(hour==this->actor.hourLow)
	{
		if(minute>= this->actor.minuteLow)
		{
			if( measurment >= this->actor.rangeHigh )
			{
				this->write( 0) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" >= " ));
				Serial.println(this->actor.rangeHigh);

				Serial.println(F("actor OFF "));

			#endif	

			}
			else 
			{
				this->write( 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.println(this->actor.rangeHigh);

				Serial.println(F("actor ON "));

			#endif	
				
			}
		}
	}
	else if(hour > this->actor.hourLow)
	{

		if( measurment >= this->actor.rangeHigh )
		{
			this->write( 0) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.print(F(" >= " ));
			Serial.println(this->actor.rangeHigh);

			Serial.println(F("actor OFF "));

		#endif	

		}
		else 
		{
			this->write( 1) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actor.rangeHigh);

			Serial.println(F("actor ON "));

		#endif	
			
		}


	}
	//start the actor
	else if(hour==this->actor.hourHigh)
	{
		if(minute>= this->actor.minuteHigh)
		{
			if( measurment < this->actor.rangeLow )
			{
				this->write( 1) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.print(F(" < " ));
				Serial.println(this->actor.rangeLow);

				Serial.println(F("actor ON "));

			#endif	

			}
			else 
			{
				this->write( 0) ;

			#if DEBUG == 1 

				Serial.print(measurment);
				Serial.println(F(" > " ));
				Serial.print(this->actor.rangeLow);

				Serial.println(F("actor OFF "));

			#endif	
				
			}
		}
	}
	else if(hour > this->actor.hourHigh)
	{

		if( measurment < this->actor.rangeLow )
		{
			this->write( 1) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.print(F(" < " ));
			Serial.println(this->actor.rangeLow);

			Serial.println(F("actor ON "));

		#endif	

		}
		else 
		{
			this->write( 0) ;

		#if DEBUG == 1 

			Serial.print(measurment);
			Serial.println(F(" > " ));
			Serial.print(this->actor.rangeLow);

			Serial.println(F("actor OFF "));

		#endif	
			
		}
	
	}

}


