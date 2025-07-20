#define DEVICE_ID "E01"

#include <ESP8266WiFi.h>
#include <TinyGPS++.h>       // GPS parsing library
#include <SoftwareSerial.h>  // For serial communication with GPS
#include <ESP8266Firebase.h>

// WiFi credentials
#define WIFI_SSID "Suraj"
#define WIFI_PASSWORD "11114444"

// Create GPS object
TinyGPSPlus gps;

// Create software serial for GPS: RX = D2, TX = D1 
SoftwareSerial gpsSerial(D2, D1);//(RX,TX)

// Define latitude, longitude, and timestamp
double latitude = 0.0;
double longitude = 0.0;
String timestamp = "";

// Define firebase_host
#define FIREBASE_HOST "https://elephant-tracking-app-default-rtdb.asia-southeast1.firebasedatabase.app"
Firebase firebase(FIREBASE_HOST);

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Connect_WiFi(); 
}

void loop() {
  SetGPS();
  checkID();
  
}

void Connect_WiFi() {
  // Connect to Wi-Fi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void SetGPS(){
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());

    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();

      // Format timestamp (e.g., YYYY/MM/DD HH:MM:SS)
      timestamp = String(gps.date.year()) + "/" +
                  String(gps.date.month()) + "/" +
                  String(gps.date.day()) + " " +
                  String(gps.time.hour()) + ":" +
                  String(gps.time.minute()) + ":" +
                  String(gps.time.second());

      Serial.print("Latitude: ");
      Serial.println(latitude);
      Serial.print("Longitude: ");
      Serial.println(longitude);
      Serial.print("Timestamp: ");
      Serial.println(timestamp);
      delay(2000);
    }
  }
}

void checkID() {
  // String path = "/elephant_locations/" + String(DEVICE_ID) + "/id";
  String path = "/";
  String result = firebase.getString(path); 

  Serial.println(path);
  Serial.println(result);

  if (result == DEVICE_ID) {
    Serial.println("ID exists. Updating...");
    // updateData();
  } else {
    Serial.println("ID not found. Creating new object...");
    // createNewObject();
  }
}

void updateData() {
  Serial.println("Updating Data");

  // String basePath = "/elephant_locations/" + String(DEVICE_ID);
  // firebase.setFloat(basePath + "/position/lat", latitude);
  // firebase.setFloat(basePath + "/position/lng", longitude);
  // firebase.setString(basePath + "/timestamp", timestamp); // formatted like "2025-06-22T03:02:00Z"

  Serial.println("Updated Data");
}

void createNewObject() {
  Serial.println("Creating New Obj");

  // String basePath = "/elephant_locations/" + String(DEVICE_ID);
  // firebase.setString(basePath + "/id", DEVICE_ID);
  // updateData();  // reuse same method

  Serial.println("Created New Obj");
}

