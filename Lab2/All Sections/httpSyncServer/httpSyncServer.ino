#include <WiFi.h>
#include <WebServer.h>
#include <lab_page.h>
#include <NewPing.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#define TRIG_PIN 5
#define ECHO_PIN 6
#define MAX_DISTANCE 400
#define NUM_PIXELS 8
#define LED_PIN 18
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "A0288423X";
const char* password = "hophopsenator";
int max_connection = 2;
int ssid_hidden = 0;
 
const int port = 80;

WebServer server(port);

void setup() {
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    strip.begin();
    strip.clear();
    strip.setPixelColor(3, strip.Color(0, 155, 0));
    strip.show();

    WiFi.softAP(ssid, password, ssid_hidden, max_connection);
    Serial.print("Access Point IP: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
    Serial.println("Server started");

    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/distance", HTTP_GET, handleDistance);
    server.on("/api/leds", HTTP_GET, handleLEDstatus);
    server.on("/api/leds", HTTP_PUT, handleLEDupdate);
    server.onNotFound(handleNotFound);
    server.on("/api/leds", HTTP_OPTIONS, handleOptions);

}

void loop() {
    server.handleClient();
}

void handleRoot() {
    Serial.println("Root request received");
    server.send_P(200, "text/HTML", LAB_PAGE);
}

void handleDistance() {
    Serial.println("Distance request received");
    unsigned int distance = sonar.ping_cm();
    String valid = (distance == 0) ? "false" : "true";
    String distance_cm = "{ \"distance_cm\":" + String(distance) + ", \"valid\":" + valid + "}";
    server.send(200, "application/json", distance_cm);
}

struct LEDState {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t brightness;
};

LEDState ledStates[NUM_PIXELS] = {};

void handleLEDstatus() {
    JsonDocument doc;
    JsonArray leds = doc["leds"].to<JsonArray>();

    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t color = strip.getPixelColor(i);
        JsonObject led = leds.add<JsonObject>();

        led["id"] = i;
        led["r"] = (color >> 16) & 0xFF;
        led["g"] = (color >> 8) & 0xFF;
        led["b"] = color & 0xFF;
        led["brightness"] = ledStates[i].brightness;
    }

    String output;
    serializeJson(doc, output);

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", output);
}


void handleOptions() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

    server.send(204);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}