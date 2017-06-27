/*
*
*	CoolBoardSensors.cpp
*	This class handles the On-Board Sensors. 
*
*
*/
#include "FS.h"
#include "Arduino.h"
#include <stdint.h>        
#include "Wire.h"
#include "ArduinoJson.h"
#include "CoolBoardSensors.h"


/**
*	CoolBoardSensors::CoolBoardSensors():
*	This Constructor is provided to start
*	the I2C interface and Init the different
*	used pins
*/
CoolBoardSensors::CoolBoardSensors()
{
	Wire.begin(2, 14);                       //I2C init Maybe change this to the CoolBoard?

	pinMode(AnMplex, OUTPUT);                //Declare Analog Multiplexer OUTPUT
	pinMode(EnMoisture, OUTPUT);             //Declare Moisture enable Pin
	pinMode(EnI2C, OUTPUT);		   //Declare I2C Enable pin 


}

/**
*	CoolBoardSensors::getJsonSize():
*	This method is provided to get
*	the sensor board answer size
*/
int CoolBoardSensors::getJsonSize()
{
	return(this->jsonSize );
}

/**
*	CoolBoardSensors::setJsonSize( JSON size):
*	This method is provided to set the
*	sensor board answer size
*/
void CoolBoardSensors::setJsonSize(int jsonSize)
{
	this->jsonSize=jsonSize;
}

/**
*	CoolBoardSensors::allActive():
*	This method is provided to allow
*	activation of all the sensor board sensors
*	without passing by the configuration file/method
*/
void CoolBoardSensors::allActive()
{
	lightDataActive.visible=1;
	lightDataActive.ir=1;
	lightDataActive.uv=1;	

	airDataActive.temperature=1;
	airDataActive.humidity=1;
	airDataActive.pressure=1;


	vbatActive=1;
	earthMoistureActive=1;


}


/**
*	CoolBoardSensors::begin():
*	This method is provided to start the
*	sensors that are on the sensor board
*/
void CoolBoardSensors::begin()
{       
	initReadI2C();

	while (!lightSensor.Begin()) {
  	  Serial.println("Si1145 is not ready!  1 second");
  	  delay(1000);
  	}
	 
	this->setEnvSensorSettings();
	delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
	this->envSensor.begin();
	delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
	Serial.println(envSensor.begin(), HEX);


}

/**
*	CoolBoardSensors::end():
*	This method is provided to end
*	the sensors on the sensor board
*/
void CoolBoardSensors::end()
{

	lightSensor.DeInit();

}

/**
*	CoolBoardSensors::read():
*	This method is provided to return the
*	data read by the sensor board
**/
String CoolBoardSensors::read()
{
	String data;
	DynamicJsonBuffer  jsonBuffer(sensorJsonSize) ;
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
		root["ultraViolet"] =lightSensor.ReadUV()/100 ;
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
	if(earthMoistureActive)
	{	
		root["soilMoisture"]=this->readMoisture();
	}
	
	
	root.printTo(data);
	


	return(data);
	

}

/**
*	CoolBoardSensors::initReadI2C():
*	This method is provided to enable the I2C
*	Interface on the sensor board.
*/
void CoolBoardSensors::initReadI2C()
{
  
	digitalWrite(EnI2C,HIGH);//HIGH= I2C Enable

}

/**
*	CoolBoardSensors::stopReadI2C():
*	This method is provided to disable the I2C
*	Interface on the sensor board
*/
void CoolBoardSensors::stopReadI2C()
{

	digitalWrite(EnI2C,LOW);//HIGH= I2C Enable

}


/**
*	CoolBoardSensors::config():
*	This method is provided to configure the
*	sensor board :	-activate   1
*			-deactivate 0
*/
bool CoolBoardSensors::config()
{
	//read config file
	//update data
	File coolBoardSensorsConfig = SPIFFS.open("/coolBoardSensorsConfig.json", "r");

	if (!coolBoardSensorsConfig) 
	{
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
			  return(false);
		} 
		else
		{  	  
			if(json["sensorJsonSize"].success() )
			{
				this->sensorJsonSize = json["sensorJsonSize"]; 
			}
			else
			{
				this->sensorJsonSize=this->sensorJsonSize;			
			}
			json["sensorJsonSize"]=this->sensorJsonSize;

			
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
				this->earthMoistureActive= json["soilMoisture"];
			}
			else
			{
				this->earthMoistureActive=this->earthMoistureActive;
			}
			json["soilMoisture"]=this->earthMoistureActive;

			coolBoardSensorsConfig.close();			
			coolBoardSensorsConfig = SPIFFS.open("/coolBoardSensorsConfig.json", "w");			
			if(!coolBoardSensorsConfig)
			{
				return(false);			
			}  

			json.printTo(coolBoardSensorsConfig);
			coolBoardSensorsConfig.close();			
			
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
	Serial.println("Sensors Conf ");
	Serial.print(sensorJsonSize);
	Serial.println(airDataActive.temperature);
	Serial.println(airDataActive.humidity);
	Serial.println(airDataActive.pressure);

	Serial.println(lightDataActive.visible);
	Serial.println(lightDataActive.ir);
	Serial.println(lightDataActive.uv);
	Serial.println(vbatActive);
	Serial.println(earthMoistureActive);
	Serial.println(" ");
}


/**
*	CoolBoardSensors::setEnvSensorSetting():
*	This method is provided to set the enviornment
*	sensor settings , if argument is ommitted , default value will be assigned
*/
void CoolBoardSensors::setEnvSensorSettings( uint8_t commInterface, uint8_t I2CAddress,    

						   uint8_t runMode , uint8_t tStandby, uint8_t filter, 						   
						   uint8_t tempOverSample,  uint8_t pressOverSample,    							   
						   uint8_t humidOverSample)
{
  envSensor.settings.commInterface = commInterface;      
  
  envSensor.settings.I2CAddress = I2CAddress;
  
  envSensor.settings.runMode = runMode; 
  
  envSensor.settings.tStandby = tStandby; 
  
  envSensor.settings.filter = filter; 
  
  envSensor.settings.tempOverSample = tempOverSample;
  
  envSensor.settings.pressOverSample = pressOverSample;
  
  envSensor.settings.humidOverSample = humidOverSample;

}

/**
*	CoolBoardSensors::readVBat():
*	This method is provided to read the
*	Battery Voltage.
*/	
float CoolBoardSensors::readVBat()
{
	digitalWrite(AnMplex, LOW);                                  //Enable Analog Switch to get the batterie tension
  	
	delay(200);
  	
	int raw = analogRead(A0);                                    //read in batterie tension
 	
	float val = 6.04 / 1024 * raw;                               //convert it apprimatly right tension in volts

	return (val);	
}

/**
*	CoolBoardSensors::readMoisture():
*	This method is provided to red the
*	Soil Moisture
*/
float CoolBoardSensors::readMoisture()
{
	  digitalWrite(EnMoisture, LOW);                 //enable moisture sensor and waith a bit
  	  
	  digitalWrite(AnMplex, HIGH);			//enable analog Switch to get the moisture
  	  
	  delay(2000);
  	  
	  int val = analogRead(A0);                       //read the value form the moisture sensor
  	  
	  float result = (float)map(val, 0, 890, 0, 100);	

	  digitalWrite(EnMoisture, HIGH);                  //disable moisture sensor for minimum wear
  	  
	  return (result);
}

