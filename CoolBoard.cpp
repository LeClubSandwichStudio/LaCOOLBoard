/**
*	\file	CoolBoard.cpp
*  	\brief	CoolBoard Source file
*	\author	Mehdi Zemzem
*	\version 1.0
*  	\date	27/06/2017
*
*
*/
#include "FS.h"
#include "Arduino.h"
#include "CoolBoard.h"
#include "ArduinoJson.h"




#define DEBUG 1 

#ifndef DEBUG

#define DEBUG 0

#endif




/**
*	CoolBoard::begin():
*	This method is provided to configure and
*	start the used CoolKit Parts.
*	It also starts the first connection try
*	If Serial is enabled,it prints the configuration
*	of the used parts.
*/
void CoolBoard::begin()
{

#if DEBUG == 1

	Serial.println("Starting the CoolBoard  " );
	Serial.println("Entering CoolBoard.begin() " );
	Serial.println();
#endif
	
	fileSystem.begin();
	delay(100);
	
	coolBoardSensors.config();
	coolBoardSensors.begin();
	coolBoardSensors.printConf();
	delay(100);

	coolBoardLed.config();
	coolBoardLed.begin();
	coolBoardLed.printConf();
	delay(100);

	mqtt.config();
	mqtt.begin();
	mqtt.printConf();
	delay(100);

	if (jetpackActive)
	{
		jetPack.config();
		jetPack.begin();
		jetPack.printConf();
		delay(100);
	}

	if (ireneActive)
	{
		irene3000.config();
		irene3000.begin();
		irene3000.printConf();
		delay(100);
	}

	if (externalSensorsActive)
	{
		externalSensors.config();
		externalSensors.begin();
		externalSensors.printConf();
		delay(100);
	}
	
	this->connect();
	delay(100);

	rtc.config();
	rtc.begin();
	rtc.printConf();
	delay(100);


	this->connect();

	rtc.config();
	rtc.begin();
	rtc.printConf();
	delay(100);


}

/**
*	CoolBoard::connect():
*	This method is provided to manage the network
*	connection and the mqtt connection.
*
*	 \return mqtt client state
*/
int CoolBoard::connect()
{

#if DEBUG == 1	

	Serial.println("Entering CoolBoard.connect " );
	Serial.println();
	Serial.println("Connecting the CoolBoard  " );
	delay(100);

#endif

	if (WiFi.status() != WL_CONNECTED)
	{		
	
	#if DEBUG == 1		

		Serial.println("CoolBoard not connected to WiFi " );
		Serial.println("Launching WiFiManager" );
		Serial.println();

	#endif
	
		wifiManager.setConfigPortalTimeout(this -> serverTimeOut);
		wifiManager.autoConnect("CoolBoard");
		delay(100);

	}


	
	if (mqtt.state() != 0)
	{	
	
	#if DEBUG == 1	
	
		Serial.println("CoolBoard not connected to MQTT " );
		Serial.println("Launching mqtt.connect()" );
		Serial.println();

	#endif	
	
		mqtt.connect(this -> getLogInterval());
		delay(100);
		
	}
	
#if DEBUG == 1

	Serial.println("mqtt state is :");
	Serial.println(mqtt.state());
	Serial.println();
	delay(100);

#endif

	return(mqtt.state());
}

/**
*	CoolBoard::onLineMode():
*	This method is provided to manage the online
*	mode:	-update clock
*		-read sensor
*		-do actions
*		-publish data
*		-read answer
*		-update config
*/
void CoolBoard::onLineMode()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoard.onLineMode() " );
	Serial.println();

