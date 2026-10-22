#include <Adafruit_NeoPixel.h>
#define NUM_PIXELS8
#define PIN18
Adafruit_NeoPixelstrip= Adafruit_NeoPixel(NUM_PIXELS,PIN,NEO_GRB+
NEO_KHZ800);
void setup(){
strip.begin();
strip.setBrightness(50);
for(int i=0;i%2== 1&&i<NUM_PIXELS;i++){
strip.setPixelColor(i,strip.Color(0,0,0));
}
strip.setPixelColor(0,strip.Color(255,255,255));
strip.setPixelColor(2,strip.Color(128,0,0));
strip.setPixelColor(4,strip.Color(0,128,0));
strip.setPixelColor(6,strip.Color(0,0,128));
strip.show();
}
void loop(){}
