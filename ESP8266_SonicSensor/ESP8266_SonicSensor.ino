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

extern int reset_config;
extern char device_name[];
extern char wifi_ssid1[];
extern char wifi_password1[];
extern char wifi_ssid2[];
extern char wifi_password2[];

Ticker led_blink;

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

  //start the serial line for debugging
  Serial.begin(115200);
  Serial.println("");
  delay(10);
  gLog.println(__FILE__ " Build data:" __DATE__ " " __TIME__);

  //clean FS, for testing
#if 0
  gLog.println("Formatting FS");
  SPIFFS.format();
#endif

  if (reset_config) {
    gLog.println("SYSTEM RESET!!!");
    led_blink.attach(0.1, led_blink_callback);
    delay(1000);
  }

  //read configuration from FS json
  gLog.println("Loading config");
  load_config();

  config_host_name(device_name);

  gLog.println("Connecting Wifi");

  if ((wifi_ssid1[0] != 0) && (wifi_password1[0] != 0) && (connect_wifi(wifi_ssid1, wifi_password1, 8000, 1, NULL) == WL_CONNECTED)) {
    // Connected to wifi1
  }
  else if ((wifi_ssid2[0] != 0) && (wifi_password2[0] != 0) && (connect_wifi(wifi_ssid2, wifi_password2, 8000, 1, NULL) == WL_CONNECTED)) {
    // Connected to wifi2
  }
  else {
    connect_wifi();
  }

  //if you get here you have connected to the WiFi
  gLog.println("Wifi connected");

  if (shouldSaveConfig) {
    config_host_name(device_name);
    save_config();
    delay(300);
    ESP.reset();
    delay(100);
  }

  gLog.println("Local IP:");
  gLog.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(-4 * 3600);
  timeClient.setUpdateInterval(3600);
  if (timeClient.forceUpdate()) {
    gLog.print(timeClient.getFormattedTime());
    gLog.println(" NTP updated");
  }

  setup_ota_server();

  setup_mqtt();

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

