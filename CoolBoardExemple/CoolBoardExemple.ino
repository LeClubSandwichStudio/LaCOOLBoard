
#include <CoolBoard.h>


CoolBoard coolBoard;


void setup()
{
	Serial.begin(115200);
	coolBoard.config();
	
	coolBoard.begin();	
	coolBoard.connect(); 

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
