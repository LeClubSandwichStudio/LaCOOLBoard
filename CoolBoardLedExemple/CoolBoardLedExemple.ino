#include<CoolBoardLed.h>

CoolBoardLed led;
void setup()
{
	led.activate();
	led.begin();
}

void loop()
{

	led.fade(255, 255, 255,  1);
	
	led.write(0,0,0);
	delay(1000);		
	
	led.blink(0,200 , 0, 1);

	led.write(0,0,0);
	delay(1000);		

	led.fadeIn(100, 100, 100, 1);

	led.write(0,0,0);
	delay(1000);		

	led.fadeOut(10,64 ,20, 1);

	led.write(0,0,0);
	delay(1000);		

	led.strobe(200, 0, 200, 1); 

	led.write(0,0,0);
	delay(1000);		


}
