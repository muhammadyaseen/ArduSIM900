#include <SoftwareSerial.h>
#include <EEPROM.h>

/*
* constants here
*/

SoftwareSerial sim(10,11);    // rx,tx

boolean interruptInProcess	= false;
int baudRate 			= 9600;
boolean stringComplete          = false;
String  inputString = "";
char ctrlZ                    = '\x1A';
int buttonPin                 = 5; //btn is connected to this pin
int buttonState;             // the current reading from the input pin
int lastButtonState           = LOW;   // the previous reading from the input pin

long lastDebounceTime         = 0;  // the last time the output pin was toggled
long debounceDelay            = 50;    // the debounce time; increase if the output flickers

void setup()
{
        pinMode(buttonPin, INPUT);
        
	inputString.reserve(300);
        
        initSIM900();
        
	setParameters();

        //delay(1000);
        
        //sendSMS();
}

void loop()
{
        checkPushButton();
        
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

void checkPushButton()
{
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH && !interruptInProcess) {
        Serial.println("Push btn PRESSED");
        
        interruptInProcess = true;
        
        makeCall();
        
      }
      else 
      {
        Serial.println("Push btn RELEASED");
      }
    }
  }
  
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading; 
  
}
void setParameters()
{
	//can set pin output modes, default values here
      
        //set module in 'text' mode
        
        sim.write("AT+CMGF=1\r");
        
        delay(500);
        
        //Send SMS msgs to Arduino serial port  
        //AT+ACNMI=mode,mt,bm,ds,bfr
        sim.write("AT+CNMI=2,2,0,0,0\r");
        
        Serial.println("Parameter setting done...");
	
}


void initSIM900()
{
      
        Serial.begin(baudRate);
        
        sim.begin(baudRate);
        
	//wait a few seconds for sim900 power on sequence to complete
	delay(5000);
	
	//enter command mode
	//sim.write("+++\r");
	delay(500);

	//required for auto-bauding to figure out baud rate
	sim.write("AT\r");
	
	delay(2000);

        Serial.println("SIm900 initialization done...");

}

void preCallSetup() {

	//we can check the balance/credit info here to inform if there is not enough balance etc
}

void makeCall() {
	
        Serial.write("Initiating call...");
	
	sim.write("ATD+923412260853;\r");
	
	delay(3000);

        interruptInProcess = false;

}

void postCallSetup() {
	
	//we can process any log,credit,time info here
}

void sendSMS() {
  
        String msgText = "Hello user, This is a test message \r\n";
        
       // msgText = "AT+CMGS=\"+923122009338\"\r" + msgText + ctrlZ ;
        
        //Serial.println(msgText);
        
        //sim.write( msgText.c_str());
        sim.write("AT+CMEE=2\r");
        delay(500);
         sim.write("AT+CMEE?\r");
          delay(500);
          
        sim.print("AT+CMGS=\"+923412260853\"\r");
        delay(100);
        sim.print("Test");
        delay(100);
        sim.write(0x1A);
         delay(500);
}

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
        
        //TODO: Extract the SENDER's number 
        
        
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
        
        saveToEEPROM(num1, 10);
        
        saveToEEPROM(num2, 21);
        
        saveToEEPROM(num3, 32);
        
        saveToEEPROM(num4, 43);
        
        saveToEEPROM(num5, 54);
        
        //We read nums to verify that we saved the correct nums. Output is on SerialMonitor
        readEEPROMNums();
        
        //Send a confirmation back to user.
        
        /*String msgText = "Hello user, You have registered following numbers : \r\n";
        msgText += num1 + "\r\n";
        msgText += num2 + "\r\n";
        msgText += num3 + "\r\n";
        msgText += num4 + "\r\n";
        msgText += num5;
        
        msgText = "AT+CMGS=\"+923412260853\"\r" + ctrlZ + msgText ;
        
        Serial.println(msgText);
        
        delay(500);
        
        sim.write(msgText.c_str());
        
        delay(500);
        */
        return true;
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
  
  //TODO: 1. check from where the message came, 
  //            -- extract sender's number
  //            -- read first num in EEPOROM
  //            -- compare both
  
  String storedNum = readFirstNum();
  
  if ( storedNum.equals( sendersNum) )
  {
  
  }
  
  
  
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

String readFirstNum()
{
   int i = 10;
   
   String num == "";
  
   Serial.println("reading ROM...");
  
  //Last num character is at 64
  
   while ( i < 22 )
   {
     char c = (char)EEPROM.read(i);
     num += c;
     i++;
   }
   
   Serial.print( " Stored number is : ");
   Serial.println(num);
   
   return num;
}
