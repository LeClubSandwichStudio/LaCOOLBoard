/*
*
* This class manages the Jetpack
*
*
*
*/
#ifndef Jetpack_H
#define Jetpack_H

#include"Arduino.h"


class Jetpack
{
public:

 //Jetpack(byte actors); //constructor
 
 virtual void begin(byte pin);		       //starts the Jetpack
 virtual void begin();		       //starts the Jetpack
 


 
 virtual void write(byte action);			//writes to the Jetpack
 
 void writeBit(byte pin,bool state); //writes to a single pin of the Jetpack
 
 void doAction(const char* data, int JSON_SIZE);
 
 bool config();
 void printConf();
private:
	// byte actors; //pin for the actor 

	byte action= B00000000; 
	byte actorsNumber;

	struct state{
	const char* type;
	byte actif;
	int low;
	int high;	
	}actors[8];

	
	const int clockPin = 4; //clock pin for the shift register
	const int dataPin = 15; //data  pin for the shift register
	const int EnI2C=5;	// I2C/Shift pin
};

#endif
