/**
/*
Morse Messenger
Clinton & Koli
DIGF-6037-001 Creation & Computation
OCAD University
Created on 04/12/20
Based on:
9 LED Patterns with Arduino, Hasarinda Manjula, https://create.arduino.cc/projecthub/thingerbits/9-led-patterns-with-arduino-277bf1
Trading Temperatures, Kate Hartman/Nick Puckett, https://github.com/DigitalFuturesOCADU/CC2020/tree/main/Experiment5/SendReceiveTouch
 *
**/

#include <WiFiNINA.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>
#include <ArduinoJson.h>
#include "Wire.h"
#include "Adafruit_MPR121.h"

//mpr stuff

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
uint16_t currtouched = 0;
uint16_t lasttouched = 0;
int tp = 12; //change this if you aren't using them all, start at pin 0


//**Details of your local Wifi Network

//Name of your access point
char ssid[] = "Agrimita";
//Wifi password
char pass[] = "doti1963";

int status = WL_IDLE_STATUS;       // the Wifi radio's status

// pubnub keys
char pubkey[] = "pub-c-928aff5e-203c-4e91-995f-4cfe8d1a12ab";
char subkey[] = "sub-c-743c03e2-3727-11eb-99ef-fa1b309c1f97";

// channel and ID data

const char* myID = "Clint"; // place your name here, this will be put into your "sender" value for an outgoing messsage

char publishChannel[] = "clintData"; // channel to publish YOUR data
char readChannel[] = "koliData"; // channel to read THEIR data

// JSON variables
StaticJsonDocument<200> dataToSend; // The JSON from the outgoing message
StaticJsonDocument<200> inMessage; // JSON object for receiving the incoming values
//create the names of the parameters you will use in your message
String JsonParamName1 = "publisher";
String JsonParamName2 = "pinTouched";



int serverCheckRate = 1000; //how often to read data on PN
unsigned long lastCheck; //time of last publish


const char* inMessagePublisher; 
int koliPinTouched;  ///the value Clint reads from Kolis board via PN
int clintPinTouched; //the value I get locally


int t = 40;
int rnd =5;
int pat1t =75;


void setup() {
  
  Serial.begin(9600);

  //run this function to connect
  connectToPubNub();
  
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

   for(int i=3; i<=12; i++) 
   pinMode(i,OUTPUT);
}


void loop() 
{
//read the capacitive sensor
checkAllPins(12);

//send and receive messages with PubNub, based on a timer
sendReceiveMessages(serverCheckRate);

///Do whatever you want with the data here!

if (koliPinTouched == 0) {
  for(int i=0; i<=rnd; i++) {
   pat8();}
  }

if (koliPinTouched == 2) {
  for(int i=0; i<=rnd; i++) {
   pat4();}
  }

if (koliPinTouched == 4) {
  for(int i=0; i<=rnd; i++) {
   pat6();}
  }

if (koliPinTouched == 6) {
  for(int i=0; i<=rnd; i++) {
   pat5();}
  }

if (clintPinTouched == 2) {
digitalWrite(5, LOW);
  }
  
else
digitalWrite(12, LOW);

   
}

void connectToPubNub()
{
    // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to the network, SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    Serial.print("*");

    // wait 2 seconds for connection:
    delay(2000);
  }

  // once you are connected :
  Serial.println();
  Serial.print("You're connected to ");
  Serial.println(ssid);
  
  PubNub.begin(pubkey, subkey);
  Serial.println("Connected to PubNub Server");

  
}

void sendReceiveMessages(int pollingRate)
{
    //connect, publish new messages, and check for incoming
    if((millis()-lastCheck)>=pollingRate)
    {
      //check for new incoming messages
      readMessage(readChannel);
      
      //save the time so timer works
      lastCheck=millis();
    }

 
}



void sendMessage(char channel[]) 
{

      Serial.print("Sending Message to ");
      Serial.print(channel);
      Serial.println(" Channel");
  
  char msg[64]; // variable for the JSON to be serialized into for your outgoing message
  
  // assemble the JSON to publish
  dataToSend[JsonParamName1] = myID; // first key value is sender: yourName
  dataToSend[JsonParamName2] = clintPinTouched; // second key value is the Pin number

  serializeJson(dataToSend, msg); // serialize JSON to send - sending is the JSON object, and it is serializing it to the char msg
  Serial.println(msg);
  
  WiFiClient* client = PubNub.publish(channel, msg); // publish the variable char 
  if (!client) 
  {
    Serial.println("publishing error"); // if there is an error print it out 
  }
  else
  {
  Serial.print("   ***SUCCESS"); 
  }

}
void checkAllPins(int totalPins)
{
  // Get the currently touched pads
  currtouched = cap.touched();
  for (uint8_t i=0; i<totalPins; i++) 
  {
    // it if *is* touched and *wasnt* touched before, send a message
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) )
    {
      clintPinTouched = i;
      sendMessage(publishChannel);
      
    }

  
  }
lasttouched = currtouched;
  
}


