#include "EEPROM.h"
#include "cc1101.h"

CC1101 cc1101;

byte count1 = 1;
int buff_count = 0;
int buff = 0;
float Temp;
int led = A3;
byte syncWord[] = {19, 9};
byte nodeAddress = 2;                     //change address per node
byte receiverAddress = 1;
int datacount = 0;
int count = 0;
int interval = 1; //initialize interval for data sending to 5 seconds
int remainder;
int quotient;
byte tempwhole;
byte tempdecimal;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
volatile byte dataReceive[]={0,0};
boolean packetAvailable = false; 
int sec = 0;

unsigned char counter;
unsigned long temp[21];
unsigned long sub;
bool data_effect=true;
unsigned int heart_rate;//the measurement result of heart rate
unsigned int temp_heart_rate;
const int max_heartpluse_duty = 2000;//you can change it follow your system's request.
            //2000 meams 2 seconds. System return error 
            //if the duty overtrip 2 second.
            
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
  cc1101.setDevAddress(nodeAddress, false);
  cc1101.enableAddressCheck(); // you can skip this line, because the default is to have the address check enabled
  cc1101.setRxState();
  // Enable wireless reception interrupt
  attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  //attachInterrupt(1, pulseInterrupt, RISING);//set interrupt 0,digital port 3
  Serial.print("THESIS "); Serial.println(" v1.0");
  Serial.println("SENDER PING");
  Serial.println("NODE ADDRESS: " + String(nodeAddress));
  Serial.println("-----------------------------------------");
  Serial.println();
}

void loop()
{        
  Send();
  delay(1000);
}

void Send()
{
  detachInterrupt(0);
  digitalWrite(led, HIGH);
  CCPACKET data;
  data.length=3;
  // the first byte in the data is the destination address
  // it must match the device address of the receiver
  data.data[0]=receiverAddress;
  data.data[1]=nodeAddress;
  data.data[2]=count1;
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
if (count1 < 21)
{
  if(cc1101.sendData(data)){
/*
    Serial.println("Sending Successful!");
    Serial.println();
*/
    Serial.print("Data Sent: ");
    Serial.println(data.data[2]);
    count1 = count1 + 1;

    }
  else{
    Serial.println("Sending Failed! "); 
   // Serial.println();
  } 
}
  digitalWrite(led, LOW);  
  attachInterrupt(0, cc1101signalsInterrupt, FALLING); 
}



