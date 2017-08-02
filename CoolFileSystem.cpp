/**
*	\file CoolFileSystem.cpp
*	\brief CoolFileSystem Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/

#include <FS.h>
#include "CoolFileSystem.h"
#include "ArduinoJson.h"      // Arduino JSON File controller  https://github.com/bblanchon/ArduinoJson



#define DEBUG 1

#ifndef DEBUG

#define DEBUG 0 

#endif


/**
*	CoolFileSystem::begin():
*	This method is provided to start the
*	SPIFFS object.
*
*	\return true if SPIFFS was initialized correctly,
*	false otherwise
*/
bool CoolFileSystem::begin()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.begin()") );
	Serial.println();	
	Serial.print( F("SPIFFS success ? ") );
	Serial.println(SPIFFS.begin());
	Serial.println();

#endif

	return( SPIFFS.begin() );                                   //Initialize Filesystem

}

/**
*	CoolFileSystem::saveSensorData( data ):
*	This method is provided to save the data on the local
*	memory when there is no internet available
*
*	sets the saved data flag to TRUE when successful
*
*	\return true if the data was saved,
*	false otherwise
*/
bool CoolFileSystem::saveSensorData(const char* data )
{

#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.saveSensorData()") );
	Serial.println();

#endif
	
	File sensorsData=SPIFFS.open("/sensorsData.json","a");

	if(!sensorsData)
	{
	
	#if DEBUG == 1
	
		Serial.println( F("failed to append to /sensorsData.json") );
		Serial.println();
	
	#endif

		this->savedData=false;
		return (false);	
	}	

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(data);

	if( root.success() )
	{
		root.printTo(sensorsData);
		sensorsData.println();
		sensorsData.close();


	#if DEBUG == 1
		
		Serial.print(F("jsonBuffer size: "));
		Serial.println(jsonBuffer.size());
		Serial.println();

		sensorsData=SPIFFS.open("/sensorsData.json","r");
		
		if(!sensorsData)
		{
			
			Serial.println(F("failed to reopen /sensorsData.json"));
						
		}
	
		Serial.println( F("saved data is : ") );
		root.printTo(Serial);
		Serial.println();

		Serial.println(F("/sensorsData.json") );
		while (sensorsData.available()) 
		{
			Serial.println(sensorsData.readString()) ;
		}
		
		Serial.println();
		
		sensorsData.close();
	
	#endif

		this->saveSensorDataCSV(data);		

		this->savedData=true;
		return (true);		
	}
	else
	{
	
	#if DEBUG == 1

		Serial.println( F("failed to parse json") );
	
	#endif

		this->savedData=false;
		return(false);
	}
	

}


/**
*	CoolFileSystem::saveSensorDataCSV( data ):
*	This method is provided to save the data on the local
*	memory in CSV format.
*
*	\return true if the data was saved,
*	false otherwise
*/
bool CoolFileSystem::saveSensorDataCSV(const char* data )
{
#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.saveSensorDataCSV()") );
	Serial.println();

#endif
	//parsing json
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(data);
	String header="",values="";
	
	//if json parse success
	if( root.success() )
	{		
		for (auto kv : root) 
		{
			//print the header(json keys ) to header string
			header+=kv.key;
			header+=',';
			
			//print the values to header string
			values+=( kv.value.as<char*>() );
			values+=',';
		}

		header.remove(header.lastIndexOf(','), 1);
		values.remove(values.lastIndexOf(','), 1);		
	
	#if DEBUG == 1
	
		Serial.println( F(" data is : ") );
		root.printTo(Serial);
		Serial.println();
		
		Serial.println(F(" header is :" ) ) ;
		Serial.println(header);
		Serial.println(F(" values are : "));
		Serial.println(values);
		
		Serial.print(F("jsonBuffer size: "));
		Serial.println(jsonBuffer.size());
		Serial.println();

	
	#endif
	
	}
	//failed to parse json
	else
	{
	
	#if DEBUG == 1

		Serial.println( F("failed to parse json") );
	
	#endif

		return(false);
	}

	//check if file exists
	File sensorsData=SPIFFS.open("/sensorsData.csv","r");
	
	//file doesn't exist
	if(!sensorsData)
	{
	
	#if DEBUG == 1
	
		Serial.println( F("/sensorsData.csv not found") );
		Serial.println( F("creating /sensorsData.csv") );
		Serial.println();
	
	#endif
		//create file
		sensorsData=SPIFFS.open("/sensorsData.csv","w");
		
		if(!sensorsData)
		{

		#if DEBUG == 1
		
			Serial.println( F("failed to create /sensorsData.csv") );
			Serial.println();
		
		#endif
		
			return(false);

		}
		
		//print the header(json keys ) to the CSV file
		sensorsData.println(header);

		//print the values to the CSV file
		sensorsData.println(values);
		
		sensorsData.close();
	
	#if DEBUG == 1

		sensorsData=SPIFFS.open("/sensorsData.csv","r");
		
		if(!sensorsData)
		{
			Serial.println(F("failed to reopen /sensorsData.csv "));
			return(false);		
		}

		Serial.println( F("/sensorsData.csv : ") );

		while (sensorsData.available()) 
		{
  			Serial.print(sensorsData.readString()) ;
		}
		Serial.println();

		//close the file
		sensorsData.close();

	#endif
		

		
		return(true);
		
	}

	//file exist
	else
	{

	#if DEBUG == 1
	
		Serial.println( F("/sensorsData.csv  found") );
		Serial.println( F("appending to /sensorsData.csv") );
		Serial.println();
	
	#endif

		//append to file
		sensorsData=SPIFFS.open("/sensorsData.csv","a");
		
		if(!sensorsData)
		{
		
		#if DEBUG == 1
			
			Serial.println( F("failed to open /sensorsData.csv") );
			Serial.println();

		#endif
			
			return(false);
		
		}

		//print the values to the CSV file
		sensorsData.println(values);
		
		sensorsData.close();

	#if DEBUG == 1

		sensorsData=SPIFFS.open("/sensorsData.csv","r");
		
		if(!sensorsData)
		{
			Serial.println(F("failed to reopen /sensorsData.csv "));
			return(false);		
		}

		
		Serial.println( F("/sensorsData.csv : ") );

		while (sensorsData.available()) 
		{
  			Serial.println(sensorsData.readString()) ;
		}
		
		Serial.println();
		
		sensorsData.close();
		
	#endif		
		
		return(true);
	
	}	



}


