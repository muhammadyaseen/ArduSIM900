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

int numStartAddresses[] = { 10,21,32,43,54 };

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

void makeCall() {
	
        Serial.write("Initiating call...");
        
        String callTo = "";
        boolean endLoop = false;
        
        //TODO : call 1st,2nd... nth number
	
       for(int i = 1; i < 6; i++)
       {
         if ( endLoop ) {
           Serial.println(" breaking out of for loop ");
           break;  //if prev call was successfull, we can end loop   
       }
         
         // read ith num
         callTo = "ATD" + getNthNumber(i) + ";\r";
         
         //initialize call to ith num
         sim.write(callTo.c_str());
         
         delay(500);
               
         //create a local software serial event listener
         String response = "";
         
         while( true )
         {
           if ( sim.available() > 0)
           {
             response += (char)sim.read();
             Serial.println(response);
           }
           
           response.trim();
           
           if ( response.length() > 17 )
           {
             String code = response.substring(17);
             Serial.println(code);
             
             if ( code.equals("NOCARRIER") || code.equals("BUSY") || code.equals("NOANSWER") || code.equals("ERROR") )
             {
                   if ( code.equals("NOCARRIER") ) endLoop = true;
                   
                   Serial.print("last code: ");
                   Serial.println(code);
                   Serial.println(" breaking out of while true ");
                   break; 
             }
           }
           
           //Serial.println(response);
         }
       } 

       delay(3000);

       interruptInProcess = false;

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

boolean parseConfigMsg(String msg )
{
    //e.g. 03412260853;  11 chars
    
    //#@5@03212260953@#
    
    //#@<HowManyNums>@<Num>@#  
    // max N=5
    
    Serial.println("This is what I recvd");
    
    Serial.println(msg);
    
    if ( msg.startsWith("#@") && msg.endsWith("@#") )     //this is a config msg, we now extract the numbers
    {
        int numPos = msg.charAt(2) - '0';
        
        Serial.println("YES : This is a config SMS");
        Serial.print("Num pos is : ");
        Serial.println(numPos);
        
        String num = msg.substring(4,15); //extract the number
        Serial.print("Extracted number : ");
        Serial.print(num);
        
        //determine where to save this number
        switch (numPos)
        {
          case 1:
            saveToEEPROM(num, 10);
            break;
          case 2:
            saveToEEPROM(num, 21);
            break;
          case 3:
            saveToEEPROM(num, 32);
            break;
          case 4:
            saveToEEPROM(num, 43);
            break;
          case 5:
            saveToEEPROM(num, 54);
            break;
          default:
             Serial.println(" num range is 1-5 ");
             return false;
        }
        
        readEEPROMNums();
        
        //TODO : Send a confirmation back to user.
        String writtenNum = getNthNumber( numPos );
        Serial.print("Written number : ");
        Serial.print(writtenNum); 
        
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
  
  String sendersNum = "0"+configMsg.substring(10,20);
  
  Serial.println(sendersNum);
  
  String storedNum = readFirstNum();
 
   if ( storedNum.equals(sendersNum) )
  {
      Serial.println("OWNER's number... Config Numbers...");
  }
  else 
  {
    Serial.println("other source");
    return;
  }

  //We are interested in newline character bcz it follows msg body or <data> section of URC
  Serial.println( "nl at : ");
  
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
     delay(10);
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
   
   String num = "";
  
   Serial.println("reading ROM...");
  
  //Last num character is at 64
  
   while ( i < 21 )
   {
     char c = (char)EEPROM.read(i);
     num += c;
     i++;
   }
   
   Serial.print( " Stored number is : ");
   Serial.println(num);
   
   return num;
}

String getNthNumber(int n)
{
  String num = "";
  int i = numStartAddresses[n-1];
  int j = 0;
  
  Serial.println("reading ROM...");
  
  //Last num character is at 64
  
   while ( j < 11 )
   {
     char c = (char)EEPROM.read(i);
     num += c;
     i++;
     j++;
   }
   
  return num; 
}
