#include <ESP8266WiFi.h>
#include "creds.h"

#define READY_PIN 12
#define BUZZ_PIN1 13
#define BUZZ_PIN2 14
#define BUTTON_PIN 5
#define HEARTBEAT_PORT 400
#define BUZZ_PORT 200
#define LISTENER_PORT 500
#define BUZZ '\x00'
#define LOCKED '\x01'
#define CLEAR '\x02'

/*const char * ssid     = "";
const char * password = "";
defined inside of creds.h
*/
 
const byte host[] = {192, 168, 1, 112};
bool canBuzz = true;
bool locked = false;
unsigned long lastHeartBeat = 0;

WiFiClient buzzerClient;
WiFiClient heartBeatClient;
WiFiServer listener(LISTENER_PORT);
IPAddress addr(host);
 
void setup(){
  pinMode(READY_PIN, OUTPUT);
  pinMode(BUZZ_PIN1, OUTPUT);
  pinMode(BUZZ_PIN2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(READY_PIN, LOW);
  digitalWrite(BUZZ_PIN1, LOW);
  digitalWrite(BUZZ_PIN2, LOW);
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\r\nConnected");
  listener.begin();
  digitalWrite(READY_PIN, HIGH);
}

bool sendHeartBeat(){
  heartBeatClient.connect(addr, HEARTBEAT_PORT);
  if(heartBeatClient.connected()){
    heartBeatClient.print("\x00");
    heartBeatClient.stop();
    lastHeartBeat = millis();
    return true;
  }
  return false;
}
void sendBuzz(){
  buzzerClient.connect(addr, BUZZ_PORT);
  if(buzzerClient.connected()){
    buzzerClient.print("\x00");
    buzzerClient.stop();
    canBuzz = false;
  }
}

void buzz(){
  digitalWrite(BUZZ_PIN1, HIGH);
  digitalWrite(BUZZ_PIN2, HIGH);
  digitalWrite(READY_PIN, HIGH);
  canBuzz = false;
}

void lock(){
  digitalWrite(BUZZ_PIN1, LOW);
  digitalWrite(BUZZ_PIN2, LOW);
  digitalWrite(READY_PIN, LOW);
  canBuzz = false;
  locked = true;
}

void reset(){
  digitalWrite(BUZZ_PIN1, LOW);
  digitalWrite(BUZZ_PIN2, LOW);
  digitalWrite(READY_PIN, HIGH);
  canBuzz = true;
  locked = false;
}

void loop(){
  if(digitalRead(BUTTON_PIN) == LOW && canBuzz){
    sendBuzz();
    return;
  }
  if((millis() - lastHeartBeat) > 2000){
    if(!(sendHeartBeat()) && (millis() - lastHeartBeat) > 6000){
      digitalWrite(READY_PIN, LOW);
      delay(1000);
      digitalWrite(READY_PIN, HIGH);
      delay(200);
      digitalWrite(READY_PIN, LOW);
    }
    else if (!locked){
      digitalWrite(READY_PIN, HIGH);
    }
  }
  WiFiClient cli = listener.available();
  if(!(cli)){
    return;  
  }
  while(!(cli.available())){
    delay(1);  
  }
  char c = cli.read();
  Serial.println(c);
  Serial.println((int)c);
  if(c == BUZZ){
    buzz();
  }
  else if(c == LOCKED){
    lock();
  }
  else if(c == CLEAR){
    reset();
  }
  cli.stop();
}
