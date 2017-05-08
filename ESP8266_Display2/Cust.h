#ifndef _CUST_H_

#define _CUST_H_

#define AUTO_FORMAT_FS  1

#define SYSTEM_RESET_PIN  0
//LED on ESP8266 GPIO2
#define LED_LIGHT_PIN     2

#define TFT_RST 5
#define TFT_RS  4
#define TFT_CS  15  // SS
#define TFT_SDI 13  // MOSI
#define TFT_CLK 14  // SCK
#define TFT_LED 0   // 0 if wired to +5V directly

#define ENABLE_NOFITICATION   0 // Choose 1 when there is no Node-Red running, otherwise 0

extern int debug_mode;

extern void cust_init(void);
extern void cust_setup(void);
extern void cust_loop(void);

#endif

