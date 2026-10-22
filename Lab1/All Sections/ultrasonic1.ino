#define TRIG_PIN 5
#define ECHO_PIN 6
#define MAX_DISTANCE 400

void setup() {
    Serial.begin(112500);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    Serial.println("Let's begin!");
}

void loop() {
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = (duration * 0.0343) / 2;
    Serial.printf("Distance: %.02fcm\n", distance);
    delay(400);
}