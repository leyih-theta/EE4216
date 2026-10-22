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

const char* ssid = "ap_name";
const char* password = "password123";
int max_connection = 2;
int ssid_hidden = 0;
 
const int port = 80;

WebServer server(port);

struct LEDState {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t brightness;
};

LEDState ledStates[NUM_PIXELS] = {};

void setup() {
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    strip.begin();
    strip.clear();

    ledStates[1] = {155, 0, 0, 120};
    ledStates[3] = {0, 155, 0, 120};
    ledStates[5] = {0, 0, 155, 120};

    for (int i = 0; i < NUM_PIXELS; i++) {
        LEDState s = ledStates[i];

        strip.setPixelColor(
            i,
            (s.r * s.brightness) / 255,
            (s.g * s.brightness) / 255,
            (s.b * s.brightness) / 255
        );
    }

    strip.show();

    WiFi.softAP(ssid, password, ssid_hidden, max_connection);
    Serial.print("Access Point IP: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
    Serial.println("Server started");

    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/distance", HTTP_GET, handleDistance);
    server.on("/api/leds", HTTP_GET, handleLEDstatus);

    for (int i = 0; i < NUM_PIXELS; i++) {
    String path = "/api/leds/" + String(i);
    server.on(path.c_str(), HTTP_PUT, handleLEDupdate);
    }

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

void handleLEDstatus() {
    Serial.println("LED status request received");
    JsonDocument doc;
    JsonArray leds = doc["leds"].to<JsonArray>();

    for (int i = 0; i < NUM_PIXELS; i++) {
        JsonObject led = leds.add<JsonObject>();

        led["id"] = i;
        led["r"] = ledStates[i].r;
        led["g"] = ledStates[i].g;
        led["b"] = ledStates[i].b;
        led["brightness"] = ledStates[i].brightness;
    }
    
    String output;
    serializeJson(doc, output);

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", output);
}

void handleLEDupdate() {
    String path = server.uri();
    String idStr = path.substring(10);

    if (idStr.length() != 1 ||
        idStr[0] < '0' || idStr[0] > '7') {
        server.send(404, "application/json",
                    "{\"error\":\"Invalid LED ID\"}");
        return;
    }

    int id = idStr.toInt();

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, server.arg("plain"));

    if (error || !doc.is<JsonObject>()) {
        server.send(400, "application/json",
                    "{\"error\":\"Invalid JSON object\"}");
        return;
    }

    // Validate RGB and brightness
    if (!doc["r"].is<int>() ||
        !doc["g"].is<int>() ||
        !doc["b"].is<int>() ||
        !doc["brightness"].is<int>()) {

        server.send(400, "application/json",
                    "{\"error\":\"Missing or invalid fields\"}");
        return;
    }

    int r = doc["r"].as<int>();
    int g = doc["g"].as<int>();
    int b = doc["b"].as<int>();
    int brightness = doc["brightness"].as<int>();

    // Validate ranges
    if (r < 0 || r > 255 ||
        g < 0 || g > 255 ||
        b < 0 || b > 255 ||
        brightness < 0 || brightness > 255) {

        server.send(400, "application/json",
                    "{\"error\":\"Values must be 0-255\"}");
        return;
    }

    // Update only the selected logical state
    ledStates[id] = {
        (uint8_t)r,
        (uint8_t)g,
        (uint8_t)b,
        (uint8_t)brightness
    };

    // Scale RGB using per-pixel brightness
    uint8_t scaledR = (r * brightness) / 255;
    uint8_t scaledG = (g * brightness) / 255;
    uint8_t scaledB = (b * brightness) / 255;

    // Update only the selected pixel
    strip.setPixelColor(id, scaledR, scaledG, scaledB);
    strip.show();

    // Return the full logical state
    JsonDocument response;

    response["id"] = id;
    response["r"] = ledStates[id].r;
    response["g"] = ledStates[id].g;
    response["b"] = ledStates[id].b;
    response["brightness"] = ledStates[id].brightness;

    String output;
    serializeJson(response, output);

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