#include <ESP8266HttpClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>

#define VALVE_1 D0
#define VALVE_2 D1
#define VALVE_3 D5
#define LED 2 //Define blinking LED pin

String relaySettings;
String statusJson;

//GENERAL WIFI 
const char* ssid = "MEB";		
const char* password = "Biology$32";
// Set Static IP address
IPAddress local_IP(192, 168, 0, 118);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);	
IPAddress dns(8,8,8,8);


//WIFI SERVER (receive HTTP-POST)
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// timeout time in milliseconds 
const long timeoutTime = 2000;
ESP8266WebServer server(80);
String header;

void initSerial() {
  Serial.begin(115200);
  
}

void initPins() {
  pinMode(VALVE_1, OUTPUT);
  pinMode(VALVE_2, OUTPUT);
  pinMode(VALVE_3, OUTPUT);
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  if (!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("STA Failed to configure"); //usually happens when invalid or mismatched ip passed to config
  }
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  //check wi-fi is connected  network
  while (WiFi.status() != WL_CONNECTED) {
	  delay(500);
	  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}



bool validateReceivedSettings() {
  if (!server.hasArg("VALVE_1") || !server.hasArg("VALVE_2") || !server.hasArg("VALVE_2")) {
    return false;
  }
  else return true;
}

void handleStatus(){
  //Compose JSON Document
  String statusResponse;
  StaticJsonDocument<200> doc;
  doc["VALVE_1"] = String(digitalRead(VALVE_1));
  doc["VALVE_2"] = String(digitalRead(VALVE_2));
  doc["VALVE_3"] = String(digitalRead(VALVE_3));

//Send JSON Response
  serializeJsonPretty(doc, statusResponse);
  server.send(200, "application/json", statusResponse);  
}

void applySetting(uint8_t pin, int value){
  if (value == 1) {
    digitalWrite(pin, HIGH);
  }
  else {
    digitalWrite(pin, LOW);
  }
}


void handleRelaySettings() {
  if (!validateReceivedSettings()) {
    server.send(400, "text/plain", "invalid POST args");
    return;
  }
  
  int valve_1_setting = server.arg("VALVE_1").toInt();
  int valve_2_setting = server.arg("VALVE_2").toInt();
  int valve_3_setting = server.arg("VALVE_3").toInt();

  Serial.println(valve_1_setting);
  Serial.println(valve_2_setting);
  Serial.println(valve_3_setting);

  applySetting(VALVE_1, valve_1_setting);
  applySetting(VALVE_2, valve_2_setting);
  applySetting(VALVE_3, valve_3_setting);
  handleStatus();
}

void initServer(){
  server.begin();
  Serial.println("Server started on " + WiFi.localIP().toString());
  server.on("/status", HTTP_GET, handleStatus);               // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/settings", HTTP_POST, handleRelaySettings);
  server.onNotFound([](){
    server.send(404, "text/plain", "404: Not found");
  });
}


void setup() {
  initPins();
  initSerial();
  initWifi();
  initServer();
  
}

void loop() {
  server.handleClient();
}