/*
*	CoolFileSystem.h
*  
*	This class handles the file system 
*  
*  
*  
*  
*  
*  
*/

#ifndef CoolFileSystem_H
#define CoolFileSystem_H


#include "Arduino.h"

class CoolFileSystem
{

public:
	bool begin(); 

	bool updateConfigFiles(String answer,int JSON_SIZE);	
	
	bool saveSensorData(const char* data,int Sensor_JSON_SIZE);

};

#endif
