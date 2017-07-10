/**
*	\file CoolBoardSensors.cpp
*	\brief CoolBoardSensors Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/
#include "FS.h"
#include "Arduino.h"
#include <stdint.h>        
#include "Wire.h"
#include "ArduinoJson.h"
#include "CoolBoardSensors.h"



#define DEBUG 1

#ifndef DEBUG

#define DEBUG 0

#endif




/**
*	CoolBoardSensors::CoolBoardSensors():
*	This Constructor is provided to start
*	the I2C interface and Init the different
*	used pins
*/
CoolBoardSensors::CoolBoardSensors()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoardSensors Constructor");
	Serial.println();

#endif
	
	Wire.begin(2, 14);                       //I2C init Maybe change this to the CoolBoard?

	pinMode(AnMplex, OUTPUT);                //Declare Analog Multiplexer OUTPUT
	pinMode(EnMoisture, OUTPUT);             //Declare Moisture enable Pin
	pinMode(EnI2C, OUTPUT);		   //Declare I2C Enable pin 


}

/**
*	CoolBoardSensors::getJsonSize():
*	This method is provided to get
*	the sensor board answer size
*	
*	\return json data size
*/
int CoolBoardSensors::getJsonSize()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoardSensors.getJsonSize()");
	Serial.println();
	Serial.print("json size is : ");
	Serial.println(this->jsonSize);
	Serial.println();

#endif

	return(this->jsonSize );
}

/**
*	CoolBoardSensors::setJsonSize( JSON size):
*	This method is provided to set the
*	sensor board answer size
*/
void CoolBoardSensors::setJsonSize(int jsonSize)
{

#if DEBUG == 1

	Serial.println("Entering CoolBoardSensors.setJsonSize()");
	Serial.println();	
	Serial.print("old json Size is : ");
	Serial.println(this->jsonSize);
#endif
		
	this->jsonSize=jsonSize;

#if DEBUG == 1 
	
	Serial.print("new json Size is : ");
	Serial.println(this->jsonSize);

#endif
	
}

/**
*	CoolBoardSensors::allActive():
*	This method is provided to allow
*	activation of all the sensor board sensors
*	without passing by the configuration file/method
*/
void CoolBoardSensors::allActive()
{

#if DEBUG == 1 

	Serial.println("Entering CoolBoardSensors.allActive()");
	Serial.println();

#endif
	
	this->lightDataActive.visible=1;
	this->lightDataActive.ir=1;
	this->lightDataActive.uv=1;	

	this->airDataActive.temperature=1;
	this->airDataActive.humidity=1;
	this->airDataActive.pressure=1;


	this->vbatActive=1;

	this->soilMoistureActive=1;
	


}


/**
*	CoolBoardSensors::begin():
*	This method is provided to start the
*	sensors that are on the sensor board
*/
void CoolBoardSensors::begin()
{  

#if DEBUG == 1 
     
	Serial.println("Entering CoolBoardSensors.begin()");
	Serial.println();

#endif

	initReadI2C();

	while (!lightSensor.Begin()) 
	{
	
	#if DEBUG == 1

		Serial.println("Si1145 is not ready!  1 second");

	#endif

		delay(1000);
  	}
	 
	this->setEnvSensorSettings();
	delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
	this->envSensor.begin();
	delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

#if DEBUG == 1 
	
	Serial.print("BME280 begin answer is :");
	Serial.println(envSensor.begin(), HEX);
	Serial.println();

#endif

}

/**
*	CoolBoardSensors::end():
*	This method is provided to end
*	the sensors on the sensor board
*/
void CoolBoardSensors::end()
{

#if DEBUG == 1 	
	Serial.println("Entering CoolBoardSensors.end()");
	Serial.println();

#endif

	lightSensor.DeInit();

}

/**
*	CoolBoardSensors::read():
*	This method is provided to return the
*	data read by the sensor board
*
*	\return a json string containing the 
*	sensors data
**/
String CoolBoardSensors::read()
{

#if DEBUG == 1 
	
	Serial.println("Entering CoolBoardSensors.read()");
	Serial.println();

#endif

	String data;
	DynamicJsonBuffer  jsonBuffer(jsonSize) ;
	JsonObject& root = jsonBuffer.createObject();
	
	initReadI2C();
	delay(100);
	//light data
	if(lightDataActive.visible)
	{
		root["visibleLight"] =lightSensor.ReadVisible() ;
	}
	
	if(lightDataActive.ir)
	{
		root["infraRed"] = lightSensor.ReadIR();
	}

	if(lightDataActive.uv)
	{
		float tempUV = (float)lightSensor.ReadUV()/100 ;
		root["ultraViolet"] = tempUV;
	}
	
	//BME280 data
	if(airDataActive.pressure)	
	{
		root["Pressure"] =envSensor.readFloatPressure();
	}
	
		
	if(airDataActive.humidity)	
	{	
		root["Humidity"] =envSensor.readFloatHumidity() ;
	}	
	
	if(airDataActive.temperature)
	{
		root["Temperature"]=envSensor.readTempC();
	}
	
	//Vbat
	if(vbatActive)	
	{	
		root["Vbat"]=this->readVBat();
	}
	
	//earth Moisture
	if(soilMoistureActive)
	{	
		root["soilMoisture"]=this->readMoisture();
	}
	
	
	root.printTo(data);

#if DEBUG == 1

	Serial.println("CoolBoardSensors data is :");
	root.printTo(Serial);
	Serial.println();

#endif

	return(data);	
	

}

