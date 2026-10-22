#include <NewPing.h>
#define TRIG_PIN 5
#define ECHO_PIN 6
#define MAX_DISTANCE 400
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
void setup() {
    Serial.begin(112500);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    Serial.println("Let's begin!");
}

void loop() {
    other();
}

void other() {
    delay(50);
    unsigned int uS = sonar.ping();
    Serial.print("Ping: ");
    Serial.print(uS / US_ROUNDTRIP_CM);
    Serial.println("cm");
}