#endif

	data="";
	answer="";

	//send saved data if any
	if(fileSystem.isDataSaved())
	{
	
	#if DEBUG == 1

		Serial.println("There is data saved on the File System");
		Serial.println("Sending saved data over MQTT ");
		Serial.println();
	
	#endif	
	
		mqtt.publish("sending saved data");
		mqtt.mqttLoop();

		data+=fileSystem.getSensorSavedData();//{..,..,..}

		//formatting data:
		String jsonData = "{\"state\":{\"reported\":";
		jsonData += data; // {"state":{"reported":{..,..,..,..,..,..,..,..}
		jsonData += " } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } }

		mqtt.publish( data.c_str() );
		mqtt.mqttLoop();
	
	#if DEBUG == 1

		Serial.println("Saved data sent " );
		Serial.println();
	
	#endif

	}

	//clock update
	rtc.update();

	//read user data if user is active
	if(userActive)
	{
	
	#if DEBUG == 1

		Serial.println("User is Active");
		Serial.println("Collecting User's data ( mac,username,timeStamp )");
		Serial.println();
	
	#endif	
	
		//reading user data
		data=this->userData();//{"":"","":"","",""}

		//formatting json 
		data.setCharAt( data.lastIndexOf('}') , ',');//{"":"","":"","","",
				
		//read sensors data
	#if DEBUG == 1

		Serial.println("Collecting sensors data " );
		Serial.println();
	
	#endif

		data+=this->readSensors();//{"":"","":"","","",{.......}		

		//formatting json correctly
		data.remove(data.lastIndexOf('{'), 1);//{"":"","":"","","",.......}
				
	}	
	else
	{
		//read sensors data
	#if DEBUG == 1

		Serial.println("Collecting sensors data " );
		Serial.println();
	
	#endif

		data=this->readSensors();//{..,..,..}
	}
	
	//do action
	if (jetpackActive)
	{
	
	#if DEBUG ==1

		Serial.println("jetpack is Active ");
		Serial.println("jetpack doing action ");
		Serial.println();

	#endif

		jetPack.doAction(data.c_str(), sensorJsonSize);
	}
	
	//formatting data:
	String jsonData = "{\"state\":{\"reported\":";
	jsonData += data; // {"state":{"reported":{..,..,..,..,..,..,..,..}
	jsonData += " } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } }
	
	//mqtt client loop to allow data handling
	mqtt.mqttLoop();

	//read mqtt answer
	
	answer = mqtt.read();

#if DEBUG == 1 

	Serial.println("checking if there's an MQTT message " );
	Serial.println("answer is : ");	
	Serial.println(answer);	
	Serial.println();

#endif	


	//check if the configuration needs update 
	//and update it if needed 
	this -> update(answer.c_str());
	

	//publishing data	
	if( this->sleepActive==0)	
	{	
		
		mqtt.publish( jsonData.c_str(), this->getLogInterval() );
		mqtt.mqttLoop();
	
	}
	else
	{
		mqtt.publish(jsonData.c_str());		
		this->sleep( this->getLogInterval() ) ;
		mqtt.mqttLoop();
	}

	
		
		
}


/**
*	CoolBoard::offlineMode():
*	This method is provided to manage the offLine
*	mode:	-read sensors
*		-do actions
*		-save data in the file system
*/
void CoolBoard::offLineMode()
{

#if DEBUG == 1	
	
	Serial.println("Entering off line mode ");	
	
#endif

	//read user data if user is active
	if(userActive)
	{

	#if DEBUG == 1
		
		Serial.println("User is Active");
		Serial.println("Collecting User's data ( mac,username,timeStamp )");
		Serial.println();

	#endif

		//reading user data
		data=this->userData();//{"":"","":"","",""}

		//formatting json 
		data.setCharAt( data.lastIndexOf('}') , ',');//{"":"","":"","","",
		
				
		//read sensors data
	#if DEBUG == 1

		Serial.println("Collecting sensors data " );
		Serial.println();

	#endif

		data+=this->readSensors();//{"":"","":"","","",{.......}

		

		//formatting json correctly
		data.remove(data.lastIndexOf('{'), 1);//{"":"","":"","","",.......}
				
	}	
	else
	{
		//read sensors data
	#if DEBUG == 1

		Serial.println("Collecting sensors data " );
		Serial.println();

	#endif

		data=this->readSensors();//{..,..,..}
	}

	//do action
	if (jetpackActive)
	{

	#if DEBUG == 1

		Serial.println("jetpack is Active ");
		Serial.println("jetpack doing action ");
		Serial.println();
	
	#endif

		jetPack.doAction(data.c_str(), sensorJsonSize);
	}
	
	
	//saving data in the file system
	
	fileSystem.saveSensorData(data.c_str(), sensorJsonSize);
}