/**
*	CoolBoardSensors::initReadI2C():
*	This method is provided to enable the I2C
*	Interface on the sensor board.
*/
void CoolBoardSensors::initReadI2C()
{

#if DEBUG == 1

 	Serial.println("Entering CoolBoardSensors.initReadI2C()");
	Serial.println();

#endif
 
	digitalWrite(EnI2C,HIGH);//HIGH= I2C Enable

}

/**
*	CoolBoardSensors::stopReadI2C():
*	This method is provided to disable the I2C
*	Interface on the sensor board
*/
void CoolBoardSensors::stopReadI2C()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoardSensors.stopReadI2C()");
	Serial.println();

#endif

	digitalWrite(EnI2C,LOW);//HIGH= I2C Enable

}


/**
*	CoolBoardSensors::config():
*	This method is provided to configure the
*	sensor board :	-activate   1
*			-deactivate 0
*
*	\return true if configuration is successful,
*	false otherwise
*/
bool CoolBoardSensors::config()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoardSensors.config()");
	Serial.println();

#endif

	//read config file
	//update data
	File coolBoardSensorsConfig = SPIFFS.open("/coolBoardSensorsConfig.json", "r");

	if (!coolBoardSensorsConfig) 
	{
	
	#if DEBUG == 1

		Serial.println("failed to read /coolBoardSensorsConfig.json");
		Serial.println();
	
	#endif

		return(false);
	}
	else
	{
		size_t size = coolBoardSensorsConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		coolBoardSensorsConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
		
		#if DEBUG == 1

			Serial.println("failed to parse coolBoardSensorsConfig json");
			Serial.println();
		
		#endif
	
			return(false);
		} 
		else
		{

		#if DEBUG == 1

			Serial.println("Configuration Json is :");
			json.printTo(Serial);
			Serial.println();
		
		#endif
  	  
			if(json["jsonSize"].success() )
			{
				this->jsonSize = json["jsonSize"]; 
			}
			else
			{
				this->jsonSize=this->jsonSize;			
			}
			json["jsonSize"]=this->jsonSize;

			
			if(json["BME280"]["temperature"].success() )
			{			
				this->airDataActive.temperature=json["BME280"]["temperature"];
			}
			else
			{
				this->airDataActive.temperature=this->airDataActive.temperature;			
			}
			json["BME280"]["temperature"]=this->airDataActive.temperature;
			
			
			if(json["BME280"]["humidity"].success() )
			{			
			
				this->airDataActive.humidity=json["BME280"]["humidity"];
			}
			else
			{
				this->airDataActive.humidity=this->airDataActive.humidity;
			}
			json["BME280"]["humidity"]=this->airDataActive.humidity;
			
			
			if(json["BME280"]["pressure"].success() )
			{
				this->airDataActive.pressure=json["BME280"]["pressure"];
			}
			else
			{
				this->airDataActive.pressure=this->airDataActive.pressure;
			}
			json["BME280"]["pressure"]=this->airDataActive.pressure;

			
			if(json["SI114X"]["visible"].success() )
			{
				this->lightDataActive.visible=json["SI114X"]["visible"];
			}
			else
			{
				this->lightDataActive.visible=this->lightDataActive.visible;
			}
			json["SI114X"]["visible"]=this->lightDataActive.visible;
			
			
			if(json["SI114X"]["ir"].success() )
			{			
				this->lightDataActive.ir=json["SI114X"]["ir"];
			}
			else
			{
				this->lightDataActive.ir=this->lightDataActive.ir;
			}
			json["SI114X"]["ir"]=this->lightDataActive.ir;

			
			if(json["SI114X"]["uv"].success() )			
			{			
				this->lightDataActive.uv=json["SI114X"]["uv"];
			}
			else
			{
				this->lightDataActive.uv=this->lightDataActive.uv;
			}
			json["SI114X"]["uv"]=this->lightDataActive.uv;


			if(json["vbat"].success() )
			{
				this->vbatActive=json["vbat"];
			}
			else
			{
				this->vbatActive=this->vbatActive;
			}
			json["vbat"]=this->vbatActive;

			
			if(json["soilMoisture"].success() )
			{			
				this->soilMoistureActive= json["soilMoisture"];
			}
			else
			{
				this->soilMoistureActive=this->soilMoistureActive;
			}
			json["soilMoisture"]=this->soilMoistureActive;

			coolBoardSensorsConfig.close();			
			coolBoardSensorsConfig = SPIFFS.open("/coolBoardSensorsConfig.json", "w");			
			if(!coolBoardSensorsConfig)
			{
			
			#if DEBUG == 1

				Serial.println("failed to write to /coolBoardSensorsConfig.json");
				Serial.println();
			
			#endif

				return(false);			
			}  

			json.printTo(coolBoardSensorsConfig);
			coolBoardSensorsConfig.close();			
			
		#if DEBUG == 1

			Serial.println("Saved Configuration Json is : ");
			json.printTo(Serial);
			Serial.println();
		
		#endif

			return(true); 
		}
	}	

}


