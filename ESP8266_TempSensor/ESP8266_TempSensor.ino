#include "adc.h"
extern "C" {
  #include "user_interface.h"
}
#include "Config.h"
#include "Wifi.h"
#include "MQTT.h"
#include "OTA.h"
#include "Helper.h"
#include "Ticker.h"
#include "Log.h"
#include "Cust.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Ticker led_blink;

extern int reset_config;

void led_blink_callback(void) {
#if (LED_LIGHT_PIN != 0)
  digitalWrite(LED_LIGHT_PIN, !digitalRead(LED_LIGHT_PIN));
#endif
}

void firmware_upload_start_callback(void) {
  gLog.println("Firmware uploading. Stop MQTT");
  client.disconnect();
}

void setup() {
  cust_init();

  if (debug_mode) {
    gLog.enable_log();
  }
  if (debug_mode) {
    //gLog.enable_serial();
  }

  if (debug_mode >= 2) {
    //start the serial line for debugging
    Serial.begin(115200);
    Serial.println("");
    delay(10);
    gLog.enable_serial();
  }

#if (SYSTEM_RESET_PIN != 0)
  pinMode(SYSTEM_RESET_PIN, INPUT);
  if (digitalRead(SYSTEM_RESET_PIN) == LOW) {
    reset_config = 1;
  }
#endif

  gLog.println(__FILE__ " Build data:" __DATE__ " " __TIME__);

  cust_setup();
}

void loop() {
  //reconnect if connection is lost
  if (WiFi.status() != WL_CONNECTED) {
    gLog.print(timeClient.getFormattedTime());
    gLog.printf(" Wifi disconnected %d\n", WiFi.status());
    gLog.println("Connecting Wifi");
    WiFi.disconnect();
    connect_wifi();
    setup_mqtt();
  }

  //maintain MQTT connection
  if (!client.loop()) {
    setup_mqtt();
  }

  if (WiFi.status() == WL_CONNECTED) {
    httpServer.handleClient();
  }

  timeClient.update();

  cust_loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10);
}

void switchAPCallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Switch to AP mode callback");
  led_blink.attach(0.2, led_blink_callback);
}

void switchSTACallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Switch to STA mode callback");
  led_blink.attach(0.5, led_blink_callback);
}

void connectedCallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Wifi connected callback");
  led_blink.detach();
#if (LED_LIGHT_PIN != 0)
  digitalWrite(LED_LIGHT_PIN, HIGH);
#endif
}

void connectFailCallback(void) {
  gLog.print(timeClient.getFormattedTime());
  gLog.println(" Wifi connect failed callback");
  led_blink.attach(0.3, led_blink_callback);
}

