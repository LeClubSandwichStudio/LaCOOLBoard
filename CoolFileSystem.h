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
#include <memory>

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

	bool updateConfigFiles(String answer );
	
	bool fileUpdate(String update,const char* path);	
	
	bool saveSensorData(const char* data );
	
	bool saveSensorDataCSV(const char* data );
	
	bool isDataSaved();
	
	std::unique_ptr<String[]> getSensorSavedData(int& size);
		
private:
	
	bool savedData=0;	

};

#endif
