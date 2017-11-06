/**
*	\file	CoolTime.cpp
*  	\brief	CoolTime Source file
*	\version 1.0  
*	\author	Mehdi Zemzem
*	\version 0.0  
*	\author Simon Juif
*  	\date	27/06/2017
*	\copyright La Cool Co SAS 
*	\copyright MIT license
*	Copyright (c) 2017 La Cool Co SAS
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*
*/


#include "FS.h"

#include "Arduino.h"

#include "CoolTime.h"

#include "ArduinoJson.h"

#include "TimeLib.h"


#define DEBUG 0


/**
*	CoolTime::begin():
*	This method is provided to init
*	the udp connection 
*	
*/
void CoolTime::begin()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolTime.begin()") );
	Serial.println();

#endif 


	Udp.begin(localPort);
	
	this->update();
	
}

/**
*	CoolTime::update():
*	This method is provided to correct the
*	rtc Time when it drifts,once every week.
*/
void CoolTime::update()
{

#if DEBUG == 1

	Serial.println( F("Entering CoolTime.update()") );
	Serial.println();

#endif 

	if( !( this->isTimeSync() ) )
	{
	
	#if DEBUG == 1

		Serial.println( F("waiting for sync") );
		Serial.println();

	#endif 

		this->timeSync=this->getNtpTime();
		breakTime(this->getNtpTime(), this->tmSet);
		this->rtc.set(makeTime(this->tmSet), CLOCK_ADDRESS); // set the clock
		this->saveTimeSync();
	}
	
}

/**
*	CoolTime::setDateTime(year,month,dat,hour,minutes,seconds):
*	This method is provided to manually set the RTc Time
*
*/
void CoolTime::setDateTime(int year, int month, int day, int hour, int minutes, int seconds)
{ 

#if DEBUG == 1

	Serial.println( F("Entering CoolTime.setDateTime") );
	Serial.println();

#endif

	tmElements_t tm;
	tm.Second=seconds; 
	tm.Minute=minutes; 
	tm.Hour=hour; 
	tm.Day=day;
	tm.Month=month; 
	tm.Year=year;
	
	this->rtc.set(makeTime(tm),CLOCK_ADDRESS);   

#if DEBUG == 1

	Serial.print( F("setting time to : ") );//"20yy-mm-ddT00:00:00Z

	Serial.print(tm.Year);
	Serial.print( F("-") );
	Serial.print( this->formatDigits( tm.Month ) );
	Serial.print( F("-") );
	Serial.print( this->formatDigits( tm.Day ) );
	Serial.print( F("T") );
	Serial.print( this->formatDigits( tm.Hour ) );
	Serial.print( F(":") );
	Serial.print( this->formatDigits( tm.Minute ) );
	Serial.print( F(":") );
	Serial.print( this->formatDigits( tm.Second ) );
	Serial.print( F("Z") );

	Serial.println();
	
	Serial.print( F("time set to : ") );
	Serial.println(this->getESDate());
	Serial.println();

#endif

}

/**
*	CoolTime::getTimeDate():
*	This method is provided to get the RTC Time
*
*	\returns a tmElements_t structre that has
*	the time in it
*/
tmElements_t CoolTime::getTimeDate()
{

#if DEBUG == 1 
	
	Serial.println( F("Entering CoolTime.getTimeDate()") );
	Serial.println();

#endif

	tmElements_t tm;
	rtc.get(CLOCK_ADDRESS);		//experimental to prevent slow rtc data
	time_t timeDate = this->rtc.get(CLOCK_ADDRESS);
	breakTime(timeDate,tm);

#if DEBUG == 1
	
	Serial.print( F("time is : ") );
	Serial.print(tm.Year+ 1970 );
	Serial.print( F("-") );
	Serial.print( this->formatDigits( tm.Month ) );
	Serial.print( F("-") );
	Serial.print( this->formatDigits( tm.Day ) );
	Serial.print( F("T") );
	Serial.print( this->formatDigits( tm.Hour ) );
	Serial.print( F(":") );
	Serial.print( this->formatDigits( tm.Minute ) );
	Serial.print( F(":") );
	Serial.print( this->formatDigits( tm.Second ) );
	Serial.print( F("Z") );

#endif
	
	return(tm);
}

/**
*	CoolTime::getESD():
*	This method is provided to return an
*	Elastic Search compatible date Format
*	
*	\return date String in Elastic Search
*	format
*/
String CoolTime::getESDate()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolTime.getESDate()") );
	Serial.println();

#endif 

	tmElements_t tm=this->getTimeDate();

  	//"20yy-mm-ddT00:00:00Z"
	String elasticSearchString =String(tm.Year+1970)+"-"+this->formatDigits(tm.Month)+"-";

	elasticSearchString +=this->formatDigits(tm.Day)+"T"+this->formatDigits(tm.Hour)+":";
	
	elasticSearchString +=this->formatDigits(tm.Minute)+":"+this->formatDigits(tm.Second)+"Z";

#if DEBUG == 1 

	Serial.print( F("elastic Search date : ") );
	Serial.println(elasticSearchString);
	Serial.println();

