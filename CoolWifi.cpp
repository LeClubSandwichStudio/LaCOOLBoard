/**
*	\file CoolWifi.cpp
*	\brief CoolWifi Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/


#include "FS.h"
#include "Arduino.h"  
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <CoolWifi.h>
#include "ArduinoJson.h"



#define DEBUG 1

#ifndef DEBUG

#define DEBUG 0

#endif




/**
*	CoolWifi::begin():
*	This method is provided to set the
*	wifiMulti Access points and the 
*	wifiManager time out 
*/
void CoolWifi::begin()
{ 

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.begin()") );
	Serial.println();

#endif
	for(int i =0;i<this->wifiCount;i++)
	{
		 this->wifiMulti.addAP(this->ssid[i].c_str() , this->pass[i].c_str() );	
	}
	
	this->wifiManager.setTimeout(this->timeOut);	
	
}

/**
*	CoolWifi::state():
*	This method is provided to return the 
*	Wifi client's state.
*	\return wifi client state:	
*		WL_NO_SHIELD        = 255, 
*    		WL_IDLE_STATUS      = 0,
*    		WL_NO_SSID_AVAIL    = 1,
*    		WL_SCAN_COMPLETED   = 2,
*    		WL_CONNECTED        = 3,
*    		WL_CONNECT_FAILED   = 4,
*    		WL_CONNECTION_LOST  = 5,
*		WL_DISCONNECTED = 6
*/
wl_status_t CoolWifi::state()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.state()") );
	Serial.println();	
	Serial.print( F("state : ") );
	Serial.println( WiFi.status() );

#endif
	
	return( WiFi.status() ) ;
}

/**
*	CoolWifi::connect( ):
*	This method is provided to connect to the strongest WiFi
*	in the provided list of wiFis.
*	If none are found , it starts the AP mode.
*	
*	\return wifi state
*/
wl_status_t CoolWifi::connect()
{       

	int i=0;

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.connect()") );
	Serial.println( F("Wifi connecting...") );
	
	Serial.println("entry time to multi : ");
	Serial.println(millis() ) ;

#endif
	//Wifi MULTI

	while( (wifiMulti.run() != WL_CONNECTED) && (i<1000)  ) 
	{

	#if DEBUG == 1

        	Serial.print(".");
		i++;
		delay(10);
	
	#endif

    	}	

#if DEBUG == 1 

	Serial.println();	
	Serial.println("exit point from multi : ");
	Serial.println(millis() );


#endif

	//Wifi Manager
	if( (i>=1000) ||  (WiFi.status() != WL_CONNECTED) ) 
	{
	
	#if DEBUG == 1 
		
		Serial.println(F("No matching wifi Found ") );
		Serial.println( F("Starting Access Point ") );	
		Serial.println();
	
	#endif
		if(!wifiManager.autoConnect("CoolBoardAP")) 
		{
		
		#if DEBUG == 1

			Serial.println( F("failed to connect and hit timeout") );
		
		#endif
			delay(300);

		} 

		  //if you get here you have connected to the WiFi
		#if DEBUG == 1

			Serial.println( F("connected...yeey :)" ));
			Serial.println("connected to ");
			Serial.println( WiFi.SSID() );
			Serial.println( WiFi.psk() ) ;
			
		#endif
			this->addWifi( WiFi.SSID() , WiFi.psk() );
		
	}
	else
	{

	#if DEBUG == 1

		Serial.println("connected to ");
		Serial.println( WiFi.SSID() );
		Serial.println( WiFi.psk() ) ;
				
	#endif
	
	}
	
	return( WiFi.status() ) ;

}

/**
*	CoolWifi::config():
*	This method is provided to set
*	the wifi parameters :	-ssid
*				-pass
*				-AP timeOut
*				-wifiCount	
*
*	\return true if successful,false otherwise
*/
bool CoolWifi::config()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.config()") );
	Serial.println();

#endif

	//read config file
	//update data
	File configFile = SPIFFS.open("/wifiConfig.json", "r");

	if (!configFile) 
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to read /wifiConfig.json") );
		Serial.println();

	#endif

		return(false);
	}
	else
	{
		size_t size = configFile.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		configFile.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
		
		#if DEBUG == 1 

			Serial.println( F("failed to parse json ") );
			Serial.println();
		
		#endif
			
			return(false);
		} 
		else
		{
		
		#if DEBUG == 1 
		
			Serial.println( F("configuration json is ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();


		#endif
			//wifiCount
			if(json["wifiCount"].success() )
			{			
				this->wifiCount=json["wifiCount"];
			}
			else
			{
				this->wifiCount=this->wifiCount;
			}
			json["wifiCount"]=this->wifiCount;

			
			//AP timeOut
			if(json["timeOut"].success() )
			{
				this->timeOut=json["timeOut"];
			}
			else
			{
				this->timeOut=this->timeOut;

			}
			json["timeOut"]=this->timeOut;
			
			
			//Wifis SSID and PASS
			for(int i =0; i<this->wifiCount ;i++)
			{
				if ( json["Wifi"+String(i)].success() )
				{
					
					if( json["Wifi"+String(i)]["ssid"].success() )
					{
						const char* tempSsid=json["Wifi"+String(i)]["ssid"]; 
						this->ssid[i]=tempSsid;					
					}
					else
					{
						this->ssid[i]=this->ssid[i];					
					}
					json["Wifi"+String(i)]["ssid"]=this->ssid[i].c_str();
					
					
					if( json["Wifi"+String(i)]["pass"].success() )
					{
						const char* tempPass =json["Wifi"+String(i)]["pass"];
						this->pass[i]=tempPass ;					
					}
					else
					{
						this->pass[i]=this->pass[i];					
					}
					json["Wifi"+String(i)]["pass"]=this->pass[i].c_str();			
				
				}
				else
				{
					
					this->ssid[i]=this->ssid[i];
					this->pass[i]=this->pass[i];					
					
				}
				json["Wifi"+String(i)]["ssid"]=this->ssid[i].c_str();
				json["Wifi"+String(i)]["pass"]=this->pass[i].c_str();			
						
			}

			configFile.close();
			configFile = SPIFFS.open("/wifiConfig.json", "w");
			if(!configFile)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /wifiConfig.json") );
			
			#endif

				return(false);				
			}
			
			json.printTo(configFile);
			configFile.close();

		#if DEBUG == 1 

			Serial.println( F("saved configuration is :") );
			json.printTo(Serial);
			Serial.println();
		
		#endif

			return(true); 
		}
	}	
	

}

