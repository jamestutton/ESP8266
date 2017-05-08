#include "MQTT.h"
#include "Log.h"
#include <NTPClient.h>
#include <TFT_22_ILI9225.h>
#include "Cust.h"

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 display = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED);

int debug_mode = 0;
int reset_config = 0;

extern NTPClient timeClient;

void cust_init(void) {
#if (SYSTEM_RESET_PIN != 0)
  pinMode(SYSTEM_RESET_PIN, INPUT);
  if (digitalRead(SYSTEM_RESET_PIN) == LOW) {
    reset_config = 1;
  }
#endif

  gLog.enable_log();
  if (debug_mode) {
    gLog.enable_serial();
  }

  //initialize the light as an output and set to LOW (off)
#if (LED_LIGHT_PIN != 0)
  pinMode(LED_LIGHT_PIN, OUTPUT);
  digitalWrite(LED_LIGHT_PIN, LOW);
#endif

  // Initialising the UI will init the display too.
  display.begin();
  SPI.setFrequency(40000000);
  //display.setOrientation(1);
  display.fillRectanglewh(0, 0, display.maxX(), display.maxY(), COLOR_BLACK);

  //display.flipScreenVertically();
  //display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Terminal6x8);
  display.setBackgroundColor(COLOR_BLACK);
}

void cust_setup(void) {
#if (LED_LIGHT_PIN != 0)
  digitalWrite(LED_LIGHT_PIN, HIGH);
#endif
}

void cust_loop(void) {
}

