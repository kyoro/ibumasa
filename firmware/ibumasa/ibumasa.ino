#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <OneWire.h>
#include <DallasTemperature.h>
  
#define ONE_WIRE_BUS 5
#define SENSER_BIT   9

#include "secrets.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int alertLevel = 3;


void wifiConnect() {
  Serial.print("Connecting to " + String(ssid));

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void sendSlackMessage(String message) {
  const char* host = "hooks.slack.com";
  
  WiFiClientSecure client;
  Serial.println("Send to Slack...");
  
  if (!client.connect(host, 443)) {
    Serial.println("Connection failed");
    return;
  }
  Serial.println("Connected");

  String post_data="payload={\"channel\": \"" + channel + "\", \"username\": \"" + username + "\", \"text\": \"" + message + "\", \"icon_emoji\": \":hotsprings:\"}";
  Serial.println(post_data);
  
  client.print("POST ");
  client.print(slack_path);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(host);
  client.println("User-Agent: ArduinoIoT/1.0");
  client.println("Connection: close");
  client.println("Content-Type: application/x-www-form-urlencoded;");
  client.print("Content-Length: ");
  client.println(post_data.length());
  client.println();
  client.println(post_data);
 
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  Serial.println(line);
  Serial.println("Done!");
}



void setup() {
 
  Serial.begin(115200);

  sensors.setResolution(SENSER_BIT);
  
  wifiConnect();

  Serial.println("Booted");

  sendSlackMessage(":muscle: Wake up! Hi, I'm IBUMASA!");  
}
float lastTemp=0;
unsigned int time_count = 0;
void loop() {
  sensors.requestTemperatures();
  Serial.println(sensors.getTempCByIndex(0));
  
  float temp = sensors.getTempCByIndex(0);
  int nextLevel = int(temp / alertLevel);
  if(abs(temp - lastTemp) > 3.0){
    String indicator;
    if(temp > lastTemp){
      indicator = ":arrow_up:";
    }else{
      indicator = ":arrow_down:";
    }
    sendSlackMessage(indicator + " " + String(temp) + "%e2%84%83");
    lastTemp = temp;
  }
  if(time_count % 600 == 0){
    sendSlackMessage(":innocent: I'm running, now " + String(temp) + "%e2%84%83");  
  }
  delay(1000);
  time_count++;
}


