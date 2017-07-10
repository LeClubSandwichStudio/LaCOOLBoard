/**
*	\file CoolTime.h
*	\brief CoolTime Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/

#ifndef CoolTime_H
#define CoolTime_H


#include "Arduino.h"

#include "TimeLib.h"

#include <WiFiUdp.h>

#include <DS1337RTC.h>

#define NTP_PACKET_SIZE  48 // NTP time is in the first 48 bytes of message

/**
*	\class CoolTime
*  
*	\brief This class manages the DS1337 RTC .
*  
*/

class CoolTime
{

public:
	void begin();
	
	void update();
	
	bool config();

	void config(IPAddress timeServer,unsigned int localPort); 
	
	void printConf();

	void setDateTime(int year, int month, int day, int hour, int minutes, int seconds);
	
	tmElements_t getTimeDate();

	String getESDate();
	
	unsigned long getLastSyncTime();
	
	bool isTimeSync(unsigned long seconds=604800);

	time_t getNtpTime();

	void sendNTPpacket(IPAddress &address);
	
	String formatDigits(int digits);
	
	bool saveTimeSync();

private:
	
	unsigned long timeSync=0;
	
	IPAddress timeServer; // time-a.timefreq.bldrdoc.gov
	
	WiFiUDP Udp;
	
	unsigned int localPort=0;  // local port to listen for UDP packets

	byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
	
	tmElements_t tmSet;
	
	DS1337RTC rtc;

};

#endif
