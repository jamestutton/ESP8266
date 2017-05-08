#ifndef _CUST_H_

#define _CUST_H_

#define AUTO_FORMAT_FS  1

#define SYSTEM_RESET_PIN  0
//LED on ESP8266 GPIO2
#define LED_LIGHT_PIN     0
#define SERVO_SIGNAL_PIN  0

#define DISPLAY_STAY_TIME 3000
#define DEFAULT_DEEP_SLEEP_DELAY    100
#define TRANSMIT_ONLY_DELAY         500

#define DATA_STORE_BEFORE_TRANSMIT  10

#define DATA_SLOTS  16

#define ENABLE_NOFITICATION   0 // Choose 1 when there is no Node-Red running, otherwise 0

extern int debug_mode;

extern void disconnectCallback(void);
extern void cust_init(void);
extern void cust_setup(void);
extern void cust_loop(void);

#endif

