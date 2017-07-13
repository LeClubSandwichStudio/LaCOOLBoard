/**
*	\file CoolFileSystem.h
*	\brief CoolFileSystem Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/

#ifndef CoolFileSystem_H
#define CoolFileSystem_H


#include "Arduino.h"

/**
*	\class CoolFileSystem
*  
*	\brief This class handles the file system 
*  
*/
class CoolFileSystem
{

public:
	bool begin(); 

	bool updateConfigFiles(String answer,int JSON_SIZE);	
	
	bool saveSensorData(const char* data,int Sensor_JSON_SIZE);
	
	bool saveSensorDataCSV(const char* data,int Sensor_JSON_SIZE);
	
	bool isDataSaved();
	
	String getSensorSavedData();
		
private:
	
	bool savedData=0;	

};

#endif
