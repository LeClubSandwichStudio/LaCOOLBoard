#include "FS.h"
#include <Adafruit_ADS1015.h>                                                                               // Necessary to run IRENE 3000*/
#include <Arduino.h>
#include <Irene3000.h>
#include "ArduinoJson.h"
//=============================================================================================================
// ----------------- > IRENE 3000 FUNCTIONS
//=============================================================================================================
void Irene3000::begin()
{
	this->ads.begin();
}

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

int Irene3000::readButton(adsGain_t gain)
{
	this->setGain(gain);
	return( this->ads.readADC_SingleEnded(button) );
	
}

void Irene3000::setGain(adsGain_t gain)
{
	this->ads.setGain(gain);
}

int Irene3000::readADSChannel2(adsGain_t gain)
{	
	this->setGain(gain);
	return( this->ads.readADC_SingleEnded(freeAdc) ) ;
}

float Irene3000::readPh(adsGain_t gain)
{
  this->setGain(gain);

  double Voltage =  gain * ( ads.readADC_SingleEnded(ph) ) / ADC_MAXIMUM_VALUE;

  float miliVolts = Voltage * 1000;
  float temporary = ((((vRef * (float)params.pH7Cal) / 32767) * 1000) - miliVolts) / opampGain;


 return( 7 - (temporary / params.pHStep) );

}

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

// pH 7 CALIBRATION ==================================================
void Irene3000::calibratepH7(adsGain_t gain)
{
  	this->setGain(gain);
        
	this->params.pH7Cal = ads.readADC_SingleEnded(ph);
 
	this->calcpHSlope();


}

// pH 4 CALIBRATION ==================================================
void Irene3000::calibratepH4(adsGain_t gain)
{

  this->setGain(gain);
        
  this->params.pH4Cal =  ads.readADC_SingleEnded(ph);
 
  this->calcpHSlope();



}

// pH SLOPE ==========================================================
void Irene3000::calcpHSlope ()
{

  params.pHStep = ((((vRef * (float)(params.pH7Cal - params.pH4Cal)) / 32767) * 1000) / opampGain) / 3;

 
}

void Irene3000::resetParams(void)
{
  //Restore to default set of parameters!
  params.WriteCheck = Write_Check;
  params.pH7Cal = 16384; //assume ideal probe and amp conditions 1/2 of 4096
  params.pH4Cal = 8192; //using ideal probe slope we end up this many 12bit units away on the 4 scale
  params.pHStep = 59.16;//ideal probe slope

  
}

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
