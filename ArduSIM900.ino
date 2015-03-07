#include <EEPROM.h>




/*
* constants here
*/

bool interruptInProcess		= false;
int baudRate 					= 115200;
int networkLED					= 13;
int powerLED					= 15;
boolean stringComplete           = false;
String  inputString = "";
String testNums = "#@+923412260853@+923212260853@";

int numCountAddr = 0;

int numAddr = 10;


void setup()
{
	inputString.reserve(300);

	setParameters();
	
	initSIM900();
}

void loop()
{

	if ( interruptReceived() && !interruptInProcess)
	{	
		interruptInProcess = true;
		
		preCallSetup();
		
		makeCall();
		
		postCallSetup();
		
		interruptInProcess = false;
	}

        if ( stringComplete ) 
        {
          Serial.println(inputString);
          inputString = "";
          stringComplete = false;
        }
}

void setParameters()
{
	//can set pin output modes, default values here
	
	Serial.begin(baudRate);
      
        //set module in 'text' mode
        
        Serial.write("AT+CMGF=1\r");
        //Send SMS msgs to Arduino serial port
        
        //AT+ACNMI=mode,mt,bm,ds,bfr
        Serial.write("AT+CNMI=2,2,0,0,0\r");
	
}


void initSIM900()
{
	//wait a few seconds for sim900 power on sequence to complete
	delay(5000);
	
	//enter command mode
	Serial.write("+++");
	
	//required for auto-bauding to figure out baud rate
	Serial.write("AT");
	
	delay(3000);
	
        //Lets try autobauding for now
	//switch to fixed bauding, and store the baud rate in non-volatile mem
	/*switch (baudRate)
	{
		case 9600:
			Serial.write("AT+IPR=9600;&W");
			
		case 38400:
			Serial.write("AT+IPR=38400;&W");
		
		case 115200:
			Serial.write("AT+IPR=115200;&W");
		
		default:
			Serial.write("AT+IPR=9600;&W");
	}
	
	delay(2000);
	*/
}

void preCallSetup() {

	//we can check the balance/credit info here to inform if there is not enough balance etc
}

void makeCall() {
	
	Serial.write("AT+CHFA=0\r");
	
	Serial.write("ATL3\r");
	
	Serial.write("AT+CMUT=1\r");
	
	Serial.write("ATD+9234122xxxxx\r");
	
	delay(30000);
	
	Serial.write("ATH\r");
}

void postCallSetup() {
	
	//we can process any log,credit,time info here
}

void sendSMS() {}

void SMSRecvd() {

	//called when sim900 receives an SMS
}

void saveNumInPhonebook() {
	
	Serial.write("AT+CPBS=\"SM\"\r");
	
	//saves the number at location LOC in memory with name NAME
	Serial.write("AT+CPBW=LOC,\"+3xxxxxxx\",92,\"NAME\"\r");

}

void queryModuleInfo() {
	
	Serial.write("Firmware info : \n ");
	Serial.write("AT+GMR\r");
	
	Serial.write("IMEI : \n");
	Serial.write("AT+GSN\r");
	
	Serial.write("SIM number : \n");
	Serial.write("AT+CNUM?\r");
	
}

boolean interruptReceived() {

	//TODO: implement switch check logic
	
	return true;

}

boolean saveNumInEEPROM(String num)
{
    //e.g. +923412260853;  13 chars
    
    if ( num.charAt(0) == '#' && num.charAt(1) == '@' ) 
    {
        int numCount = int( num.charAt(3) );
        
        for ( int n = 0; n < numCount; n++ )
        {
           String currentNumber = "";
           
            while ( num.charAt(k) != '@')
            {
                  
            }
        }
    }
    
    else 
    {
       return false; 
    }

}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}

