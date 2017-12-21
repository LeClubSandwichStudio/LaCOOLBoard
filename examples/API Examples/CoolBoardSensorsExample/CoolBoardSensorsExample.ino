/**
*	CoolBoardSensorsExample	
*
*	This example shows basic use
*	of the SensorsBoard.
*
*/
#include<CoolBoardSensors.h>

CoolBoardSensors sensors;

void setup()
{
	

	Serial.begin(115200);

	sensors.allActive();

	sensors.begin();


}

void loop()
{
	Serial.println("sensors data is ");
	Serial.println(sensors.read());
	delay(1000);
		
	Serial.println("individual readings");
	
	Serial.print("Vbat : ");Serial.println(sensors.readVBat());
	
	Serial.print("Earth Moisture : ");Serial.println(sensors.readMoisture());
	
	Serial.print("Visible light : ");Serial.println(sensors.lightSensor.ReadVisible()) ;
	
	Serial.print("InfraRed light : ");Serial.println(sensors.lightSensor.ReadIR() );
		
	Serial.print("Ultraviolet light: ");Serial.println(sensors.lightSensor.ReadUV()/100 );
	
	Serial.print("Pressure : ");Serial.println(sensors.envSensor.readFloatPressure());
	
	Serial.print("Air Humidity");Serial.println(sensors.envSensor.readFloatHumidity()) ;
	
	Serial.print("Temperature :");Serial.print(sensors.envSensor.readTempC());Serial.println("°C");
	
	delay(1000);


}
