#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* ssid_STA = "";   
const char* password_STA = "";

const char* ssid_AP = "";
const char* password_AP = "";

const char* ca = "-----BEGIN CERTIFICATE-----\n"
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
"1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
"Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
"8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
"pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
"MrY=\n"
"-----END CERTIFICATE-----\n";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress sensorIP(192, 168, 4, 2);

const int port = 80;
WebServer server(port);

float latestDistance = NAN;
String latestNode;
unsigned long latestRxMillis = 0;
bool hasReading = false;
unsigned long lastPoll = 0;
const unsigned long POLL_INTERVAL = 5000;
long lastProcessedEntry = 0;

const char* writeAPIKey_CH1 = "";
const char* readAPIKey_CH2 = "";
const char* channelID_CH2 = "";

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_AP_STA);

    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        Serial.println("Failed to configure static AP IP");
    }

    WiFi.softAP(ssid_AP, password_AP);
    Serial.println("Access Point (AP) started");

    WiFi.begin(ssid_STA, password_STA);
    Serial.print("Connecting to router");

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(500);
        Serial.print(".");
    }

    Serial.print("Access Point IP: ");
    Serial.println(WiFi.softAPIP());

    Serial.print("STA IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("STA status: ");
    Serial.println(WiFi.status());

    configTime(8 * 3600, 0, "pool.ntp.org");

    server.on("/api/sensor", HTTP_POST, handleSensor);
    server.begin();
    Serial.println("Server started");
}

void loop() {
    server.handleClient();
    if (millis() - lastPoll >= POLL_INTERVAL) {
        lastPoll = millis();
        pollThingSpeak();
    }
}

void handleSensor() {
    Serial.println("Sensor POST request received");

    time_t timestamp = time(nullptr);

    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);

    char timeString[32];
    strftime(timeString, sizeof(timeString),
             "%Y-%m-%d %H:%M:%S", &timeinfo);

    Serial.printf("Received at: %s\n", timeString);

    String payload = server.arg("plain");

    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        server.send(400, "application/json",
                    "{\"error\":\"Invalid JSON\"}");
        return;
    }

    // Validate required fields and types
    if (!doc["node"].is<const char*>() ||
        !doc["distance"].is<float>()) {
        server.send(400, "application/json",
                    "{\"error\":\"Missing or invalid fields\"}");
        return;
    }

    // Extract values
    String node = doc["node"].as<String>();
    float distance = doc["distance"].as<float>();

    Serial.printf("Node: %s\n", node.c_str());
    Serial.printf("Distance: %.2f cm\n", distance);

    latestNode = node;
    latestDistance = distance;
    latestRxMillis = millis();
    hasReading = true;

    sendToThingSpeak(latestDistance);

    // Send HTTP response
    server.send(200, "application/json",
                "{\"status\":\"success\"}");

}

void sendToThingSpeak(float distance) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("STA not connected");
        return;
    }

    WiFiClientSecure client;
    client.setCACert(ca);

    HTTPClient http;

    if (!http.begin(client, "https://api.thingspeak.com/update")) {
        Serial.println("Failed to initialise HTTPS");
        return;
    }

    http.addHeader(
        "Content-Type",
        "application/x-www-form-urlencoded"
    );

    String payload = "api_key=" + String(writeAPIKey_CH1)
                   + "&field1=" + String(distance, 2);

    int httpCode = http.POST(payload);

    if (httpCode > 0) {
        Serial.printf("HTTP response: %d\n", httpCode);
        Serial.println(http.getString());
    } else {
        Serial.printf("POST failed: %s\n",
                      http.errorToString(httpCode).c_str());
    }

    http.end();
}

void pollThingSpeak() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("STA disconnected");
        return;
    }

    WiFiClientSecure client;
    client.setCACert(ca);

    HTTPClient http;

    String url = "https://api.thingspeak.com/channels/"
               + String(channelID_CH2)
               + "/feeds/last.json?api_key="
               + String(readAPIKey_CH2);

    if (!http.begin(client, url)) {
        Serial.println("HTTPS initialization failed");
        return;
    }

    http.setTimeout(5000);

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("ThingSpeak GET failed: %d\n", httpCode);
        http.end();
        return;
    }

    String response = http.getString();
    http.end();

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        Serial.println("Invalid ThingSpeak JSON");
        return;
    }

    long entryID = doc["entry_id"] | 0L;

    // Ignore commands already processed
    if (entryID <= 0 || entryID <= lastProcessedEntry) {
        return;
    }

    // Extract ThingSpeak fields
    const char* indexStr = doc["field1"].as<const char*>();
    const char* colorStr = doc["field2"].as<const char*>();

    if (!indexStr || !colorStr) {
        Serial.println("Missing LED control fields");
        return;
    }

    char* end;

    long index = strtol(indexStr, &end, 10);
    if (*indexStr == '\0' || *end != '\0' ||
        index < 0 || index > 7) {
        Serial.println("Invalid LED index");
        return;
    }

    long colorCode = strtol(colorStr, &end, 10);
    if (*colorStr == '\0' || *end != '\0' ||
        colorCode < 0 || colorCode > 3) {
        Serial.println("Invalid color code");
        return;
    }

    int r, g, b;

    if (colorCodeToRGB(colorCode, r, g, b)) {
        sendLEDCommand(index, r, g, b);
        lastProcessedEntry = entryID;
        Serial.printf(
            "Entry %ld: LED %ld updated, color code %ld\n",
            entryID, index, colorCode
        );
    } else {
        Serial.println("Invalid input");
    }

    // Forward command to sensor node
}


bool sendLEDCommand(int index, int r, int g, int b) {
    WiFiClient client;
    HTTPClient http;

    String url = "http://" + sensorIP.toString() + "/api/led";

    if (!http.begin(client, url)) return false;

    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);

    JsonDocument command;
    command["index"] = index;
    command["r"] = r;
    command["g"] = g;
    command["b"] = b;

    String body;
    serializeJson(command, body);

    int code = http.POST(body);
    String response = http.getString();

    http.end();

    Serial.printf("Sensor response: %d %s\n",
                  code, response.c_str());

    if (code != 200) return false;

    JsonDocument ack;
    if (deserializeJson(ack, response)) return false;

    return ack["status"] == "success" &&
           ack["index"] == index;
}

bool colorCodeToRGB(int code, int &r, int &g, int &b) {
    switch (code) {
        case 0: // Off
            r = 0; g = 0; b = 0;
            break;

        case 1: // Red
            r = 255; g = 0; b = 0;
            break;

        case 2: // Yellow
            r = 255; g = 255; b = 0;
            break;

        case 3: // Green
            r = 0; g = 255; b = 0;
            break;

        default:
            return false;
    }
    return true;
}

