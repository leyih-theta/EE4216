#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <lab_page.h>


const char* ssid = "A0288423X";
const char* password = "password123";

// const char* host = "example.com";   
const int port = 80;

int max_connection = 2;
int ssid_hidden = 0;
AsyncWebServer server(port);

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to wifi...")
    }

    Serial.println("Connected to WiFi!");
    
    server.begin()
    Serial.println("Server started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", LAB_PAGE);
    });

    server.on("/api/distance")
}

void loop() {}



void handleStatus() {
    Serial.println("Status request received");
    String message = "ESP32 Status: OK\n";
    message += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    message += "WiFi Signal Strength: " + String(WiFi.RSSI()) + " dBm\n";
    // Send the response
    server.send(200, "text/plain", message);
}