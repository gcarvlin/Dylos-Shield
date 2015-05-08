/*

Software for uploading Dylos PM counts to kairosDB through an Arduino Yun.
Version 3.0
Date 04-07-15
Seto Lab
UW DEOHS

*/

#include <Wire.h>
#include <Bridge.h>
#include <Process.h>
#include <SoftwareSerial.h>
#define rxPin 10  //pin 10 on the arduino is connected to the TX pin on the converter
#define txPin 11  //pin 11 on the arduino is connected to the RX pin on the converter
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

// HIH6130 variables
unsigned int H_dat, T_dat;
byte _status;
float RH, T_C;

//String variables
String inputString,timeString,ipAddress,allData;
String nameString = "Dylos23";  //Name of unit

//Integer variables
int wifiConnected=0;

void setup() {
  pinMode(8, OUTPUT);  //Power LED
  pinMode(9, OUTPUT);  //Status LED
  pinMode(12, OUTPUT); //Data LED
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  digitalWrite(8, HIGH); //Turn on power LED
  digitalWrite(9, LOW);  //Set other LEDs to off
  digitalWrite(12, LOW); 
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Serial open..");
  Bridge.begin();
  Serial.println("Bridge open..");
  mySerial.begin(9600);
  Serial.println("Software serial setup..");
  Serial.println("DylosDuino v3.0 booted up!");
  Serial.print(nameString);
  Serial.println(" online");
}

void loop() {
  
   //Run wifi check until connected
   if (wifiConnected==0) {
   Process wifiCheck;
   wifiCheck.runShellCommand("lua \"/usr/bin/ipFinder.lua\"");
   ipAddress = "";
   while (wifiCheck.available()>0) {
      char c = wifiCheck.read();
      ipAddress += c;}
   if ((ipAddress != "192.168.240.1") & (ipAddress !="")) {
      digitalWrite(9, HIGH);
      Serial.print("Connected to network with ip address: ");
      Serial.println(ipAddress);
      wifiConnected=1;}
   else {digitalWrite(3, LOW);}  
   }

     
   //Read in data, if at end of string analyze input
   char bchar = mySerial.read();
   if ((bchar != -1) & (bchar!=13) &(bchar!=10)) {
   inputString += bchar;}
   
   if (bchar == 13) {
          
     //Turn on data LED
     digitalWrite(12, HIGH);
     
     //Get timeString
     gettimeString();
     
     //Get temp and RH data
     getRHTHIH6130();
     RH = (float) H_dat * 6.104e-3;
     T_C = (float) T_dat * 1.007e-2 - 40.0;
     
     //Send data to the serial monitor for debugging
     allData = String(nameString+","+timeString+","+inputString+","+String(RH)+","+String(T_C));
     Serial.println(allData);
     
     //Send dylos name, timeString, input string and RH/T_C to temp_data.txt
     Process echo;
     echo.runShellCommand("echo "+allData+" >> /root/temp_data.txt");
     
     //Reset variables
     inputString = "";
     timeString = "";
     
     //Turn off data LED
     digitalWrite(12, LOW);
     
     }
   
   delay(10);
}

//Gets timeString from date command
void gettimeString() {
  Process t;
  t.begin("date");
  t.addParameter("+%s");
  t.run();

  while (t.available() > 0) {
    char c = t.read();
    if ((c>47) && (c<58)) {
      timeString += c;
      }  
    }
  Serial.flush();
  
}

// Get the RH and temp from the HIH-6130
void getRHTHIH6130() {
  byte address, Hum_H, Hum_L, Temp_H, Temp_L;
  address = 0x27;;
  Wire.beginTransmission(address); 
  Wire.endTransmission();
  delay(100);
  
  Wire.requestFrom((int)address, (int) 4);
  Hum_H = Wire.read();
  Hum_L = Wire.read();
  Temp_H = Wire.read();
  Temp_L = Wire.read();
  Wire.endTransmission();
  
  _status = (Hum_H >> 6) & 0x03;
  Hum_H = Hum_H & 0x3f;
  H_dat = (((unsigned int)Hum_H) << 8) | Hum_L;
  T_dat = (((unsigned int)Temp_H) << 8) | Temp_L;
  T_dat = T_dat / 4;
}
