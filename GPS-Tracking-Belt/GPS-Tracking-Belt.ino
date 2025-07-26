#define DEVICE_ID "E01"

#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266Firebase.h>
#include <ESP8266WebServer.h>

#define WIFI_SSID "TCL20Y"
#define WIFI_PASSWORD "F55BuIvV"
#define FIREBASE_HOST "https://elephant-tracking-app-default-rtdb.asia-southeast1.firebasedatabase.app"

Firebase firebase(FIREBASE_HOST);
TinyGPSPlus gps;
SoftwareSerial gpsSerial(D4, D3);  // RX, TX
ESP8266WebServer server(80);       // HTTP server

unsigned long lastSent = 0;
const unsigned long sendInterval = 2000;  // milliseconds

float lastLat = 0.0;
float lastLng = 0.0;
String lastTime;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("- ");
    delay(500);
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // HTTP Endpoint to get current device info
  server.on("/info", []() {
    String json = "{";
    json += "\"id\": \"" + String(DEVICE_ID) + "\",";
    json += "\"lat\": " + String(lastLat, 6) + ",";
    json += "\"lng\": " + String(lastLng, 6) + ",";
    json += "\"time\": \"" + lastTime + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // Handle HTTP requests

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid() && gps.location.isUpdated()) {
    unsigned long now = millis();
    if (now - lastSent >= sendInterval) {
      lastSent = now;

      lastLat = gps.location.lat();
      lastLng = gps.location.lng();

      char timestamp[30];
      sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
        gps.date.year(),
        gps.date.month(),
        gps.date.day(),
        gps.time.hour(),
        gps.time.minute(),
        gps.time.second());

      lastTime = timestamp;

      Serial.println("------------------------------------------------------");
      Serial.print("Latitude: "); Serial.println(lastLat, 6);
      Serial.print("Longitude: "); Serial.println(lastLng, 6);
      Serial.print("Timestamp: "); Serial.println(lastTime);
      Serial.println("------------------------------------------------------");

      if (WiFi.status() == WL_CONNECTED) {
        String idPath = "/elephant_locations/" + String(DEVICE_ID) + "/id";
        String basePath = "/elephant_locations/" + String(DEVICE_ID);

        String result = firebase.getString(idPath);
        Serial.print("Checking path: "); Serial.println(idPath);
        Serial.print("Result: "); Serial.println(result);

        if (result == DEVICE_ID) {
          Serial.println("ID exists. Updating...");
        } else {
          Serial.println("ID not found. Creating new entry...");
          firebase.setString(idPath, DEVICE_ID);
        }

        firebase.setFloat(basePath + "/position/lat", lastLat);
        firebase.setFloat(basePath + "/position/lng", lastLng);
        firebase.setString(basePath + "/timestamp", lastTime);

        Serial.println("Data pushed to Firebase");
      } else {
        Serial.println("WiFi not connected. Reconnecting...");
        Connect_WiFi();
      }
    }
  }
}

void Connect_WiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("-------------------------------------------------------");
    Serial.println("WiFi Not connected");
    Serial.println("-------------------------------------------------------");
  } else {
    Serial.println("-------------------------------------------------------");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("-------------------------------------------------------");
  }
  delay(3000);
}
