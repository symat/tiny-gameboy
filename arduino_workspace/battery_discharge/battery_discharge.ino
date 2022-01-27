

// softwareI2C lib
#define I2C_HARDWARE 0
#define I2C_TIMEOUT 10
#define I2C_MAXWAIT 10
#define I2C_PULLUP 1
#define I2C_FASTMODE 1
#define SDA_PORT PORTB
#define SDA_PIN 0
#define SCL_PORT PORTB
#define SCL_PIN 2

// tinyprint lib
#define TP_PRINTLINES 0
#define TP_FLASHSTRINGHELPER 1
#define TP_NUMBERS 1
#define TP_FLOAT 0
#define TP_WINDOWSLINEENDS 0


#include "SH1106Lib.h"
#include "font_3x5.h"


#define RANDOM_SEED_BY_JOYSTICK 1
#define BUTTON_3_ENABLED 1
#include "gameboy_inputs.h"

SH1106Lib display;


void setup() {
  display.initialize();
  display.clearDisplay();

}


void loop() {

  display.fillRect(0, 0, 128, 64, WHITE);
   
  read_all_inputs();

    
}