/**
*	CoolBoardSensors::printConf():
*	This method is provided to print the 
*	configuration to the Serial Monitor
*/
void CoolBoardSensors::printConf()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoardSensors.printConf()");
	Serial.println();

#endif

	Serial.println("Sensors Configuration : ");
	
	Serial.print("json size : ");
	Serial.println(this->jsonSize);

	Serial.print("airDataActive.temperature : ");
	Serial.println(this->airDataActive.temperature);

	Serial.print("airDataActive.humidity : ");
	Serial.println(airDataActive.humidity);

	Serial.print("airDataActive.pressure : ");
	Serial.println(airDataActive.pressure);

	Serial.print("lightDataActive.visible : ");
	Serial.println(lightDataActive.visible);

	Serial.print("lightDataActive.ir : ");
	Serial.println(lightDataActive.ir);

	Serial.print("lightDataActive.uv : ");
	Serial.println(lightDataActive.uv);
	
	Serial.print("vbatActive : ");
	Serial.println(vbatActive);

	Serial.print("soilMoitureActive : ");
	Serial.println(soilMoistureActive);

	Serial.println();
}


/**
*	CoolBoardSensors::setEnvSensorSetting():
*	This method is provided to set the enviornment
*	sensor settings , if argument is ommitted , default value will be assigned
*	
*/
void CoolBoardSensors::setEnvSensorSettings( uint8_t commInterface, uint8_t I2CAddress,    

						   uint8_t runMode , uint8_t tStandby, uint8_t filter, 						   
						   uint8_t tempOverSample,  uint8_t pressOverSample,    							   
						   uint8_t humidOverSample)
{

#if DEBUG == 1
	
	Serial.println("Entering CoolBoardSensors.setEnvSensorSettings()");
	Serial.println();

#endif
  
	this->envSensor.settings.commInterface = commInterface;      

	this->envSensor.settings.I2CAddress = I2CAddress;

	this->envSensor.settings.runMode = runMode; 

	this->envSensor.settings.tStandby = tStandby; 

	this->envSensor.settings.filter = filter; 

	this->envSensor.settings.tempOverSample = tempOverSample;

	this->envSensor.settings.pressOverSample = pressOverSample;

	this->envSensor.settings.humidOverSample = humidOverSample;

}

/**
*	CoolBoardSensors::readVBat():
*	This method is provided to read the
*	Battery Voltage.
*
*	\return a float representing the battery
*	voltage
*/	
float CoolBoardSensors::readVBat()
{

#if DEBUG == 1

	Serial.println("Entering CoolBoardSensors.readVBat()");
	Serial.println();

#endif

	digitalWrite(this->AnMplex, LOW);                            //Enable Analog Switch to get the batterie tension
  	
	delay(200);
  	
	int raw = analogRead(A0);                                    //read in batterie tension
 	
	float val = 6.04 / 1024 * raw;                               //convert it apprimatly right tension in volts
	
#if DEBUG == 1

	Serial.println("Vbat is : ");
	Serial.println(val);
	Serial.println();

#endif

	return (val);	
}

/**
*	CoolBoardSensors::readMoisture():
*	This method is provided to red the
*	Soil Moisture
*
*	\return a float represnting the
*	soil moisture
*/
float CoolBoardSensors::readMoisture()
{

#if DEBUG == 1
	
	Serial.println("Entering CoolBoardSensors.readMoisture()");
	Serial.println();
	
#endif

	digitalWrite(EnMoisture, LOW);                 //enable moisture sensor and waith a bit

	digitalWrite(AnMplex, HIGH);			//enable analog Switch to get the moisture

	delay(2000);

	int val = analogRead(A0);                       //read the value form the moisture sensor

	float result = (float)map(val, 0, 890, 0, 100);	

	digitalWrite(EnMoisture, HIGH);                  //disable moisture sensor for minimum wear
	
#if DEBUG == 1 

	Serial.println("Soil Moisture is : ");
	Serial.println(result);
	Serial.println();

#endif 

	return (result);
}

