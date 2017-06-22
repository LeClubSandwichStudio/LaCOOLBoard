#include<Jetpack.h>

Jetpack jetpack;
void setup()
{

	jetpack.begin();


}

void loop()
{

	jetpack.write(0xFF);			//writes to the Jetpack
	delay(1000);
	jetpack.writeBit(3,0); //writes to a single pin of the Jetpack
	delay(1000);

}
