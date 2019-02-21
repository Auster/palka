#include <SD.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "U8glib.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define BUTTON_PIN_DOWN     19
#define BUTTON_PIN_UP       23
#define BUTTON_PIN_SELECT   21
#define BUTTON_PIN_LEFT     22
#define BUTTON_PIN_RIGHT    20

#define SD_PIN_CHIP_SELECT  10

#define LCD_PIN_A0          2
#define LCD_PIN_RESET       3
#define LCD_PIN_BACKLIGHT   4
#define LCD_PIN_MOSI        7
#define LCD_PIN_CHIP_SELECT 9
#define LCD_PIN_SCK         14

#define LEDS_PIN_DATA       6
#define LEDS_NUMPIXELS      280

#define KEY_STATE_NONE      0
#define KEY_STATE_DOWN      1
#define KEY_STATE_UP        2
#define KEY_STATE_SELECT    3
#define KEY_STATE_LEFT      4
#define KEY_STATE_RIGHT     5

#define MIN_OUTPUT_INTERVAL 0      // Microseconds
#define MAX_OUTPUT_INTERVAL 10000  // Microseconds
#define DEF_OUTPUT_INTERVAL 1000   // Microseconds

#define PRE_PRINT_DELAY     0      // Delay before print (miliseconds)

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LEDS_NUMPIXELS, LEDS_PIN_DATA, NEO_GRB + NEO_KHZ800);
U8GLIB_PCD8544 u8g(LCD_PIN_SCK, LCD_PIN_MOSI, LCD_PIN_CHIP_SELECT, LCD_PIN_A0, LCD_PIN_RESET);  


boolean menu_redraw_required = false;
unsigned int error_code = 0;
unsigned int filesNum = 0;
unsigned int currentFileIndex = 0;
char* files[12] = {};

uint8_t keyCodeFirst = KEY_STATE_NONE;
uint8_t keyCodeSecond = KEY_STATE_NONE;
uint8_t keyCode = KEY_STATE_NONE;
unsigned int delay_time = DEF_OUTPUT_INTERVAL;

void initButtons(){
  pinMode(BUTTON_PIN_UP, INPUT);
  digitalWrite(BUTTON_PIN_UP, HIGH);

  pinMode(BUTTON_PIN_RIGHT, INPUT);
  digitalWrite(BUTTON_PIN_RIGHT, HIGH);

  pinMode(BUTTON_PIN_SELECT, INPUT);
  digitalWrite(BUTTON_PIN_SELECT, HIGH);

  pinMode(BUTTON_PIN_LEFT, INPUT);
  digitalWrite(BUTTON_PIN_LEFT, HIGH);

  pinMode(BUTTON_PIN_DOWN, INPUT);
  digitalWrite(BUTTON_PIN_DOWN, HIGH);
}

void readButtons(void) {
  keyCodeSecond = keyCodeFirst;
  if ( digitalRead(BUTTON_PIN_UP) == LOW )
    keyCodeFirst = KEY_STATE_UP;
  else if ( digitalRead(BUTTON_PIN_DOWN) == LOW )
    keyCodeFirst = KEY_STATE_DOWN;
  else if ( digitalRead(BUTTON_PIN_SELECT) == LOW )
    keyCodeFirst = KEY_STATE_SELECT;
  else if ( digitalRead(BUTTON_PIN_LEFT) == LOW )
    keyCodeFirst = KEY_STATE_LEFT;
  else if ( digitalRead(BUTTON_PIN_RIGHT) == LOW )
    keyCodeFirst = KEY_STATE_RIGHT;
  else 
    keyCodeFirst = KEY_STATE_NONE;
 
  if ( keyCodeSecond == keyCodeFirst )
    keyCode = keyCodeFirst;
  else
    keyCode = KEY_STATE_NONE;
  delay(10);
}

void initPalka(){
  pixels.begin(); // This initializes the NeoPixel library.

  for (int i=1; i <= LEDS_NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(128,128,128));
    pixels.show();
  }

  for (int i=LEDS_NUMPIXELS; i >= 0; i--){
    pixels.setPixelColor(i, pixels.Color(128,128,128));
    pixels.setPixelColor(i+1, pixels.Color(0,0,0));
    pixels.show();
  }
}