void readMessage(char channel[]) 
{
  String msg;
    auto inputClient = PubNub.history(channel,1);
    if (!inputClient) 
    {
        Serial.println("message error");
        delay(1000);
        return;
    }
    HistoryCracker getMessage(inputClient);
    while (!getMessage.finished()) 
    {
        getMessage.get(msg);
        //basic error check to make sure the message has content
        if (msg.length() > 0) 
        {
          Serial.print("**Received Message on ");
          Serial.print(channel);
          Serial.println(" Channel");
          Serial.println(msg);
          //parse the incoming text into the JSON object

          deserializeJson(inMessage, msg); // parse the  JSON value received

           //read the values from the message and store them in local variables 
           inMessagePublisher = inMessage[JsonParamName1]; // this is will be "their name"
           koliPinTouched = inMessage[JsonParamName2]; // the value of their Temperature sensor

        }
    }
    inputClient->stop();
  

}

void pat1(){ 
    for(int i=3; i<=12; i++) {
      digitalWrite(i,HIGH);
      delay(pat1t);
      digitalWrite(i,LOW);
 
    }
    
    for(int i=11; i>=4; i--) {
      digitalWrite(i,HIGH);
      delay(pat1t);
      digitalWrite(i,LOW);
    }
}   
void pat2(){
     for(int i=3; i<=12; i++) {
      digitalWrite(i,HIGH);
      digitalWrite(i-1,HIGH);
      digitalWrite(i+1,HIGH);
      delay(100);
      digitalWrite(i,LOW);
      digitalWrite(i-1,LOW);
      digitalWrite(i+1,LOW);
    }
    
    for(int i=11; i>=4; i--) {
      digitalWrite(i,HIGH);
      digitalWrite(i-1,HIGH);
      digitalWrite(i+1,HIGH);
      delay(100);
      digitalWrite(i,LOW);
      digitalWrite(i-1,LOW);
      digitalWrite(i+1,LOW);
    }
}
void pat3(){
  for(int i=3; i<=12; i=i+2) {
      digitalWrite(i,HIGH);
      delay(100);
      digitalWrite(i,LOW);
 
    }
    
    for(int i=12; i>=3; i=i-2) {
      digitalWrite(i,HIGH);
      delay(100);
      digitalWrite(i,LOW);
    }       
}
void pat4(){   
      for(int i=3; i<=12; i++) {
      digitalWrite(i,HIGH);
      delay(100);
    }
    
    for(int i=12; i>=2; i--) {
      digitalWrite(i,HIGH);
      delay(100);
      digitalWrite(i,LOW);
    }
}
void pat5(){
     for(int i=3; i<=12; i++) {
      digitalWrite(i,HIGH);
      }
    delay(100);
    for(int i=3; i<=12; i++) {
      digitalWrite(i,LOW);
      }
    delay(100);  
}
void pat6(){
       for(int i=3; i<=8; i++) {
      digitalWrite(i,HIGH);
      }
    for(int i=8; i<=12; i++) {
      digitalWrite(i,LOW);
      }
    delay(200);
    for(int i=3; i<=8; i++) {
      digitalWrite(i,LOW);
      }
    for(int i=8; i<=12; i++) {
      digitalWrite(i,HIGH);
      }
    delay(200);
}
void pat7(){
       for(int i=3; i<=12; i=i+2) {
      digitalWrite(i,HIGH);
      }
    for(int i=4; i<=12; i=i+2) {
      digitalWrite(i,LOW);
      }
    delay(200);
     for(int i=3; i<=12; i=i+2) {
      digitalWrite(i,LOW);
      }
    for(int i=4; i<=12; i=i+2) {
      digitalWrite(i,HIGH);
      }
    delay(200);
}
void pat8(){
    digitalWrite(7,HIGH);
    digitalWrite(8,HIGH);
    delay(t);
    digitalWrite(7,LOW);
    digitalWrite(8,LOW);
    delay(t);
    digitalWrite(6,HIGH);
    digitalWrite(9,HIGH);
    delay(t);
    digitalWrite(6,LOW);
    digitalWrite(9,LOW);
    delay(t);
    digitalWrite(5,HIGH);
    digitalWrite(10,HIGH);
    delay(t);
    digitalWrite(5,LOW);
    digitalWrite(10,LOW);
    delay(t); 
    digitalWrite(4,HIGH);
    digitalWrite(11,HIGH);
    delay(t);
    digitalWrite(4,LOW);
    digitalWrite(11,LOW);
    delay(t);
    digitalWrite(3,HIGH);
    digitalWrite(12,HIGH);
    delay(t);
    digitalWrite(3,LOW);
    digitalWrite(12,LOW);
    delay(t);
     digitalWrite(4,HIGH);
    digitalWrite(11,HIGH);
    delay(t);
    digitalWrite(4,LOW);
    digitalWrite(11,LOW);
    delay(t);
    digitalWrite(5,HIGH);
    digitalWrite(10,HIGH);
    delay(t);
    digitalWrite(5,LOW);
    digitalWrite(10,LOW);
    delay(t);
    digitalWrite(6,HIGH);
    digitalWrite(9,HIGH);
    delay(t);
    digitalWrite(6,LOW);
    digitalWrite(9,LOW);
    delay(t);       
}    

void pat9(){ 
    for(int i=3; i<=12; i++) {
      digitalWrite(i,HIGH);
    }
     for(int i=3; i<=12; i++) {
      digitalWrite(i,LOW);
      delay(100);
      digitalWrite(i,HIGH);
    }
    for(int i=11; i>=4; i--) {
      digitalWrite(i,LOW);
      delay(100);
      digitalWrite(i,HIGH);
    }
}  
