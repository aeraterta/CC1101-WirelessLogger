#include "EEPROM.h"
#include "cc1101.h"

byte syncWord[] = {19, 9};
byte nodeAddress = 1;
byte ReceiverAddress;
byte quotient;
byte remainder; 
int Number; 
int datacount = 0;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
volatile int dataReceive[]={0,0};

CC1101 cc1101;
boolean packetAvailable = false; 

void cc1101signalsInterrupt(void){
  // set the flag that a package is available
  packetAvailable = true;
}


void setup() {
  Serial.begin(38400);
  // reserve 50 bytes for the inputString:
  inputString.reserve(50);
  cc1101.init();
  cc1101.setSyncWord(syncWord, false);
  
  // this device address need to match the target address in the sender
  cc1101.setDevAddress(nodeAddress, false);
  cc1101.enableAddressCheck(); // you can skip this line, because the default is to have the address check enabled
  cc1101.setRxState();
  // Enable wireless reception interrupt
  attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  Serial.print("THESIS "); Serial.println(" v1.0");
  Serial.println("MAIN NODE");
  Serial.println("NODE ADDRESS: " + String(nodeAddress));
  Serial.println("Node Set-up Syntax: Node Address,Interval");
  Serial.println("-----------------------------------------");
}

void loop()
{
if (stringComplete) {
    detachInterrupt(0);
    // The data to be parsed
    char len=50;
    char dat[len];
    // Temp store for each data chunk
    char *str;
    // Id ref for each chunk
    datacount=0;
    inputString.toCharArray(dat, len);
    char *p=dat;
    // Loop through the data and seperate it into
    // chunks at each "," delimeter
    while ((str = strtok_r(p, ",", &p)) != NULL)
    {
      // Add chunk to array
      dataReceive[datacount] = atoi(str);
      // Increment data count
      datacount++;
      if (datacount>1){
        break;
      }
    }
    /*
    sprintf(dat,"Acknowledge %d %d\n",dataReceive[0],dataReceive[1]);
    Serial.println(dat);
    */ 
    ReceiverAddress = byte(dataReceive[0]);
    Number = dataReceive[1];
    quotient = byte((dataReceive[1])/256);
    remainder = byte (dataReceive[1] - (int(quotient)*256));
    sendPacket(ReceiverAddress,remainder,quotient);
    // clear the string:
    inputString = "";
    stringComplete = false;
    delay(100);
    attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  }
  
   if(packetAvailable){
    // clear the flag
    packetAvailable = false;
    
    CCPACKET packet;
    
    // Disable wireless reception interrupt
    detachInterrupt(0);
    
    if(cc1101.receiveData(&packet) > 0){
      if (packet.crc_ok && packet.length > 1){
        
        Serial.print(packet.data[2]);
        Serial.print(".");
        if(packet.data[3] < 10)
        {
          Serial.print("0");
          Serial.print(packet.data[3]);
        }
        else
        {
          Serial.print(packet.data[3]);
        }
        Serial.print(",");
        if (packet.data[4] == 255)
        {
        Serial.print("-1");
        }
        else
        {
        Serial.print(packet.data[4]);
        }
        Serial.print(",");
        Serial.println(packet.data[1]);
        
      }
    }
    
    // Enable wireless reception interrupt
    attachInterrupt(0, cc1101signalsInterrupt, FALLING);
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

void sendPacket(byte receiver, byte data2, byte data3)
{
  CCPACKET data;
  data.length=3;
  data.data[0]=receiver;
  data.data[1]=data2;
  data.data[2]=data3;
  if(cc1101.sendData(data)){
    Serial.println("Setting Address: " + String(receiver));
    Serial.println("Interval: " + String(Number));
    Serial.println("Sending Successful");
    //Serial.println("");
    }
  else{
    Serial.println("Sending Failed ");
    //Serial.println("");
  } 
  cc1101.setRxState();
}
