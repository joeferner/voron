#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN        6
#define NUMPIXELS  3

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pixels.begin();
}

void loop() {
  pixels.clear();

  pixels.setPixelColor(0, pixels.Color(0, 150, 0));
  pixels.setPixelColor(1, pixels.Color(150, 0, 0));
  pixels.setPixelColor(2, pixels.Color(0, 0, 150));
  pixels.show();
//
//  for(int i=0; i<NUMPIXELS; i++) {
//
//    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
//    delay(DELAYVAL);
//  }
}
