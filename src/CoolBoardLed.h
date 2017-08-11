/**
*	\file CoolBoardLed.h	
*	\brief CoolBoardLed Header File
*	\author Mehdi Zemzem
*	\version 1.0
*
*/
#ifndef CoolOnBoardLed_H
#define CoolOnBoardLed_H

#include"Arduino.h"

#include <NeoPixelBus.h>


/**
*	\class	CoolBoardLed
*	\brief	This class handles the led in the Sensor Board
*/
class CoolBoardLed

{

public:

	void begin();			 


	void write(int R, int G, int B);       

	void end();				//delete the dynamic led;

	bool config();

	void activate();

	void printConf();


	//Neo Pixel Led methods :
	void fade(int R, int G, int B, float T);

	void blink(int R, int G, int B, float T);

	void fadeIn(int R, int G, int B, float T);

	void fadeOut(int R, int G, int B, float T);

	void strobe(int R, int G, int B, float T); 



private:


	NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* neoPixelLed = NULL; //template instance must be dynamic

	byte ledActive=0;

};

#endif
