#include "ThingSpeak.h"
#include <ESP8266WiFi.h>

unsigned long myChannelNumber1 = 2520246;
const char *myWriteAPIKey1 = "Q9STASZYN4I8224U";

const char *ssid = "wifing";
const char *password = "wifing-pub";


WiFiClient client;

const int rele = D2;
const int sensor = A0;

unsigned long lastToggleTime = 0;
unsigned long lastSendTime = 0;

const unsigned long toggleInterval = 8000; 
const unsigned long sendInterval = 20000; 
const unsigned long promSound = 36;
const unsigned long promAir = 25;

int sumSound = 0;
int sumAir = 0;
int countSound = 0;
int countAir = 0;

int maxSound = 0;
int maxAir = 0;

int minSound = analogRead(A0);
int minAir= analogRead(A0);

void setup() {
  Serial.begin(115200);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");

  ThingSpeak.begin(client);

  pinMode(rele, OUTPUT);
  digitalWrite(rele, LOW); // inicia en LOW para comenzar con sonido
}

void loop() {

  // verificar conexión WiFi y reconectar si es necesario
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexión WiFi perdida. Reconectando...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi reconectado.");
  }

  unsigned long currentTime = millis();

  // alternar el estado del relé 
  if (currentTime - lastToggleTime >= toggleInterval) {
    digitalWrite(rele, !digitalRead(rele));
    lastToggleTime = currentTime;
  }

 
  if (digitalRead(rele) == LOW) {
    int sound = analogRead(sensor);
     

    if (maxSound<sound){
      maxSound = sound;
    }
    if (minSound>sound){
      minSound= sound;
    }

    sumSound += sound;
    countSound++;
  } else {
    int air = analogRead(sensor);

if (maxAir<air){
  maxAir=air;
}

if(minAir>air){
  minAir=air;
}
    sumAir += air;
    countAir++;
  }

  if (currentTime - lastSendTime >= sendInterval) {
 
      int promSound = sumSound / countSound;
      ThingSpeak.setField(1, promSound);
      ThingSpeak.setField(2, maxSound);
      ThingSpeak.setField(3, minSound);
      Serial.print("Promedio de sonido enviado a ThingSpeak: ");
      Serial.println(promSound);
 
   
      int promAir = sumAir / countAir;
      ThingSpeak.setField(4, promAir);
      ThingSpeak.setField(5, maxAir);
      ThingSpeak.setField(6, minAir);
      Serial.print("Promedio de aire enviado a ThingSpeak: ");
      Serial.println(promAir);
    
    

    int response = ThingSpeak.writeFields(myChannelNumber1, myWriteAPIKey1);
    if (response == 200) {
      Serial.println("Datos enviados correctamente.");
      Serial.println();
    } else {
      Serial.println("Error al enviar los datos: " + String(response));
    }

    countSound = 0;
    countAir = 0;
    sumSound = 0;
    sumAir = 0;
    maxSound = 0;
    maxAir = 0;
    minSound = analogRead(A0);
    minAir= analogRead(A0);

    lastSendTime = currentTime;
  }

  delay(100); 
}

