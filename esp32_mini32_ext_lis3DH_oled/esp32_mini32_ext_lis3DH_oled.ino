
// Basic demo for accelerometer readings from Adafruit LIS3DH

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include <TimeLib.h>

// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "D:\utilisateur\sully\Documents\Arduino\libraries\ESP32_Oled_Driver_for_SSD1306_display\SSD1306.h" // alias for `#include "SSD1306Wire.h"`

// Include the UI lib
#include "OLEDDisplayUi.h"

// Include custom images
#include "images.h"



// Used for software SPI
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
// Used for hardware & software SPI
#define LIS3DH_CS 10

#define NB_MEAN   10

// software SPI
//Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);
// hardware SPI
//Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);
// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

//varaible
long int moyenne=0;
long int somme=0;
int nb=NB_MEAN;
long int mat_y[NB_MEAN];
int i=0;
int j=0;
long int old_meas=0;

float angle=0;



//oled
// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 21, 22);
// SH1106 display(0x3c, D3, D5);

OLEDDisplayUi ui ( &display );

int screenW = 128; //128;
int screenH = 64;
int screenCenterX = screenW/2;
int screenCenterY = ((screenH-16)/2)+16;   // top yellow part is 16 px height
int screenRadius = 23;

void screenOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
}

void acc_Frame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String acc_txt = String(angle);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24); //24
  display->drawString(screenCenterX + x , screenCenterY + y, acc_txt );
}

void acc_Frame_status(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String acc_txt = String(angle);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24); //24
  display->drawString(screenCenterX + x , screenCenterY + y, acc_txt );
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { acc_Frame_status, acc_Frame };

// how many frames are there?
int frameCount = 1;

// Overlays are statically drawn on top of a frame eg. a screen
OverlayCallback overlays[] = { screenOverlay };
int overlaysCount = 1;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

//  Serial.println("LIS3DH test!");

  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
//  Serial.println("LIS3DH found!");

  // lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

//  Serial.print("Range = "); Serial.print(2 << lis.getRange());
//  Serial.println("G");

  lis.setDataRate(LIS3DH_DATARATE_50_HZ);
  
//  Serial.print("Data rate set to: ");
//  switch (lis.getDataRate()) {
//    case LIS3DH_DATARATE_1_HZ: Serial.println("1 Hz"); break;
//    case LIS3DH_DATARATE_10_HZ: Serial.println("10 Hz"); break;
//    case LIS3DH_DATARATE_25_HZ: Serial.println("25 Hz"); break;
//    case LIS3DH_DATARATE_50_HZ: Serial.println("50 Hz"); break;
//    case LIS3DH_DATARATE_100_HZ: Serial.println("100 Hz"); break;
//    case LIS3DH_DATARATE_200_HZ: Serial.println("200 Hz"); break;
//    case LIS3DH_DATARATE_400_HZ: Serial.println("400 Hz"); break;
//
//    case LIS3DH_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
//    case LIS3DH_DATARATE_LOWPOWER_5KHZ: Serial.println("5 Khz Low Power"); break;
//    case LIS3DH_DATARATE_LOWPOWER_1K6HZ: Serial.println("16 Khz Low Power"); break;
//  }

//setup oled
  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(TOP);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();
  
}



void loop() {
  
  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
//  Serial.print("X:  "); Serial.print(lis.x);
//  Serial.print("  \tY:  "); Serial.print(lis.y);
//  Serial.print("  \tZ:  "); Serial.print(lis.z);

  old_meas=mat_y[i%nb];
  
  mat_y[(i++)%nb]=lis.y;

  somme+=lis.y;

  if(i>nb) somme=somme-old_meas;

  moyenne=-somme/nb;

//  Serial.print(moyenne);

  //calcul angle
  //90°= -16384   -90°=16384

  angle=(float)moyenne;
  angle=((angle*90)/16394);
  
  Serial.print(angle);
  
  
  /* Or....get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
//  Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
//  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
//  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
//  Serial.println(" m/s^2 ");

//  Serial.print(event.acceleration.x);
//  Serial.print(","); 
//  Serial.print(event.acceleration.y);

//  somme +=event.acceleration.y;


  
//  Serial.print(","); 
//  Serial.print(event.acceleration.z);
  Serial.println();

  delay(10);

//  for(j=0;j<nb;j++)
//  {
//    Serial.print(mat_y[j]);;
//    Serial.print(",");
//  }
//  Serial.println();


  //oled refresh
    //changement animation
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);

  }
}
