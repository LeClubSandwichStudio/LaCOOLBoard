
#include <CoolBoard.h>


CoolBoard coolBoard;

unsigned long current=0,previous=0;

void setup()
{
  Serial.begin(115200);
  coolBoard.config();
  coolBoard.printConf();
	coolBoard.begin();	
  coolBoard.connect(); 
  current=coolBoard.getDelay();
  Serial.print("one log every ");
  Serial.print(current/1000);
  Serial.println(" s " );
}

void loop()
{
if(current-previous>25000)
{
  Serial.print("current Timer :  ");
  Serial.println(current);  

  previous=current;
 if(coolBoard.connect()==0 )
 {
  coolBoard.onLineMode();
  }
  else
  {
    coolBoard.offLineMode();
    }
   
}
current=millis();
 
}
