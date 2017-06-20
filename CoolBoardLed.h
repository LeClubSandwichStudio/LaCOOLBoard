/*
*
* This class manages the On Board Actors
*
*
*
*/
#ifndef CoolOnBoardActors_H
#define CoolOnBoardActors_H

#include"Arduino.h"
#include "GenericActor.h"
#include <NeoPixelBus.h>


//#define PixelCount  1   // this example assumes 4 pixels, making it smaller will cause a failure
//#define PixelPin 2      // make sure to set this to the correct pin, ignored for Esp8266

class CoolBoardLed: public GenericActor

{

public:

//inherited methods
virtual void begin(byte pin);	         //starts the actor
virtual void begin();			 //starts the actor

virtual void write(bool state);	//writes to the actor
virtual void write();			//writes to the actor 

void write(int R, int G, int B);       
void end();				//delete the dynamic led;

bool config();
void printConf();


//Neo Pixel Led methods :
void colorFade(int R, int G, int B, int T);

void blink(int R, int G, int B, int T);

void fadeIn(int R, int G, int B, int T);

void fadeOut(int R, int G, int B, int T);

void strobe(int R, int G, int B, int T); 

void neoPixelLedBegin();




private:


NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* neoPixelLed = NULL; //template instance must be dynamic

byte ledActive;

};

#endif
