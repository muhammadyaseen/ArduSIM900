#include <SoftwareSerial.h>
#include <EEPROM.h>

/*
* constants here
*/

SoftwareSerial sim(10,11);    // rx,tx

bool interruptInProcess		= false;
int baudRate 			= 9600;
boolean stringComplete          = false;
String  inputString = "";
String ctrlZ                    = "\x1A";
void setup()
{
	inputString.reserve(300);
        
        initSIM900();
        
	setParameters();
}

void loop()
{

	/*if ( interruptReceived() && !interruptInProcess)
	{	
		interruptInProcess = true;
		
		preCallSetup();
		
		makeCall();
		
		postCallSetup();
		
		interruptInProcess = false;
	}
        */
        if ( sim.available() > 0 )
        {
           softSerialEvent();
           
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
      
        //set module in 'text' mode
        
        sim.write("AT+CMGF=1\r");
        
        //Send SMS msgs to Arduino serial port  
        //AT+ACNMI=mode,mt,bm,ds,bfr
        sim.write("AT+CNMI=2,2,0,0,0\r");
        
        Serial.println("Parameter setting done...");
	
}


void initSIM900()
{
      
        Serial.begin(baudRate);
        
        sim.begin(9600);
        
	//wait a few seconds for sim900 power on sequence to complete
	delay(3000);
	
	//enter command mode
	sim.write("+++\r");
	delay(500);

	//required for auto-bauding to figure out baud rate
	sim.write("AT\r");
	
	delay(3000);

        Serial.println("SIm900 initialization done...");

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

boolean interruptReceived() {

	//TODO: implement switch check logic
	
	return true;

}

boolean parseConfigMsg(String msg )
{
    //e.g. 03412260853;  11 chars
    
    //#@5@03212260953@021356832@03453034303@03132260853@03334567798@#
    
    //#@<HowManyNums>@<Num 1>@<Num 2>@...@<Num N>@#  max N=5
    
    Serial.println("This is what I recvd");
    
    Serial.println(msg);
    
    if ( msg.startsWith("#@") && msg.endsWith("@#") )     //this is a config msg, we now extract the numbers
    {
        int numCount = int( msg.charAt(3) );
        
        Serial.println("YES : This is a config SMS");
        //each num is/should be 11 digits long. 
        
        //First num is from index 4 to 15
        String num1 = msg.substring(4,15);
        
        //Second num is from index 16 to 27
        String num2 = msg.substring(16,27);
        
        //Third num is from index 28 to 39
        String num3 = msg.substring(28,39);
        
        //Fourth num is from index 40 to 51
        String num4 = msg.substring(40,51);
        
        //Fifth num is from index 52 to 63
        String num5 = msg.substring(52,63);
        
        Serial.println(num1);
        Serial.println(num2);
        Serial.println(num3);
        Serial.println(num4);
        Serial.println(num5);
        
        //save numbers to Arduino's EEPROM
        
        /*saveToEEPROM(num1, 10);
        
        saveToEEPROM(num2, 21);
        
        saveToEEPROM(num3, 32);
        
        saveToEEPROM(num4, 43);
        
        saveToEEPROM(num5, 54);*/
        
        readEEPROMNums();
        
        //Send a confirmation back to user.
        
        String msgText = "Hello user, You have register following numbers : \r\n";
        msgText += num1 + "\r\n";
        msgText += num2 + "\r\n";
        msgText += num3 + "\r\n";
        msgText += num4 + "\r\n";
        msgText += num5;
        
        msgText = "AT+CMGS=\"+923412260853\"\r" + ctrlZ + msgText ;
        
        Serial.println(msgText);
        
        sim.write(msgText.c_str());
        
    }
    
    else 
    {  
       Serial.println("NO : This is NOT config SMS");
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

void handleConfigMsg(String configMsg)
{
  Serial.println("SMS Msg received...");

  // Example msg:
  //+CMT: "+923412260853","Yaseen","15/03/08,16:52:19+20"
  //#@03212260953@021356832@03453034303@03132260853@03334567798@#
  
  //We are interested in newline character bcz it follows msg body or <data> section of URC
  Serial.println( " Newline char found at : ");
  
  int nlIndex = configMsg.indexOf('\n');
  
  Serial.print( nlIndex );

  Serial.println(" Tranferring control to saveNumInEEPROM()");
  
  parseConfigMsg(configMsg.substring( nlIndex + 1));
  
}

void softSerialEvent() {
  
  inputString = "";
  
  while (sim.available()) {
    // get the new byte:
    char inChar = (char)sim.read(); 
    // add it to the inputString:
    inputString += inChar;
     
  }
  
  String trimmed = String(inputString);
  
  trimmed.trim();
  
  if ( trimmed.startsWith("+CMT:") ) 
  {
      Serial.println("Yes, its a SMS msg");
       handleConfigMsg( trimmed ); 
  } else 
  {
      Serial.println("NO SMS msg");
  }
  stringComplete = true;
  
}

void saveToEEPROM(String s, int startAddr)
{
  int i = 0;
  
  while ( i < 11 )
  {
     EEPROM.write(startAddr + i, s.charAt(i));
    i++; 
  }

}

//utility func, used to view contents of EEPROM
void readEEPROMNums()
{
   int i = 10;
   
   Serial.println("reading ROM...");
  
  //Last num character is at 64
  
   while ( i < 65 )
   {
     char c = (char)EEPROM.read(i);
     Serial.print(c);
     i++;
   }
  
  
}
