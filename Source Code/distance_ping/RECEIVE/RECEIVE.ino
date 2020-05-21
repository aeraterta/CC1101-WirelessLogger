#include "EEPROM.h"
#include "cc1101.h"

CC1101 cc1101;
 
byte syncWord[] = {19, 9};
byte nodeAddress = 1;
byte ReceiverAddress;


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean packetAvailable = false;

float Volt = 0.00;
float Current = 0.00;
byte Vw;
byte Vr;
byte Iw;
byte Ir;



void setup() {
  Serial.begin(38400);
  inputString.reserve(200);
  cc1101.init();
  cc1101.setSyncWord(syncWord, false);
  cc1101.setDevAddress(nodeAddress, false);
  cc1101.enableAddressCheck(); 
  cc1101.setRxState();
  Serial.print("THESIS "); Serial.println(" v1.0");
  Serial.println("RECEIVER PING");
  Serial.println("NODE ADDRESS: " + String(nodeAddress));
  Serial.println("-----------------------------------------");
}

void loop()
{
  if (stringComplete) {
  int Indexab1 = inputString.indexOf(',');
  int Indexab2 = inputString.indexOf(',', Indexab1+1);
  int Indexab3 = inputString.indexOf(',', Indexab2+1);
  int Indexab4 = inputString.indexOf(',', Indexab3+1);
  int Indexab5 = inputString.indexOf(',', Indexab4+1);
  int Indexab6 = inputString.indexOf(',', Indexab5+1);
  int Indexab7 = inputString.indexOf(',', Indexab6+1);
  int Indexab8 = inputString.indexOf(',', Indexab7+1);
  String V = inputString.substring(Indexab2+1, Indexab3);  
  String I= inputString.substring(Indexab3+1, Indexab4); 
  String P= inputString.substring(Indexab4+1, Indexab5);
  /*  
  String VAab= inputString.substring(Indexab5+1, Indexab6);
  String Qab= inputString.substring(Indexab6+1, Indexab7); 
  String Qinsab= inputString.substring(Indexab7+1, Indexab8); 
  String PFab= inputString.substring(Indexab8+1); 
  */
  Volt = V.toFloat();
  Current = I.toFloat();
  Serial.println(String("Volts: ") + Volt);
  Serial.println(String("Current: ") + Current);
  float tempV = Volt * 100;
  Vw = byte(tempV/256);
  Vr = byte((tempV) - (float(Vw)*256));
  float tempI = Current * 100;
  Iw = byte(tempI/256);
  Ir = byte((tempI) - (float(Iw)*256));
  inputString = "";
  stringComplete = false;
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
    if (inChar == '\003') {
      stringComplete = true;
    } 
  }
}/*
void sendPacket(byte receiver, byte data2, byte data3)
{
  CCPACKET data;
  data.length=5;
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
*/
