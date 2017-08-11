#include<Irene3000.h>


Irene3000 irene;


void setup()
{
	pinMode(5,OUTPUT);

	digitalWrite(5,HIGH);

	Serial.begin(115200);

	irene.begin();
}

void loop()
{

	
	Serial.println(irene.readPh(irene.gainConvert(8)) );

	Serial.println(irene.readTemp(irene.gainConvert(8)));

	delay(2000);
}