#endif

	return (elasticSearchString);
}

/**
*	CoolTime::getLastSyncTime():
*	This method is provided to get the last time
*	we syncronised the time
*
*	\return unsigned long representation of
*	last syncronisation time in seconds 
*/	
unsigned long CoolTime::getLastSyncTime()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolTime.getLastSyncTime()") );
	Serial.println();
	
	Serial.print( F("last sync time : ") );
	Serial.println(this->timeSync);

#endif 

	return(this->timeSync);
}


/**
*	CoolTime::isTimeSync( time in seconds):
*	This method is provided to test if the
*	time is syncronised or not.
*	By default we test once per week.
*
*	\return true if time is syncronised,false
*	otherwise
*/
bool CoolTime::isTimeSync(unsigned long seconds)
{

#if DEBUG == 1

	Serial.println( F("Entering CoolTime.isTimeSync() ") );
	Serial.println();

#endif 

#if DEBUG == 0

	Serial.println( F("Check if Clock is ok and in sync..."));

#endif
	// expermental to prevent slow rtc data
	RTC.get(CLOCK_ADDRESS);
	unsigned long instantTime = RTC.get(CLOCK_ADDRESS);
	
#if DEBUG == 1
	Serial.print(F("Instant Time : "));
	Serial.println(instantTime);
	Serial.print(F("Last Sync    : "));
	Serial.println(this->getLastSyncTime());
	unsigned long testSync = instantTime - this->timeSync;
	Serial.print(F("Time since sy : "));
	Serial.println(testSync);
#endif

	//default is once per week we try to get a time update

	if(( instantTime - this->timeSync) > ( seconds ) ) 
	{

		Serial.println( F("time is not syncronised ") );
	
		return(false);	
	}
	
	Serial.println( F("time is syncronised : OK") );
	Serial.println();

	return(true);
}


/**
*	CoolTime::getNtopTime():
*	This method is provided to get the
*	Time through an NTP request to
*	a Time Server
*
*	\return a time_t (unsigned long ) timestamp in seconds
*/
time_t CoolTime::getNtpTime()
{

#if DEBUG == 1 

	Serial.println( F("Entering CoolTime.getNtpTime()") );
	Serial.println();

#endif 

	while (Udp.parsePacket() > 0) ; // discard any previously received packets

	WiFi.hostByName(timeServerName, timeServerIP); 

#if DEBUG == 1

	Serial.println(timeServerName);
	Serial.println(timeServerIP);

#endif

	Serial.println( F("Transmit NTP Request") );

	sendNTPpacket(timeServerIP);

	uint32_t beginWait = millis();

	while (millis() - beginWait < 2000) 
	{
		int size = Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) 
		{
		
		#if DEBUG == 1

			Serial.println( F("Receive NTP Response") );
		
		#endif

			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
		
		#if DEBUG == 1 
	
			Serial.print( F("received unix time : ") );
			Serial.println(secsSince1900 - 2208988800UL);
			Serial.println();

		#endif 

			return secsSince1900 - 2208988800UL ;
		}
	}
	
	Serial.println( F("No NTP Response :-(") );

	return 0; // return 0 if unable to get the time
}

/**
*	CoolTime::sendNTPpacket( Time Server IP address):
*	This method is provided to send an NTP request to 
*	the time server at the given address
*/ 
void CoolTime::sendNTPpacket(IPAddress &address)
{

#if DEBUG == 1 

	Serial.println( F("Enter CoolTime.sendNTPpacket()") );
	Serial.println();

#endif

	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:                 
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();	
}

/**
*	CoolTime::config(Time server IP , udp Port):
*	This method is provided to do manual configuration.
*	
*/
void CoolTime::config(IPAddress timeServer,unsigned int localPort)
{

#if DEBUG == 1 

	Serial.println( F("Enter CoomTime.config() , no SPIFFS variant ") );
	Serial.println();

#endif 

	this->timeServerIP=timeServerIP;
	this->localPort=localPort;
	
} 

