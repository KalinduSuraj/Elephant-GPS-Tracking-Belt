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

Firebase firebase(FIREBASE_HOST);  // Optional - can be removed if not used
TinyGPSPlus gps;
SoftwareSerial gpsSerial(D2, D1);  // RX, TX

unsigned long lastSent = 0;
const unsigned long sendInterval = 1000;     // GPS data every 1s
unsigned long lastDummySent = 0;
const unsigned long dummyInterval = 10000;   // Dummy data every 10s

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n-------------------------------------------------------");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("-------------------------------------------------------");
  } else {
    Serial.println("\n-------------------------------------------------------");
    Serial.println("WiFi Not connected");
    Serial.println("-------------------------------------------------------");
  }

  udp.begin(udpPort);
  Serial.println("UDP ready");
}

void loop() {
  // Read GPS data continuously
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    Serial.println(gpsSerial.available());
    Serial.println(gpsSerial.read());
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Case 1: GPS is valid and updated
    if (gps.location.isValid() && gps.location.isUpdated()) {
      if (millis() - lastSent >= sendInterval) {
        lastSent = millis();

        float lat = gps.location.lat();
        float lng = gps.location.lng();

        char timestamp[30];
        sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
          gps.date.year(), gps.date.month(), gps.date.day(),
          gps.time.hour(), gps.time.minute(), gps.time.second());

        Serial.println("------------------------------------------------------");
        Serial.print("Latitude: "); Serial.println(lat, 6);
        Serial.print("Longitude: "); Serial.println(lng, 6);
        Serial.print("Timestamp: "); Serial.println(timestamp);
        Serial.println("------------------------------------------------------");

        String message = String(DEVICE_ID) + "," + String(lat, 6) + "," + String(lng, 6);
        sendUDP(message);

        // Optional Firebase push
        /*
        String basePath = "/elephant_locations/" + String(DEVICE_ID);
        firebase.setString(basePath + "/id", DEVICE_ID);
        firebase.setFloat(basePath + "/position/lat", lat);
        firebase.setFloat(basePath + "/position/lng", lng);
        firebase.setString(basePath + "/timestamp", String(timestamp));
        Serial.println("Data pushed to Firebase");
        */
      }

    // Case 2: GPS not valid but WiFi is OK → send dummy
    } else if (millis() - lastDummySent >= dummyInterval) {
      lastDummySent = millis();
      String dummy = String(DEVICE_ID) + ",___,___";
      sendUDP(dummy);
      Serial.println("GPS not available. Sent dummy data.");
    }

  // Case 3: WiFi not connected → try reconnecting
  } else {
    Serial.println("WiFi not connected. Attempting reconnect...");
    Connect_WiFi();
  }

  // delay(100);
}

void sendUDP(String message) {
  IPAddress ip = getBroadcastIP();
  udp.beginPacket(ip, udpPort);
  udp.write(message.c_str());
  udp.endPacket();
  // Serial.println("UDP Sent: " + message);
}

IPAddress getBroadcastIP() {
  IPAddress ip = WiFi.localIP();
  ip[3] = 255;
  return ip;
}

void Connect_WiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  // while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
  //   delay(500);
  //   Serial.print(".");
  // }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n-------------------------------------------------------");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("-------------------------------------------------------");
  } else {
    Serial.println("\n-------------------------------------------------------");
    Serial.println("WiFi Not connected");
    Serial.println("-------------------------------------------------------");
  }
}
