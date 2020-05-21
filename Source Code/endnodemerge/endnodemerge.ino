#include "EEPROM.h"
#include "cc1101.h"
#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library
#include <Timer.h>

IRTherm therm;
CC1101 cc1101;
Timer t;

int buff_count = 0;
int buff = 0;
float Temp;
int led = A3;
byte syncWord[] = {19, 9};
byte nodeAddress = 2;                     //change address per node
byte receiverAddress = 1;
int datacount = 0;
int count = 0;
int interval = 5; //initialize interval for data sending to 5 seconds
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
  therm.begin(); // Initialize thermal IR sensor
  therm.setUnit(TEMP_C); // Set the library's units to Celsius
  inputString.reserve(50);     // reserve 50 bytes for the inputString:
  cc1101.init();
  cc1101.setSyncWord(syncWord, false);
  t.every(1000, timer);
  
  arrayInit();
  
  // this device address need to match the target address in the sender
  cc1101.setDevAddress(nodeAddress, false);
  cc1101.enableAddressCheck(); // you can skip this line, because the default is to have the address check enabled
  cc1101.setRxState();
  // Enable wireless reception interrupt
  attachInterrupt(0, cc1101signalsInterrupt, FALLING);
  attachInterrupt(1, pulseInterrupt, RISING);//set interrupt 0,digital port 3
  Serial.print("THESIS "); Serial.println(" v1.0");
  Serial.println("END NODE");
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
/*
    Serial.println("Sending Successful!");
    Serial.println();
*/
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

void getTemp(){
if (therm.read()) // On success, read() will return 1, on fail 0.
  {
    Temp = therm.object();
    int whole = int(Temp);
    int decimal = (Temp*100)-(whole*100);
    tempwhole = byte(whole);
    tempdecimal = byte(decimal);
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
     getTemp();
     getPulse();
     Send();
   }
   
}

void sum()
{
 if(data_effect)
    {
      temp_heart_rate=1200000/(temp[20]-temp[0]);//60*20*1000/20_total_time 
      Serial.print("Running heart_rate_is:\t");
      Serial.println(temp_heart_rate);
    }
  buff = buff + temp_heart_rate;
  buff_count = buff_count + 1;
   
   data_effect=1;//sign bit
}

void getPulse()
{
  if (interval < 60)
  {
    if(buff_count > 2)
      {
       heart_rate = (buff/buff_count);
       Serial.print("Average heart_rate_is:\t");
       Serial.println(heart_rate);
       buff = 0;
       buff_count = 0;
      } 
  }
  else 
      {
       heart_rate = (buff/buff_count);
       Serial.print("Average heart_rate_is:\t");
       Serial.println(heart_rate);
       buff = 0;
       buff_count = 0;
      }
  
}
void pulseInterrupt()
{
    temp[counter]=millis();
      Serial.println(counter,DEC);
      Serial.println(temp[counter]);
    switch(counter)
  {
    case 0:
      sub=temp[counter]-temp[20];
      Serial.println(sub);
      break;
    default:
      sub=temp[counter]-temp[counter-1];
      Serial.println(sub);
      break;
  }
  if (sub > max_heartpluse_duty)
    {
      heart_rate = -1;
      data_effect=0;//sign bit
      counter=0;
      arrayInit();
    }
    if (counter==20&&data_effect)
      {
        counter=0;
        sum();
      }
    else 
     {
        if(counter!=20&&data_effect)
         counter++;
        else 
         {
          counter=0;
          data_effect=1;
         }
    }
}

/*Function: Initialization for the array(temp)*/
void arrayInit()
{
  for(unsigned char i=0;i < 20;i ++)
  {
    temp[i]=0;
  }
  temp[20]=millis();
}

