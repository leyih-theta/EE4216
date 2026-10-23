#include <WiFi.h> // Include the Wi-Fi library
const char* ssid = "Catorce";
const char* password = "iamthesenate";



void setup() {
    Serial.begin(115200); // Initialize serial communication for debugging
    WiFi.begin(ssid, password); // Start Wi-Fi connection
    Serial.print("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(1000);
        Serial.print(".");
    }

    Serial.println("WiFi connected.");
    Serial.println("Connecting to ");
 

    Serial.println("\nConnected to Wi-Fi network");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); // Print the IP address
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("Connection strength:");
    Serial.println(WiFi.RSSI());

    WiFi.mode(WIFI_STA);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi connection lost.  Reconnecting...");
    WiFi.disconnect();
    WiFi.reconnect();

        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.print(".");
        }
    }

    delay(5000);
}