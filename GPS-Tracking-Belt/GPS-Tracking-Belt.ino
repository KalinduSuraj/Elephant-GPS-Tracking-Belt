#define DEVICE_ID "E04"

#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266Firebase.h>

#define WIFI_SSID "Suraj"
#define WIFI_PASSWORD "11114444"

#define FIREBASE_HOST "https://elephant-tracking-app-default-rtdb.asia-southeast1.firebasedatabase.app"

Firebase firebase(FIREBASE_HOST);
TinyGPSPlus gps;
SoftwareSerial gpsSerial(D2, D1);  // RX, TX

unsigned long lastSent = 0;
const unsigned long sendInterval = 5000;  // milliseconds

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Connect_WiFi();
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid() && gps.location.isUpdated()) {
    unsigned long now = millis();
    if (now - lastSent >= sendInterval) {
      lastSent = now;

      String latitude = String(gps.location.lat(), 6);
      String longitude = String(gps.location.lng(), 6);

      char timestamp[30];
      sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
        gps.date.year(),
        gps.date.month(),
        gps.date.day(),
        gps.time.hour(),
        gps.time.minute(),
        gps.time.second());

      Serial.println("------------------------------------------------------");
      Serial.print("Latitude: "); Serial.println(latitude);
      Serial.print("Longitude: "); Serial.println(longitude);
      Serial.print("Timestamp: "); Serial.println(timestamp);
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

        firebase.setFloat(basePath + "/position/lat", gps.location.lat());
        firebase.setFloat(basePath + "/position/lng", gps.location.lng());
        firebase.setString(basePath + "/timestamp", timestamp);

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
    
  }else{
    Serial.println("-------------------------------------------------------");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("-------------------------------------------------------");

  }
  delay(3000);
}