/**
*	CoolFileSystem::updateConfigFiles( mqtt answer ):
*	This method is provided to update the configuration files when
*	the appropriate mqtt answer is received
*
*	\return true if the files are updated correctly,
*	false otherwise
*/
bool CoolFileSystem::updateConfigFiles(String answer )
{

#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.updateConfigFiles") );
	Serial.println();
	
	Serial.println( F("input answer : ") );
	Serial.println(answer);
#endif

	//total json object	
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject( answer.c_str() );

#if DEBUG == 1
	
	Serial.println( F("json object : ") );	
	root.printTo(Serial);
	Serial.println();
	
	Serial.print(F("jsonBuffer size: "));
	Serial.println(jsonBuffer.size());
	Serial.println();


#endif

	if(! ( root.success() ))
	{
	
	#if DEBUG == 1

		Serial.println( F("failed to parse root ") );
		Serial.println();
	
	#endif

		return(false);
	}
	else
	{
	#if DEBUG == 1
		
		Serial.println( F("success to parse root ") );
		Serial.println();
		
	#endif	
	}
	
#if DEBUG == 1

	Serial.println( F("input message is : ") );
	root.printTo(Serial);
	Serial.println();

#endif
	//temp string
	String temp;

	//CoolBoard Configuration File

    	JsonObject& jsonCoolBoard=root["CoolBoard"];

#if DEBUG == 1

	Serial.println( F("before config CoolBoard json") );
	jsonCoolBoard.printTo(Serial);

#endif

	if(jsonCoolBoard.success())
	{
		String update;
	
		jsonCoolBoard.printTo(update);

		this->fileUpdate(update,"/coolBoardConfig.json");		
		
	}
	else
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to parse CoolBoard ") );
	
	#endif

	}		

	
	//Cool Board Sensors Configuration File
    	JsonObject& jsonSensorsBoard=root["CoolSensorsBoard"];

#if DEBUG == 1 

	Serial.println( F("before config CoolSensorsBoard json") );
	jsonSensorsBoard.printTo(Serial);

#endif 
	
	if(jsonSensorsBoard.success())
	{	
		String update;
	
		jsonSensorsBoard.printTo(update);

		this->fileUpdate(update,"/coolBoardSensorsConfig.json");		

	}
	else
	{

	#if DEBUG == 1

		Serial.println( F("failed to parse CoolSensorsBoard sensors ") );	
	
	#endif

	}

	
	//rtc configuration file
    	JsonObject& jsonRTC=root["rtc"];

#if DEBUG == 1 
	
	Serial.println( F("before config rtc json") );
	jsonRTC.printTo(Serial);

