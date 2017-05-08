#include "MQTT.h"
#include "SSD1306.h"
#include "Ticker.h"
#include "Log.h"
#include <NTPClient.h>
#include "Cust.h"
#include "Fonts.h"

#define WIFI_LOGO_X   2
#define WIFI_LOGO_Y   0
//#define WIFI_LOGO_W   16
#define WIFI_LOGO_W   23
#define WIFI_LOGO_H   10

//#define CLOUD_LOGO_X  44
#define CLOUD_LOGO_X  28
#define CLOUD_LOGO_Y  0
#define CLOUD_LOGO_W  17
#define CLOUD_LOGO_H  10

#define RSSI_FULL_LEVEL     -65.0
#define RSSI_CUT_OFF_LEVEL  -100.0
#define RSSI_LOGO_X   88
#define RSSI_LOGO_Y   0
#define RSSI_LOGO_W   16
#define RSSI_LOGO_H   10

// Assume battery level starts from 3.0V, and cut off at 2.0V
#define BATTERY_FULL_VOLATGE      3.0
#define BATTERY_CUT_OFF_VOLATGE   2.0
#define BATTERY_LOGO_X  107
#define BATTERY_LOGO_Y  0
#define BATTERY_LOGO_W  21
#define BATTERY_LOGO_H  10

typedef enum {
  WAKE_UP_MEASUREMENT_ONLY,
  WAKE_UP_TRANSMIT_ONLY,
  WAKE_UP_MEASUREMENT_AND_TRANSMIT,
  WAKE_UP_CLEAR_DISPLAY,
} wake_up_mode_t;

typedef struct {
  int16_t temp;
  uint16_t pressure;
  uint16_t humidity;
  uint16_t battery;
} stored_data_t;

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 12, 14);

int debug_mode = 0;
int reset_config = 0;
int enable_display = 1;
unsigned long display_last_refresh_time;

extern NTPClient timeClient;
extern Ticker led_blink;

extern void led_blink_callback(void);

void display_rssi(void) {
  if (enable_display) {
    int level;
    if (WiFi.status() == WL_CONNECTED) {
      level = (int)((WiFi.RSSI() - RSSI_CUT_OFF_LEVEL) / (RSSI_FULL_LEVEL - RSSI_CUT_OFF_LEVEL) * 8.0 + 0.5); // Round to 8 levels
      if (level < 0) {
        level = 0;
      }
      if (level > 8) {
        level = 8;
      }
    }
    else {
      level = 0;
    }
    display.setColor(BLACK);
    display.fillRect(RSSI_LOGO_X, RSSI_LOGO_Y, RSSI_LOGO_W, RSSI_LOGO_H);
    display.setColor(WHITE);
    display.drawXbm(RSSI_LOGO_X, RSSI_LOGO_Y, RSSI_LOGO_W, RSSI_LOGO_H, rssi_logos[level]);
    display.display();
  }
}

void display_mqtt_status(boolean connected) {
  if (enable_display) {
    display.setColor(BLACK);
    display.fillRect(CLOUD_LOGO_X, CLOUD_LOGO_Y, CLOUD_LOGO_W, CLOUD_LOGO_H);
    display.setColor(WHITE);
    if (connected) {
      display.drawXbm(CLOUD_LOGO_X, CLOUD_LOGO_Y, CLOUD_LOGO_W, CLOUD_LOGO_H, cloud_connected_logo);
      //display.drawXbm(CLOUD_LOGO_X, CLOUD_LOGO_Y, CLOUD_LOGO_W, CLOUD_LOGO_H, cloud_logo);
    }
    else
    {
      display.drawXbm(CLOUD_LOGO_X, CLOUD_LOGO_Y, CLOUD_LOGO_W, CLOUD_LOGO_H, cloud_disconnected_logo_1);
      //display.drawXbm(CLOUD_LOGO_X, CLOUD_LOGO_Y, CLOUD_LOGO_W, CLOUD_LOGO_H, cloud_disconnected_logo_2);
    }
    display.display();
  }
}

