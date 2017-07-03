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
/**
*	CoolBoard::begin():
*	This method is provided to configure,
*	begin the used CoolKit Parts.
*	If Serial is enabled,it prints the configuration
*	of the used parts.
*/
void CoolBoard::begin()
{
	fileSystem.begin();
	
	coolBoardSensors.config();
	coolBoardSensors.begin();
	coolBoardSensors.printConf();

	rtc.config();
	rtc.begin();
	rtc.printConf();

	coolBoardLed.config();
	coolBoardLed.begin();
	coolBoardLed.printConf();

	mqtt.config();
	mqtt.begin();
	mqtt.printConf();

	if (jetpackActive)
	{
		jetPack.config();
		jetPack.begin();
		jetPack.printConf();
	}

	if (ireneActive)
	{
		irene3000.config();
		irene3000.begin();
		irene3000.printConf();
	}

	if (externalSensorsActive)
	{
		externalSensors.config();
		externalSensors.begin();
		externalSensors.printConf();
	}

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
	if (WiFi.status() != WL_CONNECTED)
	{
		wifiManager.setConfigPortalTimeout(this -> serverTimeOut);
		wifiManager.autoConnect("CoolBoard");
	}

	if (mqtt.state() != 0)
	{
		mqtt.connect(this -> getLogInterval());
	}

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
	data="";

	//clock update
	rtc.update();

	//read user data if user is active
	if(userActive)
	{
		//reading user data
		data=this->userData();//{"":"","":"","",""}

		//formatting json 
		data.setCharAt( data.lastIndexOf('}') , ',');//{"":"","":"","","",
		
				
		//read sensors data
		data+=this->readSensors();//{"":"","":"","","",{.......}

		

		//formatting json correctly
		data.remove(data.lastIndexOf('{'), 1);//{"":"","":"","","",.......}
				
	}	
	else
	{
		data=this->readSensors();//{..,..,..}
	}
	
	//do action
	if (jetpackActive)
	{
		jetPack.doAction(data.c_str(), sensorJsonSize);
	}
	
	//formatting data:
	String jsonData = "{\"state\":{\"reported\":";
	jsonData += data; // {"state":{"reported":{..,..,..,..,..,..,..,..}
	jsonData += " } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } }
	
	//publishing data	
	if( this->sleepActive==0)	
	{
		mqtt.publish( jsonData.c_str(), this->getLogInterval() );
	}
	else
	{
		mqtt.publish(jsonData.c_str());
	}

	//mqtt client loop to allow data handling
	mqtt.mqttLoop();

	//read mqtt answer
	answer = mqtt.read();
	
	//mqtt client loop to allow data handling
	mqtt.mqttLoop();

	//check if the configuration needs update 
	//and update it if needed 
	this -> update(answer.c_str());
	
	//send saved data if any
	if(fileSystem.isDataSaved())
	{
		mqtt.publish("sending saved data");
		
		data+=fileSystem.getSensorSavedData();//{..,..,..}

		//formatting data:
		String jsonData = "{\"state\":{\"reported\":";
		jsonData += data; // {"state":{"reported":{..,..,..,..,..,..,..,..}
		jsonData += " } }"; // {"state":{"reported":{..,..,..,..,..,..,..,..}  } }

		mqtt.publish( data.c_str() );
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
	//clock update
	rtc.update();
	
	//read user data if user is active
	if(userActive)
	{
		//reading user data
		data=this->userData();//{"":"","":"","",""}
		
		//formatting json correctly
		data.setCharAt(data.lastIndexOf('}'), ',');//{"":"","":"","","",
	}	
	
	//read sensors data
	data+=this->readSensors();//{"":"","":"","","",{.......}
	
	//formatting json correctly
	data.remove(data.lastIndexOf('{'), 1);//{"":"","":"","","",.......}

	//do action
	if (jetpackActive)
	{
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
	//open file system
	fileSystem.begin();
	
	//open configuration file
	File configFile = SPIFFS.open("/coolBoardConfig.json", "r");
	
	if (!configFile)

	{
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
			return(false);
		}

		else
		{	
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
	Serial.println("Cool Board Conf");
	Serial.println(this->logInterval);
	Serial.println(this->sensorJsonSize);
	Serial.println(this->answerJsonSize);
	Serial.println(this->ireneActive);
	Serial.println(this->jetpackActive);
	Serial.println(this->externalSensorsActive);
	Serial.println(this->serverTimeOut);
	Serial.println(this->sleepActive);
	Serial.println(this->userActive);
	Serial.println(" ");




}

/**
*	CoolBoard::update(mqtt answer):
*	This method is provided to handle the
*	configuration update of the different parts
*/
void CoolBoard::update(const char * answer)
{
	DynamicJsonBuffer jsonBuffer(answerJsonSize);
	JsonObject & root = jsonBuffer.parseObject(answer);
	JsonObject & stateDesired = root["state"]["desired"];
	if (stateDesired.success())
	{
		if (stateDesired["update"] == 1)
		{
			String answerDesired;

			stateDesired.printTo(answerDesired);

			Serial.println(answerDesired);
			
			bool result = fileSystem.updateConfigFiles(answerDesired, answerJsonSize);

			Serial.println("update : ");

			Serial.println(result);

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
			root["state"]["reported"]=stateDesired;
			root["state"]["desired"]="null";
			
			String updateAnswer;
			root.printTo(updateAnswer);

			bool success=mqtt.publish(updateAnswer.c_str());
			
			mqtt.mqttLoop();

			delay(10);

			Serial.print("success: ");Serial.println(success);
			
			//restart the esp
			ESP.restart();
				
		}
	}
	else
	{
		Serial.println("Failed to parse update message ");	
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
	String tempMAC = WiFi.macAddress();

	tempMAC.replace(":", "");

	String userJson = "{\"user\":\"";

	userJson += mqtt.getUser();

	userJson += "\",";

	userJson += rtc.getESDate(); // "timestamp":"20yy-mm-ddThh:mm:ssZ"

	userJson += ",\"mac\":\"";

	userJson += tempMAC;

	userJson += "\"}";	
	
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
	ESP.deepSleep ( ( interval * 1000 ), WAKE_RF_DEFAULT) ;
}
