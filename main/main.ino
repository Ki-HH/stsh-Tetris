#include <Adafruit_NeoPixel.h>
#include <stdint.h>
#include "Timer.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

//prototypes
int mat2vec(int x, int y);

Timer t;

//LED PIN
#define PIN 6

//Button PINs
#define BT_GREEN 5
#define BT_RED 4
#define BT_UP 3
#define BT_RIGHT 2
#define BT_DOWN 9
#define BT_LEFT 8

//Columns and Rows
#define COLUMNS 10
#define RCOLUMNS 9 //determine number of hardware columns
#define ROWS 10

//direction of LED stripe, 0 meaning top to bottom, 1 meaning bottom to top
uint8_t column_direction[COLUMNS] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};

//When buttons are pushed, these flags are set to 1
int BT_GREEN_F = 0;
int BT_RED_F = 0;
int BT_UP_F = 0;
int BT_LEFT_F = 0;
int BT_RIGHT_F = 0;
int BT_DOWN_F = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(COLUMNS*ROWS, PIN, NEO_GRB + NEO_KHZ800);

//temporary testing variables
int X_GLOB = 3;
int Y_GLOB = 4;

void setup() {
  // put your setup code here, to run once:
  strip.begin();
  //strip.show(); // Initialize all pixels to 'off'

  //initialize timer event
  int tickEvent = t.every(1000, blinkLED);

  //initialize timer event for controller readout
  int tickEvent_readout = t.every(50, readoutController);

  //define pullup resistors for all buttons to be pushed
  pinMode(BT_GREEN, INPUT_PULLUP);
  pinMode(BT_RED, INPUT_PULLUP);
  pinMode(BT_RIGHT, INPUT_PULLUP);
  pinMode(BT_LEFT, INPUT_PULLUP);
  pinMode(BT_UP, INPUT_PULLUP);
  pinMode(BT_DOWN, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  t.update();
  //t2.update();
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void blinkLED() {
  //movement direction is counter intuitive, but works this way
  if(BT_RIGHT_F == 1) {
    X_GLOB -= 1;
  }
  if(BT_LEFT_F == 1) {
    X_GLOB += 1;
  }
  if(BT_DOWN_F == 1) {
    Y_GLOB += 1;
  }
  if(BT_UP_F == 1) {
    Y_GLOB -= 1;
  }
  int coordinate = mat2vec(X_GLOB,Y_GLOB);
  if(BT_GREEN_F == 1) {
    strip.setPixelColor(coordinate, strip.Color(0,25,0));
  } else if(BT_RED_F == 1) {
    strip.setPixelColor(coordinate, strip.Color(25,0,0));
  } else {
    strip.setPixelColor(coordinate, strip.Color(25,25,25));
  }
  strip.show();
  delay(500);
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

// This function reads out the controller pins and sets the global flag variables
void readoutController() {
  if(digitalRead(BT_GREEN) == LOW){
    BT_GREEN_F = 1;
  } else {
    BT_GREEN_F = 0;
  }

  if(digitalRead(BT_RED) == LOW){
    BT_RED_F = 1;
  } else {
    BT_RED_F = 0;
  }

  if(digitalRead(BT_RIGHT) == LOW){
    BT_RIGHT_F = 1;
  } else {
    BT_RIGHT_F = 0;
  }

  if(digitalRead(BT_LEFT) == LOW){
    BT_LEFT_F = 1;
  } else {
    BT_LEFT_F = 0;
  }

  if(digitalRead(BT_UP) == LOW){
    BT_UP_F = 1;
  } else {
    BT_UP_F = 0;
  }

  if(digitalRead(BT_DOWN) == LOW){
    BT_DOWN_F = 1;
  } else {
    BT_DOWN_F = 0;
  }
}

//function expects an array of rgb values that goes through columns first
void renderArray(int r_arr[COLUMNS*ROWS],int g_arr[COLUMNS*ROWS],int b_arr[COLUMNS*ROWS]) {
  int kk=0;
  for(int yy=0;yy<ROWS;yy++) {
    for(int xx=0;xx<COLUMNS;xx++) {
      int coordinate = mat2vec(xx,yy);
      strip.setPixelColor(coordinate, strip.Color(r_arr[kk],g_arr[kk],b_arr[kk]));
    }
    kk+=1;
  }
  strip.show();
}

