/**
*	\file Irene3000.h
*	\brief Irene3000 Header File
*	\author Mehdi Zemzem
*	\version 1.0
*	\date 27/06/2017
*
*/


#ifndef IRENE3000_H
#define IRENE3000_H

#include "internals/CoolAdafruit_ADS1015.h"   
#include<Arduino.h>


#define ADC_MAXIMUM_VALUE     32767
#define REFERENCE_VOLTAGE_GAIN_4  1.024
#define V_GAIN_2 0.0625
#define V_GAIN_4 0.03125
#define V_GAIN_8 0.015625
#define Write_Check      0x1234

#define button 0
#define temp 1
#define freeAdc 2
#define ph 3

/**
*	\class Irene3000
*	\brief This class is provided to manage
*	the Irene3000 Ph/Temperature Shield
*
*/
class Irene3000
{
public:

	void begin();

	bool config();

	void printConf();

	String read();

	int readButton();

	void setGain(adsGain_t gain);

	int readADSChannel2(adsGain_t gain);

	float readPh( double t );

	double readTemp();

	void resetParams();

	void calibratepH7();

	void calibratepH4();

	void calcpHSlope ();

	adsGain_t gainConvert(uint16_t tempGain);
	
	bool saveParams();


private:

	Adafruit_ADS1115 ads;                                                                                           // ADC Object


	struct parameters_T
	{
		unsigned int WriteCheck=0;
		int pH7Cal, pH4Cal=0;
		float pHStep=1;
	}params;

	struct state
	{
		bool active=0;
		adsGain_t gain=GAIN_TWOTHIRDS;
		String type="";

	} waterTemp, phProbe,adc2;

	const float vRef = 1.024;                                                            //Our vRef into the ADC wont be exa

	const float opampGain = 5.25;                                 //what is our Op-Amps gain (stage 1)

};


#endif
