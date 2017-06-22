/*
*  CoolTime.cpp
*  
*  This class manages the DS1337 RTC .
*  
*  
*  
*  
*  
*  
*/
#include "FS.h"
#include "Arduino.h"
#include "DS1337.h"
#include "CoolTime.h"
#include "ArduinoJson.h"
#include "Time.h"

bool CoolTime::begin()
{
	bool trust = true;

	this->rtc.init();                                                               //initialise DS1337

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

void CoolTime::update()
{
	if(timeStatus() != timeNotSet )
	{

		rtc.setDateTime(this->getNtpTime());
	        this->timeSync=this->rtc.getTimestamp();
		this->rtc.clearOSF();                               //since the sync worked fine we reset eventual error flags in the RTC
	}
	
	
}

void CoolTime::setDateTime(int year, int month, int day, int hour, int minutes, int seconds)
{
            this->rtc.setDateTime( year,  month,  day,  hour,  minutes,  seconds);                                   //set RTC to new time



}

void CoolTime::getTimeDate(int &year, int &month, int &day, int &hour, int &minute, int &second)
{	
 DS1337::getTime(rtc.getTimestamp(),  year,  month,  day,  hour,  minute,  second);
	

}

unsigned long CoolTime::getLastSyncTime()
{
	return(this->timeSync);
}



bool CoolTime::isTimeSync(unsigned long seconds)
{
//default is once per week we try to get a time update
	if (this->getLastSyncTime() + seconds < rtc.getTimestamp()) 
	{           
		return(false);
	}

return(true);
}



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

// send an NTP request to the time server at the given address
void CoolTime::sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
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
void CoolTime::config(int timeZone,IPAddress timeServer,unsigned int localPort)
{
	this->timeZone=timeZone;
	this->timeServer=timeServer;
	this->localPort=localPort;
} 

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
		{  	
			
			if(json["timeZone"].success() )
			{
				this->timeZone=json["timeZone"] ;
			}
			
			if(json["timeServer"].success() )
			{			
				String ip=json["timeServer"];
				this->timeServer.fromString(ip);
 
			}
			
			if(json["localPort"].success() )
			{						
				this->localPort=json["localPort"];
			}
						
			return(true); 
		}
	}	



}

void CoolTime::printConf()
{
	Serial.println("RTC Config") ;
	Serial.println(timeZone);
	Serial.println(timeServer);
	Serial.println(localPort);
}
