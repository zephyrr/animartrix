// A 5D coordinate mapper by Stefan Petrick 2023 
//
// VO.3 preview version
// 
// This code is licenced under a Creative Commons Attribution 
// License CC BY-NC 3.0

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>
#include <FastLED.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 32;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);


#define num_x  kMatrixWidth                       // how many LEDs are in one row?
#define num_y  kMatrixHeight                       // how many rows?
#define NUM_LEDS ((num_x) * (num_y))


CRGB leds[num_x * num_y];               // framebuffer

float polar_theta[num_x][num_y];        // look-up table for polar angles
float distance[num_x][num_y];           // look-up table for polar distances

unsigned long a, b, c;                  // for time measurements

struct render_parameters {

  float center_x = (num_x / 2) - 0.5;   // center of the matrix
  float center_y = (num_y / 2) - 0.5;
  float dist, angle;                
  float scale_x = 0.1;                  // smaller values = zoom in
  float scale_y = 0.1;
  float scale_z = 0.1;       
  float offset_x, offset_y, offset_z;     
  float z;  
  float low_limit  = 0;                 // getting contrast by highering the black point
  float high_limit = 1;                                            
};

render_parameters animation;     // all animation parameters in one place

#define num_oscillators 10

struct oscillators {

  float master_speed;            // global transition speed
  float offset[num_oscillators]; // oscillators can be shifted by a time offset
  float ratio[num_oscillators];  // speed ratios for the individual oscillators                                  
};

oscillators timings;             // all speed settings in one place

struct modulators {  

  float linear[num_oscillators];        // returns 0 to FLT_MAX
  float radial[num_oscillators];        // returns 0 to 2*PI
  float directional[num_oscillators];   // returns -1 to 1
  float noise_angle[num_oscillators];   // returns 0 to 2*PI        
};

modulators move;                 // all oscillator based movers and shifters at one place

struct rgb {

  float red, green, blue;
};

rgb pixel;



//******************************************************************************************************************


void setup() {

  Serial.begin(115200);                 // check serial monitor for current fps count
  
  matrix.addLayer(&backgroundLayer); 
  matrix.begin();

  matrix.setBrightness(255);

  backgroundLayer.enableColorCorrection(true);    
 
  render_polar_lookup_table((num_x / 2) - 0.5, (num_y / 2) - 0.5);          // precalculate all polar coordinates 
                                                                            // polar origin is set to matrix centre
  
}

//*******************************************************************************************************************

void loop() {

  //Yves();
  Scaledemo1();
  //Lava1();
  //Caleido3();
  //Caleido2();
  //Caleido1();
  //Distance_Experiment();
  //Center_Field();
  //Waves();
  Chasing_Spirals();
  //Rotating_Blob();
  //Rings();

  // if sketch uses swapBuffers(false), wait to get a new backBuffer() pointer after the swap is done:
  while(backgroundLayer.isSwapPending());

  rgb24 *buffer = backgroundLayer.backBuffer();

  for(int p = 0; p < NUM_LEDS; p++) {
    buffer[p] = (rgb24)leds[p];
  }
   // buffer is filled completely each time, use swapBuffers without buffer copy to save CPU cycles
  backgroundLayer.swapBuffers(false);
  matrix.countFPS();      // print the loop() frames per second to Serial
} 

