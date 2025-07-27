#define DEVICE_ID "E01"

#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266Firebase.h>
#include <WiFiUdp.h>

#define WIFI_SSID "Suraj"
#define WIFI_PASSWORD "11114444"
#define FIREBASE_HOST "https://elephant-tracking-app-default-rtdb.asia-southeast1.firebasedatabase.app"

WiFiUDP udp;
const unsigned int udpPort = 4210;

Firebase firebase(FIREBASE_HOST);
TinyGPSPlus gps;
SoftwareSerial gpsSerial(D2, D1);  // RX, TX

unsigned long lastSent = 0;
const unsigned long sendInterval = 1000;  // milliseconds

// float Lat = 0.0;
// float Lng = 0.0;
// String lastTime;

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

  // Dynamically set broadcast IP based on current subnet
  IPAddress broadcastIp = WiFi.localIP();
  broadcastIp[3] = 255;
  
  udp.begin(udpPort);
  Serial.println("UDP ready");
}

void loop() {
  Serial.println("-----------------------------------------------------");

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid() && gps.location.isUpdated()) {
    unsigned long now = millis();
    if (now - lastSent >= sendInterval) {
      lastSent = now;

      float lastLat = gps.location.lat();
      float lastLng = gps.location.lng();

      char timestamp[30];
      sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
        gps.date.year(),
        gps.date.month(),
        gps.date.day(),
        gps.time.hour(),
        gps.time.minute(),
        gps.time.second());

      // Lat = lastLat;
      // Lng = lastLng;
      // lastTime = timestamp;

      Serial.println("------------------------------------------------------");
      Serial.print("Latitude: "); Serial.println(lastLat, 6);
      Serial.print("Longitude: "); Serial.println(lastLng, 6);
      Serial.print("Timestamp: "); Serial.println(timestamp);
      Serial.println("------------------------------------------------------");

      String message = String(DEVICE_ID) + "," + String(lastLat, 6) + "," + String(lastLng, 6);

      // Use dynamic broadcast IP every time you send (optional but safer)
      IPAddress broadcastIp = WiFi.localIP();
      broadcastIp[3] = 255;

      udp.beginPacket(broadcastIp, udpPort);
      udp.write(message.c_str());
      udp.endPacket();

      // if (WiFi.status() == WL_CONNECTED) {
        // String idPath = "/elephant_locations/" + String(DEVICE_ID) + "/id";
        // String basePath = "/elephant_locations/" + String(DEVICE_ID);

        // String result = firebase.getString(idPath);
        // Serial.print("Checking path: "); Serial.println(idPath);
        // Serial.print("Result: "); Serial.println(result);

        // if (result == DEVICE_ID) {
        //   Serial.println("ID exists. Updating...");
        // } else {
        //   Serial.println("ID not found. Creating new entry...");
        //   firebase.setString(idPath, DEVICE_ID);
        // }

        // firebase.setFloat(basePath + "/position/lat", lastLat);
        // firebase.setFloat(basePath + "/position/lng", lastLng);
        // firebase.setString(basePath + "/timestamp", lastTime);

        // Serial.println("Data pushed to Firebase");
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

  unsigned long startAttemptTime = millis();

  // Wait for connection with timeout (10 seconds)
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n-------------------------------------------------------");
    Serial.println("WiFi Not connected");
    Serial.println("-------------------------------------------------------");
  } else {
    Serial.println("\n-------------------------------------------------------");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("-------------------------------------------------------");
  }
}
