/**
*	\file Jetpack.h
*	\brief Jetpack Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/

#ifndef Jetpack_H
#define Jetpack_H

#include "Arduino.h"
#define DEBUG 1

/**
*	\class Jetpack
*	\brief This class manages the Jetpack shield
*
*/
class Jetpack
{
public:

	void begin();		       //starts the Jetpack

 	void write(byte action);			//writes to the Jetpack
 
	void writeBit(byte pin,bool state); //writes to a single pin of the Jetpack
 
	void doAction(const char* data );

	template <typename T> 
	void normalAction(int actorNumber,T measurment)
	{
	
		//measured value lower than minimum range : activate actor
		if(measurment < this->actors[actorNumber].rangeLow)
		{
			bitWrite( this->action , actorNumber , 1) ;		
		}
		//measured value higher than maximum range : deactivate actor
		else if(measurment > this->actors[actorNumber].rangeHigh)
		{
			bitWrite( this->action , actorNumber , 0) ;	
		}
	#if DEBUG == 1
		
		Serial.print("none inverted Actor N° : ");
		Serial.println(actorNumber);

		Serial.print("measured value : ");
		Serial.println(measurment);

		Serial.print("high range : ");
		Serial.println(this->actors[actorNumber].rangeHigh);

		Serial.print("low range : ");
		Serial.println(this->actors[actorNumber].rangeLow);
	
	#endif
	
	}

	template <typename T> 
	void invertedAction(int actorNumber,T measurment)
	{
		//measured value lower than minimum range : deactivate actor
		if(measurment < this->actors[actorNumber].rangeLow)
		{
			bitWrite( this->action , actorNumber , 0) ;
		}
		//measured value higher than maximum range : activate actor
		else if(measurment > this->actors[actorNumber].rangeHigh)
		{
			bitWrite( this->action , actorNumber , 1) ;
		}

	#if DEBUG == 1
		
		Serial.print("inverted Actor N° : ");
		Serial.println(actorNumber);

		Serial.print("measured value : ");
		Serial.println(measurment);

		Serial.print("high range : ");
		Serial.println(this->actors[actorNumber].rangeHigh);

		Serial.print("low range : ");
		Serial.println(this->actors[actorNumber].rangeLow);
	
	#endif
	}

	void temporalActionOff(int actorNumber);

	void temporalActionOn(int actorNumber);

	template <typename T> 
	void mixedTemporalActionOff(int actorNumber,T measurment)
	{
		if( ( millis()- this->actors[actorNumber].actifTime  ) >= (  this->actors[actorNumber].timeHigh  ) )
			{	
				if( measurment > this->actors[actorNumber].rangeHigh )
				{
					//stop the actor
					bitWrite( this->action , actorNumber , 0) ;

					//make the actor inactif:
					this->actors[actorNumber].actif=0;

					//start the low timer
					this->actors[actorNumber].inactifTime=millis();

				}
				else 
				{
					bitWrite( this->action , actorNumber , 1) ;				
				}			
			}
	#if DEBUG == 1
		
		Serial.print("mixed Temporal Actor N° : ");
		Serial.println(actorNumber);

		Serial.print("measured value : ");
		Serial.println(measurment);

		Serial.print("high range : ");
		Serial.println(this->actors[actorNumber].rangeHigh);

		Serial.print("time high : ");
		Serial.println(this->actors[actorNumber].timeHigh);

		Serial.print("actif Time : ");
		Serial.println(this->actors[actorNumber].actifTime);
	
	#endif
	}

	template <typename T> 
	void mixedTemporalActionOn(int actorNumber,T measurment)
	{
		if( ( millis() - this->actors[actorNumber].inactifTime ) >= (  this->actors[actorNumber].timeLow  ) )
		{
			if( measurment < this->actors[actorNumber].rangeLow )
			{
				//start the actor
				bitWrite( this->action , actorNumber , 1) ;

				//make the actor actif:
				this->actors[actorNumber].actif=1;

				//start the low timer
				this->actors[actorNumber].actifTime=millis();

			}
			else 
			{
				bitWrite( this->action , actorNumber , 0) ;				
			}

		}

	#if DEBUG == 1
		
		Serial.print("mixed Temporal Actor N° : ");
		Serial.println(actorNumber);

		Serial.print("measured value : ");
		Serial.println(measurment);

		Serial.print("low range : ");
		Serial.println(this->actors[actorNumber].rangeLow);

		Serial.print("time low : ");
		Serial.println(this->actors[actorNumber].timeLow);

		Serial.print("inactif Time : ");
		Serial.println(this->actors[actorNumber].inactifTime);
	
	#endif
		
	}

	void hourAction(int actorNumber, int hour); 

	template <typename T> 
	void mixedHourAction(int actorNumber,int hour, T measurment)
	{
		//starting the actor
		if(hour >= this->actors[actorNumber].hourHigh)
		{
				if( measurment < this->actors[actorNumber].rangeLow )
				{
					bitWrite( this->action , actorNumber , 1) ;
				}
				else 
				{
					bitWrite( this->action , actorNumber , 0) ;				
				}

		}
		//stop the actor	
		else if(hour >= this->actors[actorNumber].hourLow)
		{
				if( measurment > this->actors[actorNumber].rangeHigh )
				{
					bitWrite( this->action , actorNumber , 0) ;
				}
				else 
				{
					bitWrite( this->action , actorNumber , 1) ;				
				}
		}

	#if DEBUG == 1
		
		Serial.print("mixed hour Actor N° : ");
		Serial.println(actorNumber);

		Serial.print(" hour : ");
		Serial.println(hour);

		Serial.print("high hour : ");
		Serial.println(this->actors[actorNumber].hourHigh);

		Serial.print("low hour : ");
		Serial.println(this->actors[actorNumber].hourLow);

		Serial.print("measured value : ");
		Serial.println(measurment);

		Serial.print("high range : ");
		Serial.println(this->actors[actorNumber].rangeHigh);

		Serial.print("low range : ");
		Serial.println(this->actors[actorNumber].rangeLow);
	
	#endif

	
	}

