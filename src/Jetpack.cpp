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


#define DEBUG 1


/**
*	Jetpack::begin():
*	This method is provided to
*	initialise the pin that control
*	the Jetpack shield
*/
void Jetpack::begin()
{

#if DEBUG == 1 
 
	Serial.println( F("Entering Jetpack.begin() ") );
	Serial.println();

#endif

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

#if DEBUG == 1

	Serial.println( F("Entering Jetpack.write()") );
	Serial.println();

	Serial.println( F("writing this action : ") );
	Serial.println(action,BIN);
	Serial.println();

#endif 

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

#if DEBUG == 1 

	Serial.println( F("Entering Jetpack.writeBit() ") );

	Serial.print( F("Writing ") );
	Serial.print(state);

	Serial.print( F("to pin N°") );
	Serial.print(pin);

	Serial.println();

#endif

	bitWrite(this->action, pin, state);
	digitalWrite(EnI2C, LOW);
	
	shiftOut(dataPin, clockPin, MSBFIRST, this->action);

	digitalWrite(EnI2C, HIGH);

}

/**
*	Jetpack::doAction(sensor data ):
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
*		root["Temperature"] < actors[0].low
*
*	action: invert the state of actors[0]:
*		bitWrite( action,0,!( bitRead ( action,0 ) ) )
*		write(action)
*	
*/
void Jetpack::doAction( const char* data )
{

#if DEBUG == 1 

	Serial.println( F("Entering Jetpack.doAction()") );
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
					//regular actor
					if( (this->actors[i].inverted) == 0 )
					{
						//measure >= high limit : stop actor
						if( ( root[this->actors[i].primaryType] ) >= ( this->actors[i].rangeHigh ) ) 	
						{	
							bitWrite( this->action , i , 0 ) ;	

						#if DEBUG == 1
							
							Serial.print(F("not inverted Actor N° : "));
							Serial.println(i);

							Serial.print(F("measured value : "));
							Serial.println(root[this->actors[i].primaryType].as<float>());

							Serial.print(F("high range : "));
							Serial.println(this->actors[i].rangeHigh);
						
						#endif
						
						}
						//measure <= low limit : start actor
						else if( ( root[ this->actors[i].primaryType ] ) <= ( this->actors[i].rangeLow ) )
						{
							bitWrite( this->action , i , 1 ) ;

						#if DEBUG == 1

							Serial.print(F("not inverted Actor N° : "));
							Serial.println(i);

							Serial.print(F("measured value : "));
							Serial.println(root[this->actors[i].primaryType].as<float>());

							Serial.print(F("low range : "));
							Serial.println(this->actors[i].rangeLow);
						
						#endif
											
						}
						else 
						{
							bitWrite( this->action , i , 0 ) ;						
						}
					}
					//inverted actor
					else if( (this->actors[i].inverted) == 1 )
					{
						//measure >= high limit : start actor
						if( ( root[this->actors[i].primaryType] ) >= ( this->actors[i].rangeHigh ) ) 	
						{	
							bitWrite( this->action , i , 1 ) ;

						#if DEBUG == 1

							Serial.print(F(" inverted Actor N° : "));
							Serial.println(i);
							
							Serial.print(F("measured value : "));
							Serial.println(root[this->actors[i].primaryType].as<float>());

							Serial.print(F("high range : "));
							Serial.println(this->actors[i].rangeHigh);
						
						#endif
							
						}
						//measure <= low limit : stop actor
						else if( ( root[ this->actors[i].primaryType ] ) <= ( this->actors[i].rangeLow ) )
						{
							bitWrite( this->action , i , 0 ) ;

						#if DEBUG == 1
							
							Serial.print(F("inverted Actor N° : "));
							Serial.println(i);

							Serial.print(F("measured value : "));
							Serial.println(root[this->actors[i].primaryType].as<float>());

							Serial.print(F("low range : "));
							Serial.println(this->actors[i].rangeLow);
						
						#endif
											
						}
						else 
						{
							bitWrite( this->action , i , 0 ) ;						
						}

					
					}
				}

				//if the actor is temporal
				else
				{
					//actor has a secondary type (either hour,minute or hourMinute)
					if( ( this->actors[i].secondaryType ) !="" ) 	
					{
					
					#if DEBUG == 1
						
						Serial.print(this->actors[i].secondaryType);
						Serial.print(" actor N° ");
						Serial.println(i);
						Serial.println();
					#endif
						//secondary type is hour 	
						if( ( this->actors[i].secondaryType=="hour" ) )
						{
							//time >= hourLow : stop actor
							if( ( root[this->actors[i].secondaryType] ) >= ( this->actors[i].hourLow ) ) 	
							{
						
							#if DEBUG == 1 
							
								Serial.print("deactive actor N° ");
								Serial.println(i);
						
							#endif	
								bitWrite( this->action , i , 0 ) ;	
							}
							//time >= hourHigh : start actor
							else if( ( root[ this->actors[i].secondaryType ] ) >= ( this->actors[i].hourHigh ) )
							{
						
							#if DEBUG == 1 
						
								Serial.print("active actor N° ");
								Serial.println(i);
						
							#endif
								bitWrite( this->action , i , 1 ) ;					
							}
						}

						//secondary type is minute 	
						if( ( this->actors[i].secondaryType=="minute" ) )
						{
							//time >= minuteLow : stop actor
							if( ( root[this->actors[i].secondaryType] ) >= ( this->actors[i].minuteLow ) ) 	
							{
						
							#if DEBUG == 1 
							
								Serial.print("deactive actor N° ");
								Serial.println(i);
						
							#endif	
								bitWrite( this->action , i , 0 ) ;	
							}
							//time >= minuteHigh : start actor
							else if( ( root[ this->actors[i].secondaryType ] ) >= ( this->actors[i].minuteHigh ) )
							{
						
							#if DEBUG == 1 
						
								Serial.print("active actor N° ");
								Serial.println(i);
						
							#endif
								bitWrite( this->action , i , 1 ) ;					
							}
						}

						//secondary type is hourMinute 	
						if( ( this->actors[i].secondaryType=="hourMinute" ) )
						{
							//time == hourLow :
							if( ( root["hour"] ) == ( this->actors[i].hourLow ) ) 	
							{
								//time > minuteLow : stop actor
								if( (root["minute"])>=(this->actors[i].minuteLow) )						
								{
								#if DEBUG == 1 
						
									Serial.print(" time.hour == hourLow, time.minute>=minuteLow : deactive actor N° ");
									Serial.println(i);
					
								#endif	
									bitWrite( this->action , i , 0 ) ;
								}	
							}
							//time > hourLow: stop actor
							else if( ( root["hour" ] ) > ( this->actors[i].hourLow ) )
							{
	
							#if DEBUG == 1 
						
								Serial.print("time.hour>hourLow : deactive actor N° ");
								Serial.println(i);
				
							#endif		
								bitWrite( this->action , i , 0 ) ;
													
							}
							//time == hourHigh:
							else if( ( root["hour" ] ) == ( this->actors[i].hourHigh ) )
							{
								//time > minuteHigh: start actor
								if( (root["minute"])>=(this->actors[i].minuteHigh) )
								{
						
								#if DEBUG == 1 
						
									Serial.print("time.hour==hourHigh, time.mintue>=minuteHigh : active actor N° ");
									Serial.println(i);
						
								#endif
									bitWrite( this->action , i , 1 ) ;
								}					
							}
							//time > hourHigh : start actor
							else if( ( root["hour" ] ) > ( this->actors[i].hourHigh ) )
							{
								
							#if DEBUG == 1 
						
								Serial.print("time.hour>hourHigh : active actor N° ");
								Serial.println(i);
				
							#endif		

								bitWrite( this->action , i , 1 ) ;
													
							}

						}


					}
					//actor not of type hour
					else if( ( this->actors[i].secondaryType ) == ( "" ) ) 	 
					{
					
					#if DEBUG == 1 
						
						Serial.println("not hour temporal actor");
						Serial.println(i);
						Serial.println(this->actors[i].secondaryType);
						Serial.println("actifTime : ");
						Serial.println(this->actors[i].actifTime);
						Serial.println("millis : ");
						Serial.println(millis() );
						Serial.println(" high : ");
						Serial.println(this->actors[i].timeHigh );
						Serial.println();
					
					#endif
						//if the actor was actif for highTime or more :
						if( ( millis()- this->actors[i].actifTime  ) >= (  this->actors[i].timeHigh  ) )
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
			}
			//check if actor is inactif
			else if(this->actors[i].actif==0)
			{	//check if actor is temporal
				if(this->actors[i].temporal==1)
				{
					//if the actor was inactif for lowTime or more :
					if( ( millis() - this->actors[i].inactifTime ) >= (  this->actors[i].timeLow  ) )
					{
						//start the actor
						bitWrite( this->action , i , 1) ;

						//make the actor actif:
						this->actors[i].actif=1;

						//start the low timer
						this->actors[i].actifTime=millis();

					#if DEBUG == 1 
						
						Serial.println("inactif temporal actor");
						Serial.println(this->actors[i].primaryType);
						Serial.print("temporal : ");
						Serial.println(this->actors[i].temporal);
						Serial.println(i);
						Serial.println("inactifTime : ");
						Serial.println(this->actors[i].inactifTime);
						Serial.println("millis : ");
						Serial.println(millis() );
						Serial.println(" low : ");
						Serial.println(this->actors[i].timeLow );
						Serial.println();

						Serial.println();
					
					#endif
				
					}			
			
				}
			}
		}
	
	#if DEBUG == 1 

		Serial.println( F("new action is : ") );
		Serial.println(this->action,BIN);
		Serial.println();
	
	#endif 

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

