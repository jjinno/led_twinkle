led_twinkle
==============

A random-twinkle program for Arduino + Adafruit WS2801/WS2811 libraries

If you are using WS2801-based LED strips, make sure that the following line occurs at the top of the sketch:

#define __WS2801

Otherwise the code will, by default, resort to using the WS2811 library for the LEDs.
