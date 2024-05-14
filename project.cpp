#include "ThingSpeak.h" //installed
#include <ESP8266WiFi.h>


unsigned long myChannelNumber1 = 2520246;
const char *myWriteAPIKey1 = "Q9STASZYN4I8224U"; 


const char *ssid = "bandejas"; 
const char *password = "ikus.1001"; 

WiFiClient client;

const int rele = D2;

void setup() {
  Serial.begin(115200);
  delay(100);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  pinMode(rele, OUTPUT);
  digitalWrite(rele, HIGH);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("conectando.. ");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(50);
      Serial.print(".");
    }
    Serial.println("\nwifi conectado.");
  }

  long rssi = WiFi.RSSI();

  if (digitalRead(rele) == LOW) {
    ThingSpeak.writeField(myChannelNumber1, 1, analogRead(A0), myWriteAPIKey1); //sound
  } else {
    ThingSpeak.writeField(myChannelNumber1, 2, analogRead(A0), myWriteAPIKey1); //air
  } 
  Serial.println("datos enviados a ThingSpeak :)");

  digitalWrite(rele, !digitalRead(rele));
  Serial.println(analogRead(A0));
  delay(20500);
}


