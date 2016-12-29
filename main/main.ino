#include <Adafruit_NeoPixel.h>
#include <stdint.h>
#include "Timer.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

//prototypes
int mat2vec(int x, int y);

Timer t;

#define PIN 6

#define COLUMNS 10
#define RCOLUMNS 9 //determine number of hardware columns
#define ROWS 10

//direction of LED stripe, 0 meaning top to bottom, 1 meaning bottom to top
uint8_t column_direction[COLUMNS] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};



// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(COLUMNS*ROWS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  strip.begin();
  //strip.show(); // Initialize all pixels to 'off'

  //initialize timer event
  int tickEvent = t.every(2000, blink);
  //Serial.print("2 second tick started id=");
}

void loop() {
  // put your main code here, to run repeatedly:
  t.update();
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void blink() {
  int coordinate = mat2vec(3,4);
  strip.setPixelColor(coordinate, strip.Color(25,25,25));
  strip.show();
  delay(1000);
  strip.setPixelColor(coordinate, strip.Color(0,0,0));
  strip.show();
}

int mat2vec(int x, int y) {
  // counting x and y from the bottom left with 0,0
  
  uint8_t temp_int = (RCOLUMNS-(x+1))*ROWS;
  if(column_direction[x] == 0) {
    temp_int += ROWS-(y+1);
  } else if(column_direction[x] == 1) {
    temp_int += (y);
  }
  return temp_int;
}

