#include "EEPROM.h"
#include "cc1101.h"

// The LED is wired to the Arduino Output 4 (physical panStamp pin 19)

// The syncWord of sender and receiver must be the same
byte syncWord = 199;
byte nodeAddress = 6;
byte hopAddress = 7;
byte forwardAddress =0;
byte sourceAddress = 0;
String inputString = "";                                                                  // a string to hold incoming data
boolean stringComplete = false; 
byte recAddr;
char addressCount[5];
byte messageCount[100];
byte msgCount[100];
byte buff[100];
int stringLength;
int packetlength = 0;
int indexNumber;
byte mode; 
char Word;

// The connection to the hardware chip CC1101 the RF Chip
CC1101 cc1101;

// a flag that a wireless packet has been received 
boolean packetAvailable = false;         

/**
* Let the LED Output blink one time.
* 
* With small pause after the blink to see two consecutive blinks. 
*/

/**
* Handle interrupt from CC1101 (INT0)
*/
void cc1101signalsInterrupt(void){
  // set the flag that a package is available
  packetAvailable = true;
}

/**
* The setup method gets called on start of the system.
*/
void setup()
{
  Serial.begin(38400);
  

  // initialize the RF Chip
  cc1101.init();
  cc1101.setSyncWord(&syncWord, false);
  
  // this device address need to match the target address in the sender
  cc1101.setDevAddress(nodeAddress, false);
  //cc1101.disableAddressCheck(); // you can skip this line, because the default is to have the address check enabled
  cc1101.enableAddressCheck(); // you can skip this line, because the default is to have the address check enabled
  cc1101.setRxState();
  inputString.reserve(200);
  // Enable wireless reception interrupt
  attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  Serial.print("ECE 190 PROJECT"); Serial.println(" v.1");
  Serial.println("MOBILE SENSOR NETWORK");
  Serial.println();
  Serial.println("NODE ADDRESS: " + String(nodeAddress));
  Serial.println("Modes:    1 - Direct Comm,    2 - Hop Comm");
  Serial.println("Communication Syntax: Mode,Address,Message");
  Serial.println("------------------------------------------");
}

/**
* The loop method gets called on and on after the start of the system.
*/
void loop()
{

  if(packetAvailable)
  {
    // clear the flag
    packetAvailable = false;
    
    CCPACKET packet;
    
    // Disable wireless reception interrupt
    detachInterrupt(0);
    
    if(cc1101.receiveData(&packet) > 0)
    {
      if (packet.crc_ok && packet.length > 1)
      {
         packetlength = packet.length;
         forwardAddress = packet.data[1];
         sourceAddress  = packet.data[2];
         for(int j=3; j<(packetlength); j++)
            {
              buff[j-3] = packet.data[j];
            }
         if(packet.data[0]==packet.data[1])
         {
          Serial.println("Message Received from Address: " + String(packet.data[2]));
          Serial.print("Message: ");
          for(int j=3; j<(packetlength); j++)
            {
            Word = char(packet.data[j]);
            Serial.print(Word);
            }
            Serial.print("\n");
            Serial.print("\n");
         }
        else
        {
          Serial.println("packetlength: " + String(packetlength));
          Serial.println("Message Received from Address: " + String(sourceAddress));
          Serial.println("Intended to Address: " + String(forwardAddress));
          Serial.println("Forwarding Data");
          sendPacket(forwardAddress,forwardAddress,sourceAddress,buff,packetlength);
        }
      }
    }    
        Word = '\0';
        packetlength = 0;
        attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  }

if(stringComplete)
  {
    detachInterrupt(0);
    stringLength = inputString.length();   
    indexNumber = inputString.indexOf(',');
     
    for(int x = 0;x <indexNumber;x++) 
    {
      mode = inputString[x] - 48;
    }
  
    for(int x = 2;x <(indexNumber+2);x++)
    {
      addressCount[x-2] = inputString[x];
      recAddr = addressCount[x-2] - 48;
    }
  
    for(int x = 4; x <(stringLength -1) ;x++) 
    {
      messageCount[x-4] = byte(inputString[x]);
    }
    Serial.println("Recipient Address: " + String(recAddr));
    Serial.print("Message: ");
    for(int x = 4;x <(stringLength - 1);x++)
    {
      Serial.print(inputString[x]);
    }
    Serial.println("");
    if(mode == 1)
    {
    Serial.println("Mode is 1: Direct Communication");
    sendPacket(recAddr,recAddr,nodeAddress,messageCount,(stringLength-2));
    }
    if(mode == 2)
    {
    Serial.println("Mode is 2: Hop Communication");
    sendPacket(hopAddress,recAddr,nodeAddress,messageCount,(stringLength-2));
    }
    indexNumber=0;
    stringLength = 0;
    inputString = "";
    stringComplete = false;
    attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  }
}

void serialEvent() {
  while (Serial.available()) {                                                                             
    char inChar = (char)Serial.read();                                                                           
    inputString += inChar;                                                                           
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
} 

void sendPacket(byte receiver, byte recipient, byte sender, byte *Message, int Length)
{
  CCPACKET data;
  data.length=Length;
  data.data[0]=receiver;
  data.data[1]=recipient;
  data.data[2]=sender;
  for(int x = 3;x <Length;x++)
  {
  data.data[x]=Message[x-3];
  }
  if(cc1101.sendData(data)){
    Serial.println("Sending Successful");
    Serial.println("");
    }
  else{
    Serial.println("Sending Failed ");
    Serial.println("");
  } 
  cc1101.setRxState();
}
