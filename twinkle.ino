#include "SPI.h"

#define __WS2801

#ifdef __WS2801
#include "Adafruit_WS2801.h"
#else
#include <Adafruit_NeoPixel.h>
#endif

#define UNINITIALIZED    -1   // This is how we identify an uninitialized pixel
#define LED_STRIP_LENGTH 42   // The total number of LED pixels
#define TOTAL_TWINKLES   10   // The number of LED pixels that will twinkle at once
#define INPUT1_PIN       8    // A pin that is attached to the accelerometer
#define INPUT2_PIN       9    // A pin that is attached to the accelerometer
#define DATA_PIN         13   // Data wire for pixels (both WS2801 & WS2811 style)

#ifdef __WS2801
#define CLOCK_PIN         12   // Clock wire for WS2801-style pixels only
Adafruit_WS2801 strip = Adafruit_WS2801(LED_STRIP_LENGTH, DATA_PIN, CLOCK_PIN);
#else
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_STRIP_LENGTH, DATA_PIN, NEO_GRB + NEO_KHZ800);
#endif

struct Twinkle
{
  int index;
  
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint32_t color;         // What we are heading for...
  
  int denominator;        // The magic number we are dividing by
  int numerator;          // where we are in the multiplication
  char direction;         // the direction we are headed (brighter/dimmer)
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

  if (digitalRead(INPUT1_PIN) != HIGH) {
    return;
  }
  
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

  pinMode(INPUT1_PIN, INPUT);
  pinMode(INPUT2_PIN, INPUT);
  randomSeed(analogRead(0));
  resetTwinkles();
}

void loop() {
  if (digitalRead(INPUT2_PIN) != HIGH) {
    displayTwinkles(10);
    stepTwinkles();
    initTwinkle();
  }
  else {
    for (int i=0; i<LED_STRIP_LENGTH; i++) {
      strip.setPixelColor(i, Color(255,0,0));
    }
    strip.show();
  }
}
