#include "SPI.h"
#include "Adafruit_WS2801.h"

#define UNINITIALIZED    -1  // This is how we identify an uninitialized "twinkle"
#define LED_STRIP_LENGTH 16  // The total number of LED pixels
#define TOTAL_TWINKLES   4   // The number of LED pixels that will twinkle at once

uint8_t dataPin  = 13;       // Yellow wire on Adafruit Pixels
uint8_t clockPin = 12;       // Green wire on Adafruit Pixels
Adafruit_WS2801 strip = Adafruit_WS2801(LED_STRIP_LENGTH, dataPin, clockPin);

struct Twinkle
{
  int index;
  int brightness;         // has to be an int so we can go < 0, but also up to 255
  uint32_t color;
  
  char stepSize;          // min:-128, max:127
  uint8_t maxBrightness;  // min:0, max: 255
};

Twinkle twinkles[TOTAL_TWINKLES];

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

void initTwinkle()
{
  uint8_t i=0, j=0;
  int newRandom=0;
  
  for (i=0; i<TOTAL_TWINKLES; i++) {
    
    // We set the index to 0 on purpose to identify when a specific
    // twinkle needs to be re-initialized... cause they will come and
    // go at random
    
    // Skip any non-zero (already initialized) twinkles...
    if (twinkles[i].index != UNINITIALIZED) continue;  
    
pickNewRandom:
    newRandom = random(LED_STRIP_LENGTH);
    
    // Is that number already used?
    for (j=0; j<TOTAL_TWINKLES; j++) {
      if (twinkles[j].index == newRandom) {
        goto pickNewRandom;
      }
    }
    
    // Give the next LED that new value
    twinkles[i].index = newRandom;
    // Pick a random color
    twinkles[i].color = Color(random(256), random(256), random(256));
    // Pick a random stepSize
    twinkles[i].stepSize = random(10);
    // Pick a random maxBrightness
    twinkles[i].maxBrightness = random(255 - (twinkles[i].stepSize + 1));
  }
}

void resetTwinkles()
{
  uint8_t i=0;
  for (i=0; i<TOTAL_TWINKLES; i++) {
    twinkles[i].index = UNINITIALIZED;
  }
  
  // Initialize all uninitialized twinkles... which at this point is
  // all of them...
  initTwinkle();
}

void displayTwinkles(uint8_t wait)
{
  uint8_t i=0;
  for (i=0; i<TOTAL_TWINKLES; i++) {
    if (twinkles[i].index == UNINITIALIZED) continue;
    strip.setPixelColor(twinkles[i].index, twinkles[i].color);
  }
  strip.show();
  delay(wait);
}

void stepTwinkles()
{
  uint8_t i=0;
  for (i=0; i<TOTAL_TWINKLES; i++) {
    twinkles[i].brightness += twinkles[i].stepSize;    
    if (twinkles[i].brightness > twinkles[i].maxBrightness) {
      twinkles[i].stepSize = -1 * twinkles[i].stepSize;
    }
    if (twinkles[i].brightness < 0) {
      twinkles[i].index = UNINITIALIZED;
      initTwinkle(); // Start another random twinkle...
    }
  }
}

void setup() { 
  strip.begin();
  strip.show();
  
  randomSeed(analogRead(0));
  resetTwinkles();
}

void loop() {
  displayTwinkles(10);
  stepTwinkles();
}
