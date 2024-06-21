#include "ThingSpeak.h"
#include <ESP8266WiFi.h>

unsigned long myChannelNumber1 = 2520246;
const char *myWriteAPIKey1 = "Q9STASZYN4I8224U";

const char *ssid = "wifing";
const char *password = "wifing-pub";

unsigned long myTalkBackID = 52380;
const char * myTalkBackKey = "K8XN4EXYZ8PWRLDJ";

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
  pinMode(LED_BUILTIN, OUTPUT);
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

int httpPOST(String uri, String postMessage, String &response){

  bool connectSuccess = client.connect("api.thingspeak.com",80);

  if(!connectSuccess){
    return -301;   
  }
  
  postMessage += "&headers=false";
  
  String Headers =  String("POST ") + uri + String(" HTTP/1.1\r\n") +
                    String("Host: api.thingspeak.com\r\n") +
                    String("Content-Type: application/x-www-form-urlencoded\r\n") +
                    String("Connection: close\r\n") +
                    String("Content-Length: ") + String(postMessage.length()) +
                    String("\r\n\r\n");

  client.print(Headers);
  client.print(postMessage);

  long startWaitForResponseAt = millis();
  while(client.available() == 0 && millis() - startWaitForResponseAt < 5000){
      delay(100);
  }

  if(client.available() == 0){       
    return -304; // Didn't get server response in time
  }

  if(!client.find(const_cast<char *>("HTTP/1.1"))){
      return -303; // Couldn't parse response (didn't find HTTP/1.1)
  }
  
  int status = client.parseInt();
  if(status != 200){
    return status;
  }

  if(!client.find(const_cast<char *>("\n\r\n"))){
    return -303;
  }

  String tempString = String(client.readString());
  response = tempString;
  
  return status;
    
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

  // Create the TalkBack URI
  String tbURI = String("/talkbacks/") + String(myTalkBackID) + String("/commands/execute");
  
  // Create the message body for the POST out of the values
  String postMessage =  String("api_key=") + String(myTalkBackKey);                      
                       
   // Make a string for any commands that might be in the queue
  String newCommand = String();

  // Make the POST to ThingSpeak
  int x = httpPOST (tbURI, postMessage, newCommand);
  client.stop();
  
  // Check the result
  if(x == 200){
    Serial.println("checking queue..."); 
    // Check for a command returned from TalkBack
    if(newCommand.length() != 0){

      Serial.print("  Latest command from queue: ");
      Serial.println(newCommand);
      
      if(newCommand == "TURN_ON"){
        Serial.println("contaminacion sonora");
        //digitalWrite(LED_BUILTIN, HIGH);  
      }

      if(newCommand == "TURN_OFF"){
        Serial.println("no hay contaminacion sonora");
        //digitalWrite(LED_BUILTIN, LOW);
      }
    }
    else{
      Serial.println("  Nothing new.");  
    }
    
  }
  else{
    Serial.println("Problem checking queue. HTTP error code " + String(x));
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
