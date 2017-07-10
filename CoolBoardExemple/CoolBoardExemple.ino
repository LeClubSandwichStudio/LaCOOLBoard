
#include <CoolBoard.h>


CoolBoard coolBoard;


void setup()
{
	Serial.begin(115200);
	coolBoard.config();
	
<<<<<<< HEAD
	coolBoard.begin();

  	coolBoard.printConf();

=======
	coolBoard.begin();	
	
>>>>>>> 022ecfccbd47fa628feca52fc7c1ee420c585e2b
	Serial.print("one log every ");
	Serial.print(coolBoard.getLogInterval()/1000);
	Serial.println(" s " );
}

void loop()
{
	if(coolBoard.connect()==0 )
	{
		coolBoard.onLineMode();
	}
	else
	{
		coolBoard.offLineMode();
	}


 
}
