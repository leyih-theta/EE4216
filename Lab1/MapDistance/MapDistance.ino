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
  int numLit;
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance == 0 || distance <= 10) {
    numLit = NUM_PIXELS;
  }

  else if (distance >= 100) {
    numLit = 1;
  }

  else {
    numLit = NUM_PIXELS - (distance - 10) * 7 / 90;
  }

  strip.clear();

  for (int i = 0; i < numLit; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }

  strip.show();
  delay(100);
}