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
       
	bool config();

	void printConf();

private:

	byte action= B00000000; 

	struct state{
	
	String type;

	bool actif=0; 

	int low=0;	//if temporal == 1 this is the time spent inactif in ms
			//if type is hour/minute/second this is the deactivation time

	int high=0;	//if temporal==1 this is the time spent actif in ms
			//if type is hour/minute/second this is the activation time

	bool temporal=0;
	
	unsigned long actifTime=0;
	
	unsigned long inactifTime=0;
	
	bool inverted=0;	

	}actors[8];

	const int clockPin = 4; //clock pin for the shift register
	
	const int dataPin = 15; //data  pin for the shift register

	const int EnI2C=5;	// I2C/Shift pin
	
};

#endif