#endif
	if(jsonRTC.success() )
	{
		String update;

		jsonRTC.printTo(update);

		this->fileUpdate(update,"/rtcConfig.json");			
	}
	else
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to parse rtc ") );
	
	#endif

	}
	
	
        //cool board led configuration
    	JsonObject& jsonLedBoard=root["led"];
	
#if DEBUG == 1 

	Serial.println( F("before config Led json") );
	jsonLedBoard.printTo(Serial);

#endif

	if(jsonLedBoard.success())
	{	
		String update;
	
		jsonLedBoard.printTo(update);

		this->fileUpdate(update,"/coolBoardLedConfig.json");		

	
	}
	else
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to parse led") );
	
	#endif 

	}
		

	

	//jetpack configuration
    	JsonObject& jsonJetpack=root["jetPack"];

#if DEBUG == 1 

	Serial.println( F("before config jetpack json") );
	jsonJetpack.printTo(Serial);

#endif

	if(jsonJetpack.success())
	{
	
		String update;
	
		jsonJetpack.printTo(update);

		this->fileUpdate(update,"/jetPackConfig.json");		

	}

	else
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to parse jetpack") );	
	
	#endif

	}
	
	//irene configuration	
    	JsonObject& jsonIrene=root["irene3000"];
	
#if DEBUG == 1 

	Serial.println( F("before config irene json") );	
	jsonIrene.printTo(Serial);

#endif 

	if(jsonIrene.success())
	{

		String update;
	
		jsonIrene.printTo(update);

		this->fileUpdate(update,"/irene3000Config.json");		
	
	}
	else
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to parse irene") );	
	
	#endif 


	}
	
	//external sensors
    	JsonObject& jsonExternalSensors=root["externalSensors"];

#if DEBUG == 1 

	Serial.println( F("before config external Sensors json") );
	jsonExternalSensors.printTo(Serial);

#endif

	if(jsonExternalSensors.success())
	{

		String update;
	
		jsonExternalSensors.printTo(update);

		this->fileUpdate(update,"/externalSensorsConfig.json");		

	}

	else
	{	

	#if DEBUG == 1
		
		Serial.println( F("failed to parse external sensors") );

	#endif


	}

	
	//mqtt config
    	JsonObject& jsonMQTT=root["mqtt"];
	
#if DEBUG == 1 

	Serial.println( F("before config mqtt json") );
	jsonMQTT.printTo(Serial);

#endif

	if(jsonMQTT.success())
	{

		String update;
	
		jsonMQTT.printTo(update);

		this->fileUpdate(update,"/mqttConfig.json");		

	}
	else
	{

	#if DEBUG == 1 

		Serial.println( F("failed to parse mqtt") );
	
	#endif

	
	}	

	//wifi config
    	JsonObject& jsonWifi=root["wifi"];
	
#if DEBUG == 1 

	Serial.println( F("before config wifi json") );
	jsonWifi.printTo(Serial);

#endif

	if(jsonWifi.success())
	{

		String update;
	
		jsonWifi.printTo(update);

		this->fileUpdate(update,"/wifiConfig.json");		

	}
	else
	{

	#if DEBUG == 1 

		Serial.println( F("failed to parse wifi") );
	
	#endif

	
	}	
		
	return true;

}	

/**
*	CoolFileSystem::isDataSaved():
*	This method is provided to report
*	wether there is sensor data saved in the
*	File System.
*
*	\return true if there is data saved, false
*	otherwise
*/
bool CoolFileSystem::isDataSaved()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolFileSystem.isDataSaved()") );
	Serial.println();
#endif

	File sensorsData=SPIFFS.open("/sensorsData.json","r");
	File sensorsDataCSV=SPIFFS.open("/sensorsData.csv","r");
	
	if( (!sensorsData)||(!sensorsDataCSV) )	
	{
	#if DEBUG == 1

		Serial.println( F("failed to open files") );

	#endif
		
		this->savedData=false;
	}
	else
	{		
		#if DEBUG == 1

			Serial.print(F("sensors Data file size : "));
			Serial.println(sensorsData.size());
			Serial.println();
			
			Serial.print(F("sensors Data CSV file size : "));				
			Serial.println(sensorsDataCSV.size());
			Serial.println();
		#endif	

		if( (sensorsData.size()!=0) || (sensorsDataCSV.size()!=0) )
		{
			this->savedData=true;
		}
		else
		{

			this->savedData=false;		
		
		}	
	}

#if DEBUG == 1 

	Serial.print( F("savedData : ") );
	Serial.println(this->savedData);

#endif

	return( this->savedData );
}

/**
*	CoolFileSystem::getSensorData():
*	This method is provided to return the 
*	sensor data saved in the File System
*
*	\return string json of the saved sensor 
*	data file
*/
String CoolFileSystem::getSensorSavedData()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolFileSystem.getSensorSavedData()") );
	Serial.println();