	void minuteAction(int actorNumber,int minute);

	template <typename T> 
	void mixedMinuteAction(int actorNumber,int minute,T measurment)
	{
		//starting the actor
		if(minute >= this->actors[actorNumber].minuteHigh)
		{
				if( measurment < this->actors[actorNumber].rangeLow )
				{
					bitWrite( this->action , actorNumber , 1) ;
				}
				else 
				{
					bitWrite( this->action , actorNumber , 0) ;				
				}

		}
		//stop the actor	
		else if(minute >= this->actors[actorNumber].minuteLow)
		{
				if( measurment > this->actors[actorNumber].rangeHigh )
				{
					bitWrite( this->action , actorNumber , 0) ;
				}
				else 
				{
					bitWrite( this->action , actorNumber , 1) ;				
				}
		}

	#if DEBUG == 1
		
		Serial.print("minute Actor N° : ");
		Serial.println(actorNumber);

		Serial.print(" minute : ");
		Serial.println(minute);

		Serial.print("high minute : ");
		Serial.println(this->actors[actorNumber].minuteHigh);

		Serial.print("low minute : ");
		Serial.println(this->actors[actorNumber].minuteLow);

		Serial.print("measured value : ");
		Serial.println(measurment);

		Serial.print("high range : ");
		Serial.println(this->actors[actorNumber].rangeHigh);

		Serial.print("low range : ");
		Serial.println(this->actors[actorNumber].rangeLow);
	
	#endif


	}

	void hourMinuteAction(int actorNumber,int hour,int minute);

	template <typename T> 
	void mixedHourMinuteAction(int actorNumber,int hour,int minute ,T measurment)
	{
		//start the actor
		if(hour>=this->actors[actorNumber].hourHigh)
		{
			if(minute>= this->actors[actorNumber].minuteHigh)
			{
				if( measurment < this->actors[actorNumber].rangeLow )
				{
					bitWrite( this->action , actorNumber , 1) ;
				}
				else 
				{
					bitWrite( this->action , actorNumber , 0) ;				
				}
			}
		}
		//stop the actor
		else if(hour>=this->actors[actorNumber].hourLow)
		{
			if(minute>= this->actors[actorNumber].minuteLow)
			{
				if( measurment > this->actors[actorNumber].rangeHigh )
				{
					bitWrite( this->action , actorNumber , 0) ;
				}
				else 
				{
					bitWrite( this->action , actorNumber , 1) ;				
				}
			}
		}

	#if DEBUG == 1
		
		Serial.print("hourMinute Actor N° : ");
		Serial.println(actorNumber);

		Serial.print(" hour : ");
		Serial.println(hour);
		Serial.print(" minute : ");
		Serial.println(minute);

		Serial.print("high hour : ");
		Serial.println(this->actors[actorNumber].hourHigh);

		Serial.print("high minute : ");
		Serial.println(this->actors[actorNumber].minuteHigh);

		Serial.print("low hour : ");
		Serial.println(this->actors[actorNumber].hourLow);

		Serial.print("low minute : ");
		Serial.println(this->actors[actorNumber].minuteLow);

		Serial.print("measured value : ");
		Serial.println(measurment);

		Serial.print("high range : ");
		Serial.println(this->actors[actorNumber].rangeHigh);

		Serial.print("low range : ");
		Serial.println(this->actors[actorNumber].rangeLow);
	
	#endif
	
	}

	bool config();

	void printConf();

private:

	byte action= B00000000; 

	struct state{
	
		bool actif=0;

		bool temporal=0;

		bool inverted=0;
		
		//"type":["temperature","hour"]
		String primaryType="";//the primary type is related to the sensor's type (type[0])

		String secondaryType="";// the secondary type if present is hour or minute or hourMinute (type[1]) 
		
		//"low":[20,5000,18]
		int rangeLow=0;//this is the minimum at which the actor becomes actif (low[0])
		
		unsigned long timeLow=0;//this is the time the actor is off in temporal mode (low[1])
		
		int hourLow=0; //this is the hour when to turn off the actor in temporal/hour(hourMinute) mode (low[2])
		
		int minuteLow=0;//this is the minute when to turn off the actor in temporal/minute(hourMinute) mode (low[3])


		//"high":[30,2000,17]
		int rangeHigh=0;//this is the maximum at which the actor becomes inactif(high[0])
		
		unsigned long timeHigh=0;//this is the time the actor is on in temporal mode(high[1])
		
		int hourHigh=0; //this is the hour when to turn on the actor in temporal/hour(hourMinute) mode(high[2])

		int minuteHigh=0;//this is the minute when to turn on the actor in temporal/minute(hourMinute) mode (high[3])

		unsigned long actifTime=0;
	
		unsigned long inactifTime=0;
	
	}actors[8];

	const int clockPin = 4; //clock pin for the shift register
	
	const int dataPin = 15; //data  pin for the shift register

	const int EnI2C=5;	// I2C/Shift pin
	
};

#endif
