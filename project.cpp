#include "ThingSpeak.h" //instalada 
#include <ESP8266WiFi.h>

unsigned long myChannelNumber1 = 2468698;
const char *myWriteAPIKey1 = "ZHF4C88WQ05OK4LM"; 

unsigned long myChannelNumber2 = 2520246;
const char *myWriteAPIKey2 = "Q9STASZYN4I8224U"; 


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
    ThingSpeak.writeField(myChannelNumber1, 1, rssi, myWriteAPIKey1);
  } else {
    ThingSpeak.writeField(myChannelNumber2, 1, rssi, myWriteAPIKey2);
  }
  Serial.println("datos enviados a ThingSpeak :)");

  digitalWrite(rele, !digitalRead(rele));
  Serial.println(analogRead(A0));
  delay(20500);
}