void display_wifi_status(void) {
#if 0
  if (enable_display) {
    display.setColor(BLACK);
    display.fillRect(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H);
    display.fillRect(WIFI_LOGO_X + WIFI_LOGO_W + 2, WIFI_LOGO_Y + 2, 18, 8);
    display.setColor(WHITE);
    if (WiFi.status() == WL_CONNECTED) {
      display.drawXbm(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H, wifi_logo);
      display.drawXbm(WIFI_LOGO_X + WIFI_LOGO_W + 2, WIFI_LOGO_Y + 2, 12, 8, connected_logo);
      display_mqtt_status(client.state() == MQTT_CONNECTED);
    }
    else
    {
      display.drawXbm(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H, wifi_logo);
      display.drawXbm(WIFI_LOGO_X + WIFI_LOGO_W + 2, WIFI_LOGO_Y + 2, 14, 8, disconnected_logo_1);
      //display.drawXbm(WIFI_LOGO_X + WIFI_LOGO_W + 2, WIFI_LOGO_Y + 2, 18, 8, disconnected_logo_2);
      display_mqtt_status(false);
    }
    display_rssi();
    display.display();
  }
#else
  if (enable_display) {
    display.setColor(BLACK);
    display.fillRect(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H);
    display.setColor(WHITE);
    if (WiFi.status() == WL_CONNECTED) {
      display.drawXbm(WIFI_LOGO_X + 6, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H, wifi_connected_logo1);
      //display.drawXbm(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H, wifi_connected_logo2);
      display_mqtt_status(client.state() == MQTT_CONNECTED);
    }
    else
    {
      display.drawXbm(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H, wifi_disconnected_logo);
      display_mqtt_status(false);
    }
    display_rssi();
    display.display();
  }
#endif
}

void display_battery_status(int battery_level) {
  if (enable_display) {
    int level = (int)(((battery_level / 1024.0 * 1.108) - BATTERY_CUT_OFF_VOLATGE) / (BATTERY_FULL_VOLATGE - BATTERY_CUT_OFF_VOLATGE) * 8.0 + 0.5); // Round to 8 levels
    if (level < 0) {
      level = 0;
    }
    if (level > 8) {
      level = 9;  // Charging?
    }
    display.setColor(BLACK);
    display.fillRect(BATTERY_LOGO_X, BATTERY_LOGO_Y, BATTERY_LOGO_W, BATTERY_LOGO_H);
    display.setColor(WHITE);
    display.drawXbm(BATTERY_LOGO_X, BATTERY_LOGO_Y, BATTERY_LOGO_W, BATTERY_LOGO_H, battery_logos[level]);
    display.display();
  }
}

void display_refresh(void) {
  display_rssi();
  display_battery_status(ESP.getVcc());
  //display_battery_status(analogRead(A0));
}

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
  display.init();
  display.clear();
  enable_display = true;

  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  display_battery_status(ESP.getVcc());
  //display_battery_status(analogRead(A0));
  display_last_refresh_time = millis();
}

void cust_setup(void) {
#if (LED_LIGHT_PIN != 0)
  digitalWrite(LED_LIGHT_PIN, HIGH);
#endif
}

void cust_loop(void) {
  if (millis() - display_last_refresh_time > 5000) {
    display_last_refresh_time = millis();
    display_refresh();
  }
}

void switchAPCallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Switch to AP mode callback");
  if (enable_display) {
    display.setColor(BLACK);
    display.fillRect(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H);
    display.setColor(WHITE);
    display.drawXbm(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H, wifi_AP_mode_logo);
    display.display();
  }
  led_blink.attach(0.2, led_blink_callback);
}

void switchSTACallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Switch to STA mode callback");
  if (enable_display) {
    display.setColor(BLACK);
    display.fillRect(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H);
    display.setColor(WHITE);
    display.drawXbm(WIFI_LOGO_X, WIFI_LOGO_Y, WIFI_LOGO_W, WIFI_LOGO_H, wifi_connecting_logo);
    display.display();
  }
  led_blink.attach(0.5, led_blink_callback);
}

void connectedCallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Wifi connected callback");
  display_wifi_status();
  led_blink.detach();
#if (LED_LIGHT_PIN != 0)
  digitalWrite(LED_LIGHT_PIN, HIGH);
#endif
}

void connectFailCallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Wifi connect failed callback");
  display_wifi_status();
  led_blink.attach(0.3, led_blink_callback);
}

void disconnectCallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Wifi disconnect callback");
  display_wifi_status();
}
