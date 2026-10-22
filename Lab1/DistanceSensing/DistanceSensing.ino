#include <NewPing.h>
#include <Adafruit_NeoPixel.h>
#define TRIG_PIN 5
#define ECHO_PIN 6
#define MAX_DISTANCE 400
#define NUM_PIXELS 8
#define PIN 18

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(112500);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);
  strip.begin();
  strip.setBrightness(50);
  Serial.println("Let's begin!");
  strip.clear();
  strip.show();
}

void loop() {
  unsigned int distance = sonar.ping_cm();
  uint32_t color;
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 30 || distance == 0) {
    color = strip.Color(255, 0, 0);
  }
  
  else if (distance <= 100) {
    color = strip.Color(255, 128, 0);
  }

  else {
    color = strip.Color(0, 255, 0);
  }

  strip.fill(color, 0, NUM_PIXELS);
  strip.show();
  delay(100);
}