/**
*	CoolBoard::config():
*	This method is provided to configure
*	the CoolBoard :	-log interval
*			-Size of the data to write
*			-Size of the data to read
*			-irene3000 activated/deactivated
*			-jetpack activated/deactivated
*			-external Sensors activated/deactivated
*			-mqtt server timeout
*
*	\return true if configuration is done,
*	false otherwise
*/
bool CoolBoard::config()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoard.config() ");
	Serial.println();

#endif

	//open file system
	fileSystem.begin();
	
	//open configuration file
	File configFile = SPIFFS.open("/coolBoardConfig.json", "r");
	
	if (!configFile)

	{
	
	#if DEBUG == 1

		Serial.println("failed to read /coolBoardConfig.json  ");

	#endif
	
		return(false);
	}

	else
	{
		size_t size = configFile.size();

		// Allocate a buffer to store contents of the file.
		std::unique_ptr < char[] > buf(new char[size]);

		configFile.readBytes(buf.get(), size);

		DynamicJsonBuffer jsonBuffer;

		JsonObject & json = jsonBuffer.parseObject(buf.get());

		if (!json.success())
		{
		
		#if DEBUG == 1

			Serial.println("failed to parse CoolBoard Config json object ");
	
		#endif

			return(false);
		}

		else
		{	
		
		#if DEBUG == 1
			
			Serial.println("configuration json : ");
			json.printTo(Serial);
			Serial.println();
		
		#endif
			
			//parsing userActive Key
			if (json["userActive"].success())
			{
				this -> userActive = json["userActive"];
			}

			else
			{
				this -> userActive = this -> userActive;
			}
			json["userActive"] = this -> userActive;

			//parsing logInterval key
			if (json["logInterval"].success())
			{
				this -> logInterval = json["logInterval"];
			}
			else
			{
				this -> logInterval = this -> logInterval;
			}
			json["logInterval"] = this -> logInterval;

			//parsing sensorJsonSize key
			if (json["sensorJsonSize"].success())
			{
				this -> sensorJsonSize = json["sensorJsonSize"];
			}
			else
			{
				this -> sensorJsonSize = this -> sensorJsonSize;
			}
			json["sensorJsonSize"] = this -> sensorJsonSize;
			
			//parsing answerJsonSize key			
			if (json["answerJsonSize"].success())
			{
				this -> answerJsonSize = json["answerJsonSize"];
			}
			else
			{
				this -> answerJsonSize = this -> answerJsonSize;
			}
			json["answerJsonSize"] = this -> answerJsonSize;
			
			//parsing ireneActive key			
			if (json["ireneActive"].success())
			{
				this -> ireneActive = json["ireneActive"];
			}
			else
			{
				this -> ireneActive = this -> ireneActive;
			}
			json["ireneActive"] = this -> ireneActive;
			
			//parsing jetpackActive key
			if (json["jetpackActive"].success())
			{
				this -> jetpackActive = json["jetpackActive"];
			}
			else
			{
				this -> jetpackActive = this -> jetpackActive;
			}
			json["jetpackActive"] = this -> jetpackActive;

			//parsing externalSensorsActive key
			if (json["externalSensorsActive"].success())
			{
				this -> externalSensorsActive = json["externalSensorsActive"];
			}
			else
			{
				this -> externalSensorsActive = this -> externalSensorsActive;
			}
			json["externalSensorsActive"] = this -> externalSensorsActive;

			//parsing serverTimeOut key
			if (json["serverTimeOut"].success())
			{
				this -> serverTimeOut = json["serverTimeOut"];
			}
			else
			{
				this -> serverTimeOut = this -> serverTimeOut;
			}
			json["serverTimeOut"] = this -> serverTimeOut;
			
			//parsing sleepActive key
			if (json["sleepActive"].success())
			{
				this -> sleepActive = json["sleepActive"];
			}
			else
			{
				this -> sleepActive = this -> sleepActive;
			}
			json["sleepActive"] = this -> sleepActive;

			//saving the current/correct configuration
			configFile.close();
			configFile = SPIFFS.open("/coolBoardConfig.json", "w");
			if (!configFile)
			{
			
			#if DEBUG == 1

				Serial.println("failed to write to /coolBoardConfig.json");
				Serial.println();
			
			#endif
 
				return(false);
			}

			json.printTo(configFile);
			configFile.close();
			return(true);
		}
	}
}

