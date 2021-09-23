#include "SIM800.h"
#include "RCSwitch.h"

#define LED_PIN 5
#define TRANSMITTER_PIN 6

SIM800<HardwareSerial> SIM(&Serial);
RCSwitch radioTransmitter = RCSwitch();

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
   Serial.println(F("AT+DDET=1,0,1")); //Enable DTMF key duration info. AT&W_SAVE //Doesn't properly save on R13.08. Got 1,0,0 instead of 1,0,1 - duration missing after reboot.

   Serial.println(F("AT+CLIP=1")); //Enable the calling line identity (CLI) NO_SAVE
   //Serial.println(F("AT+CMEE=1")); //Error reporting (1 - Enable +CME ERROR: <err> result code and use numeric) AT&W_SAVE
   //Serial.println(F("AT+CMGF=1")); //Select SMS Message Format (1 - Text mode) AT&W_SAVE
   //Serial.println(F("ATV0")); //Status reporting (ATV0 - digital codes, ATV1 - text) AT&W_SAVE
   //Serial.println(F("ATE0")); //Disable command echo to terminal AT&W_SAVE

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
    Serial.println(F("ATA")); //Off-hook (answer)
    Serial.println(F("AT+VTS=\"1,5,9\"")); //Play DTMF to the line
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

    switch (key) {
        case '*':
            Serial.println(F("ATH")); //Hang Up call
            break;
        case '1': //Channel A
            radioTransmitter.sendTriState("0FF1FFF01000");
            break;
        case '2': //Channel B
            radioTransmitter.sendTriState("0FF1FFF00001");
            break;
        case '3': //Channel C
            radioTransmitter.sendTriState("0FF1FFF00010");
            break;
        case '4': //Channel D
            radioTransmitter.sendTriState("0FF1FFF00100");
            break;
    }

}
