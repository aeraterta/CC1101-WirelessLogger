#include "EEPROM.h"
#include "cc1101.h"
#include <Timer.h>

CC1101 cc1101;
Timer t;

float Temp;
int led = A3;
byte syncWord[] = {19, 9};
byte nodeAddress = 4;
byte receiverAddress = 1;
int datacount = 0;
int count = 0;
int interval = 5;
int remainder;
int quotient;
byte tempwhole;
byte tempdecimal;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
volatile byte dataReceive[]={0,0};
boolean packetAvailable = false; 
int sec = 0;
unsigned int heart_rate;//the measurement result of heart rate

void cc1101signalsInterrupt(void){
  // set the flag that a package is available
  packetAvailable = true;
}


void setup() {
  Serial.begin(38400);
  pinMode(led, OUTPUT);
  inputString.reserve(50);     // reserve 50 bytes for the inputString:
  cc1101.init();
  cc1101.setSyncWord(syncWord, false);
  t.every(1000, timer);
  // this device address need to match the target address in the sender
  cc1101.setDevAddress(nodeAddress, false);
  cc1101.enableAddressCheck(); // you can skip this line, because the default is to have the address check enabled
  cc1101.setRxState();
  // Enable wireless reception interrupt
  attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  Serial.print("THESIS "); Serial.println(" v1.0");
  Serial.println("DUMMY NODE");
  Serial.println("NODE ADDRESS: " + String(nodeAddress));
  Serial.println("-----------------------------------------");
  Serial.println();
}

void loop()
{        
  t.update(); 
  
   if(packetAvailable){
    // clear the flag
    packetAvailable = false;
    
    CCPACKET packet;
    
    // Disable wireless reception interrupt
    detachInterrupt(0);
    
    if(cc1101.receiveData(&packet) > 0){
      if (packet.crc_ok && packet.length > 1){
        remainder = int(packet.data[1]);
        quotient = int(packet.data[2]);
        interval = (256*quotient) + remainder;
        Serial.print("Sending Interval Changed to: ");
        Serial.println(interval);
      }
    }
    // Enable wireless reception interrupt
    attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  }
}

void Send()
{
  detachInterrupt(0);
  digitalWrite(led, HIGH);
  CCPACKET data;
  data.length=5;
  // the first byte in the data is the destination address
  // it must match the device address of the receiver
  data.data[0]=receiverAddress;
  data.data[1]=nodeAddress;
  data.data[2]=tempwhole;
  data.data[3]=tempdecimal;
  data.data[4]=heart_rate;
/*  
  Serial.print(data.data[2]);
  Serial.print(".");
    if(data.data[3] < 10)
        {
          Serial.print("0");
          Serial.print(data.data[3]);
        }
    else
        {
          Serial.print(data.data[3]);
        }
   Serial.print(",");
   Serial.print(data.data[4]);
   Serial.print(",");
   Serial.println(data.data[1]);
*/ 
  if(cc1101.sendData(data)){
   // Serial.println("Sending Successful!");
  //Serial.println();
  Serial.print("Data Sent: ");
  Serial.print(data.data[2]);
  Serial.print(".");
    if(data.data[3] < 10)
        {
          Serial.print("0");
          Serial.print(data.data[3]);
        }
    else
        {
          Serial.print(data.data[3]);
        }
   Serial.print(",");
   Serial.print(data.data[4]);
   Serial.print(",");
   Serial.println(data.data[1]);
   
    }
  else{
    Serial.println("Sending Failed! "); 
   // Serial.println();
  } 
  digitalWrite(led, LOW);  
  attachInterrupt(0, cc1101signalsInterrupt, FALLING); 
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

void timer()
{
 sec++;
  if(sec > 21600 )
   {
    sec = 0;
   } 
  if( sec%interval==0)
   {
     dummydata();
     Send();
   }
}

void dummydata()
{
  Temp = 36.5 + random(3) + random(101)/100.00;
  heart_rate = random(60,100);
  int whole = int(Temp);
  int decimal = (Temp*100)-(whole*100);
  tempwhole = byte(whole);
  tempdecimal = byte(decimal);
}
