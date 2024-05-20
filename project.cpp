#include "ThingSpeak.h"
#include <ESP8266WiFi.h>

unsigned long myChannelNumber1 = 2520246;
const char *myWriteAPIKey1 = "Q9STASZYN4I8224U";

const char *ssid = "Apto 402!!!!";
const char *password = "agusoyaca";

WiFiClient client;

const int rele = D2;
const int sensor = A0;

unsigned long lastToggleTime = 0;
unsigned long lastSendTime = 0;

const unsigned long toggleInterval = 7500; 
const unsigned long sendInterval = 15000; 

int sumSound = 0;
int sumAir = 0;
int countSound = 0;
int countAir = 0;

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
    sumSound += sound;
    countSound++;
  } else {
    int air = analogRead(sensor);
    sumAir += air;
    countAir++;
  }

  if (currentTime - lastSendTime >= sendInterval) {
    if (countSound > 0) {
      int promSound = sumSound / countSound;
      ThingSpeak.setField(1, promSound);
      Serial.print("Promedio de sonido enviado a ThingSpeak: ");
      Serial.println(promSound);
    } else {
      ThingSpeak.setField(1, 0);
    }

    if (countAir > 0) {
      int promAir = sumAir / countAir;
      ThingSpeak.setField(2, promAir);
      Serial.print("Promedio de aire enviado a ThingSpeak: ");
      Serial.println(promAir);
    } else {
      ThingSpeak.setField(2, 0);
    }

    int response = ThingSpeak.writeFields(myChannelNumber1, myWriteAPIKey1);
    if (response == 200) {
      Serial.println("Datos enviados correctamente.");
    } else {
      Serial.println("Error al enviar los datos: " + String(response));
    }

    countSound = 0;
    countAir = 0;
    sumSound = 0;
    sumAir = 0;
    lastSendTime = currentTime;
  }

  delay(100); 
}


