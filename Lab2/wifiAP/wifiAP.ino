#include <WiFi.h>
const char* ssid = "A0288423X";
const char* password = "password123";
int max_connection = 2;
int ssid_hidden = 0;

void setup() {
    Serial.begin(115200);
    // Start the Wi-Fi access point
    WiFi.softAP(ssid, password, ssid_hidden, max_connection);
    // Print the IP address of the access point
    Serial.print("Access Point IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("Access Point MAC address: ");
    Serial.println(WiFi.softAPmacAddress());
}

void loop() {
    // Print the number of connected clients
    int numStations = WiFi.softAPgetStationNum();
    Serial.print("Connected devices: ");
    Serial.println(numStations);

    delay(5000);
}