/**
*
*	CoolBoard::printConf():
*	This method is provided to print
*	the configuration to the Serial
*	Monitor.
*/
void CoolBoard::printConf()
{

#if DEBUG == 1
	
	Serial.println("Entering CoolBoard.printConf() ");
	Serial.println();

#endif

	Serial.println("Printing Cool Board Configuration ");
	Serial.print("log interval 		: ");
	Serial.println(this->logInterval);

	Serial.print("sensor json size 		: ");
	Serial.println(this->sensorJsonSize);

	Serial.print("answer json size 		: ");
	Serial.println(this->answerJsonSize);

	Serial.print("irene active 		: ");
	Serial.println(this->ireneActive);

	Serial.print("jetpack active		: ");
	Serial.println(this->jetpackActive);

	Serial.print("external sensors active 	: ");
	Serial.println(this->externalSensorsActive);

	Serial.print("access point timeOut 	: ");
	Serial.println(this->serverTimeOut);

	Serial.print("sleept active 		: ");
	Serial.println(this->sleepActive);

	Serial.print("user active 		: ");
	Serial.println(this->userActive);

	Serial.println();




}

/**
*	CoolBoard::update(mqtt answer):
*	This method is provided to handle the
*	configuration update of the different parts
*/
void CoolBoard::update(const char * answer)
{

#if DEBUG == 1

	Serial.println("Entering CoolBoard.update() ");
	Serial.println();
	Serial.println("message is : ");
	Serial.println(answer);
	Serial.println();

#endif

	DynamicJsonBuffer jsonBuffer(answerJsonSize);
	JsonObject & root = jsonBuffer.parseObject(answer);
	JsonObject & stateDesired = root["state"];
	if (stateDesired.success())
	{
	
	#if DEBUG == 1

		Serial.println("update message parsing : success");
		Serial.println();
	
	#endif

		if (stateDesired["update"] == 1)
		{
			String answerDesired;
		
			stateDesired.printTo(answerDesired);
			
		#if DEBUG == 1		
		
			Serial.println("update is 1 ");
			Serial.println("desired update is : ");			
			Serial.println(answerDesired);
			Serial.println();
		
		#endif
			
			fileSystem.updateConfigFiles(answerDesired, answerJsonSize);

			//applying the configuration	
			this -> config();

			coolBoardSensors.config();

			rtc.config();

			coolBoardLed.config();

			mqtt.config();

			if (jetpackActive)
			{
				jetPack.config();
			}

			if (ireneActive)
			{
				irene3000.config();
			}

			if (externalSensorsActive)
			{
				externalSensors.config();
			}

			delay(10);
			mqtt.begin();

		        //answering the update msg:
			//reported = received configuration
			//desired=null
		
			String updateAnswer;
			String tempString;
			
			stateDesired.printTo(tempString);
			updateAnswer="{\"state\":{\"reported\":";
			updateAnswer+=tempString;
			updateAnswer+=",\"desired\":null}}";

		#if DEBUG == 1

			Serial.println("preparing answer message ");
			Serial.println();
			Serial.println("updateAnswer : ");
			Serial.println(updateAnswer);
		
		#endif	

			mqtt.publish(updateAnswer.c_str());
			
			mqtt.mqttLoop();

			delay(10);
			
			//restart the esp
			ESP.restart();
				
		}
		else
		{
		
		#if DEBUG == 1 

			Serial.println("update is not 1 ");
			Serial.println();
		
		#endif

		}
	}
	else
	{
	
	#if DEBUG == 1

		Serial.println("Failed to parse update message( OR no message received )");
		Serial.println();
	
	#endif
	
	}		
}

