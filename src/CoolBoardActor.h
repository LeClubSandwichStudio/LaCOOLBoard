/**
*	\file CoolBoardActor.h
*	\brief CoolBoardActor Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
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

#ifndef CoolBoardActor_H
#define CoolBoardActor_H

#include "Arduino.h"


/**
*	\class CoolBoardActor
*	\brief This class manages the CoolBoardActor 
*
*/
class CoolBoardActor
{
public:

	void begin();		       //starts the CoolBoardActor

 	void write(bool action);			//CoolBoardActor
 
	void doAction(const char* data );

	void normalAction(float measurment);

	void invertedAction(float measurment);

	void temporalActionOff();

	void temporalActionOn();

	void mixedTemporalActionOff(float measurment);

	void mixedTemporalActionOn(float measurment);

	void hourAction( int hour); 

	void mixedHourAction(int hour, float measurment);

	void minuteAction(int minute);

	void mixedMinuteAction(int minute,float measurment);

	void hourMinuteAction(int hour,int minute);

	void mixedHourMinuteAction(int hour,int minute ,float measurment);

        bool config();

	void printConf();

private:

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
	
	}actor;
	
	const int pin = 15; //pin for the actor
	
};

#endif
