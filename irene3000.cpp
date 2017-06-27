/**
*	\file Irene3000.cpp
*	\brief Irene3000 Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/


#include "FS.h"
#include <Adafruit_ADS1015.h>                                                                              
#include <Arduino.h>
#include <Irene3000.h>
#include "ArduinoJson.h"

/**
*	Irene3000::begin():
*	This method is provided to start the
*	Irene3000 ADS chip
*/
void Irene3000::begin()
{
	this->ads.begin();
}

/**
*	Irene3000:read():
*	This method is provided to read
*	the Irene3000 sensors data
*
*	\return json string of the sensors
*	data
*/
String Irene3000::read()
{	
	String data;
	DynamicJsonBuffer jsonBuffer(ireneJsonSize);
	JsonObject& root = jsonBuffer.createObject();


		
	if(waterTemp.active)
	{
		root["waterTemp"] = this->readTemp(waterTemp.gain);

		if(phProbe.active)
		{
			root["ph"] =this->readPh(phProbe.gain) ;
		}

	}

	if(adc2.active)
	{
		root[adc2.type] =this->readADSChannel2(adc2.gain);
	}
	
	root.printTo(data);
	
	return(data);
	
	

}

/**
*	Irene3000::config():
*	This method is provided to configure the
*	Irene3000 shield through a configuration file
*
*	\return true if successful,false otherwise
*/
bool Irene3000::config()
{

	File irene3000Config = SPIFFS.open("/irene3000Config.json", "r");

	if (!irene3000Config) 
	{
		return(false);
	}
	else
	{
		size_t size = irene3000Config.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);
	        uint16_t tempGain;
		irene3000Config.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
			  return(false);
		} 
		else
		{  	
			if(json["ireneJsonSize"].success() )
			{
				this->ireneJsonSize=json["ireneJsonSize"];
			}
			else
			{
				this->ireneJsonSize=this->ireneJsonSize;
			}
			json["ireneJsonSize"]=this->ireneJsonSize;

			
			if(json["waterTemp"]["active"].success() )
			{			
				this->waterTemp.active = json["waterTemp"]["active"]; 
			}
			else
			{
				this->waterTemp.active=this->waterTemp.active;
			}
			json["waterTemp"]["active"]=this->waterTemp.active;

			
			if(json["waterTemp"]["gain"].success() )
			{			
				tempGain = json["waterTemp"]["gain"]; 
				this->waterTemp.gain=this->gainConvert(tempGain);
			}
			else
			{
				this->waterTemp.gain=this->waterTemp.gain;
			}
			json["waterTemp"]["gain"]=this->waterTemp.gain;

			
			if(json["phProbe"]["active"].success())
			{
				this->phProbe.active=json["phProbe"]["active"];
			}
			else
			{
				this->phProbe.active=this->phProbe.active;
			}
			json["phProbe"]["active"]=this->phProbe.active;
	
			
			if(json["phProbe"]["gain"].success() )
			{		
				tempGain=json["phProbe"]["gain"];
				this->phProbe.gain=this->gainConvert(tempGain);			
			}
			else
			{
				this->phProbe.gain=this->phProbe.gain;
			}
			json["phProbe"]["gain"]=this->phProbe.gain;

			
			if(json["adc2"]["active"].success() )
			{
				this->adc2.active=json["adc2"]["active"];
			}
			else
			{
				this->adc2.active=this->adc2.active;
			}
			json["adc2"]["active"]=this->adc2.active;

			
			if(json["adc2"]["gain"].success() )
			{			
				tempGain=json["adc2"]["gain"];
				this->adc2.gain=this->gainConvert(tempGain);
			}
			else
			{
				this->adc2.gain=this->adc2.gain;
			}
			json["adc2"]["gain"]=this->adc2.gain;

			
			if(json["adc2"]["type"].success() )
			{
				this->adc2.type=json["adc2"]["type"].as<String>(); 
			}
			else
			{
				this->adc2.type=this->adc2.type;
			}
			json["adc2"]["type"]=this->adc2.type;

			irene3000Config.close();
			irene3000Config = SPIFFS.open("/irene3000Config.json", "w");

			if(!irene3000Config)
			{
				return(false);
			}

			json.printTo(irene3000Config);
			irene3000Config.close();

			return(true); 
		}
	}	

}

/**
*	Irene3000::printConf():
*	This method is provided to print the configuration
*	to the Serial Monitor
*/
void Irene3000::printConf()
{
	Serial.println("Irene Config ");
	Serial.println(waterTemp.active);
	Serial.println(waterTemp.gain);	
	Serial.println(phProbe.active);
	Serial.println(phProbe.gain);
	Serial.println(adc2.active);
	Serial.println(adc2.gain);
	Serial.println(adc2.type);
	Serial.println(" ");
}