/**
*	CoolBoard::getLogInterval():
*	This method is provided to get
*	the log interval
*
*	\return interval value in ms
*/
uint16_t CoolBoard::getLogInterval()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoard.getLogInterval() ");
	Serial.println();
	Serial.println("log Interval is :");
	Serial.println(logInterval);
	Serial.println();

#endif

	return(this -> logInterval);
}

/**
*	CoolBoard::readSensors():
*	This method is provided to read and
*	format all the sensors data in a
*	single json.
*	
*	\return	json string of all the sensors read.
*/
String CoolBoard::readSensors()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoard.readSensors()");
	Serial.println();

#endif

	String sensorsData;

	sensorsData = coolBoardSensors.read(); // {..,..,..}
	
	if (externalSensorsActive)
	{
		sensorsData += externalSensors.read(); // {..,..,..}{..,..}

		sensorsData.setCharAt(sensorsData.lastIndexOf('}'), ','); // {..,..,..}{..,..,
		sensorsData.setCharAt(sensorsData.lastIndexOf('{'), ','); // {..,..,..},..,..,
		sensorsData.remove(sensorsData.lastIndexOf('}'), 1); // {..,..,..,..,..,
		sensorsData.setCharAt(sensorsData.lastIndexOf(','), '}'); // {..,..,..,..,..}

	}
	if (ireneActive)
	{
		sensorsData += irene3000.read(); // {..,..,..,..,..}{..,..,..}

		sensorsData.setCharAt(sensorsData.lastIndexOf('}'), ','); // {..,..,..,..,..{..,..,..,
		sensorsData.setCharAt(sensorsData.lastIndexOf('{'), ','); // {..,..,..,..,..},..,..,..,
		sensorsData.remove(sensorsData.lastIndexOf('}'), 1); // {..,..,..,..,..,..,..,..,
		sensorsData.setCharAt(sensorsData.lastIndexOf(','), '}'); // {..,..,..,..,..,..,..,..}
		
	}
	
#if DEBUG == 1

	Serial.println("sensors data is ");
	Serial.println(sensorsData);
	Serial.println();

#endif

	return(sensorsData);

}

/**
*	CoolBoard::userData():
*	This method is provided to
*	return the user's data.
*	
*	\return json string of the user's data
*/
String CoolBoard::userData()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoard.userData() ");
	Serial.println();

#endif

	String tempMAC = WiFi.macAddress();

	tempMAC.replace(":", "");

	String userJson = "{\"user\":\"";

	userJson += mqtt.getUser();

	userJson += "\",\"timestamp\":\"";

	userJson += rtc.getESDate(); // "timestamp":"20yy-mm-ddThh:mm:ssZ"

	userJson += "\",\"mac\":\"";

	userJson += tempMAC;

	userJson += "\"}";

#if DEBUG == 1

	Serial.println("userData is : ");
	Serial.println(userJson);
	Serial.println();

#endif	
	
	return(userJson);
	
}


/**
*	CoolBoard::sleep(int interval):
*	This method is provided to allow the
*	board to enter deepSleep mode for
*	a period of time equal to interval in ms 
*/
void CoolBoard::sleep(int interval)
{

#if DEBUG == 1

	Serial.println("Entering CoolBoard.sleep() ");
	Serial.print("going to sleep for ");Serial.print(interval);Serial.println("ms");
	Serial.println();

#endif

	ESP.deepSleep ( ( interval * 1000 ), WAKE_RF_DEFAULT) ;
}