/**
*	CoolWifi::config(ssid array, pass array, number of wifis, AP timeout );
*	This method is provided to configure the Wifi without SPIFFS
*	
*	\return true if successfull, false otherwise
*/
bool CoolWifi::config(String ssid[],String pass[],int wifiNumber, int APTimeOut)
{

#if DEBUG == 1 
	
	Serial.println("Entering CoolWifi.config(), no SPIFFS variant ") ;
	
#endif
	
	if(wifiNumber>50)
	{
	
	#if DEBUG == 1 
		
		Serial.println("the limit of WiFis is 50 " );
		
	#endif
		return(false);	
	}

	this->wifiCount=wifiNumber;

	this->timeOut=APTimeOut;
	
	for(int i=0;i<wifiNumber;i++)
	{
		this->ssid[i]=ssid[i];
		
		this->pass[i]=pass[i];
	}
		
	return(true);

}


/**
*	CoolWifi::printConf():
*	This method is provided to print the
*	configuration to the Serial Monitor
*/
void CoolWifi::printConf()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolWifi.printConf()") );
	Serial.println();	

#endif
	
	Serial.println("Wifi configuration ");

	
	Serial.println("wifiCount : ");
	Serial.println(this->wifiCount);
	
	for(int i=0;i<this->wifiCount;i++)
	{	
		Serial.print("SSID");
		Serial.print(i);
		Serial.println(" : ");
		Serial.println(this->ssid[i]);
				
		Serial.print("PASS");
		Serial.print(i);
		Serial.println(" : ");
		Serial.println(this->pass[i]);
		
	}
	
	Serial.println("timeOut : ");
	Serial.println(this->timeOut);

	Serial.println();


}

/**
*	CoolWifi::addWifi(ssid,pass)	
*	This method is provided to add new WiFi
*	detected by the WiFiManager to 
*	the jsonConfig(if used ) 
*	
*	\return true if successfull , false otherwise
*/
bool CoolWifi::addWifi( String ssid , String pass )
{

#if DEBUG == 1
	
	Serial.println("Entering CoolWifi.addWifi() ") ;

#endif 	
	
	this->wifiCount++;
	if( this->wifiCount >=50)
	{
	
	#if DEBUG == 1

		Serial.println("You have reached the limit of 50");
		return(false);	
	
	#endif

	}

	this->ssid[this->wifiCount-1]=ssid;
	this->pass[this->wifiCount-1]=pass;
	
	//read config file
	//update data
	File configFile = SPIFFS.open("/wifiConfig.json", "r");

	if (!configFile) 
	{
	
	#if DEBUG == 1 

		Serial.println( F("failed to read /wifiConfig.json") );
		Serial.println();

	#endif
	}
	else
	{
		size_t size = configFile.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		configFile.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{
		
		#if DEBUG == 1 

			Serial.println( F("failed to parse json ") );
			Serial.println();
		
		#endif
		} 
		else
		{
		
		#if DEBUG == 1 
		
			Serial.println( F("configuration json is ") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();


		#endif
			//wifiCount
			if(json["wifiCount"].success() )
			{			
				json["wifiCount"]=this->wifiCount;
			}
			else
			{
				this->wifiCount=this->wifiCount;
			}
			json["wifiCount"]=this->wifiCount;

			
			//AP timeOut
			if(json["timeOut"].success() )
			{
				this->timeOut=json["timeOut"];
			}
			else
			{
				this->timeOut=this->timeOut;

			}
			json["timeOut"]=this->timeOut;
			
			
			//new Wifi SSID and PASS
			JsonObject& newWifi = json.createNestedObject( "Wifi"+String( this->wifiCount-1 ) );
			
			newWifi["ssid"] =this->ssid[this->wifiCount-1];
			newWifi["pass"] = this->pass[this->wifiCount-1];
			

			configFile.close();
			configFile = SPIFFS.open("/wifiConfig.json", "w");
			if(!configFile)
			{
			
			#if DEBUG == 1 

				Serial.println( F("failed to write to /wifiConfig.json") );
			
			#endif

			}
			
			json.printTo(configFile);
			configFile.close();

		#if DEBUG == 1 

			Serial.println( F("saved configuration is :") );
			json.printTo(Serial);
			Serial.println();
		
		#endif

			return(true); 
		}
	}	

	
	return(true);
	
}