/**
*	Irene3000::readButton(gain):
*	This method is provided to read the
*	Irene3000 button
*
*	\return the button value
*/
int Irene3000::readButton(adsGain_t gain)
{
	this->setGain(gain);
	return( this->ads.readADC_SingleEnded(button) );
	
}

/**
*	Irene3000::setGain(gain):
*	This method is provided to set the
*	ADS chip gain
*/
void Irene3000::setGain(adsGain_t gain)
{
	this->ads.setGain(gain);
}

/**
*	Irene3000::readADSChannel2(gain):
*	This method is provided to read from
*	the ADS channel 2 .
*	ADS Channel 2 is free and the user can connect
*	another analog sensor to it.
*
*	\return the ADS Channel 2 value
*/
int Irene3000::readADSChannel2(adsGain_t gain)
{	
	this->setGain(gain);
	return( this->ads.readADC_SingleEnded(freeAdc) ) ;
}

/**
*	Irene3000::readPh(gain):
*	This method is provided to read the PH probe
*	note that for the best results, PH must be 
*	correlated to Temperature.
*
*	\return the PH probe value
*/
float Irene3000::readPh(adsGain_t gain)
{
  this->setGain(gain);

  double Voltage =  gain * ( ads.readADC_SingleEnded(ph) ) / ADC_MAXIMUM_VALUE;

  float miliVolts = Voltage * 1000;
  float temporary = ((((vRef * (float)params.pH7Cal) / 32767) * 1000) - miliVolts) / opampGain;


 return( 7 - (temporary / params.pHStep) );

}

/**
*	Irene3000::readTemp(gain):
*	This method is provided to read
*	the Temeperature probe
*
*	\return the Temperature probe value
*/
double Irene3000::readTemp(adsGain_t gain)
{
  const double A = 3.9083E-3;
  const double B = -5.775E-7;
  double T;
  
  this->setGain(gain);
  int adc0 = ads.readADC_SingleEnded(temp);
 

  double R = ((adc0 * V_GAIN_8) / 0.095) / 1000;

  T = 0.0 - A;
  T += sqrt((A * A) - 4.0 * B * (1.0 - R));
  T /= (2.0 * B);
  if (T > 0 && T < 200) {
    return T;
  }
  else {
  
    T = 0.0 - A;
    T -= sqrt((A * A) - 4.0 * B * (1.0 - R));
    T /= (2.0 * B);
    return T;
  }

}


/**
*	Irene3000::calibratepH7(gain):
*	This method is provided to calibrate the
*	PH probe to 7
*/
void Irene3000::calibratepH7(adsGain_t gain)
{
  	this->setGain(gain);
        
	this->params.pH7Cal = ads.readADC_SingleEnded(ph);
 
	this->calcpHSlope();


}

/**
*	Irene3000::calibratepH4(gain):
*	This method is provided to calibrate the
*	PH probe to 4
*/
void Irene3000::calibratepH4(adsGain_t gain)
{

  this->setGain(gain);
        
  this->params.pH4Cal =  ads.readADC_SingleEnded(ph);
 
  this->calcpHSlope();



}

/**
*	Irene3000::calcpHSlop():
*	This method is provided to calculate
*	th PH slope
*/
void Irene3000::calcpHSlope ()
{

  params.pHStep = ((((vRef * (float)(params.pH7Cal - params.pH4Cal)) / 32767) * 1000) / opampGain) / 3;

 
}

/**
*	Irene3000::resetParams():
*	This method is provided to reset
*	the PH configuration, 
*	assuming Ideal configuration
*/
void Irene3000::resetParams(void)
{
  //Restore to default set of parameters!
  params.WriteCheck = Write_Check;
  params.pH7Cal = 16384; //assume ideal probe and amp conditions 1/2 of 4096
  params.pH4Cal = 8192; //using ideal probe slope we end up this many 12bit units away on the 4 scale
  params.pHStep = 59.16;//ideal probe slope

  
}

/**
*	Irene3000::gainConvert( gain : { 2/3,1,2,4,8,16 } )
*	This method is provided to convert the gain to
*	Internal Constants
*
*	\return internal representation of the ADS gain
*/
adsGain_t Irene3000::gainConvert(uint16_t tempGain)
{
	switch(tempGain)
	{
		case(2/3): return(GAIN_TWOTHIRDS);
		case(1): return (GAIN_ONE);
		case(2) : return(GAIN_TWO);
		case(4): return(GAIN_FOUR) ;   
		case(8):return(GAIN_EIGHT)  ;  
		case(16):return(GAIN_SIXTEEN); 	
	}



}