#if DEBUG == 1 

	Serial.println( F("Entering Jetpack.config() ") );
	Serial.println();

#endif

	File jetPackConfig = SPIFFS.open("/jetPackConfig.json", "r");

	if (!jetPackConfig) 
	{

	#if DEBUG == 1 

		Serial.println( F("failed to read /jetPackConfig.json ") );
		Serial.println();

	#endif

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
		
		#if DEBUG == 1 

			Serial.println( F("failed to parse jetpack config json from file ") );
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
  
			for(int i=0;i<8;i++)
			{	
				if(json[String("Act")+String(i)].success())
				{
					//parsing actif key
					if(json[String("Act")+String(i)]["actif"].success() )
					{
						this->actors[i].actif=json[String("Act")+String(i)]["actif"];
					}
					else
					{
						this->actors[i].actif=this->actors[i].actif;
					}
					json[String("Act")+String(i)]["actif"]=this->actors[i].actif;
					
					//parsing temporal key
					if(json[String("Act")+String(i)]["temporal"].success() )
					{
						this->actors[i].temporal=json[String("Act")+String(i)]["temporal"];
					}
					else
					{
						this->actors[i].temporal=this->actors[i].temporal;
					}
					json[String("Act")+String(i)]["temporal"]=this->actors[i].temporal;
					
					//parsing inverted key
					if(json[String("Act")+String(i)]["inverted"].success() )
					{
						this->actors[i].inverted=json[String("Act")+String(i)]["inverted"];
					}
					else
					{
						this->actors[i].inverted=this->actors[i].inverted;
					}
					json[String("Act")+String(i)]["inverted"]=this->actors[i].inverted;

					//parsing inverted key
					if(json[String("Act")+String(i)]["inverted"].success() )
					{
						this->actors[i].inverted=json[String("Act")+String(i)]["inverted"];
					}
					else
					{
						this->actors[i].inverted=this->actors[i].inverted;
					}
					json[String("Act")+String(i)]["inverted"]=this->actors[i].inverted;
					
					//parsing low key
					if(json[String("Act")+String(i)]["low"].success() )
					{
						this->actors[i].rangeLow=json[String("Act")+String(i)]["low"][0];
						this->actors[i].timeLow=json[String("Act")+String(i)]["low"][1];
						this->actors[i].hourLow=json[String("Act")+String(i)]["low"][2];						
						this->actors[i].minuteLow=json[String("Act")+String(i)]["low"][3];						
					}
					else
					{
						this->actors[i].rangeLow=this->actors[i].rangeLow;
						this->actors[i].timeLow=this->actors[i].timeLow;
						this->actors[i].hourLow=this->actors[i].hourLow;
						this->actors[i].minuteLow=this->actors[i].minuteLow;						
					}
					json[String("Act")+String(i)]["low"][0]=this->actors[i].rangeLow;
					json[String("Act")+String(i)]["low"][1]=this->actors[i].timeLow;
					json[String("Act")+String(i)]["low"][2]=this->actors[i].hourLow;
					json[String("Act")+String(i)]["low"][3]=this->actors[i].minuteLow;

					//parsing high key
					if(json[String("Act")+String(i)]["high"].success() )
					{
						this->actors[i].rangeHigh=json[String("Act")+String(i)]["high"][0];
						this->actors[i].timeHigh=json[String("Act")+String(i)]["high"][1];
						this->actors[i].hourHigh=json[String("Act")+String(i)]["high"][2];						
						this->actors[i].minuteHigh=json[String("Act")+String(i)]["high"][3];						
					}
					else
					{
						this->actors[i].rangeHigh=this->actors[i].rangeHigh;
						this->actors[i].timeHigh=this->actors[i].timeHigh;
						this->actors[i].hourHigh=this->actors[i].hourHigh;
						this->actors[i].minuteHigh=this->actors[i].minuteHigh;
					}
					json[String("Act")+String(i)]["high"][0]=this->actors[i].rangeHigh;
					json[String("Act")+String(i)]["high"][1]=this->actors[i].timeHigh;
					json[String("Act")+String(i)]["high"][2]=this->actors[i].hourHigh;
					json[String("Act")+String(i)]["high"][3]=this->actors[i].minuteHigh;

					//parsing type key
					if(json[String("Act")+String(i)]["type"].success() )
					{
						this->actors[i].primaryType=json[String("Act")+String(i)]["type"][0].as<String>();
						this->actors[i].secondaryType=json[String("Act")+String(i)]["type"][1].as<String>();						
						
					}
					else
					{
						this->actors[i].primaryType=this->actors[i].primaryType;
						this->actors[i].secondaryType=this->actors[i].secondaryType;
					}
					json[String("Act")+String(i)]["type"][0]=this->actors[i].primaryType;
					json[String("Act")+String(i)]["type"][1]=this->actors[i].secondaryType;
						


					
					
					 
				}
				else
				{
					this->actors[i]=this->actors[i];
				}
				
				json[String("Act")+String(i)]["actif"]=this->actors[i].actif;
				json[String("Act")+String(i)]["temporal"]=this->actors[i].temporal;
				json[String("Act")+String(i)]["inverted"]=this->actors[i].inverted;

				json[String("Act")+String(i)]["low"][0]=this->actors[i].rangeLow;
				json[String("Act")+String(i)]["low"][1]=this->actors[i].timeLow;
				json[String("Act")+String(i)]["low"][2]=this->actors[i].hourLow;
				json[String("Act")+String(i)]["low"][3]=this->actors[i].minuteLow;

				json[String("Act")+String(i)]["high"][0]=this->actors[i].rangeHigh;
				json[String("Act")+String(i)]["high"][1]=this->actors[i].timeHigh;
				json[String("Act")+String(i)]["high"][2]=this->actors[i].hourHigh;
				json[String("Act")+String(i)]["high"][3]=this->actors[i].minuteHigh;

				json[String("Act")+String(i)]["type"][0]=this->actors[i].primaryType;
				json[String("Act")+String(i)]["type"][1]=this->actors[i].secondaryType;



			}
			

			jetPackConfig.close();			
			jetPackConfig = SPIFFS.open("/jetPackConfig.json", "w");			
			if(!jetPackConfig)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /jetPackConfig.json ") );
				Serial.println();
			
			#endif
				
				return(false);			
			}  

			json.printTo(jetPackConfig);
			jetPackConfig.close();

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
*	Jetpack::printConf():
*	This method is provided to
*	print the configuration to the 
*	Serial Monitor
*/
void Jetpack::printConf()
{

#if DEBUG == 1 

	Serial.println( F("Enter Jetpack.printConf() ") );
	Serial.println();

#endif 
	Serial.println(F( "Jetpack configuration " ) ) ;
 
        for(int i=0;i<8;i++)
	{	
		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" actif :"));
		Serial.println(this->actors[i].actif);
		
		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" temporal :"));
		Serial.println(this->actors[i].temporal);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" inverted :"));
		Serial.println(this->actors[i].inverted);


		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" primary Type :"));
		Serial.println(this->actors[i].primaryType);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" secondary Type :"));		
		Serial.println(this->actors[i].secondaryType);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" range Low :"));
		Serial.println(this->actors[i].rangeLow);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" time Low :"));
		Serial.println(this->actors[i].timeLow);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" hour low:"));
		Serial.println(this->actors[i].hourLow);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" minute low:"));
		Serial.println(this->actors[i].minuteLow);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" range High:"));
		Serial.println(this->actors[i].rangeHigh);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" time High:"));
		Serial.println(this->actors[i].timeHigh);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" hour high:"));
		Serial.println(this->actors[i].hourHigh);

		Serial.print(F("actor N°"));
		Serial.print(i);
		Serial.print(F(" minute high:"));
		Serial.println(this->actors[i].minuteHigh);

		Serial.println(); 

	}

	Serial.println();
}
 


