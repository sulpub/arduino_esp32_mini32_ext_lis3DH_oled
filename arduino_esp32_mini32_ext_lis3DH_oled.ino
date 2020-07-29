/*
   Measure tilt with LIS3DH sensor and display on OLED with esp32

   Code based on Basic demo for accelerometer readings from Adafruit LIS3DH
*/


/*
  .___              .__            .___
  |   | ____   ____ |  |  __ __  __| _/____
  |   |/    \_/ ___\|  | |  |  \/ __ |/ __ \
  |   |   |  \  \___|  |_|  |  / /_/ \  ___/
  |___|___|  /\___  >____/____/\____ |\___  >
         \/     \/                \/    \/
*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <TimeLib.h>
#include <Wire.h>  // For a connection via I2C using Wire includ only needed for Arduino 1.6.5 and earlier
#include "ESP32_Oled_Driver_for_SSD1306_display\SSD1306.h" // alias for `#include "SSD1306Wire.h"`

// Include the UI lib
#include "OLEDDisplayUi.h"

// Include custom images
#include "images.h"

// Used for hardware & software SPI
// Used for software SPI for LIS3DH if used
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
#define LIS3DH_CS 10

#define NB_MEAN   10    //mean measure for reduce noise measurment
#define DEBUG TRUE


/*
  ____   ____            .__      ___.   .__
  \   \ /   /____ _______|__|____ \_ |__ |  |   ____   ______
   \   Y   /\__  \\_  __ \  \__  \ | __ \|  | _/ __ \ /  ___/
    \     /  / __ \|  | \/  |/ __ \| \_\ \  |_\  ___/ \___ \
     \___/  (____  /__|  |__(____  /___  /____/\___  >____  >
                 \/              \/    \/          \/     \/
*/

//Algotihm variables
long int mean_1axis_lis3dh = 0;
long int sum_1axis_lis3dh = 0;
int nb = NB_MEAN;
long int mat_y[NB_MEAN];
int i = 0;
int j = 0;
long int old_meas = 0;
float angle = 0;

//display variable
// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 21, 22);
// SH1106 display(0x3c, D3, D5);

OLEDDisplayUi ui ( &display );

int screenW = 128; //128;
int screenH = 64;
int screenCenterX = screenW / 2;
int screenCenterY = ((screenH - 16) / 2) + 16; // top yellow part is 16 px height
int screenRadius = 23;



/*
  ___________                   __  .__
  \_   _____/_ __  ____   _____/  |_|__| ____   ____   ______
   |    __)|  |  \/    \_/ ___\   __\  |/  _ \ /    \ /  ___/
   |     \ |  |  /   |  \  \___|  | |  (  <_> )   |  \\___ \
   \___  / |____/|___|  /\___  >__| |__|\____/|___|  /____  >
       \/             \/     \/                    \/     \/
*/

// software SPI NOT USED
//Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);
// hardware SPI NOT USED
//Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);
// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();



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



/*
   _________       __
  /   _____/ _____/  |_ __ ________
  \_____  \_/ __ \   __\  |  \____ \
  /        \  ___/|  | |  |  /  |_> >
  /_______  /\___  >__| |____/|   __/
          \/     \/           |__|
*/
void setup(void) {

  //init UART
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  // Test LIS3DH
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }

  // LIS3DH range
  // lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  //LIS3DH datarate
  //1_HZ,10_HZ,25_HZ,50_HZ,100_HZ,200_HZ,400_HZ,POWERDOWN,LOWPOWER_5KHZ,LOWPOWER_1K6HZ
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

  //setup OLED
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



/*
  .____
  |    |    ____   ____ ______
  |    |   /  _ \ /  _ \\____ \
  |    |__(  <_> |  <_> )  |_> >
  |_______ \____/ \____/|   __/
          \/            |__|
*/
void loop() {

  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
  //  Serial.print("X:  "); Serial.print(lis.x);
  //  Serial.print("  \tY:  "); Serial.print(lis.y);
  //  Serial.print("  \tZ:  "); Serial.print(lis.z);

  old_meas = mat_y[i % nb];
  mat_y[(i++) % nb] = lis.y;
  sum_1axis_lis3dh += lis.y;
  if (i > nb) sum_1axis_lis3dh = sum_1axis_lis3dh - old_meas;
  mean_1axis_lis3dh = -sum_1axis_lis3dh / nb;

  //  Serial.print(mean_1axis_lis3dh);

  //calcul angle
  //90°= -16384   -90°=16384

  angle = (float)mean_1axis_lis3dh;
  angle = ((angle * 90) / 16394);

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
  //  sum_1axis_lis3dh +=event.acceleration.y;

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
