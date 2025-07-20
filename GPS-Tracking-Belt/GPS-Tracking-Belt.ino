#define DEVICE_ID "E01"

#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266Firebase.h>  // Your library, unchanged

#define WIFI_SSID "Suraj"
#define WIFI_PASSWORD "11114444"

#define FIREBASE_HOST "https://elephant-tracking-app-default-rtdb.asia-southeast1.firebasedatabase.app"
Firebase firebase(FIREBASE_HOST);

TinyGPSPlus gps;
SoftwareSerial gpsSerial(D2, D1);

String latitude = "1";
String longitude = "1";
String timestamp = "1";

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Connect_WiFi();
}

void loop() {
  // Serial.println(WiFi.status());

  SetGPS();

  // Serial.print("Latitude: ");
  // Serial.println(latitude);
  // Serial.print("Longitude: ");
  // Serial.println(longitude);
  // Serial.print("Timestamp: ");
  // Serial.println(timestamp);
  // delay(2000);


  checkID();   // call to update firebase every loop after GPS update

  // delay(3000);
}

void Connect_WiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    // delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void SetGPS() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());

    if (gps.location.isUpdated()) {
      latitude = String(gps.location.lat(), 6);
      longitude = String(gps.location.lng(), 6);

      timestamp = String(gps.date.year()) + "/" +
                  String(gps.date.month()) + "/" +
                  String(gps.date.day()) + " " +
                  String(gps.time.hour()) + ":" +
                  String(gps.time.minute()) + ":" +
                  String(gps.time.second());

  Serial.println("------------------------------------------------------");
      Serial.print("Latitude: ");
      Serial.println(latitude);
      Serial.print("Longitude: ");
      Serial.println(longitude);
      Serial.print("Timestamp: ");
      Serial.println(timestamp);
Serial.println("------------------------------------------------------");

      delay(5000);
    }
  }
}

void checkID() {
  String path = "/elephant_locations/" + String(DEVICE_ID) + "/id";

  String result = firebase.getString(path);  // Using your library's method

  Serial.println(path);
  Serial.println(result);

  if (result == DEVICE_ID) {
    Serial.println("ID exists. Updating...");
    updateData();
  } else {
    Serial.println("ID not found. Creating new object...");
    createNewObject();
  }
}

void updateData() {
  Serial.println("Updating Data");

  String basePath = "/elephant_locations/" + String(DEVICE_ID);

  firebase.setFloat(basePath + "/position/lat", latitude.toFloat());
  firebase.setFloat(basePath + "/position/lng", longitude.toFloat());
  firebase.setString(basePath + "/timestamp", timestamp);

  Serial.println("Updated Data");
}

void createNewObject() {
  Serial.println("Creating New Obj");

  String basePath = "/elephant_locations/" + String(DEVICE_ID);

  firebase.setString(basePath + "/id", DEVICE_ID);
  updateData();

  Serial.println("Created New Obj");
}
