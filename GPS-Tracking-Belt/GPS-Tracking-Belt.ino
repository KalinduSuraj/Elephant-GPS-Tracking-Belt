#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>

#define WIFI_SSID "Suraj" //wifi Name
#define WIFI_PASSWORD "11114444" // password
#define BELT_ID "E3"

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected.");
  Serial.println(WiFi.localIP());
 
}
 
void loop() {

} 
