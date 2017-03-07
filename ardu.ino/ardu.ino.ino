#include <SD.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "U8glib.h"
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define up_button         19
#define right_button      20
#define select_button     21
#define left_button       22
#define down_button       23

#define BACKLIGHT_PIN     4
#define PALKA_LED_PIN     6
#define NUMPIXELS         280

#define KEY_NONE          0
#define KEY_UP            1
#define KEY_DOWN          2
#define KEY_SELECT        3
#define KEY_LEFT          4
#define KEY_RIGHT         5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PALKA_LED_PIN, NEO_GRB + NEO_KHZ800);
File myFile;
U8GLIB_PCD8544 u8g(14, 7, 9, 2, 3);  

boolean menu_redraw_required = false;
const int chipSelect = 10;
uint8_t uiKeyCodeFirst = KEY_NONE;
uint8_t uiKeyCodeSecond = KEY_NONE;
uint8_t uiKeyCode = KEY_NONE;

void uiStep(void) {
  uiKeyCodeSecond = uiKeyCodeFirst;
  if ( digitalRead(up_button) == LOW )
    uiKeyCodeFirst = KEY_UP;
  else if ( digitalRead(down_button) == LOW )
    uiKeyCodeFirst = KEY_DOWN;
  else if ( digitalRead(select_button) == LOW )
    uiKeyCodeFirst = KEY_SELECT;
  else if ( digitalRead(left_button) == LOW )
    uiKeyCodeFirst = KEY_LEFT;
  else if ( digitalRead(right_button) == LOW )
    uiKeyCodeFirst = KEY_RIGHT;
  else 
    uiKeyCodeFirst = KEY_NONE;
 
  if ( uiKeyCodeSecond == uiKeyCodeFirst )
    uiKeyCode = uiKeyCodeFirst;
  else
    uiKeyCode = KEY_NONE;
  delay(10);
}

void init_palka(){
  u8g.drawStr(15, 25, "Palka v0.0.1");  // put string of display at position X, Y  

  for (int i=1; i <= NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(255,255,255));
    pixels.show();
  }
  for (int i=NUMPIXELS; i >= 0; i--){
    pixels.setPixelColor(i, pixels.Color(255,255,255));
    pixels.setPixelColor(i+1, pixels.Color(0,0,0));
    pixels.show();
  }
}

void clean_palka(){
  for (int i=0; i <= NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  pixels.show();
}

void print_image(const char *filename){
  myFile = SD.open(filename);
  if (myFile) {
    unsigned int h_pixel = 0;

    while (myFile.available()) {
      char A = myFile.read();
      char buf[2];
      int r=0;
      int g=0;
      int b=0;

      switch (A){
        case '#':
          buf[0] = myFile.read();
          buf[1] = myFile.read();
          r = (int)strtol(buf, NULL, 16);

          buf[0] = myFile.read();
          buf[1] = myFile.read();
          g = (int)strtol(buf, NULL, 16);

          buf[0] = myFile.read();
          buf[1] = myFile.read();
          b = (int)strtol(buf, NULL, 16);

          pixels.setPixelColor(h_pixel, pixels.Color(r,g,b));

          h_pixel++;
          break;
        case '\n':
          h_pixel=0;
          pixels.show();
          delay(1);
          break;
      }
    }
    // close the file:
    myFile.close();
    clean_palka();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening print.txt");
  }  
}

void draw_dir() {
  File dir;
  dir = SD.open("/");
  int line_num = 0;

  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }

    if (!entry.isDirectory()) {
      u8g.drawStr(15, line_num+7, entry.name());
      line_num+=7;
    }
    entry.close();
  } 
}

void setup()
{
  Serial.begin(115200);
  pixels.begin(); // This initializes the NeoPixel library.
  analogWrite(BACKLIGHT_PIN, 0);
  u8g.setFont(u8g_font_04b_03r);
//  init_palka();
  clean_palka();

  pinMode(21, INPUT);
  digitalWrite(21, HIGH);
  
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }

  draw_dir();
}

void loop()
{
  uiStep();                                     // check for key press

  if (  menu_redraw_required != 0 ) {
    u8g.firstPage();
    do  {
      drawMenu();
    } while( u8g.nextPage() );
    menu_redraw_required = 0;
  }
  
  if (digitalRead(21) == LOW){
    print_image("print.txt");
    clean_palka();
  }
  
//  updateMenu();                            // update menu bar
}