void cleanPalka(){
  for (int i=0; i <= LEDS_NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  pixels.show();
}

void print_image(const char *filename){
  File printFile;
  disable_backlight();
  enum {BufSize=50}; // If a is short use a smaller number, eg 5 or 6 
  char filePath[BufSize];
  snprintf (filePath, BufSize, "/PROCES~1/%s", filename);

  printFile = SD.open(filePath);
  if (printFile) {
    delay(PRE_PRINT_DELAY);
    unsigned int h_pixel = 0;
    unsigned int w_line = 0;

    while (printFile.available()) {
      char A = printFile.read();
      char buf[2];
      int r=0;
      int g=0;
      int b=0;
      
      switch (A){
        case '#':
          buf[0] = printFile.read();
          buf[1] = printFile.read();
          r = (int)strtol(buf, NULL, 16);

          buf[0] = printFile.read();
          buf[1] = printFile.read();
          g = (int)strtol(buf, NULL, 16);

          buf[0] = printFile.read();
          buf[1] = printFile.read();
          b = (int)strtol(buf, NULL, 16);

          pixels.setPixelColor(h_pixel, pixels.Color(r,g,b));

          h_pixel++;
          break;
        case '\n':
          h_pixel=0;
          pixels.show();
          delayMicroseconds(delay_time);

          w_line++;

          if((w_line % 100) == 0){
            u8g.firstPage();
            do  {
              enum {BufSize=50}; // If a is short use a smaller number, eg 5 or 6 
              char buf[BufSize];
              snprintf (buf, BufSize, "Line: %d", w_line);
              u8g.drawStr(2, 6, buf);
            } while( u8g.nextPage() );
          }
          break;
      }
    }

    // close the file:
    printFile.close();
    cleanPalka();
  } else {
  }
  enable_backlight(); 
}

void draw() {
  enum {BufSize=50}; // If a is short use a smaller number, eg 5 or 6 
  char buf[BufSize];
  snprintf (buf, BufSize, "File: %s", files[currentFileIndex]);
  u8g.drawStr(2, 6, buf);

  snprintf (buf, BufSize, "Sp: %d", delay_time);
  u8g.drawStr(2, 30, buf);

  u8g.drawHLine(0, 8, 100);

  if(filesNum > 0){
    int linesNum = filesNum;
    int filesOffset = 0;
    if (linesNum > 5) linesNum = 5;
    if (currentFileIndex > linesNum - 2 && currentFileIndex + 2 < filesNum) filesOffset = currentFileIndex - linesNum + 2;
    else if (currentFileIndex > linesNum - 1 && currentFileIndex + 1 < filesNum) filesOffset = currentFileIndex - linesNum + 1;
      
    for(unsigned int line = filesOffset; line < (filesOffset + linesNum); line++){
      int baseLine = 10 + (line - filesOffset + 1)*7;
      if(currentFileIndex == line){
        u8g.drawBox(0, baseLine - 6, 70, 7);
        u8g.setColorIndex(0);
      }
      u8g.drawStr(2, baseLine, files[line]);
      u8g.setColorIndex(1);
    }
  }
}

void printError(unsigned int error_code){
  switch(error_code){
    case 1:
      u8g.drawStr(10, 25, "SD read error");  // put string of display at position X, Y
      break;
  }
}

void initSD(){
  if (!SD.begin(SD_PIN_CHIP_SELECT)) {
    error_code = 1;
    return;
  }

  File dir;
  dir = SD.open("/PROCES~1/");

  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      break;
    } else if (!entry.isDirectory()) {
      files[filesNum] = strdup(entry.name());
      filesNum++;
    }
    entry.close();
  }
}

void enable_backlight(){
  analogWrite(LCD_PIN_BACKLIGHT, 0);
}

void disable_backlight(){
  analogWrite(LCD_PIN_BACKLIGHT, 1);
}

void initScreen(){
  enable_backlight();
  u8g.setFont(u8g_font_04b_03r);
  u8g.firstPage();
  do  {
    u8g.drawStr(15, 15, "Palka v0.0.1");  // put string of display at position X, Y
    if (error_code > 0){
      printError(error_code);      
    }
  } while( u8g.nextPage() );
}

void setup()
{
  initSD();
  initButtons();
  cleanPalka();
  initPalka();
  cleanPalka();
  initScreen();
  delay(500);
  menu_redraw_required = 1;
}

void loop()
{  
  readButtons();                                     // check for key press

  if(keyCode != 0){
    if(keyCode == KEY_STATE_DOWN && currentFileIndex < filesNum){
      currentFileIndex++;
      menu_redraw_required = 1;
    }
  
    if(keyCode == KEY_STATE_UP && currentFileIndex > 0){
      currentFileIndex--;
      menu_redraw_required = 1;
    }
    
    if(keyCode == KEY_STATE_LEFT){
      if(delay_time > MIN_OUTPUT_INTERVAL){
        delay_time = delay_time - 100;
        menu_redraw_required = 1;
      }
    }
    
    if(keyCode == KEY_STATE_RIGHT){
      if(delay_time < MAX_OUTPUT_INTERVAL){
        delay_time = delay_time + 100;
        menu_redraw_required = 1;
      }
    }
  }
  
  if (menu_redraw_required != 0) {
    u8g.firstPage();
    do  {
      draw();
    } while( u8g.nextPage() );
    menu_redraw_required = 0;
  }
  
  if (keyCode == KEY_STATE_SELECT){
    print_image(files[currentFileIndex]);
    cleanPalka();
  }
  
  while(keyCode != 0){
    readButtons();
    cleanPalka();
  }
}


