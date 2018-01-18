
#include <Arduino.h>
#define ADDR 0x18

class Gauges
{
	public:
		Gauges();
		void getAllValues();
		void resetGauge1();
		void resetGauge2();
		void resetGauge3();
		void resetAllValues();
		String readGauges();
		
		uint32_t readGauge1();
		uint32_t readGauge2();
		uint32_t readGauge3();

	private:
		uint8_t rawData[12];
		uint8_t rawData1[4];
		uint32_t gauge1 = 0;
		uint32_t gauge2 = 0;
		uint32_t gauge3 = 0;
		uint32_t int32_from_array();
};