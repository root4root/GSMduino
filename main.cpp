#include "SIM800.h"
#include "RCSwitch.h"

#define LED_PIN 5
#define TRANSMITTER_PIN 6

SIM800<HardwareSerial> SIM(&Serial);
RCSwitch radioTransmitter = RCSwitch();

int statusLED = 0;

void sim800IncomingCall(char *);
void sim800IncomingSMS(char *, char *);
void sim800IncomingDTMF(char, int);

void setup() {
   //Wait for SIM800 boot up for UART speed negotiation...
   //If not, there is no chance to communicate with module.
   delay(5000);

   radioTransmitter.enableTransmit(TRANSMITTER_PIN);
   radioTransmitter.setPulseLength(469); //Optional
   //radioTransmitter.setRepeatTransmit(15); //Optional. Default is 10

   Serial.begin(9600);
   Serial.println(F("AT"));
   SIM.waitResponse();
   //Serial.println(F("AT+DDET=1,0,1")); //Enable DTMF key duration info. AT&W_SAVE

   Serial.println(F("AT+CLIP=1")); //Enable the calling line identity (CLI) NO_SAVE
   //Serial.println(F("AT+CMEE=1")); //Error reporting (1 - Enable +CME ERROR: <err> result code and use numeric) AT&W_SAVE
   //Serial.println(F("AT+CMGF=1")); //Select SMS Message Format (1 - Text mode) AT&W_SAVE
   //Serial.println(F("ATV0")); //Status reporting (ATV0 - digital codes, ATV1 - text) AT&W_SAVE
   //Serial.println(F("ATE0")); //Disable command echo to terminal AT&W_SAVE

   pinMode(LED_PIN, OUTPUT);
   pinMode(TRANSMITTER_PIN, OUTPUT);

   SIM.setIncomingCallHandler(sim800IncomingCall);
   SIM.setIncomingSMSHandler(sim800IncomingSMS);
   SIM.setIncomingDTMFHandler(sim800IncomingDTMF);
}


void loop() {
    SIM.handle(); //Checking for unsolicited notification
}



//Handlers here
void sim800IncomingCall(char *number)
{
   Serial.println(F("ATH"));

   radioTransmitter.sendTriState("0FF1FFF00100");

   if (statusLED == 0) {
      digitalWrite(LED_PIN, HIGH);
      statusLED = 1;
   } else {
      digitalWrite(LED_PIN, LOW);
      statusLED = 0;
   }

   /*
    Serial.println(F("ATA")); //Off-hook (answer)
    Serial.println(F("AT+VTS=\"5,5,5\"")); //Play DTMF to the line

    if (strcmp(number, "71234567890") == 0) {
        //Do something...
        return;
    }
    */
}

void sim800IncomingSMS(char *number, char *text)
{
    char textCopy[strlen(text)]; //*text is just pointer to buffer. We have to save content quick to prevent overriding

    //Let's send message back for example (parrot mode)
    strcpy(textCopy, text);
    SIM.sendSMS(number, textCopy);

    Serial.println(F("AT+CMGDA=\"DEL ALL\"")); //Delete all SMS to prevent overflow.
}

void sim800IncomingDTMF(char key, int duration)
{
    if (key == '*') {
        Serial.println(F("ATH")); //Hang Up call
    }
    //... some logic here ...
}
