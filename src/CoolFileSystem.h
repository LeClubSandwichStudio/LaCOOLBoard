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

	bool updateConfigFiles(String answer );
	
	bool fileUpdate(String update,const char* path);	
	
	bool saveSensorData(const char* data );
	
	bool saveSensorDataCSV(const char* data );
	
	int isDataSaved();
	
	String* getSensorSavedData(int& size);
	
	bool incrementsavedData();

	void getsavedData();
		
private:
	
	int savedData=0;
	
	int linesToSkip=0;	

};

#endif
