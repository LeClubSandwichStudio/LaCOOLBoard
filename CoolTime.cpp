/**
*	\file CoolTime.cpp
*	\brief CoolTime Source File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/


#include "FS.h"

#include "Arduino.h"

#include "CoolTime.h"

#include "ArduinoJson.h"

#include "TimeLib.h"



/**
*	CoolTime::begin():
*	This method is provided to init the rtc,
*	the udp connection 
*
*	\return true if successful,false otherwise
*/
void CoolTime::begin()
{

	Udp.begin(localPort);

	time_t tm=getNtpTime();

	breakTime(tm, this->tmSet);//get NTP time

	this->rtc.set(makeTime(this->tmSet), CLOCK_ADDRESS); // set the clock
	
}

/**
*	CoolTime::update():
*	This method is provided to correct the
*	rtc Time when it drifts,once every week.
*/
void CoolTime::update()
{
	if( !( this->isTimeSync() ) )
	{
		Serial.println("waiting for sync");
		this->timeSync=this->getNtpTime();
		breakTime(this->getNtpTime(), this->tmSet);
		this->rtc.set(makeTime(this->tmSet), CLOCK_ADDRESS); // set the clock
	}
	
}

/**
*	CoolTime::setDateTime(year,month,dat,hour,minutes,seconds):
*	This method is provided to manually set the RTc Time
*
*/
void CoolTime::setDateTime(int year, int month, int day, int hour, int minutes, int seconds)
{ 
	tmElements_t tm;
	tm.Second=seconds; 
	tm.Minute=minutes; 
	tm.Hour=hour; 
	tm.Day=day;
	tm.Month=month; 
	tm.Year=year;   

	this->rtc.set(makeTime(tm),CLOCK_ADDRESS);
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
	tmElements_t tm;
	time_t timeDate = this->rtc.get(CLOCK_ADDRESS);
	breakTime(timeDate,tm);
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
	tmElements_t tm=this->getTimeDate();

  	//"20yy-mm-ddT00:00:00Z"
	String elasticSearchString =String(tm.Year+1970)+"-"+this->formatDigits(tm.Month)+"-";

	elasticSearchString +=this->formatDigits(tm.Day)+"T"+this->formatDigits(tm.Hour)+":";
	
	elasticSearchString +=this->formatDigits(tm.Minute)+":"+this->formatDigits(tm.Second)+"Z";
	
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
//default is once per week we try to get a time update
	if( (this->getLastSyncTime()+seconds) > (RTC.get(CLOCK_ADDRESS)) ) 
	{
		return(false);	
	}

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
	while (Udp.parsePacket() > 0) ; // discard any previously received packets
	
	Serial.println("Transmit NTP Request");

	sendNTPpacket(timeServer);

	uint32_t beginWait = millis();

	while (millis() - beginWait < 1500) 
	{
		int size = Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) 
		{
			Serial.println("Receive NTP Response");
			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			return secsSince1900 - 2208988800UL + this->timeZone * SECS_PER_HOUR;
		}
	}
	Serial.println("No NTP Response :-(");
	return 0; // return 0 if unable to get the time
}

/**
*	CoolTime::sendNTPpacket( Time Server IP address):
*	This method is provided to send an NTP request to 
*	the time server at the given address
*/ 
void CoolTime::sendNTPpacket(IPAddress &address)
{
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
*	CoolTime::config(time Zone, Time server IP , udp Port):
*	This method is provided to do manual configuration.
*	
*/
void CoolTime::config(int timeZone,IPAddress timeServer,unsigned int localPort)
{
	this->timeZone=timeZone;
	this->timeServer=timeServer;
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
	File rtcConfig = SPIFFS.open("/rtcConfig.json", "r");

	if (!rtcConfig) 
	{
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
			  return(false);
		} 
		else
		{  	String ip;
			
			if(json["timeZone"].success() )
			{
				this->timeZone=json["timeZone"] ;
			}
			else
			{
				this->timeZone=this->timeZone;			
			}
			json["timeZone"]=this->timeZone;
			
			if(json["timeServer"].success() )
			{			
				 ip=json["timeServer"].as<String>();
				this->timeServer.fromString(ip);
 				
			}
			else
			{
				this->timeServer=this->timeServer;
			}
			json["timeServer"]=ip;
			
			if(json["localPort"].success() )
			{						
				this->localPort=json["localPort"];
			}
			else
			{
				this->localPort=this->localPort;
			}
			json["localPort"]=this->localPort;

			rtcConfig.close();
			rtcConfig= SPIFFS.open("/rtcConfig.json", "w");
			
			if(!rtcConfig)
			{
				return(false);
			}
			
			json.printTo(rtcConfig);
			rtcConfig.close();
						
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
	Serial.println("RTC Config") ;
	Serial.println(timeZone);
	Serial.println(timeServer);
	Serial.println(localPort);
}

/**
*	CoolTime::printDigits(digit)
*
*	utility function for digital clock display
*	adds leading 0
*	
*	\return formatted string of the input digit
*/
String CoolTime::formatDigits(int digits)
{ 
	if(digits < 10)
	{
		return( String("0") + String(digits) );
	}
	return( String(digits) );
}
