/*
*
*
* Abstract Class for a generic Actor
*
*
*
*
*/

#ifndef GenericActor_H
#define GenericActor_H

#include "Arduino.h"

class GenericActor 
{

public:

virtual void begin(byte pin)=0; //pure virtual =>define in inheriting classes
virtual void begin()=0; //pure virtual =>define in inheriting classes

virtual void write(bool state)=0; //pure virtual =>define in inheriting classes







};


#endif