/**
*	CoolTime::config():
*	This method is provided to configure
*	the CoolTime object through a configuration
*	file.
*
*	\return true if successful,false otherwise
*/
bool CoolTime::config()
{

#if DEBUG == 1 

	Serial.println( F("Enter CoolTime.config()") );
	Serial.println();

#endif 

	File rtcConfig = SPIFFS.open("/rtcConfig.json", "r");

	if (!rtcConfig) 
	{
	
		Serial.println( F("failed to read /rtcConfig.json") );
		Serial.println();

		return(false);
	}
	else
	{
		size_t size = rtcConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		rtcConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{

			Serial.println( F("failed to parse rtcConfig json") );
			Serial.println();

			return(false);
		} 
		else
		{  
		
		#if DEBUG == 1 

			Serial.println( F("configuration json is :") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();


		#endif

			String ip;
			
			if(json["timeServerIP"].success() )
			{			
				 ip=json["timeServerIP"].as<String>();
				this->timeServerIP.fromString(ip);
 				
			}
			else
			{
				this->timeServerIP=this->timeServerIP;
			}
			json["timeServerIP"]=ip;

			if(json["timeServerName"].success() )
			{				
				const char* tempServerName = json["timeServerName"]; 
				for(int i =0;i<50;i++)
				{
					timeServerName[i]=tempServerName[i];
				}
			}
			else
			{
				for(int i=0;i<50;i++)
				{
					this->timeServerName[i]=this->timeServerName[i];
				}				
			}
			json["timeServerName"]=this->timeServerName;
			
			if(json["localPort"].success() )
			{						
				this->localPort=json["localPort"];
			}
			else
			{
				this->localPort=this->localPort;
			}
			json["localPort"]=this->localPort;


			if( json["timeSync"].success() )
			{

				this->timeSync=json["timeSync"];
			}
			else
			{
				this->timeSync=this->timeSync;
			}
			json["timeSync"]=this->timeSync;

			rtcConfig.close();
			rtcConfig= SPIFFS.open("/rtcConfig.json", "w");
			
			if(!rtcConfig)
			{
			
			#if DEBUG == 1

				Serial.println( F("failed to write to /rtcConfig.json") );
				Serial.println();
			
			#endif

				return(false);
			}
			
			json.printTo(rtcConfig);
			rtcConfig.close();

		#if DEBUG == 1 

			Serial.println( F("configuration is :") );
			json.printTo(Serial);
			Serial.println();
		
		#endif
		
			return(true); 
		}
	}	



}

/**
*	CoolTime::saveTimeSync()
*	This method is provided to save
*	the last sync time in the 
*	SPIFFS.
*
*	\return true if successful,false
*	otherwise
*/
bool CoolTime::saveTimeSync()
{
	Serial.println( F("Enter CoolTime.saveTimeSync()") );
	Serial.println();

	File rtcConfig = SPIFFS.open("/rtcConfig.json", "r");

	if (!rtcConfig) 
	{
		Serial.println( F("failed to read /rtcConfig.json") );
		Serial.println();

		return(false);
	}
	else
	{
		size_t size = rtcConfig.size();
		// Allocate a buffer to store contents of the file.
		std::unique_ptr<char[]> buf(new char[size]);

		rtcConfig.readBytes(buf.get(), size);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.parseObject(buf.get());
		if (!json.success()) 
		{

			Serial.println( F("failed to parse json") );
			Serial.println();

			return(false);
		} 
		else
		{

		#if DEBUG == 1
  	
			Serial.println( F("configuration json is :") );
			json.printTo(Serial);
			Serial.println();

			Serial.print(F("jsonBuffer size: "));
			Serial.println(jsonBuffer.size());
			Serial.println();

		#endif

			String ip;
					
			if(json["timeServerIP"].success() )
			{			
				 ip=json["timeServerIP"].as<String>();
				this->timeServerIP.fromString(ip);
 				
			}
			else
			{
				this->timeServerIP=this->timeServerIP;
			}
			json["timeServerIP"]=ip;
			
			if(json["localPort"].success() )
			{						
				this->localPort=json["localPort"];
			}
			else
			{
				this->localPort=this->localPort;
			}
			json["localPort"]=this->localPort;


			if( json["timeSync"].success() )
			{
				json["timeSync"]=this->timeSync;
			}
			else
			{
				this->timeSync=this->timeSync;
			}
			json["timeSync"]=this->timeSync;


			rtcConfig.close();
			rtcConfig= SPIFFS.open("/rtcConfig.json", "w");
			
			if(!rtcConfig)
			{
			#if DEBUG == 1

				Serial.println( F("failed to write timeSync to /rtcConfig.json") );
				Serial.println();
			
			#endif

				return(false);
			}
			
			json.printTo(rtcConfig);
			rtcConfig.close();
	
		#if DEBUG == 1

			Serial.println( F("configuration is :") );
			json.printTo(Serial);
			Serial.println();
		
		#endif
			return(true); 
		}
	}	



}

/**
*	CoolTime::printConf():
*	This method is provided to print
*	the CoolTime configuration to the
*	Serial Monitor
*/
void CoolTime::printConf()
{

#if DEBUG == 1

	Serial.println(F("Entering CoolTime.printConf()"));
	Serial.println();

#endif 

	Serial.println(F("RTC Configuration"));

	Serial.print(F("timeServerName : "));
	Serial.println(timeServerName);

	Serial.print(F("timeServerIP : "));
	Serial.println(timeServerIP);
	
	Serial.print(F("localPort : :"));
	Serial.println(localPort);
}

/**
*	CoolTime::printDigits(digit)
*
*	utility method for digital clock display
*	adds leading 0
*	
*	\return formatted string of the input digit
*/
String CoolTime::formatDigits(int digits)
{

#if DEBUG == 1 

	//Serial.println( F("Entering CoolTime.formatDigits()") );
 	//Serial.println();

#endif 

	if(digits < 10)
	{
	
	#if DEBUG == 1

		//Serial.println( F("output digit : ") );
		//Serial.println( String("0") + String(digits) );

	#endif

		return( String("0") + String(digits) );
	}
	
#if DEBUG == 1 

	//Serial.println( F("output digit : ") );
	//Serial.println(digits);

#endif

	return( String(digits) );
}


