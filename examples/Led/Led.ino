/**
 *	CoolBoardExample
 *
 *	This example shows typical use
 *	of the CoolBoard.
 *
 *	Save this example in another location
 *	in order to safely modify the configuration files
 *	in the data folder.
 *
 */

#include <CoolBoardLed.h>

CoolBoardLed led;
void setup() {
  led.activate();
  led.begin();
}

void loop() {

  led.fade(255, 255, 255, 1); // fade animation(R,G,B,Time in seconds)

  led.write(0, 0, 0); // direct write to the led(R,G,B)
  delay(1000);

  led.blink(0, 200, 0, 1); // blink animation(R,G,B,Time in seconds)

  led.write(0, 0, 0);
  delay(1000);

  led.fadeIn(100, 100, 100, 1); // fadeIn animation(R,G,B,Time in seconds)

  led.write(0, 0, 0);
  delay(1000);

  led.fadeOut(10, 64, 20, 1); // fadeout animation(R,G,B,Time in seconds)

  led.write(0, 0, 0);
  delay(1000);

  led.strobe(200, 0, 200, 1); // strobe animation(R,G,B,Time in seconds)

  led.write(0, 0, 0);
  delay(1000);
}
