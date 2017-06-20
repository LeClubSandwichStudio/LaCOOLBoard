/*
*  CoolFileSystem.h
*  
*  This class manages the file system and on board server
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

	bool updateConfigFiles(const char* answer,int JSON_SIZE);	
	
	bool saveSensorData(const char* data,int Sensor_JSON_SIZE);
private:
	



	
	

};

#endif
