/*
* constants here
*/

bool interruptInProcess		= false;
baudRate 					= 115200;
networkLED					= PIN_13;
powerLED					= PIN_15;


void setup()
{
	
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
}

void setParameters()
{
	//can set pin output modes, default values here
	
	Serial.begin(baudRate);
	
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
	
	//switch to fixed bauding, and store the baud rate in non-volatile mem
	switch (baudRate)
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
	
}

void preCallSetup() {

	//we can check the balance/credit info here to inform if there is not enough balance etc
}

void makeCall() {
	
	Serial.write("AT+CHFA=0");
	
	Serial.write("ATL3");
	
	Serial.write("AT+CMUT=1");
	
	Serial.write("ATD+9234122xxxxx");
	
	delay(30000);
	
	Serial.write("ATH");
}

void postCallSetup() {
	
	//we can process any log,credit,time info here
}

void sendSMS() {}

void onSMSRecvd() {

	//called when sim900 receives an SMS
}

void saveNumInPhonebook() {
	
	Serial.write("AT+CPBS=\"SM\"");
	
	//saves the number at location LOC in memory with name NAME
	Serial.write("AT+CPBW=LOC,\"+3xxxxxxx\",92,\"NAME\");


}

void queryModuleInfo() {
	
	Serial.write("Firmware info : \n ");
	Serial.write("AT+GMR");
	
	Serial.write("IMEI : \n");
	Serial.write("AT+GSN");
	
	Serial.write("SIM number : \n");
	Serial.write("AT+CNUM?");
	
}

bool interruptReceived() {

	//TODO: implement switch check logic
	
	return true;

}