#endif

	//open sensors data file
	File sensorsData=SPIFFS.open("/sensorsData.json","r");
	
	if (!sensorsData)
	{

	#if DEBUG == 1 

		Serial.println( F("Failed to read /sensorsData.json") );

	#endif
 
		return("failed to open file");
	}

	else
	{
		size_t size = sensorsData.size();

		// Allocate a buffer to store contents of the file.
		std::unique_ptr < char[] > buf(new char[size]);

		sensorsData.readBytes(buf.get(), size);

		DynamicJsonBuffer jsonBuffer;

		JsonObject & json = jsonBuffer.parseObject(buf.get());
		
		if (!json.success())
		{

		#if DEBUG == 1
		
			Serial.println( F("failed to parse json") );
		
		#endif
		
			return("failed to parse json");
		}
		else
		{	
			//the return string
			String sensorDataString;
			
			//print the json to the string
			json.printTo(sensorDataString);
			
			//close the file
			sensorsData.close();

			//delete data in the file
			File sensorsData=SPIFFS.open("/sensorsData.json","w");
			File sensorsDataCSV=SPIFFS.open("/sensorsData.csv","w");
			if( (!sensorsData)||(!sensorsDataCSV) )	
			{
			#if DEBUG == 1
		
				Serial.println( F("failed to delete data in the file") );
		
			#endif

				return("failed to delete data in the file");
			}

			sensorsData.close();
			sensorsDataCSV.close();

			//position the saved data flag to false
			this->savedData=false;	
			
		#if DEBUG == 1 

			Serial.println( F("saved data : ") );
			Serial.println(sensorDataString);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();

		
		#endif

			//return the string
			return(sensorDataString);		
		}
		
		
	}

}

/**
*	CoolFileSystem::fileUpdate( update msg, file path):
*	This method is provided to ensure the 
*	correct update for each configuration file in the
*	File system
*
*	\return true if successful , false otherwise
*
*/
bool CoolFileSystem::fileUpdate(String update,const char* path)
{

#if DEBUG == 1

	Serial.println( F("Entering CoolFileSystem.fileUpdate()") );
	Serial.println();
	
	Serial.println(F("update msg is :"));
	Serial.println(update);
	
	Serial.println(F("file path is : "));
	Serial.println(path);	

#endif
	//transfer update String to json
	DynamicJsonBuffer updateBuffer;
	JsonObject& updateJson = updateBuffer.parseObject(update.c_str() );
	
	if(updateJson.success())
	{
	
	#if DEBUG ==1
		
		Serial.println(F("root parsing success :"));
		updateJson.printTo(Serial);
	
	#endif

	}
	else
	{
	
	#if DEBUG == 1 
	
		Serial.println(F("root parsing failure "));
	
	#endif
		
		return(false);	

	}
	
	//open file in read mode
	File configFile = SPIFFS.open( path , "r");
	
	if(!configFile)
	{	
	#if DEBUG == 1
		
		Serial.print( F("failed to read ") );
		Serial.println(path);

	#endif
		return(false);
	}

	//copy file to a json
	size_t size = configFile.size();

	// Allocate a buffer to store contents of the file.
	std::unique_ptr < char[] > buf(new char[size]);

	configFile.readBytes(buf.get(), size);

	DynamicJsonBuffer fileBuffer;

	JsonObject & fileJson = fileBuffer.parseObject(buf.get());

	if (!fileJson.success())
	{

	#if DEBUG == 1

		Serial.println( F("failed to parse json") );

	#endif

		return(false);
	}
	
	//modify root to contain all the json keys: updated ones and non updated ones
	for (auto kv : fileJson) 
	{
		if( updateJson[kv.key].success() )
		{
			fileJson[kv.key]=updateJson[kv.key];			
		}
		else
		{
			fileJson[kv.key]=fileJson[kv.key];
		}

				
	}

#if DEBUG == 1

	Serial.println(F("fileJson is now : "));
	fileJson.printTo(Serial);

#endif

	//close the file
	configFile.close();

	//open file in w mode
	configFile = SPIFFS.open( path , "w");
	
	if(!configFile)
	{	
	#if DEBUG == 1
		
		Serial.print( F("failed to open ") );
		Serial.println(path);

	#endif
		return(false);
	}
	//print json to file	
	
	fileJson.printTo(configFile);
	
	//close file
	configFile.close();


#if DEBUG == 1

	Serial.println( F("config is") );
	fileJson.printTo(Serial);
	Serial.println();

#endif
	
	return(true);
	
}
