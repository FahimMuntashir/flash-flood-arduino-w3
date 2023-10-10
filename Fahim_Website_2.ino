#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Wire.h>


const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String GAS_ID = "AKfycbz10GBIvo7GLs91FydSvZF4UQcSHI6utGy1v7UXLzCt7wRSy0nJQPi1enWN2TgAl4AS"; 


// Replace with your network credentials
const char* ssid = "Mueed"; 
const char* password = "1234567890";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

int trigpin = 13;
int echopin =12;
float distance;
float duration;
bool start = LOW;


void setup(){
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  //Serial.begin(9600);
  
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  
  // Initialize SPIFFS
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  client.setInsecure();
  
  
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html");
  });
  
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getWaterLevel().c_str());
  });
  // Start server
  server.begin();
  sendData(WiFi.localIP().toString());
}


void sendData(String distance) {
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/macros/s/" + GAS_ID + "/exec?distance=" + distance;



  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");


  if(!client.connected()){
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  }
} 

void loop(){
  
}

String getWaterLevel() {
  digitalWrite(trigpin,LOW);
  delay(2);
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin, LOW);
  duration = pulseIn(echopin, HIGH);
  distance = (duration*0.034)/2;
  delay(1000);
  Serial.print(distance);
  Serial.print("cm\n");
  //sendData(String(distance));
  return String(distance);
}
