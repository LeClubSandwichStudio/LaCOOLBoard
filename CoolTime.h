/*
*  CoolTime.h
*  
*  This class manages the DS1337 RTC .
*  
*  
*  
*  
*  
*  
*/

#ifndef CoolTime_H
#define CoolTime_H


#include "Arduino.h"
#include "DS1337.h"
#include <WiFiUdp.h>

#define NTP_PACKET_SIZE  48 // NTP time is in the first 48 bytes of message

class CoolTime
{

public:
	bool begin();
	
	void update();
	
	bool config();
	void config(int timeZone,IPAddress timeServer,unsigned int localPort); 
	void printConf();

	void setDateTime(int year, int month, int day, int hour, int minutes, int seconds);
	
	void getTimeDate(int &year, int &month, int &day, int &hour, int &minute, int &second);
	
	unsigned long getLastSyncTime();
	

	
	bool isTimeSync(unsigned long seconds=604800);

	time_t getNtpTime();

	void sendNTPpacket(IPAddress &address);
private:

	DS1337 rtc;
	
	unsigned long timeSync;
	
	int timeZone;
	
	IPAddress timeServer; // time-a.timefreq.bldrdoc.gov
	
	WiFiUDP Udp;
	
	unsigned int localPort;  // local port to listen for UDP packets

	
	
	byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets


};

#endif
