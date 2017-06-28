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
#include "DS1337.h"
#include "CoolTime.h"
#include "ArduinoJson.h"
#include "TimeLib.h"

/**
*	CoolTime::begin():
*	This method is provided to init the rtc,
*	the udp connection and the Sync Provider
*
*	\return true if successful,false otherwise
*/
bool CoolTime::begin()
{
	bool trust = true;

	this->rtc.init();                                                          //initialise DS1337

	if (!rtc.isRunning()) {                                                   //if ever the RTC is stopped
		trust = false;                                                          //clock is not ok
		this->rtc.start();

	}

	if (rtc.hasStopped()) {                                                   //if the clock has stoped one moment or another
		trust = false;
	}

	Udp.begin(localPort);
	
	setSyncProvider(std::bind(&CoolTime::getNtpTime,this));

	return(trust);

}

/**
*	CoolTime::update():
*	This method is provided to correct the
*	rtc Time when it drifts,once every week.
*/
void CoolTime::update()
{
	if(!this->isTimeSync() )
	{
		if(timeStatus() != timeNotSet )
		{

			rtc.setDateTime(this->getNtpTime());
	        	this->timeSync=this->rtc.getTimestamp();
			this->rtc.clearOSF();                         //since the sync worked fine we reset eventual error flags in the RTC
		}
	}
	
	
}

/**
*	CoolTime::setDateTime(year,month,dat,hour,minutes,seconds):
*	This method is provided to manually set the RTc Time
*
*/
void CoolTime::setDateTime(int year, int month, int day, int hour, int minutes, int seconds)
{
            this->rtc.setDateTime( year,  month,  day,  hour,  minutes,  seconds);                                   //set RTC to new time



}

/**
*	CoolTime::getTimeDate(year,month,day,hour,minute,seconds):
*	This method is provided to get the RTC Time
*/
void CoolTime::getTimeDate(int &year, int &month, int &day, int &hour, int &minute, int &second)
{	
 DS1337::getTime(rtc.getTimestamp(),  year,  month,  day,  hour,  minute,  second);


}

String CoolTime::getESDate()
{
	char yymmddhhmmss[] = "\"timestamp\":\"20yy-mm-ddT00:00:00Z\"";
	int year,month,day,hour,minute,second;
	this->getTimeDate(year,month,day,hour,minute,second);
	yymmddhhmmss[30] =  second / 10 + 48;
	yymmddhhmmss[31] = second % 10 + 48;
	yymmddhhmmss[27] = minute / 10 + 48;
	yymmddhhmmss[28] = minute % 10 + 48;
	yymmddhhmmss[24] = hour / 10 + 48;
	yymmddhhmmss[25] = hour % 10 + 48;
	yymmddhhmmss[21] = day / 10 + 48;
	yymmddhhmmss[22] = day % 10 + 48;
	yymmddhhmmss[18] = month / 10 + 48;
	yymmddhhmmss[19] = month % 10 + 48;
	yymmddhhmmss[15] = year / 10 + 48;
	yymmddhhmmss[16] = year % 10 + 48;
	return yymmddhhmmss;
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
	if (this->getLastSyncTime() + seconds < rtc.getTimestamp()) 
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

	this->sendNTPpacket(timeServer);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) 
	{
		int size = this->Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) 
		{
			this->Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
      			return secsSince1900 - 2208988800UL;


		}
	}

	return 0; // return 0 if unable to get the time
}

/**
*	CoolTime::sendNTPpacket( Time Server IP address):
*	This method is provided to send an NTP request to 
*	the time server at the given address
*/ 
void CoolTime::sendNTPpacket(IPAddress &address)
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request

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
