/**
*	\file	CoolBoard.h
*  	\brief	CoolBoard Header file
*	\author	Mehdi Zemzem
*	\version 1.0  
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

#ifndef CoolBoard_H
#define CoolBoard_H


#include"CoolFileSystem.h" 	//CoolBoard File System Manager
#include"CoolBoardSensors.h"	//CoolBoard Sensor Board Manager
#include"CoolBoardLed.h"	//CoolBoard Led Manager
#include"CoolTime.h"		//CoolBoard Real Time Clock Manager
#include"CoolMQTT.h"		//CoolBoard MQTT Manager
#include"Jetpack.h"		//CoolBoard Jetpack Manager
#include"Irene3000.h"		//CoolBoard Irene3000 Manager
#include"ExternalSensors.h"	//CoolBoard External Sensors Manager
#include"CoolWifi.h"		//CoolBoard Wifi Manager
#include"CoolBoardActor.h"	//CoolBoard Actor Manager

#include"Arduino.h"		//Arduino Defs

/**
*	\class	CoolBoard
*	
*	\brief This class manages the CoolBoard and all of
*	Its functions
*/
class CoolBoard
{

public:
	//Constructor
	CoolBoard();

	void begin(); 
	
	bool config();

	void update(const char*answer );

	void offLineMode();

	void onLineMode();

	int connect();
	
	int isConnected();

	unsigned long getLogInterval();

	void printConf();

	void sleep(unsigned long interval);

	String readSensors();
	
	void initReadI2C();
	
	String userData();


private:

	CoolFileSystem fileSystem; 

	CoolBoardSensors coolBoardSensors;

	CoolBoardLed coolBoardLed;

	CoolTime rtc;
	
	CoolWifi wifiManager;

	CoolMQTT mqtt;

	Jetpack jetPack;

	Irene3000 irene3000;

	ExternalSensors externalSensors;
	
	CoolBoardActor	onBoardActor;

	bool userActive=0;

	bool ireneActive=0;

	bool jetpackActive=0;

	bool externalSensorsActive=0;		

	bool sleepActive=0;
	
	bool manual=0;	

	unsigned long logInterval=1;//s

	String data="";

	String answer="";

	const int EnI2C = 5;                            // double usage for I2C and shift register latch , HIGH=I2C , LOW=shift register latch
							// All I2C is over pins (2,14)



};

#endif
