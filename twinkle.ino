#include "SPI.h"
#include "Adafruit_WS2801.h"

#define UNINITIALIZED    -1  // This is how we identify an uninitialized pixel
#define LED_STRIP_LENGTH 18   // The total number of LED pixels
#define TOTAL_TWINKLES   4   // The number of LED pixels that will twinkle at once

#define dataPin    13        // Yellow wire on Adafruit Pixels
#define clockPin   12        // Green wire on Adafruit Pixels
Adafruit_WS2801 strip = Adafruit_WS2801(LED_STRIP_LENGTH, dataPin, clockPin);

struct Twinkle
{
  int index;
  
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint32_t color;         // What we are heading for...
  
  int denominator;        // The magic number we are dividing by
  int numerator;          // where we are in the multiplication
  char direction;         // the direction we are headed
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
  uint8_t i=0, j=0, m=0;
  int led=0;
  
  for (i=0; i<TOTAL_TWINKLES; i++) {
    
    // We set the index to 0 on purpose to identify when a specific
    // twinkle needs to be re-initialized... cause they will come and
    // go at random
    
    // Skip any non-zero (already initialized) twinkles...
    if (twinkles[i].index != UNINITIALIZED) continue;  
    
pickNewRandom:
    led = random(LED_STRIP_LENGTH);
    
    // Is that number already used?
    for (j=0; j<TOTAL_TWINKLES; j++) {
      if (twinkles[j].index == led) {
        goto pickNewRandom;
      }
    }
    
    // Give the next LED that new value
    twinkles[i].index = led;
    
    // Pick a random END color...
    twinkles[i].r = random(256);
    twinkles[i].g = random(256);
    twinkles[i].b = random(256);

    // And the color we display each time...
    twinkles[i].color = Color(0,0,0);
    
    // Pick a percentage of brightness we are at now...
    twinkles[i].denominator = random(100,255);
    twinkles[i].numerator = 0;
    twinkles[i].direction = random(16);
  }
}

void resetTwinkles()
{
  uint8_t i=0;
  for (i=0; i<TOTAL_TWINKLES; i++) {
    twinkles[i].index = UNINITIALIZED;
    twinkles[i].r = 0;
    twinkles[i].g = 0;
    twinkles[i].b = 0;
    twinkles[i].numerator = 0;
    twinkles[i].direction = 1;
  }
  
  // Initialize all uninitialized twinkles... 
  // which at this point is all of them...
  initTwinkle();
}

void displayTwinkles(uint8_t wait)
{
  uint8_t i=0,j=0;
  boolean set = false;
  for (i=0; i<LED_STRIP_LENGTH; i++) {
    
    // If it was one of the Twinkles, set it here
    for (j=0; j<TOTAL_TWINKLES; j++) {
      if (twinkles[j].index != i) continue;
      strip.setPixelColor(i, twinkles[j].color); 
      set = true;
      break;
    }
    
    // If it was not one of the Twinkles, clear it
    if (set == false) {
      strip.setPixelColor(i, 0);
    }
    
    // Clear the value
    set = false;
  }
  
  strip.show();
  delay(wait);
}

void stepTwinkles()
{
  uint8_t i=0,j=0;
  for (i=0; i<LED_STRIP_LENGTH; i++) {
    for (j=0; j<TOTAL_TWINKLES; j++) {
      if (twinkles[j].index != i) continue;
      
      // set the color...
      twinkles[j].color = Color(
        (byte)(((double)twinkles[j].r)*((double)twinkles[j].numerator / (double)twinkles[j].denominator)),
        (byte)(((double)twinkles[j].g)*((double)twinkles[j].numerator / (double)twinkles[j].denominator)),
        (byte)(((double)twinkles[j].b)*((double)twinkles[j].numerator / (double)twinkles[j].denominator))
      );
      
      // add 'd' to 'y' until it reaches 'x' or '0'...
      twinkles[j].numerator += twinkles[j].direction;
      if (twinkles[j].numerator >= twinkles[j].denominator) {
        twinkles[j].numerator = twinkles[j].denominator;
        twinkles[j].direction = -1 * twinkles[j].direction;
      }
      else
      if (twinkles[j].numerator <= 0) {
        twinkles[j].numerator = 0;
        twinkles[j].direction = 1;
        twinkles[j].index = UNINITIALIZED;
        initTwinkle();
      }
      
      // If we got here it was because index == i, so we can break
      break;
    }
  }
}

void setup() { 
  Serial.begin(19200);
  strip.begin();
  strip.show();
  
  randomSeed(analogRead(0));
  resetTwinkles();
}

void loop() {
  displayTwinkles(10);
  stepTwinkles();
}
