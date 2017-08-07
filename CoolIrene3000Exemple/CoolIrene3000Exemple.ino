#include<Irene3000.h>
#include<Wire.h>

Irene3000 irene;


void setup()
{
	Serial.begin(115200);

	Wire.begin(2,14);

	irene.begin();
}

void loop()
{

	
	Serial.println(irene.readPh(irene.gainConvert(8)) );

	Serial.println(irene.readTemp(irene.gainConvert(8)));

	delay(2000);
}
