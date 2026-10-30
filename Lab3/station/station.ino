// EE4216 Lab 3 - Task 2: Sensor node (ESP32-S3 A)
//
// Joins the gateway AP as a station, measures ultrasonic distance and
// sends it to the gateway every 5 s:
//
//   POST /api/sensor HTTP/1.1
//   Host: 192.168.4.1
//   Content-Type: application/json
//
//   {"node":"sensor01","distance":35.6}
//
// Adapted from STA_Connection_A.ino; sonar wiring from Lab 2 IoT_Server.

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NewPing.h>
#include <Adafruit_NeoPixel.h>
#include <WebServer.h>

WebServer server(80);
#define NUM_PIXELS 8
#define LED_PIN 18
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---- Gateway AP (same as STA_Connection_A) ----
const char* ssid     = "";
const char* password = "";
const unsigned long TIMEOUT_MS = 15000; // 15 s

// ---- Gateway endpoint ----
const char* GATEWAY_URL = "http://192.168.4.1/api/sensor";
const char* NODE_ID     = "sensor01";
const unsigned long SEND_INTERVAL_MS = 20000;  // every 20 s
const uint16_t HTTP_TIMEOUT_MS = 3000;        // never hang on a dead gateway

// ---- Ultrasonic sensor (Lab 1/2 wiring) ----
const int TRIG_PIN = 5, ECHO_PIN = 6, MAX_DISTANCE_CM = 400;
const float SOUND_SPEED_CM_PER_US = 0.0343;
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE_CM);

// ---- Wi-Fi retry ----
const unsigned long WIFI_RETRY_MS = 10000;
unsigned long lastWifiAttempt = 0;
unsigned long lastSend = 0;

// Returns distance in cm (1 decimal), or -1 if no valid echo
float readDistanceCm() {
  unsigned long us = sonar.ping();
  if (us == 0) return -1;
  return round(us * SOUND_SPEED_CM_PER_US / 2.0 * 10) / 10.0;
}

void sendReading(float distance) {
  JsonDocument doc;
  doc["node"] = NODE_ID;
  doc["distance"] = distance;
  String payload;
  serializeJson(doc, payload);

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.setConnectTimeout(HTTP_TIMEOUT_MS);
  if (!http.begin(GATEWAY_URL)) {
    Serial.println("[HTTP] begin() failed");
    return;
  }
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(payload);
  if (code > 0) {
    Serial.printf("[HTTP] POST %s -> %d %s\n", payload.c_str(), code,
                  http.getString().c_str());
  } else {
    Serial.printf("[HTTP] POST failed: %s\n", http.errorToString(code).c_str());
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  strip.begin();
  strip.clear();
  strip.show();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s", ssid);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connection result: SUCCESS");
    Serial.printf("SSID:    %s\n", ssid);
    Serial.printf("RSSI:    %d dBm\n", WiFi.RSSI());
    Serial.printf("IP:      %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("MAC:     %s\n", WiFi.macAddress().c_str());
  } else {
    Serial.printf("Connection result: FAILED (status %d) after %lu s.\n",
                  WiFi.status(), TIMEOUT_MS / 1000);
    Serial.println("Will keep retrying in loop().");
  }
  lastWifiAttempt = millis();

  server.on("/api/led", HTTP_POST, handleLedPost);
  server.begin();
}

void loop() {
  unsigned long now = millis();

  // Reconnect to the gateway AP without blocking
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWifiAttempt >= WIFI_RETRY_MS) {
      lastWifiAttempt = now;
      Serial.println("[WiFi] Not connected, retrying...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    return;
  }

  if (now - lastSend >= SEND_INTERVAL_MS) {
    lastSend = now;
    float d = readDistanceCm();
    if (d < 0) {
      Serial.println("[SONAR] No valid echo, skipping this reading");
      return;
    }
    sendReading(d);
  }
}

void handleLedPost() {
  String payload = server.arg("plain");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
      server.send(400, "application/json",
                  "{\"error\":\"Invalid JSON\"}");
      return;
  }

  // 2. Validate required fields and types
  if (!doc["index"].is<int>() ||
      !doc["r"].is<int>() ||
      !doc["g"].is<int>() ||
      !doc["b"].is<int>()) {

      server.send(400, "application/json",
                  "{\"error\":\"Missing or invalid fields\"}");
      return;
  }

  int index = doc["index"].as<int>();
  int r = doc["r"].as<int>();
  int g = doc["g"].as<int>();
  int b = doc["b"].as<int>();

  // 3. Validate LED index
  if (index < 0 || index >= NUM_PIXELS) {
      server.send(400, "application/json",
                  "{\"error\":\"LED index out of range\"}");
      return;
  }

  // 4. Validate RGB values
  if (r < 0 || r > 255 ||
      g < 0 || g > 255 ||
      b < 0 || b > 255) {

      server.send(400, "application/json",
                  "{\"error\":\"RGB values must be 0-255\"}");
      return;
  }

  // 5. Update only the specified LED
  strip.setPixelColor(index, strip.Color(r, g, b));
  strip.show();

  // 6. Send acknowledgment
  JsonDocument response;

  response["status"] = "success";
  response["index"] = index;
  response["r"] = r;
  response["g"] = g;
  response["b"] = b;

  String responseBody;
  serializeJson(response, responseBody);

  server.send(200, "application/json", responseBody);
}
